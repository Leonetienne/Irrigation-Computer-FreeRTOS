//
// Created by Leon Etienne on 18.03.26.
//

#ifndef IRRIGATION_COMPUTER_STATEMACHINE_H
#define IRRIGATION_COMPUTER_STATEMACHINE_H

#include "States.h"

class StateMachine {
public:
    StateMachine() noexcept;
    StateMachine(const StateMachine&) = delete;
    StateMachine(StateMachine&&) = delete;
    StateMachine& operator=(const StateMachine&) = delete;

private:
    STATE currentState;
};

#endif //IRRIGATION_COMPUTER_STATEMACHINE_H
