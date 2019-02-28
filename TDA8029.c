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

#include "SerialComm/SerialComm.h"
#include "TDA8029.h"

#undef code 
#undef data 

#define code 
#define data 

HANDLE	TDA_Handle = NULL;

extern DWORD T_RS232_TIMEOUT_DEFAULT;


/*-------------------------------------------------------------------------------------------------
---- Fonction EmptyHSUrxFIFO
- receive char on UART until it is empty
-------------------------------------------------------------------------------------------------*/
void EmptyHSUrxFIFO (HANDLE mPort)
{
	unsigned char c;
	unsigned int i=1;

	while (SerialComm_Read (&c, &i, mPort)); // empty receive FIFO
}

/*-------------------------------------------------------------------------------------------------
---- Fonction ReceiveALPARFrame
- reception of a whole ALPAR Frame
- return the status of the reception. 0 = ERROR - 1 = OK.
-------------------------------------------------------------------------------------------------*/
BOOL ReceiveALPARFrame (BYTE *pRdBuffer, unsigned int *rlen)
{
	BOOL status = FALSE;
	int len;
	char checksum = 0x00;

	len = 4; //Get header (0x60) and length (2 bytes)
	status = SerialComm_Read (pRdBuffer, &len, TDA_Handle);
	len = ((*(pRdBuffer+1))<<8) + (*(pRdBuffer+2)) + 1;
	status &= SerialComm_Read (pRdBuffer+4, &len, TDA_Handle);
	*rlen = len + 4;
	return status;
}


/*--------------------------------------------------------------------------------------------------
---- Function to build the ALPAR frame:
     -------------------------------------------------------------------
	 | 60 | lenMSB | lenLSB | AlparCmd |    ... AlparData ...    | CRC |
	 -------------------------------------------------------------------
 ---------------------------------------------------------------------------------------------------*/
unsigned char TDA8029_BuildAlparFrame(unsigned char cmd, unsigned char *pBufferReq, unsigned int BufferReqSize, unsigned char *alparBuf, unsigned int alparBufSize)
{
	unsigned char i, crc = 0;

	unsigned char *tmpAlparCmd = pBufferReq;

	if (BufferReqSize > alparBufSize-5)
		return TDA8029_BUFFERTOOSMALL;

	*alparBuf = ALPAR_PATTERN_OK;					// 0x60
	crc ^= *(alparBuf++);
	*alparBuf = (BufferReqSize >> 8) & 0xFF;		// length MSB
	crc ^= *(alparBuf++);
	*alparBuf = BufferReqSize  & 0xFF;				// length LSB
	crc ^= *(alparBuf++);
	*alparBuf = cmd;								// Command Byte
	crc ^= *(alparBuf++);

	for (i=0; i<BufferReqSize; i++)
	{
		*alparBuf = *(pBufferReq++);					// Data
		crc ^= *(alparBuf++);
	}
	*alparBuf = crc;								// Checksum
	
	return TDA8029_OK;
}

/*-------------------------------------------------------------------------------------------------
---- Fonction TDA8029_ComputeALPARcrc
- Computes the checksum of the alpar frame.
- returns the XOR of all the frame bytes
-------------------------------------------------------------------------------------------------*/
unsigned char TDA8029_ComputeALPARcrc(unsigned char *alparBuf, unsigned int len)
{
	unsigned int i, crc = 0;
	for (i = 0; i < len; i++)
		crc ^= *(alparBuf++);
	return crc;
}

/*-------------------------------------------------------------------------------------------------
---- Fonction SendAndReceive ALPAR
---- Build the TAMA Frame, sends the Frame and receives the answer
-------------------------------------------------------------------------------------------------*/
unsigned char TDA8029_ALPAR_SendAndReceive (unsigned char cmd, unsigned char *pBufferReq, unsigned int BufferReqSize, unsigned char *receive_buffer, unsigned int *rlen)
{
	unsigned int i;
	unsigned int len = BufferReqSize + 5;
	unsigned char ALPARCmd[ALPARBUFSIZE];
	unsigned char ALPARAns[ALPARBUFSIZE];
	unsigned char frameCRC, res, status = 0;

	EmptyHSUrxFIFO(TDA_Handle); // empty receive FIFO

	res = TDA8029_BuildAlparFrame(cmd, pBufferReq, BufferReqSize, ALPARCmd, ALPARBUFSIZE);

	// Print the ALPAR frame to be sent
	printf("\n-------------------------\nALPAR Command to be sent:\n --> ");
	for (i = 0; i < BufferReqSize+5; i++)
		printf("%02X ",ALPARCmd[i]);
	printf("\n-------------------------");

	// Send the ALPAR Frame
	if(!SerialComm_Write (ALPARCmd, &len, TDA_Handle))
		return TDA8029_COMMERROR;

	// Receive the answer
	if (!ReceiveALPARFrame(ALPARAns, &len))
		return TDA8029_COMMERROR;

	// Print the received ALPAR Frame
	printf("\n-------------------------\nALPAR Frame received:\n <-- ");
	for (i = 0; i < len; i++)
		printf("%02X ",ALPARAns[i]);
	printf("\n-------------------------\n");

	// Check the received ALPAR Frame (Pattern, length, INS byte, CRC...)
	//
	if ((len - 5) > *rlen) return TDA8029_BUFFERTOOSMALL;		// Check if the buffer is big enough for this answer

	frameCRC = TDA8029_ComputeALPARcrc(ALPARAns, len);

	if (ALPARAns[ALPAR_PATTERN_OFFSET] != ALPAR_PATTERN_OK)		// Check if answer starts with 0x60 (means OK)
	{
		if ((ALPARAns[ALPAR_PATTERN_OFFSET] == ALPAR_PATTERN_NOK) &&
			(ALPARAns[ALPAR_MSBLEN_OFFSET] == 0x00) &&
			(ALPARAns[ALPAR_LSBLEN_OFFSET] == 0x01) &&
			(ALPARAns[ALPAR_CMD_OFFSET] == cmd) &&
			(frameCRC == 0))				// Check if the frame is in a correct format starting with E0 (means execution Error, but ALPAR frame is correct)
		{
			*receive_buffer = ALPARAns[ALPAR_DATA_OFFSET];
			*rlen = 1;
			return TDA8029_ALPARPATTERNNOK;						// ALPAR first byte is 0xE0 and Frame is correct
		}
		return TDA8029_ALPARPATTERNERROR;						// ALPAR Pattern (first byte) is not good, or 0xE0 with incorrect frame
	}
	
	if (ALPARAns[ALPAR_CMD_OFFSET] != cmd) return TDA8029_ALPARRESBADCMD;	// The returnde CMD byte is not the same as sent
	if (((ALPARAns[ALPAR_MSBLEN_OFFSET] * 256) + ALPARAns[ALPAR_LSBLEN_OFFSET]) != (len - 5))
		return TDA8029_ALPARBADLENGTH;										// The computed length does not correspond to the received frame

	if (frameCRC != 0) return TDA8029_ALPARBADCHECKSUM;		// The checksum is not good

	// The received ALPAR Frame is correct.
	// Copy the received Data in the returned buffer
	for (i = ALPAR_DATA_OFFSET; i < len - 1; i++)
		*(receive_buffer++) = ALPARAns[i];

	// The data size is the Frame length - 5
	*rlen = len - 5;

	return TDA8029_OK;
}


/*-------------------------------------------------------------------------------------------------
---- Fonction TDA8029_CheckPluggedDevice
---- Opens the COM port and 
---- checks if the TDA8029 is plugged
-------------------------------------------------------------------------------------------------*/
unsigned char TDA8029_CheckPluggedDevice(void)
{
	unsigned char res = TDA8029_OK;
	unsigned char maskComm[64];
	unsigned int len;
	unsigned char expectedMask[] = "07 Release 1.0";

	T_RS232_TIMEOUT_DEFAULT = 50;

	// Initialize serial port
	TDA_Handle = (HANDLE)0;
	SerialComm_Open(PORT_NUMBER, &TDA_Handle);

	if(TDA_Handle != (HANDLE)0xFFFFFFFF)
	{
		// Send a Mask command to check if the good device has been connected
		len = sizeof(maskComm);
		res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_MASK, NULL, 0, maskComm, &len);
		if(res == TDA8029_OK)
		{
			if ((memcmp(expectedMask, maskComm, sizeof(expectedMask)-1)) != 0)
			{
				SerialComm_Close(TDA_Handle);
				TDA_Handle = 0;
				res = TDA8029_ERROR;
			}
		}
		else
		{
			SerialComm_Close(TDA_Handle);
			TDA_Handle = 0;
		}
	}
	else
	{
		TDA_Handle = 0;
		res = TDA8029_ERROR;
	}
	T_RS232_TIMEOUT_DEFAULT = 500;

	return res;
}

/*-------------------------------------------------------------------------------------------------
---- Fonction TDA8029_TestStep
---- Sends a command to the TDA and checks the answer
-------------------------------------------------------------------------------------------------*/
void TDA8029_TestStep(unsigned char testNum)
{
	unsigned char rBuf[260];
	unsigned char pUpData[] = {0x00};
	unsigned char sendApduData[] = {0x00, 0xA4, 0x00, 0x00, 0x02, 0x3F, 0x00};	// APDU Select file 3F 00
	unsigned char res = TDA8029_OK;
	unsigned int len, i;
	
	switch(testNum)
	{
	case '1':
		len = sizeof(rBuf);
		res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_MASK, NULL, 0, rBuf, &len);
		if (res == TDA8029_OK)
		{
			printf("\nMask version:\n");
			for (i = 0; i < len; i++)
				printf("%c",rBuf[i]);
			printf("\n--------------------\n");
		}
		break;
	case '2':
		len = sizeof(rBuf);
		res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_GETREADERSTATUS, NULL, 0, rBuf, &len);
		if (res == TDA8029_OK)
			printf("\nReader Status : %02X\n", rBuf[0]);
		break;
	case '3':
		len = sizeof(rBuf);
		res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_CHECKCARDPRES, NULL, 0, rBuf, &len);
		if (res == TDA8029_OK)
		{
			printf("\nCard is ");
			if (rBuf[0] == 0)
				printf("ABSENT!\n");
			else if (rBuf[0] == 1)
				printf("PRESENT!\n");
			printf("--------------------\n");
		}
		break;
	case '4':
		len = sizeof(rBuf);
		res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_POWERUP5V, pUpData, sizeof(pUpData), rBuf, &len);
		if (res == TDA8029_OK)
		{
			printf("\nATR:\n");
			for (i = 0; i < len; i++)
				printf("%02X ",rBuf[i]);
			printf("\n--------------------\n");
		}
		break;
	case '5':
		len = sizeof(rBuf);
		res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_SENDAPDU, sendApduData, sizeof(sendApduData), rBuf, &len);
		if (res == TDA8029_OK)
		{
			printf("\nR-APDU:\n");
			for (i = 0; i < len; i++)
				printf("%02X ",rBuf[i]);
			printf("\n--------------------\n");
		}
		break;
	default:
		res = TDA8029_ERROR;
		break;
	}

	switch (res)
	{
	case TDA8029_ERROR:
		printf("\nERROR - UNKNOWN ERROR \n");
		break;
	case TDA8029_ALPARPATTERNNOK:
		printf("\nERROR - TDA8029 Execution Error: %02X\n", rBuf[0]);
		break;
	case TDA8029_ALPARPATTERNERROR:
		printf("\nERROR - BAD ALPAR PATTERN\n");
		break;
	case TDA8029_ALPARRESBADCMD:
		printf("\nERROR - ALPAR RESPONSE : BAD COMMAND\n");
		break;
	case TDA8029_ALPARBADLENGTH:
		printf("\nERROR - ALPAR RESPONSE : BAD LENGTH\n");
		break;
	case TDA8029_BUFFERTOOSMALL:
		printf("\nERROR - BUFFER TOO SMALL\n");
		break;
	case TDA8029_COMMERROR:
		printf("\nERROR - COMMUNICATION ERROR\n");
		break;
	case TDA8029_ALPARBADCHECKSUM:
		printf("\nERROR - ALPAR BAD CHECKSUM RECEIVED\n");
		break;
	case TDA8029_OK:
	default:
		break;
	}
	printf("\n> Press ENTER"); 
	fflush(stdin);
	(char) getchar();
}



