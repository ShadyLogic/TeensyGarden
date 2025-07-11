/*
 * main.cpp
 * 
 * Teensy Garden Irrigation System
 * 
 * Description:
 * This is the entry point for the Teensy 4.1-based garden irrigation controller.
 * It initializes menus, sets up real-time clock syncing, loads saved zone configurations,
 * and enters the main control loop that maintains irrigation zones and handles user interaction.
 *
 * Features:
 * - BLE/Serial menu interface
 * - RTC synchronization
 * - EEPROM loading/saving of zone settings
 * - Manual and automatic valve control
 * - Soil moisture monitoring and scheduling
 * 
 * Author: Jacob Rogers
 * Date: May 2025
 */


#include <Arduino.h>
#include <Maltbie_Menu.h>
#include <Maltbie_Timer.h>
#include <TimeLib.h>
#include <GardenManager.h>

#define ZONE1_SOLENOID_PIN_1    2
#define ZONE1_SOLENOID_PIN_2    3
#define ZONE2_SOLENOID_PIN_1    4
#define ZONE2_SOLENOID_PIN_2    5
#define ZONE3_SOLENOID_PIN_1    6
#define ZONE3_SOLENOID_PIN_2    7
#define ZONE4_SOLENOID_PIN_1    8
#define ZONE4_SOLENOID_PIN_2    9

#define ZONE1_SENSOR_PIN        20
#define ZONE2_SENSOR_PIN        21
#define ZONE3_SENSOR_PIN        22
#define ZONE4_SENSOR_PIN        23

#define TIME_HEADER  "T"   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message
char currentTime[20] = "10:00 05/15/2025";

Maltbie_Helper MH;

MenuManager MM;
Menu MainMenu("**** TEENSY GARDEN MENU ****");

MenuManager SchedMan;
Menu SchedMenu("**** SCHEDULE MANAGER ****");

struct ZoneSettings
{
    char            name[MAX_CHAR];
    uint8_t         schedMode;
    uint8_t         timeBetweenWatering_hr;
    time_t          lastWaterTime;
    uint16_t        dryThresh;
    uint16_t        wetThresh;
    uint8_t         durationToWater_min;
    time_t          scheduleTime_afterMidnight;
    bool            scheduleDOWday[7];
};
ZoneSettings SchedMenuSettings;

GardenManager GM;
Zone        zone1   (ZONE1_SENSOR_PIN,  ZONE1_SOLENOID_PIN_1,   ZONE1_SOLENOID_PIN_2);
Zone        zone2   (ZONE2_SENSOR_PIN,  ZONE2_SOLENOID_PIN_1,   ZONE2_SOLENOID_PIN_2);
Zone        zone3   (ZONE3_SENSOR_PIN,  ZONE3_SOLENOID_PIN_1,   ZONE3_SOLENOID_PIN_2);
Zone        zone4   (ZONE4_SENSOR_PIN,  ZONE4_SOLENOID_PIN_1,   ZONE4_SOLENOID_PIN_2);

time_t      getTeensy3Time();
void        digitalClockDisplayNow(void)    {MH.serPtr()->print(timeAndDate(now()));}
void        printGardenStatus(void);
void        setRTC(void);
void        toggleValve(void);
void        closeAllValves(void);
void        openAllValves(void);
void        setValveRunTime(void);
void        saveAllZoneInfo();
char        inputBuffer[15] = "0-0";
Timer_ms    sensorTimer;
uint8_t     menuValve = 0;

bool        exitSchedMenu = false;
void        handleScheduleMenu(void);
void        updateSchedMenu(void);
void        saveSchedMenu(void);
void        exitScheduleMenu(void)          {exitSchedMenu = true;}
char        schedMenuTime[8] = "12:00PM";
char        schedMenuMode[9] = "NONE";
Timer_ms    schedMenuTimeout;
uint8_t     currentZone = 1;


void setup() 
{
    setSyncProvider(getTeensy3Time);
	Serial.begin(115200);
	delay(100);

	MM.setupBLE();
	MM.setupEEPROM();
	MM.registerMenu(&MainMenu);
    MM.startWatchdog();

    MH.serPtr()->print("\nInitializing SD Card...");
    if (!SD.begin(BUILTIN_SDCARD)) 
    {
        MH.serPtr()->println("Card failed, or not present\n");
        SDworking = false;
    }
    else
    {
        MH.serPtr()->println("Card initialized\n");
        SDworking = true;
    }

    SchedMan.registerMenu(&SchedMenu);

    MainMenu.addOption('A', "Print Garden Status", printGardenStatus);
    MainMenu.addOption('B', "Set Time (HH:MM DD/MM/YYYY)", currentTime, setRTC);
    MainMenu.addOption('D', "Display Time", digitalClockDisplayNow);
    MainMenu.addOption('G', "Valve Run Timer (Valve#-Min)", inputBuffer, setValveRunTime);
    MainMenu.addOption('T', "Toggle Valve", &menuValve, 4, 1, toggleValve);
    MainMenu.addOption('+', "Open All Valves", openAllValves);
    MainMenu.addOption('-', "Close All Vavles", closeAllValves);
    MainMenu.addOption('L', "Print Log Data", printLog);
    MainMenu.addOption('K', "Clear Log Data", clearLog);
    MainMenu.addOption('*', "Save All Zone Info", saveAllZoneInfo);
    MainMenu.addOption('!', "SCHEDULE MENU", handleScheduleMenu);
    
    SchedMenu.addOption('A', "Current Zone", &currentZone, 4, 1, updateSchedMenu);
    SchedMenu.addOption('B', "Zone Name", SchedMenuSettings.name);
    SchedMenu.addOption('C', "Schedule Mode", &SchedMenuSettings.schedMode, 4, 0);
    SchedMenu.addOption('D', "Dry Threshold", &SchedMenuSettings.dryThresh, 1023, 0);
    SchedMenu.addOption('E', "Wet Threshold", &SchedMenuSettings.wetThresh, 1023, 0);
    SchedMenu.addOption('F', "Duration To Water", &SchedMenuSettings.durationToWater_min, 255, 0);
    SchedMenu.addOption('G', "Time Between Watering", &SchedMenuSettings.timeBetweenWatering_hr, 255, 0);
    SchedMenu.addOption('I', "Scheduled Time", schedMenuTime);
    SchedMenu.addOption('+', "Save Zone Settings", saveSchedMenu);
    SchedMenu.addOption('!', "MAIN MENU", exitScheduleMenu);

    GM.addZone(&zone1);
    GM.addZone(&zone2);
    GM.addZone(&zone3);
    GM.addZone(&zone4);

    if (StoreEE.zone1lastWaterTime == 0) StoreEE.zone1lastWaterTime = now();
    if (StoreEE.zone2lastWaterTime == 0) StoreEE.zone2lastWaterTime = now();
    if (StoreEE.zone3lastWaterTime == 0) StoreEE.zone3lastWaterTime = now();
    if (StoreEE.zone4lastWaterTime == 0) StoreEE.zone4lastWaterTime = now();

    GM.m_zones[0]->name(StoreEE.zone1name);
    GM.m_zones[0]->dryThreshold(StoreEE.zone1dryThreshold);
    GM.m_zones[0]->wetThreshold(StoreEE.zone1wetThreshold);
    GM.m_zones[0]->timeBetweenWatering_hr(StoreEE.zone1timeBetweenWatering_hr);
    GM.m_zones[0]->durationToWater_min(StoreEE.zone1durationToWater_min);
    GM.m_zones[0]->schedMode(intToSchedMode(StoreEE.zone1scheduleMode));
    GM.m_zones[0]->lastWaterTime((time_t)StoreEE.zone1lastWaterTime);
    GM.m_zones[0]->scheduleTime_afterMidnight((time_t)StoreEE.zone1scheduleTime_afterMidnight);

    GM.m_zones[1]->name(StoreEE.zone2name);
    GM.m_zones[1]->dryThreshold(StoreEE.zone2dryThreshold);
    GM.m_zones[1]->wetThreshold(StoreEE.zone2wetThreshold);
    GM.m_zones[1]->timeBetweenWatering_hr(StoreEE.zone2timeBetweenWatering_hr);
    GM.m_zones[1]->durationToWater_min(StoreEE.zone2durationToWater_min);
    GM.m_zones[1]->schedMode(intToSchedMode(StoreEE.zone2scheduleMode));
    GM.m_zones[1]->lastWaterTime((time_t)StoreEE.zone2lastWaterTime);
    GM.m_zones[1]->scheduleTime_afterMidnight((time_t)StoreEE.zone2scheduleTime_afterMidnight);

    GM.m_zones[2]->name(StoreEE.zone3name);
    GM.m_zones[2]->dryThreshold(StoreEE.zone3dryThreshold);
    GM.m_zones[2]->wetThreshold(StoreEE.zone3wetThreshold);
    GM.m_zones[2]->timeBetweenWatering_hr(StoreEE.zone3timeBetweenWatering_hr);
    GM.m_zones[2]->durationToWater_min(StoreEE.zone3durationToWater_min);
    GM.m_zones[2]->schedMode(intToSchedMode(StoreEE.zone3scheduleMode));
    GM.m_zones[2]->lastWaterTime((time_t)StoreEE.zone3lastWaterTime);
    GM.m_zones[2]->scheduleTime_afterMidnight((time_t)StoreEE.zone3scheduleTime_afterMidnight);

    GM.m_zones[3]->name(StoreEE.zone4name);
    GM.m_zones[3]->dryThreshold(StoreEE.zone4dryThreshold);
    GM.m_zones[3]->wetThreshold(StoreEE.zone4wetThreshold);
    GM.m_zones[3]->timeBetweenWatering_hr(StoreEE.zone4timeBetweenWatering_hr);
    GM.m_zones[3]->durationToWater_min(StoreEE.zone4durationToWater_min);
    GM.m_zones[3]->schedMode(intToSchedMode(StoreEE.zone4scheduleMode));
    GM.m_zones[3]->lastWaterTime((time_t)StoreEE.zone4lastWaterTime);
    GM.m_zones[3]->scheduleTime_afterMidnight((time_t)StoreEE.zone4scheduleTime_afterMidnight);

    digitalWrite(2, LOW);
    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
    digitalWrite(6, LOW);
    digitalWrite(7, LOW);
    digitalWrite(8, LOW);
    digitalWrite(9, LOW);

    memset(inputBuffer, '\0', sizeof(inputBuffer));

    MM.printHelp(&Serial, true);
}

void loop(){
	WATCHDOG_RESET
	MM.handleLaptopInput();
    GM.maintain();

    delay(10);
}

time_t getTeensy3Time()
{
    return Teensy3Clock.get();
}

void setRTC()
{
    char hour[2]    = {currentTime[0], currentTime[1]};
    char min[2]     = {currentTime[3], currentTime[4]};
    char mon[2]     = {currentTime[6], currentTime[7]};
    char day[2]     = {currentTime[9], currentTime[10]};
    char year[4]    = {currentTime[12], currentTime[13], currentTime[14], currentTime[15]};

    setTime(atoi(hour),atoi(min),0,atoi(day),atoi(mon),atoi(year));
    Teensy3Clock.set(now());
    setTime(now());

    MH.serPtr()->print("NEW TIME: ");
    MH.serPtr()->println(timeAndDate(now()));
}

void printGardenStatus()
{
    GM.printZoneStatus(MH.serPtr());
}

void toggleValve()
{
    GM.m_zones[menuValve-1]->valveIsOn() ? GM.m_zones[menuValve-1]->closeValve() : GM.m_zones[menuValve-1]->openValve();
    String temp;
    temp += String(GM.m_zones[menuValve-1]->name());
    temp += (" valve is now ");
    GM.m_zones[menuValve-1]->valveIsOn() ? temp += ("OPEN") : temp += ("CLOSED");
    logData(temp);
    menuValve = 0;
}

void closeAllValves()
{
    GM.closeAllValves();
    logData("*** ALL VALVES CLOSED ***");
}

void openAllValves()
{
    GM.openAllValves();
    logData("*** ALL VALVES OPEN ***");
}

void setValveRunTime()
{
    if (inputBuffer[1] != '-' || inputBuffer[2] == '\0')
    {
        MH.serPtr()->println("Format must be \"Valve#-Time\"(including the dash)");
        return;
    }
    int zone = inputBuffer[0] - '0';
    char buffer[15];
    strcpy(buffer, &inputBuffer[2]);
    int runTime = atoi(buffer);
    Zone* tempZone = GM.valveRunTime(zone-1, runTime);
    String temp;
    temp += String(tempZone->name());
    temp += (" valve will close at ");
    temp += String(timeToString(tempZone->timeToTurnOffValve()));
    logData(temp);
    memset(inputBuffer, '\0', sizeof(inputBuffer));
}

void handleScheduleMenu()
{
    schedMenuTimeout.Start(600000);
    updateSchedMenu();
    while (exitSchedMenu == false)
    {
        WATCHDOG_RESET
        SchedMan.handleLaptopInput();
        if (!schedMenuTimeout.isActive()) break;
        delay(10);
    }
    exitSchedMenu = false;
    MM.printMenu();
}

void updateSchedMenu()
{
    strcpy(SchedMenuSettings.name, GM.m_zones[currentZone - 1]->name());
    SchedMenuSettings.schedMode = (uint8_t)GM.m_zones[currentZone - 1]->scheduleMode();
    SchedMenuSettings.timeBetweenWatering_hr = GM.m_zones[currentZone - 1]->timeBetweenWatering_hr();
    SchedMenuSettings.dryThresh = GM.m_zones[currentZone - 1]->dryThreshold();
    SchedMenuSettings.wetThresh = GM.m_zones[currentZone - 1]->wetThreshold();
    SchedMenuSettings.durationToWater_min = GM.m_zones[currentZone - 1]->durationToWater_min();
    SchedMenuSettings.scheduleTime_afterMidnight = GM.m_zones[currentZone - 1]->scheduleTime_afterMidnight();
    SchedMenuSettings.lastWaterTime = GM.m_zones[currentZone - 1]->lastWaterTime();
    SchedMenuSettings.scheduleTime_afterMidnight = GM.m_zones[currentZone - 1]->scheduleTime_afterMidnight();
    memcpy(SchedMenuSettings.scheduleDOWday, GM.m_zones[currentZone - 1]->schedDOWday(), sizeof(SchedMenuSettings.scheduleDOWday));

    timeToString(SchedMenuSettings.scheduleTime_afterMidnight).toCharArray(schedMenuTime, sizeof(schedMenuTime));

    MH.serPtr()->println("\n");
    SchedMan.printMenu();
}

void saveSchedMenu()
{
    SchedMenuSettings.scheduleTime_afterMidnight = arrayToTime(schedMenuTime);
    
    switch (currentZone)
    {
        case 1:
        {
            strcpy(StoreEE.zone1name, SchedMenuSettings.name);
            StoreEE.zone1dryThreshold = SchedMenuSettings.dryThresh;
            StoreEE.zone1wetThreshold = SchedMenuSettings.wetThresh;
            StoreEE.zone1timeBetweenWatering_hr = SchedMenuSettings.timeBetweenWatering_hr;
            StoreEE.zone1durationToWater_min = SchedMenuSettings.durationToWater_min;
            StoreEE.zone1scheduleMode = SchedMenuSettings.schedMode;
            StoreEE.zone1lastWaterTime = SchedMenuSettings.lastWaterTime;
            StoreEE.zone1scheduleTime_afterMidnight = (double)SchedMenuSettings.scheduleTime_afterMidnight;
            break;
        }
        case 2:
        {
            strcpy(StoreEE.zone2name, SchedMenuSettings.name);
            StoreEE.zone2dryThreshold = SchedMenuSettings.dryThresh;
            StoreEE.zone2wetThreshold = SchedMenuSettings.wetThresh;
            StoreEE.zone2timeBetweenWatering_hr = SchedMenuSettings.timeBetweenWatering_hr;
            StoreEE.zone2durationToWater_min = SchedMenuSettings.durationToWater_min;
            StoreEE.zone2scheduleMode = SchedMenuSettings.schedMode;
            StoreEE.zone2lastWaterTime = SchedMenuSettings.lastWaterTime;
            StoreEE.zone2scheduleTime_afterMidnight = (double)SchedMenuSettings.scheduleTime_afterMidnight;
            break;
        }
        case 3:
        {
            strcpy(StoreEE.zone3name, SchedMenuSettings.name);
            StoreEE.zone3dryThreshold = SchedMenuSettings.dryThresh;
            StoreEE.zone3wetThreshold = SchedMenuSettings.wetThresh;
            StoreEE.zone3timeBetweenWatering_hr = SchedMenuSettings.timeBetweenWatering_hr;
            StoreEE.zone3durationToWater_min = SchedMenuSettings.durationToWater_min;
            StoreEE.zone3scheduleMode = SchedMenuSettings.schedMode;
            StoreEE.zone3lastWaterTime = SchedMenuSettings.lastWaterTime;
            StoreEE.zone3scheduleTime_afterMidnight = (double)SchedMenuSettings.scheduleTime_afterMidnight;
            break;
        }
        case 4:
        {
            strcpy(StoreEE.zone4name, SchedMenuSettings.name);
            StoreEE.zone4dryThreshold = SchedMenuSettings.dryThresh;
            StoreEE.zone4wetThreshold = SchedMenuSettings.wetThresh;
            StoreEE.zone4timeBetweenWatering_hr = SchedMenuSettings.timeBetweenWatering_hr;
            StoreEE.zone4durationToWater_min = SchedMenuSettings.durationToWater_min;
            StoreEE.zone4scheduleMode = SchedMenuSettings.schedMode;
            StoreEE.zone4lastWaterTime = SchedMenuSettings.lastWaterTime;
            StoreEE.zone4scheduleTime_afterMidnight = (double)SchedMenuSettings.scheduleTime_afterMidnight;
            break;
        }
        default:
            MH.serPtr()->println("SOMETHING WENT WRONG!");
            return;
    }

    GM.m_zones[currentZone - 1]->name(SchedMenuSettings.name);
    GM.m_zones[currentZone - 1]->dryThreshold(SchedMenuSettings.dryThresh);
    GM.m_zones[currentZone - 1]->wetThreshold(SchedMenuSettings.wetThresh);
    GM.m_zones[currentZone - 1]->timeBetweenWatering_hr(SchedMenuSettings.timeBetweenWatering_hr);
    GM.m_zones[currentZone - 1]->durationToWater_min(SchedMenuSettings.durationToWater_min);
    GM.m_zones[currentZone - 1]->scheduleMode((ScheduleMode)SchedMenuSettings.schedMode);
    GM.m_zones[currentZone - 1]->lastWaterTime(SchedMenuSettings.lastWaterTime);
    GM.m_zones[currentZone - 1]->scheduleTime_afterMidnight(SchedMenuSettings.scheduleTime_afterMidnight);

    delay(100);

    EEPROM.put(OFFSET_STOREEE, StoreEE);
    MH.serPtr()->print("Settings saved for ");
    MH.serPtr()->println(SchedMenuSettings.name);
}

void saveAllZoneInfo()
{
    GM.saveAllZones();
}

