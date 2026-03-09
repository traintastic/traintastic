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

#include "tileimagewidget.hpp"
#include <QPainter>
#include "../../board/boardcolorscheme.hpp"
#include "../../board/getboardcolorscheme.hpp"
#include "../../board/tilepainter.hpp"
#include "../../network/abstractproperty.hpp"
#include "../../network/object.hpp"
#include "../../network/object.tpp"
#include "../../settings/boardsettings.hpp"

namespace {

std::initializer_list<const QString> monitorProperties{
    QStringLiteral("aspect"),
    QStringLiteral("position"),
    QStringLiteral("color"),
    QStringLiteral("state"),
    QStringLiteral("text"),
    QStringLiteral("text_color"),
    QStringLiteral("text_align"),
    QStringLiteral("background_color"),
    QStringLiteral("color_on"),
    QStringLiteral("text_color_on"),
    QStringLiteral("color_off"),
    QStringLiteral("text_color_off")
  };

QSize getRatio(TileId m_tileId)
{
  switch(m_tileId)
  {
    case TileId::RailBlock:
      return {2, 1};

    default:
      break;
  }
  return {1, 1};
}

TileRotate getRotate(TileId m_tileId)
{
  switch(m_tileId)
  {
    case TileId::RailBlock:
    case TileId::RailDirectionControl:
    case TileId::RailDecoupler:
      return TileRotate::Deg90;

    default:
      break;
  }
  return TileRotate::Deg0;
}

}

TileImageWidget::TileImageWidget(ObjectPtr object, QWidget* parent)
  : QWidget(parent)
  , m_colorScheme{getBoardColorScheme(BoardSettings::instance().colorScheme.value())}
  , m_object{std::move(object)}
  , m_tileId{m_object->getPropertyValueEnum<TileId>(QStringLiteral("tile_id"), TileId::None)}
{
  connect(&BoardSettings::instance(), &BoardSettings::changed, this, qOverload<>(&TileImageWidget::update));

  for(const auto& name : monitorProperties)
  {
    if(auto* property = m_object->getProperty(name))
    {
      connect(property, &AbstractProperty::valueChanged, this, qOverload<>(&TileImageWidget::update));
    }
  }
}

int TileImageWidget::heightForWidth(int w) const
{
  const auto r = getRatio(m_tileId);
  return (w * r.height()) / r.width();
}

bool TileImageWidget::hasHeightForWidth() const
{
  return true;
}

QSize TileImageWidget::minimumSizeHint() const
{
  return getRatio(m_tileId) * 64;
}

void TileImageWidget::paintEvent(QPaintEvent* /*event*/)
{
  const int margin = 1;

  auto tileRect = rect();
  const auto ratio = getRatio(m_tileId);
  const auto sz = std::min(tileRect.width() / ratio.width(), tileRect.height() / ratio.height());
  tileRect.setWidth(sz * ratio.width());
  tileRect.setHeight(sz * ratio.height());

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.fillRect(tileRect, m_colorScheme->background);

  if(m_tileId != TileId::None) [[likely]]
  {
    tileRect.adjust(margin, margin, -margin, -margin);
    painter.setClipRect(tileRect);

    TilePainter tilePainter(painter, tileRect.height(), *m_colorScheme);
    const auto a = getRotate(m_tileId);
    switch(m_tileId)
    {
      case TileId::RailStraight:
      case TileId::RailCurve45:
      case TileId::RailCurve90:
      case TileId::RailCross45:
      case TileId::RailCross90:
      case TileId::RailBufferStop:
      case TileId::RailBridge45Left:
      case TileId::RailBridge45Right:
      case TileId::RailBridge90:
      case TileId::RailTunnel:
      case TileId::RailOneWay:
      case TileId::RailLink:
        tilePainter.draw(m_tileId, tileRect, a);
        break;

      case TileId::RailTurnoutLeft45:
      case TileId::RailTurnoutLeft90:
      case TileId::RailTurnoutLeftCurved:
      case TileId::RailTurnoutRight45:
      case TileId::RailTurnoutRight90:
      case TileId::RailTurnoutRightCurved:
      case TileId::RailTurnoutWye:
      case TileId::RailTurnout3Way:
      case TileId::RailTurnoutSingleSlip:
      case TileId::RailTurnoutDoubleSlip:
        tilePainter.drawTurnout(m_tileId, tileRect, a, TurnoutPosition::Unknown, m_object->getPropertyValueEnum<TurnoutPosition>("position", TurnoutPosition::Unknown));
        break;

      case TileId::RailSensor:
        tilePainter.drawSensor(m_tileId, tileRect, a, false, m_object->getPropertyValueEnum<SensorState>("state", SensorState::Unknown));
        break;

      case TileId::RailSignal2Aspect:
      case TileId::RailSignal3Aspect:
        tilePainter.drawSignal(m_tileId, tileRect, a, false, m_object->getPropertyValueEnum<SignalAspect>("aspect", SignalAspect::Unknown));
        break;

      case TileId::RailBlock:
        tilePainter.drawBlock(m_tileId, tileRect, a, false, false, m_object);
        break;

      case TileId::RailDirectionControl:
        tilePainter.drawDirectionControl(m_tileId, tileRect, a, false, m_object->getPropertyValueEnum<DirectionControlState>("state", DirectionControlState::Both));
        break;

      case TileId::PushButton:
        tilePainter.drawPushButton(tileRect,
          m_object->getPropertyValueEnum<Color>("color", Color::Yellow),
          m_object->getPropertyValueEnum<Color>("text_color", Color::Black),
          m_object->getPropertyValueString("text"));
        break;

      case TileId::RailDecoupler:
        tilePainter.drawRailDecoupler(tileRect, a, false, m_object->getPropertyValueEnum<DecouplerState>("state", DecouplerState::Deactivated));
        break;

      case TileId::RailNXButton:
        tilePainter.drawRailNX(tileRect, a, false,
          m_object->getPropertyValueBool("enabled", false),
          m_object->getPropertyValueBool("pressed", false));
        break;

      case TileId::Label:
        tilePainter.drawLabel(tileRect, a,
          m_object->getPropertyValueString("text"),
          m_object->getPropertyValueEnum<TextAlign>("text_align", TextAlign::Center),
          m_object->getPropertyValueEnum<Color>("text_color", Color::None),
          m_object->getPropertyValueEnum<Color>("background_color", Color::None));
        break;

      case TileId::Switch:
        if(m_object->getPropertyValueBool("value", false)) // on
        {
          tilePainter.drawSwitch(tileRect,
            m_object->getPropertyValueEnum<Color>("color_on", Color::Yellow),
            m_object->getPropertyValueEnum<Color>("text_color_on", Color::Black),
            m_object->getPropertyValueString("text"));
        }
        else // off
        {
          tilePainter.drawSwitch(tileRect,
            m_object->getPropertyValueEnum<Color>("color_off", Color::Gray),
            m_object->getPropertyValueEnum<Color>("text_color_off", Color::White),
            m_object->getPropertyValueString("text"));
        }
        break;

      case TileId::None:
      case TileId::ReservedForFutureExpension:
      default:
        assert(false);
        break;
    }
  }
}
