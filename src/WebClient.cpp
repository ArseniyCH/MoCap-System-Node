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

#define USE_SERIAL Serial

WebClient::WebClient(String bridge_id)
{
    b_id = bridge_id.toInt();
    if (!bridge_id.length())
        bridge_id = "0";
    strcpy(ssid, ("mcs_" + bridge_id).c_str());
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
        ws_c = false;
        _disconnect();
        break;
    case WStype_CONNECTED:
        USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);
        ws_c = true;
        _connect();
        // //10 seconds whois timeout
        // web_ticker.once<WebClient *>(10, [](WebClient *wc) {
        //     wc->webSocket.disconnect();
        // },
        //                             this);
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
                // Serial.print("Parse RGB: { ");
                // Serial.print(rgb[0]);
                // Serial.print(", ");
                // Serial.print(rgb[1]);
                // Serial.print(", ");
                // Serial.print(rgb[2]);
                // Serial.println("};");
                // uint16_t s[3]{0, 0, 0};
                // _changecolor(rgb, s);
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
        {
            // web_ticker.detach();
            sendMac();
        }

        break;
    }
    case WStype_BIN:
        USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
        hexdump(payload, length);

        switch (payload[0])
        {
        //Calibration command
        case 0x64:
            if (_calibevent)
                _calibevent();
            break;
        //Vibration command
        case 0x5A:
            if (_vibroevent)
                _vibroevent();
            break;
        //Start MoCap command
        case 0xB:
            if (_startevent)
                _startevent();
            break;
        //Stop MoCap command
        case 0xC:
            if (_stopevent)
                _stopevent();
            break;
        //Get LED colors command
        case 0x50:
            if (_getcolor)
                _getcolor();
            break;
        //Set LED colors command
        case 0x51:
            if (_changecolor)
            {
                uint16_t f[3]{payload[1], payload[2], payload[3]};
                uint16_t s[3]{0, 0, 0};
                _changecolor(f, s);
            }
            break;
        //Alarm command
        case 0x59:
            if (_alarmevent)
                _alarmevent();
            break;
        //Get bridge ID command
        case 0x14:
            sendBridgeID();
            break;
        //Get MAC command
        case 0x15:
            sendMac();
            break;
        }
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

void WebClient::onGetColor(Event event)
{
    _getcolor = event;
}

void WebClient::onVibro(Event event)
{
    _vibroevent = event;
}

void WebClient::onAlarm(Event event)
{
    _alarmevent = event;
}

void WebClient::onConnect(Event event)
{
    _connect = event;
}

void WebClient::onDisconnect(Event event)
{
    _wifidisconnect = std::bind(event);
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

void WebClient::sendMac()
{
    uint8_t *buf;
    sendBin(WiFi.macAddress(buf), 6);
}

void WebClient::sendBridgeID()
{
    uint8_t buf[4];
    *(uint32_t *)buf = b_id;
    sendBin(buf, 4);
}

void WebClient::connect()
{
    Serial.print("Connecting to ");
    Serial.print(ssid);
    //WiFi connection
    bind = false;
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    connectHandler = WiFi.onStationModeConnected([&](const WiFiEventStationModeConnected &e) {
        wifi_c = true;
        _wificonnect(e);
    });
    disconnectHandler = WiFi.onStationModeDisconnected([&](const WiFiEventStationModeDisconnected &e) {
        if (wifi_c)
        {
            wifi_c = false;
            ws_c = false;
            _wifidisconnect(e);
        }
    });
    WiFi.begin(ssid);

    //WebSocet connection
    webSocket.begin(ip, port, url);
    webSocket.onEvent(std::bind(&WebClient::webSocketEvent, this,
                                std::placeholders::_1,
                                std::placeholders::_2,
                                std::placeholders::_3));
    webSocket.setReconnectInterval(3000);
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