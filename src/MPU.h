/**
 * @brief Singleton class to work with MPU6050
 * 
 * @file MPU.h
 * @author Arseniy Churin
 * @date 2018-05-18
 */

#ifndef MPU_H
#define MPU_H

#include <WString.h>

class MPU
{
public:
  /**
   * @brief Return instance of singleton class
   * 
   * @return MPU& 
   */
  static MPU &Instance();

  /**
   * @brief Setup of MPU
   */
  void mpu_setup();
  /**
   * @brief Loop of MPU
   * 
   * @return String 
   */
  String mpu_loop();

  /**
   * @brief Calibration of MPU
   * 
   */
  void calibration();
  /**
   * @brief Disable MPU
   * 
   */
  void disable();
  /**
   * @brief Enable MPU
   * 
   */
  void enable();

private:
  /**
   * @brief Construct a new MPU object
   * 
   */
  MPU() { mpuInterrupt = false; }
  /**
   * @brief Destroy the MPU object
   * 
   */
  ~MPU() {}
  MPU(MPU const &) = delete;
  MPU &operator=(MPU const &) = delete;

  /**
   * @brief Calling when data from MPU is ready
   * 
   */
  static void dmpDataReady();

  bool enabled = false;
  bool isCalibrated = false;

  /**
   * @brief Indicates whether MPU interrupt pin has gone high
   * 
   */
  static volatile bool mpuInterrupt;
};

#endif