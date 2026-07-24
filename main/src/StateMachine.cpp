#include "StateMachine.h"

StateMachine::StateMachine() noexcept :
    currentState { STATE::INITIALIZATION }
{
}

STATE StateMachine::getState() const noexcept {
    return currentState;
}

void StateMachine::setState(STATE state) noexcept {
    currentState = state;
}
