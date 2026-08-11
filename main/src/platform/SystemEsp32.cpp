#include "platform/SystemEsp32.h"
#include "platform/GpioEsp32.h"
#include "platform/TimeEsp32.h"
#include "platform/NVSEsp32.h"
#include "platform/WifiManagerEsp32.h"
#include "platform/HttpServerEsp32.h"
#include "platform/MqttEsp32.h"
#include "GpioPinRegister.h"
#include "StateMachine.h"
#include "SettingsManager.h"
#include "ValveGroup.h"
#include "MqttSync.h"

System& getSystem() noexcept {
    static GpioEsp32 gpio;
    static TimeEsp32 time;
    static NVSEsp32 nvs;
    nvs.begin("system");
    static GpioPinRegister gpioPinRegister;
    static SettingsManager settings(nvs);

    // resolved here since wifiMan/mqtt take their indicator pin as a ctor param
    const bool connLedsEnabled = settings.retrieveConnLedsEnabled().value_or(true);
    const gpio_num_t wifiLedPin = connLedsEnabled ? settings.retrieveWifiLedGpioPin().value_or(GPIO_NUM_NC) : GPIO_NUM_NC;
    const gpio_num_t mqttLedPin = connLedsEnabled ? settings.retrieveMqttLedGpioPin().value_or(GPIO_NUM_NC) : GPIO_NUM_NC;

    static WifiManagerEsp32 wifiMan(wifiLedPin, gpio, gpioPinRegister, time);
    static MqttEsp32 mqtt(mqttLedPin, gpio, gpioPinRegister, time);
    static StateMachine stateMachine;
    static ValveGroup valveGroup(time, settings);
    static HttpServerEsp32 httpServer(valveGroup, settings, stateMachine);
    static MqttSync mqttSync(mqtt, valveGroup, settings);
    static System system(
        stateMachine, gpioPinRegister, gpio, time, nvs, settings, wifiMan, valveGroup, httpServer, mqttSync
    );
    return system;
}
