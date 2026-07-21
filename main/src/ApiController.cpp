#include "ApiController.h"

bool ApiController::executeValveOperation(ValveGroup& valveGroup, const ValveCommand& command) noexcept {
    switch (command.action) {
        case ValveAction::Open:
            return valveGroup.open(command.valveIndex);

        case ValveAction::Close:
            return valveGroup.close(command.valveIndex);
    }

    return false;
}
