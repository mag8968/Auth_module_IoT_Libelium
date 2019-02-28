#include <Arduino.h>

#include <SCLib.h>


#define SC_C2_RST              7
#define SC_C1_VCC              11
#define SC_C7_IO               10
#define SC_C2_CLK              9


#define SC_SWITCH_CARD_PRESENT 8


#define SC_SWITCH_CARD_PRESENT_INVERT false

// Create SmartCardReader object for further use
SmartCardReader sc(SC_C7_IO, SC_C2_RST, SC_C1_VCC, SC_SWITCH_CARD_PRESENT, SC_C2_CLK, SC_SWITCH_CARD_PRESENT_INVERT);

void setup() {
  Serial.begin(9600);
}


void loop() {
  uint16_t atr_received = 0;

  
  
  Serial.println("Waiting for Smartcard");

  // Wait for card to be inserted into smart card slot
  while (!sc.cardInserted())
  ;

  Serial.println("Smartcard found");

  uint8_t data[MAX_ATR_BYTES];


 
  // Gives reset sequence and wait for ATR to be send by smart card
  
  atr_received = sc.activate(data, MAX_ATR_BYTES);
  if ( atr_received > 0) {
#if defined(SC_DEBUG)
    sc.dumpHEX(data, atr_received);
#else
    Serial.print(atr_received);
	Serial.println(" bytes ATR received from card.");
#endif
    Serial.println();
  } else {
    Serial.println("Unable to identify card ... Please remove");
  }

  delay(2000);
  
  // Deactivate smart card slot (Turn of power etc)
  sc.deactivate();

  // Wait for card to be removed physicaly from slot
  while (sc.cardInserted())
  ;
}


