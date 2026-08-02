#ifndef IRRIGATION_COMPUTER_TESTS_VALVEGROUP_H
#define IRRIGATION_COMPUTER_TESTS_VALVEGROUP_H

#include <array>
#include <expected>
#include <optional>
#include "Valve.h"
#include "SettingsManager.h"
#include "hal/ITime.h"

/**
 * Aggregation of multiple valves
 */
class ValveGroup {
public:
    ValveGroup(const ITime& i_time, const SettingsManager& settings) noexcept;
    ValveGroup(const ValveGroup &) = delete;
    ValveGroup(ValveGroup &&) noexcept;
    ValveGroup& operator=(ValveGroup&& other) noexcept;
    ~ValveGroup() noexcept;

    /**
     * Will initialize all valves with a valid gpio pin
     * Time complexity: O(n)
     * @return Success state
     */
    bool initialize(std::array<Valve, 8> newValves) noexcept;

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

    /**
     * Gets the open state of a valve
     * @param index Valve index
     * @return Valve state (or success state)
     */
    [[nodiscard]] std::expected<bool, bool> getValveOpenState(std::size_t index) const noexcept;

    /**
     * @param index Valve index
     * @return Whether that index exists and has a gpio pin configured, i.e. can be opened/closed
     */
    [[nodiscard]] bool isValveOperable(std::size_t index) const noexcept;

private:
    bool isInitialized = false;
    // Optional to allow empty initialization in ctor, and create after reading NVS
    std::optional<std::array<Valve, 8>> valves;
    const ITime& i_time;
    const SettingsManager& settings;
    bool settings_doValvesTimeout = true;
    int32_t settings_valveTimeoutSeconds = 3600;
};


#endif //IRRIGATION_COMPUTER_TESTS_VALVEGROUP_H
