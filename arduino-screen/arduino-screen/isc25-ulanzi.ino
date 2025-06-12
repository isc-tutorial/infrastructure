#include <Arduino.h>
#include <FastLED.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <ArduinoJson.h>

// ======== Matrix Configuration ========
#define LED_PIN     32
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 8
#define NUM_LEDS (MATRIX_WIDTH * MATRIX_HEIGHT)
#define COLOR_ORDER GRB
#define CHIPSET     WS2812
// ======== Buttons ========
#define BTN_LEFT   26
#define BTN_MIDDLE 27
#define BTN_RIGHT  14

CRGB leds[NUM_LEDS];

// Matrix setup (top-left, left to right, row-major)
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(
  MATRIX_WIDTH, MATRIX_HEIGHT, LED_PIN,
  NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
  NEO_GRB + NEO_KHZ800
);

// ======== Text Scroll State ========
String scrollText = "Containers everywhere";
int16_t scrollX = MATRIX_WIDTH;
uint16_t textColor = matrix.Color(0, 178, 233);
unsigned long lastScroll = 0;
int scrollSpeed = 50; // ms per scroll frame
bool scrollEnabled = true;
// ======== Image State and Buffer ========
bool receivingImage = false;
int imageBytesReceived = 0;
uint8_t imageBuffer[768];  // 32x8x3
// ======== Buttons ========
volatile bool btnLeftPressed = false;
volatile bool btnMiddlePressed = false;
volatile bool btnRightPressed = false;
unsigned long lastBtnTime = 0;
const unsigned long debounceDelay = 200;

// ======== Interrupts ========
void IRAM_ATTR handleBtnLeft() {
  if (millis() - lastBtnTime > debounceDelay) {
    btnLeftPressed = true;
    lastBtnTime = millis();
  }
}
void IRAM_ATTR handleBtnMiddle() {
  if (millis() - lastBtnTime > debounceDelay) {
    btnMiddlePressed = true;
    lastBtnTime = millis();
  }
}
void IRAM_ATTR handleBtnRight() {
  if (millis() - lastBtnTime > debounceDelay) {
    btnRightPressed = true;
    lastBtnTime = millis();
  }
}

// ======== Setup ========
void setup() {
  pinMode(15, INPUT_PULLDOWN);
  Serial.begin(115200);
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(40);
  matrix.setTextColor(textColor);
  matrix.fillScreen(0);
  matrix.show();

  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_MIDDLE, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN_LEFT), handleBtnLeft, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN_MIDDLE), handleBtnMiddle, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN_RIGHT), handleBtnRight, FALLING);
}

// ======== Handle JSON Commands ========
void handleJsonCommand(const String& json) {
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, json);

  if (error) {
    Serial.println("JSON Parse Error");
    return;
  }

  const char* cmd = doc["command"];
  if (!cmd) return;

  
  if (strcmp(cmd, "fill_color") == 0) {
    scrollEnabled = false;
    int r = doc["r"] | 0;
    int g = doc["g"] | 0;
    int b = doc["b"] | 0;
    matrix.fillScreen(matrix.Color(r, g, b));
    matrix.show();
  }
  else if (strcmp(cmd, "display_text") == 0) {
    scrollEnabled = false;
    const char* txt = doc["text"] | "";
    int r = doc["r"] | 255;
    int g = doc["g"] | 255;
    int b = doc["b"] | 255;
    matrix.fillScreen(0);
    matrix.setCursor(0, 0);
    matrix.setTextColor(matrix.Color(r, g, b));
    matrix.print(txt);
    matrix.show();
  }
  else if (strcmp(cmd, "scroll_text") == 0) {
    scrollEnabled = true;
    scrollText = doc["text"] | "";
    scrollSpeed = doc["speed"] | 50;
    int r = doc["r"] | 255;
    int g = doc["g"] | 255;
    int b = doc["b"] | 255;
    textColor = matrix.Color(r, g, b);
    scrollX = matrix.width();
  }
  else if (strcmp(cmd, "start_image") == 0) {
    scrollEnabled = false;
    receivingImage = true;
    imageBytesReceived = 0;
    Serial.println("READY_FOR_IMAGE"); // optional confirmation
  }
  else if (strcmp(cmd, "set_brightness") == 0) {
    int brightness = doc["value"] | 40;
    brightness = constrain(brightness, 0, 255);
    matrix.setBrightness(brightness);
    matrix.show();
  }
}


// ======== Read Serial JSON ========
String serialBuffer;

void readSerialJson() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      handleJsonCommand(serialBuffer);
      serialBuffer = "";
    } else {
      serialBuffer += c;
    }
  }
}

// ======== Scroll Text ========
void updateScrollingText() {
  if (!scrollEnabled || scrollText.length() == 0) return;

  unsigned long now = millis();
  if (now - lastScroll >= scrollSpeed) {
    lastScroll = now;
    matrix.fillScreen(0);
    matrix.setCursor(scrollX, 0);
    matrix.setTextColor(textColor);
    matrix.print(scrollText);
    matrix.show();
    scrollX--;
    if (scrollX < -((int)scrollText.length() * 6)) {
      scrollX = matrix.width();  // restart
    }
  }
}

// ======== Read Serial Image ========
void handleImageData() {
  while (Serial.available() && imageBytesReceived < sizeof(imageBuffer)) {
    imageBuffer[imageBytesReceived++] = Serial.read();
  }

  if (imageBytesReceived >= sizeof(imageBuffer)) {
    // Image complete
    receivingImage = false;
    drawImageFromBuffer();
  }
}
// ======== Draw Image to Matrix ========
void drawImageFromBuffer() {
  int index = 0;
  for (int y = 0; y < MATRIX_HEIGHT; y++) {
    for (int x = 0; x < MATRIX_WIDTH; x++) {
      uint8_t r = imageBuffer[index++];
      uint8_t g = imageBuffer[index++];
      uint8_t b = imageBuffer[index++];
      matrix.drawPixel(x, y, matrix.Color(r, g, b));
    }
  }
  matrix.show();
}

// ======== Main Loop ========
void loop() {
  if (btnLeftPressed) {
    btnLeftPressed = false;
    Serial.println("BTN_LEFT");
  }
  if (btnMiddlePressed) {
    btnMiddlePressed = false;
    Serial.println("BTN_MIDDLE");
  }
  if (btnRightPressed) {
    btnRightPressed = false;
    Serial.println("BTN_RIGHT");
  }

  if (receivingImage) {
    handleImageData();
  } else {
    readSerialJson();
    updateScrollingText();
  }
}
