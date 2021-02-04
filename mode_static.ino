void mode_static(Packet packet) { 
    leds.fill(leds.Color(packet.primary_red, packet.primary_green, packet.primary_blue), 0, numLeds);
    leds.show();
    Serial.print("Static filled, r: ");
    Serial.println(packet.primary_red);
}