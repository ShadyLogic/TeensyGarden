// Garden Manager
// Jacob Rogers

#include <GardenManager.h>


// ********   G A R D E N   M A N A G E R   M E T H O D S   ********

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

// ********   Z O N E   M E T H O D S   ********

Zone::Zone(const char zoneName[20], uint8_t sen_pin, uint8_t val_pin1, uint8_t val_pin2)
{
    strcpy(m_zoneName, zoneName);
    m_moistureSensorPin = sen_pin;
    m_valvePin1 = val_pin1;
    m_valvePin2 = val_pin2;

    pinMode(m_moistureSensorPin, INPUT);
    pinMode(m_valvePin1, OUTPUT);
    pinMode(m_valvePin2, OUTPUT);
    digitalWrite(m_valvePin1, LOW);
    digitalWrite(m_valvePin2, LOW);
}

uint16_t Zone::moisture()
{
    uint16_t rc = analogRead(m_moistureSensorPin);
    return rc;
}

void Zone::openValve()
{
    digitalWrite(m_valvePin1, HIGH);
    digitalWrite(m_valvePin2, HIGH);
}

void Zone::closeValve()
{
    digitalWrite(m_valvePin1, LOW);
    digitalWrite(m_valvePin2, LOW);
}

void Zone::handleSchedule()
{
    switch(m_schedType)
    {
        case DOW:
        {
            handleSchedDOW();
        }
        break;

        case INTERVAL:
        {
            handleSchedInterval();
        }
        break;

        case SENSOR:
        {
            handleSchedSensor();
        }
        break;

        case NONE:
        default:
        break;

        
    }
}

void Zone::handleSchedDOW()
{

}

void Zone::handleSchedInterval()
{

}

void Zone::handleSchedSensor()
{
    if (now() < m_lastWaterTime + m_minTimeBetweenWater) return;

    
}
