import socket
import tkinter as tk

ESP32_IP = "192.168.4.1"
ESP32_PORT = 4210

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def send_cmd(cmd):
    sock.sendto(cmd.encode(), (ESP32_IP, ESP32_PORT))
    status_label.config(text=f"Sent: {cmd}")

root = tk.Tk()
root.title("ESP32 UDP Control")
root.geometry("300x150")

# Buttons
btn_on = tk.Button(root, text="BREAK_LIGHTS_ON", font=("Arial", 12),
                   command=lambda: send_cmd("BREAK_LIGHTS_ON"))
btn_on.pack(pady=10)

btn_off = tk.Button(root, text="BREAK_LIGHTS_OFF", font=("Arial", 12),
                    command=lambda: send_cmd("BREAK_LIGHTS_OFF"))
btn_off.pack(pady=5)

btn_on = tk.Button(root, text="HEAD_LIGHTS_ON", font=("Arial", 12),
                   command=lambda: send_cmd("HEAD_LIGHTS_ON"))
btn_on.pack(pady=10)

btn_off = tk.Button(root, text="HEAD_LIGHTS_OFF", font=("Arial", 12),
                    command=lambda: send_cmd("HEAD_LIGHTS_OFF"))
btn_off.pack(pady=5)

status_label = tk.Label(root, text="", fg="green")
status_label.pack(pady=10)

root.mainloop()
