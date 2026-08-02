#ifndef IRRIGATION_COMPUTER_SYSTEMSTUB_H
#define IRRIGATION_COMPUTER_SYSTEMSTUB_H

#include "System.h"

/**
 * Wires up a System instance against host-side test stubs of every hal
 * interface.
 * @return The stub-backed System instance
 */
System& getSystem() noexcept;

#endif //IRRIGATION_COMPUTER_SYSTEMSTUB_H
