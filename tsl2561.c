/**************************************************************************/
/*! 
    @file     tsl2561.c
    @author   K. Townsend (microBuilder.eu / adafruit.com)
    @section LICENSE
    Software License Agreement (BSD License)
    Copyright (c) 2010, microBuilder SARL, Adafruit Industries
    All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**************************************************************************/

#include <stdlib.h>

#include "tsl2561.h"

void TSL2561_TSL2561(uint8_t addr) {
  _addr = addr;
  _initialized = false;
  _integration = TSL2561_INTEGRATIONTIME_13MS;
  _gain = TSL2561_GAIN_16X;

  // we cant do wire initialization till later, because we havent loaded Wire yet
}

boolean TSL2561_begin(void) {
    mI2C_Start();

    mI2C_MasterSendStart(_addr, 0, 10);
    mI2C_MasterWriteByte(TSL2561_REGISTER_ID, 10);
    mI2C_MasterSendReStart(_addr, 1, 10);
    uint8 x;
    mI2C_MasterReadByte(CY_SCB_I2C_ACK, &x, 10);
    if (x & 0x0A ) {
    } else {
        return false;
    }
    _initialized = true;

    // Set default integration time and gain
    TSL2561_setTiming(_integration);
    TSL2561_setGain(_gain);
    // Note: by default, the device is in power down mode on bootup
    TSL2561_disable();

    return true;
}

void TSL2561_enable(void)
{
  if (!_initialized) TSL2561_begin();

  // Enable the device by setting the control bit to 0x03
  TSL2561_write8(TSL2561_COMMAND_BIT | TSL2561_REGISTER_CONTROL, TSL2561_CONTROL_POWERON);
}

void TSL2561_disable(void)
{
  if (!_initialized) TSL2561_begin();

  // Disable the device by setting the control bit to 0x03
  TSL2561_write8(TSL2561_COMMAND_BIT | TSL2561_REGISTER_CONTROL, TSL2561_CONTROL_POWEROFF);
}


void TSL2561_setGain(tsl2561Gain_t gain) {
  if (!_initialized) TSL2561_begin();

  TSL2561_enable();
  _gain = gain;
  TSL2561_write8(TSL2561_COMMAND_BIT | TSL2561_REGISTER_TIMING, _integration | _gain);  
  TSL2561_disable();
}

void TSL2561_setTiming(tsl2561IntegrationTime_t integration)
{
  if (!_initialized) TSL2561_begin();

  TSL2561_enable();
  _integration = integration;
  TSL2561_write8(TSL2561_COMMAND_BIT | TSL2561_REGISTER_TIMING, _integration | _gain);  
  TSL2561_disable();
}

uint32_t TSL2561_calculateLux(uint16_t ch0, uint16_t ch1)
{
  unsigned long chScale;
  unsigned long channel1;
  unsigned long channel0;

  switch (_integration)
  {
    case TSL2561_INTEGRATIONTIME_13MS:
      chScale = TSL2561_LUX_CHSCALE_TINT0;
      break;
    case TSL2561_INTEGRATIONTIME_101MS:
      chScale = TSL2561_LUX_CHSCALE_TINT1;
      break;
    default: // No scaling ... integration time = 402ms
      chScale = (1 << TSL2561_LUX_CHSCALE);
      break;
  }

  // Scale for gain (1x or 16x)
  if (!_gain) chScale = chScale << 4;

  // scale the channel values
  channel0 = (ch0 * chScale) >> TSL2561_LUX_CHSCALE;
  channel1 = (ch1 * chScale) >> TSL2561_LUX_CHSCALE;

  // find the ratio of the channel values (Channel1/Channel0)
  unsigned long ratio1 = 0;
  if (channel0 != 0) ratio1 = (channel1 << (TSL2561_LUX_RATIOSCALE+1)) / channel0;

  // round the ratio value
  unsigned long ratio = (ratio1 + 1) >> 1;

  unsigned int b, m;

#ifdef TSL2561_PACKAGE_CS
  if ((ratio >= 0) && (ratio <= TSL2561_LUX_K1C))
    {b=TSL2561_LUX_B1C; m=TSL2561_LUX_M1C;}
  else if (ratio <= TSL2561_LUX_K2C)
    {b=TSL2561_LUX_B2C; m=TSL2561_LUX_M2C;}
  else if (ratio <= TSL2561_LUX_K3C)
    {b=TSL2561_LUX_B3C; m=TSL2561_LUX_M3C;}
  else if (ratio <= TSL2561_LUX_K4C)
    {b=TSL2561_LUX_B4C; m=TSL2561_LUX_M4C;}
  else if (ratio <= TSL2561_LUX_K5C)
    {b=TSL2561_LUX_B5C; m=TSL2561_LUX_M5C;}
  else if (ratio <= TSL2561_LUX_K6C)
    {b=TSL2561_LUX_B6C; m=TSL2561_LUX_M6C;}
  else if (ratio <= TSL2561_LUX_K7C)
    {b=TSL2561_LUX_B7C; m=TSL2561_LUX_M7C;}
  else if (ratio > TSL2561_LUX_K8C)
    {b=TSL2561_LUX_B8C; m=TSL2561_LUX_M8C;}
#else
  if ((ratio >= 0) && (ratio <= TSL2561_LUX_K1T))
    {b=TSL2561_LUX_B1T; m=TSL2561_LUX_M1T;}
  else if (ratio <= TSL2561_LUX_K2T)
    {b=TSL2561_LUX_B2T; m=TSL2561_LUX_M2T;}
  else if (ratio <= TSL2561_LUX_K3T)
    {b=TSL2561_LUX_B3T; m=TSL2561_LUX_M3T;}
  else if (ratio <= TSL2561_LUX_K4T)
    {b=TSL2561_LUX_B4T; m=TSL2561_LUX_M4T;}
  else if (ratio <= TSL2561_LUX_K5T)
    {b=TSL2561_LUX_B5T; m=TSL2561_LUX_M5T;}
  else if (ratio <= TSL2561_LUX_K6T)
    {b=TSL2561_LUX_B6T; m=TSL2561_LUX_M6T;}
  else if (ratio <= TSL2561_LUX_K7T)
    {b=TSL2561_LUX_B7T; m=TSL2561_LUX_M7T;}
  else if (ratio > TSL2561_LUX_K8T)
    {b=TSL2561_LUX_B8T; m=TSL2561_LUX_M8T;}
#endif

  unsigned long temp;
  temp = ((channel0 * b) - (channel1 * m));

  // do not allow negative lux value
  if (temp < 0) temp = 0;

  // round lsb (2^(LUX_SCALE-1))
  temp += (1 << (TSL2561_LUX_LUXSCALE-1));

  // strip off fractional portion
  uint32_t lux = temp >> TSL2561_LUX_LUXSCALE;

  // Signal I2C had no errors
  return lux;
}

uint32_t TSL2561_getFullLuminosity (void)
{
  if (!_initialized) TSL2561_begin();

  // Enable the device by setting the control bit to 0x03
  TSL2561_enable();

  // Wait x ms for ADC to complete
  switch (_integration)
  {
    case TSL2561_INTEGRATIONTIME_13MS:
      delay(14);
      break;
    case TSL2561_INTEGRATIONTIME_101MS:
      delay(102);
      break;
    default:
      delay(403);
      break;
  }

  uint32_t x;
  x = TSL2561_read16(TSL2561_COMMAND_BIT | TSL2561_WORD_BIT | TSL2561_REGISTER_CHAN1_LOW);
  x <<= 16;
  x |= TSL2561_read16(TSL2561_COMMAND_BIT | TSL2561_WORD_BIT | TSL2561_REGISTER_CHAN0_LOW);

  TSL2561_disable();

  return x;
}
uint16_t TSL2561_getLuminosity (uint8_t channel) {

  uint32_t x = TSL2561_getFullLuminosity();

  if (channel == 0) {
    // Reads two byte value from channel 0 (visible + infrared)
    return (x & 0xFFFF);
  } else if (channel == 1) {
    // Reads two byte value from channel 1 (infrared)
    return (x >> 16);
  } else if (channel == 2) {
    // Reads all and subtracts out just the visible!
    return ( (x & 0xFFFF) - (x >> 16));
  }
  
  // unknown channel!
  return 0;
}

uint16_t TSL2561_read16(uint8_t reg) {
    uint8 x, t;
    uint16_t answer;

    // low level
    mI2C_MasterSendStart(_addr, 0, 10);
    mI2C_MasterWriteByte(reg, 10);
    mI2C_MasterSendReStart(_addr, 1, 10);
    mI2C_MasterReadByte(CY_SCB_I2C_ACK, &t, 10);
    mI2C_MasterReadByte(CY_SCB_I2C_NAK, &x, 10);
    mI2C_MasterSendStop(0);
    
    // high level
    /*cy_stc_scb_i2c_master_xfer_config_t *i2c;
    i2c->buffer = &reg;
    i2c->bufferSize = 1;
    i2c->slaveAddress = _addr;
    i2c->xferPending = true;
    mI2C_MasterWrite(i2c);
    while(0 != (mI2C_MasterGetStatus() & CY_SCB_I2C_MASTER_BUSY));
    uint8 rxBuffer[2];
    i2c->buffer = rxBuffer;
    i2c->bufferSize = 2;
    i2c->xferPending = false;
    mI2C_MasterRead(i2c);
    while(0 != (mI2C_MasterGetStatus() & CY_SCB_I2C_MASTER_BUSY));*/
    
    answer = x << 8;
    answer |= t;
    return answer;
}

void TSL2561_write8 (uint8_t reg, uint8_t value) {
    mI2C_MasterSendStart(_addr, 0, 10);
    mI2C_MasterWriteByte(reg, 10);
    mI2C_MasterWriteByte(value, 10);
    mI2C_MasterSendStop(0);
}
