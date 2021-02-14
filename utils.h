void initTest();

float byte2float(byte value);

byte float2byte(float value);

int scaleToLeds(byte value);

byte getWhite(uint32_t color);

byte getRed(uint32_t color);

byte getGreen(uint32_t color);

byte getBlue(uint32_t color);

bool isBlack(uint32_t color);

int numColors(const uint32_t primary, const uint32_t secondary, const uint32_t tertiary);