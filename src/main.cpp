#include <Maltbie_Menu.h>
#include <Maltbie_Timer.h>

#include <TimeLib.h>

#include <GardenManager.h>

#define TIME_HEADER  "T"   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message
char currentTime[20] = "10:00 05-15-2025";

MenuManager MM;
Maltbie_Helper MH;
Menu MainMenu("**** Welcome to the Teensy Garden Menu ****");

GardenManager GM;
Zone pepperZone     ("Peppers",     23, 2);
Zone tomatoZone     ("Tomatoes",    22, 3);
Zone cucumberZone   ("Cucumbers",   21, 4);
Zone melonZone      ("Melons",      19, 5);

bool debugPrint = false;

time_t getTeensy3Time();
void print12Hour(int digits);
void digitalClockDisplay();
void printDigits(int digits);
void printGardenStatus(void);
void setRTC(void);


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
    MainMenu.addOption('D', "Display Time", digitalClockDisplay);    

    GM.addZone(pepperZone);
    GM.addZone(tomatoZone);
    GM.addZone(cucumberZone);
    GM.addZone(melonZone);

    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);

    MM.printHelp(&Serial, true);

    WATCHDOG_RESET
}

void loop() 
{
    static uint8_t i = 0;
	WATCHDOG_RESET
	MM.handleLaptopInput();
    // if (second(now() % 2 > 0))
    // {
    //     if (debugPrint) MH.serPtr()->println("HIGH\n");
    //     digitalWrite(2, HIGH);
    // }
    // else
    // {
    //     if (debugPrint) MH.serPtr()->println("LOW\n");
    //     digitalWrite(2, LOW);
    // }
    analogWrite(2, (i%256));
    i++;
    delay(10);
}



void digitalClockDisplay(){
    // digital clock display of the time
    print12Hour(hour(now()));
    printDigits(minute(now()));
    printDigits(second(now()));
    if (hour(now()) >= 12)
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
    digitalClockDisplay();
}