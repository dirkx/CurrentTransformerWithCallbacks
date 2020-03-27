#include <CurrentTransformerWithCallbacks.h>

CurrentTransformerWithCallbacks sensor = CurrentTransformerWithCallbacks( 2 );

void setup() {
}

double factor = 3.12; // = Voltage x CoilRatio / VreferenceADC / RburdenResitor

void loop() {
 static unsigned long last = 0;

 // Report the current every 1000mSeconds, eveyr second.
 if (millis() - last > 1000) {
  last = millis();
  Serial.printf("The current is now %f Ampere (Irms).\n",
    sensor.sd() * factor);
 }
}

