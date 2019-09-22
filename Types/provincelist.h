#ifndef PROVINCELIST_H
#define PROVINCELIST_H

#include <QObject>
#include <QStringBuilder>
#include <memory>
#include <map>

class StationList;
class ProvinceList;
class ProvinceItem;

using ProvinceItemPtr = std::shared_ptr<ProvinceItem>;
using ProvinceListPtr = std::shared_ptr<ProvinceList>;

struct ProvinceItem
{
    QString name;
    QString countryCode;
    int provider;

    unsigned int hash() const {
        return qHash(name % countryCode % QString(provider));
    }
};

class ProvinceList : public QObject
{
    Q_OBJECT
public:
    explicit ProvinceList(QObject *parent = nullptr);
    virtual ~ProvinceList();

    size_t size() const;

    ProvinceItemPtr get(int index);
    void append(const ProvinceItemPtr &item);
    void appendList(ProvinceListPtr &provinceList);

signals:
    void preItemAppended();
    void postItemAppended();

public slots:

private:
    using Hash = unsigned int;
    std::map<Hash, unsigned int> m_hashToRow;
    std::vector<ProvinceItemPtr> m_provinceItems;
};


#endif // PROVINCELIST_H
