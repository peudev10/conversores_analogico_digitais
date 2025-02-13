# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configure PICO_SDK_PATH
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Import SDK
include(pico_sdk_import.cmake)
project(conversoresAD C CXX ASM)
pico_sdk_init()

# Add executable first
add_executable(conversoresAD conversoresAD.c inc/ssd1306.c)

# Then generate PIO header
pico_generate_pio_header(conversoresAD ${CMAKE_CURRENT_LIST_DIR}/ws2818b.pio)

# Set program properties
pico_set_program_name(conversoresAD "conversoresAD")
pico_set_program_version(conversoresAD "0.1")

# Configure stdio
pico_enable_stdio_uart(conversoresAD 0)
pico_enable_stdio_usb(conversoresAD 1)

# Add include directories
target_include_directories(conversoresAD PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Link libraries
target_link_libraries(conversoresAD
    pico_stdlib
    hardware_pio
    hardware_clocks
    hardware_i2c
    hardware_pwm
    hardware_adc
)

# Add extra outputs
pico_add_extra_outputs(conversoresAD)

