//
// MIDI interface using the USB Host intefrace of the Teensy 4.1
//
// Note currently supports a Behringer CDM Studio 2a
//
// John Melton G0ORX
//
#ifndef BEENHERE
#include "SDT.h"
#endif

#ifdef G0ORX_MIDI
#include "USBHost_t36.h"

USBHost myusb;
MIDIDevice midi(myusb);


void OnNoteOn(byte channel, byte note, byte velocity) {
	Serial.print("Note On, ch=");
	Serial.print(channel);
	Serial.print(", note=");
	Serial.print(note);
	Serial.print(", velocity=");
	Serial.print(velocity);
	Serial.println();
  switch(note) {
    case 6: // PITCH BLEND -
      break;
    case 7: // PITCH BLEND +
      break;
    case 36: // UP
      ButtonBandIncrease();
      break;
    case 39: // DOWN
      ButtonBandDecrease();
      break;
  }
}

void OnNoteOff(byte channel, byte note, byte velocity) {
	Serial.print("Note Off, ch=");
	Serial.print(channel);
	Serial.print(", note=");
	Serial.print(note);
	Serial.println();
}

#define COUNTS 5
static int count=0;

void OnControlChange(byte channel, byte control, byte value) {
	//Serial.print("Control Change, ch=");
	//Serial.print(channel);
	//Serial.print(", control=");
	//Serial.print(control);
	//Serial.print(", value=");
	//Serial.print(value);
	//Serial.println();
  switch(control) {
    case 3: // Left large rotory knob
      if(++count==COUNTS) {
      //centerTuneFlag = 1; //AFP 10-03-22
      if (T41State == CW_XMIT && decoderFlag == DECODE_ON) {        // No reason to reset if we're not doing decoded CW AFP 09-27-22
        ResetHistograms();
      }
      centerFreq += (long)freqIncrement * (long)(64-value);
      TxRxFreq = centerFreq + NCOFreq;
      centerTuneFlag = 1;
      count=0;
      }
      break;
    case 5: // left pitch blend rotary
      if(value>=65) { // Clockwise
      
      } else if(value<=63) { // Anticlockwise

      }
      break;
    case 19: // left HIGH 0..127
      break;
    case 20: // left MID 0.127
      break;
    case 21: // left LOW 0..127
      break;
    case 24: // A slider 0..127
      break;
    case 32: // AB Slider 0..127
      break;
    case 33: // Headphones 0..127
      break;
    case 53: // right pitch blend rotary
      if(value>=65) { // Clockwise
      
      } else if(value<=63) { // Anticlockwise

      }
      break;
    case 67: // right HIGH 0..127
      break;
    case 68: // right MID 0.127
      break;
    case 69: // right LOW 0..127
      break;
    case 72: // B Slider 0..127
      break;
  }
}

void MIDI_setup() {
  myusb.begin();
  midi.setHandleNoteOff(OnNoteOff);
	midi.setHandleNoteOn(OnNoteOn);
	midi.setHandleControlChange(OnControlChange);
}

void MIDI_loop() {
  myusb.Task();
	midi.read();
}
#endif
