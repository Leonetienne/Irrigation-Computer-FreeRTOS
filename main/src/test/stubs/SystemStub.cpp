#include "test/stubs/SystemStub.h"
#include "test/stubs/GpioStub.h"
#include "test/stubs/TimeStub.h"
#include "test/stubs/NVSStub.h"
#include "test/stubs/WifiManagerStub.h"
#include "test/stubs/HttpServerStub.h"
#include "test/stubs/MqttStub.h"
#include "GpioPinRegister.h"
#include "StateMachine.h"
#include "SettingsManager.h"
#include "ValveGroup.h"
#include "MqttSync.h"

System& getSystem() noexcept {
    static GpioStub gpio;
    static TimeStub time;
    static NVSStub nvs;
    static WifiManagerStub wifiMan;
    static MqttStub mqtt;
    static GpioPinRegister gpioPinRegister;
    static StateMachine stateMachine;
    static SettingsManager settings(nvs);
    static ValveGroup valveGroup(time, settings);
    static HttpServerStub httpServer;
    static MqttSync mqttSync(mqtt, valveGroup, settings);
    static System system(
        stateMachine, gpioPinRegister, gpio, time, nvs, settings, wifiMan, valveGroup, httpServer, mqttSync
    );
    return system;
}
