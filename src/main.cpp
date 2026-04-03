#include <chrono>
#include <iostream>
#include <csignal>
#include <thread>

#include "camera_capture.h"
#include "rectangle.h"

bool running = true;

void signal_handler(int signum) {
    std::cout << "\nВыход..." << signum << std::endl;
    running = false;
}

int main() {
    signal(SIGINT, signal_handler);

    std::cout << "=== Тест Камера + прямоугольник ===" << std::endl;

    CameraCapture cam(0);  // Камера по умолчанию

    Rectangle rect(30, 20, 50, 50);

    if (!cam.open()) {
        std::cerr << "Ошибка инициализации!" << std::endl;
        return -1;
    }

    std::cout << "Разрешение: " << cam.getWidth() << "x" << cam.getHeight() << std::endl;
    std::cout << "Нажми 'q' для выхода" << std::endl;

    cv::Mat frame;
    int frame_count = 0;
    auto start_time = std::chrono::steady_clock::now();

    while (running) {
        if (!cam.read(frame)) {
            std::cerr << "Ошибка захвата кадра!" << std::endl;
            continue;
        }

        rect.update(frame.cols, frame.rows);
        rect.draw(frame);

        frame_count++;

        cv::imshow("Test", frame);

        if (cv::waitKey(1) == 'q') break;

        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }

    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);

    cam.release();
    cv::destroyAllWindows();

    std::cout << "Тест завершён" << std::endl;
    std::cout << "Кадров: " << frame_count << std::endl;
    std::cout << "Время: " << duration.count() << " сек" << std::endl;
       if (duration.count() > 0) {
           std::cout << "Средний FPS: " << (frame_count / duration.count()) << std::endl;
       }
    return 0;
}
