#ifndef STREAM_ENCODER_H
#define STREAM_ENCODER_H

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <opencv2/opencv.hpp>
#include <string>

class StreamEncoder {
public:
    // Конструктор (размеры кадра и FPS)
    StreamEncoder(int width, int height, int fps = 30);

    // Деструктор (освобождение ресурсов)
    ~StreamEncoder();

    // Инициализация пайплайна (путь к выходному файлу)
    bool init(const std::string &output_path = "output.h264");

    // Отправка кадра на кодирование
    void pushFrame(const cv::Mat &frame);

    // Остановка и очистка
    void stop();

private:
    GstElement *pipeline;    // GStreamer пайплайн
    GstElement *appsrc;      // Источник данных (мы подаём кадры сюда)
    int width, height, fps;  // Параметры видео
    GstClockTime timestamp;  // Временная метка для каждого кадра
    bool initialized;        // Флаг инициализации
};

#endif
