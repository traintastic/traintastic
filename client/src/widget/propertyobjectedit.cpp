#include "propertyobjectedit.hpp"
#include "../network/objectproperty.hpp"
#include "../mainwindow.hpp"
#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>

PropertyObjectEdit::PropertyObjectEdit(ObjectProperty& property, QWidget *parent) :
  QWidget(parent),
  m_property{property},
  m_lineEdit{new QLineEdit(m_property.objectId(), this)},
  m_changeButton{new QToolButton(this)}
{
  bool enabled = m_property.getAttributeBool(AttributeName::Enabled, true);
  bool visible = m_property.getAttributeBool(AttributeName::Visible, true);
  connect(&m_property, &ObjectProperty::attributeChanged,
    [this](AttributeName name, const QVariant& value)
    {
      switch(name)
      {
        case AttributeName::Enabled:
          m_lineEdit->setEnabled(value.toBool());
          m_changeButton->setEnabled(value.toBool());
          break;

        case AttributeName::Visible:
          m_lineEdit->setVisible(value.toBool());
          m_changeButton->setVisible(value.toBool());
          break;

        default:
          break;
      }
    });

  QHBoxLayout* l = new QHBoxLayout();
  l->setMargin(0);

  m_lineEdit->setEnabled(enabled);
  m_lineEdit->setVisible(visible);
  m_lineEdit->setReadOnly(true);
  l->addWidget(m_lineEdit, 1);

  m_changeButton->setEnabled(enabled);
  m_changeButton->setVisible(visible);
  m_changeButton->setText("...");
  l->addWidget(m_changeButton);

  QToolButton* edit = new QToolButton(this);
  edit->setIcon(QIcon(":/dark/edit.svg"));
  edit->setEnabled(!m_property.objectId().isEmpty());
  connect(edit, &QToolButton::clicked,
    [this]()
    {
      if(!m_property.objectId().isEmpty())
        MainWindow::instance->showObject(m_property.objectId());
    });
  l->addWidget(edit);

  setLayout(l);
}
