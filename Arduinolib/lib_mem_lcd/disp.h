
// disp.h

#ifndef DISP_H_
#define DISP_H_


/////////////////////////////////////////////////////////////////////////////
// Definitions
/////////////////////////////////////////////////////////////////////////////

/// DISP_EN signal level for MCU control of LCD
#define DISP_EN_EFM8              1
/// DISP_EN signal level for board controller control of LCD
#define DISP_EN_BC                0

/// Specifies the foreground color drawn to the display
#define DISP_FOREGROUND_COLOR   (DISP_BACKGROUND_COLOR == COLOR_WHITE ? COLOR_BLACK : COLOR_WHITE)

#define COLOR_BLACK               0x00    ///< LCD color code for all black
#define COLOR_WHITE               0xFF    ///< LCD color code for all white

#define DISP_CMD_CLEAR_ALL        0x20    ///< LCD command to clear all rows
#define DISP_CMD_DYNAMIC_MODE     0x80    ///< LCD command to switch to dynamic update mode
#define DISP_CMD_DUMMY            0x00    ///< LCD dummy byte (don't care)
#define DISP_CMD_VCOM_MASK        0x40    ///< LCD software VCOM bitmask

#define DISP_WIDTH                128     ///< LCD width in pixels
#define DISP_HEIGHT               128     ///< LCD height in pixels

/// Line, or row, buffer size in bytes
#define DISP_BUF_SIZE             (DISP_WIDTH / 8)

#define DISP_BACKGROUND_COLOR     COLOR_BLACK
#define DISP_VCOM_TOGGLE_RATE_HZ  5

/////////////////////////////////////////////////////////////////////////////
// Prototypes
/////////////////////////////////////////////////////////////////////////////

/***************************************************************************//**
 * @brief Initializes the display driver and clears the display
 *
 *****************************************************************************/
void DISP_Init();

 /***************************************************************************//**
  * @brief Clear the display to the background color
  *
  *****************************************************************************/
void DISP_ClearAll();

/***************************************************************************//**
 * @brief Clear a single line
 *
 * @param row line to write (0 = top line; 127 = bottom line)
 * @param bw line color after clearing (0x00 = black; 0xFF = white)
 *
 *****************************************************************************/
void DISP_ClearLine(uint8_t row, uint8_t bw);

/***************************************************************************//**
 * @brief Write a single line to the display
 *
 * @param row line to write (0 = top line; 127 = bottom line)
 * @param line pixel values
 *             (line[0] MSB = left-most pixel; line[15] LSB = right-most pixel)
 *             (set bit to '0' to turn pixel "on" (black))
 *
 *****************************************************************************/
void DISP_WriteLine(uint8_t row, uint8_t line[DISP_BUF_SIZE]);

/** @} disp_runtime */

/** @} disp */

/** @} mem_lcd */

#endif /* DISP_H_ */
