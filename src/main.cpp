#include "animations/Fade.h"
#include "AnimationController.h"

#include <Arduino.h>
#include <MIDI.h>
#include <SoftwareSerial.h>
#include <FastLED.h>

SoftwareSerial mySerial(10, 11); // RX, TX
MIDI_CREATE_DEFAULT_INSTANCE();

#define LED_PIN 4
#define LEDS_PER_DRUM 24
#define NUMBER_OF_DRUMS 1
#define NUM_LEDS NUMBER_OF_DRUMS * LEDS_PER_DRUM
#define CHIPSET     WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

AnimationController ac = AnimationController();

#define PRINTS(s)   { mySerial.print(F(s)); }
#define PRINTSLN(s) { mySerial.println(F(s)); }
#define PRINT(v)  { mySerial.print(v); }

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
   mySerial.print(F("C: "));
   mySerial.print(channel);
   mySerial.print(F(", P: "));
   mySerial.print(pitch);
   mySerial.print(F(", V: "));
   mySerial.println(velocity);
    switch(pitch){
        case(38)://snare
            ac.fire_animation(0);
            return;
        case(48)://tom1
            ac.fire_animation(1);
            return;
        case(45)://tom2
            ac.fire_animation(2);
            return;
        case(43)://floor_tom
            ac.fire_animation(3);
            return;
        case(49)://crash
            ac.fire_animation(4);
            return;
        case(51)://ride
            ac.fire_animation(5);
            return;
    }

}

void printStats(unsigned long start, unsigned long first, unsigned long second, unsigned long last){
    PRINTS("MIDI.read() took ");
    PRINT(first - start);
    PRINTSLN("ms");
    PRINTS("ac.show() took ");
    if(second == 0){
        PRINT(last - first);
        PRINTSLN("ms");
    } else {
        PRINT(second - first);
        PRINTSLN("ms");
        PRINTS("FastLED.show() took ");
        PRINT(last - second);
        PRINTSLN("ms");
    }

}


void setup() {
    // Connect the handleNoteOn function to the library,
    // so it is called upon reception of a NoteOn.
    MIDI.setHandleNoteOn(handleNoteOn);  // Put only the name of the function

    // Initiate MIDI communications, listen to all channels
    MIDI.begin(10);
    
    mySerial.begin(9600);
    mySerial.println(F("Hello, world?"));


    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
    FastLED.setBrightness(255);

    Animation* snare = new Fade(leds, LEDS_PER_DRUM, 16, CRGB(255,0,0));
    Animation* tom1 = new Fade(&leds[LEDS_PER_DRUM], LEDS_PER_DRUM, 16, CRGB(0,255,0));
    Animation* tom2 = new Fade(&leds[LEDS_PER_DRUM*2], LEDS_PER_DRUM, 16, CRGB(0,0,255));
    Animation* floor_tom = new Fade(&leds[LEDS_PER_DRUM*3], LEDS_PER_DRUM, 16, CRGB::Yellow);
    Animation* crash = new Fade(&leds[LEDS_PER_DRUM*4], LEDS_PER_DRUM, 16, CRGB::Pink);
    Animation* ride = new Fade(&leds[LEDS_PER_DRUM*5], LEDS_PER_DRUM, 16, CRGB::SaddleBrown);
    ac.set(snare, 0);
    ac.set(tom1, 1);
    ac.set(tom2, 2);
    ac.set(floor_tom, 3);
    ac.set(crash, 4);
    ac.set(ride, 5);
    ac.set_fps(30);
    
}

void loop() {
    // Call MIDI.read the fastest you can for real-time performance.
    //unsigned long start = millis();
    MIDI.read();
    //unsigned long first = millis();
    ac.show();
    // unsigned long second = millis();
    FastLED.show();
    // unsigned long last = millis();
    // printStats(start, first, second, last);
}
