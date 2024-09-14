#include "vehiclespeedcurvewidget.hpp"

#include "../network/method.hpp"
#include "../network/error.hpp"
#include "../network/callmethod.hpp"
#include <traintastic/locale/locale.hpp>

#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>

#include <QSettings>
#include <QStandardPaths>

#include <string>

VehicleSpeedCurveWidget::VehicleSpeedCurveWidget(ObjectPtr object,
                                                 QWidget *parent)
  : QWidget(parent)
  , m_object(object)
{
    m_importMethod = m_object->getMethod("import_from_string");
    m_exportMethod = m_object->getMethod("export_to_string");

    QHBoxLayout *lay = new QHBoxLayout(this);

    m_importBut = new QPushButton(Locale::tr("qtapp:import_speed_curve"));
    lay->addWidget(m_importBut);

    m_exportBut = new QPushButton(Locale::tr("qtapp:export_speed_curve"));
    lay->addWidget(m_exportBut);

    m_invalidCurveLabel = new QLabel(Locale::tr("qtapp:invalid_speed_curve"));
    lay->addWidget(m_invalidCurveLabel);

    if(m_importMethod)
    {
        m_importBut->setEnabled(m_importMethod->getAttributeBool(AttributeName::Enabled, true));
        m_importBut->setVisible(m_importMethod->getAttributeBool(AttributeName::Visible, true));
        connect(m_importMethod, &Method::attributeChanged, this,
          [this](AttributeName name, const QVariant& value)
          {
            switch(name)
            {
              case AttributeName::Enabled:
                m_importBut->setEnabled(value.toBool());
                break;

              case AttributeName::Visible:
                m_importBut->setVisible(value.toBool());
                break;

              default:
                break;
            }
          });

        connect(m_importBut, &QPushButton::clicked,
                this, &VehicleSpeedCurveWidget::importFromFile);
    }

    if(m_exportMethod)
    {
        m_exportBut->setEnabled(m_exportMethod->getAttributeBool(AttributeName::Enabled, true));
        setCurveValid(m_exportMethod->getAttributeBool(AttributeName::Visible, true));
        connect(m_exportMethod, &Method::attributeChanged, this,
          [this](AttributeName name, const QVariant& value)
          {
            switch(name)
            {
              case AttributeName::Enabled:
                m_exportBut->setEnabled(value.toBool());
                break;

              case AttributeName::Visible:
                setCurveValid(value.toBool());
                break;

              default:
                break;
            }
          });

        connect(m_exportBut, &QPushButton::clicked,
                this, &VehicleSpeedCurveWidget::exportToFile);
    }
}

void VehicleSpeedCurveWidget::importFromFile()
{
  QSettings settings;
  settings.beginGroup("speed_curves");
  const QString pathKey{"path"};

  const QString filename = QFileDialog::getOpenFileName(
      this,
      Locale::tr("qtapp:import_speed_curve"),
      settings.value(pathKey, QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first()).toString(),
      Locale::tr("qtapp:traintastic_json_speed_curve").append(" (*.json)"));

  if(!filename.isEmpty())
  {
    settings.setValue(pathKey, QFileInfo(filename).absolutePath());

    QFile file(filename);
    if(file.open(QIODevice::ReadOnly))
    {
      callMethod(*m_importMethod,
        [this](std::optional<const Error> error)
        {
          if(error)
            error->show(this);
        },
        QString::fromLocal8Bit(file.readAll()));
    }
    else
    {
      QMessageBox::critical(
        this,
        Locale::tr("qtapp:import_speed_curve_failed"),
        Locale::tr("qtapp.error:cant_read_from_file_x").arg(filename));
    }
  }
}

void VehicleSpeedCurveWidget::exportToFile()
{
  QSettings settings;
  settings.beginGroup("speed_curves");
  const QString pathKey{"path"};

  const QString filename = QFileDialog::getSaveFileName(
    this,
    Locale::tr("qtapp:import_speed_curve"),
    settings.value(pathKey, QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first()).toString(),
    Locale::tr("qtapp:traintastic_json_speed_curve").append(" (*.json)"));

  if(!filename.isEmpty())
  {
    settings.setValue(pathKey, QFileInfo(filename).absolutePath());

    callMethodR<QString>(*m_exportMethod,
      [this, filename](const QString& str, std::optional<const Error> error)
      {
        if(error)
        {
          error->show(this);
          return;
        }

        bool success = false;
        QFile file(filename);
        if(file.open(QIODevice::WriteOnly))
        {
          qint64 n = file.write(str.toUtf8());
          if(n == str.size())
            success = true;
        }

        if(!success)
        {
          QMessageBox::critical(
            this,
            Locale::tr("qtapp:export_speed_curve_failed"),
            Locale::tr("qtapp.error:cant_write_to_file_x").arg(filename));
        }
      });
  }
}

void VehicleSpeedCurveWidget::setCurveValid(bool valid)
{
  m_exportBut->setVisible(valid);
  m_invalidCurveLabel->setVisible(!valid);
}
