#include <Arduino.h>
#define PJON_MAX_PACKETS 2
#define PJON_PACKET_MAX_LENGTH 33
#include <PJONSoftwareBitBang.h>
#include <arduino-timer.h>
#include <ButtonDebounce.h>
#include "../../Escape Room v2 Master/src/tracks.h"
#include <NeoPixelBrightnessBus.h>

// Lock combo is 4219

/*
 * PINS:
 * 2 - Mastermind LED
 * 3 - Blackbox LED (inner)
 * 4 - Audio RX
 * 5 - Audio TX
 * 6 - Panel 1 indicator
 * 7 - Panel 2 indicator
 * 8 - Panel 3 indicator
 * 9 - Panel 4 indicator
 * 10 - Tone OUT
 * 11 - DFPlayer busy in
 * 12 - Color sensor LED
 * 13 - Card reader button
 *
 * 14 -
 * 15 -
 * 16 -
 * 17 -
 *
 * 18 - DFMini TX
 * 19 - DFMini RX
 * 20 SDA - Color sensor
 * 21 SCL - Color sensor
 *
 * 22 - Mastermind Button 1
 * 23 - Mastermind Button 2
 * 24 - Mastermind Button 3
 * 25 - Mastermind Button 4
 * 26 - Mastermind Button 5
 * 27 - Mastermind ENTER
 * 28 - Switch 1
 * 29 - Switch 2
 * 30 - Switch 3
 * 31 - Switch 4
 * 32 - Switch 5
 * 33 - Switch 6
 * 34 - Tone 1 button
 * 35 - Tone 2 button
 * 36 - Tone 3 button
 * 37 - Tone 4 button
 * 38 - Tone 5 button
 * 39 - DEAD
 * 40 - Tone Play
 * 41 - Countdown DIO
 * 42 - LCD D4
 * 43 - Countdown CLK
 * 44 - LCD D5
 * 45 - Blackbox BEAM button
 * 46 - LCD D6
 * 47 - BlackBox GUESS button
 * 48 - LCD D7
 * 49 - Case Open/Close
 * 50 - LCD E
 * 51 - BlackBox place marker button
 * 52 - LCD RS
 * 53 - Blackbox LED (outer)
 *
 * A1 - LED Brightness
 * A2 - Blackbox beam X
 * A3 - Blackbox beam Y
 * A4 - Blackbox marker X
 * A5 - Blackbox marker Y
 * A6 - MP3 volume
 *
 */

/* ----------------- TONES -------------------------*/
#define TONE_BUTTON_1 1
#define TONE_BUTTON_2 2
#define TONE_BUTTON_3 3
#define TONE_BUTTON_4 4
#define TONE_BUTTON_5 5
#define TONE_PLAY 6
#define POWER_LIGHT 7

ButtonDebounce tone1(TONE_BUTTON_1, 100);
ButtonDebounce tone2(TONE_BUTTON_2, 100);
ButtonDebounce tone3(TONE_BUTTON_3, 100);
ButtonDebounce tone4(TONE_BUTTON_4, 100);
ButtonDebounce tone5(TONE_BUTTON_5, 100);
ButtonDebounce tonePlay(TONE_PLAY, 100);

NeoPixelBus<NeoRgbFeature, Neo400KbpsMethod> powerLight(1, POWER_LIGHT);
RgbColor green(0,255,0);

#define NOTES_LENGTH 15
uint8_t numNotesPlayed = 0;
int song[NOTES_LENGTH];
int notesPlayed[NOTES_LENGTH];
boolean activated = false;

PJON<SoftwareBitBang> bus(10);

void send(uint8_t *msg, uint8_t len) {
  bus.send(1, msg, len);
  bus.update();
}

void error_handler(uint8_t code, uint16_t data, void *custom_pointer) {
  if(code == PJON_CONNECTION_LOST) {
    Serial.print("Connection lost with device id ");
    Serial.println(bus.packets[data].content[0], DEC);
  }
}

void commReceive(uint8_t *data, uint16_t len, const PJON_Packet_Info &info) {
  if (data[0] == 'A') {
    activated = true;
    powerLight.SetPixelColor(1, green);
    powerLight.Show();
  }
}

void sendLcd(char *line1, char *line2) {
  uint8_t msg[33];
  msg[0] = 'L';
  strncpy((char *)&msg[1], line1, 16);
  strncpy((char *)&msg[17], line2, 16);
  send(msg, 33);
}

void sendMp3(int track) {
  uint8_t msg[2];
  msg[0] = 'M';
  msg[1] = track;
  send(msg, 2);
}

void sendTone(int tone) {
  uint8_t msg[2];
  msg[0] = 'T';
  msg[1] = tone;
  send(msg, 2);
}

void initComm() {
  bus.strategy.set_pin(13);
  bus.include_sender_info(false);
  bus.set_error(error_handler);
  bus.set_receiver(commReceive);
  bus.begin();
}

void checkNotes() {
  if (numNotesPlayed != NOTES_LENGTH) return;
  for (int i = 0; i < NOTES_LENGTH; i++) {
    if (notesPlayed[i] != song[i]) return;
  }
  sendMp3(TRACK_MODEM_ACQUIRED);
  send((uint8_t *)"D", 1);
  activated = false;
  //indicate done? Turn off ON light?
}

void tone1Pressed(const int state) {
  if (activated && numNotesPlayed < NOTES_LENGTH) {
    sendTone(0);
//    notes[numNotesPlayed++] = 0;
    checkNotes();
  }
}

void tone2Pressed(const int state) {
  if (activated && numNotesPlayed < NOTES_LENGTH) {
    sendTone(1);
//    notes[numNotesPlayed++] = 0;
    checkNotes();
  }
}

void tone3Pressed(const int state) {
  if (activated && numNotesPlayed < NOTES_LENGTH) {
    sendTone(2);
//    notes[numNotesPlayed++] = 0;
    checkNotes();
  }
}

void tone4Pressed(const int state) {
  if (activated && numNotesPlayed < NOTES_LENGTH) {
    sendTone(3);
 //   notes[numNotesPlayed++] = 0;
    checkNotes();
  }
}

void tone5Pressed(const int state) {
  if (activated && numNotesPlayed < NOTES_LENGTH) {
    sendTone(4);
//    notes[numNotesPlayed++] = 0;
    checkNotes();
  }
}

void tonePlayPressed(const int state) {
  if (activated) {
    send((uint8_t *)"P", 1);
  }
}

void initTone() {
  pinMode(TONE_BUTTON_1, INPUT_PULLUP);
  pinMode(TONE_BUTTON_2, INPUT_PULLUP);
  pinMode(TONE_BUTTON_3, INPUT_PULLUP);
  pinMode(TONE_BUTTON_4, INPUT_PULLUP);
  pinMode(TONE_BUTTON_5, INPUT_PULLUP);
  pinMode(TONE_PLAY, INPUT_PULLUP);
  powerLight.Begin();
  powerLight.Show();
  tone1.setCallback(tone1Pressed);
  tone2.setCallback(tone2Pressed);
  tone3.setCallback(tone3Pressed);
  tone4.setCallback(tone4Pressed);
  tone5.setCallback(tone5Pressed);
  tonePlay.setCallback(tonePlayPressed);
  for (int i = 0; i < NOTES_LENGTH; i++) {
    song[i] = random(5);
  }
}
/* --------------END TONES -------------------------*/


void setup() {
  delay(2000);  //let Master start first
  initTone();
}

void loop() {
  bus.update();
  bus.receive();
}