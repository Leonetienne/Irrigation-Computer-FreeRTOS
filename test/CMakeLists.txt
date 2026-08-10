cmake_minimum_required(VERSION 3.16)
project(irrigation_computer_tests CXX)

include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.13.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(irrigation_computer_tests
        GpioPinRegister.cpp
        WifiManagerStub.cpp
        GpioStub.cpp
        TimeStub.cpp
        NVSStub.cpp
        GpioDigitalWritePin.cpp
        Valve.cpp
        ValveGroup.cpp
        ApiRouteParser.cpp
        ApiController.cpp
        WifiCredentialsParser.cpp
        StateMachine.cpp
        SettingsManager.cpp
        UrlEncodedForm.cpp
        System.cpp
        MqttTopics.cpp
        MqttStub.cpp
        MqttSync.cpp

        ../main/src/platform/GpioDigitalWritePin.cpp
        ../main/src/GpioPinRegister.cpp
        ../main/src/test/stubs/GpioStub.cpp
        ../main/src/Valve.cpp
        ../main/include/Valve.h
        ../main/include/hal/ITime.h
        ../main/src/test/stubs/TimeStub.cpp
        ../main/include/test/stubs/TimeStub.h
        ../main/src/ValveGroup.cpp
        ../main/include/ValveGroup.h
        ../main/src/platform/TimeEsp32.cpp
        ../main/include/platform/TimeEsp32.h
        ../main/include/hal/IWifiManager.h
        ../main/src/test/stubs/WifiManagerStub.cpp
        ../main/include/test/stubs/WifiManagerStub.h
        ../main/include/hal/INVS.h
        ../main/src/test/stubs/NVSStub.cpp
        ../main/include/test/stubs/NVSStub.h
        ../main/include/enum/ValveAction.h
        ../main/include/ApiRouteParser.h
        ../main/src/ApiRouteParser.cpp
        ../main/include/ApiController.h
        ../main/src/ApiController.cpp
        ../main/include/WifiCredentialsParser.h
        ../main/src/WifiCredentialsParser.cpp
        ../main/include/WifiCredentials.h
        ../main/include/StateMachine.h
        ../main/src/StateMachine.cpp
        ../main/include/enum/States.h
        ../main/include/SettingsManager.h
        ../main/src/SettingsManager.cpp
        ../main/include/UrlEncodedForm.h
        ../main/src/UrlEncodedForm.cpp
        ../main/include/hal/IHttpServer.h
        ../main/include/test/stubs/HttpServerStub.h
        ../main/src/test/stubs/HttpServerStub.cpp
        ../main/include/test/stubs/SystemStub.h
        ../main/src/test/stubs/SystemStub.cpp
        ../main/include/System.h
        ../main/src/System.cpp
        ../main/include/enum/MqttConnectionState.h
        ../main/include/MqttConnectOptions.h
        ../main/include/MqttBrokerConfig.h
        ../main/include/hal/IMqtt.h
        ../main/include/MqttTopics.h
        ../main/src/MqttTopics.cpp
        ../main/include/test/stubs/MqttStub.h
        ../main/src/test/stubs/MqttStub.cpp
        ../main/include/MqttSync.h
        ../main/src/MqttSync.cpp
        WifiManagerStub.cpp
)


target_link_libraries(irrigation_computer_tests PRIVATE Catch2::Catch2WithMain)

target_include_directories(irrigation_computer_tests PRIVATE
        ../main/include
)

target_compile_definitions(irrigation_computer_tests PRIVATE HOST_BUILD=1)
target_compile_features(irrigation_computer_tests PRIVATE cxx_std_23)

target_compile_options(irrigation_computer_tests PRIVATE
        -Wall
        -Wextra
        -Werror
        -Wno-unused
)

include(CTest)
include(${catch2_SOURCE_DIR}/extras/Catch.cmake)
catch_discover_tests(irrigation_computer_tests)
