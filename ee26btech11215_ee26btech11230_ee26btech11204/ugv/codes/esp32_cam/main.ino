
#include "esp_camera.h"
#include <WiFi.h>
#include <math.h>

// =====================================================
//                 WIFI ACCESS POINT
// =====================================================

const char* AP_SSID = "UGV";
const char* AP_PASSWORD = "UGV_32145";

WiFiServer server(80);
WiFiServer streamServer(81);

// =====================================================
//              ESP32-CAM AI THINKER PINOUT
// =====================================================

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5

#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// =====================================================
//                 L293D MOTOR CONNECTIONS
// =====================================================

// Left motor
const int ENA = 13;
const int IN1 = 12;
const int IN2 = 14;

// Right motor
const int IN3 = 15;
const int IN4 = 2;
const int ENB = 4;

// Right motor physically mirrored
const bool LEFT_MOTOR_INVERTED  = false;
const bool RIGHT_MOTOR_INVERTED = true;

// =====================================================
//                 CONTROL PARAMETERS
// =====================================================

const float DEADZONE = 0.05f;

const int MIN_PWM = 110;
const int MAX_PWM = 255;

const unsigned long WATCHDOG_MS = 250;

unsigned long lastCommandTime = 0;
bool watchdogStopped = false;

int leftPWM = 0;
int rightPWM = 0;

float joystickX = 0;
float joystickY = 0;

// =====================================================
//                 PWM INITIALIZATION
// =====================================================

void setupPWM() {

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  // Use LEDC channels 6 and 7.
  // The camera uses LEDC for its XCLK signal.
  ledcAttachChannel(ENA, 1000, 8, 6);
  ledcAttachChannel(ENB, 1000, 8, 7);

  ledcWrite(ENA, 0);
  ledcWrite(ENB, 0);
}

// =====================================================
//                 MOTOR INITIALIZATION
// =====================================================

void setupMotors() {

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  setupPWM();

  stopMotors();
}

// =====================================================
//                 STOP MOTORS
// =====================================================

void stopMotors() {

  ledcWrite(ENA, 0);
  ledcWrite(ENB, 0);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  leftPWM = 0;
  rightPWM = 0;
}

// =====================================================
//          MINIMUM PWM MAPPING: 110 TO 255
// =====================================================

int magnitudeToPWM(float magnitude) {

  magnitude = fabsf(magnitude);

  if (magnitude <= DEADZONE) {
    return 0;
  }

  magnitude = constrain(magnitude, DEADZONE, 1.0f);

  float mappedPWM =
    MIN_PWM +
    ((magnitude - DEADZONE) *
    (MAX_PWM - MIN_PWM) /
    (1.0f - DEADZONE));

  return constrain(
    (int)roundf(mappedPWM),
    MIN_PWM,
    MAX_PWM
  );
}

// =====================================================
//                 MOTOR DIRECTION
// =====================================================

void setLeftMotor(int command) {

  bool forward = (command >= 0);

  if (LEFT_MOTOR_INVERTED) {
    forward = !forward;
  }

  if (forward) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  } else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  }

  ledcWrite(ENA, abs(command));
}

void setRightMotor(int command) {

  bool forward = (command >= 0);

  if (RIGHT_MOTOR_INVERTED) {
    forward = !forward;
  }

  if (forward) {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  } else {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }

  ledcWrite(ENB, abs(command));
}

// =====================================================
//                 DIFFERENTIAL DRIVE
// =====================================================

void driveJoystick(float x, float y) {

  x = constrain(x, -1.0f, 1.0f);
  y = constrain(y, -1.0f, 1.0f);

  float magnitude = sqrtf(x * x + y * y);

  if (magnitude > 1.0f) {
    x /= magnitude;
    y /= magnitude;
    magnitude = 1.0f;
  }

  joystickX = x;
  joystickY = y;

  if (magnitude <= DEADZONE) {
    stopMotors();
    return;
  }

  // Differential drive mixing
  float leftRaw  = y + x;
  float rightRaw = y - x;

  float maxRaw = fmaxf(
    fabsf(leftRaw),
    fabsf(rightRaw)
  );

  if (maxRaw > 1.0f) {
    leftRaw /= maxRaw;
    rightRaw /= maxRaw;
  }

  float leftCommand  = leftRaw * magnitude;
  float rightCommand = rightRaw * magnitude;

  int leftMagnitude = magnitudeToPWM(leftCommand);
  int rightMagnitude = magnitudeToPWM(rightCommand);

  int leftSigned =
    (leftCommand >= 0)
    ? leftMagnitude
    : -leftMagnitude;

  int rightSigned =
    (rightCommand >= 0)
    ? rightMagnitude
    : -rightMagnitude;

  leftPWM = leftSigned;
  rightPWM = rightSigned;

  if (leftMagnitude == 0) {
    ledcWrite(ENA, 0);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  } else {
    setLeftMotor(leftSigned);
  }

  if (rightMagnitude == 0) {
    ledcWrite(ENB, 0);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  } else {
    setRightMotor(rightSigned);
  }
}

// =====================================================
//                 DIRECTION LABEL
// =====================================================

String getDirection(float x, float y) {

  if (fabsf(x) < 0.05f && fabsf(y) < 0.05f)
    return "CENTER";

  if (fabsf(x) > fabsf(y)) {
    return (x > 0) ? "RIGHT" : "LEFT";
  }

  return (y > 0) ? "FORWARD" : "BACKWARD";
}

// =====================================================
//                 JSON RESPONSE
// =====================================================

String createJSON() {

  String json = "{";

  json += "\"x\":" + String(joystickX, 3) + ",";
  json += "\"y\":" + String(joystickY, 3) + ",";

  json += "\"direction\":\"";
  json += getDirection(joystickX, joystickY);
  json += "\",";

  json += "\"leftPWM\":" + String(leftPWM) + ",";
  json += "\"rightPWM\":" + String(rightPWM) + ",";

  json += "\"leftDuty\":";
  json += String(abs(leftPWM) * 100.0f / 255.0f, 1);
  json += ",";

  json += "\"rightDuty\":";
  json += String(abs(rightPWM) * 100.0f / 255.0f, 1);

  json += "}";

  return json;
}

// =====================================================
//                 HTTP RESPONSE
// =====================================================

void sendResponse(
  WiFiClient& client,
  const String& content,
  const char* contentType
) {

  client.println("HTTP/1.1 200 OK");
  client.print("Content-Type: ");
  client.println(contentType);

  client.println("Cache-Control: no-store");
  client.println("Connection: close");

  client.print("Content-Length: ");
  client.println(content.length());

  client.println();
  client.print(content);
}

// =====================================================
//                 QUERY PARAMETER
// =====================================================

float getParameter(
  const String& request,
  const char* parameter
) {

  String key = String(parameter) + "=";

  int start = request.indexOf(key);

  if (start < 0) {
    return 0.0f;
  }

  start += key.length();

  int end = request.indexOf('&', start);

  if (end < 0) {
    end = request.indexOf(' ', start);
  }

  if (end < 0) {
    end = request.length();
  }

  return request.substring(start, end).toFloat();
}

// =====================================================
//                 CAMERA INITIALIZATION
// =====================================================

bool setupCamera() {

  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;

  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;

  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 12;
  config.fb_count = 2;

  if (psramFound()) {
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.fb_count = 1;
  }

  esp_err_t result = esp_camera_init(&config);

  if (result != ESP_OK) {
    Serial.printf(
      "Camera initialization failed: 0x%x\n",
      result
    );
    return false;
  }

  Serial.println("Camera initialized successfully.");
  return true;
}

// =====================================================
//                 CAMERA STREAM
//                 GET /stream
//                 PORT 81
// =====================================================

void streamTask(void* parameter) {

  const char* boundary = "frame";

  for (;;) {

    WiFiClient client = streamServer.accept();

    if (!client) {
      vTaskDelay(pdMS_TO_TICKS(10));
      continue;
    }

    client.setNoDelay(true);
    client.setTimeout(2000);

    String request = client.readStringUntil('\n');
    request.trim();

    if (!request.startsWith("GET /stream")) {
      client.println("HTTP/1.1 404 Not Found");
      client.println("Connection: close");
      client.println();
      client.stop();
      continue;
    }

    client.println("HTTP/1.1 200 OK");
    client.println(
      "Content-Type: multipart/x-mixed-replace; boundary=frame"
    );
    client.println("Access-Control-Allow-Origin: *");
    client.println("Cache-Control: no-cache");
    client.println("Connection: close");
    client.println();

    while (client.connected() && WiFi.softAPgetStationNum() > 0) {

      camera_fb_t* fb = esp_camera_fb_get();

      if (!fb) {
        Serial.println("Camera frame capture failed.");
        vTaskDelay(pdMS_TO_TICKS(20));
        continue;
      }

      if (fb->format == PIXFORMAT_JPEG) {

        client.printf("--%s\r\n", boundary);

        client.println(
          "Content-Type: image/jpeg"
        );

        client.printf(
          "Content-Length: %u\r\n\r\n",
          (unsigned int)fb->len
        );

        size_t written = client.write(fb->buf, fb->len);

        client.print("\r\n");

        esp_camera_fb_return(fb);

        if (written == 0) {
          break;
        }

      } else {
        esp_camera_fb_return(fb);
      }

      vTaskDelay(pdMS_TO_TICKS(30));
    }

    client.stop();
  }
}

// =====================================================
//                 WEBPAGE
// =====================================================

const char webpage[] PROGMEM = R"rawliteral(

<!DOCTYPE html>
<html>
<head>

<meta name="viewport"
content="width=device-width, initial-scale=1">

<title>UGV Camera Controller</title>

<style>

* {
  box-sizing: border-box;
}

body {
  margin: 0;
  padding: 18px;
  background: #101827;
  color: white;
  font-family: Arial, sans-serif;
  text-align: center;
}

h2 {
  margin-bottom: 5px;
}

.status {
  color: #9caec5;
  margin-bottom: 18px;
}

.camera {
  width: 100%;
  max-width: 480px;
  margin: 15px auto;
  background: #000;
  border: 2px solid #53677f;
  border-radius: 12px;
  overflow: hidden;
}

.camera img {
  width: 100%;
  display: block;
  min-height: 180px;
  object-fit: contain;
}

.camera-label {
  padding: 8px;
  background: #1e2b40;
  color: #55b4ff;
}

.joystick-area {
  display: flex;
  justify-content: center;
  margin: 20px auto;
}

#joystick {
  width: 260px;
  height: 260px;
  background: #26364d;
  border: 3px solid #53677f;
  border-radius: 50%;
  position: relative;
  touch-action: none;
}

#stick {
  width: 76px;
  height: 76px;
  background: #36a3ff;
  border: 4px solid white;
  border-radius: 50%;
  position: absolute;
  left: 50%;
  top: 50%;
  transform: translate(-50%, -50%);
  box-shadow: 0 4px 15px #0008;
}

.values {
  display: flex;
  justify-content: center;
  gap: 15px;
  margin: 15px auto;
  flex-wrap: wrap;
}

.card {
  background: #1e2b40;
  padding: 15px;
  border-radius: 12px;
  width: 145px;
}

.card h3 {
  margin-top: 0;
  color: #55b4ff;
}

.value {
  font-size: 23px;
  font-weight: bold;
}

.bar-bg {
  background: #0d1522;
  height: 12px;
  border-radius: 10px;
  margin-top: 10px;
  overflow: hidden;
}

.bar {
  height: 100%;
  width: 0%;
  background: #36a3ff;
}

.info {
  background: #1e2b40;
  padding: 12px;
  border-radius: 10px;
  max-width: 340px;
  margin: 15px auto;
}

.small {
  font-size: 13px;
  color: #a8b8cc;
}

</style>
</head>

<body>

<h2>UGV CONTROL</h2>

<div class="status">
  ESP32-CAM | Live Camera and Wi-Fi Joystick
</div>

<div class="camera">

  <div class="camera-label">LIVE CAMERA FEED</div>

  <img id="cameraFeed" alt="Camera stream">

</div>

<div class="joystick-area">

  <div id="joystick">
    <div id="stick"></div>
  </div>

</div>

<div class="info">

  <div>Direction</div>
  <div class="value" id="direction">CENTER</div>

  <p class="small">
    X: <span id="x">0.00</span>
    &nbsp; | &nbsp;
    Y: <span id="y">0.00</span>
  </p>

</div>

<div class="values">

  <div class="card">

    <h3>LEFT MOTOR</h3>

    <div class="value" id="leftPWM">0</div>

    <div class="small">Signed PWM</div>

    <div class="bar-bg">
      <div class="bar" id="leftBar"></div>
    </div>

    <p class="small">
      Duty: <span id="leftDuty">0</span>%
    </p>

  </div>

  <div class="card">

    <h3>RIGHT MOTOR</h3>

    <div class="value" id="rightPWM">0</div>

    <div class="small">Signed PWM</div>

    <div class="bar-bg">
      <div class="bar" id="rightBar"></div>
    </div>

    <p class="small">
      Duty: <span id="rightDuty">0</span>%
    </p>

  </div>

</div>

<div class="info">

  <div class="small">
    Deadzone: 5% |
    Minimum PWM: 110 |
    Maximum PWM: 255 |
    Watchdog: 250 ms
  </div>

</div>

<script>

// ======================================
//           CAMERA STREAM
// ======================================

document.getElementById("cameraFeed").src =
  "http://" + location.hostname + ":81/stream";

// ======================================
//           JOYSTICK
// ======================================

const joystick = document.getElementById("joystick");
const stick = document.getElementById("stick");

const radius = 92;

let active = false;
let x = 0;
let y = 0;

let pending = null;
let busy = false;

// ======================================
//           JOYSTICK POSITION
// ======================================

function updatePosition(event) {

  const rect = joystick.getBoundingClientRect();

  let cx = rect.left + rect.width / 2;
  let cy = rect.top + rect.height / 2;

  let dx = event.clientX - cx;
  let dy = cy - event.clientY;

  let nx = dx / radius;
  let ny = dy / radius;

  let mag = Math.sqrt(nx * nx + ny * ny);

  if (mag > 1) {
    nx /= mag;
    ny /= mag;
  }

  x = nx;
  y = ny;

  stick.style.left =
    (50 + nx * radius / rect.width * 100) + "%";

  stick.style.top =
    (50 - ny * radius / rect.height * 100) + "%";

  document.getElementById("x").textContent =
    x.toFixed(2);

  document.getElementById("y").textContent =
    y.toFixed(2);
}

// ======================================
//           REQUEST QUEUE
// ======================================

function queueCommand(nx, ny) {

  pending = {
    x: nx,
    y: ny
  };

  pump();
}

async function pump() {

  if (busy || pending === null) {
    return;
  }

  busy = true;

  const command = pending;
  pending = null;

  try {

    const response = await fetch(
      "/drive?x=" + command.x.toFixed(3) +
      "&y=" + command.y.toFixed(3),
      {
        cache: "no-store"
      }
    );

    if (response.ok) {
      const data = await response.json();
      updateTelemetry(data);
    }

  } catch (error) {
    console.log("Connection error:", error);
  }

  busy = false;

  if (pending !== null) {
    pump();
  }
}

// ======================================
//           TELEMETRY
// ======================================

function updateTelemetry(data) {

  document.getElementById("direction").textContent =
    data.direction;

  document.getElementById("leftPWM").textContent =
    data.leftPWM;

  document.getElementById("rightPWM").textContent =
    data.rightPWM;

  document.getElementById("leftDuty").textContent =
    data.leftDuty;

  document.getElementById("rightDuty").textContent =
    data.rightDuty;

  document.getElementById("leftBar").style.width =
    Math.abs(data.leftPWM) / 255 * 100 + "%";

  document.getElementById("rightBar").style.width =
    Math.abs(data.rightPWM) / 255 * 100 + "%";
}

// ======================================
//           POINTER EVENTS
// ======================================

joystick.addEventListener("pointerdown", function(event) {

  active = true;

  joystick.setPointerCapture(event.pointerId);

  updatePosition(event);

  queueCommand(x, y);
});

joystick.addEventListener("pointermove", function(event) {

  if (!active) return;

  updatePosition(event);
});

joystick.addEventListener("pointerup", releaseJoystick);
joystick.addEventListener("pointercancel", releaseJoystick);

function releaseJoystick() {

  active = false;

  x = 0;
  y = 0;

  stick.style.left = "50%";
  stick.style.top = "50%";

  document.getElementById("x").textContent = "0.00";
  document.getElementById("y").textContent = "0.00";

  queueCommand(0, 0);
}

// ======================================
//           UPDATE FREQUENCY
// ======================================

// Target: 50 updates per second

setInterval(function() {

  if (active) {
    queueCommand(x, y);
  }

}, 20);

</script>

</body>
</html>

)rawliteral";

// =====================================================
//                 HTTP SERVER
// =====================================================

void handleClient() {

  WiFiClient client = server.accept();

  if (!client) {
    return;
  }

  client.setNoDelay(true);
  client.setTimeout(100);

  String request = client.readStringUntil('\n');
  request.trim();

  if (request.length() == 0) {
    client.stop();
    return;
  }

  Serial.print("HTTP REQUEST: ");
  Serial.println(request);

  // ===================================
  //           DRIVE REQUEST
  // ===================================

  if (request.startsWith("GET /drive?")) {

    float x = getParameter(request, "x");
    float y = getParameter(request, "y");

    driveJoystick(x, y);

    lastCommandTime = millis();
    watchdogStopped = false;

    sendResponse(client, createJSON(), "application/json");
  }

  // ===================================
  //           STOP REQUEST
  // ===================================

  else if (request.startsWith("GET /stop")) {

    stopMotors();

    lastCommandTime = millis();
    watchdogStopped = false;

    sendResponse(client, createJSON(), "application/json");
  }

  // ===================================
  //           MAIN WEBPAGE
  // ===================================

  else if (request.startsWith("GET / ")) {

    sendResponse(
      client,
      String(webpage),
      "text/html"
    );
  }

  else {

    sendResponse(client, "Not Found", "text/plain");
  }

  client.flush();
  client.stop();
}

// =====================================================
//                 WATCHDOG
// =====================================================

void checkWatchdog() {

  if (millis() - lastCommandTime >= WATCHDOG_MS) {

    if (!watchdogStopped) {

      stopMotors();

      watchdogStopped = true;

      Serial.println("[WATCHDOG]");
      Serial.println("No recent command. Motors stopped.");
    }
  }
}

// =====================================================
//                 SETUP
// =====================================================

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("================================");
  Serial.println("    ESP32-CAM UGV CONTROLLER");
  Serial.println("================================");

  setupMotors();

  if (!setupCamera()) {
    Serial.println("Camera failed. Check camera connection.");
  }

  WiFi.mode(WIFI_AP);

  WiFi.softAP(AP_SSID, AP_PASSWORD);

  IPAddress IP = WiFi.softAPIP();

  server.begin();
  streamServer.begin();

  lastCommandTime = millis();

  // Separate task for camera streaming.
  // Keeps the 250 ms motor watchdog responsive.
  xTaskCreatePinnedToCore(
    streamTask,
    "CameraStream",
    8192,
    NULL,
    1,
    NULL,
    0
  );

  Serial.println();
  Serial.println("WiFi Access Point Started");

  Serial.print("SSID: ");
  Serial.println(AP_SSID);

  Serial.print("Password: ");
  Serial.println(AP_PASSWORD);

  Serial.print("Controller: http://");
  Serial.println(IP);

  Serial.print("Camera: http://");
  Serial.print(IP);
  Serial.println(":81/stream");

  Serial.println();
  Serial.println("Controller Ready.");
}

// =====================================================
//                 MAIN LOOP
// =====================================================

void loop() {

  checkWatchdog();

  handleClient();

  checkWatchdog();
}

