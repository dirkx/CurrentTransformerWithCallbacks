#include <CurrentTransformer.h>

CurrentTransformer sensor = CurrentTransformer( 2 );

void setup() {

 sensor.onCurrentChange([](CurrentTransformer::state_t state) {
   switch (state) {
     case CurrentTransformer::ON:
       Serial.println("Change to on");
       break;
     case CurrentTransformer::OFF:
       Serial.println("Change to off");
       break;
     case CurrentTransformer::UNKNOWN:
       Serial.println("Unknown state");
       break;
   }
 });

}

void loop() {
}

