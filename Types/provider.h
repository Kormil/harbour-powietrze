#ifndef PROVIDER_H
#define PROVIDER_H

#include <QObject>
#include <memory>

class Connection;

class ProviderData : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(int airQualityIndex READ airQualityIndexId WRITE setAirQualityIndexId NOTIFY airQualityIndexIdChanged)
    Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(QString site READ site NOTIFY siteChanged)
    Q_PROPERTY(int nameVariant READ nameVariant WRITE setNameVariant NOTIFY dataChanged)

public:
    explicit ProviderData(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~ProviderData() {}

    int id() const;
    void setId(int value);
    Connection *connection() const;
    void setConnection(Connection *value);
    bool enabled() const;
    void setEnabled(bool value);
    QString name() const;
    void setName(const QString &value);
    QString shortName() const;
    void setShortName(const QString &value);
    QString site() const;
    void setSite(const QString &value);
    QString icon() const;
    void setIcon(const QString &value);
    int airQualityIndexId() const;
    void setAirQualityIndexId(int value);
    int nameVariant() const;
    void setNameVariant(int value);

signals:
    void dataChanged();
    void nameChanged();
    void enabledChanged();
    void iconChanged();
    void siteChanged();
    void airQualityIndexIdChanged();

private:
    int m_id;
    Connection* m_connection;
    bool m_enabled;
    QString m_name;
    QString m_shortName;
    QString m_site;
    QString m_icon;
    int m_nameVariant;
    int m_airQualityIndexId;
};

#endif // PROVIDER_H
