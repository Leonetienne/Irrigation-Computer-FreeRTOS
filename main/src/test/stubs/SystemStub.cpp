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
    nvs.begin("system");
    static GpioPinRegister gpioPinRegister;
    static SettingsManager settings(nvs);

    // resolved here since wifiMan/mqtt take their indicator pin as a ctor param
    const bool connLedsEnabled = settings.retrieveConnLedsEnabled().value_or(true);
    const gpio_num_t wifiLedPin = connLedsEnabled ? settings.retrieveWifiLedGpioPin().value_or(GPIO_NUM_NC) : GPIO_NUM_NC;
    const gpio_num_t mqttLedPin = connLedsEnabled ? settings.retrieveMqttLedGpioPin().value_or(GPIO_NUM_NC) : GPIO_NUM_NC;

    static WifiManagerStub wifiMan(wifiLedPin, gpio, gpioPinRegister, time);
    static MqttStub mqtt(mqttLedPin, gpio, gpioPinRegister, time);
    static StateMachine stateMachine;
    static ValveGroup valveGroup(time, settings);
    static HttpServerStub httpServer;
    static MqttSync mqttSync(mqtt, valveGroup, settings);
    static System system(
        stateMachine, gpioPinRegister, gpio, time, nvs, settings, wifiMan, valveGroup, httpServer, mqttSync
    );
    return system;
}
