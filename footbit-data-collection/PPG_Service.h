#ifndef PPG_SERVICE_H
#define PPG_SERVICE_H

#include <bluefruit.h>

/* PPG SERVICE: 1a240001-c2ed-4d11-ad1e-fc06d8a02d37 (OPTIONAL - Service per Data Stream)
 update the ID 
 */


#//https://learn.sparkfun.com/tutorials/max30105-particle-and-pulse-ox-sensor-hookup-guide?_ga=2.261135112.401486308.1672934969-1889964623.1672934969#advanced-functions

const uint8_t ppgServiceUuid128[] =          { 0x37, 0x2d, 0xa0, 0xd8, 0x06, 0xfc, 0x1e, 0xad, 0x11, 0x4d, 0xed, 0xc2, 0x01, 0x00, 0x25, 0x1a };
const uint8_t ppgServiceDataChrUuid128[] =   { 0x37, 0x2d, 0xa0, 0xd8, 0x06, 0xfc, 0x1e, 0xad, 0x11, 0x4d, 0xed, 0xc2, 0x02, 0x00, 0x25, 0x1a };
const uint8_t ppgServiceConfigChrUuid128[] = { 0x37, 0x2d, 0xa0, 0xd8, 0x06, 0xfc, 0x1e, 0xad, 0x11, 0x4d, 0xed, 0xc2, 0x03, 0x00, 0x25, 0x1a };

//define service here
BLEService ppgService = BLEService(ppgServiceUuid128);
BLECharacteristic ppgDataChr = BLECharacteristic(ppgServiceDataChrUuid128);
BLECharacteristic ppgConfigChr = BLECharacteristic(ppgServiceConfigChrUuid128);

/*
Configuration Array 
  - byte 0: Sensor state
    - 0 = off
    - 1 = streaming
  - byte 1: Sensor Sample Rate (Hz)
   - 0 = 50
   - 1 = 100
   - 2 = 200
   - 3 = 400
  - byte 2: Senor LED Pulse Width
    - 0 = 69us
    - 1 = 118us
    - 2 = 215us
    - 3 = 411us
  - byte 3: Senor LED brightness (0-255)
*/
const uint8_t ppgConfigDataLen = 4;
uint8_t ppgConfigData[ppgConfigDataLen] = {0x00, 0x00, 0x00, 0x00};

const uint8_t ppgDataLen = 240;
// --> const uint8_t ppgDataLen = ?;
// --> def have to change but unsure to what

uint8_t ppgDataBuf[ppgDataLen] = { 0x00 };

//current position in data buffer
uint8_t ppgCursor = 0;

//timestamp in millistec of last recorded data
uint32_t ppgLastRecordedMS = 0;

//accel config BLE write call back (everytime config get written to this is called)
void ppgConfigWriteCallback(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len)
{
  (void) conn_hdl;
  (void) chr;

  Serial.println("Did write ppg service configuration");

  if (len == ppgConfigDataLen) {
    for (uint8_t i = 0; i < len; i++) {
      ppgConfigData[i] = data[i];
    }
  }
}

//tells the serve to begin 
//writing initial data to it
//setup GATT service using bluefruit
void setupPPGService() 
{
  ppgService.begin();
  ppgConfigChr.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  ppgConfigChr.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  ppgConfigChr.setFixedLen(ppgConfigDataLen);
  ppgConfigChr.setWriteCallback(ppgConfigWriteCallback);
  ppgConfigChr.begin();
  ppgConfigChr.write(ppgConfigData,ppgConfigDataLen);

  ppgDataChr.setProperties(CHR_PROPS_NOTIFY);
  ppgDataChr.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  ppgDataChr.setFixedLen(ppgDataLen);
  ppgDataChr.begin();
  ppgDataChr.notify(ppgDataBuf, ppgDataLen);
}


// CONFIGURATION GETS
bool isPPGServiceEnabled() {
  return ppgConfigData[0] == 1;
}

uint16_t getPPGSampleRate() {
  switch (ppgConfigData[1]) {
    case 0: return 50;
    case 1: return 100;
    case 2: return 200;
    case 3: return 400;
    default: return 0;
  }
}

uint16_t getPPGPulseWidth() {
  switch (ppgConfigData[2]) {
    case 0: return 69;
    case 1: return 118;
    case 2: return 215;
    case 3: return 411;
    default: return 0;
  }
}

uint8_t getPPGBrightness() {
  return ppgConfigData[3];
}

//--> Change so taking in green as well
void recordPPG(uint32_t red, uint32_t ir, uint32_t green, int32_t referenceTimeMS)
{ 
  if (!isPPGServiceEnabled()) {
    return;
  }
  
  //difference in time between accelerometer reads
  uint32_t recordOffset;

//once it reaches a point where it can't fit any more data (more than 13) will send data over to the buffer
  if ( ppgCursor > (ppgDataLen - 9))
  
  {
    if (ppgDataChr.notifyEnabled())
    {
      Serial.println("Sending Ppg Data");
      ppgDataChr.notify(ppgDataBuf, ppgDataLen);
    }    
    ppgCursor = 0;   
  }

// we just sent over data
//for accelerometer are the four bytes, buffer, x, y and z 
// so ppg would be buffer, ppg red and ppg IR 
//fills up first four bytes with anchor timestamp then set the offset to 0
  if ( ppgCursor == 0 ) {
    // write the anchor time in the data buffer
    uint32_t anchorMS = millis() - referenceTimeMS;
    ppgDataBuf[ppgCursor++] = (anchorMS) & 0xFF;
    ppgDataBuf[ppgCursor++] = (anchorMS >> 8) & 0xFF;
    ppgDataBuf[ppgCursor++] = (anchorMS >> 16) & 0xFF;
    ppgDataBuf[ppgCursor++] = (anchorMS >> 24) & 0xFF;

    Serial.print("Timestamp: ");
    Serial.println(anchorMS);

    recordOffset = 0;
  } else {
    recordOffset = millis() - ppgLastRecordedMS;    
  }

  ppgLastRecordedMS = millis();

//red
  ppgDataBuf[ppgCursor++] = (red >> 24) & 0xFF;
  ppgDataBuf[ppgCursor++] = (red >> 16) & 0xFF;
  ppgDataBuf[ppgCursor++] = (red >> 8) & 0xFF;
  ppgDataBuf[ppgCursor++] = (red) & 0xFF;

//IR
  ppgDataBuf[ppgCursor++] = (ir >> 24) & 0xFF;
  ppgDataBuf[ppgCursor++] = (ir >> 16) & 0xFF;
  ppgDataBuf[ppgCursor++] = (ir >> 8) & 0xFF;
  ppgDataBuf[ppgCursor++] = (ir) & 0xFF;

//--> adding in green value
  ppgDataBuf[ppgCursor++] = (green >> 24) & 0xFF;
  ppgDataBuf[ppgCursor++] = (green >> 16) & 0xFF;
  ppgDataBuf[ppgCursor++] = (green >> 8) & 0xFF;
  ppgDataBuf[ppgCursor++] = (green) & 0xFF;

  ppgDataBuf[ppgCursor++] = (uint8_t)recordOffset;
  Serial.print("record offset: ");
  Serial.println(recordOffset)
}


#endif // PPG_SERVICE_H