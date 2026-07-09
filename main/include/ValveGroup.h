#ifndef IRRIGATION_COMPUTER_TESTS_VALVEGROUP_H
#define IRRIGATION_COMPUTER_TESTS_VALVEGROUP_H

#include <array>
#include "Valve.h"
#include "hal/ITime.h"

/**
 * Aggregation of multiple valves
 */
class ValveGroup {
public:
    ValveGroup(std::array<Valve, 8> valves, const ITime& i_time) noexcept;
    ValveGroup(const ValveGroup &) = delete;
    ValveGroup(ValveGroup &&) noexcept;
    ~ValveGroup() noexcept;

    /**
     * Will initialize all valves with a valid gpio pin
     * Time complexity: O(n)
     * @return Success state
     */
    bool initialize() noexcept;

    /**
     * Will release resources acquired by this valve group
     * Time complexity: O(n)
     * @return Success state
     */
    bool free() noexcept;

    /**
     * @return Whether this object is initialized
     */
    [[nodiscard]] bool isReady() const noexcept;

    /**
     * Will open a specific valve
     * @param index Valve index
     * @return Success state
     */
    bool open(std::size_t index) noexcept;

    /**
     * Will close a specific valve
     * @param index Valve index
     * @return Success state
     */
    bool close(std::size_t index) noexcept;

    /**
     * Will open or close a specific valve
     * @param index Valve index
     * @param openState true if it should be open
     * @return Success state
     */
    bool setOpenState(std::size_t index, bool openState) noexcept;

    /**
     * Will close any valves if their open time exceeds a time limit
     * Time complexity: O(n)
     *
     * @return Success state
     */
    bool autoCloseValvesAfterTimeoutPoll() noexcept;

private:
    bool isInitialized = false;
    std::array<Valve, 8> valves;
    const ITime& i_time;
};


#endif //IRRIGATION_COMPUTER_TESTS_VALVEGROUP_H
