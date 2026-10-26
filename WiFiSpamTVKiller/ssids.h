#pragma once

// Fake networks used by the "funny" mode
static const char* const FUNNY_SSIDS[] = {
  "Pretty Fly for a WiFi", "Drop it like it's Hotspot", "Tell my WiFi love her",
  "Get off my LAN", "The LAN Before Time", "WiFi Art Thou", "It Hertz When IP",
  "Silence of the LANs", "Router? I hardly know her", "NSA Surveillance Van 3",
  "FBI Van 7", "Free WiFi (not really)", "Pay me for WiFi", "No Internet",
  "Virus Distribution Center", "Ping Me Maybe", "Wu-Tang LAN",
  "Lord of the Pings", "The Ping in the North", "LAN of the Lost",
  "Dunder Mifflin WiFi", "Mom use this one", "Dad use this one",
  "Do not connect", "Error 404", "Blue Screen of Death", "Skynet Global Defense",
  "HAL 9000", "Buffering...", "5G causes COVID", "Bill Wi the Science Fi",
  "Nacho WiFi", "House of the Rising Ping", "This WiFi is Bugged", "IP Freely",
  "Yell PEN15", "The Promised LAN", "Ping Floyd", "Winternet is Coming",
  "LAN Solo", "Obi-WAN Kenobi", "Karen's WiFi", "Your WiFi", "Not Your WiFi",
  "Steal This WiFi", "Definitely Not a Trap", "The Matrix Has You",
  "Follow the White Rabbit", "Wake up Neo", "Beware of the Leopard"
};
static const size_t FUNNY_COUNT = sizeof(FUNNY_SSIDS) / sizeof(FUNNY_SSIDS[0]);

// Networks that look like real routers / phones / IoT junk
static const char* const REAL_SSIDS[] = {
  "FRITZ!Box 7590", "TP-LINK_2.4GHz_A1B2", "NETGEAR53", "NETGEAR53-5G",
  "AndroidAP", "iPhone von Anna", "Vodafone-1A2B3C", "Telekom_FON", "WLAN-Gast",
  "HOME-4F2A", "ASUS_RT-AX88U", "dlink-D8F1", "EasyBox-1A2B3C",
  "Speedport_W 724", "Zyxel_2G", "O2-WLAN-1234", "Unitymedia WLAN",
  "eduroam", "Starbucks WiFi", "IKEA WiFi", "Tesla Model 3",
  "DIRECT-roku-1234", "Chromecast1234", "Nest_Thermostat", "Amazon-Setup",
  "Sonos", "ShellyPlus1-1234", "HP-Print-2B-OfficeJet", "Printer_WLAN",
  "Gastzugang", "Nachbar", "Chef ist doof", "esp8266-1234"
};
static const size_t REAL_COUNT = sizeof(REAL_SSIDS) / sizeof(REAL_SSIDS[0]);

// Rickroll mode
static const char* const RICK_SSIDS[] = {
  "Never Gonna Give You Up", "Never Gonna Let You Down", "Never Gonna Run Around",
  "and Desert You", "Never Gonna Make You Cry", "Never Gonna Say Goodbye",
  "Never Gonna Tell a Lie", "and Hurt You", "Rick Astley", "You Got Rickrolled"
};
static const size_t RICK_COUNT = sizeof(RICK_SSIDS) / sizeof(RICK_SSIDS[0]);
