/*
 * GardenManager.cpp
 * 
 * Implements the logic for the Teensy Garden Irrigation System.
 * 
 * This file defines the behavior for the GardenManager and Zone classes:
 * - Valve timing and moisture-based watering logic
 * - Schedule handling for various modes (DOW, INTERVAL, SENSOR, INTERVAL_SENSOR)
 * - Zone initialization and real-time control
 * - Logging to SD card and EEPROM persistence
 * - Human-readable status printing for monitoring
 * 
 * Key Functions:
 * - GardenManager::maintain()      → Main control loop logic
 * - Zone::handleSchedule()         → Applies scheduling logic per zone
 * - logData(), printLog(), clearLog() → SD card logging utilities
 *
 * Author: Jacob Rogers
 * Date: May 2025
 */


#include <GardenManager.h>

bool SDworking = false;


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
                String temp;
                temp += "Closing ";
                temp += String(m_zones[i]->name());
                temp += " valve";
                logData(temp);
                m_zones[i]->closeValve();
            }
        }
        m_zones[i]->handleSchedule();
    }
    if (now() == previousMidnight(now()) + (SECS_PER_HOUR * 12)) 
    {
        saveAllZones();
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
                StoreEE.zone1scheduleTime_afterMidnight = m_zones[i]->scheduleTime_afterMidnight();
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
                StoreEE.zone2scheduleTime_afterMidnight = m_zones[i]->scheduleTime_afterMidnight();
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
                StoreEE.zone3scheduleTime_afterMidnight = m_zones[i]->scheduleTime_afterMidnight();
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
                StoreEE.zone4scheduleTime_afterMidnight = m_zones[i]->scheduleTime_afterMidnight();
                break;
            }
        }
    }
    EEPROM.put(OFFSET_STOREEE, StoreEE);
    logData("*** Settings Auto-Saved ***");
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
    if (m_schedMode == INTERVAL || m_schedMode == INTERVAL_SENSOR)
    {
        MH.serPtr()->print("Watering every ");
        MH.serPtr()->print(m_timeBetweenWatering_hr);
        MH.serPtr()->println(" hrs");
    }
    if (m_schedMode == SENSOR || m_schedMode == INTERVAL_SENSOR)
    {
        MH.serPtr()->print("Dry: ");
        MH.serPtr()->print(m_dryThreshold);
        MH.serPtr()->print(" | Wet: ");
        MH.serPtr()->println(m_wetThreshold);
        MH.serPtr()->print("Moisture Level: ");
        MH.serPtr()->println(moisture());
    }
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
        MH.serPtr()->println();
    }
    else
    {
        MH.serPtr()->print("\nLast Water @ ");
        MH.serPtr()->println(timeAndDate(m_lastWaterTime));
        if (m_schedMode == INTERVAL || m_schedMode == INTERVAL_SENSOR)
        {
            MH.serPtr()->print("Next Water @ ");
            MH.serPtr()->println(timeAndDate(m_lastWaterTime + (SECS_PER_HOUR * m_timeBetweenWatering_hr)));
        }
    }
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
        String temp;
        temp += ("Opening ");
        temp += (m_zoneName);
        temp += (" valve for ");
        temp += (m_durationToWater_min);
        temp += " min";
        logData(temp);
    }
}

void Zone::handleSchedInterval()
{
    if (now() < m_lastWaterTime + (SECS_PER_HOUR * m_timeBetweenWatering_hr)) return;
    openValve();
    timeToTurnOffValve(now() + (SECS_PER_MIN * m_durationToWater_min));
    String temp;
    temp += ("Opening ");
    temp += (m_zoneName);
    temp += (" valve for ");
    temp += (m_durationToWater_min);
    temp += " min";
    logData(temp);
}

void Zone::handleSchedSensor()
{
    if (moisture() >= m_wetThreshold)
    {
        if (now() > (m_lastWaterTime + (SECS_PER_MIN * (m_durationToWater_min + 1)))) m_lastWaterTime = now();
        return;
    }

    if (moisture() > m_dryThreshold) return;
    openValve();
    timeToTurnOffValve(now() + (SECS_PER_MIN * m_durationToWater_min));
    String temp;
    temp += ("Opening ");
    temp += (m_zoneName);
    temp += (" valve for ");
    temp += (m_durationToWater_min);
    temp += " min";
    logData(temp);
}

void Zone::handleSchedIntervalSensor()
{
    if (moisture() >= m_wetThreshold)
    {
        if (now() > (m_lastWaterTime + (SECS_PER_MIN * (m_durationToWater_min + 1)))) m_lastWaterTime = now();
        return;
    }

    if (now() < m_lastWaterTime + (SECS_PER_HOUR * m_timeBetweenWatering_hr)) return;
    openValve();
    timeToTurnOffValve(now() + (SECS_PER_MIN * m_durationToWater_min));
    String temp;
    temp += ("Opening ");
    temp += (m_zoneName);
    temp += (" valve for ");
    temp += (m_durationToWater_min);
    temp += " min";
    logData(temp);
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

String timeAndDate(time_t time){
    String rc;
    rc += timeToString(time);
    rc += " ";
    rc += timeToDate(time);

    return rc;
}

String dateAndTime(time_t time){
    String rc;
    rc += timeToDate(time);
    rc += " ";
    rc += timeToString(time);

    return rc;
}

String printDigits(int digits){
    String rc;
    if(digits < 10)
    {
        rc += "0";
        rc += String(digits);
        return rc;
    }
    rc += String(digits);
    return rc;
}

String print12Hour(int digits){
    String rc;
    int theHour = digits % 12;
    if (theHour == 0)
    {
        return "12";
    }
    else
    {
        rc += String(theHour);
        return rc;
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

time_t arrayToTime(char timeArray[8])
{
    time_t tempTime = 0;

    tempTime += (time_t)(timeArray[0] - '0') * (SECS_PER_HOUR * 10);
    tempTime += (time_t)(timeArray[1] - '0') * SECS_PER_HOUR;
    if (timeArray[0] == '1' && timeArray[1] == '2') tempTime = 0;
    tempTime += (time_t)(timeArray[3] - '0') * (SECS_PER_MIN * 10);
    tempTime += (time_t)(timeArray[4] - '0') * SECS_PER_MIN;
    if (toUpperCase(timeArray[5]) == 'P') tempTime += (SECS_PER_HOUR * 12);
    tempTime = tempTime % SECS_PER_DAY;
    return tempTime;
}

String timeToString(time_t time)
{
    String rc;
    rc += print12Hour(hour(time));
    rc += ':';
    rc += printDigits(minute(time));
    hour(time) >= 12 ? rc += "PM" : rc += "AM";
    return rc;
}

String timeToDate(time_t time)
{
    String rc;
    rc += String(month(time));
    rc += "/";
    rc += String(day(time));
    rc += "/";
    rc += String(year(time));
    return rc;
}

void logData(String data)
{
    if (!SDworking) 
    {
        MH.serPtr()->println("*** SD Card failed to initialize ***\n");
        MH.serPtr()->println(data);
        return;
    }

    File dataFile = SD.open("log.txt", FILE_WRITE);

    if (dataFile) 
    {
        dataFile.print(dateAndTime(now()));
        dataFile.print(" - ");
        dataFile.println(data);
        dataFile.close();

        MH.serPtr()->println(data);
    } 
    else 
    {
        MH.serPtr()->println("*** Error opening log.txt ***");
    }
}

void printLog()
{
    File dataFile = SD.open("log.txt");
    if (dataFile) 
    {
        while (dataFile.available()) 
        {
            MH.serPtr()->write(dataFile.read());
        }
        dataFile.close();
    } 
    else 
    {
        MH.serPtr()->println("*** Error opening log.txt ***");
    }
}

void clearLog()
{
    if (!SD.remove("log.txt")) 
    {
        MH.serPtr()->println("*** ERROR DELETING LOG ***");
    }
    else
    {
        MH.serPtr()->println("*** LOG CLEARED ***");
    }

    File dataFile = SD.open("log.txt", FILE_WRITE);
    if (dataFile) 
    {
        dataFile.println("              >>>>>> TEENSY GARDEN LOG <<<<<<\n");
        dataFile.close();
    } 
    else 
    {
        MH.serPtr()->println("*** ERROR CREATING NEW LOG ***");
    }
}