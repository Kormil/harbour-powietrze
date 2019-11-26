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

    void setValues(PollutionValue value);
    void setValues(const std::vector<PollutionValue> &value);

private:
    std::atomic_bool initialized;
    std::vector<PollutionValue> values;
};

using PollutionUnitList = std::vector<PollutionUnit>;

#endif // POLLUTION_H
