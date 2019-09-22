#ifndef COUNTRYLIST_H
#define COUNTRYLIST_H

#include <QObject>
#include <QStringBuilder>
#include <memory>
#include <map>

class CountryList;
class CountryItem;

using CountryItemPtr = std::shared_ptr<CountryItem>;
using CountryListPtr = std::shared_ptr<CountryList>;

struct CountryItem
{
    QString name;
    QString code;
    int provider;

    unsigned int hash() const {
        return qHash(name % code % QString(provider));
    }
};

class CountryList : public QObject
{
    Q_OBJECT
public:
    explicit CountryList(QObject *parent = nullptr);
    virtual ~CountryList();

    std::size_t size() const;

    CountryItemPtr get(int index);
    void append(const CountryItemPtr &country);
    void appendList(CountryListPtr &countryList);

signals:
    void preItemAppended();
    void postItemAppended();

public slots:

private:
    using Hash = unsigned int;
    std::map<Hash, unsigned int> m_hashToRow;
    std::vector<CountryItemPtr> m_countryItems;
};

#endif // COUNTRYLIST_H
