#ifndef ACCEL_SERVICE_H
#define ACCEL_SERVICE_H

#include <bluefruit.h>

/* ACCEL SERVICE: 1a240001-c2ed-4d11-ad1e-fc06d8a02d37 (OPTIONAL - Service per Data Stream)
 update the ID
*/
const uint8_t accelServiceUuid128[] =          { 0x37, 0x2d, 0xa0, 0xd8, 0x06, 0xfc, 0x1e, 0xad, 0x11, 0x4d, 0xed, 0xc2, 0x01, 0x00, 0x24, 0x1a };
const uint8_t accelServiceDataChrUuid128[] =   { 0x37, 0x2d, 0xa0, 0xd8, 0x06, 0xfc, 0x1e, 0xad, 0x11, 0x4d, 0xed, 0xc2, 0x02, 0x00, 0x24, 0x1a };
const uint8_t accelServiceConfigChrUuid128[] = { 0x37, 0x2d, 0xa0, 0xd8, 0x06, 0xfc, 0x1e, 0xad, 0x11, 0x4d, 0xed, 0xc2, 0x03, 0x00, 0x24, 0x1a };

//define service here
BLEService accelService = BLEService(accelServiceUuid128);
BLECharacteristic accelDataChr = BLECharacteristic(accelServiceDataChrUuid128);
BLECharacteristic accelConfigChr = BLECharacteristic(accelServiceConfigChrUuid128);

const uint8_t accelConfigDataLen = 3;
/*
Configuration Array 
  -byte 0: Sensor state
    - 0 = off
    - 1 = streaming
  -bytes 1-2: desired sensor freq
*/

uint8_t accelConfigData[accelConfigDataLen] = { 0x00, 0x00, 0x20 };


const uint8_t accelDataLen = 240;
uint8_t accelDataBuf[accelDataLen] = { 0x00 };

//current position in data buffer
uint8_t accelCursor = 0;

//timestamp in millistec of last recorded data
uint32_t accelLastRecordedMS = 0;

//accel config BLE write call back (everytime config get written to this is called)
void accelConfigWriteCallback(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len)
{
  (void) conn_hdl;
  (void) chr;

  Serial.println("Did write accel service configuration");

  if (len == accelConfigDataLen) {
    for (uint8_t i = 0; i < len; i++) {
      accelConfigData[i] = data[i];
    }
  }
}

//tells the serve to begin 
//writing initial data to it
//setup GATT service using bluefruit
void setupAccelService() 
{
  accelService.begin();

  accelConfigChr.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  accelConfigChr.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  accelConfigChr.setFixedLen(accelConfigDataLen);
  accelConfigChr.setWriteCallback(accelConfigWriteCallback);
  accelConfigChr.begin();
  accelConfigChr.write(accelConfigData, accelConfigDataLen);

  accelDataChr.setProperties(CHR_PROPS_NOTIFY);
  accelDataChr.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  accelDataChr.setFixedLen(accelDataLen);
  accelDataChr.begin();
  accelDataChr.notify(accelDataBuf, accelDataLen);
}


//Telling us if service is enabled
bool isAccelServiceEnabled() {
  return accelConfigData[0] == 1;
}

//Getting first two elements of the array 
float accelServiceDesiredFrequencyMS() {
  uint16_t desiredFreqHZ = accelConfigData[1] << 8 | accelConfigData[2];
  return 1000.0 / desiredFreqHZ;
}


//Taking in four arguments 
void recordAccel(int16_t x, int16_t y, int16_t z, int32_t referenceTimeMS)
{ 
  if (! isAccelServiceEnabled()) {
    return;
  }
  
  //difference in time between accelerometer reads
  uint32_t recordOffset;

//once it reaches a point where it can't fit any more data (more than 13) will send data over to the buffer
  if ( accelCursor > (accelDataLen - 7))
  {
    if (accelDataChr.notifyEnabled())
    {
      Serial.println("Sending Accel Data");

      accelDataChr.notify(accelDataBuf, accelDataLen);
    }    
    accelCursor = 0;   
  }

// we just sent over data
//fills up first four bytes with anchor timestamp then set the offset to 0
  if ( accelCursor == 0 ) {
    // write the anchor time in the data buffer
    uint32_t anchorMS = millis() - referenceTimeMS;
    accelDataBuf[accelCursor++] = (anchorMS) & 0xFF;
    accelDataBuf[accelCursor++] = (anchorMS >> 8) & 0xFF;
    accelDataBuf[accelCursor++] = (anchorMS >> 16) & 0xFF;
    accelDataBuf[accelCursor++] = (anchorMS >> 24) & 0xFF;

    Serial.print("Timestamp: ");
    Serial.println(anchorMS);

    recordOffset = 0;
  } else {
    recordOffset = millis() - accelLastRecordedMS;    
  }

  accelLastRecordedMS = millis();

  Serial.print("last recorded accel: ");
  Serial.print(accelLastRecordedMS);
  Serial.print(" record offset: ");
  Serial.println(recordOffset);

//x
    accelDataBuf[accelCursor++] = (x >> 8) & 0xFF;
    accelDataBuf[accelCursor++] = (x) & 0xFF;
   
    
//y
    accelDataBuf[accelCursor++] = (y >> 8) & 0xFF;
    accelDataBuf[accelCursor++] = (y) & 0xFF;
    
   

//z
    accelDataBuf[accelCursor++] = (z >> 8) & 0xFF;
    accelDataBuf[accelCursor++] = (z) & 0xFF;    
    


  accelDataBuf[accelCursor++] = (uint8_t)recordOffset;
}


#endif // RANDOM_NUMBER_SERVICE_H