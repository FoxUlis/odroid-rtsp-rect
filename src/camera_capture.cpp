#include "camera_capture.h"
#include "opencv2/videoio.hpp"
#include <iostream>

CameraCapture::CameraCapture(int device_id)
    : device_id(device_id), width(640), height(480) {}

CameraCapture::~CameraCapture(){
    release();
}

bool CameraCapture::open() {
    cap.open(device_id, cv::CAP_V4L2);

    if (!cap.isOpened()) {
        std::cerr << "Не удалось открыть камеру " << device_id << std::endl;
        return false;
    }

    if (!cap.set(cv::CAP_PROP_FOURCC,
        cv::VideoWriter::fourcc('M', 'J', 'P', 'G'))) {
            std::cerr << "Не удалось установить MJPG" << std::endl;
        }

    if (!cap.set(cv::CAP_PROP_FRAME_WIDTH, width)) {
        std::cerr << "Warning: Не удалось установить ширину" << std::endl;
    }

    if (!cap.set(cv::CAP_PROP_FRAME_HEIGHT, height)) {
        std::cerr << "Warning: Не удалось установить высоту" << std::endl;
    }


    std::cout << "Фактическое разрешение камеры: "
    << cap.get(cv::CAP_PROP_FRAME_WIDTH) << "x"
    << cap.get(cv::CAP_PROP_FRAME_HEIGHT)
    << std::endl;

    width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));

    std::cout << "Камера открыта " << width << "x" << height << std::endl;
    return true;
}

bool CameraCapture::read(cv::Mat &frame) {
    cap >> frame;

    if(frame.empty()) {
        std::cerr << "Пустой кадр" << std::endl;
        return false;
    }
    return true;
}

bool CameraCapture::isOpened() const {
    return cap.isOpened();
}

void CameraCapture::release() {
    if (cap.isOpened()) {
        cap.release();
    }
}
