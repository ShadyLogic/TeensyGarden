// Maltbie Menu Functions
// Jacob Rogers, Kubik Maltbie

#include <Maltbie_Menu.h>

bool doDebugPrints = true;
EEPROM_Struct StoreEE;

char usedChars[14] = {'H','M','P','S','X','Y','Z','?','#','~','&','^','$'};

///////////////////////////////////////////////////
//   C O N S T R U C T O R   F U N C T I O N S   //
///////////////////////////////////////////////////

MenuOption::MenuOption(char comChar, String desc, uint8_t *i8, uint32_t uLim, uint32_t lLim, void (*f)())
{
    m_commandChar   =   comChar;
    m_description   =   desc;
    m_data.ptr.i8   =   i8;
    m_data.tag      =   DataTypes::i8;
    m_uLim          =   uLim;
    m_lLim          =   lLim;
    m_Function      =   f;
}

MenuOption::MenuOption(char comChar, String desc, uint16_t *i16, uint32_t uLim, uint32_t lLim, void (*f)())
{
    m_commandChar   =   comChar;
    m_description   =   desc;
    m_data.ptr.i16  =   i16;
    m_data.tag      =   DataTypes::i16;
    m_uLim          =   uLim;
    m_lLim          =   lLim;
    m_Function      =   f;
}

MenuOption::MenuOption(char comChar, String desc, uint32_t *ipad, IPAddress *ippointer, void (*f)())
{
    m_commandChar   =   comChar;
    m_description   =   desc;
    m_data.ptr.ipad =   ipad;
    m_data.tag      =   DataTypes::ipad;
    m_Function      =   f;
    m_ippointer     =   ippointer;
    
    *ippointer = IPAddress(*ipad);
}

MenuOption::MenuOption(char comChar, String desc, double *dub, long uLim, long lLim, void (*f)())
{
    m_commandChar   =   comChar;
    m_description   =   desc;
    m_data.ptr.dub  =   dub;
    m_data.tag      =   DataTypes::dub;
    m_uLim          =   uLim;
    m_lLim          =   lLim;
    m_Function      =   f;
}

MenuOption::MenuOption(char comChar, String desc, char* ch, void (*f)())
{
    m_commandChar   =   comChar;
    m_description   =   desc;
    m_data.ptr.ch   =   ch;
    m_data.tag      =   DataTypes::ch;
    m_Function      =   f;
}

MenuOption::MenuOption(char comChar, String desc, bool *boo, void (*f)())
{
    m_commandChar   =   comChar;
    m_description   =   desc;
    m_data.ptr.boo  =   boo;
    m_data.tag      =   DataTypes::boo;
    m_Function      =   f;
}

MenuOption::MenuOption(char comChar, String desc, void (*f)())
{
    m_commandChar   =   comChar;
    m_description   =   desc;
    m_Function      =   f;
    m_data.tag      =   DataTypes::func;
}

MenuOption::MenuOption(char comChar, String desc, void *data, bool flag)
{
    m_commandChar   =   comChar;
    m_description   =   desc;
    m_data.ptr.dat  =   data;
    m_data.tag      =   DataTypes::dat;
    m_Function      =   &unassignedCommand;
}

Menu::Menu(String title)
{
    m_title = title; 
    m_numOptions = 0;
}


/////////////////////////////////////////////////////////
//   E X T E N D E D   C O N S T   F U N C T I O N S   //
/////////////////////////////////////////////////////////

MenuOption* Menu::addOption(char comChar, String desc, uint8_t *i8, uint32_t uLim, uint32_t lLim, void (*f)())
{
    if (comCharAvailable(comChar))
    {
        return &(m_options[m_numOptions++] = MenuOption(comChar, desc, i8, uLim, lLim, f));
    } else {
        MH.serPtr()->print(F("DUPLICATE COMMAND CHAR: "));
        MH.serPtr()->println(comChar);
        for (;;);
    }
    return nullptr;;
}

MenuOption* Menu::addOption(char comChar, String desc, uint16_t *i16, uint32_t uLim, uint32_t lLim, void (*f)())
{
    if (comCharAvailable(comChar))
    {
        return &(m_options[m_numOptions++] = MenuOption(comChar, desc, i16, uLim, lLim, f));
    } else {
        MH.serPtr()->print(F("DUPLICATE COMMAND CHAR: "));
        MH.serPtr()->println(comChar);
        for (;;);
    }
    return nullptr;;
}

MenuOption* Menu::addOption(char comChar, String desc, uint32_t *ipad, IPAddress *ippointer, void (*f)())
{
    if (comCharAvailable(comChar))
    {
        return &(m_options[m_numOptions++] = MenuOption(comChar, desc, ipad, ippointer, f));
    } else {
        MH.serPtr()->print(F("DUPLICATE COMMAND CHAR: "));
        MH.serPtr()->println(comChar);
        for (;;);
    }
    return nullptr;;
}

MenuOption* Menu::addOption(char comChar, String desc, double *dub, long uLim, long lLim, void (*f)())
{
    if (comCharAvailable(comChar))
    {
        return &(m_options[m_numOptions++] = MenuOption(comChar, desc, dub, uLim, lLim, f));
    } else {
        MH.serPtr()->print(F("DUPLICATE COMMAND CHAR: "));
        MH.serPtr()->println(comChar);
        for (;;);
    }
    return nullptr;;
}

MenuOption* Menu::addOption(char comChar, String desc, char* ch, void (*f)())
{
    if (comCharAvailable(comChar))
    {
        return &(m_options[m_numOptions++] = MenuOption(comChar, desc, ch, f));
    } else {
        MH.serPtr()->print(F("DUPLICATE COMMAND CHAR: "));
        MH.serPtr()->println(comChar);
        for (;;);
    }
    return nullptr;;
}

MenuOption* Menu::addOption(char comChar, String desc, bool *boo, void (*f)())
{
    if (comCharAvailable(comChar))
    {
        return &(m_options[m_numOptions++] = MenuOption(comChar, desc, boo, f));
    } else {
        MH.serPtr()->print(F("DUPLICATE COMMAND CHAR: "));
        MH.serPtr()->println(comChar);
        for (;;);
    }
    return nullptr;;
}

MenuOption* Menu::addOption(char comChar, String desc, void (*f)())
{
    if (comCharAvailable(comChar))
    {
        return &(m_options[m_numOptions++] = MenuOption(comChar, desc, f));
    } else {
        MH.serPtr()->print(F("DUPLICATE COMMAND CHAR: "));
        MH.serPtr()->println(comChar);
        for (;;);
    }
    return nullptr;;
}

MenuOption* Menu::addOption(char comChar, String desc, void *data, bool flag)
{
    if (comCharAvailable(comChar))
    {
        return &(m_options[m_numOptions++] = MenuOption(comChar, desc, data, flag));
    } else {
        MH.serPtr()->print(F("DUPLICATE COMMAND CHAR: "));
        MH.serPtr()->println(comChar);
        for (;;);
    }
    return nullptr;;
}

MenuOption* Menu::addOption(MenuOption op)
{
    if (comCharAvailable(op.m_commandChar))
    {
        return &(m_options[m_numOptions++] = op);
        m_numOptions++;
    } else {
        MH.serPtr()->print(F("DUPLICATE COMMAND CHAR: "));
        MH.serPtr()->println(op.m_commandChar);
        for (;;);
    }
    return nullptr;;
}


/////////////////////////////////////////
//   M E T H O D   F U N C T I O N S   //
/////////////////////////////////////////

void MenuOption::doAction()
{
    if (m_Function != NULL)
    {
        (m_Function)();
    } else {
        MH.serPtr()->println(F("NO FUNCTION REGISTERED!"));
    }
}

void MenuOption::print(Stream *serialPtr)
{
    serialPtr->print(m_commandChar);
    serialPtr->print(F(" - "));
    serialPtr->print(m_description);
    if (m_data.tag == i8 || m_data.tag == i16)
    {
        serialPtr->print(F(" ["));
        serialPtr->print(m_lLim);
        serialPtr->print(F(" - "));
        serialPtr->print(m_uLim);
        serialPtr->print(F("]"));
    }
    m_data.tag != func ? serialPtr->print(F(": ")) : serialPtr->println();
    printValue();
}

void MenuOption::printValue(Stream *serialPtr)
{
    switch (m_data.tag)
    {
        case i8:
        {
            serialPtr->println((uint8_t)*m_data.ptr.i8, DEC);
        }
        break;

        case i16:
        {
            serialPtr->println((uint16_t)*m_data.ptr.i16, DEC);
        }
        break;

        case ipad:
        {
            printIPAddress(serialPtr, *m_data.ptr.ipad);
        }
        break;

        case dub:
        {
            serialPtr->println((long)*m_data.ptr.dub, DEC);
        }
        break;

        case ch:
        {
            serialPtr->println(m_data.ptr.ch);
        }
        break;

        case boo:
        {
            (bool)*m_data.ptr.boo ? serialPtr->println(F("TRUE")) : serialPtr->println(F("FALSE"));
        }
        break;

        case func:
        {
        }
        break;

        case dat:
        {
            serialPtr->println((char*)m_data.ptr.dat);
        }
        break;
    }
}

void MenuOption::dumpConfig(Stream *serialPtr)
{
    switch (m_data.tag)
    {
    case i8:
        serialPtr->print(m_commandChar);
        serialPtr->print(*m_data.ptr.i8);
        break;

    case i16:
        serialPtr->print(m_commandChar);
        serialPtr->print(*m_data.ptr.i16);
        break;

    case ipad:
        serialPtr->print(m_commandChar);
        printIPAddress(serialPtr, *m_data.ptr.ipad);
        break;

    case ch:
        serialPtr->print(m_commandChar);
        serialPtr->print(m_data.ptr.ch);
        break;

    case dub:
        serialPtr->print(m_commandChar);
        serialPtr->print(*m_data.ptr.dub);
        break;

    case boo:
        serialPtr->print(m_commandChar);
        *m_data.ptr.boo ? serialPtr->print(1) : serialPtr->print(0);
        break;

    case func:
        break;

    case dat:
        break;
    }
    serialPtr->print("\r\n");
}

void MenuOption::UpdateValue(char* val)
{
    if (m_data.tag == boo)
    {
        switch (toUpperCase(val[1]))
        {
            case ('T'):
            case ('1'):
                *m_data.ptr.boo = true;
                break;
            
            case ('F'):
            case ('0'):
                *m_data.ptr.boo = false;
                break;
            
            default:
                MH.serPtr()->println(F("INVALID PARAMS. ENTER ONLY THE COMMAND CHARACTER"));
                return;
                break;
        }
        MH.serPtr()->print(F("New Value: "));
        *m_data.ptr.boo ? MH.serPtr()->println(F("TRUE")) : MH.serPtr()->println(F("FALSE"));
        if (m_Function != NULL) doAction();
        return;
    }

    // Return if data type is a function
    if (m_data.tag == func){MH.serPtr()->println(F("INVALID PARAMS. ENTER ONLY THE COMMAND CHARACTER")); return;}

    if ((m_data.tag == i8) || (m_data.tag == i16) || (m_data.tag == dub))
    {
        double temp = atof(&val[1]);
        MH.serPtr()->print("NEW VALUE: ");
        MH.serPtr()->println(temp);

        if (temp < m_lLim)
        {
            MH.serPtr()->println(F("ERROR: VALUE LOWER THAN LOWER LIMIT"));
            return;
        }
        if (temp > m_uLim)
        {
            MH.serPtr()->println(F("ERROR: VALUE HIGHER THAN UPPER LIMIT"));
            return;
        }

        m_data.tag == i8  ? *m_data.ptr.i8  = (uint8_t)  temp : false;
        m_data.tag == i16 ? *m_data.ptr.i16 = (uint16_t) temp : false;
        m_data.tag == dub ? *m_data.ptr.dub = (double)   temp : false;
        if (m_Function != NULL) doAction();
        return;
    }

    if (m_data.tag == ipad)
    {
        byte tempIParray[4];
        unsigned int acc = 0; // Accumulator
	    unsigned int dots = 0;
        uint8_t i = 1;

        while (val[i]) {
            char c = val[i++];
            if (c >= '0' && c <= '9') {
                acc = acc * 10 + (c - '0');
                if (acc > 255) {
                    // Value out of [0..255] range
                    MH.serPtr()->println("ERROR: Value out of range");
                    return;
                }
            } else if (c == '.') {
                if (dots == 3) {
                    // Too much dots (there must be 3 dots)
                    MH.serPtr()->println("ERROR: Too many dots");
                    return ;
                }
                tempIParray[dots++] = acc;
                acc = 0;
            } else {
                // Invalid char
                MH.serPtr()->println("ERROR: Invalid character");
                return;
            }
        }
        if (dots != 3) {
            // Too few dots (there must be 3 dots)
            MH.serPtr()->println("ERROR: Too few dots");
            return;
        }
        tempIParray[3] = acc;

        *m_data.ptr.ipad = (uint32_t(tempIParray[3]) << 24) | (uint32_t(tempIParray[2]) << 16) | (uint32_t(tempIParray[1]) << 8) | uint32_t(tempIParray[0]);

        *m_ippointer = IPAddress(*m_data.ptr.ipad);
        
        MH.serPtr()->print("NEW VALUE: ");
        MH.serPtr()->print(tempIParray[0]);
        MH.serPtr()->print(".");
        MH.serPtr()->print(tempIParray[1]);
        MH.serPtr()->print(".");
        MH.serPtr()->print(tempIParray[2]);
        MH.serPtr()->print(".");
        MH.serPtr()->println(tempIParray[3]);
        if (m_Function != NULL) doAction();
        return;
    }
    
    if (m_data.tag == ch)
    {
        strncpy(m_data.ptr.ch, &val[1], MAX_CHAR-1);
        if (m_Function != NULL) doAction();
        return;
    }

    MH.serPtr()->println(F("ERROR: VALUE DOES NOT MATCH DATA TYPE"));
    return;
}

void MenuOption::UpdateValue(void *data, size_t size)
{
    memcpy(m_data.ptr.dat, data, size);
}

MenuOption Menu::GetOptionByComChar(char comChar)
{
    int nO = m_numOptions;
    for (int i = 0; i < nO; i++)
    {
        if (m_options[i].m_commandChar == comChar) 
            return m_options[i];
    }
    MH.serPtr()->println(F("COMMAND CHARACTER NOT FOUND"));
    return MenuOption();                                    // Just returns a new MenuOption, bad way to do this but it clears the warning
}

bool Menu::comCharAvailable(char comChar)
{

    for (unsigned long long i = 0; i < sizeof(usedChars); i++){

        DEBUG_PRINT(usedChars[i]);
        DEBUG_PRINT(F(" =/= "));
        DEBUG_PRINT(comChar);
        DEBUG_PRINT(F(": "));
        if (usedChars[i] == comChar)
        {
            DEBUG_PRINTLN(F("TRUE"));
        }
        else
        {
            DEBUG_PRINTLN(F("FALSE"));
        }

        if (usedChars[i] == comChar)
        {
            return false;
        }
    }


    for (int i = 0; i < m_numOptions; i++){

        DEBUG_PRINT(m_options[i].m_commandChar);
        DEBUG_PRINT(F(" =/= "));
        DEBUG_PRINT(comChar);
        DEBUG_PRINT(F(": "));
        if (m_options[i].m_commandChar == comChar)
        {
            DEBUG_PRINTLN(F("TRUE"));
        }
        else
        {
            DEBUG_PRINTLN(F("FALSE"));
        }

        if (m_options[i].m_commandChar == comChar)
        {
            return false;
        }
    }
    return true;
}

void Menu::printOptions(Stream *serialPtr)
{
    for (int i = 0; i < m_numOptions; i++){
        if (m_options[i].m_description != "")
        {
            m_options[i].print(serialPtr);
        }
    }
}

void Menu::dumpConfig(Stream *serialPtr)
{
    for (int i = 0; i < m_numOptions; i++){
        m_options[i].dumpConfig(serialPtr);
    }
}

// DUMPS CONFIGURATION SETTINGS IN SERIAL
void MenuManager::dumpConfigCommands(Stream *serialPtr, bool handleUsbOnlyOptions)
{
    m_menuPtr->dumpConfig(serialPtr);
}

void MenuManager::printMenu()
{
    if (m_menuPtr != NULL)
    {
        m_menuPtr->printTitle(MH.serPtr());
        m_menuPtr->printOptions(MH.serPtr());
    }
}

void MenuManager::printHelp(Stream *serialPtr, bool handleUsbOnlyOptions)
{
   //showPersistentStrikerSettings(serialPtr, handleUsbOnlyOptions);
    serialPtr->println(F(EXHIBIT_NAME_VERSION));
    serialPtr->println(F("Compiled on: " __DATE__ ", at " __TIME__));

    check_mem();

   serialPtr->print(F(
            "M  - Display Menu Commands\n"
            "S  - Save Current Settings to EEPROM\n"
            "~  - Print Config (EEPROM) in Hex\n"
            "~C - Print Config as Command List\n"
            "#  - Force Exhibit Reboot\n"
            "X  - Reset Settings to Factory Defaults\n"));

#if ENABLE_BLE
   if (handleUsbOnlyOptions)
   {
       serialPtr->print(F("Y  - Reset BLE Name/PW to Factory Defaults\n"
                            "Z  - Reset All BLE Settings to Factory Defaults\n"
                            "$  - Save BLE Name/PW\n"
                            "^  - Set BLE Name [^NAME]\n"
                            "&  - Set BLE Password [&PASS]"));
   }
#endif
    serialPtr->println();

   if (isConfigEEPromMismatch(OFFSET_STOREEE, (uint8_t *)&StoreEE, sizeof(StoreEE)))
   {
       serialPtr->println(F("Config Changes Not Saved to EEPROM! (Enter \"S\")"));
   }
#if ENABLE_BLE
   if (isConfigEEPromMismatch(0, (uint8_t *)&StoreEE_BLE, sizeof(StoreEE_BLE)))
   {
       serialPtr->println(F("BlueTooth Changes Not Saved to EEPROM! (Enter \"(\")"));
   }
#endif //ENABLE_BLE
}

void MenuManager::processUserInput(char userIn[MAX_MENU_CHARS], uint8_t& indexUserIn, int inByte, Stream *serialPtr, bool handleUsbOnlyOptions)
{
    setSerialOutputStream(serialPtr);    // Send serial output to Serial or to BlueTooth

    if (((inByte == ASCII_CR) || (inByte == ASCII_LF)) && (indexUserIn > 0))
    {
        WATCHDOG_RESET;   // Reset watchdog frequently - in case we're pounding with commands

        MH.serPtr()->println('\n');

        char cmdChar = (char)toupper(userIn[0]);
        switch (cmdChar)
        {
        case 'H':
        case '?':
            {
                printHelp(serialPtr, handleUsbOnlyOptions);
            }
            break;
        
        case 'S':
            {
                EEPROM.put(OFFSET_STOREEE, StoreEE);     // Write to EEPROM at users command
                serialPtr->println(F("*** SAVED NEW VALUES TO EEPROM ***"));
            }
            break;

        case '#':
            {
                watchDogReset();
            }
            break;

        case '~':
            {
                if (inByte == ASCII_CR)
                {
                    //Serial.println("toupper(userIn[1]) 0x" + String(toupper(userIn[1]), HEX));
                    if ('C' == toupper(userIn[1]))
                    {
                        // Dump commands to make enter the settings
                        dumpConfigCommands(serialPtr, handleUsbOnlyOptions);
                    }
                    else
                    {
                        dumpEEProm(serialPtr);    // Dump the EEPROM
                    }
                    indexUserIn = 0;
                }
            }
            break;
        
        case 'X':
            {
                FactoryDefault();
            }
            break;

#if ENABLE_BLE
        case '&':
            {
                if (handleUsbOnlyOptions)
                {
                    if (strnlen(&userIn[1], MAX_PW) < (MAX_PW - 2))
                    {
                        strncpy(StoreEE_BLE.blePassword, &userIn[1], MAX_PW - 1);  // BLE must enter a pw, and match this. Must not be longer than
                        StoreEE_BLE.blePassword[MAX_PW - 1] = 0;    // ensure last char in string is null
                        serialPtr->print(F("\nNew Password: ["));
                        serialPtr->print(StoreEE_BLE.blePassword);
                        serialPtr->print(F("] - Length: "));
                        serialPtr->print(strlen(StoreEE_BLE.blePassword));
                        serialPtr->println(F("\n*** REMEMBER TO USE '$' TO SAVE CHANGES TO EEPROM ***\n"));
                    }
                    else
                    {
                        serialPtr->println(F("\nERROR: New Password Too Long."));
                    }
                }
                else
                {
                    serialPtr->println(F("\nERROR: Change Name Using USB Cable."));
                }
            }
            break;

        case '^':
            {
                if (handleUsbOnlyOptions)
                {
                    if (strnlen(&userIn[1], MAX_BT_NAME) < (MAX_BT_NAME - 2))
                    {
                        strncpy(StoreEE_BLE.bleName, &userIn[1], MAX_BT_NAME - 1);  // BLE name
                        StoreEE_BLE.bleName[MAX_PW - 1] = 0;    // ensure last char in string is null
                        serialPtr->print(F("\nNew Bluetooth Name: ["));
                        serialPtr->print(StoreEE_BLE.bleName);
                        serialPtr->print(F("]"));
                        //serialPtr->print(F("] length "));
                        //serialPtr->print(strlen(StoreEE_BLE.bleName));
                        serialPtr->println(F("*** REMEMBER TO USE '$' TO SAVE CHANGES TO EEPROM ***"));
                    }
                    else
                    {
                        serialPtr->println(F("\nERROR: New Name Too Long."));
                    }
                }
                else
                {
                    serialPtr->println(F("\nERROR: Change Name Using USB Cable."));
                }
            }
            break;

        case '$':
            {
                if (handleUsbOnlyOptions)
                {
                    EEPROM.put(0, StoreEE_BLE);     // Write to EEPROM at users command
                    serialPtr->println(F("\nSaved BLE name and PW to EEPROM.\n"));
                }
            }
            break;

        case 'Y':
            {
                if (handleUsbOnlyOptions)
                {
                    FactoryDefault_BLE();   // Reset device name and password
                    serialPtr->println(F("\nRestored Bluetooth Name and Password to Factory Default"));
                    serialPtr->println(F("*** REMEMBER TO USE '$' TO SAVE CHANGES TO EEPROM ***"));
                }
            }
            break;

        case 'Z':
            {
                serialPtr->print(F("Initialising the Bluefruit \nLE module with factory defaults... "));
                blueToothSerial.kickWatchDog();
                serialPtr->flush();
                delay(500);
                blueToothSerial.kickWatchDog();
                blueToothSerial.init_BLE_Shield(StoreEE_BLE.bleName, true);  // Do factory reset of the BT module (for init after replacement/initial)
            }
            break;

#endif //end #if ENABLE_BLE

        case 'M':
        {
            printMenu();
        }

        case ASCII_CR:
            indexUserIn = 0;
        break;

        default:
            MH.setSerialOutputStream(serialPtr);
            if (m_menuPtr->comCharAvailable(cmdChar))
            {
                serialPtr->println(F("\n***Unknown Command***"));
                break;
            }
            MenuOption tempOption = m_menuPtr->GetOptionByComChar(cmdChar);     // Doing it this way prevents the "Error: taking address of temporary [-fpermissive]" warning
            MenuOption *temp = &tempOption;
            if (userIn[1] != 0)
            {
                indexUserIn = 0;
                userIn[0] = 0;
                temp->UpdateValue(userIn);
            } else if (temp->m_data.tag == boo) {
                bool prevVal = *temp->m_data.ptr.boo;
                *temp->m_data.ptr.boo = !prevVal;
                MH.serPtr()->print(F("New Value: "));
                *temp->m_data.ptr.boo ? serialPtr->println(F("TRUE")) : serialPtr->println(F("FALSE"));
                if (temp->m_Function != NULL) temp->doAction();
            } else {
                indexUserIn = 0;
                temp->doAction();
            }
            // MH.resetSerialOutputStream();
        }
        indexUserIn = 0;
    } // end if ((inByte == ASCII_CR) || (inByte == ASCII_LF))
}

void MenuManager::handleLaptopInput(void)
{
#define ASCII_CR            0x0D                // ASCII code for Carriage Return   <CR>
#define ASCII_LF            0x0A                // ASCII code for Line Feed         <LF>
#define ASCII_ESC           0x1B                // ASCII code for Escape            <ESC>

    WATCHDOG_RESET

#if ENABLE_BLE
    if (blueToothSerial.isConnected())          // Checks if bluetooth is connected and switches the Serial output
	{
		MH.setSerialOutputStream(&blueToothSerial);
	} else {
		MH.resetSerialOutputStream();
	}
#endif

    static char userIn[MAX_MENU_CHARS];         // MAX_MENU_CHARS Def'd in Maltbie_Helper.h
    static uint8_t indexUserIn = 0;
    int inByte = 0;

    while (Serial.available())                  // Read the Serial buffer
    {
        inByte = Serial.read();
        Serial.write(inByte);                // Prints the command to Serial one byte at a time

        if (indexUserIn < MAX_MENU_CHARS)
        {
            if (!((inByte == ASCII_CR) || (inByte == ASCII_LF)))
            {
                userIn[indexUserIn++] = inByte; // Append the latest character, unless it is a CR or LF
            }

            userIn[indexUserIn] = 0;            // Sets the next byte to NULL

            processUserInput(userIn, indexUserIn, inByte, MH.serPtr());     // Attempts to processUserInput after Serial.read()ing each byte, but it only runs when inByte is <CR> or <LF>
        } else {
            indexUserIn = 0;                    
            userIn[0] = 0;                      // Attempted to overflow the input buffer, reset it.
        }

        if (indexUserIn == 0)
        {
            // Serial.print(F("\nCmd: "));              
        }
    } // end while (Serial.available())
#if ENABLE_BLE

    //////////////////////////
    // Bluetooth Comm:
    //
    uint32_t newColor = 0x000000;
    //bool doShowNewPos = false;

    //if (timerON.notAlmostDone(20))
    //if (false)  //  DOYET DEBUG
    if (blueToothSerial.isWorking())   // Don't call below if we can't talk to BLE shield (not installed/failed)
    {
        // BLE_Comm returns BCT_NONE if no characters or the command isn't complete yet
        ble_Cmd_Type btCmd = blueToothSerial.BLE_Comm(userIn, indexUserIn, MAX_MENU_CHARS, inByte, StoreEE_BLE.bleName, StoreEE_BLE.blePassword,  MAX_PW, newColor);
        switch (btCmd)
        {
            case BCT_NewPwAccepted:
            {
                setSerialOutputStream(&blueToothSerial);    // Send serial output to Serial or to BlueTooth
                printHelp(&blueToothSerial, blueToothSerial.wasMasterPwEntered());
                resetSerialOutputStream();   // Start using "Serial" again
            }
            break;

            case BCT_ArrowPressedUp:
            {
                //if (StoreEE.ledPosHalfLit < (MAXLED_NEO - 2))
                //{
                //    StoreEE.ledPosHalfLit++;
                //}
                //doShowNewPos = true;
            }
            break;

            case BCT_ArrowPressedDown:
            {
                //if (StoreEE.ledPosHalfLit >= 1)
                //    {
                //    StoreEE.ledPosHalfLit--;
                //    }
                //doShowNewPos = true;
            }
            break;

            case BCT_Color:
                {
                    // StoreEE.colorTube = MH.reduceToMaxIntensity(newColor, StoreEE.maxIntensity);
                    // EEPROM.put(OFFSET_STOREEE, StoreEE);     // Write to EEPROM
                    // Serial.println(F("Saved New Color to EEPROM.\n"));
                }
                break;

            case BCT_AlreadyHandled:
            {
                // blueToothSerial.print(F("Cmd: "));
            }
            break;

            case BCT_NonBleSpecificCmd_PwOk:
            {
                DEBUG_PRINT(F("Entry: ["));
                DEBUG_PRINT(userIn);
                DEBUG_PRINT(F("] indexUserIn: "));
                DEBUG_PRINT(indexUserIn);
                DEBUG_PRINT(F(", last char: "));
                DEBUG_PRINT(inByte);
                DEBUG_PRINT(F(", as hex:0x"));
                DEBUG_PRINT(inByte, HEX);
                DEBUG_PRINTLN();

                processUserInput(userIn, indexUserIn, inByte, &blueToothSerial, blueToothSerial.wasMasterPwEntered()); // Process a command as if entered via the serial port (but not USB only cmds)
                resetSerialOutputStream();   // Start using "Serial" again
            }
            break;

            case BCT_NonBleSpecificCmd_PwNo:
            {
                MH.serPtr()->println(F("ERROR: Enter Password First With 'P' Command [Ppassword]"));
                userIn[0] = 0;
                indexUserIn = 0;
            }
            break;

            case BCT_NONE:
            break;

            default:
            {
                MH.serPtr()->println(F("PROG Error: Unknown btCmd:"));
                MH.serPtr()->println(btCmd);
            }
            break;
        }
    }
#endif //end #if ENABLE_BLE
}


void kickTheDog()   // Use this function for registering a watch dog kicker with BLE
{
    WATCHDOG_RESET
}

// PUT FACTORY DEFAULT FUNCTIONS HERE
void MenuManager::FactoryDefault(void)
{
    MH.serPtr()->println(F("RESETTING EEPROM TO FACTORY DEFAULTS"));
    memcpy(&StoreEE, &STOREEE_DEFAULTS, sizeof(STOREEE_DEFAULTS));
}


void MenuManager::FactoryDefault_BLE(void)
{
#if ENABLE_BLE
    strncpy(StoreEE_BLE.blePassword, BT_PW_DEFAULT, MAX_PW - 1);  // BLE must enter a pw, and match this. Must not be longer than
    StoreEE_BLE.blePassword[MAX_PW - 1] = 0;    // ensure last char in string is null
    strncpy(StoreEE_BLE.bleName, THIS_DEVICE_NAME, MAX_BT_NAME - 1);  // BLE Name
    StoreEE_BLE.bleName[MAX_BT_NAME - 1] = 0;    // ensure last char in string is null

    StoreEE_BLE.eeValid_1 = EEPROM_VALID_BLE_PAT1;        // Set sig in case user stores config to EEPROM.
    StoreEE_BLE.eeValid_2 = EEPROM_VALID_BLE_PAT2;
    StoreEE_BLE.eeVersion = EEPROM_END_VER_SIG_BLE;
#endif
}

void MenuManager::setupBLE(void)
{
#if ENABLE_BLE
    EEPROM.get(0, StoreEE_BLE); // Fetch our structure of non-volitale BLE vars from EEPROM

    if ((StoreEE_BLE.eeValid_1 == EEPROM_VALID_BLE_PAT1) && (StoreEE_BLE.eeValid_2 == EEPROM_VALID_BLE_PAT2) && (StoreEE_BLE.eeVersion == EEPROM_END_VER_SIG_BLE)) // Signature Valid?
    {
        MH.serPtr()->println(F("Good: BLE Name/PW EEPROM is Initialized."));
    }
    else
    {
        MH.serPtr()->println(F("ERROR: Need to Configure and Write BLE Name and Pass to EEPROM. Using Defaults."));
        FactoryDefault_BLE();
    }

    blueToothSerial.registerWatchDogReset(kickTheDog);
    blueToothSerial.init_BLE_Shield(StoreEE_BLE.bleName, true);     // Reset to factory defaults, write name to it.

    WATCHDOG_RESET;
#endif //end #if ENABLE_BLE
}


void MenuManager::setupEEPROM(void)
{
    EEPROM.get(OFFSET_STOREEE, StoreEE); // Fetch our structure of non-volitale vars from EEPROM
    if (StoreEE.eeValid_1 != EEPROM_VALID_PAT1 || StoreEE.eeValid_2 != EEPROM_VALID_PAT2 || StoreEE.eeVersion != EEPROM_END_VER_SIG)
    {
        MH.serPtr()->println(F("*** ERROR: EEPROM could not be loaded. Loading Defaults... ***"));
        FactoryDefault();
    }
    MH.serPtr()->println(F("*** EEPROM Successfully Loaded ***"));
}


void MenuManager::startWatchdog(uint8_t pin)
{
#if USE_WATCHDOG
    #if TEENSYDOG
        WDT_T4<WDT1> wdt;
        WDT_timings_t wdt_config;   
        wdt_config.trigger = 5;     // in seconds, 0->128
        wdt_config.timeout = 10;    // in seconds, 0->128
        wdt_config.pin = pin;       // default = 0
        wdt.begin(wdt_config);
    #endif
#endif
}


void MenuManager::watchDogReset(void)
{
#if USE_WATCHDOG
    #if TEENSYDOG
        MH.serPtr()->println(F("Forcing Reset via Watch Dog"));
        delay(100);
        wdt.reset();
        MH.serPtr()->println(F("IF YOU SEE THIS THE WATCHDOG FAILED TO RESET"));

    #else
        MH.serPtr()->println(F("Forcing Reset via Watch Dog"));
        int countdownMS = Watchdog.enable(200);
        MH.serPtr()->print(F("Enabled the Watchdog Countdown of "));
        MH.serPtr()->print(countdownMS, DEC);
        MH.serPtr()->println(F(" Milliseconds!"));
        MH.serPtr()->println();
        uint32_t startWDR = (uint32_t)millis();

        while (true)
        {
            delay(10);
            MH.serPtr()->print(millis() - startWDR, DEC);
            MH.serPtr()->println(F(" Milliseconds Since Shortening WD"));
        }
    #endif
#else
    MH.serPtr()->println(F("\n>>>>>>>>>>>>>>>>>>  Watch dog NOT available! <<<<<<<<<<<<<<<<<<<<<<<\n"));
#endif
}

void printIPAddress(Stream* s, uint32_t ip) {
    s->print(ip & 0xFF); s->print(".");
    s->print((ip >> 8) & 0xFF); s->print(".");
    s->print((ip >> 16) & 0xFF); s->print(".");
    s->println((ip >> 24) & 0xFF);
}


void unassignedCommand()
{
    MH.serPtr()->println(F("NO FUNCTION REGISTERED"));
}