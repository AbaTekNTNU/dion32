void initTest() {
    leds.fill(leds.Color(127, 0, 0), 0, numLeds);
    leds.show();
    delay(500);
    leds.fill(leds.Color(0, 127, 0), 0, numLeds);
    leds.show();
    delay(500);
    leds.fill(leds.Color(0, 0, 127), 0, numLeds);
    leds.show();
    delay(500);
    leds.clear();
    leds.show();
}

float byte2float(byte value) {
    return value / (float) 255;
}

byte float2byte(float value) {
    return (byte) (value * 255);
}

int scaleToLeds(byte value) {
    return float2byte(byte2float(value) * numLeds);
}

bool isBlack(leds.Color color) {
    return (color.r == 0 && color.g == 0 && color.b == 0);
}

int numColors(leds.Color primary, leds.Color secondary, leds.Color tertiary){
    return !isBlack(primary) ? (!isBlack(secondary) ? (!isBlack(tertiary) ? 3 : 2) : 1) 0;
}