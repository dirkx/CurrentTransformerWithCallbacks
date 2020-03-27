# CurrentTransformerWithCallbacks

## Electronics

Requires a circuit such as
```
                               DC VCC (or ADC ref)
                                 |
                                 |
  +---[ machine ]----+          +++
  |                  |          | |  10k
  |                  |          | |  typ.
  |                  |          +++
  |               ---|--         |                    (2)
  |              /---|--\-----------------------------+------> ADC input of Ardino
  |            / \   |  |        |                    |
  |           |   ----------+----+  (1) 0.5 x DC      |
  |            \     | /    |    |                    |
  |              --- --     |   +++                  +++
  |                  |  C  ===  | |  10k             | | Rsense
  +----+       +-----+     ---  | |  typ.            | |
       |       |            |   +++                  +++ 
    50hz household mains    |    |                    |
                            |    |                    |
                           DC ground               DC ground
```

At point 1 there is a bias/dc-offset voltage of half the VCC (or more accurately, of 
whatever is the reference frequency of ADC; i.e. the 'max' ADC level). The C is typically
around 10uF or so.

This results in, point 2, in a shape, typically the same sine of the 50hz
households mains, that sweeps around that middle.

R-sense is picked such that at maximum current; the voltage over it is 80%
or so of the half the total ADC range.

So at maximum current; the ADC input varies between 10% and 90% of the ADC range.

At lower currents this will be lower.

If there is a concern about much higher peak currents; then consider adding a 
reverse diode to the ground and VCC; to `cap' any voltages that are (0.6 volt)
higher than either.

See https://learn.openenergymonitor.org/electricity-monitoring/ct-sensors/interface-with-arduino

## Use in code

See the examples for typical use. 

### reading

The simplest is:

    CurrentTransformerWithCallbacks sensor = CurrentTransformerWithCallbacks( GPIO_PIN );
    
    double factor = 3.12; // = Voltage x CoilRatio / VreferenceADC / RburdenResitor
    
    ...
    void loop() {
      ...
      Serial.print(sensor.sd() * factor);
 

### on/off call backs
If you are after on/off (this is what we use it for at the https://makerspaceleiden.nl -- to see of machines are on or off; then it may be easier to use callbacks:

    CurrentTransformerWithCallbacks sensor = CurrentTransformerWithCallbacks( GPIO_PIN );
   
    void setup() {
       ....
       sensor.onCurrentOn([]() {
            Serial.println("Its on !");
       });
       sensor.onCurrentOff([]() {
            Serial.println("Its off!t");
       });
    }

    void loop() {
          // nothing to do - all done in callbacks.
    }

### get called when state changed

Or alternatively ask a whole lot more:

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

by registering for a change callback.


