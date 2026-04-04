#include "rtsp_server.h"
#include <iostream>
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

RtspServer::RtspServer(int w, int h, int f)
    : server(nullptr), factory(nullptr), appsrc(nullptr),
      width(w), height(h), fps(f), running(false), main_loop(nullptr) {}

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
    g_object_set(server, "port", port, nullptr);

    // === СОЗДАЁМ ФАБРИКУ ===
    factory = gst_rtsp_media_factory_new();

    // === ПАЙПЛАЙН ДЛЯ КЛИЕНТОВ ===
    std::string pipeline_str =
        "( appsrc name=src is-live=true format=time "
        "caps=video/x-raw,format=BGR,width=" + std::to_string(width) +
        ",height=" + std::to_string(height) +
        ",framerate=" + std::to_string(fps) + "/1 ! "
        "videoconvert ! "
        "video/x-raw,format=I420 ! "
        "videorate ! video/x-raw,framerate=" + std::to_string(fps) + "/1 ! "
        "x264enc speed-preset=ultrafast tune=zerolatency bframes=0 "
        "key-int-max=" + std::to_string(fps) + " ! "
        "h264parse config-interval=1 ! "
        "rtph264pay name=pay0 pt=96 )";

    gst_rtsp_media_factory_set_launch(factory, pipeline_str.c_str());
    gst_rtsp_media_factory_set_shared(factory, TRUE);  // Несколько клиентов

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

    // === ФОРМИРУЕМ URL ===
    rtsp_url = "rtsp://<IP-плата>:" + std::to_string(port) + mount_point;

    std::cout << "=== RTSP СЕРВЕР ЗАПУЩЕН ===" << std::endl;
    std::cout << "URL: " << rtsp_url << std::endl;
    std::cout << "Пример: rtsp://192.168.1.100:" << port << mount_point << std::endl;
    std::cout << "Ожидание подключения клиентов..." << std::endl;
    std::cout << std::endl;

    running = true;
    return true;
}

void RtspServer::stop() {
    if (!running) return;

    std::cout << "Остановка RTSP сервера..." << std::endl;

    // Останавливаем сервер
    if (server) {
        gst_object_unref(server);
        server = nullptr;
    }

    // Фабрика освободится вместе с сервером
    factory = nullptr;
    appsrc = nullptr;
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

        // Сохраняем указатель для дальнейшего использования
        // (в реальном проекте лучше хранить список для нескольких клиентов)
        self->appsrc = appsrc;

        // Не делаем unref — он будет жить пока живёт media
        gst_object_unref(element);
    } else {
        std::cerr << "❌ Не удалось найти appsrc в пайплайне" << std::endl;
        gst_object_unref(element);
    }
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
