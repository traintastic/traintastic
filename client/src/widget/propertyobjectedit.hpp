#ifndef PROPERTYOBJECTEDIT_HPP
#define PROPERTYOBJECTEDIT_HPP

#include <QWidget>

class ObjectProperty;
class QLineEdit;
class QToolButton;

class PropertyObjectEdit : public QWidget
{
  Q_OBJECT

  protected:
    ObjectProperty& m_property;
    QLineEdit* m_lineEdit;
    QToolButton* m_changeButton;

  public:
    explicit PropertyObjectEdit(ObjectProperty& property, QWidget* parent = nullptr);
};

#endif
