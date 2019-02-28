/*
 * HAL_SmartCard.c
 *  Created on: 07.07.2017
 */

#ifndef __WPROGRAM_H__
#include <WaspClasses.h>
#endif

#include "HAL_SmartCard.h"
#include "twi_master.h"
#include "Buffers.h"



#define I2C_ADDRESS       0x50

#define ACK_FRAME_BYTE    0x60
#define NACK_FRAME_BYTE   0xE0

#define BUFFER_SIZE_MAX BUFFER2_SIZE_MAX
#define DATA_SIZE_MAX (BUFFER_SIZE_MAX - 4 - 1)
#define ATR_SIZE_MAX 32

#define WAKE_UP_TIMEOUT_MS 10
#define RESPONSE_TIMEOUT_MS 125000

#define SCARD_STATE_IDLE 0
#define SCARD_STATE_START_TRANSMIT_WAIT 1
#define SCARD_STATE_RESPONSE_WAIT 2

#define SCARD_EVT_CHECK_STATE 1
#define SCARD_EVT_TIMER 2

static uint8_t *buffer = buffer2;
static uint8_t atrData[ATR_SIZE_MAX];

static uint8_t atrDataSize = 0;

static uint8_t bmFindexDindex = 0x18; // Fi=372, fmax.=5MHz; Di=12
static int8_t tprotocol = -1; // 0 - T0, 1 - T1, -1 - not set
static const uint8_t bIFSC = 0xFE; // 254 bytes
static uint8_t guardTime = 0xd;
static uint8_t workWaitingTime = 10;
static uint8_t generateLRC(const uint8_t *data, uint16_t size);
static bool verifyLRC(const uint8_t *data, uint16_t size, uint8_t lrc);
static void leaveShutdownMode(void);
static void enterShutdownMode(void);
static int8_t transfer(uint8_t command, const uint8_t *requestData, uint16_t requestSize,
             const uint8_t **responseData, uint16_t *responseSize);
static int8_t transfer_async(uint8_t command,  const uint8_t *requestData, uint16_t requestSize,
               SCardAsyncReturnFunc return_func);
static int8_t scardState = SCARD_STATE_IDLE;
static uint8_t scardTaskId = 0;


static uint16_t async_requestSize;
static SCardAsyncReturnFunc async_return_func;
static void async_handle_check_state(void);

void HAL_SCard_Task_Init(uint8_t taskId) {
  scardTaskId = taskId;
}

uint16_t HAL_SCard_ProcessEvent(uint8_t taskId, uint16_t events) {
  P0IEN |= (1 << SLAVE_I2C_MUTE_PIN);
  PICTL |= 1;
  IEN1 |= (1<<5);

  if (events & SCARD_EVT_CHECK_STATE) {
    async_handle_check_state();
    return events ^ SCARD_EVT_CHECK_STATE;
  }
  if (events & SCARD_EVT_TIMER) {
    return events ^ SCARD_EVT_TIMER;
  }
  return 0;
}

void HAL_SCard_handle_irq(void)
{
  osal_start_timerEx(scardTaskId, SCARD_EVT_CHECK_STATE, 0);
}

static void async_handle_check_state(void)
{
  bool success;

  if (HAL_IO_GET(SLAVE_I2C_MUTE_PORT, SLAVE_I2C_MUTE_PIN) == 0) {
    switch (scardState) {
    case SCARD_STATE_START_TRANSMIT_WAIT:
      // Send request
      success = twi_master_transfer_alpar(I2C_ADDRESS, buffer, 4 + async_requestSize + 1, TWI_ISSUE_STOP);
      if (!success) {
        scardState = SCARD_STATE_IDLE;
        if (async_return_func != NULL)
          async_return_func(SCARD_SEND_FAILURE, NULL, 0);
      }
      scardState = SCARD_STATE_RESPONSE_WAIT;
      break;
    case SCARD_STATE_RESPONSE_WAIT:
      scardState = SCARD_STATE_IDLE;
      // Receive response
      success = twi_master_transfer_alpar(I2C_ADDRESS | TWI_READ_BIT, buffer, BUFFER_SIZE_MAX, TWI_ISSUE_STOP);
      if (!success) {
        if (async_return_func != NULL) {
          async_return_func(SCARD_RECEIVE_FAILURE, NULL, 0);
        }
        break;
      }

      uint16_t answerSize = (buffer[1] << 8) | buffer[2];


      uint8_t lrc = buffer[4 + answerSize];
      if (!verifyLRC(buffer, 4 + answerSize, lrc)) {
        if (async_return_func != NULL) {
          async_return_func(SCARD_LRC_ERROR, NULL, 0);
        }
        break;
      }

      uint8_t acknowledge = buffer[0];


      if (answerSize > 0) {
        if (async_return_func != NULL)
          async_return_func(acknowledge == ACK_FRAME_BYTE ? SCARD_SUCCESS : SCARD_ERROR_CODE,
                    buffer + 4, answerSize);
      }
      break;
    }
  }
}

bool HAL_SCard_Init(void)
{
  if (!twi_master_init()) {
    return false;
  }

  HAL_CONFIG_IO_INPUT(SLAVE_I2C_MUTE_PORT, SLAVE_I2C_MUTE_PIN);
  HAL_CONFIG_IO_OUTPUT(WAKE_UP_I2C_PORT, WAKE_UP_I2C_PIN, 1);
  HAL_CONFIG_IO_OUTPUT(SHUTDOWN_PORT, SHUTDOWN_PIN, 0);
  HAL_CONFIG_IO_OUTPUT(RESET_PORT, RESET_PIN, 1);
  P0IEN |= (1 << SLAVE_I2C_MUTE_PIN);
  PICTL |= 1;
  IEN1 |= (1<<5);
  // Reset card reader
  Utils_DelayMs(10);
  HAL_IO_SET(RESET_PORT, RESET_PIN, 0);

  // Waiting for power on (min=11ms, typical=50ms, max=100ms)
  Utils_DelayMs(100);

  return true;
}

static void keygenTest()
{
  const uint8_t apdu1[] = {0x00,0xa4,0x08,0x0c,0x02,0x50,0x15};
  const uint8_t apdu2[] = {0x00,0x20,0x00,0x81,0x08,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38};
  const uint8_t apdu3[] = {0x00,0x46,0x00,0x00,0x08,0x20,0x12,0x41,0x93,0x00,0x10,0x00,0x20};
  const uint8_t *response;
  uint16_t responsesize;
  uint8_t i,r;
  r = HAL_SCard_TransmitApdu(apdu1, sizeof(apdu1), &response, &responsesize);
  
  if ((r!= 1) || (responsesize != 2) || (response[0] != 0x90))
    return;
  r = HAL_SCard_TransmitApdu(apdu2, sizeof(apdu2), &response, &responsesize);
  

  if ((r!= 1) || (responsesize != 2) || (response[0] != 0x90))
    return;

  for(i = 0; i != 255; i++) {
    uint32 startTimeMs = osal_GetSystemClock();
    r = HAL_SCard_TransmitApdu(apdu3, sizeof(apdu3), &response, &responsesize);
    if ((r!= 1) || (responsesize != 2) || (response[0] != 0x90))
      break;
    uint32 currentTimeMs = osal_GetSystemClock();
       
  }
}

int8_t HAL_SCard_Reset(const uint8_t **atr, uint8_t *atrSize)
{
  if (atr != 0) {
    *atr = 0;
  }
  if (atrSize != 0) {
    *atrSize = 0;
  }

  const uint8_t *responseApduData;
  uint16_t responseApduSize;
  int8_t status;
  bool negotiable = true;

        // Check plugged device

  leaveShutdownMode();

  // Check plugged device
  status = transfer(0x0A, 0, 0, &responseApduData, &responseApduSize);
  if (status != SCARD_SUCCESS) {
    return status;
  }

  // Check card presense
  status = transfer(0x09, 0, 0, &responseApduData, &responseApduSize);
  if (status != SCARD_SUCCESS) {
    return status;
  }
  if ((responseApduSize == 0) || (*responseApduData != 1)) { // card in?
    return SCARD_NO_CARD;
  }

  // ATR
  // Power up ISO:
  //  - attempt to activate the card at a VCC of 3V, if the cards answers correctly
  //    and if it indicates in its ATR that it is a class A or a class AB card
  //    (TAi with T=15), then the command is finished and the ATR is returned to the host;
  //  - if in the previous stage, the card did not answer correctly or did not
  //    specify in its ATR that it was a class A or a class AB card, a new activation
  //    of the card is launched at 5V. If the card does not answer to the reset,
  //    a status giving an error code is returned to the application, otherwise
  //    the answer contains all the parameters of the card.
//  const uint8_t powerUpApduData = 1;
  status = transfer(0x69, 0, 0, &responseApduData, &responseApduSize); // power up iso
  if (status != SCARD_SUCCESS) {
    return status;
  }
  if (responseApduSize > ATR_SIZE_MAX) {
    return SCARD_INTERNAL_ERROR;
  }
  osal_memcpy(atrData, responseApduData, responseApduSize);
  atrDataSize = responseApduSize;

  // PPS
  // default value if TA1 not available
  bmFindexDindex = 0x01;
  tprotocol = 0;
  guardTime = 0xd;
  workWaitingTime = 10;
  uint8_t ppsParams[] = {0x00, 0x01}; // 1. protocol 2. Fi/Di
  if (atrDataSize > 2) {
    uint8_t content = atrData[1];
    uint8_t *atrParse = atrData + 2;
    if (content & 0x10) { // if TA1 is available...
       ppsParams[1] = atrParse[0];  // set Fi/Di from atr
       bmFindexDindex = atrParse[0];
       atrParse++;
    }
    if (content & 0x20) { // if TB1 is available, skip it
       atrParse++;
    }
    if (content & 0x40) { // if TC1 is available
       guardTime = atrParse[0];
       atrParse++;  // TC1 = guard time, 255 = minimum
    }
    if (content & 0x80) { // if TD1 is available, set protocol
      tprotocol = atrParse[0] & 0xf;
      ppsParams[0] = tprotocol;
      content = atrParse[0] & 0xf0;
      atrParse++;
      if (content & 0x10) { // if TA2 is available
        negotiable = false; // availability means, pps not negotiable
        atrParse++;
      }
      if (content & 0x20) { // if TB2 is available
        atrParse++;
      }
      if (content & 0x40) {
        workWaitingTime = atrParse[0];
          atrParse++;
      }
    }
  }

  
  if (negotiable) {
    status = transfer(0x10, ppsParams, sizeof(ppsParams), &responseApduData, &responseApduSize); // negotiate
    if (status != SCARD_SUCCESS) {
      if ((responseApduSize > 0) && (responseApduData[0]  != 0x33)) {
        return status;
      }
    }
  }

 
  if (tprotocol == 1) {
    status = transfer(0x0C, &bIFSC, sizeof(bIFSC), 0, 0);
    if (status != SCARD_SUCCESS) {
      return status;
    }
  }

  if (atr != 0) {
    *atr = atrData;
  }
  if (atrSize != 0) {
    *atrSize = atrDataSize;
  }
#ifdef LONG_APDU_STRESSTEST
  keygenTest();
    Utils_DelayMs(100);
#endif
  return SCARD_SUCCESS;
}

bool HAL_SCard_GetT0ProtocolParameters(T0ProtocolParameters *params)
{
  if (tprotocol != 0) {
    return false;
  }

  if (params == 0) {
    return false;
  }

  params->bmFindexDindex = bmFindexDindex;
  params->bmTCCKST0 = 0x00; // T=0, direct
  params->bGuardTimeT0 = guardTime;
  params->bmWaitingIntegersT0 = workWaitingTime;
  params->bClockStop = 0x00; // not allowed

  return true;
}

bool HAL_SCard_GetT1ProtocolParameters(T1ProtocolParameters *params)
{
  if (tprotocol != 1) {
    return false;
  }

  if (params == 0) {
    return false;
  }

  params->bmFindexDindex = bmFindexDindex;
  params->bmTCCKST1 = 0x10; // T=1, LRC, direct
  params->bGuardTimeT1 = guardTime;
  params->bmWaitingIntegersT1 = 0x4D; // CWI=13, BWI=4
  params->bClockStop = 0x00; // not allowed
  params->bIFSC = bIFSC;
  params->bNadValue = 0x00; // XXX do we need to get nad value?

  return true;
}

int8_t HAL_SCard_TransmitApdu(const uint8_t *requestApdu, uint16_t requestApduSize,
                const uint8_t **responseApdu, uint16_t *responseApduSize)
{
  return transfer(0x00, requestApdu, requestApduSize, responseApdu, responseApduSize);
}

int8_t HAL_SCard_TransmitApduAsync(const uint8_t *requestApdu, uint16_t requestApduSize,
                   SCardAsyncReturnFunc return_func)
{

  return transfer_async(0x00, requestApdu, requestApduSize, return_func);
}

int8_t HAL_SCard_TransmitTpdu1(const uint8_t *requestTpdu, uint16_t requestTpduSize,
                 const uint8_t **responseTpdu, uint16_t *responseTpduSize)
{
  return transfer(0x01, requestTpdu, requestTpduSize, responseTpdu, responseTpduSize);
}

void HAL_SCard_Shutdown(void)
{
  enterShutdownMode();

  tprotocol = -1;
}

static uint8_t generateLRC(const uint8_t *data, uint16_t size)
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

static bool verifyLRC(const uint8_t *data, uint16_t size, uint8_t lrc)
{
  for (uint16_t i = 0; i < size; i++) {
    lrc ^= data[i];
  }

  return (lrc == 0);
}

static void leaveShutdownMode(void)
{
  HAL_IO_SET(SHUTDOWN_PORT, SHUTDOWN_PIN, 1);

  // Waiting for power on (min=11ms, typical=50ms, max=100ms)
  for (int i = 0; i < 10; i++) {
    Utils_DelayMs(10);
    osal_run_system_tasks(); // do system work
  }
}

static void enterShutdownMode(void)
{
  uint8_t i;
  P0IEN &= ~(1 << SLAVE_I2C_MUTE_PIN);
  HAL_IO_SET(SHUTDOWN_PORT, SHUTDOWN_PIN, 0);

  HAL_IO_SET(RESET_PORT, RESET_PIN, 1);
  Utils_DelayMs(10);
  HAL_IO_SET(RESET_PORT, RESET_PIN, 0);
  for(i=0;i<10;i++) {
    Utils_DelayMs(10);
    osal_run_system_tasks();
  }
}

static int8_t transfer_async(uint8_t command,  const uint8_t *requestData, uint16_t requestSize,
               SCardAsyncReturnFunc return_func)
{


  if (requestSize > DATA_SIZE_MAX) {
    return SCARD_TOO_LONG_APDU;
  }
    if (scardState != SCARD_STATE_IDLE)
    return SCARD_BUSY;
   // Wake up smart card reader
  P0IEN |= (1 << SLAVE_I2C_MUTE_PIN);
  PICTL |= 1;
  IEN1 |= (1<<5);
  HAL_IO_SET(WAKE_UP_I2C_PORT, WAKE_UP_I2C_PIN, 0);
    Utils_DelayUs(1); // minimum duration of 0.8 us
    HAL_IO_SET(WAKE_UP_I2C_PORT, WAKE_UP_I2C_PIN, 1);
   // ! Buffers 'requestData' and 'buffer' can be the same
  for (uint16_t i = 0; i < requestSize; i++) {
    buffer[4 + i] = requestData[i];
  }
  buffer[0] = ACK_FRAME_BYTE;
  buffer[1] = (requestSize >> 8) & 0xFF;
  buffer[2] = requestSize & 0xFF;
  buffer[3] = command;
  buffer[4 + requestSize] = generateLRC(buffer, 4 + requestSize);
  async_requestSize = requestSize;
  async_return_func = return_func;
  scardState = SCARD_STATE_START_TRANSMIT_WAIT;
  return SCARD_SUCCESS;

}

static int8_t transfer(uint8_t command, const uint8_t *requestData, uint16_t requestSize,
             const uint8_t **responseData, uint16_t *responseSize)
{
  if (responseData != 0) {
    *responseData = 0;
  }
  if (responseSize != 0) {
    *responseSize = 0;
  }

  if (requestSize > DATA_SIZE_MAX) {
    return SCARD_TOO_LONG_APDU;
  }
    if (scardState != SCARD_STATE_IDLE)
    return SCARD_BUSY;
  // Wake up smart card reader
  HAL_IO_SET(WAKE_UP_I2C_PORT, WAKE_UP_I2C_PIN, 0);
  Utils_DelayUs(1); // minimum duration of 0.8 us
  HAL_IO_SET(WAKE_UP_I2C_PORT, WAKE_UP_I2C_PIN, 1);

  // Wait for wake up
  uint32 counter = 0;
  uint32 startTimeMs = osal_GetSystemClock();
  while (HAL_IO_GET(SLAVE_I2C_MUTE_PORT, SLAVE_I2C_MUTE_PIN) == 1) {
    if (osal_GetSystemClock() > startTimeMs + WAKE_UP_TIMEOUT_MS) {
      return WAKE_UP_TIMEOUT;
    }
    counter++;
    if (counter > BLUETOOTH_TIMEOUT_COUNT) {
      osal_run_system_tasks(); // do system work
    }
  }

  // ! Buffers 'requestData' and 'buffer' can be the same
  for (uint16_t i = 0; i < requestSize; i++) {
    buffer[4 + i] = requestData[i];
  }
  buffer[0] = ACK_FRAME_BYTE;
  buffer[1] = (requestSize >> 8) & 0xFF;
  buffer[2] = requestSize & 0xFF;
  buffer[3] = command;
  buffer[4 + requestSize] = generateLRC(buffer, 4 + requestSize);

  // Send request
  bool success = twi_master_transfer_alpar(I2C_ADDRESS, buffer, 4 + requestSize + 1, TWI_ISSUE_STOP);
  if (!success) {
    return SCARD_SEND_FAILURE;
  }

  // Wait for request processing
  counter = 0;
  startTimeMs = osal_GetSystemClock();
  while (HAL_IO_GET(SLAVE_I2C_MUTE_PORT, SLAVE_I2C_MUTE_PIN) == 1) {
    if (osal_GetSystemClock() > startTimeMs + RESPONSE_TIMEOUT_MS) {
      return RESPONSE_TIMEOUT;
    }
    counter++;
    if (counter > BLUETOOTH_TIMEOUT_COUNT) {
      osal_run_system_tasks(); // do system work
    }
  }

  // Receive response
  success = twi_master_transfer_alpar(I2C_ADDRESS | TWI_READ_BIT, buffer, BUFFER_SIZE_MAX, TWI_ISSUE_STOP);
  if (!success) {
    return SCARD_RECEIVE_FAILURE;
  }

  uint16_t answerSize = (buffer[1] << 8) | buffer[2];
  uint8_t lrc = buffer[4 + answerSize];
  if (!verifyLRC(buffer, 4 + answerSize, lrc)) {
    return SCARD_LRC_ERROR;
  }

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
    return SCARD_ERROR_CODE;
  }

  return SCARD_SUCCESS;
}
