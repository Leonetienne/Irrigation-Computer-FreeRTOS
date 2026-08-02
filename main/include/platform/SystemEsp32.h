#ifndef IRRIGATION_COMPUTER_SYSTEMESP32_H
#define IRRIGATION_COMPUTER_SYSTEMESP32_H

#include "System.h"

/**
 * Wires up a System instance against the esp32 platform implementation of every hal interface.
 * @return The esp32-backed System instance
 */
System& getSystem() noexcept;

#endif //IRRIGATION_COMPUTER_SYSTEMESP32_H
