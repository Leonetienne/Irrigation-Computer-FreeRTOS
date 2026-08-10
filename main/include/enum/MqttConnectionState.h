#ifndef IRRIGATION_COMPUTER_MQTTCONNECTIONSTATE_H
#define IRRIGATION_COMPUTER_MQTTCONNECTIONSTATE_H

enum class MqttConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Failed
};

#endif //IRRIGATION_COMPUTER_MQTTCONNECTIONSTATE_H
