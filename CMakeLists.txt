
cmake_minimum_required (VERSION 2.6)
project (vadSplit)

include_directories("${PROJECT_SOURCE_DIR}")

if(WIN32)
    message(STATUS ">>> config win32")
    add_definitions( -DWEBRTC_WIN )
else()
    message(STATUS ">>> config posix")
    add_definitions( -DWEBRTC_POSIX )
endif()

set( C_SRC_PREFIX "webrtc/common_audio")
set( C_SRC_RTC_PREFIX "webrtc/rtc_base")
FILE(GLOB sources vadSplit.cpp
            "${C_SRC_PREFIX}/signal_processing/*.c"
            "${C_SRC_PREFIX}/third_party/*.c"
            "${C_SRC_PREFIX}/vad/*.c"
            "${C_SRC_RTC_PREFIX}/checks.cc")

add_library(vadSplit ${sources})

# add the executable
add_executable(testVadSplit test/main.cpp vadSplit.cpp)

target_link_libraries(testVadSplit vadSplit)
