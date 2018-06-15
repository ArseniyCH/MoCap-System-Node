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
#include "EEPROM.hpp"

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
WebClient wc = WebClient(ReadString(10));
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
  switch (_state)
  {
  case Undef:
    Serial.println("Exit from Undef state");
    // state Undef exit logic
    break;
  case Bind:
    Serial.println("Exit from Bind state");
    wc.onBind(NULL);
    break;
  case Calibration:
    Serial.println("Exit from Calibration state");
    // state Calibration exit logic
    break;
  case Standby:
    Serial.println("Exit from Standby state");
    // state Standby exit logic
    break;
  case Active:
    Serial.println("Exit from Active state");
    mpu.disable();
    break;
  case Search:
    Serial.println("Exit from Search state");
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
  Serial.println("Switch to Undef state");
  // State Undef enter logic
}

/**
 * @brief Switch state machine to Search state
 * 
 */
void stateSearch()
{
  setState(Search);
  Serial.println("Switch to Search state");
}

/**
 * @brief Switch state machine to Bind state
 * 
 */
void stateBind()
{
  if (_state != Undef && _state != Search)
    return;
  setState(Bind);
  Serial.println("Switch to Search state");
  // if (wc.bind_connection())
  //   wc.websockets_connection();
  // else
  //   stateSearch();
}

/**
 * @brief Switch state machine to Calibration state
 * 
 */
void stateCalibration()
{
  if (_state != Standby)
    return;
  setState(Calibration);
  Serial.println("Switch to Search state");
  // State Calibration enter logic
}

/**
 * @brief Switch state machine to Standby state
 * 
 */
void stateStandby()
{
  setState(Standby);
  Serial.println("Switch to Search state");
  // State Standby enter logic
}

/**
 * @brief Switch state machine to Active state
 * 
 */
void stateActive()
{
  if (_state != Standby)
    return;
  setState(Active);
  Serial.println("Switch to Search state");
  mpu.enable();
  // State Active enter logic
}

/**
 * @brief On ws connect
 * 
 */
void connect()
{
  Serial.println("Connected!");
  stateStandby();
}

/**
 * @brief On ws disconnect
 * 
 */
void disconnect()
{
  Serial.println("Disconnected((");
  if (_state != Search)
    stateSearch();
}

/**
 * @brief Led on with color
 * 
 * @param f First color
 * @param s Second color 
 */
void color(uint16_t (&f)[3], uint16_t (&s)[3])
{
  led.setup_color({f[0], f[1], f[2]}, {s[0], s[1], s[2]});
}

void sendColor()
{
  //TODO: Get color from EEPROM and send it to bridge
}

void vibroResponse()
{
  if (_state != Standby && _state != Active)
    return;
  vibr.SingleVibration();
}

void Alarm()
{
  led.Alarm();
  vibr.AlarmVibration();
}

/**
 * @brief State machine setup method
 * 
 */
void state_setup()
{
  wc.onConnect(connect);
  wc.onDisconnect(disconnect);
  wc.onStart(stateActive);
  wc.onStop(stateStandby);
  wc.onBind(stateBind);
  wc.onCalibirate(stateCalibration);
  wc.onColor(color);
  wc.onGetColor(sendColor);
  wc.onVibro(vibroResponse);
  wc.onAlarm(Alarm);

  mpu.mpu_setup();
  mpu.disable();

  wc.connect();
  stateSearch();
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

  switch (_state)
  {
  case Undef:
    // state Undef loop logic
  case Bind:
    // if (millis() - wc.bindStartTime > 30000)
    //   stateSearch();
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