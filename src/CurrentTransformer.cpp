#include <CurrentTransformer.h>
#include <Arduino.h>
#include <ACNode.h>

#define BIAS_N	(3) /*seconds*/	// calculate the DC offset as a rolling window over this many seconds.

class CurrentTransformer;

// Glue logic (ESP32's ticker does not yet have TArgs). See
// https://github.com/esp8266/Arduino/pull/5030 for what 
// is needed.
//
static void _sample(uint32_t arg) {
	CurrentTransformer * c = (CurrentTransformer*)arg;
	c->sample();
};

CurrentTransformer::CurrentTransformer(uint8_t pin, uint16_t sampleFrequency ) 
	: _pin(pin),  _hz(sampleFrequency)
{
     _currentTicker = new Ticker();
     _currentTicker->attach_ms(1000 / _hz /* mSecond */, _sample,(uint32_t) this);

     pinMode(_pin, INPUT); // analog input.
     _limit = 0.10;
     _refV = 1.0; 
     _bitsDiv = 1024;
     _avg = _n = _sd = _sd2 = _interval = 0;
}

CurrentTransformer::~CurrentTransformer() {
	// na
}

CurrentTransformer& CurrentTransformer::onCurrentOn(THandlerFunction_Callback fn) {
     _callbackOn = fn;
     return *this;
};

CurrentTransformer& CurrentTransformer::onCurrentOff(THandlerFunction_Callback fn) {
     _callbackOff = fn;
     return *this;
};

CurrentTransformer& CurrentTransformer::onCurrentChange(THandlerFunction_CallbackWithState fn) {
     _callback = fn;
     return *this;
};

void CurrentTransformer::setOnLimit(float limit) {
	_limit = limit;
   };

float CurrentTransformer::onLimit(float limit) {
	return _limit;
   };

// Keep this routine as short and concise as posible; as we run on
// an interrupt/timer.
//
void CurrentTransformer::sample() {
     uint16_t val,hz0,hz1,dif, N;

     N = BIAS_N * _hz; // averaging window.

     val = analogRead(_pin);
     hz0 = (_n < N) ? _n : N;
     hz1 = hz0 + 1;
     dif = fabs((val - _avg));

     // if sd2 is 32 bits; and the ADC 10 bits — then we should stay within 32-2x10 - a 
     // few 100 samples/second I guess.
     //
     _avg = (hz0 * _avg + val       ) / hz1;
     _sd  = (hz0 * _sd  + dif       ) / hz1;
     _sd2 = (hz0 * _sd2 + dif * dif ) / hz1; 

      if (_n < N) _n++;

     // Check for a change in state 5 times a second.
     if (_interval++ <= _hz / 5) 
	return;
     _interval = 0;

     bool nstate = hasCurrent();
     if (nstate == state) 
	return;

     // Sort of expect the state change to happen within 3 seconds; otherwise we abort
     //
     if (millis() - lastStateChange > 3000) {
         lastStateChange = millis();
         notInState = 0;
     };

     if (notInState++ <  3)
       return;

     state = nstate;
     notInState = 0;
     lastStateChange = millis();

     if (_callback)
       _callback(nstate ? ON : OFF);

     if (nstate && _callbackOn)
       _callbackOn();

     if (!nstate && _callbackOff)
       _callbackOff();
};

float CurrentTransformer::sd() {
     return sqrt(fabs((float)_sd2 - (float)_sd * (float)_sd)) / _bitsDiv / _bitsDiv * _refV;
};

float CurrentTransformer::avg() {
	return _refV * ((float) _avg) / _bitsDiv;
};

bool CurrentTransformer::hasCurrent() {
     return sd() > _limit;
};

