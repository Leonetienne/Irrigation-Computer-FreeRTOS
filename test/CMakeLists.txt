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
        GpioStub.cpp
        TimeStub.cpp
        GpioDigitalWritePin.cpp
        Valve.cpp
        ValveGroup.cpp

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
