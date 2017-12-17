/* Analog sound sensor sketch
 * 
 * DESCRIPTION 
 * Uses an analog sound sensor to determine if a loud noise is found. 
 * Meant to detect if a doorbell goes off. 
 *  
 * Before the sketch starts, and after every 'recalibrate_time', the sound sensor is (re)calibrated 
 * because the sensor readings seem to fluctuate at different temperatures.
 * 
 * There are 2 timers : 1 for calibration and 1 for reading sensor
 *
 * 
 * Revision History 
 * Version 0.1 - Mark Benschop - 2107-12-16
 * 
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 * 
 */

// Enable debug prints
#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24

#include <MySensors.h>

#define ANALOG_PIN 3 // Pin of sound sensor
#define CHILD_ID 0   // Id of the sensor child


const int calibrate_steps = 32; // Number of readings for calibration
const int SLEEP = 100;

int val;                        // Value from reading sensor 
int avg_min_max[] = {0, 0, 0};  // Array that holds : avg, min, max values

// unsigned longs for time, millis variables
unsigned long now = 0;

unsigned long calibrate_interval = 216000000; // Every hour 
unsigned long calibrate_last;

unsigned long read_interval = 100;
unsigned long read_last;

bool state;
bool old_state;

// Initialize motion message
MyMessage msg(CHILD_ID, V_TRIPPED);


void setup(){
    Serial.begin(115200);//9600);
    pinMode (ANALOG_PIN, INPUT);
    unsigned long prev_millis = 0;
    calibrate(calibrate_steps);
    }


void presentation(){
    // Send the sketch version information to the gateway and Controller
    sendSketchInfo("Sound Sensor", "1.6");
    // Register all sensors to gw (they will be created as child devices)
    present(CHILD_ID, S_SOUND);
    }


/*  calibrate function 
 *  Updates avg_min_max array with new values 
 */
void calibrate(int steps){
    int x = 0;
    int i = 0;
    for ( i; i<steps; i++){
        x += analogRead(ANALOG_PIN * (5.0 / 1023.0) );
        delay(100);
        }
    int avg = x / steps;
    int min = avg - 15;
    int max = avg + 15;
    Serial.print("new avg, min, max : ");
    Serial.print(avg);
    Serial.print(" ");
    Serial.print(min);
    Serial.print(" ");
    Serial.println(max);
    avg_min_max[0] = avg;
    avg_min_max[1] = min;
    avg_min_max[2] = max;
    }


void loop(){

    now = millis();    
    
    /* Reset time - After 51 days the millis register is full and resets to 0 */
    if ( now < calibrate_last ){
        calibrate_last = 0;  
        read_last = 0;
        }
         
    /* Recalibration */
    if ( now - calibrate_last > calibrate_interval ){
        Serial.println("recalibrating");
        calibrate(calibrate_steps);
        calibrate_last = now;
        }


    /* Read sensor */
     if ( now - read_last > read_interval){
         val = analogRead(ANALOG_PIN * (5.0 / 1023.0)) ;
         
         //Serial.print("min : ");
         //Serial.println(avg_min_max[1]);

         //Serial.print("avg : ");
         //Serial.println(avg_min_max[0]);

         
         //Serial.print("val : ");
         //Serial.println(val);

         //Serial.print("max : ");
         //Serial.println(avg_min_max[2]);
         
         read_last = now;
    
         /*  Process values - If value > max or < min we have noise */
         if (val < avg_min_max[1] or val > avg_min_max[2] ){
             Serial.print("val : ");
             Serial.println(val); 
             state = true;
             }
         else {
               state = false;
               }

        if ( state != old_state ){
            Serial.print("State changed to : ");
            Serial.println(state);
            send(msg.set(state?"1":"0"));
            old_state = state;
            }
        }
    }
