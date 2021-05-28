//libraries
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <Arduino_JSON.h>

//Arduino related variables
const int openDelay = 1000; //time it takes to fully extend actuator
const int closeDelay = 1000;//time it takes to fully retract actuator
const int openRelay = 13;
const int closeRelay = 11;
const int openButton = 7;
const int closeButton = 5;

//Intitialising stuff needed to connect and recieve data from MQTT server
  //MAC & IP (change to correct values)
  byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
  IPAddress ip(94, 2, 158, 86); //will be ignored if ISP provides one
  
  //identify MQTT server
  const char* server = "broker.emqx.io"; //add tcp:// if it bricks
  
  /* creates an ethernet object which is then taken by the MQTT broker so we dont have to
  deal with the ethernet shield when coding */
  EthernetClient ethClient;
  PubSubClient mqttClient(ethClient);
  
  //declare variable for decoded JSON data
  char* type;
  int* val;
  
  
  
//Receives and decodes JSON data from MQTT broker
void subscribeReceive(char* topic, byte* payload, unsigned int length) {
  char str[length+1];
  Serial.print("Message arrived [%s]", topic);
  
  //int i = 0;
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
    str[i]=(char)payload[i];
  }
  str[i] = 0; //Null termination
  
  StaticJsonDocument<256> doc; //change the 256 to a higher value if it doesnt work
  deserializeJson(doc, str); //try payload instead of str if crashes
  
  //reads data and writes into variables which can be used
  type = doc["type"];
  val = doc["val"];
  
  //check the values
  Serial.println("type = %s", type);
  Serial.println("val = %d", val);
}

//function to publish to MQTT server
void publishVal(int newVal){boolean rc = mqttClient.publish("status/1234", newVal);}


void setup()
{
  Serial.begin(9600);
  
  //Begin Ethernet & give time to boot
  Ethernet.begin(mac, ip);
  delay(1500);                          
 
  //Set the MQTT server
  mqttClient.setServer(server, 1883);   
 
  //Try and connect
  if (mqttClient.connect("arduino-1")) {
    Serial.println("Connected");
 
    // Establish the subscribe event
    mqttClient.setCallback(subscribeReceive);
  } 
  else {Serial.println("Failed to connect");}
  
  //pinModes
  pinMode(openRelay, OUTPUT);
  pinMode(closeRelay, OUTPUT);
  pinMode(openButton, INPUT);
  pinMode(closeButton, INPUT);
}

void loop(){
  //read values from buttons
  int openButtonVal = digitalRead(openButton);
  int closeButtonVal = digitalRead(closeButton);
  
  //check server for messages on subscribed topic
  mqttClient.subscribe("devices/1234");
  mqttClient.loop();
  
  //Check if operator command
  if(type == "op"){
    
    //extend actuator if val is 100 & publish
    if(val == 100){
      digitalWrite(openRelay, HIGH);
      delay(openDelay);
      digitalWrite(openRelay, LOW);
      publishVal(100);
    }
    
    //retract actuator if val is 0 & publish
    else-if(val = 0){
      digitalWrite(closeRelay, HIGH);
      delay(closeDelay);
      digitalWrite(closeRelay, LOW);
      publishVal(0);
    }
  }
  
  //button overrides
  if(openButtonVal == HIGH){
      digitalWrite(openRelay, HIGH);
      delay(openDelay);
      digitalWrite(openRelay, LOW);
      publishVal(100);
    }
    
    if(closeButtonVal == HIGH){
      digitalWrite(closeRelay, HIGH);
      delay(closeDelay);
      digitalWrite(closeRelay, LOW);
      publishVal(0);
    }
}

