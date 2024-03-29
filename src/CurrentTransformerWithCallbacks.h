#ifndef _H_CURRENT_COUL
#define _H_CURRENT_COUL

#include <Ticker.h>
  
#include <algorithm>
#include <list>
#include <functional>

class CurrentTransformerWithCallbacks {
 public:
   CurrentTransformerWithCallbacks(uint8_t pin, uint16_t sampleFrequency = 200 );
   ~CurrentTransformerWithCallbacks();

   typedef std::function<void()> THandlerFunction_Callback;
   CurrentTransformerWithCallbacks& onCurrentOn(THandlerFunction_Callback fn);
   CurrentTransformerWithCallbacks& onCurrentOff(THandlerFunction_Callback fn);

   typedef enum { OFF, ON, UNKNOWN } state_t;
   typedef std::function<void(state_t state)> THandlerFunction_CallbackWithState;
   CurrentTransformerWithCallbacks& onCurrentChange(THandlerFunction_CallbackWithState fn);

   void setOnLimit(float limit);
   float onLimit(float limit);

   void loop();
   float sd();
   float avg();
   bool hasCurrent();

   void sample();
 private:
   Ticker * _currentTicker;
   uint8_t _pin;
   uint16_t _hz,  _interval;
   uint32_t _avg, _sd, _sd2;
   int _n;
   bool state;
   uint8_t notInState;
   unsigned long lastStateChange;
   float _limit, _refV;
   uint32_t _bitsDiv;

   THandlerFunction_Callback _callbackOn,  _callbackOff;

   THandlerFunction_CallbackWithState _callback;
};
#endif
