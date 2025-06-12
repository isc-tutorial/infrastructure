# Ulanzi TC001 Custom Firmware

A custom firmware for the **Ulanzi TC001** smart clock (ESP32-based with a 32×8 WS2812B LED matrix), enabling **serial-controlled display of colors, text, scrolling messages, and images**, along with **button press detection** via GPIO interrupts.

---

## ✨ Features

- JSON-based command interface over Serial
- Static text display and scrolling text support
- Full RGB background color setting
- Brightness control
- Image rendering via raw RGB data
- Button input support with real-time Serial feedback

---

## ⚙️ How It Works

1. **Connect via USB Serial** (`115200 baud`)
2. Send JSON commands terminated by newline `\n`
3. The firmware interprets commands and updates the LED matrix
4. Hardware buttons (Left, Middle, Right) trigger messages back to the host

---

## 🧰 Dependencies

Make sure the following libraries are installed in your Arduino IDE:

|      Library       |    Arduino Library Manager Name    |
|--------------------|------------------------------------|
| FastLED            | `FastLED`                          |
| Adafruit GFX       | `Adafruit GFX Library`             |
| Adafruit NeoMatrix | `Adafruit NeoMatrix`               |
| ArduinoJson        | `ArduinoJson` (by Benoit Blanchon) |

---

## 🛠️ Hardware Connections

|    Component     |   Pin   |
|------------------|---------|
| LED Matrix (DIN) | GPIO 5  |
| Button Left      | GPIO 26 |
| Button Middle    | GPIO 27 |
| Button Right     | GPIO 14 |

> Buttons are configured as **INPUT_PULLUP**, and trigger on **LOW (pressed)** using interrupts.

---

## 🖥️ Serial Command Format

Commands must be JSON strings terminated by newline (`\n`). See [Protocol Documentation](./PROTOCOL.md) for full command list and formats.

### Example:
```json
{"command": "display_text", "text": "Hi", "r":255, "g":255, "b":0}
````

---

## 🖼️ Sending Images

1. Send:

```json
{"command": "start_image"}
```

2. Follow with **768 bytes** of raw RGB data (32x8 pixels × 3 bytes/pixel)

Use the included [Python script](./send_image_to_clock.py) to automate this process.

---

## 🔘 Button Feedback

| Button          | Output Text  |
| --------------- | ------------ |
| Left (GPIO26)   | `BTN_LEFT`   |
| Middle (GPIO27) | `BTN_MIDDLE` |
| Right (GPIO14)  | `BTN_RIGHT`  |

Messages are sent immediately when a button is pressed (debounced via firmware).

---

## 🧪 Testing

You can test this firmware using:

* Arduino Serial Monitor (for JSON commands and button events)
* Python (for sending images)
* Any serial terminal (115200 baud, newline-delimited)

---

## 📂 File Structure

```
/arduino-screen/
├── arduino-screen/
|   └── isc25-ulanzi.ino      # Main firmware source
├── README.md                 # This file
├── PROTOCOL.md               # Full protocol reference
├── send_image_to_clock.py    # Python script to transmit images
```

---

## 📄 License

MIT License – use freely with attribution.

