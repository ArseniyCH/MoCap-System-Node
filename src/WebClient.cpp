/**
 * @brief Class to messaging with server realization
 * 
 * @file WebClient.cpp
 * @author Arseniy Churin
 * @date 2018-05-18
 */

#include "WebClient.h"
#include <WebSocketsClient.h>

#include <ESP8266WiFi.h>

#include <WString.h>
#include <Arduino.h>
#include "EEPROM.hpp"

#define USE_SERIAL Serial

WebClient::WebClient()
{
    ReadString(0);
    ssid = eRead;
}

void WebClient::loop()
{
    webSocket.loop();
}

bool WebClient::isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

bool WebClient::isWS()
{
    webSocket.sendPing();
}

void WebClient::webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
        USE_SERIAL.printf("[WSc] Disconnected!\n");
        _disconnect();
        break;
    case WStype_CONNECTED:
        USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);
        _connect();
        break;
    case WStype_TEXT:
    {
        USE_SERIAL.printf("[WSc] get text: %s\n", payload);

        /*
*007 - bind_accept
*101 - start command
*010 - stop command
*777 - calibiration command
*/
        if (strchr((char *)payload, ',') != NULL)
        {
            char buf[strlen((char *)payload)];
            char *p = buf;
            char *str;
            int16_t rgb[3];
            uint8_t i = 0;
            str = strtok_r((char *)payload, ",", &p);
            while (str != NULL && i < 3)
            {
                rgb[i++] = atoi(str);
                str = strtok_r(NULL, ",", &p);
            }

            if (i = 3)
            {
                Serial.print("Parse RGB: { ");
                Serial.print(rgb[0]);
                Serial.print(", ");
                Serial.print(rgb[1]);
                Serial.print(", ");
                Serial.print(rgb[2]);
                Serial.println("};");
                _changecolor(rgb[0], rgb[1], rgb[2]);
            }
        }

        // if (bind && strcmp("007", (char *)payload) == 0)
        //     _bndevent();

        if (strcmp("101", (char *)payload) == 0)
            _startevent();
        if (strcmp("010", (char *)payload) == 0)
            _stopevent();
        if (strcmp("777", (char *)payload) == 0)
            _calibevent();
        if (strcmp("whois", (char *)payload) == 0)
            sendId();

        break;
    }
    case WStype_BIN:
        USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
        hexdump(payload, length);
    }
}

void WebClient::onBind(Event event)
{
    _bndevent = event;
}

void WebClient::onStart(Event event)
{
    _startevent = event;
}

void WebClient::onStop(Event event)
{
    _stopevent = event;
}

void WebClient::onCalibirate(Event event)
{
    _calibevent = event;
}

void WebClient::onColor(ColorEvent event)
{
    _changecolor = event;
}

void WebClient::onConnect(Event event)
{
    _connect = event;
}

void WebClient::onDisconnect(Event event)
{
    _disconnect = event;
}

void WebClient::sendBin(uint8_t *buf, size_t length)
{
    webSocket.sendBIN(buf, length);
}

void WebClient::sendTXT(String str)
{
    webSocket.sendTXT(str);
}

void WebClient::sendId()
{
    sendTXT("id:" + WiFi.macAddress());
}

void WebClient::websockets_connection()
{
    //webSocket.begin(ip, port, url);
    webSocket.begin("192.168.4.1", 80, "/ws");

    webSocket.onEvent(std::bind(&WebClient::webSocketEvent, this,
                                std::placeholders::_1,
                                std::placeholders::_2,
                                std::placeholders::_3));

    webSocket.setReconnectInterval(5000);
}

void WebClient::wifi_connection()
{
    bind = false;
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid);

    // while (WiFi.status() != WL_CONNECTED)
    // {
    //     delay(500);
    //     Serial.print(".");
    // }
}

bool WebClient::bind_connection()
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    int n = WiFi.scanNetworks(false, true);

    for (int i = 0; i < n; ++i)
    {
        if (WiFi.SSID(i).equals("mcsbnd"))
        {
            bind = true;
            bindStartTime = millis();
            WiFi.begin("mcsbnd");
            return true;
        }
    }

    return false;
}