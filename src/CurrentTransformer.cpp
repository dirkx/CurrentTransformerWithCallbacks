#include <CurrentTransformer.h>
#include <Arduino.h>
#include <ACNode.h>

#define BIAS_N	(1) /*seconds*/	// calculate the DC offset as a rolling window over this many seconds.

class CurrentTransformer;

// glue logic - as Tickers cannot call methods; but only normal functions. So
// we keep a list of all current cloils; and sample them on a ticker (the ESP32
// SDK does not allow us to hook the ADC to a timer; and hook AD conversion
// completion to a IRW a.f.a.i.k.
//
static std::list<CurrentTransformer> _currentCoils;
static Ticker _currentTicker = Ticker();
static Ticker _currentTicker2;

CurrentTransformer::CurrentTransformer(uint8_t pin, uint16_t sampleFrequency ) 
	: _pin(pin),  _hz(sampleFrequency)
{
	// na.
}

CurrentTransformer::~CurrentTransformer() {
	// na
}

void CurrentTransformer::begin() 
{
     if (_currentCoils.size() == 0) {
        _currentTicker.attach_ms(1000 / _hz /* mSecond */, [](){  
		for (CurrentTransformer & c : _currentCoils) 
			c.sample(); 
	});
     	Log.println("Ticker() callback configured.");
     };

     pinMode(_pin, INPUT); // analog input.
     _currentCoils.insert(_currentCoils.end(), *this);

     _limit = 0.125;
      _n = 0;
     _interval = 0;
     Log.println("Ticker() started.");
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
     uint16_t val,hz0,hz1,dif;

     val = analogRead(_pin);
     hz0 = (_n < _hz) ? _n : _hz;
     hz1 = hz0 + 1;
     dif = (_avg - val);

     _avg = ((double) _n * _avg + (double)val      ) / ((double)_n + 1.);
     _sd  = (hz0 * _sd + dif       ) / hz1;

     // if sd2 is 32 bits; and the ADC 10 bits — then we should stay within 32-2x10 - a 
    // few 100 samples/second I guess.
     _sd2 = (hz0 * _sd2 + dif * dif) / hz1; 

      if (_n < (BIAS_N * _hz))
     	_n++;

     _interval++; 
     if (_interval <= _hz / 5) 
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
     // Multiply by  V x RatioCurrentTransformer / VrefADC / RsenseResitor for actual Ampere RMS.
     return sqrt(fabs((float)_sd2 - (float)_sd * (float)_sd) / 1024.);
};

float CurrentTransformer::avg() {
	return ((float) _avg) / 1024.;
};

bool CurrentTransformer::hasCurrent() {
     return sd() > _limit;
};

