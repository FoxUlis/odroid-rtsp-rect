cmake_minimum_required(VERSION 3.10)

project(bouncecast VERSION 0.1.0 LANGUAGES CXX)

# =========================
# C++ Standard
# =========================
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =========================
# Dependencies
# =========================
find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

pkg_check_modules(GST REQUIRED
    gstreamer-1.0
    gstreamer-app-1.0
    gstreamer-rtsp-server-1.0
)

# =========================
# Sources
# =========================
set(SOURCES
    src/main.cpp
    src/camera_capture.cpp
    src/rectangle.cpp
    src/stream_encoder.cpp
    src/rtsp_server.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

# =========================
# Include Directories
# =========================
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${OpenCV_INCLUDE_DIRS}
        ${GST_INCLUDE_DIRS}
)

# =========================
# Link Libraries
# =========================
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${OpenCV_LIBS}
        ${GST_LIBRARIES}
        Threads::Threads
)

# =========================
# Compile Options
# =========================
target_compile_options(${PROJECT_NAME}
    PRIVATE
        -Wall
        -Wextra
        -O2
)
