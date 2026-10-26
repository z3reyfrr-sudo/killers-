#pragma once

// ---------- Access point you connect your phone to ----------
#define AP_SSID   "FreeWiFi"        // name of the ESP's own network
#define AP_PASS   ""                // "" = open network, otherwise min 8 chars
#define AP_CHAN   1                 // 1, 6 or 11. Beacons are blasted on this channel.

// ---------- Hardware ----------
#define IR_LED_PIN  4               // D2 on NodeMCU / Wemos D1 mini (GPIO4)
#define IR_KHZ      38

// ---------- Limits ----------
#define MAX_SSIDS     100
#define TV_QUEUE_LEN  24
#define HTTP_PORT     80
