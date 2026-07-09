#include "../../include/platform/TimeEsp32.h"

time_t TimeEsp32::getTime() const noexcept {
    return time(nullptr);
}

int TimeEsp32::getSecondsSince(const time_t& since) const noexcept {
    return static_cast<int>(difftime(getTime(), since));
}
