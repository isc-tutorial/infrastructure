import argparse
import json
import serial
import threading
import time
import os
from paho.mqtt.client import Client
from PIL import Image

# Constants
BAUD_RATE = 115200
DEFAULT_COLOR = {"r": 0, "g": 178, "b": 233}
BUTTONS = {"BTN_LEFT", "BTN_MIDDLE", "BTN_RIGHT"}
WIDTH, HEIGHT = 32, 8

# Shared state
last_color = DEFAULT_COLOR.copy()
image_whitelist = {
    "decice": "images/decice.png",
}

def send_serial_command(ser, cmd):
    ser.write((json.dumps(cmd) + '\n').encode('utf-8'))

def load_image(path):
    img = Image.open(path).convert('RGB')
    img = img.resize((WIDTH, HEIGHT))
    return img

def image_to_bytes(img):
    pixels = []
    for y in range(HEIGHT):
        for x in range(WIDTH):
            r, g, b = img.getpixel((x, y))
            pixels.extend([r, g, b])
    return bytes(pixels)

def send_image(ser, image_path):
    if not os.path.isfile(image_path):
        print(f"[ERROR] Image file not found: {image_path}")
        return
    try:
        img = load_image(image_path)
        data = image_to_bytes(img)
        if len(data) != 768:
            print("[ERROR] Image must be 768 bytes (32x8 RGB)")
            return
        send_serial_command(ser, {"command": "start_image"})
        time.sleep(0.1)
        ser.write(data)
    except Exception as e:
        print(f"[ERROR] Failed to process image: {e}")

def mqtt_on_connect(client, userdata, flags, rc):
    prefix = userdata["prefix"]
    client.subscribe(f"{prefix}/color")
    client.subscribe(f"{prefix}/message")
    client.subscribe(f"{prefix}/static")
    client.subscribe(f"{prefix}/image")

def mqtt_on_message(client, userdata, msg):
    ser = userdata["serial"]
    global last_color
    topic = msg.topic
    prefix = userdata["prefix"]

    if topic == f"{prefix}/color":
        try:
            r, g, b = map(int, msg.payload.decode().split(","))
            last_color = {"r": r, "g": g, "b": b}
        except Exception as e:
            print(f"[ERROR] Invalid color format: {e}")
    elif topic == f"{prefix}/message":
        text = msg.payload.decode()
        command = {
            "command": "scroll_text",
            "text": text,
            "speed": 60,
            **last_color
        }
        send_serial_command(ser, command)
    elif topic == f"{prefix}/static":
        text = msg.payload.decode()
        command = {
            "command": "display_text",
            "text": text,
            "speed": 60,
            **last_color
        }
        send_serial_command(ser, command)
    elif topic == f"{prefix}/image":
        image_key = msg.payload.decode().strip()
        if image_key in image_whitelist:
            send_image(ser, image_whitelist[image_key])
        else:
            print(f"[ERROR] Image key '{image_key}' not whitelisted")

def serial_reader(ser, mqtt_client, prefix):
    while True:
        try:
            line = ser.readline().decode().strip()
            if line in BUTTONS:
                mqtt_client.publish(f"{prefix}/buttons", line)
        except Exception as e:
            print(f"[ERROR] Serial read error: {e}")
            break


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("serial_port", help="Serial port (e.g., /dev/ttyUSB0)")
    parser.add_argument("mqtt_prefix", help="MQTT topic prefix (e.g., 'clock')")
    parser.add_argument("--mqtt_host", default="localhost", help="MQTT broker host (default: localhost)")
    parser.add_argument("--mqtt_port", type=int, default=1881, help="MQTT broker port (default: 1881)")
    args = parser.parse_args()

    ser = serial.Serial(args.serial_port, BAUD_RATE, timeout=1)
    mqtt_client = Client(userdata={"prefix": args.mqtt_prefix, "serial": ser})
    mqtt_client.on_connect = mqtt_on_connect
    mqtt_client.on_message = mqtt_on_message

    mqtt_client.connect(args.mqtt_host, args.mqtt_port, 60)
    threading.Thread(target=serial_reader, args=(ser, mqtt_client, args.mqtt_prefix), daemon=True).start()
    mqtt_client.loop_forever()

if __name__ == "__main__":
    main()
