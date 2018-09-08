/**
 * @brief State Machine class realization
 * 
 * @file StateMachine.hpp
 * @author Arseniy Churin
 * @date 2018-05-19
 */

#ifndef STATEMACHINE_H
#define STATEMACHINE_H

//#define DEV_MODE

#include <WString.h>

#include "MPU.h"
#include "LED.h"
#include "WebClient.h"
#include "Vibro.h"
#include "EEPROM.hpp"

#include <Arduino.h>

#include <Ticker.h>

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
uint16_t mem_colors[6];

WebClient wc = WebClient(ReadString(10));

MPU &mpu = MPU::Instance();
Vibro vibr = Vibro(2);
State _state = Undef;

Ticker restart_ticker;

uint8_t *quat = new uint8_t[4 * sizeof(float)];

/**
 * @brief Set the State of StateMachine
 * 
 * @param state 
*/
void setState(State state)
{
  if (state == _state)
    return;

  switch (_state)
  {
  case Undef:
    Serial.println("Exit from Undef state");
    // state Undef exit logic
    break;
  case Bind:
    Serial.println("Exit from Bind state");
    wc.onBind(nullptr);
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
    //MPU_ticker.detach();
    mpu.disable();
    break;
  case Search:
    Serial.println("Exit from Search state");
    vibr.SingleVibration();
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
  if (_state != Undef)
    led.BlueBlink();
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

  led.CrossFade(mem_colors);

  setState(Bind);
  Serial.println("Switch to Bind state");
}

/**
 * @brief Switch state machine to Calibration state
 * 
 */
void stateCalibration()
{
  if (_state != Standby)
    return;

  led.Calibration();

  setState(Calibration);
  Serial.println("Switch to Calibration state");
}

/**
 * @brief Switch state machine to Standby state
 * 
 */
void stateStandby()
{
  setState(Standby);
  Serial.println("Switch to Standby state");
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
  Serial.println("Switch to Active state");
  mpu.enable();
  // #ifdef DEV_MODE
  //   MPU_ticker.attach_ms(50, []() {
  //     uint8_t *quat = (uint8_t *)"0000000000000000";
  //     wc.sendBin(quat, 4 * sizeof(float), MPU_DATA);
  //   });
  // #else
  //   MPU_ticker.attach_ms(50, []() {
  //     mpu.mpu_loop(quat);
  //     if (quat)
  //       wc.sendBin(quat, 4 * sizeof(float), MPU_DATA);
  //   });
  // #endif
}

/**
 * @brief On ws connect
 * 
 */
void connect()
{
  Serial.println("Connected!");
  led.CrossFade(mem_colors);
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
void changeColor(uint16_t (&c)[6])
{
  if (_state != Standby && _state != Bind)
    return;

  if (CompareArrays(c, mem_colors, color_size * 2))
    return;

  WriteRGB(c, COLOR_ADDRESS);
  for (int i = 0; i < color_size * 2; ++i)
    mem_colors[i] = c[i];
  led.CrossFade(c);
}

void sendColor()
{
  if (_state != Standby && _state != Bind)
    return;

  uint8_t colors[6];
  ReadMappedRGB(colors, COLOR_ADDRESS);

  wc.sendBin(colors, 6, COLORS);
}

void setLedIO(bool enable)
{
  led.set_availability(enable);
}

void vibroResponse(uint16_t seconds)
{
  if (seconds == 0)
    seconds = 1;
  if (seconds > 10)
    seconds = 10;
  vibr.SingleVibration(1000 * seconds);
}

void Alarm()
{
  led.Alarm();
  vibr.AlarmVibration();
}

void changeSSID(String ssid)
{
  if (_state != Bind)
    return;
  SaveString(10, (uint8_t *)ssid.c_str(), ssid.length());
}

void Restart(uint16_t seconds)
{
  if (_state != Standby)
    return;

  if (seconds > 10)
    seconds = 10;

  restart_ticker.attach(seconds, []() { ESP.restart(); });
}

/**
 * @brief State machine setup method
 * 
 */
void state_setup()
{
  led.BlueBlink();
  wc.onConnect(connect);
  wc.onDisconnect(disconnect);
  wc.onStart(stateActive);
  wc.onStop(stateStandby);
  wc.onBind(stateBind);
  wc.onSSID(changeSSID);
  wc.onCalibirate(stateCalibration);
  wc.onColor(changeColor);
  wc.onGetColor(sendColor);
  wc.onLedIO(setLedIO);
  // wc.onLedIO(std::bind(&LED::set_availability, led,
  //                      std::placeholders::_1));
  wc.onVibro(vibroResponse);
  wc.onAlarm(Alarm);
  wc.onRestart(Restart);

  delay(1);
  ReadRGB(mem_colors, COLOR_ADDRESS);

  //#ifndef DEV_MODE
  mpu.mpu_setup();
  mpu.disable();
  //#endif

  if (wc.bind_connection())
    stateBind();
  else
  {
    stateSearch();
    wc.connect();
  }
}

/**
 * @brief State machine loop method
 * 
 */
void state_loop()
{
  wc.loop();

  if (_state == Active)
    if (mpu.mpu_loop(quat))
      wc.sendBin(quat, 4 * sizeof(float), MPU_DATA);
};

#endif