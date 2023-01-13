#ifndef RANDOM_NUMBER_SERVICE_H
#define RANDOM_NUMBER_SERVICE_H

#include <bluefruit.h>

/* RANDOM NUMBER SERVICE: 1a230001-c2ed-4d11-ad1e-fc06d8a02d37 (OPTIONAL - Service per Data Stream)

*/
const uint8_t randomNumberServiceUuid128[] =          { 0x37, 0x2d, 0xa0, 0xd8, 0x06, 0xfc, 0x1e, 0xad, 0x11, 0x4d, 0xed, 0xc2, 0x01, 0x00, 0x23, 0x1a };
const uint8_t randomNumberServiceDataChrUuid128[] =   { 0x37, 0x2d, 0xa0, 0xd8, 0x06, 0xfc, 0x1e, 0xad, 0x11, 0x4d, 0xed, 0xc2, 0x02, 0x00, 0x23, 0x1a };
const uint8_t randomNumberServiceConfigChrUuid128[] = { 0x37, 0x2d, 0xa0, 0xd8, 0x06, 0xfc, 0x1e, 0xad, 0x11, 0x4d, 0xed, 0xc2, 0x03, 0x00, 0x23, 0x1a };
BLEService randomNumberService = BLEService(randomNumberServiceUuid128);
BLECharacteristic randomNumberDataChr = BLECharacteristic(randomNumberServiceDataChrUuid128);
BLECharacteristic randomNumberConfigChr = BLECharacteristic(randomNumberServiceConfigChrUuid128);
const uint8_t randomNumberConfigDataLen = 4;
uint8_t randomNumberConfigData[randomNumberConfigDataLen] = { 0x00, 0x00, 0x20, 0x00 };
const uint8_t randomNumberDataLen = 240;
uint8_t randomNumberDataBuf[randomNumberDataLen] = { 0x00 };

uint8_t rnCursor = 0;
uint32_t rnLastRecordedMS = 0;

void rn_config_write_callback(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len)
{
  (void) conn_hdl;
  (void) chr;

  Serial.println("Did write random number service configuration");

  if (len == randomNumberConfigDataLen) {
    for (uint8_t i = 0; i < len; i++) {
      randomNumberConfigData[i] = data[i];
    }
  }
}

void rn_data_write_callback(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len)
{
  (void) conn_hdl;
  (void) chr;

  Serial.println("Did write random number service data");

  if (len == randomNumberConfigDataLen) {
    for (uint8_t i = 0; i < len; i++) {
      randomNumberDataBuf[i] = data[i];     
    }
  }
}

void setupRandomNumberService() 
{
  randomNumberService.begin();

  randomNumberConfigChr.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  randomNumberConfigChr.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  randomNumberConfigChr.setFixedLen(randomNumberConfigDataLen);
  randomNumberConfigChr.setWriteCallback(rn_config_write_callback);
  randomNumberConfigChr.begin();
  randomNumberConfigChr.write(randomNumberConfigData, randomNumberConfigDataLen);

  randomNumberDataChr.setProperties(CHR_PROPS_NOTIFY);
  randomNumberDataChr.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  randomNumberDataChr.setFixedLen(randomNumberDataLen);
  randomNumberDataChr.begin();
  randomNumberDataChr.notify(randomNumberDataBuf, randomNumberDataLen);
}

bool _isRandomNumberDataStreamEnabled() {
  return randomNumberConfigData[0] == 1;
}

uint16_t _randomNumberDesiredFrequencyMS() {
  uint16_t desiredFreqHZ = randomNumberConfigData[1] << 8 | randomNumberConfigData[2];
  return 1000 / desiredFreqHZ;
}

void _recordRandomNumber(uint8_t group, uint32_t referenceTimeMS)
{ 
  if (!_isRandomNumberDataStreamEnabled()) {
    return;
  }
  
  uint32_t recordOffset;

  if ( rnCursor > (randomNumberDataLen - 3))
  {
    if (randomNumberDataChr.notifyEnabled())
    {
      Serial.println("Sending Random Number Data");

      randomNumberDataChr.notify(randomNumberDataBuf, randomNumberDataLen);
    }    
    rnCursor = 0;   
  }

  if ( rnCursor == 0 ) {
    // write the anchor time in the data buffer
    uint32_t anchorMS = millis() - referenceTimeMS;
    randomNumberDataBuf[rnCursor++] = (anchorMS) & 0xFF;
    randomNumberDataBuf[rnCursor++] = (anchorMS >> 8) & 0xFF;
    randomNumberDataBuf[rnCursor++] = (anchorMS >> 16) & 0xFF;
    randomNumberDataBuf[rnCursor++] = (anchorMS >> 24) & 0xFF;

    Serial.print("Timestamp: ");
    Serial.println(anchorMS);

    recordOffset = 0;
  } else {
    recordOffset = millis() - rnLastRecordedMS;    
  }
    
  // generate and write value
  uint8_t value = (uint8_t)random(255);
  rnLastRecordedMS = millis();
  randomNumberDataBuf[rnCursor++] = value;
  randomNumberDataBuf[rnCursor++] = group;
  randomNumberDataBuf[rnCursor++] = (uint8_t)recordOffset;
  Serial.print("Recording random number -> value: ");
  Serial.print(value);
  Serial.print(" cursor: ");
  Serial.print(rnCursor);
  Serial.print(" offset ms: ");
  Serial.println((uint8_t) recordOffset);
}

#endif // RANDOM_NUMBER_SERVICE_H