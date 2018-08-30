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

#include <Ticker.h>

LED::~LED() {}

LED::LED(uint8_t Rpin, uint8_t Gpin, uint8_t Bpin)
{
    r = Rpin;
    pinMode(Rpin, OUTPUT);

    g = Gpin;
    pinMode(Gpin, OUTPUT);

    b = Bpin;
    pinMode(Bpin, OUTPUT);
    Off();
}

void LED::set_availability(bool enable)
{
    if (enable != enabled)
    {
        enabled = enable;
        if (!enable)
        {
            digitalWrite(r, HIGH);
            digitalWrite(g, HIGH);
            digitalWrite(b, HIGH);
        }
    }
}

void LED::setup_color(RGB first, RGB second)
{
    color = first;
    sec_color = second;
}

void LED::setup_color(uint16_t (&c)[6])
{
    setup_color({c[0], c[1], c[2]}, {c[3], c[4], c[5]});
}

void LED::smooth_blink(uint16_t frequency, bool delay)
{
    delay_active = delay;
    current_step = 0;
    led_ticker.attach_ms<LED *>(frequency, [](LED *led) {
        led->PrintCurrent();
        if (led->delay_cycles > 0)
            led->delay_cycles--;
        else
        {
            led->current = calculate_values(led->current, led->steps, led->color, led->sec_color, led->rise);

            if (++led->current_step == STEPS)
            {
                led->rise = !led->rise;
                led->current_step = 0;
                if (led->delay_active)
                    led->delay_cycles = 20;
            }

            led->set_color(led->current);
        }
    },
                                this);
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

RGB LED::calculate_values(RGB current, RGB steps, RGB color, RGB sec_color, bool &rise)
{
    RGB updV = NONE;

    updV.R = calc_value(current.R, steps.R, color.R, sec_color.R, rise);
    updV.G = calc_value(current.G, steps.G, color.G, sec_color.G, rise);
    updV.B = calc_value(current.B, steps.B, color.B, sec_color.B, rise);

    // if (owerflow_check(updV, steps, color, sec_color))
    // {
    //     rise = !rise;
    // }

    return updV;
}

int16_t LED::calc_value(int16_t val, int16_t step, int16_t min, int16_t max, bool &rise)
{
    if (rise)
        val += step;
    else
        val -= step;

    return val;
}

void LED::set_color(RGB col, bool alarm)
{
    current = col;
    if (!enabled && !alarm)
        return;

    if (col.R >= 0)
        analogWrite(r, 1023 - col.R);
    if (col.G >= 0)
        analogWrite(g, 1023 - col.G);
    if (col.B >= 0)
        analogWrite(b, 1023 - col.B);
}

void LED::switch_mode(Mode mode)
{
    led_ticker.detach();
    ticker_killer.detach();
    rise = true;
    _mode = mode;

    if (_prevmode != NaI && _mode != S_BLINK)
    {
        _prev_f = color;
        _prev_s = sec_color;
    }
}

void LED::stateOn(RGB color)
{
    setup_color(color);
    switch_mode(ON);
    set_color(color);
}

void LED::Off()
{
    _prevmode = NaI;
    setup_color(NONE);
    switch_mode(OFF);
    set_color(NONE);
}

void LED::stateBlink()
{
    switch_mode(BLINK);
    steps = calculate_step(color, sec_color);
}

void LED::stateSBlink(RGB col, RGB s_col)
{
    switch_mode(S_BLINK);
    setup_color(col);
    steps = calculate_step(color, sec_color);
    set_color(sec_color);
}

void LED::ConstantLighting(RGB color)
{
    _prevmode = CONSTANT;
    stateOn(color);
}

void LED::CrossFade(RGB first, RGB second, bool withTransition, bool delay)
{
    Serial.println("CrossFade");
    _prevmode = CROSSFADE;
    setup_color(first, second);
    stateBlink();
    if (withTransition)
    {
        Serial.println("with transition");
        transition_blink(30);
        return;
    }
    set_color(sec_color);
    smooth_blink(30, delay);
}

void LED::CrossFade(uint16_t colors[6], bool withTransition, bool delay)
{
    CrossFade({colors[0], colors[1], colors[2]}, {colors[3], colors[4], colors[5]}, withTransition, delay);
}

void LED::transition_blink(uint16_t freaquency)
{
    setup_color(current, sec_color);
    steps = calculate_step(sec_color, current);

    smooth_blink(freaquency, false);

    blinkAfter(freaquency * STEPS);
}

void LED::Alarm()
{
    stateSBlink({1023, 0, 0}, NONE);

    led_ticker.attach_ms<LED *>(100, [](LED *led) {
        if (led->rise)
            led->set_color(led->color, true);
        else
            led->set_color(led->sec_color, true);

        led->rise = !led->rise;
    },
                                this);

    killAfter(3000);
}

void LED::Calibration()
{
    _prevmode = CALIBRATION;
    setup_color({1000, 500, 1000}, {0, 1000, 500}); //as variant for calibration
    stateBlink();
    set_color(sec_color);
    smooth_blink(23);
}

void LED::BlueBlink()
{
    Serial.println("BlueBlink");
    _prevmode = BLUEBLINK;
    setup_color({0, 0, 1024});
    stateBlink();
    set_color(sec_color);
    smooth_blink(30);
}

void LED::SingleBlink(RGB col)
{
    uint16_t fr = 30;
    stateSBlink(col);
    smooth_blink(fr);
    killAfter(2 * fr * STEPS);
}

void LED::killAfter(uint32_t milliseconds)
{
    ticker_killer.once_ms<LED *>(milliseconds, [](LED *led) {
        led->led_ticker.detach();
        switch (led->_prevmode)
        {
        case NaI:
            led->Off();
            return;
            break;
        case CONSTANT:
            led->ConstantLighting(led->_prev_f);
            break;
        case CROSSFADE:
            led->CrossFade(led->_prev_f, led->_prev_s);
            break;
        case BLUEBLINK:
            led->BlueBlink();
            break;
        case CALIBRATION:
            led->Calibration();
            break;
        }
    },
                                 this);
}

void LED::blinkAfter(uint32_t milliseconds)
{
    ticker_killer.once_ms<LED *>(milliseconds, [](LED *led) {
        led->led_ticker.detach();
        led->CrossFade(led->_prev_f, led->_prev_s, false);
    },
                                 this);
}