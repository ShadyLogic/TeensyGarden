// Garden Manager
// Jacob Rogers

#ifndef GARDEN_MANAGER_H
#define GARDEN_MANAGER_H

#define MAX_ZONES   4

#include <Arduino.h>
#include <Maltbie_Helper.h>
#include <TimeLib.h>

class Zone {
public:
    Zone(){}
    Zone(const char[20], uint8_t, uint8_t);
    uint16_t moisture(void);
    const char* name(void) {return m_zoneName;}

private:
    char    m_zoneName[20];
    uint8_t m_moistureSensorPin;
    uint8_t m_valvePin;
    time_t  m_lastWaterTime;
};


class GardenManager {
public:
    GardenManager(){}
    void addZone(Zone);
    void printZoneInfo(Stream*);

private:
    uint8_t m_zonesIndex = 0;
    Zone    m_zones[MAX_ZONES];
};









#endif  //GARDEN_MANAGER_H