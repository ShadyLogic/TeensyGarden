// Garden Manager
// Jacob Rogers

#ifndef GARDEN_MANAGER_H
#define GARDEN_MANAGER_H

#define MAX_ZONES   4

#include <Arduino.h>
#include <Maltbie_Helper.h>
#include <Menu_Config.h>
#include <TimeLib.h>

enum ScheduleType
{
    NONE,
    DOW,
    INTERVAL,
    SENSOR
};


String numberToDay(int);



class Zone {
public:
    Zone(){}
    Zone(const char[20], uint8_t, uint8_t, uint8_t = 200);

    const char*     name(void)                          {return m_zoneName;}
    void            name(char* newName)                 {strcpy(m_zoneName, newName);}

    ScheduleType    scheduleMode()                      {return m_schedType;}
    void            scheduleMode(ScheduleType st)       {m_schedType = st;}

    time_t          lastWaterTime(void)                 {return m_lastWaterTime;}
    void            lastWaterTime(time_t newTime)       {m_lastWaterTime = newTime;}

    time_t          timeToTurnOffValve()                {return m_timeToTurnOffValve;}
    void            timeToTurnOffValve(time_t);

    bool            valveTimerRunning()                 {return m_valveTimerRunning;}
    bool            valveTimerExpired()                 {return (now() > m_timeToTurnOffValve);}
    void            startValveTimer()                   {m_valveTimerRunning = true;}

    bool            valveIsOn(void)                     {return digitalRead(m_valvePin1);}
    void            openValve(void);
    void            closeValve(void);

    int             moisture(void)                      {return analogRead(m_moistureSensorPin);}
    void            setDryThreshold(uint16_t value)     {m_dryThreshold = value;}
    void            setWetThreshold(uint16_t value)     {m_wetThreshold = value;}

    void            handleSchedule(void);
    void            printStatus(Stream*);

    void            setScheduleDOW(time_t);
    void            printScheduleDOW();

private:
    char            m_zoneName[20];
    ScheduleType    m_schedType;
    bool            m_schedDOWday[7];
    uint8_t         m_schedDOWhour;
    uint8_t         m_schedDOWmin;
    time_t          m_nextWaterTime;
    time_t          m_lastWaterTime;
    time_t          m_timeToTurnOffValve;
    time_t          m_minTimeBetweenWater;
    time_t          m_scheduledRuntime;
    bool            m_valveTimerRunning;
    uint8_t         m_valvePin1;
    uint8_t         m_valvePin2;
    uint8_t         m_moistureSensorPin;
    uint16_t        m_wetThreshold;
    uint16_t        m_dryThreshold;

    void            handleSchedDOW(void);
    void            handleSchedInterval(void);
    void            handleSchedSensor(void);
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

    Zone*    m_zones[MAX_ZONES];

private:
    uint8_t m_zonesIndex = 0;
};









#endif  //GARDEN_MANAGER_H