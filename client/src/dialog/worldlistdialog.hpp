#ifndef WORLDLISTWIDGET_HPP
#define WORLDLISTWIDGET_HPP

#include <QDialog>
#include "../network/objectptr.hpp"

class QDialogButtonBox;
class TableWidget;
class Connection;

class WorldListDialog final : public QDialog
{
  Q_OBJECT

  protected:
    QSharedPointer<Connection> m_connection;
    int m_requestId;
    ObjectPtr m_object;
    QDialogButtonBox* m_buttons; // TODO: m_buttonLoad;
    TableWidget* m_tableWidget;
    QString m_uuid;

  public:
    explicit WorldListDialog(const QSharedPointer<Connection>& connection, QWidget* parent = nullptr);
    ~WorldListDialog() final;

    QString uuid() const { return m_uuid; }
};

#endif
