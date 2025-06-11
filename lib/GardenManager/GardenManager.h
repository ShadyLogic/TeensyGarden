// Garden Manager
// Jacob Rogers

#ifndef GARDEN_MANAGER_H
#define GARDEN_MANAGER_H

#define MAX_ZONES   4

#include <Arduino.h>
#include <Maltbie_Helper.h>
#include <TimeLib.h>

enum ScheduleType
{
    NONE,
    DOW,
    INTERVAL,
    SENSOR
};



class Zone {
public:
    Zone(){}
    Zone(const char[20], uint8_t, uint8_t, uint8_t = 200);
    uint16_t    moisture(void);
    const char* name(void) {return m_zoneName;}
    void        openValve(void);
    void        closeValve(void);
    bool        valveIsOn(void)         {return digitalRead(m_valvePin1);}
    void        logLastWaterTime(void)  {m_lastWaterTime = now();}
    void        handleSchedule(void);
    void        printStatus(Stream*);
    void        setTimeToTurnOffValve(time_t);
    time_t      timeToTurnOffValve()    {return m_timeToTurnOffValve;}
    void        startValveTimer()       {m_valveTimerRunning = true;}
    bool        valveTimerExpired()     {return (now() > m_timeToTurnOffValve);}
    bool        valveTimerRunning()     {return m_valveTimerRunning;}

private:
    char            m_zoneName[20];
    uint8_t         m_moistureSensorPin;
    uint8_t         m_valvePin1;
    uint8_t         m_valvePin2;
    uint16_t        m_wetThreshold;
    uint16_t        m_dryThreshold;
    ScheduleType    m_schedType;
    time_t          m_lastWaterTime;
    time_t          m_minTimeBetweenWater;
    time_t          m_timeToTurnOffValve;
    bool            m_valveTimerRunning;

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
    void maintain();

private:
    uint8_t m_zonesIndex = 0;
    Zone*    m_zones[MAX_ZONES];
};









#endif  //GARDEN_MANAGER_H