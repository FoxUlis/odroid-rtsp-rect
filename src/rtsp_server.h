#ifndef RTSP_SERVER_H
#define RTSP_SERVER_H

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <string>
#include <thread>
#include <vector>
#include <atomic>

class RtspServer {
public:
    RtspServer(int width, int height, int fps);
    ~RtspServer();

    // Запуск сервера
    bool start(const std::string &mount_point = "/stream", int port = 8554);

    // Остановка сервера
    void stop();

    // Получить URL для подключения
    std::string getUrl() const { return rtsp_url; }

    // Получить appsrc для отправки кадров (вызывается после подключения клиента)
    GstElement* getAppsrc();

    // Проверка, запущен ли сервер
    bool isRunning() const { return running; }

    // Проверка готовности клиента (media в состоянии prepared/PLAYING)
    bool isClientReady() const { return client_ready; }

private:
    // Обработчик подключения клиента
    static void onMediaConfigure(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer user_data);

    // Обработчик перехода media в состояние prepared
    static void onMediaPrepared(GstRTSPMedia *media, gpointer user_data);

    // Очистка ресурсов
    void cleanup();

    GstRTSPServer *server;
    GstRTSPMediaFactory *factory;
    GstElement *appsrc;  // Кэшируем appsrc после подключения клиента

    int width, height, fps;
    std::string rtsp_url;
    std::atomic<bool> running;
    std::atomic<bool> client_ready;
    GMainLoop *main_loop;
    std::thread loop_thread;
};

#endif
