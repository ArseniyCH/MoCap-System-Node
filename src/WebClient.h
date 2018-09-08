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
typedef std::function<void(uint16_t (&f)[6])> ColorEvent;
typedef std::function<void(uint16_t number)> IntEvent;
typedef std::function<void(bool flag)> BoolEvent;
typedef std::function<void(String str)> StringEvent;
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

  void loop()
  {
    webSocket.loop();
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
   * @brief Set handler for onSSID event
   * 
   * @param eventFunc 
   */
  void onSSID(StringEvent eventFunc);

  /**
   * @brief Set handler for onGetColor event 
   * 
   * @param eventFunc 
   */
  void
  onGetColor(Event eventFunc);

  /**
   * @brief Set handler for onVibro event 
   * 
   * @param eventFunc 
   */
  void onVibro(IntEvent eventFunc);

  /**
   * @brief Set handler for onLedIO event
   * 
   * @param eventFunc 
   */
  void onLedIO(BoolEvent eventFunc);

  /**
   * @brief Set handler for onAlarm event  
   * 
   * @param eventFunc 
   */
  void onAlarm(Event eventFunc);

  /**
   * @brief Set handler for onRestart event
   * 
   * @param eventFunc 
   */
  void onRestart(IntEvent eventFunc);

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
   * @brief Trying connect to Bridge AP (access point)
   * 
   */
  void connect(bool bind_connection = false);
  void connect(const char *ssid, bool bind_connection = false);

  /**
   * @brief Begin websocket connection
   * 
   * @param host 
   * @param port 
   * @param url 
   */
  void ws_connect(IPAddress host, uint16_t port, const char *url = "/");

  /**
   * @brief Trying connect to Bridge AP (access point) in binding mode
   * 
   * @return true 
   * @return false 
   */
  bool bind_connection();

  /**
   * @brief Connect to next Bridge AP in binding mode
   * 
   */
  void bind_next();

  bool bind = false;
  int32_t bindStartTime;

private:
  Ticker web_ticker, ws_ticker;

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
  IntEvent _restartevent;
  IntEvent _vibroevent;
  BoolEvent _ledioevent;
  Event _getcolor;
  ColorEvent _changecolor;
  StringEvent _changessid;
  Event _connect;
  WiFiConnectedEvent _wificonnect = [](const WiFiEventStationModeConnected &) {};
  Event _disconnect;
  WiFiDisconnectedEvent _wifidisconnect = [](const WiFiEventStationModeDisconnected &) {};

  WiFiEventHandler connectHandler;
  WiFiEventHandler disconnectHandler;

  bool wifi_c = false;
  bool ws_c = false;

  int ssid_count = 0;
  int i_ssid = 0;

  char ssid[25];

  String b_id = "";
  char *ip = (char *)"192.168.4.1";
  uint16_t port = 80;
  char *url = (char *)"\\ws";
};

#endif