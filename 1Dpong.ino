#include <Adafruit_NeoPixel.h>
#include <Metro.h>

#define PIN   8
#define LEDS  720

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(LEDS, PIN, NEO_GRB + NEO_KHZ800);


Metro cycle_interval = Metro(20);

void setup() {
    randomSeed(analogRead(A0));
    pixels.begin();
    pixels.show();

}

void loop() {
    if ( cycle_interval.passed() ) {
    }
}
