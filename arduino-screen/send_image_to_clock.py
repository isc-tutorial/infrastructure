## pip install pyserial pillow
import serial
import time
import argparse
from PIL import Image

# Serial config
BAUD_RATE = 115200

# Matrix size
WIDTH = 32
HEIGHT = 8

# Load and resize image
def load_image(path):
    img = Image.open(path).convert('RGB')
    img = img.resize((WIDTH, HEIGHT))
    return img

# Convert image to raw RGB bytes
def image_to_bytes(img):
    pixels = []
    for y in range(HEIGHT):
        for x in range(WIDTH):
            r, g, b = img.getpixel((x, y))
            pixels.extend([r, g, b])
    return bytes(pixels)

# Send image over serial
def send_image(serial_conn, img_data):
    print("Sending image command...")
    serial_conn.write(b'{"command": "start_image"}\n')
    time.sleep(0.1)  # wait for device to prepare

    print("Sending image bytes...")
    serial_conn.write(img_data)
    serial_conn.flush()
    print("Done.")

# Main
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Send image to LED matrix via serial.')
    parser.add_argument('serial_port', help='Serial port to use (e.g., /dev/ttyUSB1)')
    parser.add_argument('image_path', help='Path to the image file (e.g., image.png)')
    args = parser.parse_args()

    img = load_image(args.image_path)
    img_data = image_to_bytes(img)

    with serial.Serial(args.serial_port, BAUD_RATE, timeout=1) as ser:
        time.sleep(2)  # wait for board reset
        send_image(ser, img_data)
