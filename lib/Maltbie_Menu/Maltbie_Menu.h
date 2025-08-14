// Maltbie Menu Functions
// Jacob Rogers, Kubik Maltbie

#ifndef MALTBIE_MENU_H
#define MALTBIE_MENU_H

#include "Arduino.h"
#include <string.h>

#include <Menu_Config.h>
#include <Maltbie_Helper.h>

#define MENU_MAX_OPTIONS    64

#if USE_ETHERNET
    #ifdef TEENSYDUINO
        #define err_t s8_t      // Required to fix conflict with Adafruit_BluefruitLE_nRF51
        #include <QNEthernet.h>
    #else
        #include <Ethernet.h>
        #include <EthernetUdp.h>
    #endif
#endif

#if USE_WATCHDOG
    #ifdef TEENSYDUINO
        #define TEENSYDOG true
        #include <Watchdog_t4.h>
        extern WDT_T4<WDT1> wdt;
        #define WATCHDOG_RESET wdt.feed();
    #else
        #define TEENSYDOG false
        // #include <Adafruit_SleepyDog.h>          // COMMENT THIS OUT TO USE WITH TEENSY - THERE IS NO PREPROCESSOR WAY TO NOT SKIP INCLUDING THIS WITHOUT ERRORS
        #define WATCHDOG_RESET Watchdog.reset();
    #endif
#else
    #define WATCHDOG_RESET
#endif

#if DO_DEBUG_PRINTS
  #define DEBUG_PRINT(...)   MH.serPtr()->print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...) MH.serPtr()->println(__VA_ARGS__)
#else
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
#endif




/////////////////////
//   P R O T O S   //
/////////////////////

void printIPAddress(Stream*, uint32_t);
void kickTheDog(void);
void unassignedCommand(void);
void duplicateCommand(char);

enum DataTypes
{
    i8,
    i16,
    ipad,
    ch,
    dub,
    boo,
    func,
    dat
};

union DataPointer
{
    uint8_t*            i8;
    uint16_t*           i16;
    uint32_t*           ipad;
    char*               ch;
    double*             dub;
    bool*               boo;
    void*               dat;
};

struct DataStruct
{
    DataTypes tag;
    DataPointer ptr;
};

class MenuOption {
public:
    MenuOption() {}
    MenuOption(char comChar, String desc, uint8_t *i8, uint32_t uLim = 255UL, uint32_t lLim = 0UL, void (*f)() = NULL);
    MenuOption(char comChar, String desc, uint16_t *i16, uint32_t uLim = 65535UL, uint32_t lLim = 0UL, void (*f)() = NULL);
    MenuOption(char comChar, String desc, uint32_t *ipad, IPAddress *ippointer, void (*f)() = NULL);
    MenuOption(char comChar, String desc, double *dub, long uLim = 2147483647UL, long lLim = -2147483647UL, void (*f)() = NULL);
    MenuOption(char comChar, String desc, char* ch, void (*f)() = NULL);
    MenuOption(char comChar, String desc, bool *boo, void (*f)() = NULL);
    MenuOption(char comChar, String desc, void (*f)() = NULL);
    MenuOption(char comChar, String desc, void *data, bool flag);

    void UpdateValue(char* val);
    void UpdateValue(void *data, size_t size);
    void print(Stream *serialPtr = MH.serPtr());
    void printValue(Stream *serialPtr = MH.serPtr());
    void dumpConfig(Stream *serialPtr = MH.serPtr());
    void registerFunction(void (*f)(void)) 
    {
        m_Function = f;
    }
    void doAction(void);

private:
    friend class Menu;
    friend class MenuManager;
    String      m_description;
    char        m_commandChar;
    DataStruct  m_data;
    long        m_uLim;
    long        m_lLim;
    void        (*m_Function)(void);
    IPAddress   *m_ippointer;
};


class Menu {
public:
    Menu(char* title);
    void printTitle(Stream *serialPtr = MH.serPtr()) {serialPtr->println(m_title);}
    void printOptions(Stream *serialPtr = MH.serPtr());
    void dumpConfig(Stream *serialPtr = MH.serPtr());
    MenuOption* addOption(char comChar, String desc, uint8_t *i8, uint32_t uLim = 255UL, uint32_t lLim = 0UL, void (*f)() = NULL);
    MenuOption* addOption(char comChar, String desc, uint16_t *i16, uint32_t uLim = 65535UL, uint32_t lLim = 0UL, void (*f)() = NULL);
    MenuOption* addOption(char comChar, String desc, uint32_t *ipad, IPAddress *ippointer, void (*f)() = NULL);
    MenuOption* addOption(char comChar, String desc, double *dub, long uLim = 2147483647UL, long lLim = -2147483648UL, void (*f)() = NULL);
    MenuOption* addOption(char comChar, String desc, char* ch, void (*f)() = NULL);
    MenuOption* addOption(char comChar, String desc, bool *boo, void (*f)() = NULL);
    MenuOption* addOption(char comChar, String desc, void (*f)() = NULL);
    MenuOption* addOption(char comChar, String desc, void *data, bool flag = true);
    MenuOption* addOption(MenuOption op);
    void updateTitle(char* newTitle) {strncpy(m_title, newTitle, sizeof(m_title));}
    bool comCharAvailable(char comChar);
    MenuOption GetOptionByComChar(char comChar);
    MenuOption m_options[MENU_MAX_OPTIONS];
private:
    friend class MenuManager;
    char m_title[MAX_CHAR];
    uint8_t m_numOptions;
};

class MenuManager : public Maltbie_Helper {
public:
    MenuManager() {}
    void handleLaptopInput(void);
    void processUserInput(char userIn[MAX_MENU_CHARS], uint8_t& indexUserIn, int inByte, Stream *serialPtr = MH.serPtr(), bool handleUsbOnlyOptions = true);
    void dumpConfigCommands(Stream *serialPtr = MH.serPtr(), bool handleUsbOnlyOptions = true);
    void FactoryDefault(void);
    void FactoryDefault_BLE(void);
    void startWatchdog(uint8_t pin = 0);
    void watchDogReset(void);
    void printHelp(Stream *serialPtr = MH.serPtr(), bool handleUsbOnlyOptions = true);
    void setupBLE(void);
    void setupEEPROM(void);
    void registerMenu(Menu *menuPtr) {m_menuPtr = menuPtr;}
    void printMenu(void);

private:
    Menu *m_menuPtr;
    #if ENABLE_BLE
        EEPROM_BLE_Struct StoreEE_BLE;
    #endif
};

extern MenuManager MM;          // Global instance of MenuManager for main class sketches to use

#endif //end MALTBIE_MENU_H
