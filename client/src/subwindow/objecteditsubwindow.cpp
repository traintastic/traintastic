#include "objecteditsubwindow.hpp"
#include "../widget/objecteditwidget.hpp"

ObjectEditSubWindow::ObjectEditSubWindow(const QString& id, QWidget* parent) :
  QMdiSubWindow(parent)
{
  setWindowTitle(id);
  setWidget(new ObjectEditWidget(id, this));
}
