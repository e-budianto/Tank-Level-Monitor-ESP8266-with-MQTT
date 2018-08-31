// These #include statement were automatically added by the Particle IDE.

#include <HC_SR04.h>

#define trigPin D1
#define echoPin D2

float inches2water = 0.0;
float tank = 0.0;
int counter = 0;
float tank_limit = 45.5; //Max distance to empty
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
 
HC_SR04 rangefinder = HC_SR04(trigPin, echoPin, 5.0, 150.0);
 
void setup() 
{
    Serial.begin(9600);
    delay(2000);
    initialize();
}
 
void loop() 
{
    counter++;
    if (counter==30) { //Every ~5min pull the numbers from HRC-SR04 Rangefinder
        watertank();
        // Send to MQTT
        counter = 0; //Reset the counter to start a new 5min count down
    }
    delay(5000);
}


void watertank() {
    inches2water = 0.0; //Initialize the variable for averaging
    for (int l=0; l<3; l++) { //Take 3 readings from the sensor 2sec apart to get an average
        inches2water = inches2water + rangefinder.getDistanceInch();
        delay(2000);
    }
    inches2water = (round((inches2water/3)*10)/10) - 2.0; //Take the average and then round the number to 1 decimal point
    if (inches2water < 0.1) {
        tank = 100.0;
        h2opercent = 100.0;
    }
        else if (inches2water > tank_limit) {
            tank = 0.0;
            h2opercent = 0.0;
        }
            else {
                tank = round((100-((inches2water/tank_limit)*100))*10)/10;
                h2opercent = round((100-((inches2water/tank_limit)*100))*10)/10;
            } 
//Will be changed to MQTT
//    ubidots.add("WT_Tank",tank);
//    ubidots.add("WT_Inches",inches2water);
//    percent = String(h2opercent, 1);
//    Particle.publish("<your stream name>", percent);  //Set this to your stream name
 //End Will be changed
}

void initialize() {  //Take an initial reading so we don't have to wait 5min to see the results after reboot/updates
    watertank();
 //Will be changed to MQTT
//    ubidots.sendAll(); //Send all data at once to the Ubidot Cloud
}
