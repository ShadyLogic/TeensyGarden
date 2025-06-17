// Garden Manager
// Jacob Rogers

#include <GardenManager.h>


// ********   G A R D E N   M A N A G E R   M E T H O D S   ********

void GardenManager::addZone(Zone* newZone)
{
    m_zones[m_zonesIndex] = newZone;
    m_zonesIndex++;
}

void GardenManager::printZoneStatus(Stream* output)
{
    output->println("***** Zone Info *****");
    for (int i = 0; i < m_zonesIndex; i++)
    {
        m_zones[i]->printStatus(output);
        output->println();
    }
    output->println("*********************");
}

void GardenManager::closeAllValves()
{
    for (int i = 0; i < m_zonesIndex; i++)
    {
        m_zones[i]->closeValve();
    }
}

void GardenManager::openAllValves()
{
    for (int i = 0; i < m_zonesIndex; i++)
    {
        m_zones[i]->openValve();
    }
}

Zone* GardenManager::valveRunTime(int zone, int runTime)
{
    m_zones[zone]->openValve();
    m_zones[zone]->timeToTurnOffValve(now() + (SECS_PER_MIN * (time_t)runTime));
    return m_zones[zone];
}

void GardenManager::maintain()
{
    for (int i = 0; i < m_zonesIndex; i++)
    {
        if (m_zones[i]->valveTimerRunning() && m_zones[i]->valveTimerExpired())
        {
            {
                MH.serPtr()->print("Closing ");
                MH.serPtr()->print(m_zones[i]->name());
                MH.serPtr()->println(" valve");
                m_zones[i]->closeValve();
            }
        }
    }
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

    for(int i = 0; i < 7; i++)
    {
        m_schedDOWday[i] = false;
    }
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
    m_valveTimerRunning = false;
}

void Zone::printStatus(Stream* output)
{
    output->println(m_zoneName);
    output->print("Moisture Level: ");
    output->println(moisture());
    output->print("Valve is ");
    digitalRead(m_valvePin1) ? output->println("OPEN") : output->println("CLOSED");
    if (m_valveTimerRunning)
    {
        int hourTime = ((timeToTurnOffValve() - now()) / SECS_PER_HOUR);
        int minTime = (((timeToTurnOffValve() - now()) % SECS_PER_HOUR) / SECS_PER_MIN);
        int secTime = ((timeToTurnOffValve() - now()) % SECS_PER_MIN);
        output->print("Valve will close in ");
        if (hourTime > 0) {output->print(hourTime); output->print("h");}
        if (minTime > 0) {output->print(minTime); output->print("m");}
        if (secTime > 0) {output->print(secTime); output->print("s");}
        output->println();
    }
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
    if (now() < m_lastWaterTime + m_minTimeBetweenWater) return;
    if (m_schedDOWday[((int)dayOfWeek(now())-1)] == 0) return;
    if (m_schedDOWhour == hour() && m_schedDOWmin == minute() && !m_valveTimerRunning)
    {
        openValve();
        timeToTurnOffValve(now() + m_scheduledRuntime);
        m_lastWaterTime = now();
    }
}

void Zone::handleSchedInterval()
{
    if (now() < m_lastWaterTime + m_minTimeBetweenWater) return;
}

void Zone::handleSchedSensor()
{
    if (now() < m_lastWaterTime + m_minTimeBetweenWater) return;
    openValve();
    timeToTurnOffValve(now() + m_scheduledRuntime);
}

void Zone::timeToTurnOffValve(time_t newTime)
{
    m_timeToTurnOffValve = newTime; 
    m_valveTimerRunning = true;
}