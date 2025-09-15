cmake_minimum_required(VERSION 3.26)
set(PROJECT_NAME AudioRedirector)
project(${PROJECT_NAME} LANGUAGES CXX)

# set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Add executable target and source files
file(GLOB_RECURSE SOURCES src/*.cpp)
qt_add_resources(RESOURCES resources.qrc)

qt_add_executable(${PROJECT_NAME}
    MANUAL_FINALIZATION ${SOURCES} ${RESOURCES} resources.rc
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/miniaudio)

target_include_directories(${PROJECT_NAME} PRIVATE
    "src/Audio"
    "src/Utils"
    "src/Widgets"
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets miniaudio)

# finalizes the Qt 6 build by ensuring: Autogen, resources, and other Qt features are flushed.
qt_finalize_executable(${PROJECT_NAME}) # Only needed for Qt 6+; older versions don’t need this call.

# Disable console window for release build
target_link_options(${PROJECT_NAME} PRIVATE 
    $<$<CONFIG:Release>:/SUBSYSTEM:windows>
    $<$<CONFIG:Release>:/ENTRY:mainCRTStartup>
)

# Set the target output directory (where the executable is placed)
set(TARGET_BIN_DIR "${CMAKE_BINARY_DIR}${CMAKE_BUILD_TYPE}/$<CONFIG>")

get_target_property(QMAKE_PATH Qt6::qmake IMPORTED_LOCATION)
get_filename_component(QT_BIN_DIR "${QMAKE_PATH}" DIRECTORY)

# Deploy Qt dependencies with deploy_once.cmake
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -D CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}
        -D TARGET_BIN_DIR=${TARGET_BIN_DIR}
        -D QT_BIN_DIR=${QT_BIN_DIR}
        -D TARGET_FILE=$<TARGET_FILE:${PROJECT_NAME}>
        -P "${CMAKE_SOURCE_DIR}/deploy_once.cmake"
    COMMENT "Running deploy_once.cmake"
)
