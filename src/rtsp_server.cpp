#include "rtsp_server.h"
#include "glib.h"
#include "gst/gstpad.h"
#include <iostream>
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <gst/app/gstappsrc.h>
#include <string>
#include <thread>

RtspServer::RtspServer(int w, int h, int f)
    : server(nullptr), factory(nullptr), appsrc(nullptr),
      width(w), height(h), fps(f), running(false), client_ready(false), main_loop(nullptr) {}

RtspServer::~RtspServer() {
    stop();
}

bool RtspServer::start(const std::string &mount_point, int port) {
    if (running) {
        std::cerr << "RTSP сервер уже запущен" << std::endl;
        return false;
    }

    // === СОЗДАЁМ СЕРВЕР ===
    server = gst_rtsp_server_new();
    std::string port_str = std::to_string(port);
    g_object_set(server, "service", port_str.c_str(), nullptr);

    // === СОЗДАЁМ ФАБРИКУ ===
    factory = gst_rtsp_media_factory_new();

    // === ПАЙПЛАЙН ДЛЯ КЛИЕНТОВ ===
    std::string pipeline_str =
        "( appsrc name=src is-live=true format=time ! "
        "videoconvert ! "
        "x264enc speed-preset=ultrafast tune=zerolatency bitrate=2048 key-int-max=30 force-idr=1 ! "
        "video/x-h264,profile=baseline ! "
        "rtph264pay name=pay0 pt=96 config-interval=1 )";

    gst_rtsp_media_factory_set_launch(factory, pipeline_str.c_str());
    gst_rtsp_media_factory_set_shared(factory, FALSE);  // Отключаем shared factory
    gst_rtsp_media_factory_set_latency(factory, 0);

    // Принудительно разрешаем только TCP (interleaved) транспорт
    gst_rtsp_media_factory_set_protocols(
        factory,
        GST_RTSP_LOWER_TRANS_TCP
    );

    // === ПОДКЛЮЧАЕМ ОБРАБОТЧИК "media-configure" ===
    // Вызывается при подключении каждого клиента
    g_signal_connect(factory, "media-configure",
                     G_CALLBACK(onMediaConfigure), this);

    // === ДОБАВЛЯЕМ ФАБРИКУ НА СЕРВЕР ===
    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);
    gst_rtsp_mount_points_add_factory(mounts, mount_point.c_str(), factory);
    gst_object_unref(mounts);

    // === ПРИКРЕПЛЯЕМ СЕРВЕР К GMainLoop ===
    if (!gst_rtsp_server_attach(server, nullptr)) {
        std::cerr << "Не удалось запустить RTSP сервер" << std::endl;
        cleanup();
        return false;
    }

    main_loop = g_main_loop_new(nullptr, FALSE);

    loop_thread = std::thread([this]() {
        g_main_loop_run(main_loop);
    });

    // === ФОРМИРУЕМ URL ===
    rtsp_url = "rtsp://<IP-плата>:" + std::to_string(port) + mount_point;

    std::cout << "=== RTSP СЕРВЕР ЗАПУЩЕН ===" << std::endl;
    std::cout << "URL: " << rtsp_url << std::endl;
    std::cout << "Пример: rtsp://192.168.1.100:" << port << mount_point << std::endl;
    std::cout << "Ожидание подключения клиентов..." << std::endl;
    std::cout << std::endl;

    client_ready = false;
    running = true;
    return true;
}

void RtspServer::stop() {
    if (!running) return;

    std::cout << "Остановка RTSP сервера..." << std::endl;

    if (main_loop) {
        g_main_loop_quit(main_loop);

        if (loop_thread.joinable()) {
            loop_thread.join();
        }

        g_main_loop_unref(main_loop);
        main_loop = nullptr;
    }

    if (server) {
        gst_object_unref(server);
        server = nullptr;
    }

    factory = nullptr;
    appsrc = nullptr;
    client_ready = false;
    running = false;

    std::cout << "RTSP сервер остановлен" << std::endl;
}

GstElement* RtspServer::getAppsrc() {
    return appsrc;
}

void RtspServer::onMediaConfigure(GstRTSPMediaFactory *factory,
                                   GstRTSPMedia *media,
                                   gpointer user_data) {
    RtspServer *self = static_cast<RtspServer*>(user_data);

    // Получаем элемент пайплайна для этого клиента
    GstElement *element = gst_rtsp_media_get_element(media);

    // Находим appsrc внутри пайплайна
    GstElement *appsrc = gst_bin_get_by_name(GST_BIN(element), "src");

    if (appsrc) {
        std::cout << "✅ Клиент подключился! appsrc готов" << std::endl;

        // Разрешаем повторное использование media
        gst_rtsp_media_set_reusable(media, TRUE);

        g_object_set(appsrc,
                     "is-live", TRUE,
                     "format", GST_FORMAT_TIME,
                     "block", TRUE,
                     "stream-type", GST_APP_STREAM_TYPE_STREAM,
                     nullptr);

        GstCaps *caps = gst_caps_new_simple(
            "video/x-raw",
            "format", G_TYPE_STRING, "BGR",
            "width", G_TYPE_INT, self->width,
            "height", G_TYPE_INT, self->height,
            "framerate", GST_TYPE_FRACTION, self->fps, 1,
            nullptr);

        gst_app_src_set_caps(GST_APP_SRC(appsrc), caps);
        gst_caps_unref(caps);

        self->appsrc = appsrc;

        g_signal_connect(media, "prepared",
                         G_CALLBACK(onMediaPrepared), self);

        gst_object_unref(element);
    } else {
        std::cerr << "❌ Не удалось найти appsrc в пайплайне" << std::endl;
        gst_object_unref(element);
    }

    GstBuffer *test_buf = gst_buffer_new_allocate(nullptr, self->width * self->height *3, nullptr);
    gst_buffer_memset(test_buf, 0, 0, self->width * self->height * 3);

    GST_BUFFER_PTS(test_buf) = 0;
    GST_BUFFER_DTS(test_buf) = 0;
    GST_BUFFER_DURATION(test_buf) = GST_SECOND / self->fps;

    GstFlowReturn ret;
    g_signal_emit_by_name(appsrc, "push-buffer", test_buf, &ret);
    gst_buffer_unref(test_buf);

    std::cout << "Тестовый кадр отправлен, ret =" << gst_flow_get_name(ret) << std::endl;
}

void RtspServer::onMediaPrepared(GstRTSPMedia *media, gpointer user_data) {
    RtspServer *self = static_cast<RtspServer*>(user_data);
    self->client_ready = true;
    std::cout << "🚀 Media готова (prepared/PLAYING)" << std::endl;
}

void RtspServer::cleanup() {
    if (factory) {
        gst_object_unref(factory);
        factory = nullptr;
    }
    if (server) {
        gst_object_unref(server);
        server = nullptr;
    }
    appsrc = nullptr;
    running = false;
}
