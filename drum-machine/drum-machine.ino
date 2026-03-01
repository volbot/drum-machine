/*
  Drum Machine

  Very basic drum machine code for an Arduino board.
  Requires a potentiometer and LEDs.

  allomyrina volbot <tech@volbot.org>
*/

#include <LiquidCrystal.h>
#include <Vector.h>

int beat_increment;

const int rs = 13, en = 12, d4 = 11, d5 = 10, d6 = 9, d7 = 8;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int latch_pin = 5;
int clock_pin = 6;
int data_pin = 4;
byte leds_1 = 0;
byte leds_2 = 0;
const int CACHE_MAX = 5;
int storage_array[CACHE_MAX];
Vector<int> vector(storage_array);

void updateShiftRegisters() {
  digitalWrite(latch_pin, LOW);
  shiftOut(data_pin, clock_pin, LSBFIRST, leds_1);
  shiftOut(data_pin, clock_pin, LSBFIRST, leds_2);
  digitalWrite(latch_pin, HIGH);
}

void setup() {
  // shift register setup
  pinMode(latch_pin, OUTPUT);
  pinMode(clock_pin, OUTPUT);
  pinMode(data_pin, OUTPUT);
  digitalWrite(clock_pin, 0);
  digitalWrite(data_pin, 0);
  // lcd setup
  lcd.begin(16, 2);
  lcd.print(":3");
}

void loop() {
  leds_1 = 0;
  leds_2 = 0;
  int sensorValue = analogRead(A0);
  if (vector.size() == vector.max_size())
    vector.remove(0);
  vector.push_back(sensorValue);
  int avg = 0;
  for (int element : vector)
    avg += element;
  avg /= vector.size();
  int bpm = 300 - ((avg / 16) * 4);
  double bpms = bpm / 60000.0;
  double beat_time = (1.0 / bpms) / 4.0;
  if (beat_increment < 8) {
    bitSet(leds_1, beat_increment);
  } else {
    bitSet(leds_2, beat_increment - 8);
  }
  updateShiftRegisters();
  beat_increment++;
  if (beat_increment >= 16)
    beat_increment = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tempo: ");
  lcd.print(bpm);
  lcd.print("bpm");
  delay(beat_time);
}
