#include <Maltbie_Menu.h>
#include <Maltbie_Timer.h>

#include <TimeLib.h>

#include <GardenManager.h>

#define TIME_HEADER  "T"   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 

MenuManager MM;
Maltbie_Helper MH;
Menu MainMenu("**** Welcome to the Teensy Garden Menu ****");

GardenManager GM;
Zone pepperZone     ("Peppers",     23, 2);
Zone tomatoZone     ("Tomatoes",    22, 3);
Zone cucumberZone   ("Cucumbers",   21, 4);
Zone melonZone      ("Melons",      19, 5);

time_t getTeensy3Time();
void print12Hour(int digits);
void digitalClockDisplay();
void printDigits(int digits);


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

    GM.addZone(pepperZone);
    GM.addZone(tomatoZone);
    GM.addZone(cucumberZone);
    GM.addZone(melonZone);

    MM.printHelp(&Serial, true);

    WATCHDOG_RESET
}

void loop() 
{
	WATCHDOG_RESET
	MM.handleLaptopInput();
    GM.printZoneInfo(&Serial);
    delay(100);
}



void digitalClockDisplay(){
    // digital clock display of the time
    print12Hour(hour(now()));
    printDigits(minute(now()));
    printDigits(second(now()));
    if (hour(now()) >= 12)
    {
        Serial.print("PM,");
    }
    else
    {
        Serial.print("AM,");
    }
    Serial.print(" ");
    Serial.print(month());
    Serial.print("/");
    Serial.print(day());
    Serial.print("/");
    Serial.print(year()); 
    Serial.println("\n"); 
}

time_t getTeensy3Time()
{
    return Teensy3Clock.get();
}

void printDigits(int digits){
    // utility function for digital clock display: prints preceding colon and leading 0
    Serial.print(":");
    if(digits < 10)
        Serial.print('0');
    Serial.print(digits);
}

void print12Hour(int digits){
    int theHour = digits % 12;
    if (theHour == 0)
    {
        Serial.print(12);
    }
    else
    {
        Serial.print(theHour);
    }
}
