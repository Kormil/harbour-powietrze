#include "provider.h"
#include "src/settings.h"

Connection *ProviderData::connection() const
{
    return m_connection;
}

void ProviderData::setConnection(Connection *value)
{
    m_connection = value;
}

bool ProviderData::enabled() const
{
    return m_enabled;
}

void ProviderData::setEnabled(bool value)
{
    m_enabled = value;
    emit enabledChanged();
    emit dataChanged();

    Settings * settings = qobject_cast<Settings*>(Settings::instance(nullptr, nullptr));
    settings->setProviderSettings(m_name, "enabled", value);
}

QString ProviderData::name() const
{
    return m_name;
}

void ProviderData::setName(const QString &value)
{
    m_name = value;
    emit nameChanged();
}

QString ProviderData::shortName() const
{
    return m_shortName;
}

void ProviderData::setShortName(const QString &value)
{
    m_shortName = value;
}

QString ProviderData::site() const
{
    return m_site;
}

void ProviderData::setSite(const QString &value)
{
    m_site = value;
    emit siteChanged();
}

QString ProviderData::icon() const
{
    return m_icon;
}

void ProviderData::setIcon(const QString &value)
{
    m_icon = value;
    emit iconChanged();
}

int ProviderData::airQualityIndexId() const
{
    return m_airQualityIndexId;
}

void ProviderData::setAirQualityIndexId(int value)
{
    m_airQualityIndexId = value;
    emit airQualityIndexIdChanged();
    emit dataChanged();

    Settings * settings = qobject_cast<Settings*>(Settings::instance(nullptr, nullptr));
    settings->setProviderSettings(m_name, "aqi", value);
}

int ProviderData::nameVariant() const
{
    return m_nameVariant;
}

void ProviderData::setNameVariant(int value)
{
    m_nameVariant = value;
    emit dataChanged();

    Settings * settings = qobject_cast<Settings*>(Settings::instance(nullptr, nullptr));
    settings->setProviderSettings(m_name, "nameVariant", value);
}

int ProviderData::id() const
{
    return m_id;
}

void ProviderData::setId(int value)
{
    m_id = value;
}
