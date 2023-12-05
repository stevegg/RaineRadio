#include <Arduino.h>
#include "RadioStateMachine.h"

RadioStateMachine::RadioStateMachine()
{
   this->onTransitionCallback = NULL;
   this->stateList = new LinkedList<RadioState *>();
}

RadioStateMachine::~RadioStateMachine()
{
}

/*
 * Main execution of the machine occurs here in run
 * The current state is executed and it's transitions are evaluated
 * to determine the next state.
 *
 * By design, only one state is executed in one loop() cycle.
 */
void RadioStateMachine::run()
{
   // Serial.println("StateMachine::run()");
   //  Early exit, no states are defined
   if (stateList->size() == 0)
      return;

   // Initial condition
   if (currentState == -1)
   {
      currentState = 0;
   }

   // Execute state logic and return transitioned
   // to state number.
   int next = stateList->get(currentState)->execute();
   if (currentState != next)
   {
      Serial.println("Transitioning to state: " + String(next));
      if (this->onTransitionCallback != NULL)
      {
         Serial.println("Calling transition callback");
         (this->onTransitionCallback)(next);
      }
   }
   executeOnce = (currentState == next) ? false : true;
   currentState = next;
}

/*
 * Adds a state to the machine
 * It adds the state in sequential order.
 */
RadioState *RadioStateMachine::addState(void (*functionPointer)())
{
   RadioState *s = new RadioState();
   s->stateLogic = functionPointer;
   this->stateList->add(s);
   s->index = stateList->size() - 1;
   return s;
}

RadioState *RadioStateMachine::transitionTo(RadioState *s)
{
   if (this->onTransitionCallback != NULL)
   {
      Serial.println("Calling transition callback");
      (this->onTransitionCallback)(s->index);
   }
   this->currentState = s->index;
   this->executeOnce = true;
   return s;
}

int RadioStateMachine::transitionTo(int i)
{
   if (this->onTransitionCallback != NULL)
   {
      Serial.println("Calling transition callback");
      (this->onTransitionCallback)(i);
   }
   if (i < stateList->size())
   {
      this->currentState = i;
      this->executeOnce = true;
      return i;
   }
   return currentState;
}

void RadioStateMachine::setTransitionCallback(void (*functionPointer)(int newState))
{
   this->onTransitionCallback = functionPointer;
}
