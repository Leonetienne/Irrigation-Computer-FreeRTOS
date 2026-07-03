//
// Created by Agent on 08.08.26.
//

#include "../../../include/test/stubs/TimeStub.h"

time_t TimeStub::getTime() const noexcept {
    return this->stubbedCurrentTime;
}

int TimeStub::getSecondsSince(const time_t& since) const noexcept {
    return static_cast<int>(stubbedCurrentTime - since);
}

void TimeStub::setStubbedTime(time_t stubbedTime) noexcept {
    this->stubbedCurrentTime = stubbedTime;
}
