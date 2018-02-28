#include <Arduino.h>
#include <MIDI.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#define CIRCULAR_BUFFER_XS
#include <CircularBuffer.h>

SoftwareSerial mySerial(10, 11); // RX, TX
MIDI_CREATE_DEFAULT_INSTANCE();

#define PRINTS(s)   { mySerial.print(F(s)); }
#define PRINTSLN(s) { mySerial.println(F(s)); }
#define PRINT(v)  { mySerial.print(v); }

#define SLAVE_ADDRESS 8

long last = 0;

typedef struct midi_note {
    uint8_t pitch;
    uint8_t velocity;
} midi_note;

CircularBuffer<midi_note,100> notes;

void logNote(byte channel, byte pitch, byte velocity){
   mySerial.print(F("C: "));
   mySerial.print(channel);
   mySerial.print(F(", P: "));
   mySerial.print(pitch);
   mySerial.print(F(", V: "));
   mySerial.println(velocity);
}

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
    //logNoteOn(channel, pitch, velocity);
    struct midi_note new_note = {pitch, velocity};
    notes.push(new_note);

}

void requestEvent(){
    if(!notes.isEmpty()){
        midi_note cur_note = notes.shift();
        Wire.write((uint8_t *)&cur_note, sizeof(midi_note));
    } else {
        midi_note cur_note = {255,255};
        Wire.write((uint8_t *)&cur_note, sizeof(midi_note));
    }
}

void setup() {
    // Connect the handleNoteOn function to the library,
    // so it is called upon reception of a NoteOn.
    MIDI.setHandleNoteOn(handleNoteOn);  // Put only the name of the function

    // Initiate MIDI communications, listen to all channels
    MIDI.begin(10);

    Wire.begin(SLAVE_ADDRESS);
    Wire.onRequest(requestEvent);
    
    mySerial.begin(9600);
    mySerial.println(F("Hello, world?"));    
}

void loop() {
    MIDI.read();  
}
