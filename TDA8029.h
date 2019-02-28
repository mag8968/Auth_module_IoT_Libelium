/* /////////////////////////////////////////////////////////////////////////////////////////////////
//                     Copyright (c) NXP Semiconductors
//
//         All rights are reserved. Reproduction in whole or in part is
//        prohibited without the written consent of the copyright owner.
//      NXP reserves the right to make changes without notice at any time.
//     NXP makes no warranty, expressed, implied or statutory, including but
//   not limited to any implied warranty of merchantability or fitness for any
//  particular purpose, or that the use will not infringe any third party patent,
//     copyright or trademark. NXP must not be liable for any loss or damage
//                            arising from its use.
///////////////////////////////////////////////////////////////////////////////////////////////// */

/*! 
 *
 * Project: TDA code. 
 *
 */
#ifndef TDA8029_H
#define TDA8029_H

#define PORT_NUMBER						1



#define TDA8029_OK						0
#define TDA8029_ERROR					1
#define TDA8029_ALPARPATTERNNOK			2
#define TDA8029_ALPARPATTERNERROR		3
#define TDA8029_ALPARRESBADCMD			4
#define TDA8029_ALPARBADLENGTH			5
#define TDA8029_BUFFERTOOSMALL			6
#define TDA8029_COMMERROR				7
#define TDA8029_ALPARBADCHECKSUM		8

#define ALPAR_PATTERN_OK				0x60
#define ALPAR_PATTERN_NOK				0xE0

#define ALPAR_PATTERN_OFFSET			0
#define ALPAR_MSBLEN_OFFSET				1
#define ALPAR_LSBLEN_OFFSET				2
#define ALPAR_CMD_OFFSET				3
#define	ALPAR_DATA_OFFSET				4


#define ALPAR_CMD_MASK					0x0A
#define ALPAR_CMD_GETREADERSTATUS		0xAA
#define ALPAR_CMD_CHECKCARDPRES			0x09
#define ALPAR_CMD_POWERUP5V				0x6E
#define ALPAR_CMD_SENDAPDU				0x00

#define ALPARBUFSIZE					512

extern unsigned char TDA8029_CheckPluggedDevice(void);
extern void TDA8029_TestStep(unsigned char testNum);

#endif /* TDA_ENUMERATEDEVICES_H */
