#ifndef MENU_CONFIG_H
#define MENU_CONFIG_H

//////////   INCLUDES   //////////
#if defined(__arm__) && defined(TEENSYDUINO)
    #include <QNEthernet.h>
#else
    // #include <Ethernet.h>
#endif

//////////   EXHIBIT NAME   //////////
#define EXHIBIT_NAME_VERSION    "Teensy Garden 1.0"

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
#define EEPROM_END_VER_SIG  0x0100
struct EEPROM_Struct
{
    uint8_t     eeValid_1;      // EE is Valid_1: 0xAA
    uint8_t     eeValid_2;      // EE is Valid_2: 0x55
//////////////////////////////////////
// Application Specific Config Here //                                                      //STRUCT DEFINITION//
//////////////////////////////////////////////////////
//                                                  //
    uint32_t            deviceIP;
    uint32_t            gateway;
    uint32_t            subnet;
    uint32_t            dns;
    uint32_t            remoteIP;
    uint16_t            remotePort;
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
    704751808,                  // deviceIP - 192.168.1.42    
    4278298816,                 // gateway  - 192.168.1.255    
    16777215,                   // subnet   - 255.255.255.255    
    134744072,                  // dns      - 8.8.8.8    
    838969536,                  // remoteIP - 192.168.1.50
    50000,                      // remotePort
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