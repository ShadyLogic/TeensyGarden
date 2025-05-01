/*
 * TCPManager - A lightweight TCP connection watchdog for Teensy 4.1 + QNEthernet
 * Author: Jacob Rogers, ChatGPT, Kubik Maltbie
 * 
 * Description:
 * ------------
 * TCPManager provides reliable, non-blocking management of a persistent TCP connection
 * to a remote server using the QNEthernet library on a Teensy 4.1.
 * 
 * Features:
 * - Monitors Ethernet link status (cable connected/disconnected)
 * - Automatically attempts reconnects when the connection is lost
 * - Sends periodic application-layer keep-alive packets ("PING") to detect dead servers
 * - Configurable keep-alive and reconnect intervals
 * - Optional debug logging to Serial
 * - Lightweight and suitable for use in main loop or timer-based environments
 * 
 * Usage:
 * ------
 * 1. Instantiate with a pointer to a qn::EthernetClient, server IP, and port
 * 2. Call handleConnection() regularly from your main loop
 * 3. Use okToWrite() to determine if it's safe to send data
 * 
 * Example:
 * --------
 * TCPManager tcpManager(&client, IPAddress(192,168,1,100), 12345);
 * 
 * void loop() {
 *     tcpManager.handleConnection();
 *     if (tcpManager.okToWrite()) {
 *         client.println("Hello Server!");
 *     }
 * }
 */


#pragma once

#include <QNEthernet.h>
namespace qn = qindesign::network;

/**
 * @brief TCPManager - A lightweight TCP connection watchdog for Teensy 4.1 + QNEthernet.
 * 
 * Manages a persistent TCP connection, handles Ethernet link loss, 
 * reconnects if the connection is lost, sends optional application-layer keep-alive messages, 
 * and optionally validates server responses.
 */
class TCPManager {
private:
    qn::EthernetClient* client;
    IPAddress serverIP;
    uint16_t serverPort;
    unsigned long lastAttemptTime;
    unsigned long reconnectInterval;
    unsigned long lastLinkCheckTime;
    qn::EthernetLinkStatus previousLinkStatus;

    unsigned long lastKeepAliveTime;
    unsigned long keepAliveInterval;

    const char* keepAliveMessage;
    bool keepAliveEnabled;
    bool debugEnabled;

    bool keepAlivePending;
    unsigned long keepAliveSentTime;
    unsigned long keepAliveResponseTimeout;

    void sendKeepAlive();

public:
    TCPManager(qn::EthernetClient* client, const IPAddress& serverIP, uint16_t serverPort, bool enableKeepAlive = true);

    void handleConnection();
    bool okToWrite() const;

    void setKeepAliveInterval(unsigned long intervalMs);
    void setReconnectInterval(unsigned long intervalMs);
    void enableDebug(bool enable);
    void setKeepAliveMessage(const char* message);
    void enableKeepAlive(bool enable);
};
