#ifndef __RADIOSTATEMACHINE_H__
#define __RADIOSTATEMACHINE_H__

#include <LinkedList.h>
#include "RadioState.h"

class RadioStateMachine
{
public:
   RadioStateMachine();
   ~RadioStateMachine();

   void init();
   void run();

   // When a stated is added we pass the function that represents
   // that state logic
   RadioState *addState(void (*functionPointer)());
   RadioState *transitionTo(RadioState *s);
   int transitionTo(int i);

   void setTransitionCallback(void (*functionPointer)(int newState));

   // Attributes
   LinkedList<RadioState *> *stateList;
   bool executeOnce = true; // Indicates that a transition to a different state has occurred
   int currentState = -1;   // Indicates the current state number

private:
   void (*onTransitionCallback)(int newState);
};

#endif