//
// Created by Agent on 08.08.26.
//

#ifndef IRRIGATION_COMPUTER_TESTS_TIMESTUB_H
#define IRRIGATION_COMPUTER_TESTS_TIMESTUB_H

#include "hal/ITime.h"

class TimeStub : public ITime {
public:
    TimeStub() = default;
    // Since this is purely a non-static-class to allow for DI and does not hold internal state, we're fine allowing copies.
    TimeStub(const TimeStub&) = default;
    TimeStub(TimeStub&&) = default;

    /**
     * @return Current calendar time in seconds since epoch
     */
    [[nodiscard]] time_t getTime() const noexcept override;

    /**
    * @param since The time reference to measure the time distance towards
    * @return How many seconds have elapsed since reference
    */
    [[nodiscard]] int getSecondsSince(const time_t &since) const noexcept override;

    /**
     * Testing stub method: set the current time reported to callers
     */
    void setStubbedTime(time_t stubbedTime) noexcept;

private:
    time_t stubbedCurrentTime = 1700000000; // Tu 14. Nov 23:13:20 CET 2023
};


#endif //IRRIGATION_COMPUTER_TESTS_TIMESTUB_H
