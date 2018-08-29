// These #include statement were automatically added by the Particle IDE.
#include <Ubidots.h>
#include <Adafruit_DHT.h>
#include <HC_SR04.h>

#define DHTPIN D6
#define DHTTYPE DHT22
#define trigPin D1
#define echoPin D2
#define LEDPIN D7
#define TOKEN "<your token>" //Replace with your Ubidots token

Ubidots ubidots(TOKEN);

DHT dht(DHTPIN, DHTTYPE);

float inches2water = 0.0;
float tank = 0.0;
double h2opercent = 0.0; //double type variable for particle cloud sharing
float temp = 0.0;
float hum = 0.0;
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
    pinMode(LEDPIN,OUTPUT);
    Serial.begin(9600);
    dht.begin();
    delay(2000);
    initialize();
}
 
void loop() 
{
    digitalWrite(LEDPIN,HIGH);  //Turn the LED on to act as an additional heartbeat
    counter++;
    if (counter==30) { //Every ~5min pull the numbers from the DHT Sensor and HRC-SR04 Rangefinder
        environmentals();
        watertank();
        ubidots.sendAll(); //Send all data at once to the Ubidot Cloud
        counter = 0; //Reset the counter to start a new 5min count down
    }
    delay(5000);
    digitalWrite(LEDPIN,LOW);
    delay(5000);
}

void environmentals() { //Environmental Data Function
    temp = round(dht.getTempCelcius()*100)/100; //Reads in Celcius
    hum = round(dht.getHumidity()*100)/100; //Reads in %RH
    ubidots.add("WT_Temp",temp);
    ubidots.add("WT_Humidity",hum);
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
    ubidots.add("WT_Tank",tank);
    ubidots.add("WT_Inches",inches2water);
    percent = String(h2opercent, 1);
    Particle.publish("h2opercent_isustrik", percent);
}

void initialize() {  //Take an initial reading so we don't have to wait 5min to see the results after reboot/updates
    environmentals();
    watertank();
    ubidots.sendAll(); //Send all data at once to the Ubidot Cloud
}
