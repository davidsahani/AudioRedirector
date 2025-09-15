if(NOT CMAKE_C_COMPILER)
    enable_language(C)
endif()

add_library(miniaudio STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/miniaudio.c
)

target_include_directories(miniaudio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# set a standalone version
set_target_properties(miniaudio PROPERTIES VERSION 0.11.22)
