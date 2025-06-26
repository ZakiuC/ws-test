cmake_minimum_required(VERSION 3.10)
project(EmbeddedWebsocketCommunication)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找libwebsockets库
find_package(Libwebsockets REQUIRED)
include_directories(${LIBWEBSOCKETS_INCLUDE_DIRS})

# 添加公共WebSocket客户端库
add_library(ws_client STATIC
    common/ws_client.cpp
    common/ws_client.h
)
target_link_libraries(ws_client PUBLIC ${LIBWEBSOCKETS_LIBRARIES})

# 添加开发板1的可执行文件
add_executable(board1
    board1/main.cpp
)
target_link_libraries(board1 ws_client)

# 添加开发板2的可执行文件
add_executable(board2
    board2/main.cpp
)
target_link_libraries(board2 ws_client)

# 安装规则（可选）
install(TARGETS board1 board2
    RUNTIME DESTINATION bin
)