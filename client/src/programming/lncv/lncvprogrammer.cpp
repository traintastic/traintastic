/**
 * client/src/programming/lncv/lncvprogrammer.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2025 Reinder Feenstra
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

#include "lncvprogrammer.hpp"
#include <cmath>
#include <QStackedWidget>
#include <QStatusBar>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QDebug>
#include <traintastic/locale/locale.hpp>
#include <traintastic/utils/standardpaths.hpp>
#include "lncvprogramminglistmodel.hpp"
#include "../../network/connection.hpp"
#include "../../network/callmethod.hpp"
#include "../../network/event.hpp"
#include "../../theme/theme.hpp"

static QDomElement getElementByLanguage(const QList<QDomElement>& list, const QString& language)
{
  for(const auto &element : list)
    if(element.attribute("language", "") == language)
      return element;

  return {};
}

static QDomElement getElementByLanguage(const QDomElement& element, const QString& tagName)
{
  static const auto appLanguage = QString::fromStdU32String(Locale::instance->filename.filename().u32string());

  QList<QDomElement> list;
  for(QDomElement e = element.firstChildElement(tagName); !e.isNull(); e = e.nextSiblingElement(tagName))
    list.append(e);

  QDomElement e = getElementByLanguage(list, appLanguage);
  if(!e.isNull())
    return e;

  e = getElementByLanguage(list, "en-us");
  if(!e.isNull())
    return e;

  e = getElementByLanguage(list, "");
  if(!e.isNull())
    return e;

  return {};
}

static int getAttributeBool(const QDomElement element, const QString attribute, bool default_ = false)
{
  const auto s = element.attribute(attribute);
  if(s == "true")
    return true;
  else if(s == "false")
    return false;
  return default_;
}

static int getAttributeInt(const QDomElement element, const QString attribute, int default_ = 0, int min = std::numeric_limits<int>::min(), int max = std::numeric_limits<int>::max())
{
  bool ok;
  const int r = element.attribute(attribute).toInt(&ok);
  return (ok && r >= min && r <= max) ? r : default_;
}

static double getAttributeFloat(const QDomElement element, const QString attribute, double default_ = std::numeric_limits<double>::quiet_NaN())
{
  bool ok;
  const double r = element.attribute(attribute).toDouble(&ok);
  return ok ? r : default_;
}

LNCVProgrammer::LNCVProgrammer(std::shared_ptr<Connection> connection, QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
  , m_connection{std::move(connection)}
  , m_requestId{Connection::invalidRequestId}
  , m_pages{new QStackedWidget(this)}
  , m_statusBar{new QStatusBar(this)}
  , m_interface{new QComboBox(this)}
  , m_module{new QComboBox(this)}
  , m_otherModule{new QSpinBox(this)}
  , m_address{new QSpinBox(this)}
  , m_broadcastAddress{new QCheckBox(Locale::tr("lncv_programmer:use_broadcast_address"), this)}
  , m_lncvs{new QTableWidget(0, 3, this)}
  , m_start{new QPushButton(Locale::tr("lncv_programmer:start"))}
  , m_read{new QPushButton(Locale::tr("lncv_programmer:read"))}
  , m_write{new QPushButton(Locale::tr("lncv_programmer:write"))}
  , m_stop{new QPushButton(Locale::tr("lncv_programmer:stop"))}
{
  setWindowTitle(Locale::tr("lncv_programmer:lncv_programmer"));
  Theme::setWindowIcon(*this, "lncv_programmer");

  loadInterfaces();
  connect(m_interface, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LNCVProgrammer::updateStartEnabled);

  loadModules();
  connect(m_module, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LNCVProgrammer::moduleChanged);

  m_otherModule->setValue(0);
  m_otherModule->setRange(moduleIdMin, moduleIdMax);

  m_address->setValue(1);
  m_address->setRange(1, 65535);

  connect(m_broadcastAddress, &QCheckBox::toggled, this, &LNCVProgrammer::useBroadcastAddressChanged);

  connect(m_start, &QPushButton::clicked,
    [this]()
    {
      loadLNCVs();
      m_pages->widget(pageStart)->setEnabled(false);
      m_statusBar->showMessage(Locale::tr("lncv_programmer:sending_start"));

      if(const auto& world = m_connection->world())
      {
        if(auto* getLNCVProgrammer = world->getMethod("get_lncv_programmer"))
        {
          callMethodR<ObjectPtr>(*getLNCVProgrammer,
            [this](const ObjectPtr& object, std::optional<const Error> /*error*/)
            {
              if(object)
              {
                if(auto* onReadResponse = object->getEvent("on_read_response"))
                {
                  connect(onReadResponse, &Event::fired, this,
                    [this](QVariantList arguments)
                    {
                      const bool success = arguments[0].toBool();
                      const int lncv = arguments[1].toInt();
                      const int value = arguments[2].toInt();

                      if(success)
                      {
                        m_statusBar->clearMessage();

                        if(auto it = m_lncvToRow.find(lncv); it != m_lncvToRow.end())
                        {
                          auto* w = m_lncvs->cellWidget(it->second, columnValue);
                          if(auto* spinBox = dynamic_cast<QSpinBox*>(w))
                          {
                            spinBox->setEnabled(true);
                            spinBox->setValue(value);
                          }
                          else if(auto* doubleSpinBox = dynamic_cast<QDoubleSpinBox*>(w))
                          {
                            const double gain = doubleSpinBox->property("lncv_gain").toDouble();
                            const double offset = doubleSpinBox->property("lncv_offset").toDouble();
                            doubleSpinBox->setEnabled(true);
                            doubleSpinBox->setValue(value * gain + offset);
                          }
                          else if(auto* comboBox = dynamic_cast<QComboBox*>(w))
                          {
                            comboBox->setEnabled(true);
                            bool found = false;
                            const int itemCount = comboBox->count();
                            for(int i = 0; i < itemCount; i++)
                            {
                              if(comboBox->itemData(i) == value)
                              {
                                comboBox->setCurrentIndex(i);
                                found = true;
                                break;
                              }
                            }
                            if(!found)
                            {
                              comboBox->addItem(QString::number(value), value);
                              comboBox->setCurrentIndex(itemCount);
                            }
                          }
                          else
                          {
                            m_lncvs->setItem(it->second, columnValue, new QTableWidgetItem(QString::number(value)));
                          }
                        }
                      }
                      else
                      {
                        m_statusBar->showMessage(Locale::tr("lncv_programmer:reading_lncv_x_failed").arg(lncv), showMessageTimeout);
                      }

                      setState(State::Idle);
                    });
                }
                else
                  assert(false);

                if(auto* start = object->getMethod("start"))
                {
                  m_object = object;
                  m_pages->setCurrentIndex(pageProgramming);
                  m_read->setEnabled(false);
                  m_write->setEnabled(false);

                  const int moduleId =
                    (m_module->currentIndex() == m_module->count() - 1)
                      ? m_otherModule->value()
                      : m_module->currentData(roleModuleId).toInt();

                  setState(State::WaitForStart);

                  callMethodR<bool>(*start,
                    [this](const bool sent, std::optional<const Error> /*error*/)
                    {
                      if(sent)
                      {
                        m_statusBar->showMessage(Locale::tr("lncv_programmer:waiting_for_module_to_respond"));
                      }
                      else
                      {
                        m_statusBar->showMessage(Locale::tr("lncv_programmer:sending_start_failed"), showMessageTimeout);
                        reset();
                      }
                    }, moduleId, m_address->value());
                }
                else
                  assert(false);
              }
            }, m_interface->currentText());
        }
        else
          assert(false);
      }
      else
        assert(false);
    });

  connect(m_read, &QPushButton::clicked,
    [this]()
    {
      if(const int lncv = getSelectedLNCV(); lncv != -1)
      {
        m_statusBar->showMessage(Locale::tr("lncv_programmer:reading_lncv_x").arg(lncv));
        if(auto* read = m_object->getMethod("read"))
        {
          callMethod(*read, nullptr, lncv);
          setState(State::WaitForRead);
        }
      }
    });

  connect(m_write, &QPushButton::clicked,
    [this]()
    {
      if(const int lncv = getSelectedLNCV(); lncv != -1)
      {
        if(const int lncvValue = getSelectedLNCVValue(); lncvValue != -1)
        {
          m_statusBar->showMessage(Locale::tr("lncv_programmer:writing_lncv_x").arg(lncv));
          if(auto* write = m_object->getMethod("write"))
          {
            callMethod(*write, nullptr, lncv, lncvValue);
            setState(State::WaitForWrite);
          }
        }
      }
    });

  connect(m_stop, &QPushButton::clicked,
    [this]()
    {
      if(auto* stop = m_object->getMethod("stop"))
      {
        m_statusBar->clearMessage();
        callMethod(*stop);
        reset();
      }
      else
        assert(false);
    });

  m_lncvs->setHorizontalHeaderLabels({QString("LNCV"), Locale::tr("lncv_programmer:value"), Locale::tr("lncv_programmer:description")});
  m_lncvs->horizontalHeader()->setStretchLastSection(true);
  m_lncvs->verticalHeader()->setVisible(false);
  m_lncvs->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_lncvs->setSelectionMode(QAbstractItemView::SingleSelection);
  m_lncvs->setSelectionBehavior(QAbstractItemView::SelectRows);
  connect(m_lncvs, &QTableWidget::currentItemChanged,
    [this](QTableWidgetItem* /*current*/, QTableWidgetItem* /*previous*/)
    {
      updateReadWriteEnabled();
    });

  // start page:
  {
    auto* form = new QFormLayout();
    form->addRow(Locale::tr("hardware:interface"), m_interface);
    form->addRow(Locale::tr("lncv_programmer:module"), m_module);
    form->addRow("", m_otherModule);
    form->addRow(Locale::tr("hardware:address"), m_address);
    form->addRow("", m_broadcastAddress);
    form->addRow("", m_start);
    auto* w = new QWidget(this);
    w->setLayout(form);
    m_pages->addWidget(w);
  }

  // programming page:
  {
    auto* v = new QVBoxLayout();
    v->addWidget(m_lncvs);

    auto* h = new QHBoxLayout();
    h->addStretch();
    h->addWidget(m_read);
    h->addWidget(m_write);
    h->addWidget(m_stop);
    h->addStretch();
    v->addLayout(h);
    auto* w = new QWidget(this);
    w->setLayout(v);
    m_pages->addWidget(w);
  }

  auto* l = new QVBoxLayout();
  l->setContentsMargins(0, 0, 0, 0);
  l->addWidget(m_pages);
  l->addWidget(m_statusBar);
  setLayout(l);
}

LNCVProgrammer::~LNCVProgrammer()
{
  if(m_requestId != Connection::invalidRequestId)
    m_connection->cancelRequest(m_requestId);
  m_object.reset();
}

void LNCVProgrammer::loadInterfaces()
{
  m_requestId = m_connection->getObject("world.lncv_programming_controllers",
    [this](const ObjectPtr& object, std::optional<const Error> /*error*/)
    {
      m_requestId = Connection::invalidRequestId;

      if(object)
        m_requestId = m_connection->getTableModel(object,
          [this](const TableModelPtr& table, std::optional<const Error> /*error*/)
          {
            m_requestId = Connection::invalidRequestId;

            if(table)
              m_interface->setModel(new LNCVProgrammingListModel(table, m_interface));
          });
    });
}

void LNCVProgrammer::loadModules()
{
  m_module->clear();
  m_module->addItem("");

  QDir moduleDir(QString::fromStdString(getLNCVXMLPath().string()));
  moduleDir.setNameFilters({"*.xml"});
  for(const QString& entry : moduleDir.entryList(QDir::Files | QDir::Readable, QDir::Name))
  {
    const QString filename = moduleDir.absoluteFilePath(entry);
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
      continue;

    QDomDocument doc;
    if(!doc.setContent(&file))
      continue;

    QDomElement lncvModule = doc.documentElement();
    if(lncvModule.tagName() != "lncvmodule" || !lncvModule.hasAttribute("id"))
      continue;

    bool ok;
    if(int moduleId = lncvModule.attribute("id").toInt(&ok); ok && moduleId >= moduleIdMin && moduleId <= moduleIdMax)
    {
      if(auto name = getElementByLanguage(lncvModule, "name"); !name.isNull())
      {
        QString label{name.text()};

        if(auto vendor = getElementByLanguage(lncvModule, "vendor"); !name.isNull())
          label.prepend(" ").prepend(vendor.text());

        m_module->addItem(label, filename);
        m_module->setItemData(m_module->count() - 1, moduleId, roleModuleId);
      }
    }
  }

  m_module->addItem(Locale::tr("lncv_programmer:other_module"));

  moduleChanged();
}

void LNCVProgrammer::loadLNCVs()
{
  m_lncvs->setRowCount(0);
  m_lncvToRow.clear();

  if(const auto filename = m_module->currentData(roleFilename).toString(); !filename.isEmpty())
  {
    QFile file(filename);
    if(file.open(QIODevice::ReadOnly))
    {
      QDomDocument doc;
      if(doc.setContent(&file))
      {
        QDomElement lncvModule = doc.documentElement();

        bool ok;
        QDomElement lncv = lncvModule.firstChildElement("lncv");
        while(!lncv.isNull())
        {
          if(int number = lncv.attribute("lncv").toInt(&ok); ok && number >= lncvMin && number <= lncvMax)
          {
            const int row = m_lncvs->rowCount();
            m_lncvs->setRowCount(row + 1);

            m_lncvs->setItem(row, columnLNCV, new QTableWidgetItem(lncv.attribute("lncv")));
            m_lncvs->setItem(row, columnDescription, new QTableWidgetItem(getElementByLanguage(lncv, "name").text()));

            QWidget* w = nullptr;
            if(QDomElement value = lncv.firstChildElement("value"); !value.isNull())
            {
              if(getAttributeBool(value, "readonly"))
              {
                // no widget
              }
              else
              {
                const int defaultValue = getAttributeInt(value, "default", -1, lncvValueMin, lncvValueMax);

                QDomElement option = value.firstChildElement("option");
                if(!option.isNull())
                {
                  auto* cb = new QComboBox(this);

                  while(!option.isNull())
                  {
                    const int v = getAttributeInt(option, "value", -1, lncvValueMin, lncvValueMax);
                    if(v != -1)
                    {
                      auto name = getElementByLanguage(option, "name");
                      if(!name.isNull())
                        cb->addItem(name.text(), v);
                      else
                        cb->addItem(QString::number(v), v);

                      if(defaultValue == v)
                        cb->setCurrentIndex(cb->count() - 1);
                    }
                    option = option.nextSiblingElement("option");
                  }
                  w = cb;
                }
                else
                {
                  const int min = getAttributeInt(value, "min", lncvValueMin, lncvValueMin, lncvValueMax);
                  const int max = getAttributeInt(value, "max", lncvValueMax, lncvValueMin, lncvValueMax);
                  double gain = getAttributeFloat(value, "gain");
                  double offset = getAttributeFloat(value, "offset");
                  double dummy;
                  if((std::isfinite(gain) && std::modf(gain, &dummy) != 0) ||
                      (std::isfinite(offset) && std::modf(offset, &dummy) != 0))
                  {
                    if(std::isnan(gain))
                      gain = 1;
                    if(std::isnan(offset))
                      offset = 0;

                    auto* sb = new QDoubleSpinBox(this);

                    sb->setProperty("lncv_gain", gain);
                    sb->setProperty("lncv_offset", offset);

                    sb->setSingleStep(gain);
                    sb->setDecimals(-std::floor(std::log10(std::abs(gain))));

                    if(max >= min)
                      sb->setRange(min * gain + offset, max * gain + offset);
                    else
                      sb->setRange(lncvValueMin * gain + offset, lncvValueMax * gain + offset);

                    if(auto unit = value.attribute("unit", ""); !unit.isEmpty())
                      sb->setSuffix(unit.prepend(" "));

                    if(defaultValue != -1)
                      sb->setValue(defaultValue * gain + offset);

                    w = sb;
                  }
                  else
                  {
                    auto* spinBox = new QSpinBox(this);

                    if(max >= min)
                      spinBox->setRange(min, max);
                    else
                      spinBox->setRange(lncvValueMin, lncvValueMax);

                    if(auto unit = value.attribute("unit", ""); !unit.isEmpty())
                      spinBox->setSuffix(unit.prepend(" "));

                    if(defaultValue != -1)
                      spinBox->setValue(defaultValue);

                    w = spinBox;
                  }
                }
              }
            }
            else
            {
              auto* sb = new QSpinBox(this);
              sb->setRange(lncvValueMin, lncvValueMax);
              w = sb;
            }

            if(w)
            {
              m_lncvs->setCellWidget(row, columnValue, w);
              w->setEnabled(false);
            }

            m_lncvToRow[number] = row;
          }
          lncv = lncv.nextSiblingElement("lncv");
        }
      }
    }
  }
}

void LNCVProgrammer::moduleChanged()
{
  m_otherModule->setVisible(m_module->currentIndex() == m_module->count() - 1);
  updateStartEnabled();
}

void LNCVProgrammer::useBroadcastAddressChanged()
{
  if(m_broadcastAddress->isChecked())
  {
    m_lastAddress = m_address->value();
    m_address->setValue(broadcastAddress);
    m_address->setEnabled(false);
  }
  else
  {
    m_address->setValue(m_lastAddress);
    m_address->setEnabled(true);
  }
}

void LNCVProgrammer::updateStartEnabled()
{
  m_start->setEnabled(
    m_interface->currentIndex() != 0 && m_interface->count() > 1 &&
    m_module->currentIndex() != 0);
}

void LNCVProgrammer::updateReadWriteEnabled()
{
  const bool b = (getSelectedLNCV() != -1) && (m_state == State::Idle);
  m_read->setEnabled(b);
  m_write->setEnabled(b && getSelectedLNCVValue() != -1);
}

void LNCVProgrammer::reset()
{
  m_pages->widget(pageStart)->setEnabled(true);
  m_pages->setCurrentIndex(pageStart);
  m_object.reset();
}

int LNCVProgrammer::getSelectedLNCV() const
{
  if(const auto* item = m_lncvs->item(m_lncvs->currentRow(), columnLNCV))
  {
    bool ok;
    if(int lncv = item->text().toInt(&ok); ok && lncv >= lncvMin && lncv <= lncvMax)
      return lncv;
  }
  return -1;
}

int LNCVProgrammer::getSelectedLNCVValue() const
{
  if(const auto* widget = m_lncvs->cellWidget(m_lncvs->currentRow(), columnValue))
  {
    if(!widget->isEnabled())
      return -1;

    if(const auto* spinBox = dynamic_cast<const QSpinBox*>(widget))
    {
      return spinBox->value();
    }
    if(const auto* comboBox = dynamic_cast<const QComboBox*>(widget))
    {
      int x = comboBox->currentData().toInt();
      return x;
    }
    if(const auto* doubleSpinBox = dynamic_cast<const QDoubleSpinBox*>(widget))
    {
      const double gain = doubleSpinBox->property("lncv_gain").toDouble();
      const double offset = doubleSpinBox->property("lncv_offset").toDouble();
      return std::round((doubleSpinBox->value() - offset) / gain);
    }
  }
  else if(const auto* item = m_lncvs->item(m_lncvs->currentRow(), columnValue))
  {
    bool ok;
    if(int lncv = item->text().toInt(&ok); ok && lncv >= lncvMin && lncv <= lncvMax)
      return lncv;
  }
  return -1;
}

void LNCVProgrammer::setState(State value)
{
  m_state = value;
  updateReadWriteEnabled();
}
