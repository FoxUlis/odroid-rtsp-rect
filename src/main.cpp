#include <iostream>
#include <csignal>
#include "camera_capture.h"

bool running = true;

void signal_handler(int signum) {
    std::cout << "\nВыход..." << signum << std::endl;
    running = false;
}

int main() {
    signal(SIGINT, signal_handler);

    std::cout << "=== Тест CameraCapture ===" << std::endl;

    CameraCapture cam(0);  // Камера по умолчанию

    if (!cam.open()) {
        std::cerr << "Ошибка инициализации!" << std::endl;
        return -1;
    }

    std::cout << "Разрешение: " << cam.getWidth() << "x" << cam.getHeight() << std::endl;
    std::cout << "Нажми 'q' для выхода" << std::endl;

    cv::Mat frame;
    int frame_count = 0;
    auto start_time = std::chrono::steady_clock::now();


    cv::namedWindow("Camera Test", cv::WINDOW_AUTOSIZE);

    while (running) {
        if (!cam.read(frame)) {
            std::cerr << "Ошибка захвата кадра!" << std::endl;
            continue;
        }

        frame_count++;

        cv::imshow("Camera Test", frame);

        if (cv::waitKey(1) == 'q') break;
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
