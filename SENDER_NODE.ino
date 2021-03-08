
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <PZEM004Tv30.h>
#include "DHT.h"

DHT dht;
PZEM004Tv30 pzem(5,4);
RF24 radio(9,10);                    // nRF24L01(+) radio attached using Getting Started board 
 
RF24Network network(radio);          // Network uses that radio
 
const uint16_t this_node = 011;        // Address of our node in Octal format
const uint16_t other_node = 00;       // Address of the other node in Octal format
 
const unsigned long interval = 2000; //ms  // How often to send 'hello world to the other unit
 
unsigned long last_sent;             // When did we last send?
unsigned long packets_sent = 0;          // How many have we sent already
 
struct payload_t {                  // Structure of our payload
  float Energy;
  float Voltage;
  float Current;
  float temp;
  float humid;
};
 
void setup(void)
{
  dht.setup(6); // data pin 6
  //delay(dht.getMinimumSamplingPeriod());
  delay(1000);
  Serial.begin(115200);
  Serial.println("RF24Network/examples/helloworld_tx/");
 
  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 85, /*node address*/ this_node);
}
 
void loop() {
  delay(2500);
  network.update();                          // Check the network regularly
  unsigned long now = millis();              // If it's time to send a message, send it!
  if ( now - last_sent >= interval  )
  {
    last_sent = now;
 
    Serial.print("Sending...");
    payload_t payload = { get_energy() , get_voltage() , get_current() , get_temperature() , get_humidity() };
    RF24NetworkHeader header(/*to node*/ other_node);
    bool ok = network.write(header,&payload,sizeof(payload));
    if (ok)
      {
        Serial.print("ok, send to node: ");
      Serial.println(header.to_node,OCT);
      }
    else
      Serial.println("failed.");
  }
}

float get_temperature(){
  return dht.getTemperature();
  }

float get_humidity(){
  return dht.getHumidity();
  }
float get_energy(){
  return pzem.energy();
  }

float get_voltage(){
   return pzem.voltage();
  }

float get_current(){
   return pzem.current();
  }
