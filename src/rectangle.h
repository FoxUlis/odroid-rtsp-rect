#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <opencv2/opencv.hpp>

class Rectangle {
    public:
        Rectangle(int width, int height, int start_x = 50, int start_y = 50);

        void update(int frame_width, int frame_height);

        void draw (cv::Mat &frame);

        int getX() const {return x;}
        int getY() const {return y;}

    private:
        int x, y;
        int dx, dy;
        int width, height;

};

#endif
