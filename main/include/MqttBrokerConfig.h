#ifndef IRRIGATION_COMPUTER_MQTTBROKERCONFIG_H
#define IRRIGATION_COMPUTER_MQTTBROKERCONFIG_H

#include <string>

struct MqttBrokerConfig {
    std::string uri;
    std::string username;
    std::string password;
};

#endif //IRRIGATION_COMPUTER_MQTTBROKERCONFIG_H
