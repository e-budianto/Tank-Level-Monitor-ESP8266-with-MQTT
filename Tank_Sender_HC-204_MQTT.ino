// These #include statement were automatically added by the Particle IDE.
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <HC_SR04.h>

#define trigPin D1
#define echoPin D2

const char* ssid = "YourNetworkName";
const char* password =  "YourNetworkPassword";

const char* mqttServer = "192.168.2.150";
const int mqttPort = 8883;
const char* mqttUser = "mqtt_user";
const char* mqttPassword = "YourMqttUserPassword";

float cm2water = 0.0;
float tank = 0.0;
int counter = 0;
float tank_limit = 120; //Max distance to empty
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
HC_SR04 rangefinder = HC_SR04(trigPin, echoPin, 30.0, 200.0);

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
    counter++;
    if (counter==30) { //Every ~5min pull the numbers from HRC-SR04 Rangefinder
        watertank();
        counter = 0; //Reset the counter to start a new 5min count down
    }
    client.loop();
    delay(5000);
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
        h2opercent = 100.0;
    }
        else if (cm2water > tank_limit) {
            tank = 0.0;
            h2opercent = 0.0;
        }
            else {
                tank = round((100-((cm2water/tank_limit)*100))*10)/10;
                h2opercent = round((100-((cm2water/tank_limit)*100))*10)/10;
            } 
        client.publish("esp01-1/SENSOR/Tank", tank);
        client.publish("esp01-1/SENSOR/cm2water", cm2water);
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
