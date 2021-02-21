#include <Arduino.h>
#define ARTNET_ENABLE_WIFI // tell Artnet.h we are using wifi
#define ESP_PLATFORM // tell Artnet.h we are using esp32 and include correct wifi library
#include <Artnet.h>
#include <Adafruit_NeoPixel.h>
#include "utils.h"


#ifdef VSCODE // when building arduino, all ino files are automatically included, this is just a fix for vs code
#include "mode_raw.ino"
#include "wifi-creds.ino"
#endif

// Config Options
// Rename "wifi-creds.ino-example" to "wifi-creds.ino" and add your credentials into the updateWifiCreds() function
char* ssid;
char* password;

IPAddress ip(192, 168, 1, 5);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

const int startUniverse = 0;
const int numLeds = 120;


// static
const byte dataPin = 13;

const byte DIRECTION_POSITIVE = 0;
const byte DIRECTION_CENTER_OUT = 1;
const byte DIRECTION_NEGATIVE = 2;
const byte DIRECTION_CENTER_IN = 3;

const byte MODE_RESET = 0;
const byte MODE_STATIC = 1;
const byte MODE_ROTATE = 2;
const byte MODE_BLOCKS = 3;
const byte MODE_SPARCLE = 4;
const byte MODE_WAVE = 5;
const byte MODE_PUSHER = 6;
const byte MODE_PINGPONG = 7;
const byte MODE_RAW = 255;

Adafruit_NeoPixel leds = Adafruit_NeoPixel(numLeds, dataPin, NEO_GRB + NEO_KHZ800);
ArtnetWiFiReceiver artnet;

struct Packet
{
    int size;
    byte groups;
    int phase;
    int spacing;
    byte state;
    float colorBlending;
    float speed;
    byte direction;
    
    byte primary_red;
    byte primary_green;
    byte primary_blue;
    uint32_t primary;    
    
    byte secondary_red;
    byte secondary_green;
    byte secondary_blue;
    uint32_t secondary;    
    
    byte tertiary_red;
    byte tertiary_green;
    byte tertiary_blue;
    uint32_t tertiary;

    uint32_t activeColors[];
    byte activeColorsLength;

    int totalLeds;
    bool mirror;

    Packet(const byte* data) {
        size = scaleToLeds(data[0]);
        groups = data[1];
        phase = scaleToLeds(data[2]);
        spacing = scaleToLeds(data[3]);
        state = data[4];
        colorBlending = byte2float(data[5]);
        speed = byte2float(data[6]);
        direction = data[7];
        primary_red = data[8];
        primary_green = data[9];
        primary_blue = data[10];
        secondary_red = data[11];
        secondary_green = data[12];
        secondary_blue = data[13];
        tertiary_red = data[14];
        tertiary_green = data[15];
        tertiary_blue = data[16];


        primary = leds.Color(primary_red, primary_green, primary_blue);
        secondary = leds.Color(secondary_red, secondary_green, secondary_blue);
        tertiary = leds.Color(tertiary_red, tertiary_green, tertiary_blue);

        activeColorsLength = getNumColors(primary, secondary, tertiary);

        mirror = direction == DIRECTION_CENTER_OUT || direction == DIRECTION_CENTER_IN;
        totalLeds = numLeds;
        totalLeds = mirror ? totalLeds / 2 : totalLeds;
   }

    int translateCoord(int led) {
        led = (led + phase) % totalLeds;
        if (direction == DIRECTION_CENTER_OUT || direction == DIRECTION_NEGATIVE) {
            led = totalLeds - led;
        }
        return led;
    }

    void setColor(int led, uint32_t color) {
        led = translateCoord(led);
        leds.setPixelColor(led, color);
        if (mirror) {
            led = totalLeds - led;
            led += totalLeds;
            leds.setPixelColor(led, color);
        }
    }

    void setColor(int led, int r, int g, int b) {
        setColor(led, leds.Color(r, g, b));
    }

    void setColor(int led, int r, int g, int b, int w) {
        setColor(led, leds.Color(r, g, b, w));
    }

    uint32_t getActiveColor(int i) {
        uint32_t cocols[] = {primary, secondary, tertiary};
        return cocols[i & activeColorsLength];
    }
};

void mode_reset(Packet packet);
void mode_static(Packet packet);
void mode_rotate(Packet packet);
void mode_blocks(Packet packet);
void mode_sparcle(Packet packet);
void mode_wave(Packet packet);
void mode_pusher(Packet packet);
void mode_pingpong(Packet packet);


typedef void (*ModeFunction) (Packet packet);
ModeFunction modes[] = {mode_reset, mode_static, mode_rotate, mode_blocks, mode_sparcle, mode_wave, mode_pusher, mode_pingpong};

int mode = 0;

void setup() {
    Serial.begin(115200);

    // WiFi stuff
    updateWifiCreds();
    WiFi.begin(ssid, password);
    WiFi.config(ip, gateway, subnet);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.print("WiFi connected, IP = ");
    Serial.println(WiFi.localIP());

    artnet.begin();

    artnet.subscribe(dmx);

    leds.begin();
    initTest();
}

void loop() {
    artnet.parse();  // check if artnet packet has come and execute callback
}

void dmx(const uint32_t universe, const byte* data, const uint16_t length) {
    mode = data[0];
    if (mode == MODE_RAW) {
        raw_mode(universe, data + 1, length - 1);
        return;
    }
    leds.clear();
    for(int i = 1; i < length; i += 9) {
        Serial.print("\nRunning data i: ");
        Serial.print(i);
        Serial.print(", of length: ");
        Serial.println(length);
        draw(data + i);
    }
    leds.show();
}

void draw(const byte* data) {
    const uint16_t x = (data[0]<<8) | data[1];
    const uint16_t width = (data[2] << 8) | data[3];
    const byte r = data[4];
    const byte g = data[5];
    const byte b = data[6];
    const byte mode = data[7];
    const byte mode_data = data[8];

    const uint32_t color = leds.Color(r,g,b);

    leds.fill(color, x, width);
    Serial.print("Drawing color: ");
    Serial.print(color);
    Serial.print(", x: ");
    Serial.print(x);
    Serial.print(", w: ");
    Serial.println(width);
}