#ifndef IRRIGATION_COMPUTER_TESTS_ITIME_H
#define IRRIGATION_COMPUTER_TESTS_ITIME_H

#include "compat/time_t.h"

/**
 * Abstract time interface
 */
class ITime {
public:
    virtual ~ITime() = default;

    /**
     * @return Current calendar time in seconds since epoch
     */
    [[nodiscard]] virtual time_t getTime() const noexcept = 0;

    /**
     * @param since The time reference to measure the time distance towards
     * @return How many seconds have elapsed since reference
     */
    [[nodiscard]] virtual int getSecondsSince(const time_t& since) const noexcept = 0;
};


#endif //IRRIGATION_COMPUTER_TESTS_ITIME_H
