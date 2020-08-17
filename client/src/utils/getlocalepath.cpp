#include "getlocalepath.hpp"
#include <QProcessEnvironment>
#ifdef Q_OS_WINDOWS
  #include <QStandardPaths>
#elif defined(Q_OS_LINUX)
#else
  #include <QDir>
#endif

#define TRAINTASTIC_LOCALE_PATH QStringLiteral("TRAINTASTIC_LOCALE_PATH")

const QString& getLocalePath()
{
  static QString path;
  if(path.isEmpty())
  {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if(env.contains(TRAINTASTIC_LOCALE_PATH))
      path = env.value(TRAINTASTIC_LOCALE_PATH);
    else
    {
#ifdef Q_OS_WINDOWS
      path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "traintastic\\client\\lang", QStandardPaths::LocateDirectory);
#elif defined(Q_OS_LINUX)
      path = "/opt/traintastic/lang";
#else
      path = QDir::currentPath() + "/lang";
#endif
    }
  }
  return path;
}
