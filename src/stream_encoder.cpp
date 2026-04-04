#include "stream_encoder.h"
#include "glib-object.h"
#include "gst/gstbin.h"
#include "gst/gstbuffer.h"
#include "gst/gstclock.h"
#include "gst/gstelement.h"
#include "gst/gstobject.h"
#include "gst/gstpad.h"
#include <ctime>
#include <iostream>
#include <string>

StreamEncoder::StreamEncoder(int w, int h, int fps):
    pipeline(nullptr), appsrc(nullptr),
    width(w), height(h), fps(fps), timestamp(0), initialized(false) {}

StreamEncoder::~StreamEncoder() {
    stop();
}

bool StreamEncoder::init(const std::string &output_path) {
    if (initialized) {
        std::cerr << "Encoder уже инициализирован" << std::endl;
        return false;
    }

    GError *error = nullptr;

    // === СТРОИМ ПАЙПЛАЙН ===
    // appsrc → videoconvert → x264enc → h264parse → filesink
    //
    // appsrc: принимает raw-данные от нас
    // videoconvert: конвертирует BGR → формат для кодировщика
    // x264enc: кодирует в H.264
    // h264parse: делает поток корректным (stream-format=byte-stream)
    // filesink: пишет в файл

    std::string pipeline_str =
        "appsrc name=src "
        "is-live=true "
        "format=time "
        "do-timestamp=true "
        "caps=video/x-raw,format=BGR,width=" + std::to_string(width) +
        ",height=" + std::to_string(height) + " ! "
        "videoconvert ! "
        "videorate ! "
        "video/x-raw,format=I420,framerate=25/1 ! "
        "x264enc "
        "speed-preset=ultrafast "
        "tune=zerolatency "
        "bframes=0 "
        "key-int-max=25 "
        "byte-stream=true ! "
        "h264parse config-interval=1 ! "
        "video/x-h264,stream-format=byte-stream,alignment=au ! "
        "filesink location=" + output_path;

    std::cout << "Пайплайн GStreamer" << std::endl;
    std::cout << pipeline_str << std::endl << std::endl;

    pipeline = gst_parse_launch(pipeline_str.c_str(), &error);

    if (error) {
        std::cerr << "Ошибка создания пайплайна" << error->message << std::endl;
        g_error_free(error);
        return false;
    }

    appsrc = gst_bin_get_by_name(GST_BIN(pipeline), "src");

    if (!appsrc) {
        std::cerr << "Не удалось получить appsrc" << std::endl;
        return false;
    }

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    initialized = true;
    std::cout << "GStreamer инициализирован" << std::endl;
    return true;
}

void StreamEncoder::pushFrame(const cv::Mat &frame) {

        if (!initialized || !appsrc || frame.empty()) {
            return;
        }

        cv::Mat continuousFrame = frame.isContinuous() ? frame : frame.clone();

        const size_t dataSize = width * height * 3;

        GstBuffer *buffer = gst_buffer_new_allocate(nullptr, dataSize, nullptr);
        gst_buffer_fill(buffer, 0, continuousFrame.data, dataSize);

        GstFlowReturn ret;
        g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);
        gst_buffer_unref(buffer);

        if (ret != GST_FLOW_OK) {
            std::cerr << "Ошибка отправки кадра: "
                      << gst_flow_get_name(ret) << std::endl;
        }
    }

void StreamEncoder::stop() {
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);

        if (appsrc) {
            gst_object_unref(appsrc);
            appsrc = nullptr;
        }
        gst_object_unref(pipeline);
        pipeline = nullptr;

        initialized = false;
        std::cout << "GStreamer остановлен" << std::endl;
    }
}
