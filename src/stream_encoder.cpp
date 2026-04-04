#include "stream_encoder.h"
#include "glib-object.h"
#include "gst/gstbuffer.h"
#include "gst/gstclock.h"
#include "gst/gstelement.h"
#include "gst/gstpad.h"
#include "rtsp_server.h"
#include <ctime>
#include <iostream>
#include <string>

StreamEncoder::StreamEncoder(int w, int h, int fps)
    : rtsp_server(nullptr), width(w), height(h), fps(25),
    timestamp(0), initialized(false) {}

StreamEncoder::~StreamEncoder() {
    stop();
}

bool StreamEncoder::initRtsp(const std::string &mnt_point, int port) {
    if (initialized) {
        std::cerr << "Encoder уже инициализирован" << std::endl;
        return false;
    }

    rtsp_server = new RtspServer(width, height, fps);

    if (!rtsp_server -> start(mnt_point, port)) {
        std::cerr << "Ошибка запуска RTSP сервера" << std::endl;
        delete rtsp_server;
        rtsp_server = nullptr;
        return false;
    }

    initialized = true;
    timestamp = 0;
    return true;
}

void StreamEncoder::pushFrame(const cv::Mat &frame) {

    if (!initialized || frame.empty()){
        return;
    }

    GstElement *appsrc = rtsp_server->getAppsrc();
    cv::Mat continuousFrame = frame.isContinuous() ? frame : frame.clone();

    if (!appsrc) {
        return;
    }

    if (frame.cols != width || frame.rows != height) {
        std::cerr << "Размер кадра: " << frame.cols << "x" << frame.rows
        << "(ожидалось: " << width << "x" << height << ")" << std::endl;

        cv::Mat resized;
        cv::resize(frame, resized, cv::Size(width, height));
        continuousFrame  = resized;
    } else {
        continuousFrame = frame.isContinuous() ? frame : frame.clone();
    }

    const size_t dataSize = width * height * 3;

    if(continuousFrame.total() * continuousFrame.elemSize() != dataSize) {
        std::cerr << "Размер данных не совпадает: "
        << (continuousFrame.total() * continuousFrame.elemSize())
        << " vs " << dataSize << std::endl;

        return;
    }

    GstBuffer *buffer = gst_buffer_new_allocate(nullptr, dataSize, nullptr);

    //копирование данных
    gst_buffer_fill(buffer, 0, continuousFrame.data, dataSize);

    //Временные метки
    GST_BUFFER_PTS(buffer) = timestamp;
    GST_BUFFER_DTS(buffer) = timestamp;
    GST_BUFFER_DURATION(buffer) = GST_SECOND / fps;
    timestamp += GST_SECOND / fps;

    //отправка
    GstFlowReturn ret;
    g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);
    gst_buffer_unref(buffer);

    if (ret != GST_FLOW_OK && ret != GST_FLOW_FLUSHING){
        std::cerr << "Ошибка отправки: " << gst_flow_get_name(ret) << std::endl;
    }

}

void StreamEncoder::stop() {
    if(!initialized) return;

    if(rtsp_server){
        rtsp_server->stop();
        delete rtsp_server;
        rtsp_server = nullptr;
    }

    initialized = false;
    std::cout << "StreamEncoder оставлен" << std::endl;
}

std::string StreamEncoder::getRtspUrl()const {
    return rtsp_server ? rtsp_server->getUrl() : "";
}
