#include "ws_client.h"
#include <iostream>
#include <unistd.h>

int main() {
    WebSocketClient client("ws://your-server-ip:8765", "board1");
    
    client.setHandler([](const std::string& msg) {
        std::cout << "Received: " << msg << std::endl;
    });
    
    if (!client.connect()) {
        std::cerr << "Connection failed" << std::endl;
        return 1;
    }
    
    // 发送注册消息
    client.send("server", R"({"id": "board1"})");
    
    // 示例：每5秒发送数据到board2
    int count = 0;
    while (true) {
        std::string data = "SensorData-" + std::to_string(++count);
        client.send("board2", data);
        sleep(5);
    }
    
    client.disconnect();
    return 0;
}