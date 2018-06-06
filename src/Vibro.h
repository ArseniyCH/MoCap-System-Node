/**
 * @brief 
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
  Vibro(uint8_t pin);

  void vibrate();

private:
  static void vibro(uint8_t pin, uint16_t delta, Ticker ticker[4]);

  Ticker a, b, c, d;
  Ticker off_tickers[4] = {a, b, c, d};
  uint8_t _pin;
};

#endif