cmake_minimum_required(VERSION 3.0)

project(MinimalReStreamer)

add_subdirectory(WebRTSP)

file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    *.cpp
    *.h
    *.conf.sample
    README.md
    *.sh
    *.cmake)

file(GLOB_RECURSE SNAP RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    snap/*)

file(GLOB_RECURSE WWW RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    www/[^.]*)

add_executable(${PROJECT_NAME} ${SOURCES} ${SNAP} ${WWW})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/WebRTSP)
target_link_libraries(${PROJECT_NAME}
    Http
    RtspParser
    Signalling
    RtStreaming)
