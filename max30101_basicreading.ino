/*
  MAX30105 Breakout: Output all the raw Red/IR/Green readings
  By: Nathan Seidle @ SparkFun Electronics
  Date: October 2nd, 2016
  https://github.com/sparkfun/MAX30105_Breakout

  Outputs all Red/IR/Green values.

  Hardware Connections (Breakoutboard to Arduino):
  -5V = 5V (3.3V is allowed)
  -GND = GND
  -SDA = A4 (or SDA)
  -SCL = A5 (or SCL)
  -INT = Not connected

  The MAX30105 Breakout can handle 5V or 3.3V I2C logic. We recommend powering the board with 5V
  but it will also run at 3.3V.

  This code is released under the [MIT License](http://opensource.org/licenses/MIT).
*/
#include <Adafruit_LSM6DS33.h>    // Library for support of LSM6DS33 chip
#include <Wire.h>
#include "MAX30105.h"

MAX30105 particleSensor;
Adafruit_LSM6DS33 accelerometer;  // Create an accelerometer 

#define debug Serial //Uncomment this line if you're using an Uno or ESP
//#define debug SerialUSB //Uncomment this line if you're using a SAMD21

void setup()
{
  //debug.begin(9600); //tried to change to 115200 but won't work 
  Serial.begin(115200);
  accelerometer.begin_I2C();  

    Wire.begin();
  debug.println("MAX30105 Basic Readings Example");

  // Initialize sensor
  if (particleSensor.begin() == false)
  {
    debug.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }

  //Setup to sense a nice looking saw tooth on the plotter
  byte ledBrightness = 0x1F; //Options: 0=Off to 255=50mA
  //byte sampleAverage = 8; //Options: 1, 2, 4, 8, 16, 32
  // Also unsure whether to keep this

  byte ledMode = 3; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  int sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384

  //??? ^unsure what changing this will do

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor. Use 6.4mA for LED drive
}





void loop()
{
  debug.print(" R[");
  debug.print(particleSensor.getRed());
  debug.print("] IR[");
  debug.print(particleSensor.getIR());
  debug.print("] G[");
  debug.print(particleSensor.getGreen());
  debug.print("]");

  debug.println();

  float ax, ay, az;
  sensors_event_t accel, gyro, temp;

  accelerometer.getEvent(&accel, &gyro, &temp);  //  get the data

  ax = accel.acceleration.x;   //  Copy to convenient variables. Not necessary
  ay = accel.acceleration.y;
  az = accel.acceleration.z;

  debug.print(ax);                          //  Print plain data so that
  debug.print(",");   
  debug.print(ay);    //  Serial Plotter can be used
  debug.print(",");   
  debug.println(az);
}


