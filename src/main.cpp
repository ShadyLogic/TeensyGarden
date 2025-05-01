#include <Arduino.h>

#include <Maltbie_Menu.h>
#include <Maltbie_Timer.h>

#include <TCPManager.h>

MenuManager MM;
Maltbie_Helper MH;
Menu MainMenu("**** Welcome to the Ethernet Test Menu ****");

namespace qn = qindesign::network;

qn::EthernetUDP udp;
qn::EthernetClient client;

TCPManager TM(&client, IPAddress(10,10,0,50), 50000);

uint8_t mac[6];
IPAddress deviceIP;
IPAddress gateway;
IPAddress subnet;
IPAddress dns;
IPAddress remoteIP;


void setup() 
{
	Serial.begin(115200);
	delay(100);
	MM.setupBLE();
	MM.setupEEPROM();
	MM.registerMenu(&MainMenu);
    MM.startWatchdog(20);

	MainMenu.addOption('A', "Device IP", &StoreEE.deviceIP, &deviceIP);
	MainMenu.addOption('B', "Gateway IP", &StoreEE.gateway, &gateway);
	MainMenu.addOption('C', "Subnet IP", &StoreEE.subnet, &subnet);
	MainMenu.addOption('D', "DNS IP", &StoreEE.dns, &dns);
	MainMenu.addOption('E', "Remote IP", &StoreEE.remoteIP, &remoteIP);
	MainMenu.addOption('F', "Remote Port", &StoreEE.remotePort);

    qn::Ethernet.macAddress(mac);
    qn::Ethernet.begin(mac, deviceIP, dns, gateway, subnet);
    delay(1000);
    TM.setKeepAliveMessage(StoreEE.pingMessage);
    TM.enableDebug(StoreEE.tcpDebugMessages);

    MM.printHelp(&Serial, true);

    WATCHDOG_RESET
}

void loop() 
{
	WATCHDOG_RESET
	MM.handleLaptopInput();
    TM.handleConnection();
}