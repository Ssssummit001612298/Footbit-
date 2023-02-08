#ifndef INFO_SERVICE_H
#define INFO_SERVICE_H

#include <bluefruit.h>

/* INFO SERVICE: 1a220001-c2ed-4d11-ad1e-fc06d8a02d37 (REQUIRED)
*/
const uint8_t infoServiceUuid128[] =                { 0x37, 0x2d, 0xa0, 0xd8, 0x06, 0xfc, 0x1e, 0xad, 0x11, 0x4d, 0xed, 0xc2, 0x01, 0x00, 0x22, 0x1a };
// characteristic that holds the a epoch timestamp
const uint8_t infoServiceEpochChrUuid128[] =        { 0x37, 0x2d, 0xa0, 0xd8, 0x06, 0xfc, 0x1e, 0xad, 0x11, 0x4d, 0xed, 0xc2, 0x02, 0x00, 0x22, 0x1a };
// read only characteristic that holds the specification version
const uint8_t infoServiceSpecVersionChrUuid128[] =  { 0x37, 0x2d, 0xa0, 0xd8, 0x06, 0xfc, 0x1e, 0xad, 0x11, 0x4d, 0xed, 0xc2, 0x03, 0x00, 0x22, 0x1a };
// read only characteristic that holds the specification id
const uint8_t infoServiceSpecIdChrUuid128[] =       { 0x37, 0x2d, 0xa0, 0xd8, 0x06, 0xfc, 0x1e, 0xad, 0x11, 0x4d, 0xed, 0xc2, 0x04, 0x00, 0x22, 0x1a };
// notify characteristic that indicates that the epoch time should be refreshed (not used in this example, always set to 0)
const uint8_t infoServiceRefreshEpochChrUuid128[] = { 0x37, 0x2d, 0xa0, 0xd8, 0x06, 0xfc, 0x1e, 0xad, 0x11, 0x4d, 0xed, 0xc2, 0x05, 0x00, 0x22, 0x1a };

// BlueFruit BLE objects
BLEService infoService = BLEService(infoServiceUuid128);
BLECharacteristic epochChr = BLECharacteristic(infoServiceEpochChrUuid128);
BLECharacteristic specVersionChr = BLECharacteristic(infoServiceSpecVersionChrUuid128);
BLECharacteristic specIdChr = BLECharacteristic(infoServiceSpecIdChrUuid128);
BLECharacteristic refreshEpochChr = BLECharacteristic(infoServiceRefreshEpochChrUuid128);

// epoch timestamp variables
const uint8_t epochDataLen = 8;
uint8_t epochDataBuf[epochDataLen] = { 0x00 };

bool isEpochSet = false;
uint32_t referenceTimeMS = 0;

// when the epoch timestamp is written, keep note the system time
void epoch_write_callback(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len)
{
  (void) conn_hdl;
  (void) chr;
  
  referenceTimeMS = millis();
  Serial.print("Did write epoch time: ");

  if (len == epochDataLen) {
    for (uint8_t i = 0; i < len; i++) {
      epochDataBuf[i] = data[i];
      Serial.print(epochDataBuf[i]);
      Serial.print("\r");
    }
  }
  Serial.println("");

  isEpochSet = true;
}

void setupInfoService()
{
  infoService.begin();

  // setup epoch characteristic
  epochChr.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  epochChr.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  epochChr.setFixedLen(8);
  epochChr.setWriteCallback(epoch_write_callback);
  epochChr.begin();
  epochChr.write(epochDataBuf, epochDataLen); 

  // setup spec version characteristic
  specVersionChr.setProperties(CHR_PROPS_READ);
  specVersionChr.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  specVersionChr.setFixedLen(3);
  specVersionChr.begin();
  uint8_t specVersion[3] = { 0x00, 0x03, 0x06 };
  specVersionChr.write(specVersion, 3);

  // setup spec id characteristic
  specIdChr.setProperties(CHR_PROPS_READ);
  specIdChr.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  specIdChr.setFixedLen(24);
  specIdChr.begin();
  uint8_t specId[24] = { 0x66, 0x6F, 0x6F, 0x74, 0x62, 0x69, 0x74, 0x2D, 0x73, 0x70, 0x65, 0x63 }; // "footbit-spec"
  specIdChr.write(specId, 24);

  // setup refresh epoch characteristic
  refreshEpochChr.setProperties(CHR_PROPS_NOTIFY);
  refreshEpochChr.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  refreshEpochChr.setFixedLen(1);
  refreshEpochChr.begin();
  refreshEpochChr.notify(0); 
}

#endif  // INFO_SERVICE_H



// #ifndef INFO_SERVICE_H
// #define INFO_SERVICE_H

// #include <bluefruit.h>

// /* INFO SERVICE: 1a220001-c2ed-4d11-ad1e-fc06d8a02d37 (REQUIRED)

// */
// const uint8_t infoServiceUuid128[] =               { 0x37, 0x2d, 0xa0, 0xd8, 0x06, 0xfc, 0x1e, 0xad, 0x11, 0x4d, 0xed, 0xc2, 0x01, 0x00, 0x22, 0x1a };
// const uint8_t infoServiceEpochChrUuid128[] =       { 0x37, 0x2d, 0xa0, 0xd8, 0x06, 0xfc, 0x1e, 0xad, 0x11, 0x4d, 0xed, 0xc2, 0x02, 0x00, 0x22, 0x1a };
// const uint8_t infoServiceSpecVersionChrUuid128[] = { 0x37, 0x2d, 0xa0, 0xd8, 0x06, 0xfc, 0x1e, 0xad, 0x11, 0x4d, 0xed, 0xc2, 0x03, 0x00, 0x22, 0x1a };
// const uint8_t infoServiceSpecIdChrUuid128[] =      { 0x37, 0x2d, 0xa0, 0xd8, 0x06, 0xfc, 0x1e, 0xad, 0x11, 0x4d, 0xed, 0xc2, 0x04, 0x00, 0x22, 0x1a };
// BLEService infoService = BLEService(infoServiceUuid128);
// BLECharacteristic epochChr = BLECharacteristic(infoServiceEpochChrUuid128);
// BLECharacteristic specVersionChr = BLECharacteristic(infoServiceSpecVersionChrUuid128);
// BLECharacteristic specIdChr = BLECharacteristic(infoServiceSpecIdChrUuid128);
// const uint8_t epochDataLen = 8;
// uint8_t epochDataBuf[epochDataLen] = { 0x00 };

// bool isEpochSet = false;
// uint32_t referenceTimeMS = 0;

// void epoch_write_callback(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len)
// {
//   (void) conn_hdl;
//   (void) chr;
  
//   referenceTimeMS = millis();
//   Serial.print("Did write epoch time: ");

//   if (len == epochDataLen) {
//     for (uint8_t i = 0; i < len; i++) {
//       epochDataBuf[i] = data[i];
//       Serial.print(epochDataBuf[i]);
//       Serial.print("\r");
//     }
//   }
//   Serial.println("");

//   isEpochSet = true;
// }

// void setupInfoService()
// {
//   infoService.begin();

//   epochChr.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
//   epochChr.setPermission(SECMODE_OPEN, SECMODE_OPEN);
//   epochChr.setFixedLen(8);
//   epochChr.setWriteCallback(epoch_write_callback);

//   epochChr.begin();


//   epochChr.write(epochDataBuf, epochDataLen); 

//   specVersionChr.setProperties(CHR_PROPS_READ);
//   specVersionChr.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
//   specVersionChr.setFixedLen(3);
//   specVersionChr.begin();
//   uint8_t specVersion[3] = { 0x00, 0x03, 0x05 };
//   specVersionChr.write(specVersion, 3);

//   specIdChr.setProperties(CHR_PROPS_READ);
//   specIdChr.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
//   specIdChr.setFixedLen(24);
//   specIdChr.begin();
//   uint8_t specId[24] = { 0x66, 0x6F, 0x6F, 0x74, 0x62, 0x69, 0x74, 0x2D, 0x73, 0x70, 0x65, 0x63 }; // "footbit-spec"
//   specIdChr.write(specId, 24);
// }

// #endif  // INFO_SERVICE_H