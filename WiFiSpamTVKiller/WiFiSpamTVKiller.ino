/* =====================================================================
 *  WiFi Spam + TV Killer        ESP8266 (NodeMCU / Wemos D1 mini)
 * ---------------------------------------------------------------------
 *  - Serves a web UI on its own access point (http://192.168.4.1)
 *  - Blasts 802.11 beacon frames with arbitrary SSIDs, so every phone
 *    in range fills its WiFi list with fake networks
 *  - Drives an IR LED to send TV power codes (switch TVs off)
 * ===================================================================== */

#define SEND_NEC
#define SEND_SONY
#define SEND_RC5
#define SEND_RC6
#define SEND_SAMSUNG
#define SEND_LG
#define SEND_PANASONIC
#define SEND_JVC

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

extern "C" {
  #include "user_interface.h"      // wifi_send_pkt_freedom(), wifi_set_channel()
}

#include "config.h"
#include "ssids.h"
#include "tv_codes.h"
#include "webpage.h"

// ---------------------------------------------------------------------
//  Globals
// ---------------------------------------------------------------------
IRsend irsend(IR_LED_PIN);
ESP8266WebServer server(HTTP_PORT);
DNSServer dns;

bool     spamOn      = false;
uint8_t  spamMode    = 0;             // 0 random 1 funny 2 real 3 rick 4 message
uint16_t spamCount   = 40;
uint16_t spamDelay   = 120;
uint8_t  spamChannel = AP_CHAN;
String   customMsg   = "YOU ARE BEING WATCHED";

std::vector<String> ssidList;

uint8_t  tvQueue[TV_QUEUE_LEN];
uint8_t  tvHead = 0, tvTail = 0;
uint32_t tvNext = 0;

// ---------------------------------------------------------------------
//  Beacon frame template
// ---------------------------------------------------------------------
static const uint8_t BEACON_HEAD[38] = {
  0x80, 0x00,                                             // frame control: beacon
  0x00, 0x00,                                             // duration
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff,                     // DA (broadcast)
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00,                     // SA   (patched)
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00,                     // BSSID(patched)
  0x00, 0x00,                                             // sequence
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,         // timestamp
  0x64, 0x00,                                             // beacon interval (100 TU)
  0x31, 0x04,                                             // capability: ESS + privacy
  0x00, 0x00                                              // SSID tag, length patched
};

static const uint8_t BEACON_TAIL[19] = {
  0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x0c, 0x12, 0x18, 0x24, // supported rates
  0x03, 0x01, 0x01,                                             // DS param, channel patched
  0x05, 0x04, 0x00, 0x01, 0x00, 0x00                            // TIM
};

static uint8_t frame[128];

void sendFakeBeacon(const char* ssid, uint32_t seed, uint8_t ch) {
  size_t n = strlen(ssid);
  if (n > 32) n = 32;
  if (n == 0) return;

  memcpy(frame, BEACON_HEAD, sizeof(BEACON_HEAD));
  frame[37] = (uint8_t)n;                 // SSID length
  memcpy(frame + 38, ssid, n);

  size_t off = 38 + n;
  memcpy(frame + off, BEACON_TAIL, sizeof(BEACON_TAIL));
  frame[off + 12] = ch;                   // DS channel

  // stable pseudo-random BSSID so each fake network keeps its identity
  uint32_t h = seed * 2654435761UL + 0x9E3779B9UL;
  frame[10] = 0x02;
  for (uint8_t i = 1; i < 6; i++) frame[10 + i] = (uint8_t)(h >> (i * 4));
  memcpy(frame + 16, frame + 10, 6);

  wifi_send_pkt_freedom(frame, off + sizeof(BEACON_TAIL), 0);
}

// ---------------------------------------------------------------------
//  SSID list building
// ---------------------------------------------------------------------
String randomSSID() {
  static const char cs[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  uint8_t len = random(6, 12);
  String s;
  for (uint8_t i = 0; i < len; i++) s += cs[random(0, 36)];
  return s;
}

String uniqify(const char* base, uint16_t i, size_t listSize) {
  String s = base;
  if (i >= listSize) s += " " + String(i / listSize + 1);
  return s;
}

void rebuildList() {
  ssidList.clear();
  ssidList.reserve(spamCount);

  switch (spamMode) {
    case 0:
      for (uint16_t i = 0; i < spamCount; i++) ssidList.push_back(randomSSID());
      break;
    case 1:
      for (uint16_t i = 0; i < spamCount; i++) ssidList.push_back(uniqify(FUNNY_SSIDS[i % FUNNY_COUNT], i, FUNNY_COUNT));
      break;
    case 2:
      for (uint16_t i = 0; i < spamCount; i++) ssidList.push_back(uniqify(REAL_SSIDS[i % REAL_COUNT], i, REAL_COUNT));
      break;
    case 3:
      for (uint16_t i = 0; i < spamCount; i++) ssidList.push_back(uniqify(RICK_SSIDS[i % RICK_COUNT], i, RICK_COUNT));
      break;
    case 4: {
      uint16_t idx = 1;
      for (size_t p = 0; p < customMsg.length() && ssidList.size() < spamCount; p += 28) {
        String chunk = customMsg.substring(p, p + 28);
        chunk.trim();
        if (!chunk.length()) continue;
        char pre[4];
        snprintf(pre, sizeof(pre), "%02u", idx++);
        ssidList.push_back(String(pre) + " " + chunk);
      }
      if (ssidList.empty()) ssidList.push_back("HELLO");
      break;
    }
  }
}

// ---------------------------------------------------------------------
//  IR
// ---------------------------------------------------------------------
void sendTvCode(uint8_t idx) {
  if (idx >= TV_COUNT) return;
  const TvCode& c = TV_CODES[idx];
  switch (c.proto) {
    case P_NEC:      irsend.sendNEC(c.code, c.bits); break;
    case P_SONY:     irsend.sendSony(c.code, c.bits); break;
    case P_RC5:      irsend.sendRC5(c.code, c.bits); break;
    case P_RC6:      irsend.sendRC6(c.code, c.bits); break;
    case P_SAMSUNG:  irsend.sendSamsung(c.code, c.bits); break;
    case P_LG:       irsend.sendLG(c.code, c.bits); break;
    case P_PANASONIC:irsend.sendPanasonic64(c.code, c.bits); break;
    case P_JVC:      irsend.sendJVC(c.code, c.bits); break;
  }
}

void queueTv(uint8_t idx) {
  uint8_t next = (tvTail + 1) % TV_QUEUE_LEN;
  if (next == tvHead) return;
  tvQueue[tvTail] = idx;
  tvTail = next;
}

void serviceTv() {
  if (tvHead == tvTail) return;
  if ((int32_t)(millis() - tvNext) < 0) return;
  sendTvCode(tvQueue[tvHead]);
  tvHead = (tvHead + 1) % TV_QUEUE_LEN;
  tvNext = millis() + 170;
}

// ---------------------------------------------------------------------
//  JSON helpers
// ---------------------------------------------------------------------
String jsonEscape(const String& in) {
  String o;
  for (size_t i = 0; i < in.length(); i++) {
    char c = in[i];
    if (c == '"' || c == '\\') o += '\\';
    if ((uint8_t)c < 0x20) continue;
    o += c;
  }
  return o;
}

// ---------------------------------------------------------------------
//  HTTP handlers
// ---------------------------------------------------------------------
void handleState() {
  String j = "{\"spam\":" + String(spamOn ? "true" : "false") +
             ",\"mode\":" + String(spamMode) +
             ",\"count\":" + String(spamCount) +
             ",\"speed\":" + String(spamDelay) +
             ",\"ch\":" + String(spamChannel) +
             ",\"msg\":\"" + jsonEscape(customMsg) + "\",\"ssids\":[";
  size_t shown = ssidList.size() > 80 ? 80 : ssidList.size();
  for (size_t i = 0; i < shown; i++) {
    if (i) j += ',';
    j += '"' + jsonEscape(ssidList[i]) + '"';
  }
  j += "],\"brands\":[";
  for (size_t i = 0; i < TV_COUNT; i++) {
    if (i) j += ',';
    j += '"' + String(TV_CODES[i].brand) + '"';
  }
  j += "]}";
  server.send(200, "application/json", j);
}

void handleUptime() {
  uint32_t s = millis() / 1000;
  char buf[24];
  snprintf(buf, sizeof(buf), "%02u:%02u:%02u", s / 3600, (s / 60) % 60, s % 60);
  server.send(200, "application/json", String("{\"uptime\":\"") + buf + "\"}");
}

void applyChannel(uint8_t ch) {
  if (ch < 1) ch = 1;
  if (ch > 13) ch = 13;
  spamChannel = ch;
  wifi_set_channel(ch);
}

void setupServer() {
  server.on("/", []() {
    server.sendHeader("Cache-Control", "no-store");
    server.send_P(200, "text/html", INDEX_HTML);
  });

  server.on("/api/state",  handleState);
  server.on("/api/uptime", handleUptime);

  server.on("/api/spam", []() {
    spamOn = server.arg("on").toInt() != 0;
    if (spamOn && ssidList.empty()) rebuildList();
    server.send(200, "application/json", "{\"ok\":1}");
  });

  server.on("/api/mode", []() {
    uint8_t m = constrain(server.arg("v").toInt(), 0, 4);
    if (m != spamMode) { spamMode = m; rebuildList(); }
    server.send(200, "application/json", "{\"ok\":1}");
  });

  server.on("/api/count", []() {
    uint16_t c = constrain(server.arg("v").toInt(), 1, MAX_SSIDS);
    if (c != spamCount) { spamCount = c; rebuildList(); }
    server.send(200, "application/json", "{\"ok\":1}");
  });

  server.on("/api/speed", []() {
    spamDelay = constrain(server.arg("v").toInt(), 20, 1000);
    server.send(200, "application/json", "{\"ok\":1}");
  });

  server.on("/api/chan", []() {
    applyChannel(server.arg("v").toInt());
    server.send(200, "application/json", "{\"ok\":1}");
  });

  server.on("/api/msg", []() {
    customMsg = server.arg("v");
    if (customMsg.length() > 180) customMsg = customMsg.substring(0, 180);
    if (spamMode == 4) rebuildList();
    server.send(200, "application/json", "{\"ok\":1}");
  });

  server.on("/api/tv", []() {
    queueTv(server.arg("v").toInt());
    server.send(200, "application/json", "{\"ok\":1}");
  });

  server.on("/api/killall", []() {
    for (size_t i = 0; i < TV_COUNT; i++) queueTv(i);
    server.send(200, "application/json", "{\"ok\":1}");
  });

  server.onNotFound([]() {
    server.sendHeader("Location", "http://192.168.4.1/", true);
    server.send(302, "text/plain", "");
  });

  server.begin();
}

// ---------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(200);

  randomSeed(micros());
  pinMode(IR_LED_PIN, OUTPUT);
  irsend.begin();

  WiFi.persistent(false);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, (strlen(AP_PASS) ? AP_PASS : nullptr), spamChannel, 0, 4);
  WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1),
                    IPAddress(255, 255, 255, 0));
  wifi_set_channel(spamChannel);

  dns.setErrorReplyCode(DNSReplyCode::NoError);
  dns.start(53, "*", IPAddress(192, 168, 4, 1));

  rebuildList();
  setupServer();

  Serial.println();
  Serial.println(F("[WiFi Spam + TV Killer] ready"));
  Serial.print(F("AP: ")); Serial.print(AP_SSID);
  Serial.println(F("  ->  http://192.168.4.1"));
}

void loop() {
  dns.processNextRequest();
  server.handleClient();
  serviceTv();

  if (!spamOn || ssidList.empty()) {
    delay(4);
    return;
  }

  uint32_t t0 = millis();
  for (size_t i = 0; i < ssidList.size(); i++) {
    sendFakeBeacon(ssidList[i].c_str(), i + 1, spamChannel);
    if ((i & 7) == 7) {                 // keep the web UI responsive
      server.handleClient();
      yield();
    }
  }
  uint32_t dt = millis() - t0;
  if (dt < spamDelay) delay(spamDelay - dt);
}
