/*
 * SCard.cpp
 *  Created on: 09.07.2017
 */



extern "C" {
  #include "twi.h"
}

//#include "Wire.h"

#ifndef __WPROGRAM_H__
	#include "WaspClasses.h"
#endif


SCard::SCard(){

	twi_init();
 	twi_setAddress(I2C_ADDRESS);
 	delayMicroseconds(4); 
}

void SCard::SCBegin(void){
	twi_init();
 	twi_setAddress(I2C_ADDRESS);
 	delayMicroseconds(4); 
}

static int8_t SCard::transfer(uint8_t command, const uint8_t *requestData, uint16_t requestSize,
             const uint8_t **responseData, uint16_t *responseSize){

static uint8_t *buffer;
if (responseData != 0) {
    *responseData = 0;
  }
  if (responseSize != 0) {
    *responseSize = 0;
  }
  digitalWrite(DIGITAL7, LOW); //wake up pin
  delayMicroseconds(100);
  digitalWrite(DIGITAL7, HIGH); // wake up pin
  int counterx=0;
  while (digitalRead(DIGITAL6) == 1){
    //time out setting
    if(counterx > 1250000) break;
    counterx++;
  }
  for (uint16_t i = 0; i < requestSize; i++) {
    buffer[4 + i] = requestData[i];
  }
  buffer[0] = ACK_FRAME_BYTE;
  buffer[1] = (requestSize >> 8) & 0xFF;
  buffer[2] = requestSize & 0xFF;
  buffer[3] = command;
  buffer[4 + requestSize] = generateLRC(buffer, 4 + requestSize);  
  //Wire.beginTransmission(0x50);
  bool success =  twi_writeTo(I2C_ADDRESS, buffer, 4 + requestSize + 1, 1);
  if (!success) {
    USB.print("...SCARD not success...");
    return -1;
  }
  uint16_t answerSize = (buffer[1] << 8) | buffer[2];
  uint8_t lrc = buffer[4 + answerSize];
  uint8_t acknowledge = buffer[0];
  if (answerSize > 0) {
    if (responseData != 0) {
      *responseData = &buffer[4];
    }
    if (responseSize != 0) {
      *responseSize = answerSize;
    }
  }
  if (acknowledge != ACK_FRAME_BYTE) { // error
    USB.print("...ACK not success...");
    return -2;
  }
  USB.print("***SCARD success***");
  return 1;

  
}


static uint8_t SCard::generateLRC(const uint8_t *data, uint16_t size)
{
  if (size == 0) {
    return 0;
  }

  uint8_t lrc = data[0];
  for (uint16_t i = 1; i < size; i++) {
    lrc ^= data[i];
  }
  return lrc;
}      


static void SCard::leaveShutdownMode(void)
{
  digitalWrite(DIGITAL8, HIGH); //leave shutdown
  delay(100);
  // Waiting for power on (min=11ms, typical=50ms, max=100ms)
  
}


static void SCard::enterShutdownMode(void)
{
  uint8_t i;
  //P0IEN &= ~(1 << SLAVE_I2C_MUTE_PIN);
  digitalWrite(DIGITAL8, LOW); //leave shutdown
  //delay(100);
  Utils.blinkLEDs(100);
  //HAL_IO_SET(RESET_PORT, RESET_PIN, 1);
  //Utils_DelayMs(10);
  //HAL_IO_SET(RESET_PORT, RESET_PIN, 0);
  // Waiting for power on (min=11ms, typical=50ms, max=100ms)
  delay(100);
}

