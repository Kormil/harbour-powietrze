#ifndef POLLUTION_H
#define POLLUTION_H

#include <QString>
#include <QDateTime>
#include <QVariant>
#include <memory>
#include <numeric>
#include <atomic>
#include <vector>

struct PollutionUnit
{
    QString pollutionCode;
    QString unit;
    QString label;
};

struct PollutionValue
{
    float value;
    QDateTime date;
};

class Pollution
{
public:
    QVariant id;
    QString name;
    QString code;
    QString unit;
    QDateTime date;

    Pollution();
    Pollution(const Pollution& second);
    Pollution& operator=(const Pollution& second);
    float value() const;
    float avg(size_t hours) const;
    bool isInitialized() const;
    void setInitialized(bool m_initialized);

    void setValues(PollutionValue value);
    void setValues(const std::vector<PollutionValue> &value);

private:
    std::atomic_bool m_initialized;
    std::vector<PollutionValue> m_values;
};

using PollutionUnitList = std::vector<PollutionUnit>;

#endif // POLLUTION_H
