/**
 * @brief 
 * 
 * @file LED.h
 * @author Arseniy Churin
 * @date 2018-05-18
 */

#ifndef LED_H_
#define LED_H_

#define DELTA 125
#define STEPS 25

#define NONE \
  {          \
    0, 0, 0  \
  }

/**
   * @brief Struct to store colors
   * 
   */
struct RGB
{
  int16_t R;
  int16_t G;
  int16_t B;
};

/**
   * @brief LED class
   * 
   */
class LED
{
public:
  /**
   * @brief 
   * 
   */
  typedef enum
  {
    OFF,
    BLINK,
    ON
  } Mode;

  /**
   * @brief Current state
   */
  Mode _mode = OFF;

  /**
   * @brief Construct a new LED object
   */
  LED(){};

  /**
   * @brief Construct a new LED::LED object
   * 
   * @param Rpin 
   * @param Gpin 
   * @param Bpin 
   */
  LED(uint8_t Rpin, uint8_t Gpin, uint8_t Bpin);
  /**
   * @brief Destroy the LED::LED object
   * 
   */
  ~LED();

  /**
   * @brief Led setup method
   * 
   */
  void setup();
  /**
   * @brief Main loop of led
   * 
   */
  void loop();

  /**
   * @brief Setup color of lighting 
   * 
   * Set 2 colors of lighting to switch between slowly 
   * 
   * @param first Main color
   * @param second Optional color
   */
  void setup_color(RGB first, RGB second = NONE);

  /**
   * @brief Switch led state machine to ON state
   * 
   */
  void stateOn();
  /**
   * @brief Switch led state machine to OFF state
   * 
   */
  void stateOff();
  /**
   * @brief Switch led state machine to BLINK state
   * 
   */
  void stateBlink();

private:
  RGB color = NONE;
  RGB sec_color = NONE;

  uint8_t r, g, b;

  RGB current = NONE;
  RGB steps = NONE;
  bool rise = true;

  /**
   * @brief Calculate steps to all of colors
   * 
   * Calculate steps for R, G and B led that need
   * to step change value over all range of values
   * from one color to another
   * 
   * @param prev 
   * @param end 
   * @return RGB 
   */
  RGB calculate_step(RGB prev, RGB end);
  /**
   * @brief Calculate next value to led
   * 
   * Calculate next value to led by step and current value
   * 
   * @param val 
   * @param step 
   * @param min 
   * @param max 
   * @return int16_t 
   */
  int16_t calc_value(int16_t val, int16_t step, int16_t min, int16_t max);
  /**
   * @brief Calculate values to all of colors
   * 
   * Calculate values for R, G and B led 
   * from one color to another based on steps and current colour
   * 
   * @return RGB 
   */
  RGB calculate_values();

  /**
   * @brief Check color to owerflow
   * 
   * Check if some of leds out of range
   * Return true if value out of range
   * 
   * @param val 
   * @return true 
   * @return false 
   */
  bool owerflow_check(RGB val);
  /**
   * @brief Set color of RGB led
   * 
   * @param col 
   */
  void set_color(RGB col);
  void update_color(int16_t current);
  /**
   * @brief Switch mode of led state machine
   * 
   * Modes: ON, OF, BLINK 
   * 
   * @param mode
   */
  void switch_mode(Mode mode);
};

#endif