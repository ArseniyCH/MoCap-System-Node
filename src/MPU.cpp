/**
 * @brief 
 * 
 * @file MPU.cpp
 * @author Arseniy Churin
 * @date 2018-05-18
 */

#include "MPU.h"
#include "Arduino.h"

#include <WString.h>

#include "I2Cdev.h"

#include "MPU6050_6Axis_MotionApps20.h"

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

#define INTERRUPT_PIN 15

const char DEVICE_NAME[] = "mpu6050";

/*
    * MPU control/status vars
    */
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

Quaternion q; // [w, x, y, z]         quaternion container

MPU6050 _mpu;

volatile bool MPU::mpuInterrupt;

void MPU::dmpDataReady()
{
    MPU::mpuInterrupt = true;
}

MPU &MPU::Instance()
{
    static MPU s;
    return s;
}

void MPU::disable()
{
    enabled = false;
    _mpu.setSleepEnabled(true);
}

void MPU::enable()
{
    enabled = true;
    _mpu.setSleepEnabled(false);
}

void MPU::mpu_setup()
{
    Wire.begin();
    Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties

    // initialize device
    Serial.println(F("Initializing I2C devices..."));
    _mpu.initialize();
    pinMode(INTERRUPT_PIN, INPUT);

    // verify connection
    Serial.println(F("Testing device connections..."));
    Serial.println(_mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

    // load and configure the DMP
    Serial.println(F("Initializing DMP..."));
    devStatus = _mpu.dmpInitialize();

    // supply your own gyro offsets here, scaled for min sensitivity
    _mpu.setXGyroOffset(220);
    _mpu.setYGyroOffset(76);
    _mpu.setZGyroOffset(-85);
    _mpu.setZAccelOffset(1788); // 1688 factory default for my test chip

    // make sure it worked (returns 0 if so)
    if (devStatus == 0)
    {
        // turn on the DMP, now that it's ready
        Serial.println(F("Enabling DMP..."));
        _mpu.setDMPEnabled(true);

        // enable Arduino interrupt detection
        Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
        attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
        mpuIntStatus = _mpu.getIntStatus();

        // set our DMP Ready flag so the main loop() function knows it's okay to use it
        Serial.println(F("DMP ready! Waiting for first interrupt..."));
        dmpReady = true;

        // get expected DMP packet size for later comparison
        packetSize = _mpu.dmpGetFIFOPacketSize();
    }
    else
    {
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        Serial.print(F("DMP Initialization failed (code "));
        Serial.print(devStatus);
        Serial.println(F(")"));
    }
}

void MPU::mpu_loop(uint8_t *quat)
{
    if (!enabled)
        return;
    // if programming failed, don't try to do anything
    if (!dmpReady)
        return;

    // wait for MPU interrupt or extra packet(s) available
    if (!mpuInterrupt && fifoCount < packetSize)
        return;

    // reset interrupt flag and get INT_STATUS byte
    mpuInterrupt = false;
    mpuIntStatus = _mpu.getIntStatus();

    // get current FIFO count
    fifoCount = _mpu.getFIFOCount();

    // check for overflow (this should never happen unless our code is too inefficient)
    if ((mpuIntStatus & 0x10) || fifoCount == 1024)
    {
        // reset so we can continue cleanly
        _mpu.resetFIFO();
        Serial.println(F("FIFO overflow!"));

        // otherwise, check for DMP data ready interrupt (this should happen frequently)
    }
    else if (mpuIntStatus & 0x02)
    {
        // wait for correct available data length, should be a VERY short wait
        while (fifoCount < packetSize)
            fifoCount = _mpu.getFIFOCount();

        // read a packet from FIFO
        _mpu.getFIFOBytes(fifoBuffer, packetSize);

        // track FIFO count here in case there is > 1 packet available
        // (this lets us immediately read more without waiting for an interrupt)
        fifoCount -= packetSize;

        // display quaternion values in easy matrix form: w x y z
        _mpu.dmpGetQuaternion(&q, fifoBuffer);
        // uint8_t *w = (uint8_t *)&q.w,
        //         *x = (uint8_t *)&q.x,
        //         *y = (uint8_t *)&q.y,
        //         *z = (uint8_t *)&q.z;

        memcpy(quat, &q.w, sizeof(float));
        memcpy(quat + sizeof(float), &q.x, sizeof(float));
        memcpy(quat + 2 * sizeof(float), &q.y, sizeof(float));
        memcpy(quat + 3 * sizeof(float), &q.z, sizeof(float));
    }
}