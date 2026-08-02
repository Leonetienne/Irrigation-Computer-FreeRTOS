#ifndef IRRIGATION_COMPUTER_VALVEOPERATIONRESULT_H
#define IRRIGATION_COMPUTER_VALVEOPERATIONRESULT_H

/**
 * Outcome of a requested valve open/close operation
 */
enum class ValveOperationResult {
    Success,
    InvalidRequest, // valve index out of range, or that valve has no gpio configured
    HardwareFailure // valve exists and is configured, but the actual gpio write failed
};

#endif //IRRIGATION_COMPUTER_VALVEOPERATIONRESULT_H
