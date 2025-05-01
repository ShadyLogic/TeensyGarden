#include "TCPManager.h"
#include <Arduino.h>

TCPManager::TCPManager(qn::EthernetClient* client, const IPAddress& serverIP, uint16_t serverPort, bool enableKeepAlive)
    : client(client), serverIP(serverIP), serverPort(serverPort),
      lastAttemptTime(0), reconnectInterval(5000),
      lastLinkCheckTime(5000),
      previousLinkStatus(qn::EthernetLinkStatus::Unknown),
      lastKeepAliveTime(0), keepAliveInterval(10000),
      keepAliveMessage("PING\n"),
      keepAliveEnabled(enableKeepAlive),
      debugEnabled(true),
      keepAlivePending(false),
      keepAliveSentTime(0),
      keepAliveResponseTimeout(500) // 500 ms
{}

void TCPManager::handleConnection() {
    unsigned long now = millis();
    qn::EthernetLinkStatus link = qn::Ethernet.linkStatus();

    if (link != previousLinkStatus) {
        if (debugEnabled) {
            if (link == qn::EthernetLinkStatus::LinkON) {
                Serial.println("[TCPManager] Ethernet cable connected. Link is up.");
            } else if (link == qn::EthernetLinkStatus::LinkOFF) {
                Serial.println("[TCPManager] Ethernet cable disconnected. Link is down.");
            } else {
                Serial.println("[TCPManager] Ethernet link status unknown.");
            }
        }
        previousLinkStatus = link;
    }

    if (link != qn::EthernetLinkStatus::LinkON) {
        return;
    }

    if (!client->connected()) {
        keepAlivePending = false; // Clear pending if disconnected
        if (now - lastAttemptTime >= reconnectInterval) {
            client->stop();
            if (client->connect(serverIP, serverPort)) {
                if (debugEnabled) {
                    Serial.println("[TCPManager] Connection to server successful.");
                }
            } else {
                if (debugEnabled) {
                    Serial.println("[TCPManager] Connection to server failed. Will retry...");
                }
            }
            lastAttemptTime = now;
        }
    } 
    else 
    {
        if (keepAliveEnabled) {
            if (keepAlivePending) {
                if (client->available()) {
                    // Flush all available incoming data
                    while (client->available()) {
                        char response[10] = {0};
                        int bytesRead = client->readBytesUntil('\n', response, sizeof(response) - 1); // Read until we reach the terminating character, in this case '\n'.
                        if (bytesRead > 0) {
                            if (debugEnabled) {
                                unsigned long rtt = millis() - keepAliveSentTime;
                                Serial.print("[TCPManager] Keep-alive reply received (RTT = ");
                                Serial.print(rtt);
                                Serial.print(" ms): ");
                                Serial.println(response);
                            }
                        }
                    }
                    keepAlivePending = false; // Clear pending after flushing
                } else if (now - keepAliveSentTime >= keepAliveResponseTimeout) {
                    if (debugEnabled) {
                        Serial.println("[TCPManager] Keep-alive timeout. Server may be unreachable.");
                    }
                    client->stop();
                    keepAlivePending = false;
                }
            } else if (now - lastKeepAliveTime >= keepAliveInterval) {
                sendKeepAlive();
                lastKeepAliveTime = now;
            }
        }
    }
}

bool TCPManager::okToWrite() const {
    return client->connected() && client->availableForWrite();
}

void TCPManager::sendKeepAlive() {
    if (!keepAliveMessage) {
        if (debugEnabled) {
            Serial.println("[TCPManager] No keep-alive message set. Skipping.");
        }
        return;
    }

    if (!client->writeFully(keepAliveMessage, strlen(keepAliveMessage))) {
        if (debugEnabled) {
            Serial.println("[TCPManager] Keep-alive send failed. Server may be unreachable.");
        }
        client->stop();
        keepAlivePending = false;
    } else {
        if (debugEnabled) {
            Serial.println("[TCPManager] Keep-alive sent, waiting for reply...");
        }
        keepAlivePending = true;
        keepAliveSentTime = millis();
    }
}

void TCPManager::setKeepAliveInterval(unsigned long intervalMs) {
    keepAliveInterval = intervalMs;
}

void TCPManager::setReconnectInterval(unsigned long intervalMs) {
    reconnectInterval = intervalMs;
}

void TCPManager::enableDebug(bool enable) {
    debugEnabled = enable;
}

void TCPManager::setKeepAliveMessage(const char* message) {
    keepAliveMessage = message;
}

void TCPManager::enableKeepAlive(bool enable) {
    keepAliveEnabled = enable;
}
