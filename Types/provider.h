#ifndef PROVIDER_H
#define PROVIDER_H

#include <QObject>
#include <memory>

class Connection;
class Settings;
class ModelsManager;

class ProviderData : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(int airQualityIndex READ airQualityIndexId WRITE setAirQualityIndexId NOTIFY airQualityIndexChanged)
    Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(QString site READ site NOTIFY siteChanged)
    Q_PROPERTY(int nameVariant READ nameVariant WRITE setNameVariant NOTIFY dataChanged)
    Q_PROPERTY(QString apiKey READ apiKey WRITE setApiKey NOTIFY apiKeyChanged)
    Q_PROPERTY(int requestLimit READ requestLimit NOTIFY requestLimitChanged)
    Q_PROPERTY(int requestRemaining READ requestRemaining NOTIFY requestLimitChanged)

public:
    explicit ProviderData(QObject* parent = nullptr);
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
    QString apiKey() const;
    void setApiKey(const QString &apiKey);    
    int requestLimit() const;
    void setRequestLimit(int requestLimit);
    int requestRemaining() const;
    void setRequestRemaining(int requestRemaining);

    void setModelsManager(ModelsManager *modelsManager);

signals:
    void dataChanged();
    void nameChanged();
    void enabledChanged();
    void iconChanged();
    void siteChanged();
    void airQualityIndexChanged();
    void apiKeyChanged();
    void requestLimitChanged();

private:
    int m_id;
    Connection* m_connection;
    bool m_enabled;
    QString m_name;
    QString m_shortName;
    QString m_site;
    QString m_icon;
    int m_nameVariant = 0;
    int m_airQualityIndexId;
    QString m_apiKey;
    int m_requestLimit = 0;
    int m_requestRemaining = 0;

    Settings * m_settings;
    ModelsManager * m_modelsManager;
};

#endif // PROVIDER_H
