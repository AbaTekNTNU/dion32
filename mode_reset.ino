void mode_reset(Packet packet) {
    leds.clear();
    leds.show();
    Serial.println("Reset.");
}