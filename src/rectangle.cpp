#include "rectangle.h"
#include "opencv2/core/types.hpp"
#include "opencv2/imgproc.hpp"
#include <string>

Rectangle::Rectangle(int w, int h, int start_x, int start_y):
    x(start_x), y(start_y), dx(2), dy(2), width(w), height(h){}

void Rectangle::update(int frame_width, int frame_height) {
    x += dx;
    y += dy;

    // отскок от левой/правой границы
    if (x <= 0) {
        x = 0;
        dx = -dx;
    } else if (x + width >= frame_width) {
        x = frame_width - width;
        dx = -dx;
    }

    if (y <= 0) {
        y = 0;
        dy = -dy;
    } else if (y + height >= frame_height) {
        y = frame_height - height;
        dy = -dy;
    }

}

void Rectangle::draw(cv::Mat &frame) {
    cv::Point top_left(x, y);
    cv::Point bottom_right(x + width, y + height);
    cv::rectangle(frame, top_left, bottom_right, cv::Scalar(0, 255, 0), 2);

    //координаты для отладки
    std::string info = "X: " + std::to_string(x) + "Y: " + std::to_string(y);
    cv::putText(frame, info, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0,255,255), 2);
}
