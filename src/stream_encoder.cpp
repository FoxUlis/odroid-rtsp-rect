#include "stream_encoder.h"
#include "glib-object.h"
#include "gst/gstbin.h"
#include "gst/gstbuffer.h"
#include "gst/gstclock.h"
#include "gst/gstelement.h"
#include "gst/gstobject.h"
#include "gst/gstpad.h"
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
        "appsrc name=src emit-signals=false is-live=true "
        "caps=video/x-raw,format=BGR,width=" + std::to_string(width) +
        ",height=" + std::to_string(height) +
        ",framerate=" + std::to_string(fps) + "/1 ! " +
        "videoconvert ! " +
        "video/x-raw, format=I420 ! " +
        "x264enc speed-preset=ultrafast tune=zerolatency ! " +
        "h264parse ! " +
        "video/x-h264,stream-format=byte-stream ! " +
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

    //Создаем буфер Gstreamer
    // Выделяем память под размер кадра
    GstBuffer *buffer = gst_buffer_new_allocate(
        nullptr,
        frame.total() * frame.elemSize(),
        nullptr);

    //Копируем данные из cv::Mat в буффер
    // frame.data указывает на сырые байты изображения
    gst_buffer_fill(
        buffer,
        0,
        frame.data,
        frame.total() * frame.elemSize());

    //Устанавливаем временные метки
    // PTS (Presentation Time Stamp)= когда показывать кадр
    // Duration - сколько длится кадр
    GST_BUFFER_PTS(buffer) = timestamp;
    GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale(1, GST_SECOND, fps);
    timestamp += GST_BUFFER_DURATION(buffer);

    //Отправляем буфер в пайплайн
    GstFlowReturn ret;
    g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);

    //освобождаем буффер
    gst_buffer_unref(buffer);

    if (ret != GST_FLOW_OK) {
        std::cerr << "Ошибка отправки кадра: " << gst_flow_get_name(ret) << std::endl;
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
