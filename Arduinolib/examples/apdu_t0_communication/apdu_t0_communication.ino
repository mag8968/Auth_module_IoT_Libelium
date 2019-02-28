#include <Arduino.h>

#include <SCLib.h>


#define SC_C2_RST              7
#define SC_C1_VCC              11
#define SC_C7_IO               10
#define SC_C2_CLK              9



#define SC_SWITCH_CARD_PRESENT 8


#define SC_SWITCH_CARD_PRESENT_INVERT false

#if !defined(ASYNC_CARDS) && !defined(APDU_SUPPORT)
#error This example only works if ASYNC_CARDS and APDU_SUPPORT is enabled in SCLib.h
#endif

// Create SmartCardReader object for further use
SmartCardReader sc(SC_C7_IO, SC_C2_RST, SC_C1_VCC, SC_SWITCH_CARD_PRESENT, SC_C2_CLK, SC_SWITCH_CARD_PRESENT_INVERT);

void setup() {
  Serial.begin(9600);
}


void loop() {
  uint16_t bytes_received = 0;
  APDU_t   command;

 
  Serial.println("Waiting for Smartcard");

  // Wait for card to be inserted into smart card slot
  while (!sc.cardInserted())
  ;

  Serial.println("Smartcard found");

  uint8_t data[255];

  // Gives reset sequence and wait for ATR to be send by smart card
  bytes_received = sc.activate(data, MAX_ATR_BYTES);
  if ( bytes_received > 0) {
    Serial.println("Received ATR ...");

#if defined(SC_DEBUG)
    sc.dumpHEX(data, bytes_received);
#else
    Serial.print(bytes_received);
	Serial.println(" bytes ATR received from card.");
#endif

    // We just use the T=0 byte transfer
    command.cla       = 0x00;
    command.ins       = 0xA4;
    command.p1        = 0x04;
    command.p2        = 0x00;
    command.data_buf  = data;
    command.data_size = 7;
    command.resp_size = 0x100;
    
    data[0] = 0xA0;
    data[1] = 0x00;
    data[2] = 0x00;
    data[3] = 0x00;
    data[4] = 0x04;
    data[5] = 0x10;
    data[6] = 0x10;
    
    Serial.println("SELECT FILE Command ...");
 
    uint16_t result = sc.sendAPDU(&command);
    
    Serial.print("Received ... ");
    Serial.println(result, HEX);

    
    if (((result >> 8) & 0xFF) == 0x61) {
      uint16_t receive_size = result & 0xFF;
      Serial.println("GET RESPONSE Command ...");
      
      // Read Coammnd result via GET RESPONSE
      command.cla       = 0x00;
      command.ins       = 0xC0;
      command.p1        = 0x00;
      command.p2        = 0x00;
      command.data_buf  = data;
      command.data_size = receive_size;
      
     
      result = sc.sendAPDU(&command, false);
      
      Serial.print("Received ... ");
      Serial.println(result, HEX);

#if defined(SC_DEBUG)
      sc.dumpHEX(data, receive_size);
#else
    Serial.print(receive_size);
	Serial.println(" bytes as command response received from card.");
#endif

    }
    
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


