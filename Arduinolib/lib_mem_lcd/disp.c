// disp.c

#include "bsp.h"
#include "utils.h"
#include "spi.h"
#include "tick.h"
#include "disp.h"
#include <string.h>

////////////////////////////////////////////////////////////////////////
// Display driver for Sharp LS013B7DH03 128x128 monochrome memory LCD //
////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Globals
/////////////////////////////////////////////////////////////////////////////

uint16_t LastVcomToggle = 0;

/////////////////////////////////////////////////////////////////////////////
// Static Function Prototypes
/////////////////////////////////////////////////////////////////////////////

uint8_t BitReverse(uint8_t input);
uint8_t SoftwareVcomToggle(uint8_t cmd);

/////////////////////////////////////////////////////////////////////////////
// Function
/////////////////////////////////////////////////////////////////////////////

uint8_t BitReverse(uint8_t input)
{
    uint8_t output = input;

    output = (output & 0xF0) >> 4 | (output & 0x0F) << 4;
    output = (output & 0xCC) >> 2 | (output & 0x33) << 2;
    output = (output & 0xAA) >> 1 | (output & 0x55) << 1;

    return output;
}

/***************************************************************************//**
 * @brief Toggle the software VCOM command bit at 5 Hz
 * @param cmd The display SPI command byte
 * @return The display SPI command byte with M1 toggled at 5 Hz
 *
 *****************************************************************************/
uint8_t SoftwareVcomToggle(uint8_t cmd)
{
    uint16_t tick = GetTickCount();

    if (tick - LastVcomToggle > HZ_TO_MS(DISP_VCOM_TOGGLE_RATE_HZ))
    {
        // Pulse software VCOM
        cmd |= DISP_CMD_VCOM_MASK;

        LastVcomToggle = tick;
    }

    return cmd;
}

/***************************************************************************//**
 * @brief Initializes the display driver and clears the display
 *
 *****************************************************************************/
void DISP_Init()
{
    BSP_DISP_CS = SPI_CS_DEASSERT_LVL;
    BSP_DISP_EN = DISP_EN_EFM8;

    Wait(10);

    DISP_ClearAll();
}

/***************************************************************************//**
 * @brief Clear the display to the background color
 *
 *****************************************************************************/
void DISP_ClearAll()
{
#if DISP_BACKGROUND_COLOR == COLOR_WHITE
    uint8_t tx[2];

    tx[0] = SoftwareVcomToggle(DISP_CMD_CLEAR_ALL);
    tx[1] = DISP_CMD_DUMMY;

    SPI_StartTransfer(sizeof(tx));
    SPI_WriteQueue(tx, sizeof(tx));
#else
    uint8_t i;

    for (i = 0; i < DISP_HEIGHT; i++)
    {
        DISP_ClearLine(i, COLOR_BLACK);
    }
#endif
}

/***************************************************************************//**
 * @brief Clear a single line
 *
 * @param row line to write (0 = top line; 127 = bottom line)
 * @param bw line color after clearing (0x00 = black; 0xFF = white)
 *
 *****************************************************************************/
void DISP_ClearLine(uint8_t row, uint8_t bw)
{
    uint8_t i;
    uint8_t cmd[DISP_BUF_SIZE + 4];

    if (bw)
    {
        bw = COLOR_WHITE;
    }
    else
    {
        bw = COLOR_BLACK;
    }

    // Send write command and line address
    cmd[0] = SoftwareVcomToggle(DISP_CMD_DYNAMIC_MODE);
    cmd[1] = BitReverse(row+1);

    // Send line data
    for (i = 0; i < DISP_BUF_SIZE; i++)
    {
      cmd[2 + i] = bw;
    }

    // Send dummy data
    cmd[DISP_BUF_SIZE + 2] = DISP_CMD_DUMMY;
    cmd[DISP_BUF_SIZE + 3] = DISP_CMD_DUMMY;

    // Send command
    SPI_StartTransfer(cmd, DISP_BUF_SIZE + 4);
}

/***************************************************************************//**
 * @brief Write a single line to the display
 *
 * @param row line to write (0 = top line; 127 = bottom line)
 * @param line pixel values
 *             (line[0] MSB = left-most pixel; line[15] LSB = right-most pixel)
 *             (set bit to '0' to turn pixel "on" (black))
 *
 *****************************************************************************/
void DISP_WriteLine(uint8_t row, uint8_t line[DISP_BUF_SIZE])
{
    uint8_t cmd[DISP_BUF_SIZE + 4];

    // Send write command and line address
    cmd[0] = SoftwareVcomToggle(DISP_CMD_DYNAMIC_MODE);
    cmd[1] = BitReverse(row+1);

    memcpy(&cmd[2], line, DISP_BUF_SIZE);

    // Send dummy data
    cmd[DISP_BUF_SIZE + 2] = DISP_CMD_DUMMY;
    cmd[DISP_BUF_SIZE + 3] = DISP_CMD_DUMMY;

    // Send command
    SPI_StartTransfer(cmd, DISP_BUF_SIZE + 4);
}
