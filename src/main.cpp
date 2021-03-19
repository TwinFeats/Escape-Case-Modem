#include <Arduino.h>
#define PJON_MAX_PACKETS 4
#define PJON_PACKET_MAX_LENGTH 52
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

ButtonDebounce tone1(PIN_TONE_BUTTON_1, 50);
ButtonDebounce tone2(PIN_TONE_BUTTON_2, 50);
ButtonDebounce tone3(PIN_TONE_BUTTON_3, 50);
ButtonDebounce tone4(PIN_TONE_BUTTON_4, 50);
ButtonDebounce tone5(PIN_TONE_BUTTON_5, 50);
ButtonDebounce tonePlay(PIN_TONE_PLAY, 50);

#define NOTES_LENGTH 15
uint8_t numNotesPlayed = 0;
uint8_t song[NOTES_LENGTH];
int notesPlayed[NOTES_LENGTH];
boolean activated = false;

PJON<SoftwareBitBang> bus(11);

void send(uint8_t *msg, uint8_t len) {
  bus.send(1, msg, len);
  while (bus.update()) {};//wait for send to be completed
}

void send(const char *msg, int len) {
  uint8_t buf[35];
  memcpy(buf, msg, len);
  send(buf, len);
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
    uint8_t msg[NOTES_LENGTH+1];
    msg[0] = 'S';
    memcpy(&msg[1],song,NOTES_LENGTH);
    send(msg, NOTES_LENGTH+1);
  } else if (data[0] == 'W') {  //player has won

  } else if (data[0] == 'L') {  //player has lost

  }
}

void sendLcd(const char *line1, const char *line2) {
  uint8_t msg[35];
  msg[0] = 'L';
  strncpy((char *)&msg[1], line1, 17);
  strncpy((char *)&msg[18], line2, 17);
  send(msg, 35);
}

void sendMp3(int track) {
  uint8_t msg[2];
  msg[0] = 'M';
  msg[1] = track;
  send(msg, 2);
}

void sendTone(uint8_t tone) {
  notesPlayed[numNotesPlayed++] = tone;
  uint8_t msg[2];
  msg[0] = 'T';
  msg[1] = tone;
  send(msg, 2);
  char line2[17];
  for (int i=0;i<numNotesPlayed;i++) {
    sprintf(&line2[i], "%i", (notesPlayed[i]+1));
  }
  sendLcd("Song",line2);
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
  numNotesPlayed = 0;
  for (int i = 0; i < NOTES_LENGTH; i++) {
    if (notesPlayed[i] != song[i]) return;
  }
  sendMp3(TRACK_MODEM_ACQUIRED);
  send("D", 1);
  activated = false;
  digitalWrite(PIN_POWER_LIGHT, LOW);
}

void tone1Pressed(const int state) {
  if (activated && state == LOW && numNotesPlayed < NOTES_LENGTH) {
    sendTone(0);
    checkNotes();
  }
}

void tone2Pressed(const int state) {
  if (activated && state == LOW && numNotesPlayed < NOTES_LENGTH) {
    sendTone(1);
    checkNotes();
  }
}

void tone3Pressed(const int state) {
  if (activated && state == LOW && numNotesPlayed < NOTES_LENGTH) {
    sendTone(2);
    checkNotes();
  }
}

void tone4Pressed(const int state) {
  if (activated && state == LOW && numNotesPlayed < NOTES_LENGTH) {
    sendTone(3);
    checkNotes();
  }
}

void tone5Pressed(const int state) {
  if (activated && state == LOW && numNotesPlayed < NOTES_LENGTH) {
    sendTone(4);
    checkNotes();
  }
}

void tonePlayPressed(const int state) {
  if (activated && state == LOW) {
    send("P", 1);
    numNotesPlayed = 0;
    sendLcd(" ", " ");
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
  Serial.begin(9600);
  delay(2000);  //let Master start first
  randomSeed(analogRead(0));
  initComm();
  initTone();
}

void loop() {
  bus.update();
  bus.receive(750);
  tone1.update();
  tone2.update();
  tone3.update();
  tone4.update();
  tone5.update();
  tonePlay.update();
}