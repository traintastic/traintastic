/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_WIDGET_DECODER_DECODERFUNCTIONSWIDGET_HPP
#define TRAINTASTIC_CLIENT_WIDGET_DECODER_DECODERFUNCTIONSWIDGET_HPP

#include <QWidget>
#include "../../network/objectptr.hpp"

class QToolBar;
class QTableView;
class ObjectProperty;
class MethodAction;

class DecoderFunctionsWidget : public QWidget
{
private:
  ObjectProperty& m_decoderProperty;
  ObjectPtr m_decoder;
  QToolBar* m_toolbar;
  MethodAction* m_delete = nullptr;
  MethodAction* m_moveUp = nullptr;
  MethodAction* m_moveDown = nullptr;
  QTableView* m_table;
  int m_requestId;

  const ObjectPtr& getSelectedFunction() const;

  void cancelRequest();
  void decoderChanged();

public:
  explicit DecoderFunctionsWidget(ObjectProperty& decoderProperty, QWidget* parent = nullptr);
  ~DecoderFunctionsWidget() override;
};

#endif