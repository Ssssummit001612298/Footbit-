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
int adcRange = 8192;
 byte sampleAverage = 1;
 byte ledMode = 3;


int pulseWidthVal;
int sampleVal;


uint32_t lastAccelRecording = millis();
uint32_t lastPPGRecording = millis();
uint16_t ppgSampleRate = 0;
// Possible samples: 50, 100, 200, 400, 800, 1000, 1600, 3200 samples/second
//1600 and 3200 not really poss
uint16_t ppgPulseWidth = 0;

uint16_t PPGBrightness = 0;
//uint16_t adcRange = 0; 
//don't want to be able to change adcRange

/


void setup(){

  Serial.begin(115200);

  Serial.println("FlexiBLE nRF Bluefruit Example");
  Serial.println("-------------------------\n");
  setupBluefruit();

  Wire.begin();
  int result = bioHub.begin();
 // int result = MAX30105.begin();
// ^unsure what biohub should be replaced with


  if (result == 0) // Zero errors!
    Serial.println("Sensor started!");

  Serial.println("Configuring Sensor...."); 
  int error = MAX30105.configSensor(); // Configure Sensor and BPM mode , MODE_TWO also available
  if (error == 0){// Zero errors.
    Serial.println("Sensor configured.");
  }
  else {
    Serial.println("Error configuring sensor.");
    Serial.print("Error: "); 
    Serial.println(error); 
  }

  // Set pulse width.
  error = MAX30105.setPulseWidth(width);
  if (error == 0){// Zero errors.
    Serial.println("Pulse Width Set.");
  }
  else {
    Serial.println("Could not set Pulse Width.");
    Serial.print("Error: "); 
    Serial.println(error); 
  }

  // Check that the pulse width was set. 
  pulseWidthVal = MAX30105.readPulseWidth();
  Serial.print("Pulse Width: ");
  Serial.println(pulseWidthVal);

  // Set sample rate per second. Remember that not every sample rate is
  // available with every pulse width. Check hookup guide for more information.  
  error = MAX30105.setSampleRate(samples);
  if (error == 0){// Zero errors.
    Serial.println("Sample Rate Set.");
  }
  else {
    Serial.println("Could not set Sample Rate!");
    Serial.print("Error: "); 
    Serial.println(error); 
  }

  // Check sample rate.
  Serial.print("Sample rate is set to: ");
  Serial.println(sampleVal); 
  
  // Data lags a bit behind the sensor, if you're finger is on the sensor when
  // it's being configured this delay will give some time for the data to catch
  // up. 
  Serial.println("Loading up the buffer with data....");
  delay(4000);

  accelerometer.begin_I2C();

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);

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
    uint16_t ppg_freq = ppgServiceDesiredFrequencyMS();

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
    
    if ( (millis() - lastPPGRecording) >= ppg_freq-4 && isPPGServiceEnabled()) {
///getSampleRate
      if ( ppgSampleRate != getPPGSampleRate() && getPPGSampleRate() > 0 ) {
        Serial.println("setting ppg sample rate");
        ppgSampleRate = getPPGSampleRate();
        uint8_t error = MAX30105.setSampleRate(ppgSampleRate);
        if (error == 0){// Zero errors.
          Serial.println("Pulse Width Set.");
        }
      }
///getPulseWidth
      if ( ppgPulseWidth != getPPGPulseWidth() && getPPGPulseWidth() > 0 ) {
        Serial.println("setting ppg pulse rate");
        ppgPulseWidth = getPPGPulseWidth();
        uint8_t error = MAX30105.setPulseWidth(ppgPulseWidth);
        if (error == 0){// Zero errors.
          Serial.println("Pulse Width Set.");
        }
///getPPGBrightness
        if ( getPPGBrightness != getPPGBrightness() && getPPGBrightness() > 0 ) {
        Serial.println("setting ppg brightness");
        ppgBrightness = getPPGBrightness();
        uint8_t error = MAX30105.setSampleRate(ppgBrightness); 
        if (error == 0){// Zero errors.
          Serial.println("PPG Brightness Set.");
        }
      }

  

      uint32_t start = millis();
      
      
      lastPPGRecording = millis();

      Serial.print(particleSensor.getRed()); 
      Serial.print(", ");
      Serial.print(particleSensor.getIR());
      Serial.print(", ");      
      Serial.println(particleSensor.getGreen());
      Serial.print("sensor read: ");
      Serial.println(millis() - start);

      recordPPG(particleSensor.getRed, particleSensor.getIR, particleSensor.getGreen, referenceTimeMS); //body.greenLed,
    }
}