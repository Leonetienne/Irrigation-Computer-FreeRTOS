idf_component_register(SRCS
                       "src/main.cpp"
                       "src/StateMachine.cpp"
                       "src/System.cpp"
                       "src/GpioPinRegister.cpp"
                       "src/platform/TimeEsp32.cpp"
                       "src/platform/GpioEsp32.cpp"
                       "src/platform/WifiManagerEsp32.cpp"
                       "src/platform/GpioDigitalWritePin.cpp"
                       PRIV_REQUIRES spi_flash
                       REQUIRES esp_driver_gpio esp_wifi esp_event esp_netif
                       INCLUDE_DIRS "include")

target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++23)
