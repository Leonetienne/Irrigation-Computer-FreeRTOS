#ifndef IRRIGATION_COMPUTER_STATEMACHINE_H
#define IRRIGATION_COMPUTER_STATEMACHINE_H

#include "enum/States.h"

class StateMachine {
public:
    StateMachine() noexcept;
    StateMachine(const StateMachine&) = delete;
    StateMachine(StateMachine&&) = delete;
    StateMachine& operator=(const StateMachine&) = delete;

    /**
     * @return The current state
     */
    [[nodiscard]] STATE getState() const noexcept;

private:
    STATE currentState = STATE::INITIALIZATION;
};

#endif //IRRIGATION_COMPUTER_STATEMACHINE_H
