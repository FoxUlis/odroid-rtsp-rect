#ifndef STREAM_ENCODER_H
#define STREAM_ENCODER_H

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <opencv2/opencv.hpp>
#include <string>
#include "gst/gstclock.h"
#include "rtsp_server.h"

class StreamEncoder {
public:
    // Конструктор (размеры кадра и FPS)
    StreamEncoder(int width, int height, int fps = 25);

    // Деструктор (освобождение ресурсов)
    ~StreamEncoder();

    // Инициализация пайплайна (путь к выходному файлу)
    bool initRtsp(const std::string &mnt_point = "/stream", int port = 8554);

    // Отправка кадра на кодирование
    void pushFrame(const cv::Mat &frame);

    // Остановка и очистка
    void stop();

    std::string getRtspUrl() const;

private:
    RtspServer *rtsp_server;
    int width, height, fps;
    GstClockTime timestamp;
    bool initialized;

};

#endif
