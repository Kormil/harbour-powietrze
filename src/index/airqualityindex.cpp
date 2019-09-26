#include "airqualityindex.h"
#include "src/modelsmanager.h"

ModelsManager* AirQualityIndex::m_modelsManager;

int AirQualityIndex::id() const
{
    return m_id;
}

void AirQualityIndex::setModelsManager(ModelsManager *modelsManager)
{
    m_modelsManager = modelsManager;
}
