#include "animations/Fade.h"
#include "AnimationController.h"

#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <FastLED.h>

#define SLAVE_ADDRESS 8

long last_fastled = 0;
long last_i2c = 0;

SoftwareSerial mySerial(10, 11); // RX, TX

#define LED_PIN 4
#define LEDS_PER_DRUM 24
#define NUMBER_OF_DRUMS 7
#define NUM_LEDS NUMBER_OF_DRUMS * LEDS_PER_DRUM
#define CHIPSET     WS2811
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

AnimationController ac = AnimationController();

typedef struct midi_note {
    uint8_t pitch;
    uint8_t velocity;
} midi_note;


#define PRINTS(s)   { mySerial.print(F(s)); }
#define PRINTSLN(s) { mySerial.println(F(s)); }
#define PRINT(v)  { mySerial.print(v); }

void logNote(byte channel, byte pitch, byte velocity){
   mySerial.print(F("C: "));
   mySerial.print(channel);
   mySerial.print(F(", P: "));
   mySerial.print(pitch);
   mySerial.print(F(", V: "));
   mySerial.println(velocity);
}

void handleNote(byte pitch, byte velocity)
{
    logNote(10, pitch, velocity);
    switch(pitch){
        case(38)://snare
            ac.fire_animation(0);
            return;
        case(48)://tom1
            ac.fire_animation(1);
            return;
        case(36)://bass
            ac.fire_animation(7);
            return;
        case(45)://tom2
            ac.fire_animation(2);
            return;
        case(43)://floor_tom
            ac.fire_animation(3);
            return;
        case(51)://crash
            ac.fire_animation(4);
            return;
        case(49)://ride
            ac.fire_animation(5);
            return;
        case(42):// Hi-Hat Closed
        case(44):// Hi-Hat Pedal 44
        case(46):// Hi-Hat Open 46
        case(23)://Hi-Hat Half-Open 23
            ac.fire_animation(6);
            return;
    }

}

void startup_sequence(int number_of_animations, int delay, int num_cycles){
    for(int j = 0; j < num_cycles; j++){
        int i = 0;
        long next_animation_time = millis();        
        while(i < number_of_animations){
            long now = millis();
            if(now - 12 > last_fastled){
                last_fastled = now;
                ac.show();
                FastLED.show();
            }
            if(now > next_animation_time || next_animation_time - now > delay){
                next_animation_time = next_animation_time + delay;
                ac.fire_animation(i);
                i = i + 1;
            }
        }
    }
}

void setup() {
    Wire.begin();
    mySerial.begin(115200);
    mySerial.println(F("Hello, world?"));  

    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
    FastLED.setBrightness(100);

    Animation* snare = new Fade(leds, LEDS_PER_DRUM, 12, CRGB::Red);
    Animation* tom1 = new Fade(&leds[LEDS_PER_DRUM], LEDS_PER_DRUM, 12, CRGB::Green);
    Animation* bass = new Fade(&leds[LEDS_PER_DRUM + 16], LEDS_PER_DRUM, 12, CRGB::Orange);
    Animation* tom2 = new Fade(&leds[LEDS_PER_DRUM*2 + 8], LEDS_PER_DRUM, 12, CRGB::Blue);
    Animation* floor_tom = new Fade(&leds[LEDS_PER_DRUM*3], LEDS_PER_DRUM, 12, CRGB::Yellow);
    Animation* ride = new Fade(&leds[LEDS_PER_DRUM*4], LEDS_PER_DRUM, 12, CRGB::Pink);
    Animation* crash = new Fade(&leds[LEDS_PER_DRUM*5], LEDS_PER_DRUM, 12, CRGB::SaddleBrown);
    Animation* hi_hat = new Fade(&leds[LEDS_PER_DRUM*6], LEDS_PER_DRUM, 12, CRGB::Purple);

    ac.set(snare, 0);
    ac.set(tom1, 1);
    ac.set(tom2, 2);
    ac.set(floor_tom, 3);
    ac.set(ride, 4);
    ac.set(crash, 5);
    ac.set(hi_hat, 6);
    ac.set(bass, 7);
    ac.set_fps(30);

    startup_sequence(8, 100, 5);
}

void loop() {
    long now_millis = millis();
    if(now_millis - 12 > last_fastled){
        last_fastled = now_millis;
        ac.show();
        FastLED.show();
    }
    long now_micros = micros();
    if(now_micros - 100 >  last_i2c){
        last_i2c = now_micros;
        if(Wire.requestFrom(SLAVE_ADDRESS, sizeof(midi_note))){
            //PRINTSLN("I2C data: ")
            midi_note cur_note = {};
            int success = Wire.readBytes((char *)&cur_note, sizeof(midi_note));
            if(success && cur_note.pitch < 128){
                //logNote(10, cur_note.pitch, cur_note.velocity);
                handleNote(cur_note.pitch, cur_note.velocity);
            }
        } else {
            PRINTSLN("At least Serial is working!");
        }
    } else {
        //PRINTSLN("No i2c this loop");
    }

}