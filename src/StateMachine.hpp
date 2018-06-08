/**
 * @brief State Machine class realization
 * 
 * @file StateMachine.hpp
 * @author Arseniy Churin
 * @date 2018-05-19
 */

#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <WString.h>

#include "MPU.h"
#include "LED.h"
#include "WebClient.h"
#include "Vibro.h"

#include <Arduino.h>

typedef enum
{
  Undef,
  Bind,
  Calibration,
  Standby,
  Active,
  Search
} State;

LED led = LED(13, 12, 14);
WebClient wc = WebClient();
MPU &mpu = MPU::Instance();
Vibro vibr = Vibro(0);
State _state = Undef;

/**
 * @brief Set the State of StateMachine
 * 
 * @param state 
*/
void setState(State state)
{
  switch (state)
  {
  case Undef:
    // state Undef exit logic
  case Bind:
    wc.onBind(NULL);
    break;
  case Calibration:
    // state Calibration exit logic
    break;
  case Standby:
    // state Standby exit logic
    break;
  case Active:
    mpu.disable();
    break;
  case Search:
    // state Searching exit logic
    break;
  }
  _state = state;
}

/**
     * @brief Switch state machine to Undef state
     * 
     */
void stateUndef()
{
  setState(Undef);
  // State Undef enter logic
}

/**
     * @brief Switch state machine to Search state
     * 
     */
void stateSearch()
{
  setState(Search);
  wc.wifi_connection();
  //wc.websockets_connection();
}

/**
     * @brief Switch state machine to Bind state
     * 
     */
void stateBind()
{
  setState(Bind);
  if (wc.bind_connection())
    wc.websockets_connection();
  else
    stateSearch();
}

/**
     * @brief Switch state machine to Calibration state
     * 
     */
void stateCalibration()
{
  setState(Calibration);
  // State Calibration enter logic
}

/**
     * @brief Switch state machine to Standby state
     * 
     */
void stateStandby()
{
  setState(Standby);
  // State Standby enter logic
}

/**
     * @brief Switch state machine to Active state
     * 
     */
void stateActive()
{
  setState(Active);
  mpu.enable();
  // State Active enter logic
}

/**
 * @brief On wifi connect
 * 
 */
void wifi_connect()
{
  wc.websockets_connection();
}

/**
     * @brief On ws connect
     * 
     */
void connect()
{
  stateStandby();
}

/**
     * @brief On ws disconnect
     * 
     */
void disconnect()
{
}

/**
     * @brief Led on with color
     * 
     * @param r 
     * @param g 
     * @param b 
     */
void color(int16_t r, int16_t g, int16_t b)
{
  led.setup_color({r, g, b});
}

/**
     * @brief State machine setup method
     * 
     */
void state_setup()
{
  wc.onWiFiConnect(wifi_connect);
  wc.onConnect(connect);
  wc.onDisconnect(disconnect);
  wc.onStart(stateActive);
  wc.onStop(stateStandby);
  wc.onBind(stateBind);
  wc.onCalibirate(stateCalibration);
  wc.onColor(color);
  stateSearch();

  mpu.mpu_setup();
  mpu.disable();
}

String mac = String(WiFi.macAddress());

/**
     * @brief State machine loop method
     * 
     */
void state_loop()
{
  ///
  ///
  ///
  String inString = "";
  while (Serial.available() > 0)
  {
    int inChar = Serial.read();
    if (inChar != '\n')
      inString += (char)inChar;
  }

  if (!inString.equals(""))
  {
    if (inString.equals("0"))
    {
      led.Alarm();
    }
    if (inString.equals("1"))
    {
      Serial.println(inString);
      led.ConstantLighting({0, 1000, 0});
    }
    if (inString.equals("2"))
    {
      Serial.println(inString);
      led.CrossFade({500, 0, 0}, {0, 0, 1000});
    }
    if (inString.equals("3"))
    {
      Serial.println(inString);
      led.Off();
    }
    if (inString.equals("4"))
    {
      Serial.println(inString);
      led.Calibration();
    }
    if (inString.equals("5"))
    {
      Serial.println(inString);
      led.BlueBlink();
    }
    if (inString.equals("6"))
    {
      Serial.println(inString);
      led.SingleBlink({55, 100, 400});
    }
    if (inString.equals("7"))
    {
      Serial.println(inString);
      vibr.AlarmVibration();
    }
  }
  ///
  ///
  ///

  String q = mpu.mpu_loop();
  //wc.loop();

  switch (_state)
  {
  case Undef:
    // state Undef loop logic
  case Bind:
    if (millis() - wc.bindStartTime > 30000)
      stateSearch();
    break;
  case Calibration:
    // state Calibration loop logic
    break;
  case Standby:
    // state Standby loop logic
    break;
  case Active:
  {
    if (!q.equals(""))
    {
      //wc.sendTXT(mac + ":" + q);
    }

    break;
  }
  case Search:

    break;
  }
};

#endif