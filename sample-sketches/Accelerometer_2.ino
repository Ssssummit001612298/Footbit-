
#include <Adafruit_LSM6DS33.h>    // Library for support of LSM6DS33 chip
Adafruit_LSM6DS33 accelerometer;  // Create an accelerometer object

// unsure what the sampling rate is for this?? 

// --------------------------------------------------------------------------------
void setup() 
{
  Serial.begin(115200);
  accelerometer.begin_I2C();        //  Start the I2C interface to the sensors
}

// --------------------------------------------------------------------------------
void loop() 
{
  float ax, ay, az;
  sensors_event_t accel, gyro, temp;

  accelerometer.getEvent(&accel, &gyro, &temp);  //  get the data

  ax = accel.acceleration.x;   //  Copy to convenient variables. Not necessary
  ay = accel.acceleration.y;
  az = accel.acceleration.z;

  Serial.print(ax);                          //  Print plain data so that
  Serial.print("  ");   Serial.print(ay);    //  Serial Plotter can be used
  Serial.print("  ");   Serial.println(az);

  //delay(25);                   //  Arbitrary delay to slow down display
}
