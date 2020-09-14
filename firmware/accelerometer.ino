
#include "Wire.h"

#define LIS2DH12_ADDR AM_BSP_ACCELEROMETER_I2C_ADDRESS

#define LIS2DH12_WHO_AM_I              0x0FU


int32_t lis2dh12_write_platform_apollo3_arduino(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
  Wire.beginTransmission(LIS2DH12_ADDR);
  Wire.write(reg);
  Wire.write(bufp, len);
  Wire.endTransmission();

  return 0;
}

/*
 * @brief  Read generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to read
 * @param  bufp      pointer to buffer that store the data read
 * @param  len       number of consecutive register to read
 *
 */
int32_t lis2dh12_read_platform_apollo3(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
  Wire.beginTransmission(LIS2DH12_ADDR);
  Wire.write(reg);
  Wire.endTransmission();

  uint16_t got = Wire.requestFrom(LIS2DH12_ADDR, len);

  if(got != len){ return -1; }
  if(got > len){ return -1; }

  for(size_t idx = 0; idx < got; idx++){
    *(bufp + idx) = Wire.read();
  }
  
  return 0;
}


void getAccMag( void ){

  int32_t ret;
  uint8_t buff = 0x00;
  ret = lis2dh12_read_platform_apollo3(null, LIS2DH12_WHO_AM_I, &buff, 1);
  return ret;

  Serial.print("tried to read whoami: ");
  Serial.print(buff);
  Serial.println();
}
