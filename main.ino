#define UIPETHERNET_DEBUG_UDP

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

IPAddress ip(192, 168, 1, 5);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

const int startUniverse = 0;
const int numLeds = 360;

// static
const byte dataPin = 13;
const byte dataPin2 = 12; // ledstrip uses backup

Adafruit_NeoPixel leds = Adafruit_NeoPixel(numLeds, dataPin, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel leds2 = Adafruit_NeoPixel(numLeds, dataPin2, NEO_GRB + NEO_KHZ800);

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
    Ethernet.begin(mac, ip);
    Serial.println(" - Ethernet: Checking HW status....");
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
        Serial.println("Ethernet shield was not found. Sorry, can't run without hardware. :(");

        while (true)
        {
            delay(1); // do nothing, no point running without Ethernet hardware
        }
    }
    Serial.println(" - Ethernet: Checking Link status....");
    if (Ethernet.linkStatus() == LinkOFF)
    {
        delay(500);
        if (Ethernet.linkStatus() == LinkOFF)
        {
            Serial.println("Ethernet cable is not connected.");
            while (true)
            {
                delay(1); // do nothing, no point running without Ethernet hardware
            }
        }
    }
    Serial.print(" - Ethernet connected, IP = ");
    Serial.println(Ethernet.localIP());

#endif

    Serial.println(" - ArtNet: Starting...");

    artnet.begin();

    Serial.println(" - ArtNet: Subscribing...");

    artnet.subscribe(dmx);

    Serial.println(" - Leds: Beginning...");

    leds.begin();

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
    // The EthernetENC library seems to have a problem with UDP sockets.
    // When the first message from the (ip, port) pair is received the
    // socket gets locked to only receiving packets from that (ip, port).
    // By restarting the socket we can start receiving from other (ip, port)s.
    Serial.println("Restarting ArtNet socket...");
    artnet.stop();
    artnet.begin();
    Serial.println("Restarted socket done.");
#endif

    int numBlobs = data[0];
    byte blobSize = 9;
    if (numBlobs == MODE_RAW)
    {
        raw_mode(universe, data + 1, length - 1);
        return;
    }
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
    const uint16_t width = (data[2] << 8) | data[3];
    const byte r = data[4];
    const byte g = data[5];
    const byte b = data[6];
    const byte mode = data[7];
    const byte mode_data = data[8];

    for (uint16_t i = x; i < x + width; i += 1)
    {
        addColor(i, r, g, b, ((float)i) / ((float)width));
    }
}

void addColor(uint16_t n, byte r, byte g, byte b, float opacity)
{
    uint32_t cur = leds.getPixelColor(n);

    uint16_t old_r = 0xff << 16 & cur;
    uint16_t old_g = 0xff << 8 & cur;
    uint16_t old_b = 0xff & cur;

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
    Serial.println(b);

    uint32_t color = leds.Color(r, g, b);
    leds.setPixelColor(n, color);
    leds2.setPixelColor(n, color);
}