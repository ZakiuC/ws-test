#pragma once
#include <libwebsockets.h>
#include <string>
#include <functional>
#include <thread>

class WebSocketClient {
public:
    using MessageHandler = std::function<void(const std::string&)>;
    
    WebSocketClient(const std::string& url, const std::string& id);
    ~WebSocketClient();
    
    bool connect();
    void send(const std::string& target, const std::string& data);
    void setHandler(MessageHandler handler);
    void disconnect();

private:
    static int callback(struct lws *wsi, enum lws_callback_reasons reason, 
                        void *user, void *in, size_t len);
    void runEventLoop();

    struct lws_context* context = nullptr;
    struct lws* wsi = nullptr;
    std::string url;
    std::string deviceId;
    MessageHandler msgHandler;
    std::thread eventThread;
    bool running = false;
};