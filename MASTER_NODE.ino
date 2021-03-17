#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <ESP8266WiFi.h>

const char* ssid = "WSNPP";
const char* pass = "PP01650167";
char thingSpeakAddress[] = "api.thingspeak.com";
String writeAPIKey1 = "GTZ8V5B5XUUO1YNU"; // คีย์จากเมนู key Api แบบ write
String writeAPIKey2 = "DC9B6T98J079GHJO"; 
String writeAPIKey3 = "OTH43QBC6CV4QGY7"; 
String writeAPIKey4 = "WGL53KQHZ4OI9GUW"; 
WiFiClient client;

RF24 radio(4,2);                // nRF24L01(+) radio attached using Getting Started board 
RF24Network network(radio);      // Network uses that radio
const uint16_t this_node = 00;    // Address of our node in Octal format ( 04,031, etc)
 
struct payload_t {                  // Structure of our payload
  float Energy;
  float Voltage;
  float Current;
  float temp;
  float humid;
};
  
void setup(void)
{
  Serial.begin(115200);
  Serial.println("Set up");
  WiFi.begin(ssid, pass);
  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 85, /*node address*/ this_node);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.print("IP address.....");
  Serial.println(WiFi.localIP());
}
 
void loop(void){
  
  network.update();                  // Check the network regularly 
  while ( network.available() ) {    // Is there anything ready for us?
    Serial.print("received");
    payload_t payload;
    RF24NetworkHeader header;        // If so, grab it and print it out
    network.read(header,&payload,sizeof(payload));    
    Serial.print("The data is from node: ");
    Serial.println(header.from_node,OCT);
    Serial.print("Energy: ");
    Serial.print(payload.Energy,3);
    Serial.println(" kWh"); 
    Serial.print("Voltage: ");
    Serial.print(payload.Voltage);
    Serial.println(" V");
    Serial.print("Current: ");
    Serial.print(payload.Current);
    Serial.println(" A"); 
    Serial.print("Temperature: ");
    Serial.println(payload.temp);
    Serial.print("Humidity: ");
    Serial.println(payload.humid);
    uint16_t from_node_OCT = header.from_node;
    String node = (String)from_node_OCT; // แปลงเป็น string 
    String temp = (String)payload.temp; 
    String humid = (String)payload.humid; 
    String v = (String)payload.Voltage; 
    String i = (String)payload.Current; 
    String e = (String)payload.Energy;  
    String data = "field1=" + e + "&field2=" + v + "&field3=" + i + "&field4=" + temp + "&field5=" + humid;
    String apikey;
    
    if(node == "1"){
      apikey = writeAPIKey1;
    }
    if(node == "2"){
      apikey = writeAPIKey2;
    }
    if(node == "9"){
      apikey = writeAPIKey3;
    }
    if(node == "17"){
      apikey = writeAPIKey4;
    }
    if (client.connect(thingSpeakAddress, 80)) {
      client.print("POST /update HTTP/1.1\n");
      client.print("Host: api.thingspeak.com\n");
      client.print("Connection: close\n");
      client.print("X-THINGSPEAKAPIKEY: "+apikey+"\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: ");
      client.print(data.length());
      client.print("\n\n");
      client.print(data);
    }
  }
}
