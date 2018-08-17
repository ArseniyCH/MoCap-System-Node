/**
 * @brief Class to messaging with server
 * 
 * Connection to Access point and WS
 * 
 * @file WebClient.h
 * @author Arseniy Churin
 * @date 2018-05-18
 */

#ifndef CLIENT_H
#define CLIENT_H

#include <ESP8266WIFI.h>
#include <WebSocketsClient.h>

#include <WString.h>
#include <Ticker.h>

#define BIND_BIN (uint8_t *)"bndcheck", 8

//Command defines
#define MPU_DATA 0xA
#define COLORS 0x50
#define BRIDGE_ID 0x14
#define CALIBRATION_OFFSET 0x64

typedef std::function<void()> Event;
typedef std::function<void(uint16_t (&f)[3], uint16_t (&s)[3])> ColorEvent;
typedef std::function<void(const WiFiEventStationModeConnected &)> WiFiConnectedEvent;
typedef std::function<void(const WiFiEventStationModeDisconnected &)> WiFiDisconnectedEvent;

class WebClient
{
public:
  /**
   * @brief Construct a new Web Client object
   * 
   */
  WebClient(String bridge_id);

  ~WebClient(){};

  /**
   * @brief WebClient setup method
   * 
   */
  void setup();

  void loop()
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      webSocket.loop();
    }
    else
    {
      webSocket.disconnect();
    }
  }

  /**
   * @brief Check is Node to Bridge WiFi connected
   * 
   * Check WiFi status
   * 
   * @return true 
   * @return false 
   */
  bool isConnected();
  /**
   * @brief Check is WebSocket connected
   * 
   * Ping server
   * 
   * @return true 
   * @return false 
   */
  bool isWS();

  /**
 * @brief Print WiFi status in Serial Monitor
 * 
 */
  void status()
  {
    Serial.println(WiFi.status());
  }

  /**
   * @brief Main ws event
   * 
   * Get data, parse and call functions
   * 
   * @param type 
   * @param payload 
   * @param length 
   */
  void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);

  /**
   * @brief Set handler for onBind event 
   * 
   * @param eventFunc 
   */
  void onBind(Event eventFunc);
  /**
   * @brief Set handler for onCalibirate event 
   * 
   * @param eventFunc 
   */
  void onCalibirate(Event eventFunc);
  /**
   * @brief Set handler for onStart event 
   * 
   * @param eventFunc 
   */
  void onStart(Event eventFunc);
  /**
   * @brief Set handler for onStop event 
   * 
   * @param eventFunc 
   */
  void onStop(Event eventFunc);
  /**
   * @brief Set handler for onColor event 
   * 
   * @param eventFunc 
   */
  void onColor(ColorEvent eventFunc);

  /**
   * @brief Set handler for onGetColor event 
   * 
   * @param eventFunc 
   */
  void onGetColor(Event eventFunc);

  /**
   * @brief Set handler for onVibro event 
   * 
   * @param eventFunc 
   */
  void onVibro(Event eventFunc);

  /**
   * @brief Set handler for onAlarm event  
   * 
   * @param eventFunc 
   */
  void onAlarm(Event eventFunc);

  /**
   * @brief Set handler for onConnect event 
   * 
   * @param eventFunc 
   */
  void onConnect(Event eventFunc);

  /**
   * @brief Set handler for onDisconnect event 
   * 
   * @param eventFunc 
   */
  void onDisconnect(Event eventFunc);

  /**
   * @brief Send binary data to WS server (Bridge)
   * 
   * @param buf 
   * @param length 
   */
  void sendBin(uint8_t *buf, size_t length, uint8_t command = 0x00);
  /**
   * @brief Send text data to WS server (Bridge)
   * 
   * @param str 
   */
  void sendTXT(String str);

  /**
   * @brief Trying connect to Bridge AP (access point) and wc connection
   * 
   */
  void connect();
  /**
   * @brief Trying connect to Bridge AP (access point) in binding mode
   * 
   * @return true 
   * @return false 
   */
  bool bind_connection();

  bool bind = false;
  int32_t bindStartTime;

private:
  Ticker web_ticker;

  WebSocketsClient webSocket;

  /**
   * @brief Send mac address to ws server
   * 
   */
  void sendMac();

  /**
   * @brief Send bridge ID to ws server
   * 
   */
  void sendBridgeID();

  Event _bndevent;
  Event _startevent;
  Event _stopevent;
  Event _calibevent;
  Event _alarmevent;
  Event _vibroevent; //TODO: power and duration from bridge!
  Event _getcolor;
  ColorEvent _changecolor;
  Event _connect;
  WiFiConnectedEvent _wificonnect = [](const WiFiEventStationModeConnected &) {};
  Event _disconnect;
  WiFiDisconnectedEvent _wifidisconnect = [](const WiFiEventStationModeDisconnected &) {};
  ;

  WiFiEventHandler connectHandler;
  WiFiEventHandler disconnectHandler;

  bool wifi_c = false;
  bool ws_c = false;

  int ssid_count;

  char ssid[25];

  uint32_t b_id = 0;
  char *ip = (char *)"192.168.4.1";
  uint16_t port = 80;
  char *url = (char *)"\\ws";
};

#endif