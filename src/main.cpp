#include <iostream>
#include <csignal>
#include "camera_capture.h"

bool running = true;

void signal_handler(int signum) {
    std::cout << "\nВыход..." << std::endl;
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
    cv::namedWindow("Camera Test", cv::WINDOW_AUTOSIZE);

    while (running) {
        if (!cam.read(frame)) {
            std::cerr << "Ошибка захвата кадра!" << std::endl;
            continue;
        }

        cv::imshow("Camera Test", frame);

        if (cv::waitKey(1) == 'q') break;
    }

    cam.release();
    cv::destroyAllWindows();

    std::cout << "Тест завершён" << std::endl;
    return 0;
}
