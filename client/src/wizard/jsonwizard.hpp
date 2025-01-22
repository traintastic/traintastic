/**
 * client/src/wizard/jsonwizard.hpp
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

#ifndef TRAINTASTIC_CLIENT_WIZARD_JSONWIZARD_HPP
#define TRAINTASTIC_CLIENT_WIZARD_JSONWIZARD_HPP

#include "wizard.hpp"
#include <map>
#include <QJsonObject>
#include <QFuture>
#include "../network/objectptr.hpp"
#include "../network/create/properties.hpp"

class CreateInterface;

class JSONWizard : public Wizard
{
  private:
    struct PageInfo
    {
      int id;
      QJsonObject data;
    };

    ObjectPtr m_world;
    std::map<QString, QFuture<ObjectPtr>> m_objects;
    std::map<QString, QVariant> m_variables;
    std::map<QString, PageInfo> m_pages;
    QVector<int> m_ids;
    std::shared_ptr<CreateInterface> m_createInterface;

    int createPage(PageInfo& pageInfo);

    void doActions(const QJsonObject& actions);
    void undoActions(const QJsonObject& actions);

    Properties toProperties(const QJsonObject& object);

  protected:
    void initializePage(int id) override;
    void cleanupPage(int id) override;

  public:
    static constexpr int errorPage = 0;

    JSONWizard(const QString& filename, ObjectPtr world, QWidget* parent = nullptr);
    ~JSONWizard() override;

    int nextId() const override
    {
      if(currentId() == errorPage)
      {
        return -1;
      }
      return Wizard::nextId();
    }

    QString translateAndReplaceVariables(const QString& text) const;
    QFuture<ObjectPtr> getObject(const QString& reference) const;

    int getPageId(const QString& page);
};

#endif
