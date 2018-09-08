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
    if (!bridge_id.length())
        bridge_id = "0";
    b_id = bridge_id;
    strcpy(ssid, ("mcs_" + bridge_id).c_str());

    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);

    connectHandler = WiFi.onStationModeConnected([&](const WiFiEventStationModeConnected &e) {
        Serial.println("WiFi connect");
        wifi_c = true;
        _wificonnect(e);

        if (WiFi.gatewayIP() != INADDR_NONE)
            ws_connect(WiFi.gatewayIP(), 80, "/ws");
        else
            ws_ticker.attach_ms<WebClient *>(500, [](WebClient *wc) {
                if (WiFi.gatewayIP() != INADDR_NONE)
                {
                    Serial.println("Hi!");
                    wc->ws_connect(WiFi.gatewayIP(), 80, "/ws");
                    wc->ws_ticker.detach();
                }
            },
                                             this);
    });
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
        if (!ws_c)
            break;

        if (bind)
        {
            bind_next();
        }
        else if (wifi_c)
            WiFi.disconnect();
        else
            _disconnect();
        ws_c = false;
        break;
    case WStype_CONNECTED:
        USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);
        ws_c = true;
        ws_ticker.detach();
        if (bind)
            web_ticker.detach();
        else
            _connect();
        break;
    case WStype_TEXT:
    {
        USE_SERIAL.printf("[WSc] get text: %s\n", payload);
        break;
    }
    case WStype_BIN:
        USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
        hexdump(payload, length);
        Serial.print("Get binary; length: ");
        Serial.println(length);
        Serial.println("content :");
        for (int i = 0; i < length; ++i)
        {
            Serial.print(i);
            Serial.print(": ");
            Serial.print(payload[i]);
            Serial.println();
        }

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
            {
                if (length > 1)
                    _vibroevent(payload[1]);
                else
                    _vibroevent(0);
            }
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
                uint16_t c[6] = {0};

                if (length > 3)
                    for (int i = 0; i < 3; ++i)
                        c[i] = payload[i + 1] * 4;
                else
                    break;

                if (length > 6)
                    for (int i = 3; i < 6; ++i)
                        c[i] = payload[i + 1] * 4;
                _changecolor(c);
            }
            break;
        //Led enable command
        case 0x55:
            if (_ledioevent)
                _ledioevent(true);
            break;
        //Led disable command
        case 0x56:
            if (_ledioevent)
                _ledioevent(false);
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
            Serial.println("Send MAC");
            sendMac();
            Serial.println("Send MAC end");
            break;
        //Bind accept command
        case 0xC8:
            if (bind && _changessid)
            {
                Serial.println("Accept bind command");
                String ssid = WiFi.SSID(i_ssid);
                if (!ssid.startsWith("mcsbnd_") || !ssid.length() > 7)
                    break;
                b_id = WiFi.SSID(i_ssid).substring(7);
                strcpy(this->ssid, ("mcs_" + b_id).c_str());
                _changessid(b_id);
                bind = false;
                _disconnect();
                connect();
            }
            break;
        //Bind reject command
        case 0xC9:
            if (bind)
            {
                Serial.println("Reject bind command");
                bind_next();
                // _disconnect();
            }
            break;
            //Reset command
        case 0x96:
        {
            if (length > 1)
                _restartevent(payload[1]);
            else
                _restartevent(0);
        }
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

void WebClient::onSSID(StringEvent event)
{
    _changessid = event;
}

void WebClient::onGetColor(Event event)
{
    _getcolor = event;
}

void WebClient::onVibro(IntEvent event)
{
    _vibroevent = event;
}

void WebClient::onLedIO(BoolEvent event)
{
    _ledioevent = event;
}

void WebClient::onAlarm(Event event)
{
    _alarmevent = event;
}

void WebClient::onRestart(IntEvent event)
{
    _restartevent = event;
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

void WebClient::sendBin(uint8_t *buf, size_t length, uint8_t command)
{
    if (!command)
    {
        webSocket.sendBIN(buf, length);
        return;
    }

    uint8_t *b = new uint8_t[length + 1];

    b[0] = command;
    memcpy(b + 1, buf, length);

    webSocket.sendBIN(b, length + 1);
    delete[] b;
}

void WebClient::sendTXT(String str)
{
    webSocket.sendTXT(str);
}

void WebClient::sendMac()
{
    uint8_t buf[7];
    buf[0] = 0x15;
    WiFi.macAddress(&(buf[0]) + sizeof(uint8_t));
    sendBin(buf, 7);
}

void WebClient::sendBridgeID()
{
    uint8_t buf[4];
    *(uint32_t *)buf = b_id.toInt();
    sendBin(buf, 4);
}

void WebClient::connect(bool bind_connection)
{
    connect(this->ssid, bind_connection);
}

void WebClient::connect(const char *ssid, bool bind_connection)
{
    Serial.print("Connecting to ");
    Serial.println(ssid);

    if (bind_connection)
    {
        Serial.println("Bind");

        disconnectHandler = WiFi.onStationModeDisconnected([&](const WiFiEventStationModeDisconnected &e) {
            Serial.println("WiFi disconnect");
            if (wifi_c)
            {
                Serial.println("WiFi disconnect accepted");
                ws_ticker.detach();
                web_ticker.detach();
                wifi_c = false;
                bind_next();
            }
        });

        web_ticker.once<WebClient *>(10, [](WebClient *wc) {
            Serial.println("Time is out");
            wc->bind_next();
        },
                                     this);
    }
    else
    {
        Serial.println("Normal");

        bind = false;

        disconnectHandler = WiFi.onStationModeDisconnected([&](const WiFiEventStationModeDisconnected &e) {
            Serial.println("WiFi disconnect 1");
            if (wifi_c)
            {
                Serial.println("WiFi disconnect accepted 1");
                ws_ticker.detach();
                web_ticker.detach();
                wifi_c = false;
                ws_c = false;
                _wifidisconnect(e);
            }
        });
    }
    WiFi.begin(ssid);

    //WebSocet connection
    //webSocket.begin(ip, port, url);
}

void WebClient::ws_connect(IPAddress host, uint16_t port, const char *url)
{
    Serial.print("ws_connect; host: ");
    Serial.println(host);
    webSocket.begin(host, port, url);
    webSocket.onEvent(std::bind(&WebClient::webSocketEvent, this,
                                std::placeholders::_1,
                                std::placeholders::_2,
                                std::placeholders::_3));
    webSocket.setReconnectInterval(5000);
    Serial.print("ws_connect_end");
}

bool WebClient::bind_connection()
{
    Serial.println("bind_connection start");
    int n = ssid_count = WiFi.scanNetworks(false, true);
    String ssid = "";
    //bool my_bind = false;

    Serial.print("ssid_count: ");
    Serial.println(ssid_count);

    for (int i = 0; i < n; ++i)
    {
        ssid = WiFi.SSID(i);
        if (strcmp(ssid.c_str(), this->ssid) == 0)
        {
            Serial.println("find my bridge");
            Serial.println("bind_connection end");
            return false;
        }

        //  if (ssid == "mcsbnd_" + b_id)
        //    my_bind = true;
    }

    for (i_ssid = 0; i_ssid < n; ++i_ssid)
    {
        ssid = WiFi.SSID(i_ssid);

        if (ssid.length() > 7 && ssid.startsWith("mcsbnd_"))
        {
            bind = true;
            i_ssid++;
            _bndevent();
            connect(ssid.c_str(), true);
            Serial.println("bind_connection end");
            return true;
        }
    }

    Serial.println("bind_connection end");
    return false;
}

void WebClient::bind_next()
{
    Serial.println("bind_next start");
    Serial.print("i_ssid: ");
    Serial.println(i_ssid);

    if (!bind)
        return;

    String ssid;

    for (; i_ssid < ssid_count; ++i_ssid)
    {
        ssid = WiFi.SSID(i_ssid);

        if (ssid.length() > 7 && ssid.startsWith("mcsbnd_"))
        {
            Serial.print("ssid found: ");
            Serial.println(ssid);
            i_ssid++;
            connect(ssid.c_str(), true);
            Serial.println("bind_next end");
            return;
        }
    }

    if (i_ssid == ssid_count)
    {
        Serial.println("i_ssid == ssid_count");
        bind = false;
        wifi_c = ws_c = false;
        WiFi.disconnect();
        _disconnect();
        connect();
    }
    Serial.println("bind_next end");
}