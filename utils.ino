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