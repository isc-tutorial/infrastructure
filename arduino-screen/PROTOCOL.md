# 📄 Ulanzi TC001 Custom Firmware Protocol Documentation

**Firmware Version:** v1.0
**Device:** Ulanzi TC001 (ESP32 + 32x8 WS2812 LED matrix)

---

## 📡 Communication

* **Interface:** Serial over USB
* **Baud Rate:** `115200`
* **Data Format:**

  * JSON-formatted commands, sent as UTF-8 strings, terminated by `\n`
  * Raw binary for image data (see below)

---

## 🔠 JSON Command Format

Each command must be sent as a single line of JSON followed by a newline:

```
{"command": "<command_name>", ...}
```

---

## 🔧 Supported Commands

### 1. `fill_color`

Fills the matrix with a solid color.

```json
{"command": "fill_color", "r": 255, "g": 0, "b": 0}
```

* `r`, `g`, `b`: RGB values (0–255)

---

### 2. `display_text`

Displays static text at position (0, 0).

```json
{"command": "display_text", "text": "Hello", "r": 255, "g": 255, "b": 255}
```

* `text`: string to display
* `r`, `g`, `b`: text color

⛔ Scrolling is **disabled** when this command runs.

---

### 3. `scroll_text`

Scrolls text horizontally **left to right**.

```json
{"command": "scroll_text", "text": "Welcome", "speed": 60, "r": 0, "g": 255, "b": 0}
```

* `text`: string to scroll
* `speed`: ms between scroll steps (lower is faster)
* `r`, `g`, `b`: scroll text color

✔️ This **enables scrolling mode** and runs continuously until overridden.

---

### 4. `set_brightness`

Adjusts display brightness.

```json
{"command": "set_brightness", "value": 120}
```

* `value`: 0–255

---

### 5. `start_image`

Begins raw image transmission mode. You must immediately send **768 bytes** of binary data (RGB pixels).

```json
{"command": "start_image"}
```

#### ➡ Followed by:

* **768 bytes (32 × 8 × 3)** raw RGB pixel data
* Order: **left-to-right**, then **top-to-bottom**
* Each pixel = 3 bytes: R, G, B

---

## 🖱️ Hardware Buttons (GPIO Interrupts)

* Buttons use **internal pull-up resistors** and are **active-low**
* Button presses generate serial output:

| Button | GPIO | Serial Output |
| ------ | ---- | ------------- |
| Left   | 26   | `BTN_LEFT`    |
| Middle | 27   | `BTN_MIDDLE`  |
| Right  | 14   | `BTN_RIGHT`   |

📤 Button states are **debounced** and reported via Serial immediately on press.

---

## 🔄 Behavior Switching

|     Command      | Affects Display |      Cancels Scroll     |
|------------------|-----------------|-------------------------|
| `fill_color`     | ✔               | ✔                       |
| `display_text`   | ✔               | ✔                       |
| `scroll_text`    | ✔               | ✘ (enables scroll mode) |
| `start_image`    | ✔               | ✔                       |
| `set_brightness` | ✔               | ✘                       |

---

## 🧪 Sample Command Flow

1. Set brightness:

```json
{"command": "set_brightness", "value": 100}
```

2. Show static white text:

```json
{"command": "display_text", "text": "Hi!", "r":255, "g":255, "b":255}
```

3. Scroll green message:

```json
{"command": "scroll_text", "text": "Welcome!", "r":0, "g":255, "b":0, "speed":60}
```

4. Draw image:

```json
{"command": "start_image"}
```

→ Send 768 bytes of RGB pixel data

---

## 🧰 Tools

A sample Python script for sending images is available:

```bash
python send_image_to_clock.py
```

