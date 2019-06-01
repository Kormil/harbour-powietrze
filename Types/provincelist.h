#ifndef PROVINCELIST_H
#define PROVINCELIST_H

#include <QObject>
#include <memory>

class StationList;
class ProvinceList;
class ProvinceItem;

using ProvinceItemPtr = std::unique_ptr<ProvinceItem>;
using ProvinceListPtr = std::unique_ptr<ProvinceList>;

struct ProvinceItem
{
    QString name;
};

class ProvinceList : public QObject
{
    Q_OBJECT
public:
    explicit ProvinceList(QObject *parent = nullptr);
    virtual ~ProvinceList();
\
    bool setItemAt(unsigned int index, const ProvinceItem &provinceItem);

    size_t size() const;

    const std::vector<ProvinceItemPtr> &provinceItems() const;
    ProvinceItem* get(unsigned int index) const;

    static ProvinceListPtr getFromJson(const QJsonDocument& jsonDocument);

    void append(const ProvinceItem &item);

signals:
    void preItemAppended();
    void postItemAppended();

public slots:

private:
    std::vector<ProvinceItemPtr> m_provinceItems;
};


#endif // PROVINCELIST_H
