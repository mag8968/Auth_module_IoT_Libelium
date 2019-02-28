/*
 * HAL_SmartCard.h
 *  Created on: 07.07.2017
 */

#ifndef HAL_SMART_CARD_H_
#define HAL_SMART_CARD_H_


#define SCARD_SUCCESS                 1 
#define SCARD_END_OF_STREAM           0
#define SCARD_PARITY_ERROR          (-3)
#define SCARD_MISSING_BITS_ERROR    (-4)
#define SCARD_TIMING_TOO_FAST_ERROR (-5)
#define SCARD_NO_BUFFER_PROVIDED    (-6)
#define SCARD_TOO_LONG_APDU         (-7)
#define SCARD_SEND_FAILURE          (-8)
#define SCARD_RECEIVE_FAILURE       (-9)
#define SCARD_LRC_ERROR             (-10)
#define SCARD_ERROR_CODE            (-11)
#define SCARD_NO_CARD               (-12)
#define SCARD_INTERNAL_ERROR        (-13)
#define WAKE_UP_TIMEOUT             (-14)
#define RESPONSE_TIMEOUT            (-15)
#define SCARD_BUSY					(-16)

#define SLAVE_I2C_MUTE_PORT 	0
#define SLAVE_I2C_MUTE_PIN 		5

typedef struct {
    // B7-4 - FI - Index into the table 7 in ISO/IEC 7816-3:1997 selecting a clock rate conversion factor
    // B3-0 - DI - Index into the table 8 in ISO/IEC 7816-3:1997 selecting a baud rate conversion factor
    uint8_t bmFindexDindex;
    //uint8_t bmFindexDindex;
    // For T=0 ,B0 - 0b, B7-2 - 000000b
    // B1 - Convention used (b1=0 for direct, b1=1 for inverse)
    uint8_t bmTCCKST0;
    // Extra Guardtime between two characters. Add 0 to 254 etu to the normal guardtime of 12etu. FFh is the same as 00h.
    uint8_t bGuardTimeT0;
    // WI for T=0 used to define WWT
    uint8_t bmWaitingIntegersT0;
    // ICC Clock Stop Support
    // 00 = Stopping the Clock is not allowed
    // 01 = Stop with Clock signal Low
    // 02 = Stop with Clock signal High
    // 03 = Stop with Clock either High or Low
    uint8_t bClockStop;
} T0ProtocolParameters;

typedef struct {
    // B7-4 - FI - Index into the table 7 in ISO/IEC 7816-3:1997 selecting a clock rate conversion factor
    // B3-0 - DI - Index into the table 8 in ISO/IEC 7816-3:1997 selecting a baud rate conversion factor
    uint8_t bmFindexDindex;
    // For T=1, B7-2 - 000100b
    // B0 - Checksum type (b0=0 for LRC, b0=1 for CRC)
    // B1 - Convention used (b1=0 for direct, b1=1 for inverse)
    uint8_t bmTCCKST1;
    // Extra Guardtime (0 to 254 etu between two characters). If value is FFh, then guardtime is reduced by 1.
    uint8_t bGuardTimeT1;
    // B7-4 = BWI
    // B3-0 = CWI
    uint8_t bmWaitingIntegersT1;
    // ICC Clock Stop Support
    // 00 = Stopping the Clock is not allowed
    // 01 = Stop with Clock signal Low
    // 02 = Stop with Clock signal High
    // 03 = Stop with Clock either High or Low
    uint8_t bClockStop;
    // Size of negotiated IFSC
    uint8_t bIFSC;
    // Nad value used by CCID
    uint8_t bNadValue;
} T1ProtocolParameters;

typedef void (*SCardAsyncReturnFunc)(int8_t status, const uint8_t *response, uint16_t responseSize);

bool HAL_SCard_Init(void);

int8_t HAL_SCard_Reset(const uint8_t **atr, uint8_t *atrSize);

bool HAL_SCard_GetT0ProtocolParameters(T0ProtocolParameters *params);

bool HAL_SCard_GetT1ProtocolParameters(T1ProtocolParameters *params);

int8_t HAL_SCard_TransmitApdu(const uint8_t *requestApdu, uint16_t requestApduSize,
                            const uint8_t **responseApdu, uint16_t *responseApduSize);
int8_t HAL_SCard_TransmitApduAsync(const uint8_t *requestApdu, uint16_t requestApduSize,
							  	 SCardAsyncReturnFunc return_func);

int8_t HAL_SCard_TransmitTpdu1(const uint8_t *requestTpdu, uint16_t requestTpduSize,
                             const uint8_t **responseTpdu, uint16_t *responseTpduSize);

void HAL_SCard_Shutdown(void);
void HAL_SCard_Task_Init(uint8_t taskId);
void HAL_SCard_handle_irq(void);
uint16_t HAL_SCard_ProcessEvent(uint8_t taskId, uint16_t events);
#endif /* HAL_SMART_CARD_H_ */
