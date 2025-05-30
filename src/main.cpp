#include <Maltbie_Menu.h>
#include <Maltbie_Timer.h>

#include <TimeLib.h>

#include <GardenManager.h>

#define PEPPER_SOLENOID_PIN     2
#define TOMATO_SOLENOID_PIN     3
#define CUCUMBER_SOLENOID_PIN   4
#define MELON_SOLENOID_PIN      5

#define PEPPER_SENSOR_PIN       23
#define TOMATO_SENSOR_PIN       22
#define CUCUMBER_SENSOR_PIN     21
#define MELON_SENSOR_PIN        19

#define TIME_HEADER  "T"   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message
char currentTime[20] = "10:00 05/15/2025";

MenuManager MM;
Maltbie_Helper MH;
Menu MainMenu("**** Welcome to the Teensy Garden Menu ****");

GardenManager GM;
Zone pepperZone     ("Peppers",     PEPPER_SENSOR_PIN,      PEPPER_SOLENOID_PIN);
Zone tomatoZone     ("Tomatoes",    TOMATO_SENSOR_PIN,      TOMATO_SOLENOID_PIN);
Zone cucumberZone   ("Cucumbers",   CUCUMBER_SENSOR_PIN,    CUCUMBER_SOLENOID_PIN);
Zone melonZone      ("Melons",      MELON_SENSOR_PIN,       MELON_SOLENOID_PIN);

bool debugPrint = false;

time_t getTeensy3Time();
void print12Hour(int digits);
void digitalClockDisplay(time_t time);
void digitalClockDisplayNow(void) {digitalClockDisplay(now());}
void printDigits(int digits);
void printGardenStatus(void);
void setRTC(void);
void toggleValves(void);

void timeTest(void);
time_t lastTime;


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
    MM.startWatchdog(20);

    MainMenu.addOption('A', "Print Garden Status", printGardenStatus);
    MainMenu.addOption('B', "Show Debug Prints", &debugPrint);    
    MainMenu.addOption('C', "Set Time (HH:MM DD/MM/YYYY)", currentTime, setRTC);    
    MainMenu.addOption('D', "Display Time", digitalClockDisplayNow);    
    MainMenu.addOption('T', "Toggle Valves", toggleValves);    

    GM.addZone(pepperZone);
    GM.addZone(tomatoZone);
    GM.addZone(cucumberZone);
    GM.addZone(melonZone);

    MM.printHelp(&Serial, true);

    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(7, OUTPUT);
    pinMode(8, OUTPUT);
    pinMode(9, OUTPUT);

    WATCHDOG_RESET

    // digitalWrite(2, HIGH);
    // digitalWrite(3, HIGH);
    // digitalWrite(4, HIGH);
    // digitalWrite(5, HIGH);
    // digitalWrite(6, HIGH);
    // digitalWrite(7, HIGH);
    // digitalWrite(8, HIGH);
    // digitalWrite(9, HIGH);

    lastTime = now();
}

void loop(){
	WATCHDOG_RESET
	MM.handleLaptopInput();
    timeTest();

    // static int curSec = 0;
    // if (second(now()) != curSec){
    //     curSec = second(now());
    //     if (second(now()) % 2 > 0) digitalWrite(2, !digitalRead(2));
    //     if (second(now()) % 3 > 0) digitalWrite(3, !digitalRead(3));
    //     if (second(now()) % 4 > 0) digitalWrite(4, !digitalRead(4));
    //     if (second(now()) % 5 > 0) digitalWrite(5, !digitalRead(5));
    //     if (second(now()) % 6 > 0) digitalWrite(6, !digitalRead(6));
    //     if (second(now()) % 7 > 0) digitalWrite(7, !digitalRead(7));
    //     if (second(now()) % 8 > 0) digitalWrite(8, !digitalRead(8));
    //     if (second(now()) % 9 > 0) digitalWrite(9, !digitalRead(9));
    // }
    delay(1000);
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
    GM.printZoneInfo(MH.serPtr());
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

void toggleValves()
{
    bool toggleState = !digitalRead(6);
    digitalWrite(6, toggleState);
    digitalWrite(7, toggleState);

    MH.serPtr()->print("Valves are now ");
    toggleState ? MH.serPtr()->println("ON") : MH.serPtr()->println("OFF");
}

void timeTest()
{
    Serial.print("Now - Last = ");
    Serial.println(now() - lastTime);
    Serial.print("Seconds Since Last = ");
    Serial.println(numberOfSeconds(now() - lastTime));
    Serial.print("Minutes Since Last = ");
    Serial.println(numberOfMinutes(now() - lastTime));
    Serial.print("Now = ");
    digitalClockDisplayNow();
    Serial.print("In 1.5 Hours = ");
    digitalClockDisplay(now() + (SECS_PER_HOUR * 1.5));
}