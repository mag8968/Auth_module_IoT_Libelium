/*
 * SmartCard.h
 *  Created on: 14.07.2017
 */
#ifndef SmartCard_h
#define SmartCard_h

/******************************************************************************
* Includes
******************************************************************************/

#include <inttypes.h>
#include "HAL_SmartCard.h"


/******************************************************************************
* Definitions & Declarations
******************************************************************************/

#define SCARD_OUT_OF_MEMORY      (-20)
#define SCARD_CERTIFICATE_ERROR  (-21)
#define SCARD_BUFFER_OVERFLOW    (-22)

typedef struct {
  const uint8_t *data;
  uint8_t length;
  const char *name;
} ATR;

extern const ATR SUPPORTED_ATR_ARRAY[];
extern const uint16_t SUPPORTED_ATR_COUNT;

/******************************************************************************
* Class
******************************************************************************/

//! myLibrary Class
/*!
defines all the variables and functions used 
*/
class SmartCard
{

/// private methods //////////////////////////
private:


/// public methods and attributes ////////////
public:

SmartCard(); 
void ON(); 
int8_t SCard_GetCardInfo(uint8_t *pCardATRLength, uint16_t *pCardATRNumber);

int8_t SCard_ReadCertificatePersonName(uint8_t nameBufLength, char *nameBuf, uint8_t *nameResultLength,
                                     uint8_t *cardATRLength, uint16_t *cardATRNumber);

int8_t SCard_ReadCertificate(uint8_t **certificateDataPointer, uint16_t *certificateDataSize,
                           const uint8_t *certPath, uint8_t certPathSize, bool aesPadding);




};

extern SmartCard SCard;

#endif