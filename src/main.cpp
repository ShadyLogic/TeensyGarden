#include <Maltbie_Menu.h>
#include <Maltbie_Timer.h>

#include <TimeLib.h>

#include <GardenManager.h>

#define PEPPER_SOLENOID_PIN_1       2
#define PEPPER_SOLENOID_PIN_2       3
#define TOMATO_SOLENOID_PIN_1       4
#define TOMATO_SOLENOID_PIN_2       5
#define CUCUMBER_SOLENOID_PIN_1     6
#define CUCUMBER_SOLENOID_PIN_2     7
#define MELON_SOLENOID_PIN_1        8
#define MELON_SOLENOID_PIN_2        9

#define PEPPER_SENSOR_PIN       20
#define TOMATO_SENSOR_PIN       21
#define CUCUMBER_SENSOR_PIN     22
#define MELON_SENSOR_PIN        23

#define TIME_HEADER  "T"   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message
char currentTime[20] = "10:00 05/15/2025";

MenuManager MM;
Maltbie_Helper MH;
Menu MainMenu("**** Welcome to the Teensy Garden Menu ****");

GardenManager GM;
Zone pepperZone     ("Peppers",     PEPPER_SENSOR_PIN,      PEPPER_SOLENOID_PIN_1, PEPPER_SOLENOID_PIN_2);
Zone tomatoZone     ("Tomatoes",    TOMATO_SENSOR_PIN,      TOMATO_SOLENOID_PIN_1, TOMATO_SOLENOID_PIN_2);
Zone cucumberZone   ("Cucumbers",   CUCUMBER_SENSOR_PIN,    CUCUMBER_SOLENOID_PIN_1, CUCUMBER_SOLENOID_PIN_2);
Zone melonZone      ("Melons",      MELON_SENSOR_PIN,       MELON_SOLENOID_PIN_1, MELON_SOLENOID_PIN_2);

bool debugPrint = false;

Timer_ms    sensorTimer;
uint8_t     menuValve = 0;

time_t getTeensy3Time();
void print12Hour(int digits);
void digitalClockDisplay(time_t time);
void digitalClockDisplayNow(void) {digitalClockDisplay(now());}
void printDigits(int digits);
void printGardenStatus(void);
void setRTC(void);
void toggleValve(void);
void clickyValveTest(void);
void closeAllValves(void);
void openAllValves(void);
void setValveRunTime(void);
char valveRunTime[15] = "0-0";

void timeTest(void);
time_t lastTime;

bool funkyLights = false;


void setup() 
{
    setSyncProvider(getTeensy3Time);
	Serial.begin(115200);
	delay(100);

    // setTime(16,50,0,1,5,2025);
    // Teensy3Clock.set(now());
    // setTime(now());

	MM.setupBLE();
	MM.setupEEPROM();
	MM.registerMenu(&MainMenu);
    MM.startWatchdog();

    MainMenu.addOption('A', "Print Garden Status", printGardenStatus);
    MainMenu.addOption('B', "Show Debug Prints", &debugPrint);
    MainMenu.addOption('C', "Set Time (HH:MM DD/MM/YYYY)", currentTime, setRTC);
    MainMenu.addOption('D', "Display Time", digitalClockDisplayNow);
    MainMenu.addOption('F', "Print Garden Status", printGardenStatus);
    MainMenu.addOption('T', "Toggle Valve", &menuValve, 4, 1, toggleValve);
    MainMenu.addOption('V', "Clicky Valve Test", clickyValveTest);
    MainMenu.addOption('+', "Open All Valves", openAllValves);
    MainMenu.addOption('-', "Close All Vavles", closeAllValves);
    MainMenu.addOption('E', "Time Test", timeTest);
    MainMenu.addOption('G', "Valve Run Time (Valve#-Min)", valveRunTime, setValveRunTime);
    MainMenu.addOption('I', "Funky Lights", &funkyLights);

    GM.addZone(&pepperZone);
    GM.addZone(&tomatoZone);
    GM.addZone(&cucumberZone);
    GM.addZone(&melonZone);

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

    memset(valveRunTime, '\0', sizeof(valveRunTime));
}

void loop(){
	WATCHDOG_RESET
	MM.handleLaptopInput();
    GM.maintain();

    // if (!sensorTimer.isActive())
    // {
    //     MH.serPtr()->print("Moisture Level: ");
    //     MH.serPtr()->println(pepperZone.moisture());
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
    MH.serPtr()->print(month());
    MH.serPtr()->print("/");
    MH.serPtr()->print(day());
    MH.serPtr()->print("/");
    MH.serPtr()->print(year()); 
    MH.serPtr()->println("\n"); 
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
            pepperZone.valveIsOn() ? pepperZone.closeValve() : pepperZone.openValve();
            MH.serPtr()->print("Pepper Zone Valve is now ");
            pepperZone.valveIsOn() ? MH.serPtr()->println("ON") : MH.serPtr()->println("OFF");
        break;

        case 2:
            tomatoZone.valveIsOn() ? tomatoZone.closeValve() : tomatoZone.openValve();
            MH.serPtr()->print("Tomato Zone Valve is now ");
            tomatoZone.valveIsOn() ? MH.serPtr()->println("ON") : MH.serPtr()->println("OFF");
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
    pepperZone.openValve();
    delay(500);
    pepperZone.closeValve();
    delay(500);
    tomatoZone.openValve();
    delay(500);
    tomatoZone.closeValve();
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
}

void setValveRunTime()
{
    int zone = valveRunTime[0] - '0';
    char buffer[15];
    strcpy(buffer, &valveRunTime[2]);
    int runTime = atoi(buffer);
    Zone* tempZone = GM.valveRunTime(zone-1, runTime);
    MH.serPtr()->print(tempZone->name());
    MH.serPtr()->print(" valve will turn off at ");
    digitalClockDisplay(tempZone->timeToTurnOffValve());
    memset(valveRunTime, '\0', sizeof(valveRunTime));
}