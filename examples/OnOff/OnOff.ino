#include <CurrentTransformerWithCallbacks.h>

CurrentTransformerWithCallbacks sensor = CurrentTransformerWithCallbacks( 2 );

void setup() {

 // Or alternatively - listen to just what you need
 // by getting a On or Off callback.
 //
 sensor.onCurrentOn([]() {
   Serial.println("Its on !");
 });

 sensor.onCurrentOff([]() {
   Serial.println("Its off!t");
 });
}

void loop() {
 // put your main code here, to run repeatedly:
}

