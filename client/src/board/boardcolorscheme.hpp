#ifndef ASD
#define ASD

#include <QColor>

struct BoardColorScheme
{
  static const BoardColorScheme dark;
  static const BoardColorScheme light;

  const QColor background;
  const QColor track;
  const QColor trackDisabled;
  const QColor blockFree;
  const QColor blockOccupied;
  const QColor blockUnknown;
  const QColor sensorFree;
  const QColor sensorOccupied;
  const QColor sensorIdle;
  const QColor sensorTriggered;
  const QColor sensorUnknown;
  const QColor turnoutState;
};

#endif
