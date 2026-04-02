/**
 * bouncecast - RTSP сервер с анимированным прямоугольником
 * Этап 1: Захват видео с веб-камеры
 *
 * Компиляция:
 *   mkdir build && cd build && cmake .. && make
 * Запуск:
 *   ./bouncecast
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    std::cout << "=== BOUNCECAST v0.1 ===" << std::endl;
    std::cout << "Этап 1: Захват с веб-камеры" << std::endl;
    std::cout << std::endl;

    // 1. Открываем камеру (device /dev/video0)
    cv::VideoCapture cap(0);

    if (!cap.isOpened()) {
        std::cerr << "❌ Ошибка: Не удалось открыть камеру!" << std::endl;
        std::cerr << "   Проверьте, подключена ли камера (lsusb)" << std::endl;
        return -1;
    }

    std::cout << "✅ Камера открыта успешно" << std::endl;

    // 2. Устанавливаем разрешение (640x480 для стабильности)
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    // 3. Получаем фактическое разрешение
    int width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double fps = cap.get(cv::CAP_PROP_FPS);

    std::cout << "📷 Разрешение: " << width << "x" << height << std::endl;
    std::cout << "📷 FPS: " << fps << std::endl;
    std::cout << std::endl;

    // 4. Создаём окно для предпросмотра (опционально)
    cv::namedWindow("BounceCast Preview", cv::WINDOW_AUTOSIZE);

    // 5. Основной цикл захвата
    cv::Mat frame;
    int frame_count = 0;
    auto start_time = std::chrono::steady_clock::now();

    std::cout << " Захват видео... (нажми 'q' для выхода)" << std::endl;

    while (true) {
        // Читаем кадр с камеры
        cap >> frame;

        // Проверяем, что кадр не пустой
        if (frame.empty()) {
            std::cerr << "⚠️  Пустой кадр, пропускаем..." << std::endl;
            continue;
        }

        frame_count++;

        // Отображаем кадр в окне
        cv::imshow("BounceCast Preview", frame);

        // Обработка нажатий клавиш
        char key = cv::waitKey(1);
        if (key == 'q' || key == 27) {  // 'q' или Escape
            std::cout << "👋 Выход по команде пользователя" << std::endl;
            break;
        }

        // Сохраняем тестовый снимок каждые 30 кадров
        if (frame_count % 30 == 0) {
            std::string filename = "test_output/frame_" + std::to_string(frame_count) + ".jpg";
            cv::imwrite(filename, frame);
            std::cout << "📸 Сохранён тестовый кадр: " << filename << std::endl;
        }
    }

    // 6. Статистика
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);

    std::cout << std::endl;
    std::cout << "=== СТАТИСТИКА ===" << std::endl;
    std::cout << "Всего кадров: " << frame_count << std::endl;
    std::cout << "Время работы: " << duration.count() << " сек" << std::endl;
    if (duration.count() > 0) {
        std::cout << "Средний FPS: " << (frame_count / duration.count()) << std::endl;
    }

    // 7. Освобождение ресурсов
    cap.release();
    cv::destroyAllWindows();

    std::cout << "✅ Ресурсы освобождены, выход" << std::endl;
    return 0;
}
