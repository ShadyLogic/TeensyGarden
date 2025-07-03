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
    MH.serPtr()->println("***** Zone Info *****");
    for (int i = 0; i < m_zonesIndex; i++)
    {
        m_zones[i]->printStatus(output);
        MH.serPtr()->println();
    }
    MH.serPtr()->println("*********************");
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
    if (now() == previousMidnight(now())) 
    {
        
        EEPROM.put(OFFSET_STOREEE, StoreEE);
        MH.serPtr()->println("*** MIDNIGHT: Settings Saved");
        delay(1000);
    }
}

void GardenManager::saveAllZones()
{
    for (int i = 0; i < 4; i++)
    {
        switch (i+1)
        {
            case 1:
            {
                strcpy(StoreEE.zone1name, m_zones[i]->name());
                StoreEE.zone1dryThreshold = m_zones[i]->dryThreshold();
                StoreEE.zone1wetThreshold = m_zones[i]->wetThreshold();
                StoreEE.zone1timeBetweenWatering_hr = m_zones[i]->timeBetweenWatering_hr();
                StoreEE.zone1durationToWater_min = m_zones[i]->durationToWater_min();
                StoreEE.zone1scheduleMode = m_zones[i]->schedMode();
                StoreEE.zone1lastWaterTime = m_zones[i]->lastWaterTime();
                break;
            }
            case 2:
            {
                strcpy(StoreEE.zone2name, m_zones[i]->name());
                StoreEE.zone2dryThreshold = m_zones[i]->dryThreshold();
                StoreEE.zone2wetThreshold = m_zones[i]->wetThreshold();
                StoreEE.zone2timeBetweenWatering_hr = m_zones[i]->timeBetweenWatering_hr();
                StoreEE.zone2durationToWater_min = m_zones[i]->durationToWater_min();
                StoreEE.zone2scheduleMode = m_zones[i]->schedMode();
                StoreEE.zone2lastWaterTime = m_zones[i]->lastWaterTime();
                break;
            }
            case 3:
            {
                strcpy(StoreEE.zone3name, m_zones[i]->name());
                StoreEE.zone3dryThreshold = m_zones[i]->dryThreshold();
                StoreEE.zone3wetThreshold = m_zones[i]->wetThreshold();
                StoreEE.zone3timeBetweenWatering_hr = m_zones[i]->timeBetweenWatering_hr();
                StoreEE.zone3durationToWater_min = m_zones[i]->durationToWater_min();
                StoreEE.zone3scheduleMode = m_zones[i]->schedMode();
                StoreEE.zone3lastWaterTime = m_zones[i]->lastWaterTime();
                break;
            }
            case 4:
            {
                strcpy(StoreEE.zone4name, m_zones[i]->name());
                StoreEE.zone4dryThreshold = m_zones[i]->dryThreshold();
                StoreEE.zone4wetThreshold = m_zones[i]->wetThreshold();
                StoreEE.zone4timeBetweenWatering_hr = m_zones[i]->timeBetweenWatering_hr();
                StoreEE.zone4durationToWater_min = m_zones[i]->durationToWater_min();
                StoreEE.zone4scheduleMode = m_zones[i]->schedMode();
                StoreEE.zone4lastWaterTime = m_zones[i]->lastWaterTime();
                break;
            }
        }
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
    m_lastWaterTime = now();
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
    MH.serPtr()->print(m_zoneName);
    MH.serPtr()->print(" - Mode: ");
    MH.serPtr()->println(SchedModeToString(m_schedMode));
    MH.serPtr()->print("Dry: ");
    MH.serPtr()->print(m_dryThreshold);
    MH.serPtr()->print(" | Wet: ");
    MH.serPtr()->println(m_wetThreshold);
    MH.serPtr()->print("Moisture Level: ");
    MH.serPtr()->println(moisture());
    MH.serPtr()->print("Valve is ");
    digitalRead(m_valvePin1) ? MH.serPtr()->print("\033[1;32mOPEN\033[0m") : MH.serPtr()->print("\033[1;31mCLOSED\033[0m");
    if (m_valveTimerRunning)
    {
        int hourTime = ((timeToTurnOffValve() - now()) / SECS_PER_HOUR);
        int minTime = (((timeToTurnOffValve() - now()) % SECS_PER_HOUR) / SECS_PER_MIN);
        int secTime = ((timeToTurnOffValve() - now()) % SECS_PER_MIN);
        MH.serPtr()->print(" for ");
        if (hourTime > 0) {MH.serPtr()->print(hourTime); MH.serPtr()->print("h");}
        if (minTime > 0) {MH.serPtr()->print(minTime); MH.serPtr()->print("m");}
        if (secTime > 0) {MH.serPtr()->print(secTime); MH.serPtr()->print("s");}
    }
    else
    {
        MH.serPtr()->print("\nLast Watered @ ");
        digitalClockDisplay(m_lastWaterTime);
    }
    MH.serPtr()->println();
}

void Zone::handleSchedule()
{
    switch(m_schedMode)
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

        case INTERVAL_SENSOR:
        {
            handleSchedIntervalSensor();
        }
        break;

        case NONE:
        default:
        break;
    }
}

void Zone::handleSchedDOW()
{
    if (now() < m_lastWaterTime + (SECS_PER_HOUR * m_timeBetweenWatering_hr)) return;
    if (m_schedDOWday[((int)dayOfWeek(now())-1)] == 0) return;
    if (hour(previousMidnight(now()) + m_scheduleTime_afterMidnight) == hour() && minute(previousMidnight(now()) + m_scheduleTime_afterMidnight) == minute() && !m_valveTimerRunning)
    {
        MH.serPtr()->print(name());
        MH.serPtr()->println(" running scheduled watering");
        openValve();
        timeToTurnOffValve(now() + (SECS_PER_MIN * m_durationToWater_min));
    }
}

void Zone::handleSchedInterval()
{
    if (now() < m_lastWaterTime + (SECS_PER_HOUR * m_timeBetweenWatering_hr)) return;
    openValve();
    timeToTurnOffValve(now() + (SECS_PER_MIN * m_durationToWater_min));
    MH.serPtr()->print("Watering ");
    MH.serPtr()->print(m_zoneName);
    MH.serPtr()->print(" for ");
    MH.serPtr()->print(m_durationToWater_min);
    MH.serPtr()->println(" min");
}

void Zone::handleSchedSensor()
{
    if (moisture() > m_dryThreshold) return;
    openValve();
    timeToTurnOffValve(now() + (SECS_PER_MIN * m_durationToWater_min));
}

void Zone::handleSchedIntervalSensor()
{
    if (moisture() >= m_wetThreshold)
    {
        if (now() < (m_lastWaterTime + (SECS_PER_HOUR * 4))) m_lastWaterTime = now();
        return;
    }
    if (now() < m_lastWaterTime + (SECS_PER_HOUR * m_timeBetweenWatering_hr)) return;
    openValve();
    timeToTurnOffValve(now() + (SECS_PER_MIN * m_durationToWater_min));
    MH.serPtr()->print("Watering ");
    MH.serPtr()->print(m_zoneName);
    MH.serPtr()->print(" for ");
    MH.serPtr()->print(m_durationToWater_min);
    MH.serPtr()->println(" min");
}

void Zone::setScheduleDOW(time_t time)
{
    m_schedDOWday[(int)dayOfWeek(time)-1]   = true;
    m_scheduleTime_afterMidnight            = time - previousMidnight(time);

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
    MH.serPtr()->print(hour(m_scheduleTime_afterMidnight));
    MH.serPtr()->print(":");
    if (minute(m_scheduleTime_afterMidnight) < 10) MH.serPtr()->print('0');
    MH.serPtr()->print(minute(m_scheduleTime_afterMidnight));
    MH.serPtr()->print(" for ");
    MH.serPtr()->print(m_durationToWater_min);
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

void digitalClockDisplay(time_t time){
    // digital clock display of the time
    print12Hour(hour(time));
    printDigits(minute(time));
    // printDigits(second(time));
    if (hour(time) >= 12)
    {
        MH.serPtr()->print("PM,");
    }
    else
    {
        MH.serPtr()->print("AM,");
    }
    MH.serPtr()->print(" ");
    MH.serPtr()->print(month(time));
    MH.serPtr()->print("/");
    MH.serPtr()->print(day(time));
    MH.serPtr()->print("/");
    MH.serPtr()->print(year(time)); 
}

void printDigits(int digits){
    // utility function for digital clock display: prints preceding colon and leading 0
    MH.serPtr()->print(":");
    if(digits < 10)
        MH.serPtr()->print('0');
    MH.serPtr()->print(digits);
}

void print12Hour(int digits){
    int theHour = digits % 12;
    if (theHour == 0)
    {
        MH.serPtr()->print(12);
    }
    else
    {
        MH.serPtr()->print(theHour);
    }
}

ScheduleMode intToSchedMode(int number) 
{
    return (ScheduleMode)number;
}

uint8_t SchedModeToInt(ScheduleMode mode) 
{
    return (uint8_t)mode;
}

String SchedModeToString(ScheduleMode mode)
{
    switch (mode)
    {
        case ScheduleMode::NONE:
            return "NONE";
        
        case ScheduleMode::DOW:
            return "DOW";

        case ScheduleMode::INTERVAL:
            return "INTERVAL";

        case ScheduleMode::SENSOR:
            return "SENSOR";

        case ScheduleMode::INTERVAL_SENSOR:
            return "INTERVAL/SENSOR";
    }
    
    return "ERROR";
}