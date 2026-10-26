# WiFi Spam + TV Killer

An ESP8266 that hosts its own little website with two buttons:

1. **Fake WiFi spam** — floods the air with 802.11 beacon frames using made-up
   network names. Every phone, laptop and tablet nearby suddenly shows a wall of
   fake networks in Settings → WiFi.
2. **TV killer** — an IR LED that fires power codes, so you can switch off TVs
   without getting up.

## Hardware

| ESP8266 pin | Goes to |
|---|---|
| `D2` (GPIO4) | 100 Ω → base of a 2N2222 / BC547 |
| `5V` (VIN) | IR LED anode → 100 Ω → transistor collector |
| `GND` | transistor emitter |

A bare GPIO can drive a small IR LED directly, but range is bad. Use the
transistor (or a small MOSFET) and you get 5–8 m line of sight.

## Flash

Arduino IDE: install the **esp8266** board package, plus the
**IRremoteESP8266** library, then open `WiFiSpamTVKiller.ino` and upload.

PlatformIO: `pio run -t upload`.

## Use

1. On your phone, connect to the WiFi network `FreeWiFi` (open, no password).
2. A captive portal should pop up. If not, browse to **http://192.168.4.1**.
3. Hit **Spam: OFF** to turn it on, then open your phone's WiFi settings.
4. Point the IR LED at a TV and tap a brand button — or **KILL ALL TVs**.

## Modes

- **Random** — random 6–11 char names
- **Funny** — 50 joke SSIDs
- **Realistic** — router / IoT names that look plausible
- **Rickroll** — the lyrics, spread over ten networks
- **Custom message** — your text, split into 32-byte chunks with an index prefix
  so the phone's alphabetical sort keeps it readable

## Notes

- Beacons only reach devices on the same channel. Channel 1, 6 and 11 are
  selectable — the ESP's own AP moves with it, so re-join after switching.
- Duplicate names merge into one entry on most phones, which is why extra
  copies get a numeric suffix.
- Phones with "WiFi scanning always available" off only refresh the list while
  the settings screen is open. Keep it open.
- IR codes vary between models. If a button does nothing, try another brand —
  many share codes — or add your own to `tv_codes.h`.
- Newer TVs with Bluetooth/RF remotes won't respond to IR at all.

## License

MIT
