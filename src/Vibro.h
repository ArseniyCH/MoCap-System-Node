/**
 * @brief Vibration manage class
 * 
 * @file Vibro.h
 * @author Arseniy Churin
 * @date 2018-05-28
 */
#ifndef VIBRO_H
#define VIBRO_H

#include <inttypes.h>
#include <Ticker.h>

class Vibro
{
public:
  /**
   * @brief Construct a new Vibro object with specific pin
   * 
   * @param pin 
   */
  Vibro(uint8_t pin, uint8_t gnd_pin);

  /**
   * @brief Method to do alarm AlarmVibration
   * 
   * Pulse vibration for 3 sec.
   * 
   */
  void AlarmVibration();
  /**
   * @brief Done Vibration
   * 
   */
  void DoneVibration();
  /**
   * @brief Single Vibration
   * 
   */
  void SingleVibration(uint16_t power = 1023, uint16_t milliseconds = 500);

private:
  /**
   * @brief On vibro motor
   * 
   * @param power 
   */
  void on(uint16_t power = 1023);
  /**
   * @brief Off vibro motor
   * 
   */
  void off();
  /**
   * @brief kill vibro_ticker after milliseconds
   * 
   * @param milliseconds 
   */
  void killAfter(uint32_t milliseconds);

  Ticker vibro_ticker;
  Ticker ticker_killer;
  uint8_t _pin;
  bool tumbler = true;
};

#endif