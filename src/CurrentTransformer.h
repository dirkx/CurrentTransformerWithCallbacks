#ifndef _H_CURRENT_COUL
#define _H_CURRENT_COUL

#include <Ticker.h>
  
#include <algorithm>
#include <list>
#include <functional>

class CurrentTransformer {
 public:
   CurrentTransformer(uint8_t pin, uint16_t sampleFrequency = 200 );

   typedef std::function<void()> THandlerFunction_Callback;
   CurrentTransformer& onCurrentOn(THandlerFunction_Callback fn);
   CurrentTransformer& onCurrentOff(THandlerFunction_Callback fn);

   typedef enum { OFF, ON, UNKNOWN } state_t;
   typedef std::function<void(state_t state)> THandlerFunction_CallbackWithState;
   CurrentTransformer& onCurrentChange(THandlerFunction_CallbackWithState fn);

   void setOnLimit(float limit);
   float onLimit(float limit);

   float sd();
   bool hasCurrent();

   void sample();
   void callbackCheck();

 private:
   uint8_t _pin;
   uint16_t _hz;
   uint32_t _avg, _sd, _sd2;
   bool state;
   uint8_t notInState;
   unsigned long lastStateChange;
   float _limit;

   THandlerFunction_Callback _callbackOn,  _callbackOff;

   THandlerFunction_CallbackWithState _callback;
};
#endif