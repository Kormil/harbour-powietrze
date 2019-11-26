#include "pollution.h"

Pollution::Pollution() {
    initialized = false;
}

Pollution::Pollution(const Pollution& second) {
    id = second.id;
    name = second.name;
    code = second.code;
    values = second.values;
    date = second.date;
    unit = second.unit;
    initialized = second.isInitialized();
}

Pollution& Pollution::operator=(const Pollution& second) {
    id = second.id;
    name = second.name;
    code = second.code;
    values = second.values;
    date = second.date;
    unit = second.unit;
    initialized = second.isInitialized();

    return *this;
}

float Pollution::value() const {
    if (values.size())
        return values.front().value;

    return 0.0f;
}

float Pollution::avg(size_t hours) const {
    int secods = hours * 60;
    QDateTime lastDateTime = QDateTime::currentDateTime().addSecs(-secods);
    size_t index = 1;

    float sum = values[index].value;
    for (; index < values.size(); ++index) {
        if (lastDateTime <= values[index].date) {
            sum += values[index].value;
        } else {
            break;
        }
    }

    return sum / index;
}

bool Pollution::isInitialized() const {
    return initialized;
}


void Pollution::setValues(PollutionValue value)
{
    values.push_back(value);
    initialized = true;
}

void Pollution::setValues(const std::vector<PollutionValue> &value)
{
    values = value;
    initialized = true;
}
