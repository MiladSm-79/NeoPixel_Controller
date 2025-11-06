// =============================================
// ESP32 NeoPixel Controller – GAMMA CORRECTION + GRADIENT + 17 ANIMATIONS
// • Dual-core, 60+ FPS
// =============================================
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define LED_PIN 13
#define MAX_LEDS 100
#define DEFAULT_LEDS 48

// --- WiFi ---
const char* ssid = "MiladSm";
const char* password = "sm6662123512";
WiFiServer server(80);

// --- NTP ---
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 12600, 60000);

// --- NeoPixel ---
Adafruit_NeoPixel strip(MAX_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
int ledCount = DEFAULT_LEDS;

// --- Modes ---
#define MODE_SOLID 0
#define MODE_RAINBOW 1
#define MODE_NOISE 2
#define MODE_PULSE 3
#define MODE_RAINDROP 4
#define MODE_MIRROR 5
#define MODE_COMET 6
#define MODE_BONFIRE 7
#define MODE_AURORA 8
#define MODE_FIREFLIES 9
#define MODE_OCEAN 10
#define MODE_PLASMA 11
#define MODE_GRADIENT 12
#define MODE_CANDLEFLICKER 13
#define MODE_CLOUD         14
#define MODE_LAVALAMP      15
#define MODE_SUNRISE       16
#define MODE_MATRIXRAIN    17
#define MODE_COLORCYCLE    18

// --- Gradient Buffer ---
uint8_t gradientColors[MAX_LEDS * 3];
bool gradientActive = false;

// --- GAMMA 8-BIT TABLE (0–255) ---
const uint8_t PROGMEM gammaTable[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255
};
#define GAMMA(x) pgm_read_byte(&gammaTable[(x)])

// --- SET PIXEL WITH GAMMA ---
inline void setPixel(uint16_t i, uint8_t r, uint8_t g, uint8_t b) {
  strip.setPixelColor(i, GAMMA(r), GAMMA(g), GAMMA(b));
}

// --- LERP ---
#define LERP(a,b,t) ((a) + (((b)-(a)*(t))>>8))

// --- SIN8 ---
const uint8_t sinTable[256] = {
  128,131,134,137,140,143,146,149,152,155,158,162,165,167,170,173,
  176,179,182,185,188,190,193,196,198,201,203,206,208,211,213,215,
  218,220,222,224,226,228,230,232,234,235,237,239,240,242,243,245,
  246,247,248,249,250,251,252,252,253,254,254,255,255,255,255,255,
  255,255,255,255,255,254,254,253,252,252,251,250,249,248,247,246,
  245,243,242,240,239,237,235,234,232,230,228,226,224,222,220,218,
  215,213,211,208,206,203,201,198,196,193,190,188,185,182,179,176,
  173,170,167,165,162,158,155,152,149,146,143,140,137,134,131,128,
  124,121,118,115,112,109,106,103,100, 97, 94, 90, 87, 85, 82, 79,
   76, 73, 70, 67, 65, 62, 59, 57, 54, 52, 49, 47, 44, 42, 40, 38,
   35, 33, 31, 29, 27, 25, 23, 21, 19, 18, 16, 14, 13, 11, 10,  8,
    7,  6,  5,  4,  3,  2,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  1,  1,  2,  3,  3,  4,  5,  6,  7,  8,  9,
   10, 12, 13, 15, 16, 18, 20, 21, 23, 25, 27, 29, 31, 33, 35, 38,
   40, 42, 44, 47, 49, 52, 54, 57, 59, 62, 65, 67, 70, 73, 76, 79,
   82, 85, 87, 90, 94, 97,100,103,106,109,112,115,118,121,124,127
};
#define SIN8(x) sinTable[(uint8_t)(x)]

// --- Timer ---
class Timer {
  unsigned long last = 0; uint16_t ms;
public:
  Timer(uint16_t m) : ms(m) {}
  bool ready() { unsigned long n = millis(); if (n - last >= ms) { last = n; return true; } return false; }
};

// --- State ---
struct State {
  int mode = MODE_SOLID;
  uint8_t r = 255, g = 120, b = 30;
  bool useSleep = false;
  int sleepStart = 3, sleepEnd = 13;
  float fade = 0; bool fadeUp = true;
  float noisePos = 0;
  int meteorPos = 0; bool meteorActive = false;
  uint16_t rainbow = 0;
  float pulse = 0; bool pulseUp = true; int pulseRange = 24;
  int cometIndex = 0;
  int mirrorStep = 0; bool mirrorFill = true;
  float fireHeat[MAX_LEDS] = {0};
  float auroraA = 0, auroraB = 2.7;
  int starPos[12]; float starBright[12] = {0};
  int meteorDir = 1; unsigned long lastMeteor = 0;
  float ffPos[8] = {0}, ffSpeed[8] = {0}, ffBright[8] = {0};
} s;

// =============================================
// Web Server Task (Core 0)
// =============================================
void webTask(void*) {
  server.begin();
  Serial.printf("Web server started at http://%s\n", WiFi.localIP().toString().c_str());
  for (;;) {
    WiFiClient client = server.available();
    if (!client) { vTaskDelay(1); continue; }

    String req = "";
    unsigned long timeout = millis() + 1500;
    while (millis() < timeout && client.connected()) {
      if (client.available()) {
        char c = client.read();
        req += c;
        if (req.endsWith("\r\n")) break;
      }
    }
    client.flush();

    int lineEnd = req.indexOf("\r\n");
    if (lineEnd >= 0) req = req.substring(0, lineEnd);
    else { client.stop(); vTaskDelay(1); continue; }

    auto get = [&](const String& k) -> String {
      int i = req.indexOf(k + "=");
      if (i < 0) return "";
      int j = req.indexOf("&", i);
      if (j < 0) j = req.length();
      return req.substring(i + k.length() + 1, j);
    };

    // PIXEL COUNT
    String pc = get("pixel_count");
    if (pc.length()) {
      int n = pc.toInt();
      if (n > 0 && n <= MAX_LEDS && n != ledCount) {
        ledCount = n;
        strip.updateLength(ledCount);
        Serial.printf("LED Count updated to %d\n", ledCount);
      }
    }

    // GRADIENT
    String colors = get("colors");
    if (colors.length()) {
      int idx = 0, pos = 0, parsed = 0;
      while (pos < colors.length() && parsed < ledCount) {
        int c1 = colors.indexOf(',', pos);
        if (c1 == -1) break;
        int c2 = colors.indexOf(',', c1 + 1);
        if (c2 == -1) break;
        int pipe = colors.indexOf('|', c2 + 1);
        if (pipe == -1) pipe = colors.length();

        uint8_t r = colors.substring(pos, c1).toInt();
        uint8_t g = colors.substring(c1 + 1, c2).toInt();
        uint8_t b = colors.substring(c2 + 1, pipe).toInt();

        gradientColors[idx++] = r;
        gradientColors[idx++] = g;
        gradientColors[idx++] = b;
        parsed++;
        pos = pipe + 1;
      }

      if (parsed > 0 && parsed < ledCount) {
        uint8_t lastR = gradientColors[(parsed - 1) * 3 + 0];
        uint8_t lastG = gradientColors[(parsed - 1) * 3 + 1];
        uint8_t lastB = gradientColors[(parsed - 1) * 3 + 2];
        while (parsed < ledCount) {
          gradientColors[idx++] = lastR;
          gradientColors[idx++] = lastG;
          gradientColors[idx++] = lastB;
          parsed++;
        }
      }

      gradientActive = true;
      s.mode = MODE_GRADIENT;
      Serial.printf("GRADIENT APPLIED: %d LEDs\n", parsed);
    }

    String m = get("mode"); if (m.length()) s.mode = m.toInt();
 bool colorChanged = false;

String r = get("r");
if (r.length()) { s.r = r.toInt(); colorChanged = true; }

String g = get("g");
if (g.length()) { s.g = g.toInt(); colorChanged = true; }

String b = get("b");
if (b.length()) { s.b = b.toInt(); colorChanged = true; }

if (colorChanged) {
    s.mode = MODE_SOLID;
    gradientActive = false;  
}
    String en = get("enable"); if (en.length()) s.useSleep = en.toInt();
    String st = get("start"); if (st.length()) s.sleepStart = st.toInt();
    String ed = get("end"); if (ed.length()) s.sleepEnd = ed.toInt();

    client.println("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nOK");
    client.stop();
    vTaskDelay(1);
  }
}

// =============================================
// Animation Task (Core 1)
// =============================================
void animTask(void*) {
  strip.begin();
  strip.clear();
  strip.show();
   
   // drawing random star twinkles for Aurora animation
  for (int i = 0; i < 12; i++) s.starPos[i] = random(ledCount);
  for (int i = 0; i < 8; i++) {
    s.ffPos[i] = random(ledCount);
    s.ffSpeed[i] = 0.03 + random(5) * 0.01;
    s.ffBright[i] = random(20, 100) / 100.0;
  }

  for (;;) {
    
    timeClient.update();
    int hr = timeClient.getHours();

    if (s.useSleep && hr >= s.sleepStart && hr < s.sleepEnd) {
      gradientActive = false;          
      s.mode = MODE_SOLID;     
      s.r = s.g = s.b = 0;      
      strip.clear();
      strip.show();
      vTaskDelay(150 / portTICK_PERIOD_MS);
      continue; // skip animations entirely if sleep mode active
    }

    switch (s.mode) {
      case MODE_GRADIENT: applyGradient(); break;
      case MODE_SOLID: solid(); break;
      case MODE_RAINBOW: rainbow(); break;
      case MODE_NOISE: noise(); break;
      case MODE_PULSE: pulse(); break;
      case MODE_RAINDROP: raindrop(); break;
      case MODE_MIRROR: mirror(); break;
      case MODE_COMET: comet(); break;
      case MODE_BONFIRE: bonfire(); break;
      case MODE_AURORA: aurora(); break;
      case MODE_FIREFLIES: fireflies(); break;
      case MODE_OCEAN: ocean(); break;
      case MODE_PLASMA: plasmaWarp(); break;
      case MODE_CANDLEFLICKER: candle(); break;
      case MODE_CLOUD: cloud(); break;
      case MODE_LAVALAMP: lavaLamp(); break;
      case MODE_SUNRISE: sunrise(); break;
      case MODE_MATRIXRAIN: matrixRain(); break;
      case MODE_COLORCYCLE: colorCycle(); break;

      default: solid(); break;
    }

    if (strip.canShow()) strip.show();
    vTaskDelay(1);
  }
}

// =============================================
// APPLY GRADIENT (WITH GAMMA)
// =============================================
void applyGradient() {
  if (!gradientActive) return;
  for (int i = 0; i < ledCount; i++) {
    uint8_t r = gradientColors[i * 3 + 0];
    uint8_t g = gradientColors[i * 3 + 1];
    uint8_t b = gradientColors[i * 3 + 2];
    if (r < 8 && g < 8 && b < 8) {
      setPixel(i, 0, 0, 0);
    } else {
      setPixel(i, r, g, b);
    }
  }
}

// =============================================
// ANIMATIONS 
// =============================================
void solid() {
  static Timer t(100); if (!t.ready()) return;
  for (int i = 0; i < ledCount; i++) setPixel(i, s.r, s.g, s.b);
}

void rainbow() {
  static Timer t(16); 
  if (!t.ready()) return;
  s.rainbow++;
  for (int i = 0; i < ledCount; i++) {
    uint32_t color = wheel((i * 256 / ledCount + s.rainbow) & 255);
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    setPixel(i, r, g, b);
  }
}

void noise() {
  static Timer t(20); if (!t.ready()) return;
  s.noisePos += 0.03;
  uint16_t p = s.noisePos * 40;
  for (int i = 0; i < ledCount; i++) {
    uint8_t n = (SIN8(i * 12 + p) + SIN8(i * 7 - p * 1.5)) >> 1;
    setPixel(i, (s.r * n) >> 8, (s.g * n) >> 8, (s.b * n) >> 8);
  }
}

void pulse() {
  static Timer t(15); if (!t.ready()) return;
  if (s.pulseUp) { s.pulse += 0.02; if (s.pulse >= 1) { s.pulseUp = false; s.pulseRange = random(3, ledCount / 2); } }
  else { s.pulse -= 0.02; if (s.pulse <= 0) s.pulseUp = true; }
  int c = ledCount / 2;
  for (int i = 0; i < ledCount; i++) {
    int d = abs(i - c);
    if (d > s.pulseRange) { setPixel(i, 0, 0, 0); continue; }
    uint8_t v = (1.0 - d / (float)s.pulseRange) * s.pulse * 255;
    setPixel(i, (s.r * v) >> 8, (s.g * v) >> 8, (s.b * v) >> 8);
  }
}

void raindrop() {
  static Timer drop(350), fade(16);
  if (drop.ready()) setPixel(random(ledCount), s.r, s.g, s.b);
  if (fade.ready()) {
    for (int i = 0; i < ledCount; i++) {
      uint32_t c = strip.getPixelColor(i);
      uint8_t r = ((c >> 16) & 0xFF) * 97 / 100;
      uint8_t g = ((c >> 8) & 0xFF) * 97 / 100;
      uint8_t b = (c & 0xFF) * 97 / 100;
      setPixel(i, r, g, b);
    }
  }
}

void mirror() {
  static Timer t(100); if (!t.ready()) return;
  int mid = ledCount / 2;
  if (s.mirrorFill) {
    for (int i = 0; i <= s.mirrorStep && i < mid; i++) {
      setPixel(i, s.r, s.g, s.b);
      setPixel(ledCount - 1 - i, s.r, s.g, s.b);
    }
    if (++s.mirrorStep >= mid) { s.mirrorFill = false; s.mirrorStep = 0; }
  } else {
    strip.clear();
    for (int i = 0; i < mid - s.mirrorStep; i++) {
      setPixel(i, s.r, s.g, s.b);
      setPixel(ledCount - 1 - i, s.r, s.g, s.b);
    }
    if (++s.mirrorStep >= mid) { s.mirrorFill = true; s.mirrorStep = 0; }
  }
}
void comet() {
  static Timer t(50);
  if (!t.ready()) return;

  s.cometIndex = (s.cometIndex + 1) % ledCount;

  strip.clear();

  const int tailLength = 12; 
  const float curve = 2.4f; 

  for (int i = 0; i < ledCount; i++) {

    int d = s.cometIndex - i;
    if (d < 0) d += ledCount;  

    if (d == 0) {

      setPixel(i, s.r, s.g, s.b);
      continue;
    }

    if (d > tailLength) continue;

    float b = pow(1.0f - (float)d / tailLength, curve);
    uint8_t v = (uint8_t)(b * 255.0f);

    setPixel(i,
      (s.r * v) >> 8,
      (s.g * v) >> 8,
      (s.b * v) >> 8
    );
  }
}

void bonfire() {
  static Timer t(80); if (!t.ready()) return;
  int mid = ledCount / 2;
  for (int i = 0; i < ledCount; i++) s.fireHeat[i] = max(0.0f, s.fireHeat[i] - random(0, 30) / 200.0f);
  if (random(255) < 130) { int y = mid + random(-2, 3); if (y >= 0 && y < ledCount) s.fireHeat[y] = min(1.0f, s.fireHeat[y] + random(150, 255) / 255.0f); }
  if (random(100) < 1) { int x = random(ledCount); s.fireHeat[x] = min(1.0f, s.fireHeat[x] + random(120, 255) / 255.0f); }
  for (int i = 0; i < ledCount; i++) {
    int d = abs(i - mid);
    if (d >= 2 && i != mid) {
      int l = i > mid ? i - 1 : i + 1;
      int l2 = i > mid ? i - 2 : i + 2;
      if (l >= 0 && l < ledCount && l2 >= 0 && l2 < ledCount)
        s.fireHeat[i] = (s.fireHeat[l] + s.fireHeat[l2] + s.fireHeat[i] * 0.5) / 2.5;
    }
  }
  for (int i = 0; i < ledCount; i++) {
    float h = s.fireHeat[i];
    uint8_t r, g, b;
    if (h > 0.8) { r = 255; g = map(h * 100, 80, 100, 60, 120); b = map(h * 100, 80, 100, 0, 15); }
    else if (h > 0.45) { r = map(h * 100, 45, 80, 180, 255); g = map(h * 100, 45, 80, 25, 60); b = 0; }
    else { r = map(h * 100, 0, 45, 10, 160); g = 0; b = 0; }
    setPixel(i, r, g, b);
  }
}
void aurora() {
  static Timer t(30); 
  if (!t.ready()) return;

  s.auroraA += 0.006f; 
  s.auroraB += 0.004f; 

  for (int i = 0; i < ledCount; i++) {
    float pos = (float)i / ledCount;

    float wave1 = sin(pos * 4.0f + s.auroraA) * 0.5f + 0.5f;
    float wave2 = sin(pos * 3.2f + s.auroraB + 1.3f) * 0.5f + 0.5f;

    float intensity = (wave1 * 0.6f + wave2 * 0.4f);

    uint8_t r = (uint8_t)(intensity * 60);         
    uint8_t g = (uint8_t)(intensity * 140 + 30);  
    uint8_t b = (uint8_t)(intensity * 200 + 55);  

    float pulse = (sin(millis() * 0.0003f) + 1.0f) * 0.5f;
    pulse = 0.8f + pulse * 0.2f; 
    r = (uint8_t)(r * pulse);
    g = (uint8_t)(g * pulse);
    b = (uint8_t)(b * pulse);

    setPixel(i, r, g, b);
  }

  static uint32_t lastStarUpdate = 0;
  if (millis() - lastStarUpdate > 3000) {
    lastStarUpdate = millis();
    for (int i = 0; i < 5; i++) {
      int pos = random(ledCount);
      setPixel(pos, 40, 80, 120); 
    }
  }
}
void fireflies() {
  static Timer t(16); if (!t.ready()) return;
  strip.clear();
  for (int i = 0; i < 8; i++) {
    s.ffPos[i] += random(2) ? s.ffSpeed[i] : -s.ffSpeed[i];
    if (s.ffPos[i] >= ledCount) s.ffPos[i] -= ledCount;
    if (s.ffPos[i] < 0) s.ffPos[i] += ledCount;
    s.ffBright[i] += random(-2, 3) * 0.01;
    s.ffBright[i] = constrain(s.ffBright[i], 0.2, 1.0);
    int p = s.ffPos[i];
    uint8_t bb = s.ffBright[i] * 255;
    setPixel(p, (s.r * bb) >> 8, (s.g * bb) >> 8, (s.b * bb) >> 8);
  }
}
void candle() {
  static Timer t(60);
  if (!t.ready()) return;
  for (int i = 0; i < ledCount; i++) {
    uint8_t flicker = 180 + random(75);
    setPixel(i, flicker, flicker * 0.45, flicker * 0.1);
  }
}
void cloud() {
  static Timer t(40);
  if (!t.ready()) return;
  float breath = (sin(millis() * 0.0008) + 1) * 0.5;
  breath = pow(breath, 2); // Smooth ease
  uint8_t v = breath * 100;
  for (int i = 0; i < ledCount; i++) {
    float noise = SIN8(i * 8 + millis() * 0.03) / 255.0;
    uint8_t b = v * (0.7 + noise * 0.3);
    setPixel(i, b * 0.3, b * 0.5, b);
  }
}
void lavaLamp() {
  static Timer t(50);
  if (!t.ready()) return;
  static float blob[3] = {0, 0.33, 0.66};
  for (int b = 0; b < 3; b++) {
    blob[b] += 0.003;
    if (blob[b] > 1.0) blob[b] = 0;
  }
  for (int i = 0; i < ledCount; i++) {
    float p = (float)i / ledCount;
    float r = 0, g = 0, b = 0;
    for (int bb = 0; bb < 3; bb++) {
      float d = fabs(p - blob[bb]);
      if (d < 0.2) {
        float intensity = 1.0 - d / 0.2;
        intensity = pow(intensity, 2);
        if (bb == 0) { r += 255 * intensity; }
        if (bb == 1) { g += 180 * intensity; b += 50 * intensity; }
        if (bb == 2) { r += 200 * intensity; g += 100 * intensity; }
      }
    }
    setPixel(i, r, g, b);
  }
}
void sunrise() {
  int mins = timeClient.getMinutes() + timeClient.getHours() * 60;
  float progress = (mins % 1440) / 1440.0; // 0–1 over day
  float sunriseStart = 5.5 / 24.0, sunsetStart = 18.5 / 24.0;
  uint8_t r, g, b;
  if (progress < sunriseStart) {
    float t = (progress / sunriseStart);
    r = 10 + t * 140; g = 5 + t * 80; b = 30 + t * 50;
  } else if (progress > sunsetStart) {
    float t = (progress - sunsetStart) / (1.0 - sunsetStart);
    r = 255 - t * 180; g = 100 - t * 80; b = 30 - t * 20;
  } else {
    r = 255; g = 100; b = 30;
  }
  for (int i = 0; i < ledCount; i++) setPixel(i, r, g, b);
}
void matrixRain() {
  static Timer t(100);
  if (!t.ready()) return;
  static uint8_t drops[MAX_LEDS] = {0};
  for (int i = 0; i < ledCount; i++) {
    if (random(20) == 0) drops[i] = 255;
    if (drops[i] > 0) {
      setPixel(i, 0, drops[i], 0);
      drops[i] = max(0, drops[i] - 15);
    } else {
      setPixel(i, 0, 0, 0);
    }
  }
}
void colorCycle() {
  static Timer t(50);
  if (!t.ready()) return;
  static uint16_t hue = 0;
  hue = (hue + 1) % 768;
  uint32_t rgb = hsvToRgb(hue / 3, 255, 255);
  uint8_t r = (rgb >> 16) & 0xFF;
  uint8_t g = (rgb >> 8) & 0xFF;
  uint8_t b = rgb & 0xFF;
  for (int i = 0; i < ledCount; i++) setPixel(i, r, g, b);
}

uint32_t hsvToRgb(uint8_t h, uint8_t s, uint8_t v) {
  uint8_t region = h / 43;
  uint8_t remainder = (h - (region * 43)) * 6;
  uint8_t p = (v * (255 - s)) >> 8;
  uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
  uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;
  switch (region) {
    case 0: return strip.Color(v, t, p);
    case 1: return strip.Color(q, v, p);
    case 2: return strip.Color(p, v, t);
    case 3: return strip.Color(p, q, v);
    case 4: return strip.Color(t, p, v);
    default: return strip.Color(v, p, q);
  }
}
void ocean() {
    static Timer t(16);
    if (!t.ready()) return;

    const int LAYERS = 4;

    struct Wave {
        float pos, speed, width, amp;
        float phase, wobbleAmp, wobbleFreq;
        bool flash;
        float flashAge, flashFreq, flashPeak;
    };

    static bool init = false;
    static Wave W[LAYERS];

    auto reset = [&](Wave &w) {
        w.pos   = random(-120, -50) / 100.0f;
        w.speed = 0.0023f + random(25) / 18000.0f;

        w.width = 0.06f + random(15) / 160.0f;
        w.amp   = 0.80f + random(40) / 80.0f;

        w.phase      = random(1000) / 1000.0f * TWO_PI;
        w.wobbleAmp  = 0.003f + random(10) / 12000.0f;
        w.wobbleFreq = 0.02f + random(10) / 900.0f;

        w.flash     = false;
        w.flashAge  = 0.0f;
        w.flashFreq = 0.05f + random(25) / 130.0f;
        w.flashPeak = 1.1f + random(60) / 55.0f;
    };

    if (!init) {
        for (int i = 0; i < LAYERS; i++) reset(W[i]);
        init = true;
    }

    for (int i = 0; i < LAYERS; i++) {
        Wave &w = W[i];

        w.pos += w.speed;
        w.pos += sinf(w.phase) * w.wobbleAmp;
        w.phase += w.wobbleFreq;

        if (!w.flash && w.pos > 0.78f) {
            int chance = 2 + (int)((w.pos - 0.78f) * 100);
            if (random(100) < chance) {
                w.flash = true;
                w.flashAge = 0;
            }
        }

        if (w.flash) {
            w.flashAge += w.flashFreq;
            if (w.flashAge >= 1.0f) w.flash = false;
        }

        if (w.pos > 1.45f) reset(w);
    }

    for (int i = 0; i < ledCount; i++) {
        float pos = (float)i / ledCount;

        float base = 100.0f / 255.0f;  

        float r = 0.0f;
        float g = 0.0f;
        float b = 100.0f; 

        float glow = powf(pos, 1.15f);
        r += 6  * glow;
        g += 18 * glow;
        b += 70 * glow;

        for (int w = 0; w < LAYERS; w++) {
            Wave &Wv = W[w];

            float dist = fabsf(pos - Wv.pos);
            if (dist >= Wv.width) continue;

            float x = 1.0f - dist / Wv.width;
            float intensity = x * x * (1.0f - 0.15f * dist);

            float waveR = 10;
            float waveG = 55;
            float waveB = 190;

            r += waveR * intensity * Wv.amp;
            g += waveG * intensity * Wv.amp;
            b += waveB * intensity * Wv.amp;

            // -------------------------------------
            // BIOLUMINESCENCE
            // -------------------------------------
            if (Wv.flash && pos > 0.82f) {
                float loc = (pos - 0.82f) / 0.18f;
                float falloff = powf(1.0f - loc, 3.2f);
                float env = sinf(Wv.flashAge * PI);
                float f = Wv.flashPeak * env * falloff;

                r += 18  * f;
                g += 110 * f;
                b += 255 * f;
            }
        }
        if (b < 100) b = 100;

        setPixel(
            i,
            (uint8_t)constrain(r, 0, 255),
            (uint8_t)constrain(g, 0, 255),
            (uint8_t)constrain(b, 0, 255)
        );
    }
}

void plasmaWarp() {
  static Timer t(16); if (!t.ready()) return;
  static float corePhase = 0, swirlPhase = 0, pulsePhase = 0;
  corePhase += 0.015f;
  swirlPhase += 0.022f;
  pulsePhase += 0.008f;
  float pulse = (sin(pulsePhase) + 1.0f) * 0.5f;
  pulse = 0.7f + pulse * 0.3f;
  for (int i = 0; i < ledCount; i++) {
    float p = (float)i / ledCount;
    float arm1 = sin(p * 8.0f * PI + corePhase + sin(p * 3.0f) * 2.0f);
    float arm2 = sin(p * 10.0f * PI - swirlPhase + cos(p * 4.0f) * 1.5f);
    float field = (arm1 + arm2) * 0.5f;
    field = (field + 2.0f) * 0.25f;
    float turb = 0;
    for (int k = 1; k <= 3; k++) {
      float freq = k * 12.0f;
      float amp = 1.0f / k;
      turb += sin(p * freq * PI + corePhase * k) * amp;
    }
    field += turb * 0.2f;
    field = constrain(field, 0.0f, 1.0f);
    float hue = field * 1.4f;
    uint8_t r, g, b;
    if (hue < 0.3f) { r = s.r * 0.1f; g = s.g * 0.05f; b = s.b * 0.3f; }
    else if (hue < 0.7f) { r = s.r * (0.6f + (hue - 0.3f) * 1.0f); g = s.g * (0.1f + (hue - 0.3f) * 0.5f); b = s.b * (0.4f + (hue - 0.3f) * 1.2f); }
    else if (hue < 1.0f) { float t = (hue - 0.7f) * 3.33f; r = s.r * (1.0f - t * 0.5f); g = s.g * (0.6f + t * 0.4f); b = s.b * (1.0f + t * 0.5f); }
    else { r = g = b = 255; }
    r = r * pulse; g = g * pulse; b = b * pulse;
    setPixel(i, r, g, b);
  }
}

uint32_t wheel(uint8_t p) {
  if (p < 85) return strip.Color(p * 3, 255 - p * 3, 0);
  if (p < 170) { p -= 85; return strip.Color(255 - p * 3, 0, p * 3); }
  p -= 170; return strip.Color(0, p * 3, 255 - p * 3);
}

// =============================================
// Setup
// =============================================
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(200); Serial.print("."); }
  Serial.println("\nWiFi Connected");
  timeClient.begin();
  memset(gradientColors, 0, sizeof(gradientColors));
  gradientActive = false;

  xTaskCreatePinnedToCore(webTask, "Web", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(animTask, "Anim", 8192, NULL, 5, NULL, 1);
}

void loop() {
  vTaskDelay(1000);
}