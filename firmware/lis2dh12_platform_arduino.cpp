#include "Wire.h"
#include "lis2dh12_platform_arduino.h"

#define LIS2DH12_ADDR AM_BSP_ACCELEROMETER_I2C_ADDRESS
#define LIS2DH12_WHO_AM_I              0x0FU


int32_t lis2dh12_write_platform_arduino(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
  Wire.beginTransmission(LIS2DH12_ADDR);
  Wire.write(reg);
  Wire.write(bufp, len);
  byte ret = Wire.endTransmission();

//  Serial.print("writing... ");
//  if(ret){
//    Serial.print("error on write! ");
//    Serial.print(ret);
//  }else{
//    Serial.print("success!");
//  }
//  Serial.println();
  return 0;
}

int32_t lis2dh12_read_platform_arduino(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{

//  Serial.print("[");
  
  for(size_t idx = 0; idx < len; idx++){

    while(Wire.available()){
      Wire.read();
    }
    
    Wire.beginTransmission(LIS2DH12_ADDR);
    Wire.write(reg + idx);
    Wire.endTransmission(false);

    uint16_t got = Wire.requestFrom(LIS2DH12_ADDR, 1);
    if(!got){
      Serial.println("length mismatch!");
      return -1;
    }

    *(bufp + idx) = Wire.read();
    
//    Serial.print(*(bufp + idx));
//    Serial.print(", ");
  }

//  Serial.print("] ");
//
//  Serial.print("completed read!");
//  Serial.println();
  
  return 0;
}
