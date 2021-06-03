#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

//Arduino variables
const int openDelay = 10000; //time it takes to fully extend actuator
const int closeDelay = 10000;//time it takes to fully retract actuator
const int openRelay = 9;
const int closeRelay = 8;
const int openButton = 7;
const int closeButton = 6;
boolean windowOpen = false;

//MAC and IP address
byte mac[]= {  0xAA, 0xBB, 0xCC, 0x00, 0xAB, 0xCD};
IPAddress ip(192, 168, 1, 33);

//server URL
const char* server = "test.mosquitto.org";

//creates ethernet client handled by PubSub
EthernetClient ethClient;
PubSubClient mqttClient(ethClient);

//receive data and decode and parse JSON data
void callback(char* topic, byte* payload, unsigned int length) {

  char str[length+1];
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  int i=0;
  for (i=0;i<length;i++) {
    Serial.print((char)payload[i]);
    str[i]=(char)payload[i];
  }
  str[i] = 0; // Null termination
  
  StaticJsonDocument<256> doc;
  deserializeJson(doc,payload);
  //deserializeJson(doc,str); can use str instead of payload if it doesnt work
  
  //reads data and writes into variables which can be used
  String type = doc["type"];
  int val = doc["val"];
  
  Serial.println("type =");
  Serial.println(type);
  Serial.print(val);
  
  
  //Check if operating command
  if(type == "op"){
    
    //extend actuator if val is 100 & publish
    if(val == 100){
      if (!windowOpen){
        digitalWrite(openRelay, HIGH);
        delay(openDelay);
        digitalWrite(openRelay, LOW);
        Serial.print("Actuator Opened");
        windowOpen = true;
      }
      boolean rc = mqttClient.publish("status/devices/1234", "100");
    }
    
    //retract actuator if val is 0 & publish
    else if(val == 0){
      if(windowOpen){
        digitalWrite(closeRelay, HIGH);
        delay(closeDelay);
        digitalWrite(closeRelay, LOW);
        Serial.print("Actuator Closed");
        windowOpen = false;
      }
      boolean rc = mqttClient.publish("status/devices/1234", "0");
    }
  }
}

void setup()
{
  Serial.begin(9600);
  
  Serial.println("connecting");
  if(Ethernet.begin(mac) == 0){
    Ethernet.begin(mac, ip);
  }
  delay(10000); // Allow the hardware to sort itself out
  Serial.println(Ethernet.localIP());
  
  mqttClient.setServer(server, 1883);
  mqttClient.setCallback(callback);


  while(!mqttClient.connected()){
  if (mqttClient.connect("arduino-1234")) {
    // connection succeeded
    Serial.println("Connected ");
    boolean r= mqttClient.subscribe("devices/1234");
    Serial.println("subscribed");
    Serial.println(r);
  } 
  else {
    Serial.println("Connection failed ");
    Serial.println(mqttClient.state());
    
  }
  }
  
  //Pin Modes
  pinMode(openRelay, OUTPUT);
  pinMode(closeRelay, OUTPUT);
  pinMode(openButton, INPUT);
  pinMode(closeButton, INPUT);
}

void loop(){
  //read values from buttons
  boolean openButtonVal = digitalRead(openButton);
  boolean closeButtonVal = digitalRead(closeButton);
  boolean closeRelayVal = digitalRead(closeRelay);
  boolean openRelayVal = digitalRead(openRelay);
  
  //button overrides
  if(openButtonVal == HIGH){
    if(!windowOpen){
        digitalWrite(openRelay, HIGH);
        delay(openDelay);
        digitalWrite(openRelay, LOW);
        windowOpen = true;
    }
      boolean rc = mqttClient.publish("status/devices/1234", "100");
    }
    
  if(closeButtonVal == HIGH){
    if(windowOpen){
      digitalWrite(closeRelay, HIGH);
      delay(closeDelay);
      digitalWrite(closeRelay, LOW);
      windowOpen = false;
    }
    boolean rc = mqttClient.publish("status/devices/1234", "0");
  }
  
  //check server for messages on subscribed topic
  mqttClient.subscribe("devices/1234");
  mqttClient.loop();
  
  digitalWrite(closeRelay, 0);
  Serial.print('c');
  Serial.print(digitalRead(closeRelay));
  Serial.print(digitalRead(closeButton));
  Serial.print('o');
  Serial.print(digitalRead(openRelay));
  Serial.println(digitalRead(openButton));
  
  delay(1000); //does not override the system
}
