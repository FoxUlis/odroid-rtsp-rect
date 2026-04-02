/**
 * bouncecast - RTSP сервер с анимированным прямоугольником
 * Этап 2: Отрисовка прямоугольника на кадре
 *
 * Сборка:
 *   cd build && cmake .. && make
 * Запуск:
 *   ./bouncecast
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <thread>

// === КОНСТАНТЫ ПРЯМОУГОЛЬНИКА ===
const int RECT_WIDTH = 30;   // Ширина прямоугольника (пиксели)
const int RECT_HEIGHT = 20;  // Высота прямоугольника (пиксели)
const int RECT_X = 50;       // Начальная позиция X
const int RECT_Y = 50;       // Начальная позиция Y

/**
 * Функция отрисовки прямоугольника на кадре
 * @param frame Кадр из камеры (cv::Mat)
 */
void draw_rectangle(cv::Mat &frame) {
    // Координаты левого верхнего угла
    cv::Point top_left(RECT_X, RECT_Y);

    // Координаты правого нижнего угла
    cv::Point bottom_right(RECT_X + RECT_WIDTH, RECT_Y + RECT_HEIGHT);

    // Цвет: зелёный (B, G, R) = (0, 255, 0)
    cv::Scalar color(0, 255, 0);

    // Толщина линии: 2 пикселя
    int thickness = 2;

    // Рисуем прямоугольник
    cv::rectangle(frame, top_left, bottom_right, color, thickness);
}

int main() {
    std::cout << "=== BOUNCECAST v0.2 ===" << std::endl;
    std::cout << "Этап 2: Отрисовка прямоугольника" << std::endl;
    std::cout << std::endl;

    // 1. Открываем камеру
    cv::VideoCapture cap(0);

    if (!cap.isOpened()) {
        std::cerr << "❌ Ошибка: Не удалось открыть камеру!" << std::endl;
        return -1;
    }

    std::cout << "✅ Камера открыта успешно" << std::endl;

    // 2. Устанавливаем разрешение
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    int width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);

    std::cout << "📷 Разрешение: " << width << "x" << height << std::endl;
    std::cout << "📦 Размер прямоугольника: " << RECT_WIDTH << "x" << RECT_HEIGHT << std::endl;
    std::cout << std::endl;

    // 3. Основной цикл
    cv::Mat frame;
    int frame_count = 0;

    std::cout << " Захват видео с прямоугольником... (нажми 'q' для выхода)" << std::endl;

    while (true) {
        // Читаем кадр
        cap >> frame;

        if (frame.empty()) {
            std::cerr << "⚠️  Пустой кадр" << std::endl;
            continue;
        }

        // === ОТРИСОВКА ПРЯМОУГОЛЬНИКА ===
        draw_rectangle(frame);
        // =================================

        frame_count++;

        // Отображение (если есть GUI)
        cv::imshow("BounceCast Preview", frame);

        // Обработка клавиш
        char key = cv::waitKey(1);
        if (key == 'q' || key == 27) {
            std::cout << "👋 Выход" << std::endl;
            break;
        }

        // Сохраняем кадр каждые 60 кадров (примерно 2 секунды)
        if (frame_count % 60 == 0) {
            std::string filename = "test_output/rect_frame_" + std::to_string(frame_count) + ".jpg";
            cv::imwrite(filename, frame);
            std::cout << "📸 Сохранён кадр с прямоугольником: " << filename << std::endl;
        }
    }

    // 4. Статистика
    std::cout << std::endl;
    std::cout << "=== СТАТИСТИКА ===" << std::endl;
    std::cout << "Всего кадров обработано: " << frame_count << std::endl;

    // 5. Освобождение ресурсов
    cap.release();
    cv::destroyAllWindows();

    std::cout << "✅ Ресурсы освобождены" << std::endl;
    return 0;
}
