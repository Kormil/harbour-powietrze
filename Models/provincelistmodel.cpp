#include "provincelistmodel.h"
#include "stationlistmodel.h"
#include "src/modelsmanager.h"
#include "src/connection.h"

#include <QtQml>

ProvinceListModel::ProvinceListModel(QObject *parent)
    : QAbstractListModel(parent),
      m_provinceList(nullptr),
      m_stationListModel(nullptr)
{
}

int ProvinceListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || m_provinceList == nullptr)
        return 0;

    return m_provinceList->size();
}

QVariant ProvinceListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || m_provinceList == nullptr)
        return QVariant();

    const ProvinceItem* province = m_provinceList->provinceItems().at(index.row()).get();
    switch (role)
    {
    case Qt::DisplayRole:
    {
        QString name = province->name.toLower();
        name[0] = name[0].toUpper();
        return QVariant(name);
    }
    }

    return QVariant();
}

QHash<int, QByteArray> ProvinceListModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[Qt::DisplayRole] = "name";
    return names;
}

void ProvinceListModel::setModels(ModelsManager *modelsManager)
{
    setStationListModel(modelsManager->stationListModel());
}

void ProvinceListModel::setProvinceList(ProvinceListPtr provinceList)
{
    beginResetModel();

    if (m_provinceList)
        m_provinceList->disconnect(this);

    m_provinceList = std::move(provinceList);

    if (m_provinceList)
    {
        connect(m_provinceList.get(), &ProvinceList::preItemAppended, this, [this]() {
            const int index = m_provinceList->size();
            beginInsertRows(QModelIndex(), index, index);
        });

        connect(m_provinceList.get(), &ProvinceList::postItemAppended, this, [this]() {
            endInsertRows();
        });
    }

    endResetModel();
}

void ProvinceListModel::setStationListModel(StationListModel *value)
{
    m_stationListModel = value;
    connect(m_stationListModel, &StationListModel::stationListLoaded, this, &ProvinceListModel::onStationListModelLoaded);
}

void ProvinceListModel::onStationListModelLoaded()
{
    ProvinceListPtr provinceList(new ProvinceList());
    std::vector<QString> provinceNames = m_stationListModel->provinceNames();

    for (const auto& name: provinceNames)
    {
        ProvinceItem item;
        item.name = name;
        provinceList->append( item );
    }

    setProvinceList(std::move(provinceList));
    emit provinceLoaded();
}

void ProvinceListModel::onItemClicked(int index)
{
    if (index != -1)
    {
        m_selectedItem = m_provinceList->get(index);
    }
    else
    {
        m_selectedItem = nullptr;
    }

    emit selectedProvinceNameChanged();
}

ProvinceItem *ProvinceListModel::selectedItem() const
{
    return m_selectedItem;
}

QString ProvinceListModel::selectedProvinceName() const
{
    if (m_selectedItem)
        return m_selectedItem->name;

    return QStringLiteral("");
}
