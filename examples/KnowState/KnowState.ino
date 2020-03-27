#include <CurrentTransformerWithCallbacks.h>

CurrentTransformerWithCallbacks sensor = CurrentTransformerWithCallbacks( 2 );

void setup() {

 sensor.onCurrentChange([](CurrentTransformerWithCallbacks::state_t state) {
   switch (state) {
     case CurrentTransformerWithCallbacks::ON:
       Serial.println("Change to on");
       break;
     case CurrentTransformerWithCallbacks::OFF:
       Serial.println("Change to off");
       break;
     case CurrentTransformerWithCallbacks::UNKNOWN:
       Serial.println("Unknown state");
       break;
   }
 });

}

void loop() {
}

