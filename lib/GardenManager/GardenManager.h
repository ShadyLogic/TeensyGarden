// Garden Manager
// Jacob Rogers

#ifndef GARDEN_MANAGER_H
#define GARDEN_MANAGER_H

#define MAX_ZONES   4

#include <Arduino.h>
#include <Maltbie_Helper.h>
#include <Menu_Config.h>
#include <TimeLib.h>

enum ScheduleMode
{
    NONE,
    DOW,
    INTERVAL,
    SENSOR,
    INTERVAL_SENSOR
};


String          numberToDay(int);
void            printDigits(int digits);
void            digitalClockDisplay(time_t time);
void            print12Hour(int digits);
ScheduleMode    intToSchedMode(int number);
uint8_t         SchedModeToInt(ScheduleMode mode);
String          SchedModeToString(ScheduleMode mode);
time_t          arrayToTime(char timeArray[8]);
void            timeToArray(time_t time, char* destArray);



class Zone {
public:
    Zone(){}
    Zone(uint8_t, uint8_t, uint8_t = 200);

    const char*     name(void)                                  {return m_zoneName;}
    void            name(char* newName)                         {strcpy(m_zoneName, newName);}

    ScheduleMode    scheduleMode()                              {return m_schedMode;}
    void            scheduleMode(ScheduleMode st)               {m_schedMode = st;}

    time_t          lastWaterTime(void)                         {return m_lastWaterTime;}
    void            lastWaterTime(time_t newTime)               {m_lastWaterTime = newTime;}

    uint8_t         timeBetweenWatering_hr(void)                {return m_timeBetweenWatering_hr;}
    void            timeBetweenWatering_hr(uint8_t newTBW)      {m_timeBetweenWatering_hr = newTBW;}

    time_t          timeToTurnOffValve()                        {return m_timeToTurnOffValve;}
    void            timeToTurnOffValve(time_t);

    bool            valveTimerRunning()                         {return m_valveTimerRunning;}
    bool            valveTimerExpired()                         {return (now() > m_timeToTurnOffValve);}
    void            startValveTimer()                           {m_valveTimerRunning = true;}

    bool            valveIsOn(void)                             {return digitalRead(m_valvePin1);}
    void            openValve(void);
    void            closeValve(void);

    int             moisture(void)                              {return analogRead(m_moistureSensorPin);}
    uint16_t        dryThreshold(void)                          {return m_dryThreshold;}
    void            dryThreshold(uint16_t value)                {m_dryThreshold = value;}
    uint16_t        wetThreshold(void)                          {return m_wetThreshold;}
    void            wetThreshold(uint16_t value)                {m_wetThreshold = value;}

    uint8_t         durationToWater_min(void)                   {return m_durationToWater_min;}
    void            durationToWater_min(uint8_t newDur)         {m_durationToWater_min = newDur;}

    void            handleSchedule(void);
    void            printStatus(Stream*);

    ScheduleMode    schedMode(void)                             {return m_schedMode;}
    void            schedMode(ScheduleMode mode)                {m_schedMode = mode;}

    void            setScheduleDOW(time_t);
    void            printScheduleDOW();

    time_t          scheduleTime_afterMidnight(void)            {return m_scheduleTime_afterMidnight;}
    void            scheduleTime_afterMidnight(time_t newTime)  {m_scheduleTime_afterMidnight = newTime;}

    bool*           schedDOWday(void)                           {return m_schedDOWday;}
    void            schedDOWday(bool newDays[7])                {memcpy(m_schedDOWday, newDays, sizeof(m_schedDOWday));}

private:
    char            m_zoneName[20];
    ScheduleMode    m_schedMode;
    bool            m_schedDOWday[7];
    time_t          m_scheduleTime_afterMidnight;
    time_t          m_nextWaterTime;
    time_t          m_lastWaterTime;
    time_t          m_timeToTurnOffValve;
    uint8_t         m_timeBetweenWatering_hr;
    uint8_t         m_durationToWater_min;
    bool            m_valveTimerRunning;
    uint8_t         m_valvePin1;
    uint8_t         m_valvePin2;
    uint8_t         m_moistureSensorPin;
    uint16_t        m_wetThreshold;
    uint16_t        m_dryThreshold;

    void            handleSchedDOW(void);
    void            handleSchedInterval(void);
    void            handleSchedSensor(void);
    void            handleSchedIntervalSensor(void);
};


class GardenManager {
public:
    GardenManager(){}
    void addZone(Zone*);
    void printZoneStatus(Stream*);
    void closeAllValves();
    void openAllValves();
    Zone* valveRunTime(int, int);
    void maintain();
    void saveAllZones();

    Zone*    m_zones[MAX_ZONES];

private:
    uint8_t m_zonesIndex = 0;
};









#endif  //GARDEN_MANAGER_H