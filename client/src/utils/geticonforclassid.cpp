#include "geticonforclassid.hpp"

QIcon getIconForClassId(const QString& classId)
{
  if(classId == "lua.script" || classId == "lua.script_list")
    return QIcon(":/dark/lua.svg");
  else
    return QIcon();
}
