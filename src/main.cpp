#include <Maltbie_Menu.h>
#include <Maltbie_Timer.h>

// #include <mcurses.h>

#include <TimeLib.h>

#include <GardenManager.h>

#define ZUCCHINI_SOLENOID_PIN_1     2
#define ZUCCHINI_SOLENOID_PIN_2     3
#define PEPPER_SOLENOID_PIN_1       4
#define PEPPER_SOLENOID_PIN_2       5
#define CUCUMBER_SOLENOID_PIN_1     6
#define CUCUMBER_SOLENOID_PIN_2     7
#define MELON_SOLENOID_PIN_1        8
#define MELON_SOLENOID_PIN_2        9

#define ZUCCHINI_SENSOR_PIN         20
#define PEPPER_SENSOR_PIN           21
#define CUCUMBER_SENSOR_PIN         22
#define MELON_SENSOR_PIN            23

#define TIME_HEADER  "T"   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message
char currentTime[20] = "10:00 05/15/2025";

MenuManager MM;
Maltbie_Helper MH;
Menu MainMenu("**** Welcome to the Teensy Garden Menu ****");

GardenManager GM;
Zone zucchiniZone   ("Zucchini",    ZUCCHINI_SENSOR_PIN,    ZUCCHINI_SOLENOID_PIN_1,    ZUCCHINI_SOLENOID_PIN_2);
Zone pepperZone     ("Peppers",     PEPPER_SENSOR_PIN,      PEPPER_SOLENOID_PIN_1,      PEPPER_SOLENOID_PIN_2);
Zone cucumberZone   ("Cucumbers",   CUCUMBER_SENSOR_PIN,    CUCUMBER_SOLENOID_PIN_1,    CUCUMBER_SOLENOID_PIN_2);
Zone melonZone      ("Melons",      MELON_SENSOR_PIN,       MELON_SOLENOID_PIN_1,       MELON_SOLENOID_PIN_2);

bool debugPrint = false;

Timer_ms    sensorTimer;
uint8_t     menuValve = 0;

time_t  getTeensy3Time();
void    print12Hour(int digits);
void    digitalClockDisplay(time_t time);
void    digitalClockDisplayNow(void) {digitalClockDisplay(now());}
void    printDigits(int digits);
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

    GM.addZone(&zucchiniZone);
    GM.addZone(&pepperZone);
    GM.addZone(&cucumberZone);
    GM.addZone(&melonZone);

    GM.m_zones[0]->setDryThreshold(StoreEE.zoneAdryThreshold);
    GM.m_zones[0]->setWetThreshold(StoreEE.zoneAwetThreshold);
    GM.m_zones[1]->setDryThreshold(StoreEE.zoneBdryThreshold);
    GM.m_zones[1]->setWetThreshold(StoreEE.zoneBwetThreshold);
    GM.m_zones[2]->setDryThreshold(StoreEE.zoneCdryThreshold);
    GM.m_zones[2]->setWetThreshold(StoreEE.zoneCwetThreshold);
    GM.m_zones[3]->setDryThreshold(StoreEE.zoneDdryThreshold);
    GM.m_zones[3]->setWetThreshold(StoreEE.zoneDwetThreshold);

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

    GM.m_zones[0]->setScheduleDOW(now()+SECS_PER_MIN);
}

void loop(){
	WATCHDOG_RESET
	MM.handleLaptopInput();
    GM.maintain();

    // if (!sensorTimer.isActive())
    // {
    //     MH.serPtr()->print("Moisture Level: ");
    //     MH.serPtr()->println(zucchiniZone.moisture());
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



void digitalClockDisplay(time_t time){
    // digital clock display of the time
    print12Hour(hour(time));
    printDigits(minute(time));
    printDigits(second(time));
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
    MH.serPtr()->println(); 
}

time_t getTeensy3Time()
{
    return Teensy3Clock.get();
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

void printGardenStatus()
{
    GM.printZoneStatus(MH.serPtr());
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
}

void toggleValve()
{
    switch (menuValve)
    {
        case 1:
            zucchiniZone.valveIsOn() ? zucchiniZone.closeValve() : zucchiniZone.openValve();
            MH.serPtr()->print("Zucchini Zone Valve is now ");
            zucchiniZone.valveIsOn() ? MH.serPtr()->println("ON") : MH.serPtr()->println("OFF");
        break;

        case 2:
            pepperZone.valveIsOn() ? pepperZone.closeValve() : pepperZone.openValve();
            MH.serPtr()->print("Pepper Zone Valve is now ");
            pepperZone.valveIsOn() ? MH.serPtr()->println("ON") : MH.serPtr()->println("OFF");
        break;

        case 3:
            cucumberZone.valveIsOn() ? cucumberZone.closeValve() : cucumberZone.openValve();
            MH.serPtr()->print("Cucumber Zone Valve is now ");
            cucumberZone.valveIsOn() ? MH.serPtr()->println("ON") : MH.serPtr()->println("OFF");
        break;

        case 4:
            melonZone.valveIsOn() ? melonZone.closeValve() : melonZone.openValve();
            MH.serPtr()->print("Melon Zone Valve is now ");
            melonZone.valveIsOn() ? MH.serPtr()->println("ON") : MH.serPtr()->println("OFF");
        break;

        default:
        break;
    }
    menuValve = 0;
}

void clickyValveTest()
{
    zucchiniZone.openValve();
    delay(500);
    zucchiniZone.closeValve();
    delay(500);
    pepperZone.openValve();
    delay(500);
    pepperZone.closeValve();
    delay(500);
    cucumberZone.openValve();
    delay(500);
    cucumberZone.closeValve();
    delay(500);
    melonZone.openValve();
    delay(500);
    melonZone.closeValve();
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
    MH.serPtr()->print("In 1.5 Hours = ");
    digitalClockDisplay(now() + (SECS_PER_HOUR * 1.5));
    MH.serPtr()->print("Time @ \'0\' = ");
    digitalClockDisplay((time_t)0);
    MH.serPtr()->print("Previous Midnight = ");
    digitalClockDisplay(previousMidnight(now()));
    MH.serPtr()->print("8PM Today = ");
    digitalClockDisplay(previousMidnight(now()) + (SECS_PER_HOUR * 20));
    MH.serPtr()->println("This text is \033[1;32mGREEN\033[0m");
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
        GM.m_zones[zone]->setDryThreshold(moistureRead);
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
        GM.m_zones[zone]->setDryThreshold(newThresh);
        MH.serPtr()->print(GM.m_zones[zone]->name());
        MH.serPtr()->print(" - New Dry Threshold: ");
        MH.serPtr()->println(newThresh);
        memset(inputBuffer, '\0', sizeof(inputBuffer));
    }

    switch (zone)
    {
        case 0:
            StoreEE.zoneAdryThreshold = newThresh;
            break;
        
        case 1:
            StoreEE.zoneBdryThreshold = newThresh;
            break;

        case 2:
            StoreEE.zoneCdryThreshold = newThresh;
            break;
        case 3:

            StoreEE.zoneDdryThreshold = newThresh;
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
        GM.m_zones[zone]->setWetThreshold(moistureRead);
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
        GM.m_zones[zone]->setWetThreshold(newThresh);
        MH.serPtr()->print(GM.m_zones[zone]->name());
        MH.serPtr()->print(" - New Wet Threshold: ");
        MH.serPtr()->println(newThresh);
        memset(inputBuffer, '\0', sizeof(inputBuffer));
    }

    switch (zone)
    {
        case 0:
            StoreEE.zoneAwetThreshold = newThresh;
            break;
        
        case 1:
            StoreEE.zoneBwetThreshold = newThresh;
            break;

        case 2:
            StoreEE.zoneCwetThreshold = newThresh;
            break;
        case 3:

            StoreEE.zoneDwetThreshold = newThresh;
            break;

        default:
            break;
    }
}