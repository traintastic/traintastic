#ifndef TRAINSMODEL_H
#define TRAINSMODEL_H

#include <QAbstractTableModel>

#include <QMap>

#include <traintastic/enum/color.hpp>

class Simulator;

class TrainsModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns
    {
        Name = 0,
        State,
        NCols
    };

    explicit TrainsModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &p = QModelIndex()) const override;
    int columnCount(const QModelIndex &p = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    bool addTrain(const QString& name, Color c, size_t numWagons, size_t segmentIdx,
                  QString *errOut);

    bool removeTrain(const QString& name);

    void setSimulator(Simulator *sim);

private:
    Simulator *mSimulator = nullptr;

    struct Train
    {
        QString name;
        Color color;
        double speedKmH;
    };

    QMap<QString, Train> mTrains;
};

#endif // TRAINSMODEL_H
