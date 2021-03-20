#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <PZEM004Tv30.h>
#include "DHT.h"
DHT dht;

PZEM004Tv30 pzem(5,4);              // (RX , TX) pin of sensor (TX , RX) pin of board
RF24 radio(9,10);                   // nRF24L01(+) radio attached using Getting Started board 
RF24Network network(radio);         // Network uses that radio

//fix needed (depends on your design)
const uint16_t this_node = 01;      // Address of our node in Octal format
byte rMode = 1;                     //if 1 it is mean router mode, another is end device mode(reccomend 0)
byte sInterval = 0;                 //sInterval = [0 1 2 3] = [1 sec 1 min 10 min 15 min]

//unneeded
const uint16_t master_node = 00;       
bool timer = true; 
unsigned long tInterval; 
unsigned long tStart; 
int count;  
struct payload_t {                  
  float Energy;
  float Voltage;
  float Current;
  float temp;
  float humid;
};
 
void setup(void)
{
  dht.setup(6); // data pin 6
  delay(1000);
  pinMode(7, OUTPUT); //set digital pin 7 as output to control status LED
  setSleepInterval(sInterval); 
  SPI.begin();
  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);                             //Set this for PA level
  network.begin(/*channel*/ 85, /*node address*/ this_node);  //Set channel for communicate channel
  Serial.begin(115200);
  digitalWrite(7,HIGH); //turn on the light
}

void loop() {
    network.update();    
    if(timer) {                     
    Serial.print("Sending...");
    payload_t payload = {get_energy() , get_voltage() , get_current() , get_temperature() , get_humidity()};
    RF24NetworkHeader header(/*to node*/ master_node);
    bool ok = network.write(header,&payload,sizeof(payload));
    if (ok)
      {
        Serial.print("ok, send to node: ");
        Serial.println(header.to_node,HEX);
        digitalWrite(7,LOW); //transmit was successful so make sure status LED is off
      }
    else {
      Serial.println("failed.");
      PIND |= (1<<PIND7); //this toggles the status LED at pin seven to show transmit failed
      }
    }
  if(!rMode) { //if we are in end device mode get ready to go to sleep
    timer = true; //wer are not using the timer so make sure variable is set to true
    Serial.end();
    radio.powerDown(); //power down the nRF24L01 before we go to sleep
    ADCSRA &= ~(1<<ADEN); //Turn off ADC before going to sleep(set ADEN bit to 0).this saves even more power
    goToSleep(); //function for putting the Atmega328 to sleep
    ADCSRA |= (1<<ADEN); //Turn the ADC back on
    Serial.begin(115200);
    radio.powerUp();
  }
  else { //we are in router mode 
    timer = checkTimer(); //update timer and check to see if it is time to transmit
  }
}

void setSleepInterval(byte interval) {

  //only for ATmega328
  //if u want to create by yourself, watch RF24Network library
  //these are just my set up
  
  if(interval == 0) { //set transmit interval to 1 sec
    count = 1;
    tInterval = 1000; //testing if time interval is going off
    network.setup_watchdog(6); //Sets sleep interval of WDT, wdt_1s
  }
  else if(interval == 1) { //set transmit interval to 1 min
    count = 15;
    tInterval = 60000;
    network.setup_watchdog(8); //Sets sleep interval of WDT, wdt_4s
  }
  else if(interval == 2) { //sets transmit interval to 10 min
    count = 75;
    tInterval = 600000;
    network.setup_watchdog(9); //Sets sleep interval of WDT, wdt_8s
  }
  else {
    count = 225;
    tInterval = 900000;
    network.setup_watchdog(8); //Sets sleep interval of WDT, wdt_4s
  }
}

bool goToSleep() {
  Serial.end();
  if(network.sleepNode(count,255)) return true; //count sets the number of intervals to sleep, 255 means do not wake up on interrupt
  else return false;
}

bool checkTimer() {
  unsigned long now = millis();     //get timer value
  if ( now - tStart >= tInterval  ) //check to see if it is time to transmit based on set interval
  {
    tStart = now; //reset start time of timer
    return true;
  }
  else return false;
}

float get_energy(){
  return pzem.energy();
}
float  get_voltage(){
   return pzem.voltage();
}
float get_current(){
   return pzem.current();
}
float get_temperature(){
  return dht.getTemperature(); // 
}
float get_humidity(){
  return dht.getHumidity(); // 
}
