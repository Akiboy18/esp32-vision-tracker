#include <WiFi.h>
#include <Wire.h>
#include <ESP32Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// -----------------------------
// 🔧 I2C Pins (GME12864-50 OLED)
// -----------------------------
#define SDA_PIN 8
#define SCL_PIN 9
#define OLED_ADDR 0x3C  // Change if your I2C scanner shows different

// OLED setup
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// -----------------------------
// 🔧 WiFi Settings
// -----------------------------
const char* ssid = "trojanhorse";
const char* password = "bhuvanesh2011";

WiFiServer server(8080);

// -----------------------------
// 🔧 Servo & Buzzer Pins
// -----------------------------
Servo servoAzimuth;
Servo servoElevation;
const int azPin = 18;
const int elPin = 19;
const int buzzerPin = 3;

// -----------------------------
// 🔧 Global Variables
// -----------------------------
float azimuth = 0, elevation = 0;
int azServoAngle = 90, elServoAngle = 90;
unsigned long buzzerTimer = 0;
bool buzzerOn = false;

void setup() {
  Serial.begin(115200);

  // ✅ Initialize I2C with custom pins
  Wire.begin(SDA_PIN, SCL_PIN);

  // ✅ Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("❌ OLED not found!");
    while (1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("OLED Ready...");
  display.display();

  // ✅ WiFi Connection
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected!");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  server.begin();
  Serial.println("TCP server started on port 8080");

  // ✅ Servo Setup
  servoAzimuth.attach(azPin);
  servoElevation.attach(elPin);
  servoAzimuth.write(90);
  servoElevation.write(90);

  // ✅ Buzzer Setup
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
}

// ---------------------------------------------------------
// Function: Draw hand position on OLED (XY plot)
// ---------------------------------------------------------
void drawGraph(float az, float el) {
  display.clearDisplay();

  // Map az (-30,+30) and el (-20,+20) to 0–128 and 0–64 display coordinates
  int x = map(az, -30, 30, 0, 127);
  int y = map(el, -20, 20, 63, 0);  // invert Y so positive elevation is up

  // Crosshair center
  display.drawLine(63, 0, 63, 63, SSD1306_WHITE);
  display.drawLine(0, 31, 127, 31, SSD1306_WHITE);

  // Hand position dot
  display.fillCircle(x, y, 3, SSD1306_WHITE);

  // Show numeric values
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.printf("Az: %.1f  El: %.1f", az, el);

  display.display();
}

// ---------------------------------------------------------
// Function: Handle buzzer trigger when hand near center
// ---------------------------------------------------------
void handleBuzzer(float az, float el) {
  if (abs(az) < 3 && abs(el) < 3 && !buzzerOn) { // near camera center
    digitalWrite(buzzerPin, HIGH);
    buzzerOn = true;
    buzzerTimer = millis();
  }

  if (buzzerOn && millis() - buzzerTimer > 2000) {
    digitalWrite(buzzerPin, LOW);
    buzzerOn = false;
  }
}

// ---------------------------------------------------------
// MAIN LOOP
// ---------------------------------------------------------
void loop() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("📡 Client connected");
    String data = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          int comma = data.indexOf(',');
          if (comma > 0) {
            azimuth = data.substring(0, comma).toFloat();
            elevation = data.substring(comma + 1).toFloat();

            // Map angles to servo range
            int azServo = map(azimuth, -30, 30, 35, 145);
            int elServo = map(elevation, -20, 20, 35, 145);
            azServo = constrain(azServo, 45, 135);
            elServo = constrain(elServo, 45, 135);

            servoAzimuth.write(azServo);
            servoElevation.write(elServo);

            // Draw on OLED
            drawGraph(azimuth, elevation);

            // Check buzzer
            handleBuzzer(azimuth, elevation);

            Serial.printf("Az: %.2f  El: %.2f  → Servo(%d,%d)\n",
                          azimuth, elevation, azServo, elServo);
          }
          data = "";
        } else {
          data += c;
        }
      }
    }
    client.stop();
    Serial.println("Client disconnected");
  }

  // Update buzzer state even if no data
  handleBuzzer(azimuth, elevation);
}
