RTSP-видеостриминг с анимированной графикой на C++

Проект демонстрирует захват видео с веб-камеры, отрисовку анимированного прямоугольника с физикой отскока и потоковую трансляцию через RTSP-сервер.

Функциональность

    ✅ Захват видео с веб-камеры через OpenCV
    ✅ Отрисовка прямоугольника (30×20 px), движущегося под углом 45°
    ✅ Физика отскока от границ кадра (угол падения = углу отражения)
    ✅ Кодирование потока в H.264 через GStreamer
    ✅ RTSP-сервер для потоковой передачи (gst-rtsp-server)
    ✅ Поддержка клиентов: ffplay, mpv, VLC


Структура проекта

```bash
├── build/                    # Директория сборки CMake
├── CMakeLists.txt           # Конфигурация сборки
├── README.md                # Документация
├── src/
│   ├── main.cpp             # Точка входа, главный цикл
│   ├── camera_capture.cpp/h # Захват камеры (OpenCV)
│   ├── rectangle.cpp/h      # Логика движущегося прямоугольника
│   ├── stream_encoder.cpp/h # Кодирование и отправка кадров
│   └── rtsp_server.cpp/h    # RTSP-сервер на gst-rtsp-server
└── test_output/             # Директория для тестовых файлов
```

Требования

```bash
# Ubuntu/Debian
sudo apt-get install \
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    libgstreamer-plugins-good1.0-dev \
    libgstreamer-plugins-bad1.0-dev \
    libgstreamer-rtsp-server-1.0-dev \
    libopencv-dev \
    gstreamer1.0-tools \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-plugins-ugly \
    cmake \
    build-essential
```

Клиенты для просмотра потока

ffplay (рекомендуется) - ffplay -rtsp_transport tcp rtsp://<IP>:8554/stream
mpv - mpv --rtsp-transport=tcp rtsp://<IP>:8554/stream
VLC - vlc :rtsp-transport=tcp rtsp://<IP>:8554/stream

⚠️ Важно: Сервер настроен на TCP-транспорт. Обязательно указывайте :rtsp-transport=tcp или -rtsp_transport tcp при подключении.

Сборка и запуск

1. Сборка проекта

```bash
mkdir -p build && cd build

cmake ..

make -j$(nproc)
```

2. Запуск

```bash
./bouncecast

# Вывод:
# === BOUNCECAST v0.5 ===
# Этап 5: RTSP Стриминг
# Камера открыта 640x480
# === RTSP СЕРВЕР ЗАПУЩЕН ===
# URL: rtsp://<IP-плата>:8554/stream
```

3. Подключение клиента

На другом устройстве в той же сети:

```bash
# ffplay (наиболее стабильный)
ffplay -rtsp_transport tcp rtsp://192.168.1.18:8554/stream

# mpv
mpv --rtsp-transport=tcp rtsp://192.168.1.18:8554/stream

# VLC (консольная версия)
cvlc :rtsp-transport=tcp rtsp://192.168.1.18:8554/stream
```

##Порт по умолчанию: 8554

#Диагностика и отладка

Проверка камеры:
```bash
ls -l /dev/video*

#Тест через GStreamer
gst-launch-1.0 v4l2src ! videoconvert ! autovideosink
```

Проверка плагинов GStreamer
```bash
gst-inspect-1.0 x264enc
gst-inspect-1.0 appsrc
gst-inspect-1.0 rtspclientsink
```

Проверка сети

```bash
# Слушает ли сервер порт?
netstat -tlnp | grep 8554
# или
ss -tlnp | grep 8554

# Доступен ли порт с клиента?
nc -zv 192.168.1.18 8554
```
