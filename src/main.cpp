#include <Maltbie_Menu.h>
#include <Maltbie_Timer.h>

// #include <mcurses.h>

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
Menu MainMenu("**** Welcome to the Teensy Garden Menu ****");

MenuManager SchedMan;
Menu SchedMenu("**** Schedule Manager ****");

struct ZoneSettings
{
    char            name[20];
    uint8_t         schedMode;
    uint8_t         timeBetweenWatering_hr;
    time_t          lastWaterTime;
    uint16_t        dryThresh;
    uint16_t        wetThresh;
    uint8_t         durationToWater_min;
    uint8_t         scheduleHour;
    uint8_t         scheduleMin;
    bool            scheduleDOWday[7];
};
ZoneSettings SchedMenuSettings;

bool exitSchedMenu = false;
uint8_t currentZone = 1;

GardenManager GM;
Zone zone1  (ZONE1_SENSOR_PIN,  ZONE1_SOLENOID_PIN_1,   ZONE1_SOLENOID_PIN_2);
Zone zone2  (ZONE2_SENSOR_PIN,  ZONE2_SOLENOID_PIN_1,   ZONE2_SOLENOID_PIN_2);
Zone zone3  (ZONE3_SENSOR_PIN,  ZONE3_SOLENOID_PIN_1,   ZONE3_SOLENOID_PIN_2);
Zone zone4  (ZONE4_SENSOR_PIN,  ZONE4_SOLENOID_PIN_1,   ZONE4_SOLENOID_PIN_2);

bool debugPrint = false;

Timer_ms    sensorTimer;
uint8_t     menuValve = 0;

time_t  getTeensy3Time();
void    digitalClockDisplayNow(void) {digitalClockDisplay(now());}
void    printGardenStatus(void);
void    setRTC(void);
void    toggleValve(void);
void    clickyValveTest(void);
void    closeAllValves(void);
void    openAllValves(void);
void    setValveRunTime(void);
void    setDryThreshold(void);
void    setWetThreshold(void);
char    inputBuffer[15] = "0-0";

void    handleScheduleMenu(void);
void    updateSchedMenu(void);
void    saveSchedMenu(void);
void    exitScheduleMenu(void)     {exitSchedMenu = true;}
char    schedMenuTime[8] = "12:00PM";
char    schedMenuMode[9] = "NONE";
Timer_ms schedMenuTimeout;

void    timeTest(void);
time_t  lastTime;

bool    funkyLights = false;
bool    PWMtest     = false;


void setup() 
{
    setSyncProvider(getTeensy3Time);
	Serial.begin(115200);
	delay(100);

	MM.setupBLE();
	MM.setupEEPROM();
	MM.registerMenu(&MainMenu);
    MM.startWatchdog();

    SchedMan.registerMenu(&SchedMenu);

    MainMenu.addOption('A', "Print Garden Status", printGardenStatus);
    MainMenu.addOption('B', "Show Debug Prints", &debugPrint);
    MainMenu.addOption('C', "Set Time (HH:MM DD/MM/YYYY)", currentTime, setRTC);
    MainMenu.addOption('D', "Display Time", digitalClockDisplayNow);
    MainMenu.addOption('E', "Clicky Valve Test", clickyValveTest);
    MainMenu.addOption('F', "Time Test", timeTest);
    MainMenu.addOption('G', "Valve Run Time (Valve#-Min)", inputBuffer, setValveRunTime);
    MainMenu.addOption('I', "Funky Lights", &funkyLights);
    MainMenu.addOption('J', "PWM Test", &PWMtest);
    MainMenu.addOption('K', "Set Zone Dry Threshold (Valve#-Value)", inputBuffer, setDryThreshold);
    MainMenu.addOption('L', "Set Zone Wet Threshold (Valve#-Value)", inputBuffer, setWetThreshold);
    MainMenu.addOption('T', "Toggle Valve", &menuValve, 4, 1, toggleValve);
    MainMenu.addOption('+', "Open All Valves", openAllValves);
    MainMenu.addOption('-', "Close All Vavles", closeAllValves);
    MainMenu.addOption('!', "Schedule Menu", handleScheduleMenu);

    SchedMenu.addOption('A', "Current Zone", &currentZone, 4, 1, updateSchedMenu);
    SchedMenu.addOption('B', "Zone Name", SchedMenuSettings.name);
    SchedMenu.addOption('C', "Schedule Mode", &SchedMenuSettings.schedMode, 3, 0);
    SchedMenu.addOption('D', "Dry Threshold", &SchedMenuSettings.dryThresh);
    SchedMenu.addOption('E', "Wet Threshold", &SchedMenuSettings.wetThresh);
    SchedMenu.addOption('F', "Duration To Water", &SchedMenuSettings.durationToWater_min);
    SchedMenu.addOption('G', "Time Between Watering", &SchedMenuSettings.timeBetweenWatering_hr);
    SchedMenu.addOption('I', "Scheduled Time", &schedMenuTime);
    SchedMenu.addOption('+', "Save Zone Settings", saveSchedMenu);
    SchedMenu.addOption('!', "EXIT SCHEDULE MENU", exitScheduleMenu);

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

    GM.m_zones[1]->name(StoreEE.zone2name);
    GM.m_zones[1]->dryThreshold(StoreEE.zone2dryThreshold);
    GM.m_zones[1]->wetThreshold(StoreEE.zone2wetThreshold);
    GM.m_zones[1]->timeBetweenWatering_hr(StoreEE.zone2timeBetweenWatering_hr);
    GM.m_zones[1]->durationToWater_min(StoreEE.zone2durationToWater_min);
    GM.m_zones[1]->schedMode(intToSchedMode(StoreEE.zone2scheduleMode));
    GM.m_zones[1]->lastWaterTime((time_t)StoreEE.zone2lastWaterTime);

    GM.m_zones[2]->name(StoreEE.zone3name);
    GM.m_zones[2]->dryThreshold(StoreEE.zone3dryThreshold);
    GM.m_zones[2]->wetThreshold(StoreEE.zone3wetThreshold);
    GM.m_zones[2]->timeBetweenWatering_hr(StoreEE.zone3timeBetweenWatering_hr);
    GM.m_zones[2]->durationToWater_min(StoreEE.zone3durationToWater_min);
    GM.m_zones[2]->schedMode(intToSchedMode(StoreEE.zone3scheduleMode));
    GM.m_zones[2]->lastWaterTime((time_t)StoreEE.zone3lastWaterTime);

    GM.m_zones[3]->name(StoreEE.zone4name);
    GM.m_zones[3]->dryThreshold(StoreEE.zone4dryThreshold);
    GM.m_zones[3]->wetThreshold(StoreEE.zone4wetThreshold);
    GM.m_zones[3]->timeBetweenWatering_hr(StoreEE.zone4timeBetweenWatering_hr);
    GM.m_zones[3]->durationToWater_min(StoreEE.zone4durationToWater_min);
    GM.m_zones[3]->schedMode(intToSchedMode(StoreEE.zone4scheduleMode));
    GM.m_zones[3]->lastWaterTime((time_t)StoreEE.zone4lastWaterTime);

    MM.printHelp(&Serial, true);

    WATCHDOG_RESET

    digitalWrite(2, LOW);
    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
    digitalWrite(6, LOW);
    digitalWrite(7, LOW);
    digitalWrite(8, LOW);
    digitalWrite(9, LOW);

    lastTime = now();

    memset(inputBuffer, '\0', sizeof(inputBuffer));

    updateSchedMenu();
}

void loop(){
	WATCHDOG_RESET
	MM.handleLaptopInput();
    GM.maintain();

    // if (!sensorTimer.isActive())
    // {
    //     MH.serPtr()->print("Moisture Level: ");
    //     MH.serPtr()->println(zone1.moisture());
    //     sensorTimer.Start(1000);
    // }
    if (funkyLights)
    {
        static int curSec = 0;
        if (second(now()) != curSec){
            curSec = second(now());
            if (second(now()) % 2 > 0) digitalWrite(2, !digitalRead(2));
            if (second(now()) % 3 > 0) digitalWrite(3, !digitalRead(3));
            if (second(now()) % 4 > 0) digitalWrite(4, !digitalRead(4));
            if (second(now()) % 5 > 0) digitalWrite(5, !digitalRead(5));
            if (second(now()) % 6 > 0) digitalWrite(6, !digitalRead(6));
            if (second(now()) % 7 > 0) digitalWrite(7, !digitalRead(7));
            if (second(now()) % 8 > 0) digitalWrite(8, !digitalRead(8));
            if (second(now()) % 9 > 0) digitalWrite(9, !digitalRead(9));
        }
    }

    if (PWMtest)
    {
        static int pwmSec = 0;
        analogWrite(2, pwmSec % 255);
        pwmSec++;
        pinMode(2, OUTPUT);
    }

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
    digitalClockDisplayNow();
    MH.serPtr()->println();
}

void printGardenStatus()
{
    GM.printZoneStatus(MH.serPtr());
}

void toggleValve()
{
    switch (menuValve)
    {
        case 1:
            zone1.valveIsOn() ? zone1.closeValve() : zone1.openValve();
            MH.serPtr()->print("Zucchini Zone Valve is now ");
            zone1.valveIsOn() ? MH.serPtr()->println("ON") : MH.serPtr()->println("OFF");
        break;

        case 2:
            zone2.valveIsOn() ? zone2.closeValve() : zone2.openValve();
            MH.serPtr()->print("Pepper Zone Valve is now ");
            zone2.valveIsOn() ? MH.serPtr()->println("ON") : MH.serPtr()->println("OFF");
        break;

        case 3:
            zone3.valveIsOn() ? zone3.closeValve() : zone3.openValve();
            MH.serPtr()->print("Cucumber Zone Valve is now ");
            zone3.valveIsOn() ? MH.serPtr()->println("ON") : MH.serPtr()->println("OFF");
        break;

        case 4:
            zone4.valveIsOn() ? zone4.closeValve() : zone4.openValve();
            MH.serPtr()->print("Melon Zone Valve is now ");
            zone4.valveIsOn() ? MH.serPtr()->println("ON") : MH.serPtr()->println("OFF");
        break;

        default:
        break;
    }
    menuValve = 0;
}

void clickyValveTest()
{
    zone1.openValve();
    delay(500);
    zone1.closeValve();
    delay(500);
    zone2.openValve();
    delay(500);
    zone2.closeValve();
    delay(500);
    zone3.openValve();
    delay(500);
    zone3.closeValve();
    delay(500);
    zone4.openValve();
    delay(500);
    zone4.closeValve();
    delay(500);
}

void closeAllValves()
{
    GM.closeAllValves();
}

void openAllValves()
{
    GM.openAllValves();
}

void timeTest()
{
    MH.serPtr()->print("Raw Now = ");
    MH.serPtr()->println(now());
    MH.serPtr()->print("Now - Last = ");
    MH.serPtr()->println(now() - lastTime);
    MH.serPtr()->print("Seconds Since Last = ");
    MH.serPtr()->println(numberOfSeconds(now() - lastTime));
    MH.serPtr()->print("Minutes Since Last = ");
    MH.serPtr()->println(numberOfMinutes(now() - lastTime));
    MH.serPtr()->print("Hours Since Last = ");
    MH.serPtr()->println(numberOfHours(now() - lastTime));
    MH.serPtr()->print("Now = ");
    digitalClockDisplayNow();
    MH.serPtr()->println();
    MH.serPtr()->print("In 1.5 Hours = ");
    digitalClockDisplay(now() + (SECS_PER_HOUR * 1.5));
    MH.serPtr()->println();
    MH.serPtr()->print("Time @ \'0\' = ");
    digitalClockDisplay((time_t)0);
    MH.serPtr()->println();
    MH.serPtr()->print("Previous Midnight = ");
    digitalClockDisplay(previousMidnight(now()));
    MH.serPtr()->println();
    MH.serPtr()->print("8PM Today = ");
    digitalClockDisplay(previousMidnight(now()) + (SECS_PER_HOUR * 20));
    MH.serPtr()->println();
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
    MH.serPtr()->print(tempZone->name());
    MH.serPtr()->print(" valve will turn off at ");
    digitalClockDisplay(tempZone->timeToTurnOffValve());
    MH.serPtr()->println();
    memset(inputBuffer, '\0', sizeof(inputBuffer));
}

void setDryThreshold()
{
    int zone = inputBuffer[0] - '0' - 1;
    if (zone < 0 || zone > 3)
    {
        MH.serPtr()->print("Invalid Zone: ");
        MH.serPtr()->println(zone + 1);
        memset(inputBuffer, '\0', sizeof(inputBuffer));
        return;
    }

    if (inputBuffer[1] != '-' && inputBuffer[1] != '\0')
    {
        MH.serPtr()->println("Format must be \"Valve#-Value\"(including the dash)");
        memset(inputBuffer, '\0', sizeof(inputBuffer));
        return;
    }

    int newThresh;
    if (inputBuffer[1] == '\0')
    {
        int moistureRead = GM.m_zones[zone]->moisture();
        GM.m_zones[zone]->dryThreshold(moistureRead);
        MH.serPtr()->print(GM.m_zones[zone]->name());
        MH.serPtr()->print(" - New Dry Threshold: ");
        MH.serPtr()->println(moistureRead);
        memset(inputBuffer, '\0', sizeof(inputBuffer));
        newThresh = moistureRead;
    }
    else
    {
        char buffer[15];
        strcpy(buffer, &inputBuffer[2]);
        newThresh = atoi(buffer);
        GM.m_zones[zone]->dryThreshold(newThresh);
        MH.serPtr()->print(GM.m_zones[zone]->name());
        MH.serPtr()->print(" - New Dry Threshold: ");
        MH.serPtr()->println(newThresh);
        memset(inputBuffer, '\0', sizeof(inputBuffer));
    }

    switch (zone)
    {
        case 0:
            StoreEE.zone1dryThreshold = newThresh;
            break;
        
        case 1:
            StoreEE.zone2dryThreshold = newThresh;
            break;

        case 2:
            StoreEE.zone3dryThreshold = newThresh;
            break;
        case 3:

            StoreEE.zone4dryThreshold = newThresh;
            break;

        default:
            break;
    }
}

void setWetThreshold()
{
    int zone = inputBuffer[0] - '0' - 1;
    if (zone < 0 || zone > 3)
    {
        MH.serPtr()->print("Invalid Zone: ");
        MH.serPtr()->println(zone + 1);
        memset(inputBuffer, '\0', sizeof(inputBuffer));
        return;
    }

    if (inputBuffer[1] != '-' && inputBuffer[1] != '\0')
    {
        MH.serPtr()->println("Format must be \"Valve#-Value\"(including the dash)");
        memset(inputBuffer, '\0', sizeof(inputBuffer));
        return;
    }

    int newThresh;
    if (inputBuffer[1] == '\0')
    {
        int moistureRead = GM.m_zones[zone]->moisture();
        GM.m_zones[zone]->wetThreshold(moistureRead);
        MH.serPtr()->print(GM.m_zones[zone]->name());
        MH.serPtr()->print(" - New Wet Threshold: ");
        MH.serPtr()->println(moistureRead);
        memset(inputBuffer, '\0', sizeof(inputBuffer));
        newThresh = moistureRead;
    }
    else
    {
        char buffer[15];
        strcpy(buffer, &inputBuffer[2]);
        newThresh = atoi(buffer);
        GM.m_zones[zone]->wetThreshold(newThresh);
        MH.serPtr()->print(GM.m_zones[zone]->name());
        MH.serPtr()->print(" - New Wet Threshold: ");
        MH.serPtr()->println(newThresh);
        memset(inputBuffer, '\0', sizeof(inputBuffer));
    }

    switch (zone)
    {
        case 0:
            StoreEE.zone1wetThreshold = newThresh;
            break;
        
        case 1:
            StoreEE.zone2wetThreshold = newThresh;
            break;

        case 2:
            StoreEE.zone3wetThreshold = newThresh;
            break;
        case 3:

            StoreEE.zone4wetThreshold = newThresh;
            break;

        default:
            break;
    }
}

void handleScheduleMenu()
{
    schedMenuTimeout.Start(600000);
    SchedMan.printMenu();
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
    SchedMenuSettings.scheduleHour = GM.m_zones[currentZone - 1]->schedDOWhour();
    SchedMenuSettings.scheduleMin = GM.m_zones[currentZone - 1]->schedDOWmin();
    SchedMenuSettings.lastWaterTime = GM.m_zones[currentZone - 1]->lastWaterTime();
    memcpy(SchedMenuSettings.scheduleDOWday, GM.m_zones[currentZone - 1]->schedDOWday(), sizeof(SchedMenuSettings.scheduleDOWday));

    int tempHour = SchedMenuSettings.scheduleHour;
    if (tempHour > 12) tempHour = tempHour-12;
    if (tempHour == 0) tempHour = 12;
    itoa(tempHour, schedMenuTime, DEC);
    if (schedMenuTime[1] == '\0') {schedMenuTime[1] = schedMenuTime[0]; schedMenuTime[0] = '0';}
    schedMenuTime[2] = ':';
    itoa(SchedMenuSettings.scheduleMin, &schedMenuTime[3], DEC);
    if (schedMenuTime[4] == '\0') {schedMenuTime[4] = schedMenuTime[3]; schedMenuTime[3] = '0';}
    SchedMenuSettings.scheduleHour >= 12 ? schedMenuTime[5] = 'P' : schedMenuTime[5] = 'A';
    schedMenuTime[6] = 'M';
    Serial.println(schedMenuTime);
    Serial.println();

    SchedMan.printMenu();
}

void saveSchedMenu()
{
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
            break;
        }
        default:
            MH.serPtr()->println("SOMETHING WENT WRONG!");
            return;
    }

    GM.m_zones[currentZone - 1]->name(SchedMenuSettings.name);
    GM.m_zones[currentZone - 1]->scheduleMode((ScheduleMode)SchedMenuSettings.schedMode);
    GM.m_zones[currentZone - 1]->dryThreshold(SchedMenuSettings.dryThresh);
    GM.m_zones[currentZone - 1]->wetThreshold(SchedMenuSettings.wetThresh);
    GM.m_zones[currentZone - 1]->schedDOWhour(SchedMenuSettings.scheduleHour);
    GM.m_zones[currentZone - 1]->schedDOWmin(SchedMenuSettings.scheduleMin);
    GM.m_zones[currentZone - 1]->durationToWater_min(SchedMenuSettings.durationToWater_min);
    GM.m_zones[currentZone - 1]->timeBetweenWatering_hr(SchedMenuSettings.timeBetweenWatering_hr);
    GM.m_zones[currentZone - 1]->lastWaterTime(SchedMenuSettings.lastWaterTime);

    EEPROM.put(OFFSET_STOREEE, StoreEE);
    MH.serPtr()->print("Settings saved for ");
    MH.serPtr()->println(SchedMenuSettings.name);
}