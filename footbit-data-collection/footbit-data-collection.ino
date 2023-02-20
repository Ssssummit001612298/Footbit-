#include <Adafruit_LSM6DS33.h>    // Library for support of LSM6DS33 chip
#include <Wire.h>
#include "MAX30105.h"

#include "ble.h"
#include "info_service.h"
#include "Accel_Service.h"
#include "PPG_Service.h"


MAX30105 particleSensor;
Adafruit_LSM6DS33 accelerometer;  // Create an accelerometer 

//#define debug Serial //Uncomment this line if you're using an Uno or ESP


// Possible widths: 69, 118, 215, 411us
//int width = 411; 

// PARTICLE SENSOR CONSTANTS
const int adcRange = 8192;
const uint8_t sampleAverage = 1;  
const uint8_t ledMode = 3;


uint32_t lastAccelRecording = millis();
uint32_t lastPPGRecording = millis();
//uint16_t adcRange = 0; 
//don't want to be able to change adcRange

uint16_t lastPPGBrightness = 0;
uint16_t lastPPGSampleRate = 0;
uint16_t lastPPGPulseWidth = 0;

void particleSensorConfigure() {
  if (lastPPGBrightness != getPPGBrightness() || lastPPGSampleRate != getPPGSampleRate() || lastPPGPulseWidth != getPPGPulseWidth()) {
    
    Serial.println("configuring particle sensor");
    particleSensor.setup(getPPGBrightness(), sampleAverage, ledMode, getPPGSampleRate(), getPPGPulseWidth(), adcRange);
    lastPPGBrightness = getPPGBrightness();
    lastPPGSampleRate = getPPGSampleRate();
    lastPPGPulseWidth = getPPGPulseWidth();

  }
}

void setup(){

  Serial.begin(115200);
  Serial.println("FlexiBLE nRF Bluefruit Example");
  Serial.println("-------------------------\n");
  setupBluefruit();
  Wire.begin();

  // Initialize sensor

  if (particleSensor.begin(Wire, I2C_SPEED_FAST) == false) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  
  accelerometer.begin_I2C();
  particleSensorConfigure();
}

void loop(){
    while (!isConnected) {
        Serial.println("");

        Serial.println("waiting for ble connection");
        delay(1000); 
    }
    
    while (!isEpochSet) {
        Serial.println("waiting for set of epoch time");
        delay(1000);
    }

    uint32_t now = millis();
    uint16_t accel_freq = accelServiceDesiredFrequencyMS();

    if ( (millis() - lastAccelRecording) >= accel_freq-1 && isAccelServiceEnabled()) {
      int32_t ax, ay, az;

      Serial.print("now: ");
      Serial.print(now);
      Serial.print(" accel freq: ");
      Serial.print(accel_freq);
      Serial.print(" time diff: ");
      Serial.print(now - lastAccelRecording);
      Serial.println();
      
      sensors_event_t accel, gyro, temp;

      accelerometer.getEvent(&accel, &gyro, &temp); 
      lastAccelRecording = millis();

      ax = (accel.acceleration.x * 100.0);  
      ay = (accel.acceleration.y * 100.0);
      az = (accel.acceleration.z * 100.0);

      Serial.print("ax: ");
      Serial.print(ax);
      Serial.print(" ay: ");
      Serial.print(ay);
      Serial.print(" az: ");
      Serial.println(az);

      // uint32_t refTime = now - referenceTimeMS;
      // Serial.print("refernece time: ");
      // Serial.println(refTime);
      uint32_t _start = millis();
      recordAccel(ax, ay, az, referenceTimeMS);
      Serial.print("time elapsed in recording BLE: ");
      Serial.println(millis() - _start);
    }
    
    if ( isPPGServiceEnabled() ) {
      particleSensorConfigure();
      // Check the sensor, read up to 3 samples
      particleSensor.check();

      if ( particleSensor.available() ) {

        uint32_t red = particleSensor.getFIFORed();
        uint32_t ir = particleSensor.getFIFOIR();
        uint32_t green = particleSensor.getFIFOGreen();

        Serial.print(red); 
        Serial.print(", ");
        Serial.print(ir);
        Serial.print(", ");      
        Serial.println(green);

        recordPPG(red, ir, green, referenceTimeMS);
        
        lastPPGRecording = millis();

        particleSensor.nextSample();
      }
    }
}