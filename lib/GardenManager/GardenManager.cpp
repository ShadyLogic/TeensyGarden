// Garden Manager
// Jacob Rogers

#include <GardenManager.h>

void GardenManager::addZone(Zone newZone)
{
    m_zones[m_zonesIndex] = newZone;
    m_zonesIndex++;
}

void GardenManager::printZoneInfo(Stream* output)
{
    output->println("***** Zone Info *****");
    for (int i = 0; i < m_zonesIndex; i++)
    {
        output->print(m_zones[i].name());
        output->print(" Moisture Level: ");
        output->println(m_zones[i].moisture());
    }
    output->println();
}

Zone::Zone(const char zoneName[20], uint8_t sen_pin, uint8_t val_pin)
{
    strcpy(m_zoneName, zoneName);
    m_moistureSensorPin = sen_pin;
    m_valvePin = val_pin;

    pinMode(m_moistureSensorPin, INPUT);
    pinMode(m_valvePin, OUTPUT);
    digitalWrite(m_valvePin, LOW);
}

uint16_t Zone::moisture()
{
    uint16_t rc = analogRead(m_moistureSensorPin);
    return rc;
}