#ifndef IRRIGATION_COMPUTER_TIME_T_H
#define IRRIGATION_COMPUTER_TIME_T_H

// If compiling for host, define time_t as int, otherwise include idf header
#ifdef HOST_BUILD
#include <time.h>
#else
#include "FreeRTOS_POSIX/time.h"
#endif

#endif //IRRIGATION_COMPUTER_TIME_T_H