#include <iostream>
#include <csignal>
#include <chrono>
#include <thread>
#include "camera_capture.h"
#include "rectangle.h"
#include "stream_encoder.h"  // ← Новый заголовок

bool running = true;

void signal_handler(int /* signum */) {
    std::cout << "\n👋 Выход..." << std::endl;
    running = false;
}

int main() {
    signal(SIGINT, signal_handler);

    std::cout << "=== BOUNCECAST v0.4 ===" << std::endl;
    std::cout << "Этап 4: Кодирование H.264" << std::endl;
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

    // 3. Кодировщик (передаём реальные размеры)
    StreamEncoder encoder(cam.getWidth(), cam.getHeight(), 30);
    if (!encoder.init("output.h264")) {
        std::cerr << "Ошибка инициализации encoder!" << std::endl;
        return -1;
    }

    std::cout << "📹 Запись в output.h264... (Ctrl+C для остановки)" << std::endl;
    std::cout << std::endl;

    cv::Mat frame;
    int frame_count = 0;
    auto start_time = std::chrono::steady_clock::now();

    // Опционально: предпросмотр
    cv::namedWindow("Preview", cv::WINDOW_AUTOSIZE);

    while (running) {
        if (!cam.read(frame)) continue;

        // Обновление и отрисовка
        rect.update(frame.cols, frame.rows);
        rect.draw(frame);

        // === ОТПРАВКА НА КОДИРОВАНИЕ ===
        encoder.pushFrame(frame);

        frame_count++;

        // Предпросмотр (можно закомментировать для headless)
        cv::imshow("Preview", frame);
        if (cv::waitKey(1) == 'q') break;

        // Прогресс
        if (frame_count % 60 == 0) {
            std::cout << "📼 Записано кадров: " << frame_count << std::endl;
        }

        // Синхронизация ~30 FPS
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }

    // Статистика
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);

    std::cout << std::endl;
    std::cout << "=== СТАТИСТИКА ===" << std::endl;
    std::cout << "Кадров: " << frame_count << std::endl;
    std::cout << "Время: " << duration.count() << " сек" << std::endl;
    std::cout << "Файл: output.h264" << std::endl;

    // Очистка
    cam.release();
    encoder.stop();
    cv::destroyAllWindows();
    gst_deinit();

    std::cout << "✅ Завершено" << std::endl;
    return 0;
}
