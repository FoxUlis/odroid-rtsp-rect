cmake_minimum_required(VERSION 3.10)
project(bouncecast VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED
    gstreamer-1.0
    gstreamer-app-1.0
    gstreamer-rtsp-server-1.0
)

set(SOURCES
    src/main.cpp
    src/camera_capture.cpp
    src/rectangle.cpp
    src/stream_encoder.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${GST_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    ${GST_LIBRARIES}
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -O2
)
