#include "pollution.h"

Pollution::Pollution() {
    m_initialized = false;
}

Pollution::Pollution(const Pollution& second) {
    id = second.id;
    name = second.name;
    code = second.code;
    m_values = second.m_values;
    date = second.date;
    unit = second.unit;
    m_initialized = second.isInitialized();
}

Pollution& Pollution::operator=(const Pollution& second) {
    id = second.id;
    name = second.name;
    code = second.code;
    m_values = second.m_values;
    date = second.date;
    unit = second.unit;
    m_initialized = second.isInitialized();

    return *this;
}

float Pollution::value() const {
    if (m_values.size())
        return m_values.front().value;

    return 0.0f;
}

float Pollution::avg(size_t hours) const {
    int secods = hours * 60;
    QDateTime lastDateTime = QDateTime::currentDateTime().addSecs(-secods);
    size_t index = 1;

    float sum = m_values[index].value;
    for (; index < m_values.size(); ++index) {
        if (lastDateTime <= m_values[index].date) {
            sum += m_values[index].value;
        } else {
            break;
        }
    }

    return sum / index;
}

bool Pollution::isInitialized() const {
    return m_initialized;
}

void Pollution::setInitialized(bool initialized)
{
    m_initialized = initialized;
}


void Pollution::setValues(PollutionValue value)
{
    m_values.push_back(value);
    m_initialized = true;
}

void Pollution::setValues(const std::vector<PollutionValue> &value)
{
    m_values = value;
    m_initialized = true;
}
