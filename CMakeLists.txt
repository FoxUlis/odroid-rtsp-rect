cmake_minimum_required(VERSION 3.10)
project(bouncecast)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# === НАХОДИМ ЗАВИСИМОСТИ ===
find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED)

pkg_check_modules(GST REQUIRED
    gstreamer-1.0
    gstreamer-app-1.0
    gstreamer-rtsp-server-1.0
)

# === ИСХОДНЫЕ ФАЙЛЫ ===
set(SOURCES
    src/main.cpp
    src/camera_capture.cpp
    src/rectangle.cpp
    src/stream_encoder.cpp
    src/rtsp_server.cpp
)

set(HEADERS
    src/camera_capture.h
    src/rectangle.h
    src/stream_encoder.h
    src/rtsp_server.h
)

# === СОЗДАЁМ EXECUTABLE ===
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# === ПОДКЛЮЧАЕМ ЗАГОЛОВКИ ===
target_include_directories(${PROJECT_NAME} PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${GST_INCLUDE_DIRS}
    src/
)

# === ЛИНКУЕМ БИБЛИОТЕКИ ===
target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    ${GST_LIBRARIES}
    gstapp-1.0
    gst-rtsp-server-1.0
)

# === КОПИРУЕМ БИНАРНИК В BUILD ===
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
