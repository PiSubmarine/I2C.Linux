include("${PISUBMARINE_BUILD_SYSTEM}")

message("Adding library: ${PISUBMARINE_MODULE_NAME}")
add_library(${PISUBMARINE_MODULE_NAME}
        PiSubmarine/I2C/Linux/Driver.cpp)
target_include_directories(${PISUBMARINE_MODULE_NAME} PUBLIC "../public")
PiSubmarineInitTarget(${PISUBMARINE_MODULE_NAME})

PiSubmarineAddDependency("https://github.com/PiSubmarine/I2C.Api.git" "")
target_link_libraries(${PISUBMARINE_MODULE_NAME} PUBLIC PiSubmarine.I2C.Api)

PiSubmarineAddDependency("https://github.com/PiSubmarine/Error.Api.git" "")
target_link_libraries(${PISUBMARINE_MODULE_NAME} PUBLIC PiSubmarine.Error.Api)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${PISUBMARINE_MODULE_NAME} PUBLIC spdlog::spdlog)
