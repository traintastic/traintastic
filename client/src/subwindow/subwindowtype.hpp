#ifndef SWT
#define SWT

#include <QString>

enum class SubWindowType
{
  Object,
  Board,
  Throttle,
};

inline QString toString(SubWindowType value)
{
  switch(value)
  {
    case SubWindowType::Object:
      return QStringLiteral("object");

    case SubWindowType::Board:
      return QStringLiteral("board");

    case SubWindowType::Throttle:
      return QStringLiteral("throttle");
  }
  Q_ASSERT(false);
  return QString();
}

#endif
