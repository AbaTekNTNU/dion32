void mode_static(Packet packet) { 
    //leds.fill(packet.primary, 0, numLeds);
    //leds.show();
    //Serial.print("Static filled, r: ");
    //Serial.println(packet.primary_red);

    int numColors = numColors(packet.primary, packet.secondary, packet.tertiary);
    int start = packet.phase;
    leds.Color colors[] = {packet.primary, packet.secondary, packet.tertiary};
    
    for (int i=0; i<packet.groups; i++){
        // Sets the color of each group based on numColors so that the groups alterate between the given colors
        for (int j=0; j<packet.size; j++){
            leds.setPixelColor(start+j, colors[(i%numColors)]); 
        }
        // Sets start to the rigth value for the next group
        start = start + packet.space + packet.size;
        if (start > numLeds) start -= numLeds;
    }
  
    leds.show();
}