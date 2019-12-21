#ifndef OBJECTEDITSUBWINDOW_HPP
#define OBJECTEDITSUBWINDOW_HPP

#include <QMdiSubWindow>

class ObjectEditSubWindow : public QMdiSubWindow
{
  public:
    ObjectEditSubWindow(const QString& id, QWidget* parent = nullptr);
};

#endif
