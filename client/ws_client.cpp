#include "ws_client.h"
#include <cstring>
#include <iostream>
#include <mutex>
#include <queue>

struct ConnectionData {
    std::queue<std::string> sendQueue;
    std::mutex queueMutex;
    WebSocketClient* client;
};

WebSocketClient::WebSocketClient(const std::string& url, const std::string& id) 
    : url(url), deviceId(id) {}

bool WebSocketClient::connect() {
    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;
    
    context = lws_create_context(&info);
    if (!context) return false;

    struct lws_client_connect_info ccinfo;
    memset(&ccinfo, 0, sizeof(ccinfo));
    ccinfo.context = context;
    ccinfo.address = url.substr(url.find("://") + 3).c_str();
    ccinfo.port = url.find("wss://") == 0 ? 443 : 80;
    ccinfo.path = "/";
    ccinfo.host = ccinfo.address;
    ccinfo.origin = ccinfo.address;
    ccinfo.protocol = protocols[0].name;
    ccinfo.ssl_connection = url.find("wss://") == 0 ? LCCSCF_USE_SSL : 0;

    wsi = lws_client_connect_via_info(&ccinfo);
    if (!wsi) return false;

    running = true;
    eventThread = std::thread(&WebSocketClient::runEventLoop, this);
    return true;
}

void WebSocketClient::runEventLoop() {
    while (running) {
        lws_service(context, 50);
    }
    lws_context_destroy(context);
}

int WebSocketClient::callback(lws *wsi, lws_callback_reasons reason, 
                             void *user, void *in, size_t len) {
    ConnectionData* data = (ConnectionData*)user;
    
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            lws_callback_on_writable(wsi);
            break;
            
        case LWS_CALLBACK_CLIENT_RECEIVE:
            if (data && data->client && in) {
                std::string msg((char*)in, len);
                data->client->msgHandler(msg);
            }
            break;
            
        case LWS_CALLBACK_CLIENT_WRITEABLE:
            if (data && !data->sendQueue.empty()) {
                std::lock_guard<std::mutex> lock(data->queueMutex);
                auto& msg = data->sendQueue.front();
                lws_write(wsi, (unsigned char*)msg.data(), msg.size(), LWS_WRITE_TEXT);
                data->sendQueue.pop();
            }
            lws_callback_on_writable(wsi);
            break;
            
        default:
            break;
    }
    return 0;
}

void WebSocketClient::send(const std::string& target, const std::string& payload) {
    if (!wsi) return;
    
    std::string msg = json::encode({
        {"target", target},
        {"data", payload}
    });
    
    ConnectionData* data = (ConnectionData*)lws_wsi_user(wsi);
    if (data) {
        std::lock_guard<std::mutex> lock(data->queueMutex);
        data->sendQueue.push(std::move(msg));
    }
}

void WebSocketClient::setHandler(MessageHandler handler) {
    msgHandler = handler;
}

void WebSocketClient::disconnect() {
    running = false;
    if (eventThread.joinable()) eventThread.join();
}

// 协议定义
static struct lws_protocols protocols[] = {
    {
        "ws-protocol",
        WebSocketClient::callback,
        sizeof(ConnectionData),
        1024,
    },
    { NULL, NULL, 0, 0 }
};