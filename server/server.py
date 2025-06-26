# server.py
import asyncio
import websockets
import json

# 连接池 {device_id: websocket}
connections = {}

async def router(websocket, path):
    device_id = None
    try:
        # 注册设备
        msg = await websocket.recv()
        data = json.loads(msg)
        device_id = data['id']
        connections[device_id] = websocket
        print(f"Device {device_id} connected")

        # 持续接收消息
        async for message in websocket:
            data = json.loads(message)
            target = data['target']
            payload = data['data']
            
            if target in connections:
                await connections[target].send(json.dumps({
                    'from': device_id,
                    'data': payload
                }))
                print(f"Routed: {device_id} -> {target}")
            else:
                print(f"Target {target} offline")
                
    except Exception as e:
        print(f"Error: {str(e)}")
    finally:
        if device_id in connections:
            del connections[device_id]
            print(f"Device {device_id} disconnected")

if __name__ == "__main__":
    start_server = websockets.serve(router, "0.0.0.0", 8765, ping_interval=None)
    asyncio.get_event_loop().run_until_complete(start_server)
    print("WebSocket Server running on ws://0.0.0.0:8765")
    asyncio.get_event_loop().run_forever()