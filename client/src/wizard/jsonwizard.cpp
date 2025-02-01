/**
 * client/src/wizard/jsonwizard.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "jsonwizard.hpp"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QListWidget>
#include <QButtonGroup>
#include <QAbstractButton>
#include <QFormLayout>
#include <QFutureWatcher>
#include <traintastic/locale/locale.hpp>
#include "../network/create/createinterface.hpp"
#include "../network/object.hpp"
#include "page/listpage.hpp"
#include "page/propertypage.hpp"
#include "page/radiopage.hpp"

static QJsonDocument loadJSON(const QString& filename)
{
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    return {};
  }
  return QJsonDocument::fromJson(file.readAll());
}

static void setTitleAndText(JSONWizard& wizard, TextPage* page, const QJsonObject& object)
{
  page->setTitle(Locale::instance->parse(object["title"].toString()));
  page->setText(wizard.translateAndReplaceVariables(object["text"].toString()));
}

class PageJSON
{
  public:
    virtual QJsonObject getActions() const
    {
      return {};
    }
};

class TextPageJSON : public TextPage, public PageJSON
{
  private:
    const QJsonObject& m_pageData;

  public:
    TextPageJSON(const QJsonObject& pageData, JSONWizard& wizard)
      : TextPage(&wizard)
      , m_pageData{pageData}
    {
    }

    void initializePage() override
    {
      setTitleAndText(*static_cast<JSONWizard*>(wizard()), this, m_pageData);
    }

    int nextId() const override
    {
      const QString next = m_pageData["next"].toString();
      return next.isEmpty() ? -1 : static_cast<JSONWizard*>(wizard())->getPageId(next);
    }
};

class ListPageJSON : public ListPage, public PageJSON
{
  private:
    const QJsonObject& m_pageData;

  public:
    ListPageJSON(const QJsonObject& pageData, JSONWizard& wizard)
      : ListPage(&wizard)
      , m_pageData{pageData}
    {
      connect(m_list, &QListWidget::currentItemChanged, this,
        [this]()
        {
          emit completeChanged();
        });
    }

    void initializePage() override
    {
      setTitleAndText(*static_cast<JSONWizard*>(wizard()), this, m_pageData);

      QStringList items;
      for(const auto& option : m_pageData["options"].toArray())
      {
        items.append(static_cast<JSONWizard*>(wizard())->translateAndReplaceVariables(option.toObject()["name"].toString()));
      }
      setItems(items);
    }

    void cleanupPage() override
    {
      m_list->clear();
    }

    bool isComplete() const override
    {
      return selectedItemIndex() >= 0;
    }

    int nextId() const override
    {
      if(isComplete())
      {
        auto next = m_pageData["options"].toArray()[selectedItemIndex()].toObject()["next"].toString();
        return static_cast<JSONWizard*>(wizard())->getPageId(next);
      }
      return JSONWizard::errorPage;
    }

    QJsonObject getActions() const override
    {
      assert(isComplete());
      return m_pageData["options"].toArray()[selectedItemIndex()].toObject()["actions"].toObject();
    }
};

class PropertyPageJSON : public PropertyPage, public PageJSON
{
  private:
    const QJsonObject& m_pageData;
    ObjectPtr m_object;
    QFutureWatcher<ObjectPtr>* m_objectFutureWatcher = nullptr;

    void addProperties()
    {
      assert(m_object);
      for(const auto& propertyName : m_pageData["properties"].toArray())
      {
        if(auto* property = m_object->getProperty(propertyName.toString()))
        {
          addProperty(*property);
        }
      }
    }

  public:
    PropertyPageJSON(const QJsonObject& pageData, JSONWizard& wizard)
      : PropertyPage(&wizard)
      , m_pageData{pageData}
    {
    }

    void initializePage() override
    {
      setTitleAndText(*static_cast<JSONWizard*>(wizard()), this, m_pageData);

      auto objectFuture = static_cast<JSONWizard*>(wizard())->getObject(m_pageData["object"].toString());
      if(objectFuture.isFinished())
      {
        m_object = objectFuture.result();
        addProperties();
      }
      else
      {
        m_objectFutureWatcher = new QFutureWatcher<ObjectPtr>(this);
        m_objectFutureWatcher->setFuture(objectFuture);
        connect(m_objectFutureWatcher, &QFutureWatcher<ObjectPtr>::finished,
          [this]()
          {
            m_object = m_objectFutureWatcher->future().result();
            addProperties();
          });
      }
    }

    void cleanupPage() override
    {
      while(m_propertyLayout->rowCount() != 0)
      {
        m_propertyLayout->removeRow(0);
      }
    }

    int nextId() const override
    {
      const QString next = m_pageData["next"].toString();
      return next.isEmpty() ? -1 : static_cast<JSONWizard*>(wizard())->getPageId(next);
    }
};

class RadioPageJSON : public RadioPage, public PageJSON
{
  private:
    const QJsonObject& m_pageData;

  public:
    RadioPageJSON(const QJsonObject& pageData, JSONWizard& wizard)
      : RadioPage(&wizard)
      , m_pageData{pageData}
    {
      connect(m_group, qOverload<QAbstractButton*>(&QButtonGroup::buttonClicked), this,
        [this](QAbstractButton* /*button*/)
        {
          emit completeChanged();
        });
    }

    void initializePage() override
    {
      auto* jsonWizard = static_cast<JSONWizard*>(wizard());

      setTitleAndText(*jsonWizard, this, m_pageData);

      for(const auto& option : m_pageData["options"].toArray())
      {
        auto item = option.toObject();
        addItem(jsonWizard->translateAndReplaceVariables(item["name"].toString()), item["checked"].toBool(), item["disabled"].toBool());
      }

      setBottomText(jsonWizard->translateAndReplaceVariables(m_pageData["bottom_text"].toString()));
    }

    void cleanupPage() override
    {
      clear();
    }

    bool isComplete() const override
    {
      return currentIndex() >= 0;
    }

    int nextId() const override
    {
      if(isComplete())
      {
        auto next = m_pageData["options"].toArray()[currentIndex()].toObject()["next"].toString();
        return static_cast<JSONWizard*>(wizard())->getPageId(next);
      }
      return JSONWizard::errorPage;
    }

    QJsonObject getActions() const override
    {
      assert(isComplete());
      return m_pageData["options"].toArray()[currentIndex()].toObject()["actions"].toObject();
    }
};

JSONWizard::JSONWizard(const QString& filename, ObjectPtr world, QWidget* parent)
  : Wizard(parent)
  , m_world{std::move(world)}
{
  auto doc = loadJSON(filename);

  // setup error page in case something goes wrong:
  {
    auto* page = new TextPage(this);
    page->setTitle("Error");
    page->setText("Sorry, the wizard is broken, please report this on <a href=\"https://github.com/traintastic/traintastic/issues\">GitHub</a> or the <a href=\"https://forum.traintastic.org\">forum</a>.");
    page->setFinalPage(true);
    setPage(errorPage, page);
  }

  if(doc.isObject()) /*[[likely]]*/
  {
    auto root = doc.object();

    auto wizard = root["wizard"].toObject();
    setWindowTitle(Locale::instance->parse(wizard["title"].toString()));

    auto pages = wizard["pages"].toObject();
    for(const auto& id : pages.keys())
    {
      m_pages.emplace(id, PageInfo{errorPage, pages[id].toObject()});
    }

    if(m_pages.find("start") != m_pages.end())
    {
      setStartId(createPage(m_pages["start"]));
    }
  }

  if(m_pages.empty())
  {
    setStartId(errorPage);
  }
}

JSONWizard::~JSONWizard() = default;


QString JSONWizard::translateAndReplaceVariables(const QString& text) const
{
  auto result = Locale::instance->parse(text);

  static const QRegularExpression re{"%([a-z_]+)%"};
  auto match = re.match(result);
  while(match.hasMatch())
  {
    if(auto it = m_variables.find(match.captured(1)); it != m_variables.end())
    {
      auto value = it->second.toString();
      result.replace(match.capturedStart(), match.capturedLength(), Locale::instance->parse(value));
      match = re.match(result, match.capturedStart() + value.length());
    }
    else
    {
      match = re.match(result, match.capturedEnd());
    }
  }

  return result;
}

QFuture<ObjectPtr> JSONWizard::getObject(const QString& reference) const
{
  assert(m_objects.find(reference) != m_objects.end());
  return m_objects.at(reference);
}

int JSONWizard::getPageId(const QString& page)
{
  if(auto it = m_pages.find(page); it != m_pages.end())
  {
    return (it->second.id == errorPage) ? createPage(it->second) : it->second.id;
  }
  return errorPage;
}

void JSONWizard::initializePage(int id)
{
  if(!m_ids.empty())
  {
    if(auto* pageJSON = dynamic_cast<PageJSON*>(page(m_ids.back())))
    {
      if(auto actions = pageJSON->getActions(); !actions.isEmpty())
      {
        doActions(actions);
      }
    }
  }

  m_ids.append(id);

  Wizard::initializePage(id);
}

void JSONWizard::cleanupPage(int id)
{
  m_ids.pop_back();

  if(!m_ids.empty())
  {
    if(auto* pageJSON = dynamic_cast<PageJSON*>(page(m_ids.back())))
    {
      if(auto actions = pageJSON->getActions(); !actions.isEmpty())
      {
        undoActions(actions);
      }
    }
  }
  QWizard::cleanupPage(id);
}

int JSONWizard::createPage(PageInfo& pageInfo)
{
  assert(pageInfo.id == errorPage);
  const QString type = pageInfo.data["type"].toString();
  if(type == "text")
  {
    pageInfo.id = addPage(new TextPageJSON(pageInfo.data, *this));
  }
  else if(type == "list")
  {
    pageInfo.id = addPage(new ListPageJSON(pageInfo.data, *this));
  }
  else if(type == "property")
  {
    pageInfo.id = addPage(new PropertyPageJSON(pageInfo.data, *this));
  }
  else if(type == "radio")
  {
    pageInfo.id = addPage(new RadioPageJSON(pageInfo.data, *this));
  }
  return pageInfo.id;
}

void JSONWizard::doActions(const QJsonObject& actions)
{
  if(auto createInterface = actions["create_interface"].toObject(); !createInterface.isEmpty())
  {
    if(const QString classId = createInterface["class_id"].toString(); !classId.isEmpty())
    {
      const QString reference = createInterface["reference"].toString("interface");
      m_createInterface = std::make_shared<CreateInterface>(m_world, classId, toProperties(createInterface["properties"].toObject()));
      m_objects.emplace(reference, m_createInterface->future());
    }
  }
  if(auto setVariables = actions["set_variables"].toObject(); !setVariables.isEmpty())
  {
    for(const auto& name : setVariables.keys())
    {
      m_variables.emplace(name, setVariables[name].toVariant());
    }
  }
}

void JSONWizard::undoActions(const QJsonObject& actions)
{
  if(auto createInterface = actions["create_interface"].toObject(); !createInterface.isEmpty())
  {
    const QString reference = createInterface["reference"].toString("interface");
    m_createInterface->cancel();
    m_createInterface.reset();
    m_objects.erase(reference);
  }
  if(auto setVariables = actions["set_variables"].toObject(); !setVariables.isEmpty())
  {
    for(const auto& name : setVariables.keys())
    {
      m_variables.erase(name);
    }
  }
}

Properties JSONWizard::toProperties(const QJsonObject& object)
{
  Properties properties;
  for(const auto& key : object.keys())
  {
    auto value = object[key];

    if(value.isString())
    {
      static const QRegularExpression re{"^%([a-z_]+)%$"};
      auto match = re.match(value.toString());
      if(match.hasMatch())
      {
        if(auto it = m_variables.find(match.captured(1)); it != m_variables.end())
        {
          properties.emplace_back(key, it->second);
          continue;
        }
      }
    }

    if(value.isBool() || value.isDouble() || value.isString())
    {
      properties.emplace_back(key, value.toVariant());
    }
    else
    {
      assert(false);
    }
  }
  return properties;
}
