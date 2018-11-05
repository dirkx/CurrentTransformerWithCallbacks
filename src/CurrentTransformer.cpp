#include <CurrentTransformer.h>
#include <Arduino.h>

#define BIAS_N	(100) /*seconds*/	// calculate the DC offset as a rolling window over this many seconds.

class CurrentTransformer;

// glue logic - as Tickers cannot call methods; but only normal functions. So
// we keep a list of all current cloils; and sample them on a ticker (the ESP32
// SDK does not allow us to hook the ADC to a timer; and hook AD conversion
// completion to a IRW a.f.a.i.k.
//
static std::list<CurrentTransformer> _currentCoils;


// Fast ticker - to just measure.
//
static Ticker _currentTicker = Ticker();

static void currentSample() {
 for (CurrentTransformer c : _currentCoils)    
	c.sample();
};

// Slow ticker - to see if we need to
// warn main-land about a change.
//
static Ticker _currentCallbackTicker = Ticker();
static void currentCallbackCheck() {
 for (CurrentTransformer c : _currentCoils)    
	c.callbackCheck();
};


CurrentTransformer::CurrentTransformer(uint8_t pin, uint16_t sampleFrequency ) : _pin(pin),  _hz(sampleFrequency)
   {
     if (_currentCoils.size() == 0) {
       _currentTicker.attach(1000 / _hz /* mSecond */, currentSample);
       _currentCallbackTicker.attach(200, currentCallbackCheck);
     };

     _currentCoils.insert(_currentCoils.end(), *this);
     _limit = 5.f;
   };

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
     uint16_t val = analogRead(_pin);
     uint16_t hz1 = _hz + 1;
     uint16_t dif = (_avg - val);

     _avg = (BIAS_N * _hz * _avg + val      ) / (_hz * BIAS_N + 1);
     _sd  = (_hz * _sd + dif       ) / hz1;
     // if sd2 is 32 bits; and the ADC 10 bits — then we should stay within 32-2x10 - a few 100 samples/second I guess.
     _sd2 = (_hz * _sd2 + dif * dif) / hz1; 
   };

   float CurrentTransformer::sd() {
     // Multiply by  V x RatioCurrentTransformer / VrefADC / RsenseResitor for actual Ampere RMS.
     return sqrt(fabs(_sd2 - _sd * _sd) / 1024.);
   };

bool CurrentTransformer::hasCurrent() {
     return _limit < sd()/1024.;
   };

void CurrentTransformer::callbackCheck() {
     bool nstate = hasCurrent();

     if (nstate == state) 
	return;

     // Sort of expect the state change to happen within 3 seconds; otherwise we abort
     //
     if (millis() - lastStateChange > 3000) {
         lastStateChange = millis();
         notInState = 0;
     };

     notInState ++;

     if (notInState <  10)
       return;

     state = nstate;
     lastStateChange = millis();

     if (_callback)
       _callback(nstate ? ON : OFF);

     if (nstate && _callbackOn)
       _callbackOn();

     if (!nstate && _callbackOff)
       _callbackOff();
};

