#include <Adafruit_LSM6DS33.h>    // Library for support of LSM6DS33 chip
#include <SparkFun_Bio_Sensor_Hub_Library.h>
#include <Wire.h>

#include "ble.h"
#include "info_service.h"

// Reset pin, MFIO pin
int resPin = A4;
int mfioPin = A1;

// Possible widths: 69, 118, 215, 411us
int width = 411; 
// Possible samples: 50, 100, 200, 400, 800, 1000, 1600, 3200 samples/second
// Not every sample amount is possible with every width; check out our hookup
// guide for more information.
int samples = 100; 
int pulseWidthVal;
int sampleVal;

// Takes address, reset pin, and MFIO pin.
SparkFun_Bio_Sensor_Hub bioHub(resPin, mfioPin); 

Adafruit_LSM6DS33 accelerometer;  // Create an accelerometer object

bioData body; 
// ^^^^^^^^^
// What's this!? This is a type (like "int", "byte", "long") unique to the SparkFun
// Pulse Oximeter and Heart Rate Monitor. Unlike those other types it holds
// specific information on the LED count values of the sensor and ALSO the
// biometric data: heart rate, oxygen levels, and confidence. "bioLedData" is 
// actually a specific kind of type, known as a "struct". I chose the name
// "body" but you could use another variable name like "blood", "readings",
// "ledBody" or whatever. Using the variable in the following way gives the
// following data: 
// body.irLed      - Infrared LED counts. 
// body.redLed     - Red LED counts. 
// body.heartrate  - Heartrate
// body.confidence - Confidence in the heartrate value
// body.oxygen     - Blood oxygen level
// body.status     - Has a finger been sensed?

void setup(){

  Serial.begin(115200);

  Serial.println("FlexiBLE nRF Bluefruit Example");
  Serial.println("-------------------------\n");
  setupBluefruit();

  Wire.begin();
  int result = bioHub.begin();
  if (result == 0) // Zero errors!
    Serial.println("Sensor started!");

  Serial.println("Configuring Sensor...."); 
  int error = bioHub.configSensor(); // Configure Sensor and BPM mode , MODE_TWO also available
  if (error == 0){// Zero errors.
    Serial.println("Sensor configured.");
  }
  else {
    Serial.println("Error configuring sensor.");
    Serial.print("Error: "); 
    Serial.println(error); 
  }

  // Set pulse width.
  error = bioHub.setPulseWidth(width);
  if (error == 0){// Zero errors.
    Serial.println("Pulse Width Set.");
  }
  else {
    Serial.println("Could not set Pulse Width.");
    Serial.print("Error: "); 
    Serial.println(error); 
  }

  // Check that the pulse width was set. 
  pulseWidthVal = bioHub.readPulseWidth();
  Serial.print("Pulse Width: ");
  Serial.println(pulseWidthVal);

  // Set sample rate per second. Remember that not every sample rate is
  // available with every pulse width. Check hookup guide for more information.  
  error = bioHub.setSampleRate(samples);
  if (error == 0){// Zero errors.
    Serial.println("Sample Rate Set.");
  }
  else {
    Serial.println("Could not set Sample Rate!");
    Serial.print("Error: "); 
    Serial.println(error); 
  }

  // Check sample rate.
  sampleVal = bioHub.readSampleRate();
  Serial.print("Sample rate is set to: ");
  Serial.println(sampleVal); 
  
  // Data lags a bit behind the sensor, if you're finger is on the sensor when
  // it's being configured this delay will give some time for the data to catch
  // up. 
  Serial.println("Loading up the buffer with data....");
  delay(4000);

  accelerometer.begin_I2C();

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


    body = bioHub.readSensor();
    Serial.print(body.irLed); 
    Serial.print(", ");
    Serial.println(body.redLed);

    float ax, ay, az;
    sensors_event_t accel, gyro, temp;

    accelerometer.getEvent(&accel, &gyro, &temp);  //  get the data

    ax = accel.acceleration.x;   //  Copy to convenient variables. Not necessary
    ay = accel.acceleration.y;
    az = accel.acceleration.z;

    Serial.print(ax);                          //  Print plain data so that
    Serial.print("  ");   Serial.print(ay);    //  Serial Plotter can be used
    Serial.print("  ");   Serial.println(az);
}