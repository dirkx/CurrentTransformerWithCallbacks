#include <CurrentTransformerWithCallbacks.h>
#include <Arduino.h>
#include <ACNode.h>

#define BIAS_N	(3) /*seconds*/	// calculate the DC offset as a rolling window over this many seconds.

class CurrentTransformerWithCallbacks;

// Glue logic (ESP32's ticker does not yet have TArgs). See
// https://github.com/esp8266/Arduino/pull/5030 for what 
// is needed.
//
static void IRAM_ATTR _sample(uint32_t arg) {
	CurrentTransformerWithCallbacks * c = (CurrentTransformerWithCallbacks*)arg;
	c->sample();
};

CurrentTransformerWithCallbacks::CurrentTransformerWithCallbacks(uint8_t pin, uint16_t sampleFrequency ) 
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

CurrentTransformerWithCallbacks::~CurrentTransformerWithCallbacks() {
	// na
}

CurrentTransformerWithCallbacks& CurrentTransformerWithCallbacks::onCurrentOn(THandlerFunction_Callback fn) {
     _callbackOn = fn;
     return *this;
};

CurrentTransformerWithCallbacks& CurrentTransformerWithCallbacks::onCurrentOff(THandlerFunction_Callback fn) {
     _callbackOff = fn;
     return *this;
};

CurrentTransformerWithCallbacks& CurrentTransformerWithCallbacks::onCurrentChange(THandlerFunction_CallbackWithState fn) {
     _callback = fn;
     return *this;
};

void CurrentTransformerWithCallbacks::setOnLimit(float limit) {
	_limit = limit;
   };

float CurrentTransformerWithCallbacks::onLimit(float limit) {
	return _limit;
   };

// Keep this routine as short and concise as posible; as we run on
// an interrupt/timer.
//
void IRAM_ATTR CurrentTransformerWithCallbacks::sample() {
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

     notInState++;
}

void CurrentTransformerWithCallbacks::loop() {
     if (notInState <  3)
       return;
     notInState = 0;

     state = hasCurrent();

     lastStateChange = millis();

     if (_callback)
       _callback(state ? ON : OFF);

     if (state && _callbackOn)
       _callbackOn();

     if (!state && _callbackOff)
       _callbackOff();
};

float CurrentTransformerWithCallbacks::sd() {
     return sqrt(fabs((float)_sd2 - (float)_sd * (float)_sd)) / _bitsDiv / _bitsDiv * _refV;
};

float CurrentTransformerWithCallbacks::avg() {
	return _refV * ((float) _avg) / _bitsDiv;
};

bool CurrentTransformerWithCallbacks::hasCurrent() {
     return sd() > _limit;
};

