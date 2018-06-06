/**
 * @brief 
 * 
 * @file Vibro.cpp
 * @author Arseniy Churin
 * @date 2018-05-28
 */
#include "Vibro.h"

#include <Arduino.h>
#include <Ticker.h>

Vibro::Vibro(uint8_t pin)
{
    _pin = pin;
    pinMode(pin, OUTPUT);
}

void Vibro::vibrate()
{
    vibro(_pin, 100, off_tickers);
}

void Vibro::vibro(uint8_t pin, uint16_t delta, Ticker ticker[4])
{
    uint16_t millis_counter = 0;
    for (uint16_t i = 0; i < 4; i += 2)
    {
        ticker[i].once_ms<uint8_t>(millis_counter, [](uint8_t _pin) { digitalWrite(_pin, HIGH); }, pin);
        millis_counter += delta;
        ticker[i + 1].once_ms<uint8_t>(millis_counter, [](uint8_t _pin) { digitalWrite(_pin, LOW); }, pin);
        millis_counter += delta;
    }
}