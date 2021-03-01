#ifdef VSCODE
#include <Adafruit_NeoPixel.h>
#endif

void playSuccessAnimation()
{
    leds.fill(leds.Color(127, 0, 0), 0, numLeds);
    leds2.fill(leds.Color(127, 0, 0), 0, numLeds);
    leds.show();
    leds2.show();
    delay(500);
    leds.fill(leds.Color(0, 127, 0), 0, numLeds);
    leds2.fill(leds.Color(0, 127, 0), 0, numLeds);
    leds.show();
    leds2.show();
    delay(500);
    leds.fill(leds.Color(0, 0, 127), 0, numLeds);
    leds2.fill(leds.Color(0, 0, 127), 0, numLeds);
    leds.show();
    leds2.show();
    delay(500);
    leds.clear();
    leds2.clear();
    leds.show();
    leds2.show();
}

void playEthernetModuleError()
{
    error(1);
}

void playEthernetCableError()
{
    error(2);
}

void playEthernetNoIPError()
{
    error(3);
}

void playWifiNotConnectedError()
{
    error(4);
}

void error(int num)
{
    while (true)
    {
        leds.fill(leds.Color(50, 0, 0), 0, num);
        leds2.fill(leds.Color(50, 0, 0), 0, num);
        leds.show();
        leds2.show();
        delay(500);

        leds.clear();
        leds2.clear();
        leds.show();
        leds2.show();
        delay(500);
    }
}

float byte2float(byte value)
{
    return ((float)value) / (float)255.0;
}

byte float2byte(float value)
{
    return (byte)(value * 255);
}

int scaleToLeds(byte value)
{
    return byte2float(value) * numLeds;
}

byte getWhite(uint32_t color)
{
    return (color >> 24) && 0xff;
}

byte getRed(uint32_t color)
{
    return (color >> 16) && 0xff;
}

byte getGreen(uint32_t color)
{
    return (color >> 8) && 0xff;
}

byte getBlue(uint32_t color)
{
    return color && 0xff;
}

bool isBlack(uint32_t color)
{
    return getRed(color) == 0 && getGreen(color) == 0 && getBlue(color) == 0;
}

int getNumColors(const uint32_t primary, const uint32_t secondary, const uint32_t tertiary)
{
    return isBlack(secondary) ? 1 : isBlack(tertiary) ? 2
                                                      : 3;
}