/**
 * @brief 
 * 
 * @file main.cpp
 * @author Arseniy Churin
 * @date 2018-05-18
 */

#include <Arduino.h>

#include "StateMachine.hpp"

/**
 * @brief Main setup method
 * 
 */
void setup()
{
    Serial.begin(115200);
    Serial.println("1");
    Serial.setDebugOutput(true);

    Serial.println("2");
    state_setup();
}

/**
 * @brief Main loop method
 * 
 */
void loop()
{
    state_loop();
}