# communicate to esp32 via udp
import socket

ESP32_IP = "192.168.4.1"
ESP32_PORT = 4210

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

while True:
    cmd = input("Enter cmd to send: ")
    sock.sendto(cmd.encode(), (ESP32_IP, ESP32_PORT))
    print(f"Sent: {cmd} to {ESP32_IP}:{ESP32_PORT}")