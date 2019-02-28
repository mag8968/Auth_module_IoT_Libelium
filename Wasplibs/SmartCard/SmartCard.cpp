/*
 * SmartCard.cpp
 *  Created on: 14.07.2017
 */

/******************************************************************************
* Includes
******************************************************************************/

#ifndef __WPROGRAM_H__
#include <WaspClasses.h>
#endif

#include "SmartCard.h"



/******************************************************************************
* User API
******************************************************************************/


/******************************************************************************
* PRIVATE FUNCTIONS *
******************************************************************************/

static const uint8_t selectPKCS15Directory[] = {
  // CLA INS P1 P2
  0x00, 0xA4, 0x08, 0x00, // INS - SELECT, P1 - Select from MF
  // Lc
  0x02,
  // Data = path without the identifier of the MF
  0x50, 0x15, // Path - 5015
  // Le
  0x00
};

static const uint8_t selectCertificate[] = {
  // CLA INS P1 P2
  0x00, 0xA4, 0x02, 0x00, // INS - SELECT, P1 - Select EF under current DF
  // Lc
  0x02,
  // Data = EF identifier
  0x45, 0x43, // FID - 4543
  // Le
  0x00
};

static const uint8_t readCertificatePart1[] = {
  // CLA INS P1 P2
  0x00, 0xB0, 0x00, 0x00, // INS - READ BINARY
  // Le
  0xFE
};

// None
static const uint8_t ATR_00[] = {0x00};

// Siemens CardOS/M 3.0 (SLE66CX160S)
static const uint8_t ATR_01[] = {0x3B, 0xE9, 0x00, 0xFF, 0xC1, 0x10, 0x31, 0xFE, 0x55, 0xC8, 0x01, 0x20, 0x50, 0x4E, 0x34, 0x30, 0x31, 0x32, 0xAD};

// Siemens CardOS M4.0 (Aladdin eToken PRO (USB token))
static const uint8_t ATR_02[] = {0x3B, 0xE2, 0x00, 0xFF, 0xC1, 0x10, 0x31, 0xFE, 0x55, 0xC8, 0x02, 0x9C};

// Siemens CardOS M4.01 (SLE66CX320P)
static const uint8_t ATR_03[] = {0x3B, 0xF2, 0x98, 0x00, 0xFF, 0xC1, 0x10, 0x31, 0xFE, 0x55, 0xC8, 0x03, 0x15};

// Siemens CardOS M4.01a (SLE66CX322P)
static const uint8_t ATR_04[] = {0x3B, 0xF2, 0x98, 0x00, 0xFF, 0xC1, 0x10, 0x31, 0xFE, 0x55, 0xC8, 0x04, 0x12};

// Siemens CardOS M4.2 (SLE66CX642P)
static const uint8_t ATR_05[] = {0x3B, 0xF2, 0x18, 0x00, 0xFF, 0xC1, 0x0A, 0x31, 0xFE, 0x55, 0xC8, 0x06, 0x8A};

// Siemens CardOS V4.2B
static const uint8_t ATR_06[] = {0x3B, 0xF2, 0x18, 0x00, 0x02, 0xC1, 0x0A, 0x31, 0xFE, 0x58, 0xC8, 0x09, 0x75};

// Siemens CardOS V4.2C (SLE66CX360PE dual interface)
static const uint8_t ATR_07[] = {0x3B, 0xF2, 0x18, 0x00, 0x02, 0xC1, 0x0A, 0x31, 0xFE, 0x58, 0xC8, 0x0B, 0x77};

// Siemens CardOS V4.3
static const uint8_t ATR_08[] = {0x3B, 0xF2, 0x18, 0x00, 0x02, 0xC1, 0x0A, 0x31, 0xFE, 0x55, 0xC8, 0x07, 0x76};

// Siemens Card CardOS M4.3B
static const uint8_t ATR_09[] = {0x3B, 0xF2, 0x18, 0x00,   0x02, 0xC1, 0x0A, 0x31,   0xFE, 0x58, 0xC8, 0x08, 0x74};

// Siemens Card CardOS M4.4
static const uint8_t ATR_10[] = {0x3B, 0xD2, 0x18, 0x02,   0xC1, 0x0A, 0x31, 0xFE,   0x58, 0xC8, 0x0D, 0x51};

// Sparkasse
static const uint8_t ATR_11[] = {0x3b, 0xff, 0x18, 0x00,   0xff, 0x81, 0x31, 0xfe,   0x45, 0x65, 0x63, 0x11,
                               0x08, 0x50, 0x02, 0x50,   0x00, 0x10, 0x34, 0x70,   0x04, 0x17, 0x06, 0x30, 0x7f};

// Axalto - Cyberflex 64K (Gemalto TOP IM FIPS CY2 (product code HWP115291A)
static const uint8_t ATR_12[] = {0x3B, 0x95, 0x95, 0x40, 0xFF, 0xAE, 0x01, 0x03, 0x00, 0x00};

// Starcos 3.0
static const uint8_t ATR_13[] = {0x3B, 0xBB, 0x18, 0x00, 0xC0, 0x10, 0x31, 0xFE,
                               0x45, 0x80, 0x67, 0x04, 0x12, 0xB0, 0x03, 0x03,
                               0x00, 0x00, 0x81, 0x05, 0x3C};

// Starcos 3.2
static const uint8_t ATR_14[] = {0x3B, 0x9F, 0x96, 0x81, 0xB1, 0xFE, 0x45, 0x1F,
                               0x07, 0x00, 0x64, 0x05, 0x1E, 0xB2, 0x00, 0x31,
                               0xB0, 0x73, 0x96, 0x21, 0xDB, 0x05, 0x90, 0x00};

const ATR SUPPORTED_ATR_ARRAY[] = {
                     // 1234567890123456
  {ATR_00, sizeof(ATR_00), ""                },
  {ATR_01, sizeof(ATR_01), "S. CardOS M3.0"  },
  {ATR_02, sizeof(ATR_02), "S. CardOS M4.0"  },
  {ATR_03, sizeof(ATR_03), "S. CardOS M4.01" },
  {ATR_04, sizeof(ATR_04), "S. CardOS M4.01A"},
  {ATR_05, sizeof(ATR_05), "S. CardOS M4.2"  },
  {ATR_06, sizeof(ATR_06), "S. CardOS M4.2B" },
  {ATR_07, sizeof(ATR_07), "S. CardOS M4.2C" },
  {ATR_08, sizeof(ATR_08), "S. CardOS M4.3"  },
  {ATR_09, sizeof(ATR_09), "S. CardOS M4.3B" },
  {ATR_10, sizeof(ATR_10), "S. CardOS M4.4"  },
  {ATR_11, sizeof(ATR_11), "Sparkasse"       },
  {ATR_12, sizeof(ATR_12), "A. Cyberflex 64K"},
  {ATR_13, sizeof(ATR_13), "Starcos 3.0"     },
  {ATR_14, sizeof(ATR_14), "Starcos 3.2"     },
};
const uint16_t SUPPORTED_ATR_COUNT = sizeof(SUPPORTED_ATR_ARRAY) / sizeof(SUPPORTED_ATR_ARRAY[0]);

/******************************************************************************
* PUBLIC FUNCTIONS
******************************************************************************/
SmartCard::SmartCard(){
}

void SmartCard::ON(){

}

int8_t SmartCard::SCard_GetCardInfo(uint8_t *pCardATRLength, uint16_t *pCardATRNumber)
{
  const uint8_t *cardAtr = 0;
  uint8_t cardATRLength = 0;
  uint16_t cardATRNumber = 0;

  int8_t status = HAL_SCard_Reset(&cardAtr, &cardATRLength);


  if (status != SCARD_SUCCESS) {
    HAL_SCard_Shutdown();
    return status;
  }

  // ATR info
  for (uint16_t i = 1; i < SUPPORTED_ATR_COUNT; i++) {
    const ATR *supportedAtr = &SUPPORTED_ATR_ARRAY[i];
    if (supportedAtr->length == cardATRLength) {
   
    }
  }

  if (pCardATRLength != 0) {
    *pCardATRLength = cardATRLength;
  }
  if (pCardATRNumber != 0) {
    *pCardATRNumber = cardATRNumber;
  }

  HAL_SCard_Shutdown();

  return SCARD_SUCCESS;
}

int8_t SmartCard::SCard_ReadCertificatePersonName(uint8_t nameBufLength, char *nameBuf, uint8_t *nameResultLength,
                                     uint8_t *cardATRLength, uint16_t *cardATRNumber)
{
  
#define CHECK_STATUS                                            
  if (status != SCARD_SUCCESS) {                                 
    HAL_SCard_Shutdown();                                         
    return status;                                                 
  }

#define TRANCEIVE(requestApdu)                                        
  status = HAL_SCard_TransmitApdu(requestApdu, sizeof(requestApdu),    
      &responseApdu, &responseApduSize);                                \
  CHECK_STATUS

  int8_t status;
  const uint8_t *responseApdu = 0;
  uint16_t responseApduSize = 0;
  uint16_t userNameOffset = 194;
  bool unknownCard = true;

  uint16_t charIndex;
  uint16_t userNameLength;
  const uint8_t *atr;
  uint8_t atrSize;

  status = HAL_SCard_Reset(&atr, &atrSize);
  CHECK_STATUS

  // ATR info
  if (cardATRLength != 0) {
    *cardATRLength = atrSize;
  }
  if (cardATRNumber != 0) {
    *cardATRNumber = 0;
    for (uint16_t i = 0; i < SUPPORTED_ATR_COUNT; i++) {
      const ATR *supportedAtr = &SUPPORTED_ATR_ARRAY[i];
      if (supportedAtr->length == atrSize) {
       
      }
    }
  }

  if (unknownCard) {
    static const char HEX_CHAR_ARRAY[] = {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
      'A', 'B', 'C', 'D', 'E', 'F'
    };
    uint8_t atrCharCount = (uint8_t) (2 * responseApduSize);
    uint8_t n = (nameBufLength < atrCharCount ? nameBufLength : atrCharCount);
    for (uint8_t i = 0; i < n; i++) {
      uint8_t b = responseApdu[i / 2];
      if ((i & 0x01) == 0x00) {
        nameBuf[i] = HEX_CHAR_ARRAY[0x0F & (b >> 4)];
      } else {
        nameBuf[i] = HEX_CHAR_ARRAY[0x0F & b];
      }
    }
    if (nameResultLength) {
      *nameResultLength = n;
    }
  }

  if (!unknownCard) {
    TRANCEIVE(selectPKCS15Directory);

    TRANCEIVE(selectCertificate);

    TRANCEIVE(readCertificatePart1);

    userNameLength = 0;
    for (uint16_t i = userNameOffset; i < responseApduSize ; i++) {
      if (responseApdu[i] == 32) {
        userNameLength = i - userNameOffset;
        break;
      }
    }
    for (uint16_t i = 0; i < nameBufLength; i++) {
      if (i < userNameLength) {
        charIndex = i + userNameOffset;
        nameBuf[i] = responseApdu[charIndex];
      } else {
        nameBuf[i] = 0;
      }
    }
    if (nameResultLength) {
      *nameResultLength = userNameLength;
    }
  }

  HAL_SCard_Shutdown();

  return SCARD_SUCCESS;

#undef CHECK_RESULT
#undef TRANCEIVE

}

int8_t SmartCard::SCard_ReadCertificate(uint8_t **certificateDataPointer, uint16_t *certificateDataSize,
                           const uint8_t *certPath, uint8_t certPathSize, bool aesPadding)
{
  int8_t status;
  const uint8_t *responseApdu = 0;
  uint16_t responseApduSize = 0;
  uint8_t *dataPointer = 0;
  uint16_t dataSize = 0;
  uint8_t selectApdu[5 + 12] = {0x00, 0xA4, 0x08, 0x0C, 0x00};

  if (certPathSize > 12) {
    return SCARD_BUFFER_OVERFLOW;
  }

  selectApdu[4] = certPathSize;
  for (uint8_t i = 0; i < certPathSize; i++) {
    selectApdu[5 + i] = certPath[i];
  }

  status = HAL_SCard_Reset(0, 0);
  if (status != SCARD_SUCCESS) {
    HAL_SCard_Shutdown();
    return status;
  }
  osal_run_system_tasks(); // do system work

  status = HAL_SCard_TransmitApdu(selectApdu, 5 + certPathSize,
                                  &responseApdu, &responseApduSize);
  if (status != SCARD_SUCCESS) {
    HAL_SCard_Shutdown();
    return status;
  }
  osal_run_system_tasks(); // do system work

  if ((responseApduSize < 2) || (responseApdu[0] != 0x90) || (responseApdu[1] != 0x00)) {
    return responseApdu[0];
  }

  uint16_t offset = 0;
  uint8_t length = 0xFE;
  uint16_t certSize = 0;

  while (true) {
    const uint8_t readCertificatePart[] = {
      // CLA INS P1 P2
      0x00, 0xB0, ((offset >> 8) & 0xFF), (offset & 0xFF), // INS - READ BINARY
      // Le
      length
    };

    status = HAL_SCard_TransmitApdu(readCertificatePart, sizeof(readCertificatePart),
                                    &responseApdu, &responseApduSize);
    if ((responseApduSize != length + 2) || ((offset == 0)
         && ((responseApdu[0] != 0x30) || (responseApdu[1] == 0x80)))) {
      status = SCARD_CERTIFICATE_ERROR;
    }
    if (status != SCARD_SUCCESS) {
      if (dataPointer != 0) {
        osal_mem_free(dataPointer);
        dataPointer = 0;
      }
      HAL_SCard_Shutdown();
      return status;
    }
    osal_run_system_tasks(); // do system work

    if (offset == 0) {
      if (responseApdu[1] < 0x80) {
        certSize = responseApdu[1]; // Size of payload
      } else if (responseApdu[1] > 0x80) {
          uint8_t descSize = responseApdu[1] & 0x7F;
          for (uint8_t i = 0; i < descSize; i++) {
            certSize = (certSize << 8) + responseApdu[i + 2]; // Size of payload, most significant byte first
          }

          certSize += descSize; // ... plus the length of the size descriptor ...
          certSize += 2; // ...and two for the sequence byte (the 0x30) and the first byte of the length themselves.
      }

      if (aesPadding) {
        dataSize = AESCipher_GetEncryptSize(certSize);
      } else {
        dataSize = certSize;
      }

      dataPointer = osal_mem_alloc(dataSize);
      if (dataPointer == 0) { // out of memory
        HAL_SCard_Shutdown();
        return SCARD_OUT_OF_MEMORY;
      }
    }

    osal_memcpy(dataPointer + offset, responseApdu, length);
    offset += length;
    if (length > certSize - offset) {
      length = certSize - offset;
    }

    if (offset >= certSize) {
      break;
    }
  }

  HAL_SCard_Shutdown();

  if (aesPadding && (dataSize >= certSize)) {
    uint8_t padding = (uint8_t) (dataSize - certSize);
    for (uint16_t pos = certSize; pos < dataSize; pos++) {
      dataPointer[pos] = padding;
    }
  }

  if (certificateDataPointer != 0) {
    *certificateDataPointer = dataPointer;
  }
  if (certificateDataSize != 0) {
    *certificateDataSize = dataSize;
  }

  return SCARD_SUCCESS;
  
}



/// Preinstantiate Objects /////////////////////////////////////////////////////
SmartCard SCard = SmartCard();