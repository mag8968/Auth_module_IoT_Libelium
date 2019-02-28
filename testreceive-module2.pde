/*  
 *  ------ [802_03] - receive XBee packets -------- 
 *  
 *  Explanation: This program shows how to receive packets with 
 *  XBee-802.15.4 modules.
 *  
 *  Copyright (C) 2016 Libelium Comunicaciones Distribuidas S.L. 
 *  http://www.libelium.com 
 *  
 *  This program is free software: you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by 
 *  the Free Software Foundation, either version 3 of the License, or 
 *  (at your option) any later version. 
 *  
 *  This program is distributed in the hope that it will be useful, 
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 *  GNU General Public License for more details. 
 *  
 *  You should have received a copy of the GNU General Public License 
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 *  
 *  Version:           3.0
 *  Design:            David Gascón 
 *  Implementation:    Yuri Carmona
 */
 
#include <WaspXBee802.h>
#include <WaspFrame.h> 


// Destination MAC address
//////////////////////////////////////////
char RX_ADDRESS[] = "0013A20040F5BC02";
//////////////////////////////////////////


// define variable
uint8_t error;
void receive_process(void);
void send_process(int8_t);
int phase=-1;
char auth[20]="A0C11D8623ABE5749D";
char Rand[20]="993E4D2A1C75489BF0";
bool Ack=false;

void setup()
{  
  
  USB.ON(); // init USB port
  PWR.setSensorPower(SENS_3V3,SENS_ON);
  delay(50);
  PWR.setSensorPower(SENS_5V,SENS_ON);
  delay(50);
  USB.println(F("Auth example"));
  // set Waspmote identifier
  frame.setID("node_2");

  // init XBee 
  xbee802.ON();
  pinMode(DIGITAL8,OUTPUT); //shutdown mode = 0, leave shutdown =1
  digitalWrite(DIGITAL8, LOW); //Enter shutdown
  pinMode(DIGITAL7,OUTPUT); // wake up I2C pin
  pinMode(DIGITAL6,INPUT); // Slave mute I2C pin
  pinMode(DIGITAL5,INPUT); // present I2C pin
  if(!RTC.isON) RTC.setMode(RTC_ON, RTC_I2C_MODE);
  delay(50);
  Wire.begin(); 
  delay(50);
  Utils.blinkLEDs(100);
  
}

int counter=0;
void loop()
{ 
  // receive XBee packet (wait for 10 seconds)
  receive_process();
  

} 



void send_process(int8_t phase){
  if(phase==0){
    // 1.1. create new frame
  frame.createFrame(ASCII);  
  frame.addTimestamp();
  frame.addSensor(SENSOR_STR, "phase:");
  frame.addSensor(SENSOR_STR, phase);
  //USB.println(F("\n1. Created frame to be sent"));
  frame.showFrame();  
  // send XBee packet
  error = xbee802.send( RX_ADDRESS, frame.buffer, frame.length );   
  }
  if(phase ==1){  
  USB.println(F("*********Cert OK*********"));
  USB.println("Send Random challenge");
  frame.createFrame(ASCII);  
  frame.addTimestamp();
  frame.addSensor(SENSOR_STR, "phase:");
  frame.addSensor(SENSOR_STR, phase);
   frame.addSensor(SENSOR_STR, Rand);
  //USB.println(F("\n1. Created frame to be sent"));
  frame.showFrame();  
  // send XBee packet
  error = xbee802.send( RX_ADDRESS, frame.buffer, frame.length ); 
  
  if(phase==2){
      
  Ack=true;
  USB.println("*********Accept Key******");
  frame.createFrame(ASCII);  
  frame.addTimestamp();
  frame.addSensor(SENSOR_STR, "phase:");
  frame.addSensor(SENSOR_STR, phase);
  frame.addSensor(SENSOR_STR, "Ack:");
  frame.addSensor(SENSOR_STR, Ack);
   frame.showFrame();  
  // send XBee packet
  error = xbee802.send( RX_ADDRESS, frame.buffer, frame.length );     
  }
    
    /*
  uint8_t stat=wakeup();
  
  frame.createFrame(ASCII);  
  frame.addTimestamp();
  frame.addSensor(SENSOR_STR, stat);
  //USB.println(F("\n1. Created frame to be sent"));
  frame.showFrame();  
  // send XBee packet
  error = xbee802.send( RX_ADDRESS, frame.buffer, frame.length );
  */
  }
  
  if( error == 0 )
  {
    USB.println(F("Xbee send ok"));
    USB.print("phase:");
    USB.println(phase);
    delay(5);
    receive_process();

    // blink green LED
    Utils.blinkGreenLED();  

  }
  else 
  {
    USB.println(F("send error"));
    USB.print("error:");
     USB.println(error);
    delay(20);
    // blink red LED
    Utils.blinkRedLED(); 
    send_process(phase);
  } 
  
}

void receive_process(void){
   // receive XBee packet (wait for 10 seconds)
   error = xbee802.receivePacketTimeout( 10000 );

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

