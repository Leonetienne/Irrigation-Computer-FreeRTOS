//
// Created by Leon Etienne on 18.03.26.
//

#ifndef IRRIGATION_COMPUTER_STATES_H
#define IRRIGATION_COMPUTER_STATES_H

#include <cstdint>

enum class STATE : uint8_t {
    INITIALIZATION,  // System is starting up
    WIFI_ONBOARDING, // System has launched a wifi access point for the user to configure proper wifi credentials
    WAIT_WIFI_CONNECTION,
    OPERATIONAL,     // System is fine and operating normally
    FAULT,           // The system has entered an invalid state.
    SHUTTING_DOWN    // System is shutting down
};

#endif //IRRIGATION_COMPUTER_STATES_H
