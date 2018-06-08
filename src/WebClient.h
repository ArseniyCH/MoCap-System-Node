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

typedef std::function<void()> Event;
typedef std::function<void(int16_t r, int16_t g, int16_t b)> ColorEvent;
typedef std::function<void(const WiFiEventStationModeConnected &)> WiFiConnectedEvent;

class WebClient
{
public:
  /**
   * @brief Construct a new Web Client object
   * 
   */
  WebClient();

  /**
   * @brief WebClient setup method
   * 
   */
  void setup();
  /**
   * @brief WebClient loop method
   * 
   */
  void loop();

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
  void onColor(std::function<void(int16_t r, int16_t g, int16_t b)> eventFunc);

  /**
   * @brief Set handler for onConnect event 
   * 
   * @param eventFunc 
   */
  void onConnect(Event eventFunc);
  /**
   * @brief Set handler for onWiFiConnect event
   * 
   * @param eventFunc
   */
  void onWiFiConnect(Event eventFunc);
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
  void sendBin(uint8_t *buf, size_t length);
  /**
   * @brief Send text data to WS server (Bridge)
   * 
   * @param str 
   */
  void sendTXT(String str);

  /**
   * @brief Trying connect to WS server (Bridge)
   * 
   */
  void websockets_connection();

  /**
   * @brief Trying connect to Bridge AP (access point)
   * 
   */
  void wifi_connection();
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
  void sendId();
  /**
   * @brief Find bridge after scan
   * 
   * @param n count of found networks 
   */
  std::function<void(int)> onScan;

  void f(const WiFiEventStationModeConnected &);

  Event _bndevent;
  Event _startevent;
  Event _stopevent;
  Event _calibevent;
  ColorEvent _changecolor;

  Event _connect;
  WiFiConnectedEvent _wificonnect;
  Event _disconnect;
  WiFiEventHandler connectHandler;

  int ssid_count;

  char *ssid = (char *)"esp-async";
  char *ip = (char *)"192.168.4.1";
  uint16_t port = 80;
  char *url = (char *)"\\ws";
};

#endif