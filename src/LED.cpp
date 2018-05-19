/*static const uint8_t D0   = 16; 
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2; 
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;*/

/**
 * @brief   
 * 
 * @file LED.cpp
 * @author Arseniy Churin
 * @date 2018-05-18
 */

#include "Arduino.h"
#include "LED.h"
#include "math.h"

LED::~LED() {}

LED::LED(uint8_t Rpin, uint8_t Gpin, uint8_t Bpin)
{
    r = Rpin;
    pinMode(Rpin, OUTPUT);

    g = Gpin;
    pinMode(Gpin, OUTPUT);

    b = Bpin;
    pinMode(Bpin, OUTPUT);
    stateOff();
}

void LED::setup()
{
}

void LED::setup_color(RGB first, RGB second)
{
    color = first;
    sec_color = second;
}

void LED::loop()
{
    current = calculate_values();
    set_color(current);
}

RGB LED::calculate_step(RGB prev, RGB end)
{
    RGB step = {prev.R - end.R, prev.G - end.G, prev.B - end.B};

    if (step.R)
        step.R = step.R / STEPS;
    if (step.G)
        step.G = step.G / STEPS;
    if (step.B)
        step.B = step.B / STEPS;

    return step;
}

RGB LED::calculate_values()
{
    RGB updV = NONE;

    updV.R = calc_value(current.R, steps.R, color.R, sec_color.R);
    updV.G = calc_value(current.G, steps.G, color.G, sec_color.G);
    updV.B = calc_value(current.B, steps.B, color.B, sec_color.B);

    if (owerflow_check(updV))
        rise = !rise;

    return updV;
}

int16_t LED::calc_value(int16_t val, int16_t step, int16_t min, int16_t max)
{
    if (rise)
        val += step;
    else
        val -= step;

    return val;
}

bool LED::owerflow_check(RGB val)
{
    if (steps.R)
        if (val.R - steps.R < min(color.R, sec_color.R) || val.R + steps.R > max(color.R, sec_color.R))
        {
            return true;
        }

    if (steps.G)
        if (val.G - steps.G < min(color.G, sec_color.G) || val.G + steps.G > max(color.G, sec_color.G))
        {
            return true;
        }

    if (steps.B)
        if (val.B - steps.B < min(color.B, sec_color.B) || val.B + steps.B > max(color.B, sec_color.B))
        {
            return true;
        }

    return false;
}

void LED::set_color(RGB col)
{
    current = col;
    analogWrite(r, 1023 - col.R);
    analogWrite(g, 1023 - col.G);
    analogWrite(b, 1023 - col.B);
}

void LED::switch_mode(Mode mode)
{
    switch (mode)
    {
    case (ON):
        // state ON exit logic
        break;
    case (OFF):
        // state OFF exit logic
        break;
    case (BLINK):
        // state BLINK exit logic
        break;
    }
    _mode = mode;
}

void LED::stateOn()
{
    switch_mode(ON);
    set_color(color);
}

void LED::stateOff()
{
    switch_mode(OFF);
    set_color({0, 0, 0});
}

void LED::stateBlink()
{
    switch_mode(BLINK);
    steps = calculate_step(color, sec_color);
    set_color(sec_color);
}