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
    digitalWrite(pin, LOW);
}

void Vibro::AlarmVibration()
{
    vibro_ticker.attach_ms<Vibro *>(100, [](Vibro *v) {
        if (v->tumbler)
            v->on();
        else
            v->off();

        v->tumbler = !v->tumbler;
    },
                                    this);

    killAfter(3000);
}

void Vibro::DoneVibration()
{
    vibro_ticker.attach_ms<Vibro *>(20, [](Vibro *v) {
        if (v->tumbler)
            v->on();
        else
            v->off();

        v->tumbler = !v->tumbler;
    },
                                    this);

    killAfter(1000);
}

void Vibro::SingleVibration(uint16_t power, uint16_t milliseconds)
{
    on(power);
    vibro_ticker.attach_ms<Vibro *>(milliseconds, [](Vibro *v) {
        v->off();
    },
                                    this);
}

void Vibro::on(uint16_t power)
{
    analogWrite(_pin, power);
}

void Vibro::off()
{
    digitalWrite(_pin, LOW);
}

void Vibro::killAfter(uint32_t milliseconds)
{
    ticker_killer.once_ms<Vibro *>(milliseconds, [](Vibro *v) {
        v->vibro_ticker.detach();
        v->off();
        v->tumbler = true;
    },
                                   this);
}