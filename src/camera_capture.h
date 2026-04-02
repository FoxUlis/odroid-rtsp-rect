#ifndef CAMERA_CAPTURE_H
#define CAMERA_CAPTURE_H

#include "opencv2/opencv.hpp"
#include "opencv2/videoio.hpp"

class CameraCapture {
public:
    CameraCapture(int device_id = 0);
    ~CameraCapture();

    bool open();
    bool read(cv::Mat &frame);
    bool isOpened() const;

    int getWidth() const {return width;}
    int getHeight() const {return height;}

    void release();

    private:
        cv::VideoCapture cap;
        int device_id;
        int width;
        int height;

};

#endif
