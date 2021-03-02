#define UIPETHERNET_DEBUG_UDP
// #define RAW
#define ETHERNET_RESET_SOCKET

#define min(a, b) ((a) < (b) ? (a) : (b))
//#define WIFI
#ifdef WIFI

#define ARTNET_ENABLE_WIFI
#ifdef VSCODE
#define ESP_PLATFORM // tell Artnet.h we are using esp32 and include correct wifi library
#endif

#else

#include <EthernetENC.h>
#include <SPI.h>
#undef ESP_PLATFORM
#define ARTNET_ENABLE_ETHER

#endif

#include "artnet/ArtNet.h"
#include <Adafruit_NeoPixel.h>

#ifdef VSCODE // when building arduino, all ino files are automatically included, this is just a fix for vs code
#include "mode_raw.ino"
#include "utils.ino"
#include "wifi-creds.ino"
#endif

// Config Options
// Rename "wifi-creds.ino-example" to "wifi-creds.ino" and add your credentials into the updateWifiCreds() function
char *ssid;
char *password;

IPAddress ip(13, 37, 69, 1);
IPAddress gateway(10, 0, 0, 22);
IPAddress subnet(255, 255, 255, 0);

const int startUniverse = 0;
const int numLeds = 300;

// static
const byte dataPin = 13;
const byte dataPin2 = 33; // ledstrip uses backup

Adafruit_NeoPixel leds = Adafruit_NeoPixel(numLeds, dataPin, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel leds2 = Adafruit_NeoPixel(numLeds, dataPin2, NEO_RGB + NEO_KHZ800);

#ifdef WIFI
ArtnetWiFiReceiver artnet;
#else
ArtnetReceiver artnet;
#endif

void setup()
{
    Serial.begin(115200);

    Serial.println(" - Starting....");

#ifdef WIFI
    Serial.println(" - WiFi: Fetching credentials...");
    updateWifiCreds();
    Serial.println(" - WiFi: Beginning...");
    WiFi.begin(ssid, password);
    Serial.println(" - WiFi: Configuring IPv4...");
    WiFi.config(ip, gateway, subnet);
    Serial.println(" - WiFi: Checking status");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }
    Serial.print("WiFi connected, IP = ");
    Serial.println(WiFi.localIP());
#else

    byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
    Ethernet.init(5);
    Serial.println(" - Ethernet: Beginning...");
    // Ethernet.begin(mac, ip);
    Ethernet.begin(mac, ip, gateway, gateway, subnet);
    Serial.println(" - Ethernet: Checking HW status....");
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
        Serial.println("Ethernet shield was not found. Sorry, can't run without hardware. :(");
        playEthernetModuleError();
    }
    Serial.println(" - Ethernet: Checking Link status....");
    if (Ethernet.linkStatus() == LinkOFF)
    {
        delay(500);
        if (Ethernet.linkStatus() == LinkOFF)
        {
            Serial.println("Ethernet cable is not connected.");
            playEthernetCableError();
        }
    }
    Serial.print(" - Ethernet connected, IP = ");
    Serial.println(Ethernet.localIP());
    Serial.print(" - Ethernet subnet: ");
    Serial.println(Ethernet.subnetMask());
    Serial.print(" - Ethernet gateway: ");
    Serial.println(Ethernet.gatewayIP());

#endif

    Serial.println(" - ArtNet: Starting...");

    artnet.begin();

    Serial.println(" - ArtNet: Subscribing...");

    artnet.subscribe(dmx);

    Serial.println(" - Leds: Beginning...");

    leds.begin();
    leds2.begin();
    leds.clear();
    leds2.clear();

    Serial.println(" - Leds: Success, playing animation....");

    playSuccessAnimation();
}

void loop()
{
    artnet.parse(); // check if artnet packet has come and execute callback
}

const byte MODE_RAW = 255;

void dmx(const uint32_t universe, const byte *data, const uint16_t length)
{
    Serial.println("Running dmx callback");

#ifndef WIFI
#ifdef ETHERNET_RESET_SOCKET
    // The EthernetENC library seems to have a problem with UDP sockets.
    // When the first message from the (ip, port) pair is received the
    // socket gets locked to only receiving packets from that (ip, port).
    // By restarting the socket we can start receiving from other (ip, port)s.
    Serial.println("Restarting ArtNet socket...");
    artnet.stop();
    artnet.begin();
    Serial.println("Restarted socket done.");
#endif
#endif

#ifdef RAW
    raw_mode2(universe, data, length);
#else

    int numBlobs = data[0];
    byte blobSize = 9;
    for (int i = 0; i < length; i += 1)
    {
        Serial.print("i: ");
        Serial.print(i);
        Serial.print(", value: ");
        Serial.println(data[i]);
    }
    uint16_t l = ((length - 1) / blobSize) * blobSize;
    if (l > numBlobs * blobSize)
    {
        l = numBlobs * blobSize;
    }
    leds.clear();
    leds2.clear();
    for (int i = 0; i < l; i += blobSize)
    {
        Serial.print("\nRunning data i: ");
        Serial.print(i);
        Serial.print(", of length: ");
        Serial.println(length);
        draw(data + i + 1);
    }
    leds.show();
    leds2.show();

#endif
    Serial.println("Done running dmx callback");
}

const byte MODE_FADE_RIGHT_LEFT = 0;
const byte MODE_FADE_LEFT_RIGHT = 1;
const byte MODE_FADE_MIDDLE_OUT = 2;
const byte MODE_FADE_OUT_MIDDLE = 3;
const byte MODE_FLAME = 4;
const byte MODE_SPARKLE = 5;

void draw(const byte *data)
{
    const uint16_t x = (data[0] << 8) | data[1];
    const uint16_t w = (data[2] << 8) | data[3];
    const byte r = data[4];
    const byte g = data[5];
    const byte b = data[6];
    const byte mode = data[7];
    const byte mode_data = data[8];

    switch (mode)
    {
    case MODE_FADE_RIGHT_LEFT:
    case MODE_FADE_LEFT_RIGHT:
    case MODE_FADE_MIDDLE_OUT:
    case MODE_FADE_OUT_MIDDLE:
    {
        boolean negative = (mode == MODE_FADE_RIGHT_LEFT) || (mode == MODE_FADE_OUT_MIDDLE);
        boolean mirror = (mode == MODE_FADE_MIDDLE_OUT) || (mode == MODE_FADE_OUT_MIDDLE);
        int width = mirror ? w / 2 : w;

        for (uint16_t i = x; i < x + width; i += 1)
        {
            float opacity = ((float)(i - x + 1)) / ((float)width);
            addColor(x, i, x + width, r, g, b, opacity, negative, mirror);
        }
        break;
    }
    case MODE_FLAME:
        Serial.println("Flame mode");
        break;
    case MODE_SPARKLE:
        Serial.println("Sparkle mode");
        break;
    }
}

void addColor(int start, int i, int end, byte r, byte g, byte b, float opacity, boolean negative, byte mirror)
{
    const int width = end - start;
    if (negative)
    {
        i = end - (i - start);
    }
    addColor(i, r, g, b, opacity);
    if (mirror)
    {
        start += width - 1;
        i += width - 1;
        end += width - 1;
        i = end - (i - start);
        addColor(i, r, g, b, opacity);
    }
}

void addColor(uint16_t n, byte r, byte g, byte b, float opacity)
{
    uint32_t cur = leds.getPixelColor(n);

    byte old_r = (0xff << 16 & cur) >> 16;
    byte old_g = (0xff << 8 & cur) >> 8;
    byte old_b = 0xff & cur;

    r = (byte)(((float)r) * opacity);
    r = min(r + old_r, 255);
    g = (byte)(((float)g) * opacity);
    g = min(g + old_g, 255);
    b = (byte)(((float)b) * opacity);
    b = min(b + old_b, 255);

    Serial.print("n: ");
    Serial.print(n);
    Serial.print(", r:");
    Serial.print(r);
    Serial.print(", g:");
    Serial.print(g);
    Serial.print(", b:");
    Serial.print(b);

    Serial.print(", old; r:");
    Serial.print(old_r);
    Serial.print(", g:");
    Serial.print(old_g);
    Serial.print(", b:");
    Serial.print(old_b);

    Serial.print(", cur: ");
    Serial.println(cur);

    uint32_t color = leds.Color(r, g, b);
    leds.setPixelColor(n, color);
    leds2.setPixelColor(n, color);
}