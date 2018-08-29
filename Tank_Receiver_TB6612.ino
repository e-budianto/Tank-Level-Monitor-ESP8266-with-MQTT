// These #include statement were automatically added by the Particle IDE.
#include <Stepper.h>

// change this to the number of steps on your motor
#define STEPS 600
#define LED D7
#define alertPin A5

//Using WaterTankView Particle Photon

/* create an instance of the stepper class, specifying
the number of steps of the motor and the pins it's
attached to */
Stepper stepper(STEPS, D0, D1, D2, D3);

float h2opercent = 100.0; //double type variable for particle cloud sharing
boolean freshStart = TRUE;
int move = 0;
int lastmove = 0;
int change = 0;
boolean tickTock = TRUE;
uint8_t dayVar = 0;
boolean alarm = FALSE;

void setup()
{
    Particle.subscribe("<your event>", DisplayLevel); //Place your event name here
    pinMode(LED,OUTPUT);
    pinMode(alertPin,OUTPUT);
    //Serial.begin(9600);
    // set the speed of the motor to 30 RPMs
    stepper.setSpeed(60);
    //Set the guage to position "home position" after restart
    stepper.step(-(STEPS+50)); //Go all the way back to the zero marker
    dayVar = Time.day();
}
 
void loop()
{
    if (dayVar != Time.day()) { //If the day has changed then reset the needle to ensure it's correct (perhaps it got out of sync or someone bumped it)
        stepper.step(-STEPS);
        freshStart = TRUE;  //Reset the gauge to ensure it's reading correctly
        dayVar = Time.day();
    }
    
    if (alarm && (h2opercent > 7.1)) {  //This ensures the alert LED is not left in the ON state depending were in the alerting loops it was left when filling occured
        digitalWrite(alertPin,LOW);
        alarm = FALSE;
    }
    if ((h2opercent < 7.0) && (h2opercent >= 4.0))  { //From 3" to 2" slow pulse the LED
        slowpulse();
    } else if (h2opercent < 4.0)  { //Under 2" strobe the LED
            strobe();
        } else { //If the above conditions are not met, then just chill for a sec and start over
            delay(1000);
        }
}

void DisplayLevel (const char *event, const char *percent)
{
    h2opercent = atof(percent);
    move = (int)((h2opercent/100)*600);
    if (freshStart == TRUE) {  //Run only after a reboot or value reset
        freshStart = FALSE;
        stepper.step(move);
        lastmove = move;  //store the variable for the next go around comparison
    } else if (move > lastmove) {
                change = move - lastmove;
                stepper.step(change);
                lastmove = move;
            } else if (move < lastmove) {
                    change =  lastmove - move;
                    stepper.step(-change);
                    lastmove = move;
                    }
}

void slowpulse() {
    alarm = TRUE;
    if (tickTock) {
            for (int i = 0; i < 256; i++) {
                analogWrite(alertPin,i);
                delay(20);
            }
            tickTock = FALSE;
            delay(1000);
    } else {
        for (int i = 255; i >= 0; i--) {
            analogWrite(alertPin,i);
            delay(20);
        }
        tickTock = TRUE;
    }
}

void strobe() {
    alarm = TRUE;
    if (tickTock) {
        digitalWrite(alertPin,HIGH);
        delay(75);
        tickTock = FALSE;
    } else {
        digitalWrite(alertPin,LOW);
        delay(75);
        tickTock = TRUE;
    }
}
