#include "trainsmodel.h"

#include <traintastic/simulator/simulator.hpp>

TrainsModel::TrainsModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

QVariant TrainsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case Columns::Name:
            return tr("Name");
        case Columns::State:
            return tr("State");
        default:
            break;
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

int TrainsModel::rowCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : mTrains.size();
}

int TrainsModel::columnCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : NCols;
}

QVariant TrainsModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() || idx.column() >= NCols || idx.row() >= mTrains.size())
        return QVariant();

    auto it = mTrains.begin();
    std::advance(it, idx.row());
    const Train& train = *it;

    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (idx.column())
        {
        case Name:
            return train.name;
        case State:
        {
            if(train.speedKmH == 0)
                return tr("Stopped");
            return tr("%1 km/h").arg(train.speedKmH);
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }

    return QVariant();
}

bool TrainsModel::addTrain(const QString &name, Color c, size_t numWagons, size_t segmentIdx, QString *errOut)
{
    if(!mSimulator)
        return false;

    std::lock_guard<std::recursive_mutex> lock(mSimulator->stateMutex());

    if(mSimulator->trainExists(name.toStdString()))
    {
        *errOut = tr("Name already in use!");
        return false;
    }

    if(segmentIdx >= mSimulator->staticData.trackSegments.size())
    {
        *errOut = tr("Segment does not exist!");
        return false;
    }

    if(mSimulator->segmentOccupied(segmentIdx))
    {
        *errOut = tr("Segment already occupied");
        return false;
    }

    const auto& segment = mSimulator->staticData.trackSegments.at(segmentIdx);
    if(segment.type != Simulator::TrackSegment::Type::Straight &&
            segment.type != Simulator::TrackSegment::Type::Curve)
    {
        *errOut = tr("Segment is not straight or curve");
        return false;
    }

    std::string baseName = name.toStdString() + ".";

    std::vector<Simulator::Train::VehicleItem> vehicles;
    for(size_t i = 0; i < numWagons; i++)
    {
        Simulator::Train::VehicleItem item;
        item.vehicle = mSimulator->addVehicle(baseName + std::to_string(i), 20.0f, c);
        item.reversed = false;
        vehicles.push_back(item);
    }

    if(!mSimulator->addTrain(name.toStdString(), DecoderProtocol::DCCShort, 3,
                             vehicles, segmentIdx))
    {
        for(const auto &item : vehicles)
        {
            mSimulator->removeVehicle(item.vehicle);
        }

        *errOut = tr("Train is too long");
        return false;
    }

    beginResetModel();
    mTrains.insert(name, Train{name, c, 0.0});
    endResetModel();
    return true;
}

bool TrainsModel::removeTrain(const QString &name)
{
    if(!mSimulator || !mTrains.contains(name))
        return false;

    if(!mSimulator->removeTrain(name.toStdString(), true))
        return false;

    beginResetModel();
    mTrains.remove(name);
    endResetModel();
    return true;
}

void TrainsModel::setSimulator(Simulator *sim)
{
    beginResetModel();
    mTrains.clear();

    mSimulator = sim;

    if(mSimulator)
    {
        for(const auto& it : mSimulator->stateData().trains)
        {
            Train train;
            train.name = QString::fromStdString(it.second->name);
            train.speedKmH = it.second->state.speed;
            if(it.second->state.reverse)
                train.speedKmH = -train.speedKmH;

            train.color = it.second->vehicles.front().vehicle->color;

            mTrains.insert(train.name, train);
        }
    }

    endResetModel();
}
