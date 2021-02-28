#include <Arduino.h>
#define PJON_MAX_PACKETS 4
#define PJON_PACKET_MAX_LENGTH 33
#include <PJONSoftwareBitBang.h>
#include <arduino-timer.h>
#include <ButtonDebounce.h>
#include "../../Escape Room v2 Master/src/tracks.h"

// Lock combo is 4219

/* ----------------- TONES -------------------------*/
#define PIN_TONE_BUTTON_1 2
#define PIN_TONE_BUTTON_2 3
#define PIN_TONE_BUTTON_3 4
#define PIN_TONE_BUTTON_4 5
#define PIN_TONE_BUTTON_5 6
#define PIN_TONE_PLAY     7
#define PIN_POWER_LIGHT   8
#define PIN_COMM          13

ButtonDebounce tone1(PIN_TONE_BUTTON_1, 100);
ButtonDebounce tone2(PIN_TONE_BUTTON_2, 100);
ButtonDebounce tone3(PIN_TONE_BUTTON_3, 100);
ButtonDebounce tone4(PIN_TONE_BUTTON_4, 100);
ButtonDebounce tone5(PIN_TONE_BUTTON_5, 100);
ButtonDebounce tonePlay(PIN_TONE_PLAY, 100);

#define NOTES_LENGTH 15
uint8_t numNotesPlayed = 0;
int song[NOTES_LENGTH];
int notesPlayed[NOTES_LENGTH];
boolean activated = false;

PJON<SoftwareBitBang> bus(11);

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
    digitalWrite(PIN_POWER_LIGHT, HIGH);
  } else if (data[0] == 'W') {  //player has won

  } else if (data[0] == 'L') {  //player has lost

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
  bus.strategy.set_pin(PIN_COMM);
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
  digitalWrite(PIN_POWER_LIGHT, LOW);
}

void tone1Pressed(const int state) {
  if (activated && numNotesPlayed < NOTES_LENGTH) {
    sendTone(0);
    checkNotes();
  }
}

void tone2Pressed(const int state) {
  if (activated && numNotesPlayed < NOTES_LENGTH) {
    sendTone(1);
    checkNotes();
  }
}

void tone3Pressed(const int state) {
  if (activated && numNotesPlayed < NOTES_LENGTH) {
    sendTone(2);
    checkNotes();
  }
}

void tone4Pressed(const int state) {
  if (activated && numNotesPlayed < NOTES_LENGTH) {
    sendTone(3);
    checkNotes();
  }
}

void tone5Pressed(const int state) {
  if (activated && numNotesPlayed < NOTES_LENGTH) {
    sendTone(4);
    checkNotes();
  }
}

void tonePlayPressed(const int state) {
  if (activated) {
    send((uint8_t *)"P", 1);
  }
}

void initTone() {
  pinMode(PIN_TONE_BUTTON_1, INPUT_PULLUP);
  pinMode(PIN_TONE_BUTTON_2, INPUT_PULLUP);
  pinMode(PIN_TONE_BUTTON_3, INPUT_PULLUP);
  pinMode(PIN_TONE_BUTTON_4, INPUT_PULLUP);
  pinMode(PIN_TONE_BUTTON_5, INPUT_PULLUP);
  pinMode(PIN_TONE_PLAY, INPUT_PULLUP);
  pinMode(PIN_POWER_LIGHT, OUTPUT);
  digitalWrite(PIN_POWER_LIGHT, LOW);
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
  initComm();
  initTone();
}

void loop() {
  bus.update();
  bus.receive(750);
}