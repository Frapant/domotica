# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD ilabs_challenger_rp2350_wifi_ble CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(ATlib C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(ATlib main.c 
                    esp_at.c 
                    esp_mqtt.c 
                    esp_wifi.c
                    MQTT.c)
                    

pico_set_program_name(ATlib "ATlib")
pico_set_program_version(ATlib "0.1")

# Modify the below lines to enable/disable output over UART/USB
# No stdio (no UART debug, no USB stdio)
pico_enable_stdio_uart(ATlib 1)
pico_enable_stdio_usb(ATlib 0)

target_link_libraries(ATlib
        pico_stdlib
        hardware_uart
        hardware_gpio
        hardware_irq
)

target_include_directories(ATlib PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(ATlib)
