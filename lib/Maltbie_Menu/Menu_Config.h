#ifndef MENU_CONFIG_H
#define MENU_CONFIG_H

//////////   INCLUDES   //////////
#if defined(__arm__) && defined(TEENSYDUINO)
    #include <QNEthernet.h>
#else
    // #include <Ethernet.h>
#endif

//////////   EXHIBIT NAME   //////////
#define EXHIBIT_NAME_VERSION    "Teensy Garden 1.2"

//////////   BLE DEVICE INFO   //////////
#define THIS_DEVICE_NAME     "TeensyGarden"
#define BT_PW_DEFAULT        "letitgrow"       // Same for all exhibits
#define OFFSET_STOREEE 50

//////////   SOFTWARE CODE SWITCHES   //////////
#define USE_WATCHDOG        true
#define USE_ETHERNET        true
#define ENABLE_BLE          true
#define DO_DEBUG_PRINTS     false

//////////   ARRAY SIZE DEFINITION   //////////
#define MAX_CHAR                30


/////////////////////////////////////////
//   E E P R O M   S T R U C T U R E   //
/////////////////////////////////////////
 
#define EEPROM_VALID_PAT1   0xAA
#define EEPROM_VALID_PAT2   0x55
#define EEPROM_END_VER_SIG  0x0102
struct EEPROM_Struct
{
    uint8_t     eeValid_1;      // EE is Valid_1: 0xAA
    uint8_t     eeValid_2;      // EE is Valid_2: 0x55
//////////////////////////////////////
// Application Specific Config Here //                                                      //STRUCT DEFINITION//
//////////////////////////////////////////////////////
//                                                  //
    char        zone1name[20];
    uint16_t    zone1wetThreshold;
    uint16_t    zone1dryThreshold;
    uint8_t     zone1timeBetweenWatering;
    uint8_t     zone1durationToWater;
    char        zone2name[20];
    uint16_t    zone2wetThreshold;
    uint16_t    zone2dryThreshold;
    uint8_t     zone2timeBetweenWatering;
    uint8_t     zone2durationToWater;
    char        zone3name[20];
    uint16_t    zone3wetThreshold;
    uint16_t    zone3dryThreshold;
    uint8_t     zone3timeBetweenWatering;
    uint8_t     zone3durationToWater;
    char        zone4name[20];
    uint16_t    zone4wetThreshold;
    uint16_t    zone4dryThreshold;
    uint8_t     zone4timeBetweenWatering;
    uint8_t     zone4durationToWater;
    
//                                                  //
//////////////////////////////////////////////////////
    uint16_t    eeVersion;  // Change this if the eeprom layout changes
};

const EEPROM_Struct STOREEE_DEFAULTS =
{
    EEPROM_VALID_PAT1,
    EEPROM_VALID_PAT2,
/////////////////////////////////////
// Application Default Config Here //                                                      //STRUCT DEFINITION//
//////////////////////////////////////////////////////
//                                                  //
    "Zone 1",               // zone1name
    500,                    // zone1wetThreshold
    100,                    // zone1dryThreshold
    24,                     // zone1timeBetweenWatering
    60,                     // zone1durationToWater
    "Zone 2",               // zone2name
    500,                    // zone2wetThreshold
    100,                    // zone2dryThreshold
    24,                     // zone2timeBetweenWatering
    60,                     // zone2durationToWater
    "Zone 3",               // zone3name
    500,                    // zone3wetThreshold
    100,                    // zone3dryThreshold
    24,                     // zone3timeBetweenWatering
    60,                     // zone3durationToWater
    "Zone 4",               // zone4name
    500,                    // zone4wetThreshold
    100,                    // zone4dryThreshold
    24,                     // zone4timeBetweenWatering
    60,                     // zone4durationToWater
//                                                  //
//////////////////////////////////////////////////////
    EEPROM_END_VER_SIG,
}; 

extern struct EEPROM_Struct StoreEE;


/////////////////////////////////////////////////
//   B L E   E E P R O M   S T R U C T U R E   //
/////////////////////////////////////////////////

#if ENABLE_BLE

    #include "Maltbie_BLE_Class.h"
    #define MAX_PW                      16
    #define MAX_BT_NAME                 16
    #define EEPROM_VALID_BLE_PAT1       0xAB
    #define EEPROM_VALID_BLE_PAT2       0x65
    #define EEPROM_END_VER_SIG_BLE      0xA101
    struct EEPROM_BLE_Struct
    {
        uint8_t     eeValid_1;              // EE is Valid_1: 0xAA
        uint8_t     eeValid_2;              // EE is Valid_2  0x55
        char        blePassword[MAX_PW];    // Password for BLE access
        char        bleName[MAX_BT_NAME];   // Name of BLE as appears to other device
        uint16_t    eeVersion;              // Change this if the eeprom layout changes
    };

    const EEPROM_BLE_Struct STOREEE_BLE_DEFAULTS =
    {
        EEPROM_VALID_BLE_PAT1,      //eeValid_1
        EEPROM_VALID_BLE_PAT2,      //eeValid_2
        BT_PW_DEFAULT,              //blePassword[MAX_PW]
        THIS_DEVICE_NAME,           //bleName[MAX_BT_NAME]
        EEPROM_END_VER_SIG_BLE      //eeVersion
    };

    extern struct EEPROM_BLE_Struct StoreEE_BLE;
    
#endif //ENABLE_BLE

#endif //MENU_CONFIG_H