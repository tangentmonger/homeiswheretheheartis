

#include <Arduino.h>

#define CHANNEL 1
#define TIMEOUT_MS (5 * 1000)

// LXn cues, also used as the state machine
#define OFF 0
#define PORTAL_1_PULSE   1
#define PORTAL_2_PULSE   2
#define PORTAL_3_PULSE   3
#define PORTAL_4_PULSE   4
#define PORTAL_1_BUILDUP     10
#define PORTAL_2_BUILDUP     20
#define PORTAL_3_BUILDUP     30
#define PORTAL_4_COMPLETION     40

// GPIOs for break beam sensors
#define PORTAL_1_SENSOR 2
#define PORTAL_2_SENSOR 3
#define PORTAL_3_SENSOR 4
#define PORTAL_4_SENSOR 5

// ISR flags for break beam triggers
volatile bool portal_1_triggered = false;
volatile bool portal_2_triggered = false;
volatile bool portal_3_triggered = false;
volatile bool portal_4_triggered = false;

int mode = OFF;
int timerStart = -1 * TIMEOUT_MS;  // start in a timed-out state



void sendMIDINoteOnOff(byte note)
{
    //MIDI channels 1-16 are really 0-15
    byte noteOnStatus=0x90 + (CHANNEL-1);
    //temp debug
    //Serial.println(note);

    //Turn note on
    Serial.write(noteOnStatus);
    Serial.write(note);
    Serial.write(100);   // high velocity = note on
    
    //Hold note for 60 ms (delay() used for simplicity)
    delay(60); 
    
    //Turn note off (velocity = 0)
    Serial.write(noteOnStatus);
    Serial.write(note);
    Serial.write(0);        // low velocity = note off
}


void setup()
{
    //Set up serial output with standard MIDI baud rate
    Serial.begin(31250);
    pinMode(LED_BUILTIN, OUTPUT);
    
    pinMode(PORTAL_1_SENSOR, INPUT);
    pinMode(PORTAL_2_SENSOR, INPUT);
    pinMode(PORTAL_3_SENSOR, INPUT); 
    pinMode(PORTAL_4_SENSOR, INPUT);    
 
    // we need more than two interrupts, so faff about with bits
    PCMSK2 |= bit (PCINT18); // Pin D02
    PCMSK2 |= bit (PCINT19); // Pin D03
    PCMSK2 |= bit (PCINT20); // Pin D04
    PCMSK2 |= bit (PCINT21); // Pin D05
    PCIFR  |= bit (PCIF2);   // clear any outstanding interrupts
    PCICR  |= bit (PCIE2);   // enable pin change interrupts for D00 to D07
    
}

ISR (PCINT2_vect) //D02-07
{
    if (bitRead(PIND, PORTAL_1_SENSOR) == 1) portal_1_triggered = true;
    if (bitRead(PIND, PORTAL_2_SENSOR) == 1) portal_2_triggered = true;
    if (bitRead(PIND, PORTAL_3_SENSOR) == 1) portal_3_triggered = true;
    if (bitRead(PIND, PORTAL_4_SENSOR) == 1) portal_4_triggered = true;
}

void loop()
{
    while(true)
    {
        sendMIDINoteOnOff(PORTAL_1_PULSE);
        delay(1000);
    
    }


    if (portal_1_triggered)
    {
        portal_1_triggered = false;
        sendMIDINoteOnOff(PORTAL_1_PULSE);
    
        if ((mode == OFF) or (millis() - timerStart <= TIMEOUT_MS))
        {
            mode = PORTAL_1_BUILDUP;
            //note the time
            timerStart = millis();
        }
    }
    
    if (portal_2_triggered)
    {
        portal_2_triggered = false;
        //always react with a pulse
        sendMIDINoteOnOff(PORTAL_2_PULSE);
        
        // see if we need to do the collection
        if (mode == PORTAL_1_BUILDUP)
        {
            if (millis() - timerStart <= TIMEOUT_MS)
            {
                // portal 2 has been triggered within 30s of portal 1, so do the collection effects
                mode = PORTAL_2_BUILDUP;
                sendMIDINoteOnOff(PORTAL_2_BUILDUP);
                //note the time
                timerStart = millis();
            }
            else
            {
                //the hippies are doing it wrong, reset the system
                mode = OFF;
            }
        }
    }
    
    if (portal_3_triggered)
    {
        portal_3_triggered = false;
        //always react with a pulse
        sendMIDINoteOnOff(PORTAL_3_PULSE);
        
        // see if we need to do the collection
        if (mode == PORTAL_2_BUILDUP)
        {
            if (millis() - timerStart <= TIMEOUT_MS)
            {
                // portal 3 has been triggered within 30s of portal 2 (etc), so do the collection effects
                mode = PORTAL_3_BUILDUP;
                sendMIDINoteOnOff(PORTAL_3_BUILDUP);
                //note the time
                timerStart = millis();
            }
            else
            {
                //the hippies are doing it wrong AGAIN, reset the system
                mode = OFF;
            }
        }
    }    
   
    if (portal_4_triggered)
    {
        portal_4_triggered = false;
        //always react with a pulse
        sendMIDINoteOnOff(PORTAL_4_PULSE);
        
        // see if we need to do the collection
        if (mode == PORTAL_3_BUILDUP)
        {
            if (millis() - timerStart <= TIMEOUT_MS)
            {
                // portal 4 has been triggered within 30s of portal 3 (etc), so do the collection effects
                mode = PORTAL_4_COMPLETION;
                sendMIDINoteOnOff(PORTAL_4_COMPLETION);
                //note the time
                timerStart = millis();
            }
            else
            {
                //seriously what is it with these hippies, reset the system
                mode = OFF;
            }
        }
    }    
   
    delay(10);

}


 

