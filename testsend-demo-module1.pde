

#include <WaspXBee802.h>
#include <WaspFrame.h>
#include <WaspXBeeZB.h>
#include <Wire.h>

// Destination MAC address
//////////////////////////////////////////
char RX_ADDRESS[] = "0013A20040F5BBFD";
//////////////////////////////////////////

// define methods
//int8_t wakeup();
void receive_process(void);
void send_process(int8_t);


// define variable
uint8_t error;
char auth[20]="A0B01A99BC75F612D4";
int8_t phase=0;
bool first_time= true;
bool first_time2= true;
uint8_t stat;


void setup()
{
  
  USB.ON();  // init USB port
  PWR.setSensorPower(SENS_3V3,SENS_ON);
  delay(50);
  PWR.setSensorPower(SENS_5V,SENS_ON);
  delay(50);
  
  USB.println(F("Node Example"));

  // set Waspmote identifier
  
  frame.setID("node_1");

  // init XBee
  xbee802.ON();
  
  pinMode(DIGITAL8,OUTPUT); //shutdown mode = 0, leave shutdown =1
  digitalWrite(DIGITAL8, LOW); //Enter shutdown
  pinMode(DIGITAL7,OUTPUT); // wake up I2C pin
  pinMode(DIGITAL6,INPUT); // Slave mute I2C pin
  pinMode(DIGITAL5,INPUT); // present I2C pin
  if(!RTC.isON) RTC.setMode(RTC_ON, RTC_I2C_MODE);
  delay(50);
 // Wire.begin(); 
  delay(50);
  //checkNetworkParams();
  Utils.blinkLEDs(100);
}

int counter = 0;

void loop()
{ 
  

  //////////////////////////
  // 1. create frame
  //////////////////////////  
 send_process(phase);
  // 1.1. create new frame
  //frame.createFrame(ASCII);  

  // 1.2. add frame fields
  //frame.addTimestamp();
 // addSensor(SENSOR_STR, pr_str);
  
  
  //USB.println(F("\n1. Created frame to be sent"));
  //frame.showFrame();

  //////////////////////////
  // 2. send packet
  //////////////////////////  
  
  // send XBee packet
  //error = xbee802.send( RX_ADDRESS, frame.buffer, frame.length );   
  
  //USB.println(F("\n2. Send a packet to the RX node: "));
  
  // check TX flag
  

  //wakeup();
  Utils.blinkLEDs(1);
  
  // wait for 5 seconds
  USB.println(F("\n----------------------------------"));
  delay(50);

}

//int8_t
void send_process(int8_t phase){
  if(phase==0){
    // 1.1. create new frame
  frame.createFrame(ASCII);  
  frame.addTimestamp();
  frame.addSensor(SENSOR_STR, phase);
  //USB.println(F("\n1. Created frame to be sent"));
  frame.showFrame();  
  // send XBee packet
  error = xbee802.send( RX_ADDRESS, frame.buffer, frame.length );   
  }
  if(phase ==1){
    if(first_time){
    if (stat == 0){
    frame.createFrame(ASCII);  
    frame.addTimestamp();
    frame.addSensor(SENSOR_STR, phase);
    frame.addSensor(SENSOR_STR, "stat:");
    frame.addSensor(SENSOR_STR, stat);
    frame.addSensor(SENSOR_STR, auth[20]);
     //USB.println(F("\n1. Created frame to be sent"));
    frame.showFrame();  
     // send XBee packet
    error = xbee802.send( RX_ADDRESS, frame.buffer, frame.length );
    first_time= false;
    }
    else{
    USB.println(F("!!!!!!!!!!Card Problem!!!!!!!!"));
    error =14;
    }
    }
    else{
       error = xbee802.send( RX_ADDRESS, frame.buffer, frame.length );
        USB.println(F("Send Again"));
    }
  }
  if(phase ==2){
    if(first_time2){
    if (stat == 0){
    frame.createFrame(ASCII);  
    frame.addTimestamp();
    frame.addSensor(SENSOR_STR, phase);
    frame.addSensor(SENSOR_STR, "stat:");
    frame.addSensor(SENSOR_STR, stat);
    frame.addSensor(SENSOR_STR, auth[20]);
     //USB.println(F("\n1. Created frame to be sent"));
    frame.showFrame();  
     // send XBee packet
    error = xbee802.send( RX_ADDRESS, frame.buffer, frame.length );
    first_time2= false;
    }
    else{
    USB.println(F("!!!!!!!!!!Card Problem!!!!!!!!"));
    error =15;
    }
    }
    else{
       error = xbee802.send( RX_ADDRESS, frame.buffer, frame.length );
        USB.println(F("Send Again"));
    }
  }

  if( error == 0 )
  {
    USB.println(F("Xbee send ok"));
    USB.print("phase:");
    USB.println(phase);
    //delay(5);
    receive_process();

    // blink green LED
    Utils.blinkGreenLED();  

  }
  else 
  {
    USB.println(F("send error"));
    delay(20);
    // blink red LED
    Utils.blinkRedLED(); 
    send_process(phase);
  } 
  
}

void receive_process(void){
   // receive XBee packet (wait for 10 seconds)
   error = xbee802.receivePacketTimeout( 10000 );
    USB.println(F("Receive:"));

  if( error == 0 ) 
  {
    // Show data stored in '_payload' buffer indicated by '_length'
    USB.print(F("Data: "));  
    USB.println( xbee802._payload, xbee802._length);
    
    // Show data stored in '_payload' buffer indicated by '_length'
    USB.print(F("Length: "));  
    USB.println( xbee802._length,DEC);
    delay(50);
    phase ++;
    send_process(phase);
  }
  else
  {
    // Print error message:
    /*
     * '7' : Buffer full. Not enough memory space
     * '6' : Error escaping character within payload bytes
     * '5' : Error escaping character in checksum byte
     * '4' : Checksum is not correct   
     * '3' : Checksum byte is not available 
     * '2' : Frame Type is not valid
     * '1' : Timeout when receiving answer   
    */
    counter++;
    USB.print(F("Error receiving a packet:"));
    USB.println(error,DEC);    
    if(counter < 4){
    receive_process();
    }
    else{
      counter = 0;
      send_process(phase);
    }
  }
}

