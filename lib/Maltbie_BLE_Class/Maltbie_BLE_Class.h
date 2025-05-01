// Maltbie Bluetooth Low Energy Functions, uses Adafruit Bluefruit shield
// Kubik Maltbie, Sid Roberts

#ifndef MALTBIE_BLE_CLASS_H
#define MALTBIE_BLE_CLASS_H

#define DEFAULT_BLE_PW "MuseumBLE_PW"    // If BLE is used, this will be the password

#include <string.h>
#include <Arduino.h>
#include <SPI.h>
//#include <Adafruit_NeoPixel.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "BluefruitConfig.h"

#include "Maltbie_Timer.h"
#include "Maltbie_Helper.h"

#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"

// Enable ENABLE_ADA_COLOR_PICK only if want to use the Adafruit Connect BLE app for color picking,
//  otherwise, if user enters '!', it will hang the BLE until enough chars are entered.
#define ENABLE_ADA_COLOR_PICK false 

// Common ASCII Chars
#define ASCII_STX           0x02
#define ASCII_CR            0x0D
#define ASCII_LF            0x0A
#define ASCII_ESC           0x1B


enum ble_Cmd_Type
{
   BCT_NONE                     = 0 // May be part way through a command entry
   ,BCT_NewPwAccepted           = 1 // Password accepted (might want to show the help screen)
   ,BCT_NonBleSpecificCmd_PwOk  = 2 // Call processUserInput() to handle an entered command
   ,BCT_NonBleSpecificCmd_PwNo  = 3 // No valid Password entered
   ,BCT_ArrowPressedUp          = 4
   ,BCT_ArrowPressedDown        = 5 
   ,BCT_Color                   = 6
   ,BCT_AlreadyHandled          = 7// End of a command string gotten, but not one we use
   //,BCT_
};

class Maltbie_BLE : public Adafruit_BluefruitLE_SPI 
{
public:
    Maltbie_BLE(int8_t csPin, int8_t irqPin, int8_t rstPin);
    Maltbie_BLE(int8_t clkPin, int8_t misoPin, int8_t mosiPin, int8_t csPin, int8_t irqPin, int8_t rstPin); // Bitbanged SPI

    bool init_BLE_Shield(char deviceName[], bool doFactoryReset);
    ble_Cmd_Type BLE_Comm(char userIn[], uint8_t& indexUserIn, uint8_t maxInChars, int& inByte, char devName[], char passWord[], uint8_t maxPwLen, uint32_t& newColor);
    bool showPeerInfo(void);
    bool wasMasterPwEntered(void){ return m_masterPwVerified;}
    bool connectedNow(void){ return m_lastConnState;}
    void registerWatchDogReset(void (*f)())     // Example caller: blueToothSerial.registerWatchDogReset(kickTheDog);  (provide also: kickTheDog();)
    {
        m_watchDogResetFunction = f;
    }
    void kickWatchDog(void);    // If a watchdog reset callback has been registered, use it.
    bool havePassWordMatch(void) { return m_pwMatch; }
    //bool moduleIsOk(void) { return m_BLE_attached;}

private:
    bool     m_pwMatch;
    //bool     m_BLE_attached;
    bool     m_masterPwVerified;
    bool     m_thisPhoneLastPwstate;
    bool     m_lastConnState;
    char     m_lastDeviceID[18];
    bool     m_lastDeviceIdValid = false;
    Timer_ms m_checkIsConnectedTimer;
    Timer_ms m_sendPwPromptTimer;
    Timer_ms m_welcomeBackTimer;
    void (*m_watchDogResetFunction)(void);
};

extern Maltbie_BLE blueToothSerial;  //(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

#endif //end MALTBIE_BLE_CLASS_H



