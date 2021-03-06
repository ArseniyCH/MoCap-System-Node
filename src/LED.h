/**
 * @brief 
 * 
 * @file LED.h
 * @author Arseniy Churin
 * @date 2018-05-18
 */
#include <Ticker.h>

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
    S_BLINK,
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
   * @brief Method for enable/disable led indicator
   * 
   * @param enable 
   */
  void set_availability(bool enable);

  /**
   * @brief Transition from old colors to new colors
   * 
   */
  void transition_blink(uint16_t frequency = 25);

  /**
   * @brief Main loop of led
   * 
   */
  void smooth_blink(uint16_t frequency, bool delay = false);

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
   * @brief Setup color of lighting 
   * 
   * Other method to setup colors
   * 
   * @param c array of colors
   */
  void setup_color(uint16_t (&c)[6]);

  /**
   * @brief Turns on constant indication
   * 
   * @param color 
   */

  /**
   * @brief 
   * 
   * @param color 
   */
  void ConstantLighting(RGB color);

  /**
   * @brief Cross fading between two colors
   */
  void CrossFade(RGB first, RGB second = {0, 0, 0}, bool withTransition = true, bool delay = true);

  /**
   * @brief Cross fading between two colors
   */
  void CrossFade(uint16_t colors[6], bool withTransition = true, bool delay = true);

  /**
   * @brief Alarm indication
   * 
   * Red fast blinkig for 3 second
   */
  void Alarm();
  /**
   * @brief Switch led state machine to OFF state and turn off indication
   * 
   */
  void Off();
  /**
   * @brief Rainbow indication while MPU calibrating
   * 
   */
  void Calibration();
  /**
   * @brief Blue smooth blinking for search and bind states
   * 
   */
  void BlueBlink();
  /**
   * @brief Single blink
   * 
   * @param col 
   */
  void SingleBlink(RGB col);

  void PrintCurrent()
  {
    Serial.print("current values:");
    Serial.print("R:");
    Serial.print(current.R);
    Serial.print("\tG:");
    Serial.print(current.G);
    Serial.print("\tB:");
    Serial.println(current.B);
  }

private:
  /**
   * @brief Switch led state machine to ON state
   * 
   */
  void stateOn(RGB color);

  /**
   * @brief Switch led state machine to BLINK state
   * 
   */
  void stateBlink();

  /**
   * @brief Switch led state machine to SINGLE BLINK state
   * 
   * @param col 
   */
  void stateSBlink(RGB col, RGB s_col = NONE);

  /**
   * @brief Detach led_ticker after milliseconds
   * 
   * @param milliseconds 
   */
  void killAfter(uint32_t milliseconds);

  /**
   * @brief Detach led_ticker after milliseconds and run blink
   * 
   * 
   * @param milliseconds 
   */
  void blinkAfter(uint32_t milliseconds);

  Ticker led_ticker;
  Ticker ticker_killer;
  Ticker transition_ticker;

  RGB color = NONE;
  RGB sec_color = NONE;

  /**
   * @brief Blink modes enum
   * 
   */
  typedef enum
  {
    NaI,
    CONSTANT,
    CROSSFADE,
    BLUEBLINK,
    CALIBRATION
  } InfiniteMode;

  /**
   * @brief previous state
   * 
   */
  InfiniteMode _prevmode = NaI;
  RGB _prev_f = NONE;
  RGB _prev_s = NONE;
  uint32_t _prev_fr = 0;

  uint8_t r, g, b;

  RGB current = NONE;
  RGB steps = NONE;
  bool rise = true;
  uint8_t current_step = 0;
  bool enabled = true;

  uint16_t delay_cycles = 0;
  bool delay_active = false;

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
  static int16_t calc_value(int16_t val, int16_t step, int16_t min, int16_t max, bool &rise);
  /**
   * @brief Calculate values to all of colors
   * 
   * Calculate values for R, G and B led 
   * from one color to another based on steps and current colour
   * 
   * @param current 
   * @param steps 
   * @param color 
   * @param sec_color 
   * @param rise 
   * @return RGB 
   */
  static RGB calculate_values(RGB current, RGB steps, RGB color, RGB sec_color, bool &rise);

  /**
   * @brief Set color of RGB led
   * 
   * @param col 
   */
  void set_color(RGB col, bool alarm = false);

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