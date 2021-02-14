void mode_static(Packet packet) {

    int start = packet.phase;

    leds.clear();
    
    for (int group = 0; group < packet.groups; group++) {
        for (int led = 0; led < packet.size; led++) {
            packet.setColor(start+led, packet.getActiveColor(led)); 
        }
        start = start + packet.spacing + packet.size;
        if (start > numLeds) start -= numLeds;
    }
  
    leds.show();
}