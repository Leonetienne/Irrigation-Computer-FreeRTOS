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
    static GpioPinRegister gpioPinRegister;
    static WifiManagerEsp32 wifiMan(gpio, gpioPinRegister, time);
    static MqttEsp32 mqtt;
    static StateMachine stateMachine;
    static SettingsManager settings(nvs);
    static ValveGroup valveGroup(time, settings);
    static HttpServerEsp32 httpServer(valveGroup, settings, stateMachine);
    static MqttSync mqttSync(mqtt, valveGroup, settings);
    static System system(
        stateMachine, gpioPinRegister, gpio, time, nvs, settings, wifiMan, valveGroup, httpServer, mqttSync
    );
    return system;
}
