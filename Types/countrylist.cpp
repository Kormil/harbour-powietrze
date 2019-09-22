#include "countrylist.h"
#include <algorithm>

CountryList::CountryList(QObject *parent) : QObject(parent)
{

}


CountryList::~CountryList()
{
}

std::size_t CountryList::size() const
{
    return m_countryItems.size();
}

CountryItemPtr CountryList::get(int index)
{
    return m_countryItems[index];
}

void CountryList::append(const CountryItemPtr &country)
{
    if (country == nullptr)
        return;

    auto hash = country->hash();
    auto countryIt = m_hashToRow.find(hash);

    if (m_hashToRow.end() == countryIt)
    {
        emit preItemAppended();
        int row = m_countryItems.size();
        m_countryItems.push_back(country);
        m_hashToRow[hash] = row;
        emit postItemAppended();
    }
}

void CountryList::appendList(CountryListPtr &countryList)
{
    for (auto& country: countryList->m_countryItems)
    {
        append(country);
    }

    countryList = nullptr;
}
