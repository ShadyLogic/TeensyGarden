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
        m_zones[i]->handleSchedule();
    }
}

// ********   Z O N E   M E T H O D S   ********

Zone::Zone(uint8_t sen_pin, uint8_t val_pin1, uint8_t val_pin2)
{
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

void Zone::timeToTurnOffValve(time_t newTime)
{
    m_timeToTurnOffValve = newTime; 
    m_valveTimerRunning = true;
}

void Zone::printStatus(Stream* output)
{
    output->print(m_zoneName);
    output->print(" - Mode: ");

    switch (m_schedType)
    {
        case ScheduleMode::NONE:
            output->println("None");
            break;

        case ScheduleMode::DOW:
            output->println("DOW");
            break;

        case ScheduleMode::INTERVAL:
            output->println("Interval");
            break;

        case ScheduleMode::SENSOR:
            output->println("Sensor");
            break;
    }

    output->print("Dry: ");
    output->print(m_dryThreshold);
    output->print(" | Wet: ");
    output->println(m_wetThreshold);
    output->print("Moisture Level: ");
    output->println(moisture());
    output->print("Valve is ");
    digitalRead(m_valvePin1) ? output->print("\033[1;32mOPEN\033[0m") : output->print("\033[1;31mCLOSED\033[0m");
    if (m_valveTimerRunning)
    {
        int hourTime = ((timeToTurnOffValve() - now()) / SECS_PER_HOUR);
        int minTime = (((timeToTurnOffValve() - now()) % SECS_PER_HOUR) / SECS_PER_MIN);
        int secTime = ((timeToTurnOffValve() - now()) % SECS_PER_MIN);
        output->print(" for ");
        if (hourTime > 0) {output->print(hourTime); output->print("h");}
        if (minTime > 0) {output->print(minTime); output->print("m");}
        if (secTime > 0) {output->print(secTime); output->print("s");}
    }
    output->println();
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
    if (now() < m_lastWaterTime + m_timeBetweenWatering) return;
    if (m_schedDOWday[((int)dayOfWeek(now())-1)] == 0) return;
    if (m_schedDOWhour == hour() && m_schedDOWmin == minute() && !m_valveTimerRunning)
    {
        MH.serPtr()->print(name());
        MH.serPtr()->println(" running scheduled watering");
        openValve();
        timeToTurnOffValve(now() + (SECS_PER_MIN * m_durationToWater));
        m_lastWaterTime = now();
    }
}

void Zone::handleSchedInterval()
{
    if (now() < m_lastWaterTime + m_timeBetweenWatering) return;
}

void Zone::handleSchedSensor()
{
    if (now() < m_lastWaterTime + m_timeBetweenWatering) return;
    if (moisture() > m_dryThreshold) return;
    openValve();
    timeToTurnOffValve(now() + (SECS_PER_MIN * m_durationToWater));
    m_lastWaterTime = now();
}

void Zone::setScheduleDOW(time_t time)
{
    m_schedDOWday[(int)dayOfWeek(time)-1] = true;
    m_schedDOWhour  = hour(time);
    m_schedDOWmin   = minute(time);
    m_schedType     = ScheduleMode::DOW;
    m_durationToWater = 15;

    printScheduleDOW();
}

void Zone::printScheduleDOW()
{
    MH.serPtr()->println("Scheduled Run Times:");
    for (int i = 0; i < 7; i++)
    {
        if (m_schedDOWday[i])
        {
            MH.serPtr()->print("- ");
            MH.serPtr()->println(numberToDay(i));
        }
    }
    MH.serPtr()->print("@ ");
    MH.serPtr()->print(m_schedDOWhour);
    MH.serPtr()->print(":");
    if (m_schedDOWmin < 10) MH.serPtr()->print('0');
    MH.serPtr()->print(m_schedDOWmin);
    MH.serPtr()->print(" for ");
    MH.serPtr()->print(m_durationToWater);
    MH.serPtr()->println(" min");
}

String numberToDay(int number)
{
    switch (number)
    {
        case 0:
            return "Sunday";
        case 1:
            return "Monday";
        case 2:
            return "Tuesday";
        case 3:
            return "Wednesday";
        case 4:
            return "Thursday";
        case 5:
            return "Friday";
        case 6:
            return "Saturday";
        default:
            return "ERROR";
    }
}