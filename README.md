Rquires a circut such as
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

