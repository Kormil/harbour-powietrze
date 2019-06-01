#include "provincelist.h"
#include "stationlist.h"
#include <algorithm>

ProvinceList::ProvinceList(QObject *parent) : QObject(parent)
{
}

ProvinceList::~ProvinceList()
{
}

bool ProvinceList::setItemAt(unsigned int index, const ProvinceItem &provinceItem)
{
    if (index > m_provinceItems.size())
        return false;

    m_provinceItems[index] = ProvinceItemPtr( new ProvinceItem(provinceItem) );
    return true;
}

size_t ProvinceList::size() const
{
    return m_provinceItems.size();
}

const std::vector<ProvinceItemPtr>& ProvinceList::provinceItems() const
{
    return m_provinceItems;
}

ProvinceItem *ProvinceList::get(unsigned int index) const
{
    if (index >= m_provinceItems.size())
        return nullptr;

    return m_provinceItems[index].get();
}

ProvinceListPtr ProvinceList::getFromJson(const QJsonDocument &jsonDocument)
{
    ProvinceListPtr provinceList(new ProvinceList());

    QJsonArray array = jsonDocument.array();

    for (const auto& station: array)
    {
        ProvinceItem item;
        item.name = station.toObject()["city"].toObject()["commune"].toObject()["provinceName"].toString();

        bool exist = false;
        for (const auto& province: provinceList->m_provinceItems)
        {
            if (item.name == province->name)
            {
                exist = true;
                break;
            }
        }

        if (!exist)
            provinceList->append(item);
    }

    std::sort(provinceList->m_provinceItems.begin(), provinceList->m_provinceItems.end(),
              [](const ProvinceItemPtr& a, const ProvinceItemPtr& b) {
                 return a->name < b->name;
              });

    return std::move(provinceList);
}

void ProvinceList::append(const ProvinceItem &item)
{
    emit preItemAppended();

    m_provinceItems.push_back( ProvinceItemPtr(new ProvinceItem(item)) );

    emit postItemAppended();
}
