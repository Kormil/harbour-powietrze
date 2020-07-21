#include "airqualityindexmodel.h"

AirQualityIndexModel::AirQualityIndexModel(QObject *parent) :
    QAbstractListModel(parent)
{
    m_airQualityIndexList.push_back(std::make_shared<AirIndexFromServer>());
    m_airQualityIndexList.push_back(std::make_shared<EuropeanAQ>());
}

int AirQualityIndexModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_airQualityIndexList.size();
}

QVariant AirQualityIndexModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || m_airQualityIndexList.size() == 0)
        return QVariant();

    switch (role)
    {
    case Qt::UserRole: {
        return QVariant(m_airQualityIndexList[index.row()]->id());
    }
    case Qt::DisplayRole: {
        return QVariant(m_airQualityIndexList[index.row()]->name());
    }
    }

    return QVariant();
}

QHash<int, QByteArray> AirQualityIndexModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[Qt::UserRole] = "id";
    names[Qt::DisplayRole] = "name";
    return names;
}

AirQualityIndexPtr AirQualityIndexModel::index(int id)
{
    return m_airQualityIndexList[id];
}

void AirQualityIndexModel::setModelsManager(ModelsManager *modelsManager)
{
    m_modelsManager = modelsManager;
    AirQualityIndex::setModelsManager(modelsManager);
}
