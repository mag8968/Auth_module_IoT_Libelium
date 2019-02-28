#include <SCLib.h>



#define SC_C2_RST              7
#define SC_C1_VCC              11
#define SC_C7_IO               10
#define SC_C3_CLK              9


#define SC_SWITCH_CARD_PRESENT 8


#define SC_SWITCH_CARD_PRESENT_INVERT false

#if !defined(ASYNCHRON_CARDS) || !defined(APDU_SUPPORT) || !defined(T1_SUPPORT)
#error This example only works if ASYNC_CARDS and APDU_SUPPORT and T1_SUPPORT is enabled in SCLib.h
#endif

// Create SmartCardReader object for further use
SmartCardReader sc(SC_C7_IO, SC_C2_RST, SC_C1_VCC, SC_SWITCH_CARD_PRESENT, SC_C3_CLK, SC_SWITCH_CARD_PRESENT_INVERT);

void setup() {
  Serial.begin(9600);
}


void loop() {
  uint16_t bytes_received = 0;
  uint16_t b = 0;

 
  
  Serial.println("Waiting for Smartcard");

  // Wait for card to be inserted into smart card slot
  while (!sc.cardInserted())
  ;

  Serial.println("Smartcard found");

  uint8_t data[255];

  
  bytes_received = sc.activate(data, MAX_ATR_BYTES, CLK_1MHZ);
  if ( bytes_received > 0) {
    Serial.println("Received ATR ...");

#if defined(SC_DEBUG)
    sc.dumpHEX(data, bytes_received);
#else
    Serial.print(bytes_received);
    Serial.println(" bytes ATR received from card.");
#endif
    Serial.print("Checking for T1 - Protocol ... ");

    if (sc.getProtocolFromATR(data, bytes_received) == 1) {
      Serial.println("Supported");
      
      Serial.print("Try to change IFS ... ");
      

     
      if (sc.changeIFS(0, 0x20)) {
        Serial.print("OKAY (Set to 0x");
        Serial.print(sc.getCurrentIFS(), HEX);
        Serial.println(")");
      } else {
        Serial.println("FAILED");
      }

      delay(100);

      // Fill command into user data
      b = 0;
      data[b++] = 0x00;
      data[b++] = 0xA4;
      data[b++] = 0x04;
      data[b++] = 0x00;
      data[b++] = 0x06;
      data[b++] = 0xD2;
      data[b++] = 0x76;
      data[b++] = 0x00;
      data[b++] = 0x01;
      data[b++] = 0x24;
      data[b++] = 0x01;
      data[b++] = 0x00;

      Serial.print("Selecting AID D2 76 00 01 24 01 (OpenPGP) ... ");
      sc.send_IBlock(0x00, data, b);

      for(int i=0; i < 100; i++) {
        bytes_received = sc.receiveBytes(data, 255);
        if (bytes_received > 0) {
          break;
        }
        Serial.print(".");
      }
      Serial.println();
      if (bytes_received > 0)
        sc.dumpHEX(data, bytes_received);
    } else {
      Serial.println(" - NOT Supported -- Abort");
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

