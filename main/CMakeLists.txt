idf_component_register(SRCS
                       "src/main.cpp"
                       "src/StateMachine.cpp"
                       "src/System.cpp"
                       "src/GpioPinRegister.cpp"
                       "src/platform/TimeEsp32.cpp"
                       "src/platform/GpioEsp32.cpp"
                       "src/platform/WifiManagerEsp32.cpp"
                       "src/platform/NVSEsp32.cpp"
                       "src/platform/HttpServerEsp32.cpp"
                       "src/platform/GpioDigitalWritePin.cpp"
                       "src/platform/SystemEsp32.cpp"
                       "src/Valve.cpp"
                       "src/ValveGroup.cpp"
                       "src/ApiRouteParser.cpp"
                       "src/ApiController.cpp"
                       "src/SettingsManager.cpp"
                       "src/WifiCredentialsParser.cpp"
                       "src/UrlEncodedForm.cpp"
                       PRIV_REQUIRES spi_flash
                       REQUIRES esp_driver_gpio esp_wifi esp_event esp_netif nvs_flash esp_http_server esp_hw_support esp_system
                       INCLUDE_DIRS "include"
                       EMBED_FILES "web/index.html" "web/style.css" "web/settings.html" "web/advanced.html")

target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++23)
