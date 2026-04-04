#include <iostream>
#include <csignal>
#include <chrono>
#include <thread>
#include "camera_capture.h"
#include "rectangle.h"
#include "stream_encoder.h"

bool running = true;

void signal_handler(int /* signum */) {
    std::cout << "\n Выход..." << std::endl;
    running = false;
}

int main() {
    signal(SIGINT, signal_handler);


    std::cout << "=== BOUNCECAST v0.5 ===" << std::endl;
    std::cout << "Этап 5: RTSP Стриминг" << std::endl;
    std::cout << std::endl;

    // === ИНИЦИАЛИЗАЦИЯ GSTREAMER ===
    gst_init(nullptr, nullptr);

    // 1. Камера
    CameraCapture cam(0);
    if (!cam.open()) {
        return -1;
    }

    // 2. Прямоугольник
    Rectangle rect(30, 20, 50, 50);

    // 3. Кодировщик (RTSP)
    StreamEncoder encoder(cam.getWidth(), cam.getHeight(), 30);
    if (!encoder.initRtsp("/stream", 8554)) {
        std::cerr << "Ошибка инициализации encoder!" << std::endl;
        return -1;
    }

    std::cout << "📡 Стриминг: " << encoder.getRtspUrl() << std::endl;
    std::cout << "Подключайтесь в VLC: rtsp://<IP-плата>:8554/stream" << std::endl;
    std::cout << "(Ctrl+C для остановки)" << std::endl;
    std::cout << std::endl;

    cv::Mat frame;
    int frame_count = 0;
    auto start_time = std::chrono::steady_clock::now();

    // Опционально: предпросмотр
    cv::namedWindow("Preview", cv::WINDOW_AUTOSIZE);

    const int target_fps = 25;
    const auto frame_duration = std::chrono::milliseconds(1000 / target_fps);

    while (running) {
        auto frame_start = std::chrono::steady_clock::now();

        if (!cam.read(frame)) continue;

        // Обновление и отрисовка
        rect.update(frame.cols, frame.rows);
        rect.draw(frame);

        // === ОТПРАВКА НА СТРИМИНГ ===
        encoder.pushFrame(frame);

        // Предпросмотр (можно закомментировать для headless)
        // cv::imshow("Preview", frame);
        // if (cv::waitKey(1) == 'q') break;

        auto frame_end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(frame_end - frame_start);

        if (elapsed < frame_duration) {
            std::this_thread::sleep_for(frame_duration - elapsed);
        }

        frame_count++;
    }

    // Статистика
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);

    std::cout << std::endl;
    std::cout << "=== СТАТИСТИКА ===" << std::endl;
    std::cout << "Кадров: " << frame_count << std::endl;
    std::cout << "Время: " << duration.count() << " сек" << std::endl;
    std::cout << "Средний FPS: " << (frame_count / (duration.count() ? duration.count() : 1)) << std::endl;

    // Очистка
    cam.release();
    encoder.stop();
    cv::destroyAllWindows();
    gst_deinit();

    std::cout << "✅ Завершено" << std::endl;
    return 0;
}
