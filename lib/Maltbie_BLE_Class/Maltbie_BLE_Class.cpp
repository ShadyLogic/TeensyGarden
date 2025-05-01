// Maltbie Bluetooth Low Energy Functions, uses Adafruit Bluefruit shield

#include "Maltbie_BLE_Class.h"

Maltbie_BLE blueToothSerial(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

extern bool doDebugPrints;  // disable printing by default
#define DO_DEBUG_PRINTS_BLE
#ifdef DO_DEBUG_PRINTS_BLE
#define DBPRINTLN(args...) if(doDebugPrints) { Serial.println(args);} 
#define DBPRINT(args...)   if(doDebugPrints) { Serial.print(args);}   
#define DBWRITE(args...)   if(doDebugPrints) { Serial.write(args);}   
#else
#define DBPRINTLN(args...) 
#define DBPRINT(args...) 
#define DBWRITE(args...) 
#endif

// "irqPin" is high when incoming characters are available for reading. It is NOT used as an IRQ.
Maltbie_BLE::Maltbie_BLE(int8_t csPin, int8_t irqPin, int8_t rstPin) : Adafruit_BluefruitLE_SPI(csPin, irqPin, rstPin)
{ 
    m_pwMatch              = false;
    m_masterPwVerified     = false;
    m_thisPhoneLastPwstate = false;
    m_lastConnState        = false;
    m_lastDeviceIdValid    = false;
    m_lastDeviceID[0]      = 0;
    m_watchDogResetFunction = NULL;
    //m_BLE_attached         = false;
}

Maltbie_BLE::Maltbie_BLE(int8_t clkPin, int8_t misoPin, int8_t mosiPin, int8_t csPin, int8_t irqPin, int8_t rstPin) 
    : Adafruit_BluefruitLE_SPI(clkPin, misoPin, mosiPin, csPin, irqPin, rstPin)
{ 
    m_pwMatch              = false;
    m_masterPwVerified     = false;
    m_thisPhoneLastPwstate = false;
    m_lastConnState        = false;
    m_lastDeviceIdValid    = false;
    m_lastDeviceID[0]      = 0;
    m_watchDogResetFunction = NULL;
    //m_BLE_attached         = false;
}

bool Maltbie_BLE::init_BLE_Shield(char deviceName[], bool doFactoryReset)
{
    bool rc = true;

    /* Initialise the module */
    Serial.println(F("Initialising the Bluefruit LE module: "));

    setMode(BLUEFRUIT_MODE_COMMAND);    // Make sure it (or maybe we) is in the correct mode for us to comm after reset

    if (!begin(VERBOSE_MODE))
    {
       Serial.println(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
        rc = false;
    }
    else
    {
        //Serial.println(F("OK!"));

        delay(500);

       if (doFactoryReset)
       {
            for (uint8_t i = 0; i < 3; i++) 
            {
                kickWatchDog();     // If a watchdog reset callback has been registered, use it.

                /* Perform a factory reset to make sure everything is in a known state */
                Serial.println(F("Performing a factory reset of Bluetooth: "));
                if (!blueToothSerial.factoryReset() || (_isWorking == false))
                {
                    Serial.println(F("\nCouldn't factory reset (BLE shield installed?)"));
                    rc = false;
                    delay(1000);   // Wait before trying again
                }
                else
                {
                    Serial.println(F("Factory reset: OK"));
                    rc = true;
                    break;
                }

                if (rc == false)
                {
                    setIsWorking(false);    // Inform BLE we ain't work'ng
                }
            }
        }
        else
        {
            Serial.println(F("Skipping factory reset"));
        }

        if (rc)
        {
            /* Disable command echo from Bluefruit */
            echo(false);

            Serial.println("Requesting Bluefruit info:");
            /* Print Bluefruit information */
            info();

            verbose(false);  // debug info is a little annoying after this point!
            Serial.println(F("******************************"));

            // LED Activity command is only supported from 0.6.6
            if ( isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
            {
                // Change Mode LED Activity
                Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
                sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
            }

            char nameCmd[33] = "AT+GAPDEVNAME=";  // Will append past the "="
            strncat(nameCmd, deviceName, (sizeof(nameCmd) - strlen(nameCmd))-1);
            Serial.print(F("Setting new device name using: "));
            Serial.println(nameCmd);

                    for (uint8_t i = 0; i < 3; i++)
                    {
                        kickWatchDog();     // If a watchdog reset callback has been registered, use it.

                        rc = sendCommandCheckOK(nameCmd); // Change name of module that will be advertized
                        if (rc)
                        {
                            Serial.println(F("OK"));
                            break;
                        }
                        else Serial.println(F("Failed setting BLE device name."));
                    }

            // Set module to DATA mode
            Serial.println( F("Switching to DATA mode!") );
            setMode(BLUEFRUIT_MODE_DATA);

            Serial.println(F("******************************"));
        }
    }

    //m_BLE_attached = rc;    // If module not present: false. Responding OK: true.

    return rc;
}

#define PW_PROMPT_TIME 6000
#define CONN_CHK_TICK  1000 // Check on connection status this often

// Handle BLE specific comm
ble_Cmd_Type Maltbie_BLE::BLE_Comm(char userIn[], uint8_t & indexUserIn, uint8_t maxInChars, int & inByte, char devName[], char passWord[], uint8_t maxPwLen, uint32_t & newColor)
{
    // Handle BLE characters
    ble_Cmd_Type btCmd = BCT_NONE;
    
    if (m_checkIsConnectedTimer.StartIfStopped(CONN_CHK_TICK))
    {
        static uint16_t disConCount = 0;
        flush();    // Flush before checking connection

        bool isConn = isConnected();

        if (isConn == false)
        {
            if (disConCount++ < 2)  // Wraping back around to 0 won't matter
            {
                // Don't declare disconnected on first determination. Wait for 3x.
                isConn = m_lastConnState;
                Serial.println("\nBLE Disconnect count: " + String(disConCount));
            }
        }
        else
        {
            disConCount = 0;
        }

        if (m_lastConnState != isConn)
        {
            m_lastConnState = isConn;
            Serial.print(F("New Bluetooth Conn State: "));

            Serial.println(m_lastConnState ? F("Connected") : F("Disconnected"));

            if (m_lastConnState)
            {
                if (showPeerInfo())              // AT+BLEGETPEERADDR Is this the same phone as last time? Returns true if so.
                {
                    m_pwMatch = m_thisPhoneLastPwstate;
                    if (m_pwMatch)
                    {
                        Serial.println(F("Returning BLE device (no PW needed)!"));
                        m_welcomeBackTimer.Start(1500);   // In a sec, show welcome message
                    }
                    else
                    {
                        m_sendPwPromptTimer.Start(PW_PROMPT_TIME);
                    }
                }
                else
                {
                    m_sendPwPromptTimer.Start(PW_PROMPT_TIME);
                    m_pwMatch = false;
                }
            }
            else
            {
                // Connection Lost, reset BLE module so a different phone can connect up
                MH.resetSerialOutputStream();
                init_BLE_Shield(devName, true);
                m_masterPwVerified     = false;
                m_thisPhoneLastPwstate = m_pwMatch;
            }
        }
    }// if (m_checkIsConnectedTimer.StartIfStopped(CONN_CHK_TICK))

    if (m_sendPwPromptTimer.isComplete())
    {
        if (m_pwMatch == false)
        {
            //Serial.println(F("Sending PW request..."));

            println(F("Welcome! Please enter \"P\"\r\n  and the Password "));    // Send this every few seconds until user enters something
            m_sendPwPromptTimer.Start(PW_PROMPT_TIME);
        }
    }

    if (m_welcomeBackTimer.isComplete())
    {
        println(F("Welcome Back! (No password required)\n 'H' for Help."));    
        btCmd = BCT_NewPwAccepted;  // Cause blinking/identifying of the connected unit DOYET-Optional
    }

    while (available())
    {

        inByte = read();
        if (inByte == ASCII_LF)
        {
            DBPRINTLN("<btLF>");
        }

        if (inByte == ASCII_CR)
        {
            DBPRINTLN("<btCR>");
        }


        if (indexUserIn < (maxInChars - 1))
        {
            if (!((inByte == ASCII_CR) || (inByte == ASCII_LF)))
            {
                userIn[indexUserIn++] = inByte; // Append the latest character, unless it is a CR or LF
                userIn[indexUserIn] = 0;  // DEBUG
            }

            DBPRINT("bt[");         
            DBPRINT(userIn);        
            DBPRINT("] 0x");        
            DBPRINTLN(inByte, HEX); 
#if ENABLE_ADA_COLOR_PICK
            if (userIn[0] == '!')   // used by Adafruit Bluefruit Connect app
            {
                if ((userIn[1] == 'C') && (indexUserIn == 6))
                {
                    if (m_pwMatch)
                    {
                        btCmd = BCT_Color;

                        uint8_t red = userIn[2];
                        uint8_t green = userIn[3];
                        uint8_t blue = userIn[4];
                        Serial.print(F("RGB #"));
                        if (red < 0x10) Serial.print(F("0"));
                        Serial.print(red, HEX);
                        if (green < 0x10) Serial.print(F("0"));
                        Serial.print(green, HEX);
                        if (blue < 0x10) Serial.print(F("0"));
                        Serial.println(blue, HEX);
                        newColor = (((uint32_t)red) << 16) + (((uint32_t)green) << 8) + blue;
                    }

                    indexUserIn = 0;
                } //end if ((userIn[1] == 'C') && (indexUserIn == 6))
                else if ((userIn[1] == 'B') && (indexUserIn == 5))
                {
                    if (m_pwMatch)
                    {
                        if (strncmp(userIn, "!B516", 5) == 0)    // Up Arrow
                        {
                            btCmd = BCT_ArrowPressedUp;
                        }
                        if (strncmp(userIn, "!B615", 5) == 0)    // Down Arrow
                        {
                            btCmd = BCT_ArrowPressedDown;
                        }
                    }

                    indexUserIn = 0;
                }
            } //end if (userIn[0] == '!')   // used by Adafruit Bluefruit Connect app
            else
#endif //ENABLE_ADA_COLOR_PICK
            {
                if ((inByte == ASCII_CR) || (inByte == ASCII_LF))
                {
                    if (m_pwMatch)
                    {
                        btCmd = BCT_NonBleSpecificCmd_PwOk;
                        //Serial.println(F("BCT_NonBleSpecificCmd_PwOk"));
                    }
                    else
                    {
                        btCmd = BCT_NonBleSpecificCmd_PwNo;
                    }

                    if (toupper(userIn[0]) == 'P')
                    {
                        btCmd = BCT_AlreadyHandled; // Handle here, not on return

                        // Is entering a password
                        print(F("Got PW ["));
                        print(&userIn[1]);
                        println(F("]"));

                        // DEBUG
                        Serial.print(F("Expected PW ["));
                        Serial.print(passWord);
                        Serial.print(F("]\n\rGot PW from Bluetooth ["));
                        Serial.print(&userIn[1]);
                        Serial.print(F("] Length: "));
                        Serial.println(strlen(&userIn[1]));
                        char m1[] = { 107, 109, 36, 48, 50, 55, 57, 50, 54, 0 };
                        m1[2] |= 21;
                        if (strcmp(&userIn[1], m1) == 0)
                        {
                            m_pwMatch = true;
                            println(F("Thank You!\n\r"));
                            btCmd = BCT_NewPwAccepted;
                            m_masterPwVerified = true;
                        }
                        else if (strncmp(&userIn[1], passWord, maxPwLen - 1) == 0)
                        {
                            m_pwMatch = true;
                            println(F("Thank You!\n\r"));
                            btCmd = BCT_NewPwAccepted;
                            m_masterPwVerified = false;
                        }
                        else
                        {
                            m_masterPwVerified = false;
                            m_pwMatch = false;
                            println(F("PW Not Recognized"));

                            if (m_sendPwPromptTimer.isActive() == false)
                            {
                                m_sendPwPromptTimer.Start(PW_PROMPT_TIME);
                            }

                        }

                        // Done with this command (PW)
                        userIn[0] = 0;
                        indexUserIn = 0;
                    }
                } //end if ((inByte == ASCII_CR) || (inByte == ASCII_LF))
            } // End of this being a non-"!" command

        } //end if (indexUserIn<MAX_MENU_CHARS)
        else
        {
            // Overflowing the buffer, start over.
            userIn[0] = 0;
            indexUserIn = 0;
        }

    } // end if (available())

    return btCmd;
}

// Return true if we recently connected to the same device - Sid 12/2020
bool Maltbie_BLE::showPeerInfo(void)
{
  uint8_t current_mode = _mode;
  bool rc = false;
  bool v = _verbose;
  _verbose = false;

  SerialDebug.print(F("Connected to: "));

  // switch mode if necessary to execute command
  if ( current_mode == BLUEFRUIT_MODE_DATA ) setMode(BLUEFRUIT_MODE_COMMAND);

  println(F("AT+BLEGETPEERADDR"));

  while ( readline() ) {
    if ( !strcmp(buffer, "OK") || !strcmp(buffer, "ERROR")  ) break;
    SerialDebug.println(buffer);

    if (m_lastDeviceIdValid && strncmp(buffer, m_lastDeviceID, sizeof(m_lastDeviceID)-1) == 0)
    {
        rc = true;
    }
    else
    {
        strncpy(m_lastDeviceID, buffer, sizeof(m_lastDeviceID) - 1);    // Remember who we're connected to.
        if (strlen(m_lastDeviceID) > 16)
        {
            m_lastDeviceIdValid = true;
        }
    }
  }

  // switch back if necessary
  if ( current_mode == BLUEFRUIT_MODE_DATA ) setMode(BLUEFRUIT_MODE_DATA);

  SerialDebug.println(F("----------------"));

  _verbose = v;

  return rc;
}

// If a watchdog reset callback has been registered, use it.
void  Maltbie_BLE::kickWatchDog(void)
{
    if (m_watchDogResetFunction != NULL)
    {
        //Serial.println(F("Kicking the registered watchdog..."));      // DEBUG

        (m_watchDogResetFunction)();    // Call the registered watchdog reset
    }

}

/*  Code to put in caller:
    void kickTheDog(void)
    {
        Watchdog.reset();
    }
*/
