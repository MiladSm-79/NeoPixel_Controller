#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define LED_PIN D1

// ---- Default LED settings ----
int ledCount = 48;
Adafruit_NeoPixel strip(ledCount, LED_PIN, NEO_GRB + NEO_KHZ800);

// ---- WiFi credentials ----
const char* ssid = ""; // edit this to be your own usid
const char* password = ""; // edit this to be you own wifi password

// ---- Web server ----
WiFiServer server(80);

// ---- LED state ----
bool lightsOn = false;
int currentR = 0, currentG = 255, currentB = 0;

// ---- NTP client (default pool) ----
WiFiUDP ntpUDP;
long ntpOffsetSeconds = 12600; // 3.5 hours default
NTPClient timeClient(ntpUDP, "pool.ntp.org", ntpOffsetSeconds, 60000);

// ---- Sleep window (optional) ----
bool useSleepTimer = true;
int sleepStart = 3;
int sleepEnd = 13;

// ---- Helper Functions ----
void setStripColor(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < ledCount; i++) strip.setPixelColor(i, strip.Color(r, g, b));
  strip.show();
}

bool inSleepTime(int hour) { return (hour >= sleepStart && hour < sleepEnd); }

void updatePixelCount(int newCount) {
  if (newCount != ledCount && newCount > 0) {
    // Clear current LEDs first
    for (int i = 0; i < ledCount; i++) strip.setPixelColor(i, 0);
    strip.show();

    // Update length
    ledCount = newCount;
    strip.updateLength(ledCount);
    strip.begin();

    // Fill with current color (only the new pixels)
    setStripColor(currentR, currentG, currentB);
    lightsOn = true;
  }
}

// ---- URL helper ----
String getParam(String req, const String& param) {
  int idx = req.indexOf(param + "=");
  if (idx == -1) return "";
  int amp = req.indexOf("&", idx);
  if (amp == -1) amp = req.length();
  return req.substring(idx + param.length() + 1, amp);
}

// ---- Setup ----
void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.show();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(250);

  server.begin();
  timeClient.begin();
  timeClient.setTimeOffset(ntpOffsetSeconds);
  timeClient.update();

  Serial.print("Device Ready - IP: ");
  Serial.print(WiFi.localIP());
  Serial.println(":80");

  int hour = timeClient.getHours();
  if (useSleepTimer && inSleepTime(hour)) {
    setStripColor(0, 0, 0);
    lightsOn = false;
  } else {
    setStripColor(currentR, currentG, currentB);
    lightsOn = true;
  }
}

// ---- Loop ----
void loop() {
  timeClient.update();
  int hour = timeClient.getHours();

  WiFiClient client = server.available();
  if (client) {
    String req = client.readStringUntil('\r');
    client.flush();

    if (req.indexOf("/apply") != -1) {
      // Parse color
      String rStr = getParam(req, "r");
      String gStr = getParam(req, "g");
      String bStr = getParam(req, "b");
      if (rStr.length() && gStr.length() && bStr.length()) {
        currentR = rStr.toInt();
        currentG = gStr.toInt();
        currentB = bStr.toInt();
      }

      // Timer settings
      String enableStr = getParam(req, "enable");
      useSleepTimer = (enableStr == "1");

      String startStr = getParam(req, "start");
      String endStr = getParam(req, "end");
      if (startStr.length()) sleepStart = constrain(startStr.toInt(), 0, 23);
      if (endStr.length()) sleepEnd = constrain(endStr.toInt(), 0, 23);

      // Timezone offset
      String offsetStr = getParam(req, "ntp_offset_minutes");
      if (offsetStr.length()) {
        long minutes = offsetStr.toInt();
        ntpOffsetSeconds = minutes * 60L;
        timeClient.setTimeOffset(ntpOffsetSeconds);
        timeClient.forceUpdate();
      }

      // Pixel count
      String pixelStr = getParam(req, "pixel_count");
      if (pixelStr.length()) {
        int count = constrain(pixelStr.toInt(), 1, 512);
        updatePixelCount(count);
      }

      // Apply LED state
      if (useSleepTimer && inSleepTime(hour)) {
        setStripColor(0, 0, 0);
        lightsOn = false;
      } else {
        setStripColor(currentR, currentG, currentB);
        lightsOn = true;
      }
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println("OK");
    client.stop();
  }
}
