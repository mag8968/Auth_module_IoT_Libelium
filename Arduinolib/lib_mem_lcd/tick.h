/////////////////////////////////////////////////////////////////////////////
// Tick.h
/////////////////////////////////////////////////////////////////////////////

#ifndef TICK_H_
#define TICK_H_

/////////////////////////////////////////////////////////////////////////////
// Macros
/////////////////////////////////////////////////////////////////////////////

// Convert Hertz to milliseconds
#define HZ_TO_MS(hz)            ((1000 + hz/2) / hz)

/////////////////////////////////////////////////////////////////////////////
// Prototypes
/////////////////////////////////////////////////////////////////////////////

uint16_t GetTickCount();
void Wait(uint16_t ms);

#endif /* TICK_H_ */
