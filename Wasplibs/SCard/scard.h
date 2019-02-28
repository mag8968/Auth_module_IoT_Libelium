/*
 * SCard.h
 *  Created on: 09.07.2017
 */

#ifndef SCard_h
#define SCard_h


#define I2C_ADDRESS 0x50
#define ACK_FRAME_BYTE    0x60
#define NACK_FRAME_BYTE   0xE0
#define BUFFER_SIZE_MAX   100


class SmartCard
{

	private:
  	bool _5V_ON;
	bool _3V3_ON;


	public:
    SmartCard();
    void SCBegin(void)
    static uint8_t generateLRC(const uint8_t *data, uint16_t size);
	static int8_t transfer(uint8_t command, const uint8_t *requestData, uint16_t requestSize,
             const uint8_t **responseData, uint16_t *responseSize);
	static void leaveShutdownMode(void);
	static void enterShutdownMode(void);




}

extern SCard SC;

#endif	