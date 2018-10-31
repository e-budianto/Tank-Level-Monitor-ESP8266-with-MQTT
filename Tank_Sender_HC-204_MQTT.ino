#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <HC_SR04.h>

#define trigPin 5 // D1 NodeMCU
#define echoPin 4 // D2 NodeMCU
#define vibr_Pin 14 // D5 NodeMCU


const char* ssid = "Your Wifi";
const char* password =  "Wifi Password";
 
const char* mqttServer = "mqtt server";
const int mqttPort = 1883;
const char* mqttUser = "mqtt_user";
const char* mqttPassword = "mqtt_pass";

float cm2water = 0.0;
float tank = 0.0;
int counter1 = 0;
int counter2 = 0;
float tank_limit = 200; //Max distance to empty
long last_vibrate1 = 0;
long last_vibrate2 = 0;
int pump_state = 0;
String percent;


/*
Connect an HC-SR04 Range finder as follows:
Spark   HC-SR04
GND     GND
5V      VCC
D1     Trig
D2      Echo
 
The default usable rangefinder is 10cm to 250cm. Outside of that range -1 is returned as the distance.
 
You can change this range by supplying two extra parameters to the constructor of minCM and maxCM, like this:
 
HC_SR04 rangefinder = HC_SR04(trigPin, echoPin, 5.0, 300.0);
*/
 
WiFiClient espClient;
PubSubClient client(espClient); 
HC_SR04 rangefinder = HC_SR04(trigPin, echoPin, 5.0, 220.0);


long TP_init(){
  delay(10);
  long measurement=pulseIn (vibr_Pin, HIGH);  //wait for the pin to get HIGH and returns measurement
  return measurement;
}


void watertank() {
    cm2water = 0.0; //Initialize the variable for averaging
    for (int l=0; l<3; l++) { //Take 3 readings from the sensor 2sec apart to get an average
        cm2water = cm2water + rangefinder.getDistanceCM();
        delay(2000);
    }
    cm2water = (round((cm2water/3)*10)/10) - 2.0; //Take the average and then round the number to 1 decimal point
    if (cm2water < 0.1) {
        tank = 100.0;
    }
        else if (cm2water > tank_limit) {
            tank = 0.0;
        }
            else {
                tank = round((100-((cm2water/tank_limit)*100))*10)/10;
            } 
        client.publish("esp01-1/SENSOR/Tank", String(tank).c_str());
        client.publish("esp01-1/SENSOR/cm2water", String(cm2water).c_str());
        client.subscribe("esp01-1/SENSOR");

        Serial.print("CM :");
        Serial.println(cm2water);

        Serial.print("Tank Level :");
        Serial.println(tank);
		
}

void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();
}

void setup() {

  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

    if (client.connect("ESP01-1", mqttUser, mqttPassword )) {

      Serial.println("connected");  

    } else {

      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);

    }
  }
  
}
 
void loop() 
{
    long vibrate =TP_init();
    counter1++;
    counter2++;
    if (counter1==150) 
    { //Every ~5min pull the numbers from HRC-SR04 Rangefinder
        watertank();
        counter1 = 0; //Reset the counter to start a new 5min count down
    }
	
    if ((vibrate==0) and (last_vibrate1==0) and (last_vibrate2==0) and (pump_state=1)) 
    {
      pump_state = 0;
      Serial.println("Pump confirmed Off");
      client.publish("esp01-1/SENSOR/Pump", "Off");
    }
	
    if ((vibrate>50) and (last_vibrate1>50) and (last_vibrate2>50) and (pump_state=0))
    {
      pump_state = 1;
      Serial.println("Pump confirmed On");
      client.publish("esp01-1/SENSOR/Pump", "On");
    }
	
    last_vibrate2 = last_vibrate1;	
    last_vibrate1 = vibrate;
    client.loop();
    delay(200);
}
