const int numberOfChannels = numLeds * 3; // Total number of channels you want to receive (1 led = 3 channels)
const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
bool universesReceived[maxUniverses];
bool sendFrame = 1;
int previousDataLength = 0;

void raw_mode2(const uint32_t universe, const uint8_t *data, const uint16_t length)
{
  Serial.print("Raw mode, uni: ");
  Serial.println(universe);
  Serial.print("Raw mode, length: ");
  Serial.println(length);

  const int startUni = 0;
  const int uniSize = 170;
  const int start = (universe - startUni) * uniSize;
  if (start < 0)
  {
    Serial.println("Universe too low, ignoring");
    return;
  }
  if (universe - startUni > 1)
  {
    Serial.println("Universe too big, ignoring");
    return;
  }

  Serial.print("Displaying ");
  Serial.print(start);
  Serial.print(" to ");
  Serial.println(length);

  for (int i = 0; i < min(length, 510); i += 3)
  {
    /*
    Serial.print(i);
    Serial.print(": ");
    Serial.println(data[i]);
    */
    const uint32_t color = leds.Color(data[i], data[i + 1], data[i + 2]); // BRG
    leds.setPixelColor((i / 3) + start, color);
    leds2.setPixelColor((i / 3) + start, color);
  }
  leds.show();
  leds2.show();
}

void raw_mode(const uint32_t universe, const uint8_t *data, const uint16_t length)
{
  sendFrame = 1;
  // set brightness of the whole strip
  /*
  if (universe == 15) {
    leds.setBrightness(data[0]);
    leds.show();
  }
  */
  Serial.print("Raw mode, uni: ");
  Serial.println(universe);

  // Store which universe has got in
  if ((universe - startUniverse) < maxUniverses)
    universesReceived[universe - startUniverse] = 1;

  for (int i = 0; i < maxUniverses; i++)
  {
    if (universesReceived[i] == 0)
    {
      Serial.println("Broke");
      sendFrame = 0;
      break;
    }
  }

  // read universe and put into the right part of the display buffer
  for (int i = 0; i < length / 3; i++)
  {
    int led = i + (universe - startUniverse) * (previousDataLength / 3);
    if (led < numLeds)
      leds.setPixelColor(led, data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
  }
  previousDataLength = length;

  if (sendFrame)
  {
    leds.show();
    // Reset universeReceived to 0
    memset(universesReceived, 0, maxUniverses);
    Serial.println("Showing");
  }
}