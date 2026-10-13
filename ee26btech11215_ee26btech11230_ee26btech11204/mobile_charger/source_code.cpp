#include <WiFi.h>
#include <WebServer.h>

const char* AP_SSID = "USB-Charger";
const char* AP_PASSWORD = "Charge_4325";

IPAddress local_IP(192, 168, 6, 9);
IPAddress gateway(192, 168, 6, 9);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

const int touch12V = 32;
const int touch5V = 33;

const int relayPin = 25;

const int led5V = 26;
const int led12V = 27;

int activeVoltage = 5;

void setVoltage(int voltage)
{
    activeVoltage = voltage;

    if (voltage == 5)
    {
        digitalWrite(relayPin, HIGH);
        digitalWrite(led5V, HIGH);
        digitalWrite(led12V, LOW);
    }
    else
    {
        digitalWrite(relayPin, LOW);
        digitalWrite(led5V, LOW);
        digitalWrite(led12V, HIGH);
    }
}

bool detectDoubleClick(int pin)
{
    static bool lastState[2] = {LOW, LOW};
    static bool waiting[2] = {false, false};
    static unsigned long firstTap[2] = {0, 0};

    int index = (pin == touch12V) ? 0 : 1;

    bool currentState = digitalRead(pin);

    if (currentState == HIGH && lastState[index] == LOW)
    {
        unsigned long now = millis();

        if (!waiting[index])
        {
            firstTap[index] = now;
            waiting[index] = true;
        }
        else
        {
            if (now - firstTap[index] <= 600)
            {
                waiting[index] = false;
                lastState[index] = currentState;
                return true;
            }
            else
            {
                firstTap[index] = now;
            }
        }
    }

    if (waiting[index] &&
        millis() - firstTap[index] > 600)
    {
        waiting[index] = false;
    }

    lastState[index] = currentState;

    return false;
}

void handleRoot()
{
    String page = R"rawliteral(
<!DOCTYPE html>
<html>

<head>
<meta name="viewport" content="width=device-width,initial-scale=1">

<title>USB Charger</title>

<style>

body {
    font-family: Arial;
    text-align: center;
    margin-top: 50px;
}

#voltage {
    font-size: 32px;
    margin: 30px;
}

button {
    width: 180px;
    height: 70px;
    font-size: 24px;
    margin: 15px;
}

</style>
</head>

<body>

<h1>USB Charger</h1>

<div id="voltage">
Current Output: <b>5V</b>
</div>

<button onclick="set5V()">5V</button>

<button onclick="set12V()">12V</button>

<script>

function updateVoltage()
{
    fetch('/status')
    .then(response => response.text())
    .then(voltage => {

        document.getElementById("voltage").innerHTML =
        "Current Output: <b>" + voltage + "V</b>";

    });
}

function set5V()
{
    fetch('/5v');
}

function set12V()
{
    fetch('/12v');
}

setInterval(updateVoltage, 300);

updateVoltage();

</script>

</body>
</html>
)rawliteral";

    server.send(200, "text/html", page);
}

void handleStatus()
{
    server.send(
        200,
        "text/plain",
        String(activeVoltage)
    );
}

void handle5V()
{
    setVoltage(5);
    server.send(200, "text/plain", "5V");
}

void handle12V()
{
    setVoltage(12);
    server.send(200, "text/plain", "12V");
}

void setup()
{
    Serial.begin(115200);

    pinMode(touch12V, INPUT_PULLDOWN);
    pinMode(touch5V, INPUT_PULLDOWN);

    pinMode(relayPin, OUTPUT);
    pinMode(led5V, OUTPUT);
    pinMode(led12V, OUTPUT);

    // Default output
    setVoltage(5);

    // ESP32 hotspot
    WiFi.mode(WIFI_AP);

    WiFi.softAPConfig(
        local_IP,
        gateway,
        subnet
    );

    WiFi.softAP(
        AP_SSID,
        AP_PASSWORD
    );

    server.on("/", handleRoot);
    server.on("/status", handleStatus);
    server.on("/5v", handle5V);
    server.on("/12v", handle12V);

    server.begin();

    Serial.println("USB Charger AP started");
    Serial.print("IP: ");
    Serial.println(WiFi.softAPIP());
}

void loop()
{
    server.handleClient();

    // Left double touch = 12V
    if (detectDoubleClick(touch12V))
    {
        setVoltage(12);
    }

    // Right double touch = 5V
    if (detectDoubleClick(touch5V))
    {
        setVoltage(5);
    }
}
