# 🎥 BounceCast — RTSP видеостриминг с анимацией (C++ / GStreamer)

Проект демонстрирует захват видео с WEB-камеры, отрисовку анимированного прямоугольника с физикой отскока и потоковую трансляцию в формате **H.264** через **RTSP-сервер** на базе `gst-rtsp-server`.

Проект ориентирован на Linux-системы (V4L2 + GStreamer) и подходит для одноплатных компьютеров (например, Odroid).

---

## 🚀 Функциональность

✅ Захват видео с WEB-камеры через **OpenCV (V4L2)**  
✅ Отрисовка прямоугольника (30×20 px), движущегося под углом 45°  
✅ Корректная физика отскока (угол падения = углу отражения)  
✅ Кодирование видеопотока в **H.264 (x264enc)**  
✅ RTSP-сервер на базе **gst-rtsp-server**  
✅ Поддержка клиентов: **ffplay, mpv, VLC**  
✅ Поддержка транспортов **TCP и UDP**

---

## 🧠 Архитектура

```
CameraCapture (OpenCV / V4L2)
        ↓
Rectangle (логика движения и отскока)
        ↓
StreamEncoder (GStreamer appsrc + x264)
        ↓
RtspServer (gst-rtsp-server)
```

### Основной pipeline GStreamer

```
appsrc → videoconvert → x264enc tune=zerolatency → rtph264pay → RTSP
```

RTSP-сервер поддерживает как **TCP (interleaved)**, так и **UDP (RTP/RTCP)** транспорт.  
Клиент автоматически выбирает подходящий способ передачи.

---

## 📁 Структура проекта

```
├── build/                    # Директория сборки
├── CMakeLists.txt            # Конфигурация сборки
├── README.md                 # Документация
├── src/
│   ├── main.cpp              # Точка входа
│   ├── camera_capture.cpp/h  # Захват видео
│   ├── rectangle.cpp/h       # Логика движения прямоугольника
│   ├── stream_encoder.cpp/h  # Кодирование кадров и отправка в appsrc
│   └── rtsp_server.cpp/h     # RTSP-сервер (gst-rtsp-server)
└── test_output/              # Тестовые файлы
```

---

## 🛠 Требования

### Ubuntu / Debian

```bash
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

---

## 🔧 Сборка

```bash
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```

---

## ▶ Запуск

```bash
./bouncecast
```

Пример вывода:

```
=== BOUNCECAST v0.5 ===
Камера открыта 640x480
=== RTSP СЕРВЕР ЗАПУЩЕН ===
URL: rtsp://<IP>:8554/stream
```

По умолчанию используется порт **8554**.

---

## 📡 Подключение клиента

На другом устройстве в той же сети:

### 🎬 ffplay (TCP)

```bash
ffplay -rtsp_transport tcp rtsp://<IP>:8554/stream
```

### 🎬 ffplay (UDP)

```bash
ffplay -rtsp_transport udp rtsp://<IP>:8554/stream
```

### 🎥 mpv

```bash
mpv rtsp://<IP>:8554/stream
```

### 📺 VLC

В графическом интерфейсе выберите:
```
Media → Open Network Stream → rtsp://<IP>:8554/stream
```

---

## 🧪 Тестирование TCP и UDP

### Проверка используемого транспорта

**TCP:**
```bash
ffplay -rtsp_transport tcp rtsp://<IP>:8554/stream
```

**UDP:**
```bash
ffplay -rtsp_transport udp rtsp://<IP>:8554/stream
```

При UDP поток передаётся по RTP/RTCP через отдельные порты.  
Если UDP не работает — проверьте firewall.

---

### Проверка открытых портов

```bash
ss -tlnp | grep 8554
```

UDP-порты можно проверить так:

```bash
ss -ulnp
```

---

## 🔍 Диагностика

### Проверка камеры

```bash
ls -l /dev/video*
gst-launch-1.0 v4l2src ! videoconvert ! autovideosink
```

### Проверка плагинов GStreamer

```bash
gst-inspect-1.0 x264enc
gst-inspect-1.0 appsrc
gst-inspect-1.0 rtph264pay
```

---

## 🎯 Соответствие ТЗ

- ✅ Захват изображений с WEB-камеры  
- ✅ Отрисовка прямоугольника 30×20 px  
- ✅ Движение под углом 45°  
- ✅ Корректная физика отражения  
- ✅ Сжатие потока в H.264  
- ✅ RTSP-сервер на базе gst-rtsp-server  
- ✅ Поддержка TCP и UDP транспортов  

---

## 📌 Примечания

- Используется фиксированное разрешение камеры (по умолчанию 640×480) для стабильного FPS.
- Поток кодируется с настройками `tune=zerolatency` для минимальной задержки.
- Подходит для headless-систем.
- UDP обеспечивает меньшую задержку, но может терять пакеты.
- TCP обеспечивает более стабильную передачу.

---

## ✅ Завершение работы

Для остановки нажмите:

```
Ctrl + C
```

---

**Автор:**  
Проект выполнен в рамках тестового задания по разработке RTSP-видеостриминга на C++.