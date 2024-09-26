#ifndef TRAINTASTIC_CLIENT_WIDGET_VEHICLE_SPEED_CURVE_WIDGET_HPP
#define TRAINTASTIC_CLIENT_WIDGET_VEHICLE_SPEED_CURVE_WIDGET_HPP

#include <QWidget>
#include "../network/objectptr.hpp"

class Method;
class QPushButton;
class QLabel;

class VehicleSpeedCurveWidget : public QWidget
{
  Q_OBJECT

  private:
    ObjectPtr m_object;
    Method *m_importMethod;
    Method *m_exportMethod;

    QPushButton *m_importBut;
    QPushButton *m_exportBut;
    QLabel *m_invalidCurveLabel;

  private slots:
    void importFromFile();
    void exportToFile();

private:
    void setCurveValid(bool valid);

  public:
    explicit VehicleSpeedCurveWidget(ObjectPtr object, QWidget* parent = nullptr);
};

#endif // TRAINTASTIC_CLIENT_WIDGET_VEHICLE_SPEED_CURVE_WIDGET_HPP
