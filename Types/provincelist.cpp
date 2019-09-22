#include "provincelist.h"
#include "stationlist.h"
#include <algorithm>

ProvinceList::ProvinceList(QObject *parent) : QObject(parent)
{
}

ProvinceList::~ProvinceList()
{
}

size_t ProvinceList::size() const
{
    return m_provinceItems.size();
}

ProvinceItemPtr ProvinceList::get(int index)
{
    return m_provinceItems[index];
}

void ProvinceList::append(const ProvinceItemPtr &item)
{
    auto hash = item->hash();
    auto provinceIt = m_hashToRow.find(hash);

    if (provinceIt == m_hashToRow.end())
    {
        emit preItemAppended();
        int row = m_provinceItems.size();
        m_provinceItems.push_back(item);
        m_hashToRow[hash] = row;
        emit postItemAppended();
    }
}

void ProvinceList::appendList(ProvinceListPtr &provinceList)
{
    for (auto& province: provinceList->m_provinceItems)
    {
        append(province);
    }

    provinceList = nullptr;
}
