/*
   VB-909 Rhythm Composer

   Arduino drum machine.
   Requirements pending.

   allomyrina volbot <tech@volbot.org>
   */

// #################
// # BACKEND SETUP #
// #################

#include <Vector.h>

const int CACHE_MAX = 5;
int storage_array[CACHE_MAX];
Vector<int> vector(storage_array);

// #################
// ## AUDIO LOGIC ##
// #################
// # SETUP #
// #########

int beat_increment = 0;
// int bpm = 0;
int bpm = 120;
unsigned long last_beat_time = 0;
double beat_time = 0;

int instrument_index = 0;
char *instruments[8] = {"Kck1", "Kck2", "Clap", "Rim",
                        "Snre", "Tom",  "Hat1", "Hat2"};

int program[4][8] = {
    {0x8080, 0, 0x0808, 0, 0, 0, 0, 0x2223}, // HOUSE
    {0x5555, 0x8888, 0, 0, 0x2222, 0, 0, 0}, // EMPTY
    {0x8888, 0, 0, 0x1212, 0, 0, 0x2222, 0}, // REGGAETON
    {0, 0, 0, 0, 0, 0, 0, 0},                // EMPTY
};

int program_index = 0;
void incProgIndex() {
  program_index++;
  if (program_index >= 4)
    program_index = 0;
}

// Rotary Encoder pins: Channel A, Channel B, Button
const int PIN_RE1[3] = {0, 1, 13};
// Limited Value Constants: Default, Interval, Min, Max
const int VAL_BPM[4] = {120, 5, 60, 220};
// Limited Value Variables: Current, Last
volatile int VAR_BPM[2] = {VAL_BPM[0], VAL_BPM[0]};

// Rotary Encoder pins: Channel A, Channel B, Button
const int PIN_RE2[3] = {2, 11, 12};
// Limited Value Constants: Default, Interval, Min, Max
const int VAL_VOL[4] = {20, 2, 0, 30};
// Limited Value Variables: Current, Last
volatile int VAR_VOL[2] = {VAL_VOL[0], VAL_VOL[0]};

// #############
// # FUNCTIONS #
// #############

void readBPM() {
  int sensorValue = analogRead(A0);
  if (vector.size() == vector.max_size())
    vector.remove(0);
  vector.push_back(sensorValue);
  int avg = 0;
  for (int element : vector)
    avg += element;
  avg /= vector.size();

  float convIn = 200 * ((976 - avg) / 7);
  int convOut = convIn / 200;
  propagateBPM(convOut + 60);
}

void propagateBPM(int bpm_) {
  bpm = bpm_;
  double bpms = bpm_ / 60000.0;
  beat_time = (1.0 / bpms) / 4.0;
}

// ###############
// ## LCD SETUP ##
// ###############
// # SETUP #
// #########

#include <LiquidCrystal_74HC595.h>

// const int rs = 13, en = 12, d4 = 11, d5 = 10, d6 = 9, d7 = 8;
#define DS 10
#define SHCP 8
#define STCP 9
#define RS 1
#define E 2
#define D4 3
#define D5 4
#define D6 5
#define D7 6
LiquidCrystal_74HC595 lcd(DS, SHCP, STCP, RS, E, D4, D5, D6, D7);

// #############
// # FUNCTIONS #
// #############

int get_bit(int num, int bit_position) {
  int mask = 1 << bit_position;
  return (num & mask) >> bit_position;
}

void setupLCD() {
  lcd.begin(16, 2);
  lcd.print(":3");
}

void printLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("[");
  lcd.print(bpm);
  lcd.setCursor(4, 0);
  lcd.print("] ");
  lcd.print(instrument_index + 1);
  lcd.print(":");
  lcd.print(instruments[instrument_index]);
  lcd.setCursor(13, 0);
  lcd.print("P");
  lcd.print(":");
  lcd.print(program_index + 1);
  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++) {
    char *toprint = "-";
    if (get_bit(program[program_index][instrument_index], 15 - i)) {
      toprint = "*";
    } else {
      int found = 0;
      for (int j = 0; j < 8; j++) {
        if (get_bit(program[program_index][j], 15 - i)) {
          toprint = "+";
          break;
        }
      }
    }
    // if(i == beat_increment) toprint = "*";
    lcd.print(toprint);
  }
  lcd.cursor();
  lcd.setCursor(beat_increment, 1);
}

// ###################
// ## GENERAL SETUP ##
// ###################
#include <DFMiniMp3.h>

#include <SoftwareSerial.h>
SoftwareSerial secondarySerial(7, 6); // RX, TX

#define DebugOut Serial
#define DfMiniMp3Debug DebugOut

class Mp3Notify;

// define a handy type using serial and our notify class
//
// typedef DFMiniMp3<SoftwareSerial, Mp3Notify, Mp3ChipOriginal, 1600> DfMp3;
typedef DFMiniMp3<SoftwareSerial, Mp3Notify> DfMp3;

class Mp3Notify {
public:
  static void PrintlnSourceAction(DfMp3_PlaySources source,
                                  const char *action) {
    if (source & DfMp3_PlaySources_Sd) {
      DebugOut.print("SD Card, ");
    }
    if (source & DfMp3_PlaySources_Usb) {
      DebugOut.print("USB Disk, ");
    }
    if (source & DfMp3_PlaySources_Flash) {
      DebugOut.print("Flash, ");
    }
    DebugOut.println(action);
  }
  static void OnError([[maybe_unused]] DfMp3 &mp3, uint16_t errorCode) {
    // see DfMp3_Error for code meaning
    DebugOut.println();
    DebugOut.print("Com Error ");
    DebugOut.println(errorCode);
  }
  static void OnPlayFinished([[maybe_unused]] DfMp3 &mp3,
                             [[maybe_unused]] DfMp3_PlaySources source,
                             uint16_t track) {
    DebugOut.print("Play finished for #");
    DebugOut.println(track);
  }
  static void OnPlaySourceOnline([[maybe_unused]] DfMp3 &mp3,
                                 DfMp3_PlaySources source) {
    PrintlnSourceAction(source, "online");
  }
  static void OnPlaySourceInserted([[maybe_unused]] DfMp3 &mp3,
                                   DfMp3_PlaySources source) {
    PrintlnSourceAction(source, "inserted");
  }
  static void OnPlaySourceRemoved([[maybe_unused]] DfMp3 &mp3,
                                  DfMp3_PlaySources source) {
    PrintlnSourceAction(source, "removed");
  }
};

DfMp3 dfmp3(secondarySerial);

void setupMp3() {
  DebugOut.begin(115200);

  DebugOut.println("initializing...");

  dfmp3.begin();

  DebugOut.println("initializing...2");
  // dfmp3.reset();

  DebugOut.println("initializing...3");
  uint16_t version = dfmp3.getSoftwareVersion();
  DebugOut.print("version ");
  DebugOut.println(version);

  // show some properties and set the volume
  uint16_t volume = dfmp3.getVolume();
  DebugOut.print("volume ");
  DebugOut.println(volume);
  dfmp3.setVolume(VAR_VOL[0]);

  uint16_t count = dfmp3.getTotalTrackCount(DfMp3_PlaySource_Sd);
  DebugOut.print("files ");
  DebugOut.println(count);

  uint16_t mode = dfmp3.getPlaybackMode();
  DebugOut.print("playback mode ");
  DebugOut.println(mode);

  DebugOut.println("starting...");
}

#define NUMBER_OF_SHIFT_CHIPS 2
#define DATA_WIDTH 16
#define PULSE_WIDTH_USEC 5
#define POLL_DELAY_MSEC 1
#define BYTES_VAL_T unsigned int
unsigned long last_poll_time = 0;

int INlatchPin = 5;
int INdataPin = 3;
int INclockPin = 4;

BYTES_VAL_T pinValues;
BYTES_VAL_T oldPinValues;

BYTES_VAL_T read_shift_regs() {
  digitalWrite(INlatchPin, LOW);
  digitalWrite(INlatchPin, HIGH);

  for (int i = 0; i < DATA_WIDTH; i++) {
    int bit = digitalRead(INdataPin);
    if (bit == HIGH) {
      DebugOut.print("1");
    } else {
      DebugOut.print("0");
    }

    digitalWrite(INclockPin, HIGH);
    digitalWrite(INclockPin, LOW);
  }
  DebugOut.println();
}

void setupShiftRegisters() {
  pinMode(INlatchPin, OUTPUT);
  pinMode(INclockPin, OUTPUT);
  pinMode(INdataPin, INPUT);
  // digitalWrite(clockPin, LOW);
  // digitalWrite(ploadPin, HIGH);
  // pinValues = read_shift_regs();
  // display_pin_values();
  // oldPinValues = pinValues;
}

int read_encoder(int PIN_RE[], int VAL[], volatile int VAR[]) {
  // Encoder routine. Updates counter if they are valid
  // and if rotated a full indent

  int index = PIN_RE[0] == 0 ? 0 : 1;
  static uint8_t old_AB[2] = {3, 3}; // Lookup table index
  static int8_t encval[2] = {0, 0};  // Encoder value
  static const int8_t enc_states[] = {0, -1, 1, 0, 1, 0,  0, -1, -1,
                                      0, 0,  1, 0, 1, -1, 0}; // Lookup table

  old_AB[index] <<= 2; // Remember previous state

  if (digitalRead(PIN_RE[0]))
    old_AB[index] |= 0x02; // Add current state of pin A
  if (digitalRead(PIN_RE[1]))
    old_AB[index] |= 0x01; // Add current state of pin B

  encval[index] += enc_states[(old_AB[index] & 0x0f)];

  // Update counter if encoder has rotated a full indent, that is at least 4
  // steps
  int changed = 0;
  if (encval[index] > 3) { // Four steps forward
    VAR[0] += VAL[1];      // Increase counter
    encval[index] = 0;
    changed = 1;
  } else if (encval[index] < -3) { // Four steps backwards
    VAR[0] -= VAL[1];              // Decrease counter
    encval[index] = 0;
    changed = 1;
  }

  if (changed) {
    if (VAR[0] > VAL[3]) {
      VAR[0] = VAL[3];
    } else if (VAR[0] < VAL[2]) {
      VAR[0] = VAL[2];
    }
  }
  return changed;
}

int read_button(int PIN, bool last, void (*callback)()) {
  int state = digitalRead(PIN);
  if (state == LOW) {
    if (!last) {
      last = 1;
      if (callback) {
        callback();
      }
    }
  } else {
    last = 0;
  }
  return last;
}

bool playing = 0;
void togglePlaying() { playing = !playing; }

void setup() {
  setupLCD();
  setupMp3();
  setupShiftRegisters();
  propagateBPM(VAR_BPM[0]);
  pinMode(PIN_RE1[0], INPUT_PULLUP);
  pinMode(PIN_RE1[1], INPUT_PULLUP);
  pinMode(PIN_RE1[2], INPUT_PULLUP);
  pinMode(PIN_RE2[0], INPUT_PULLUP);
  pinMode(PIN_RE2[1], INPUT_PULLUP);
  pinMode(PIN_RE2[2], INPUT_PULLUP);
  playing = 1;
  pinValues = 0;
  oldPinValues = 0;
}

void loop() {

  unsigned long time = 0;
  time = millis();

  // static int lastCounterA = counterA;
  // static int lastCounterB = counterB;
  static int button1Engaged = 0;
  static int button2Engaged = 0;
  int shift = button1Engaged;
  int state_changed = 0;

  if (playing) {
    if (time - last_beat_time >= beat_time) {
      last_beat_time = time;
      beat_increment++;
      if (beat_increment >= 16)
        beat_increment = 0;
      state_changed = 1;
      for (int i = 0; i < 8; i++) {
        if (get_bit(program[program_index][i], 15 - beat_increment)) {
          dfmp3.playMp3FolderTrack(i + 1);
          break;
        }
      }
    }
  }

  if (read_encoder(PIN_RE1, VAL_BPM, VAR_BPM)) {
    state_changed = 1;
    propagateBPM(VAR_BPM[0]);
  }
  if (read_encoder(PIN_RE2, VAL_VOL, VAR_VOL)) {
    state_changed = 1;
    dfmp3.setVolume(VAR_VOL[0]);
  }

  if (time - last_poll_time >= 50) {
    last_poll_time = time;
    // Step 1: Sample
    digitalWrite(INlatchPin, LOW);
    digitalWrite(INlatchPin, HIGH);
    pinValues = 0;

    // Step 2: Shift
    for (int i = 0; i < DATA_WIDTH; i++) {
      int dex = i >= 8 ? (15 - i) + 8 : (7 - i);
      int bit = digitalRead(INdataPin);
      pinValues |= (bit << dex);
      if (bit == HIGH) {
        // DebugOut.print("1");
        if (oldPinValues << dex == 0) {
          DebugOut.println(dex, DEC);
          // DebugOut.println(get_bit(program[program_index][instrument_index],
          // dex), DEC);
          if (!shift) {
            program[program_index][instrument_index] ^= (1 << (15 - dex));
          } else {
            if (dex < 8) {
              instrument_index = dex;
            } else if (dex < 12) {
              program_index = dex - 8;
            }
          }
        }
      } else {
        // DebugOut.print("0");
      }
      // DebugOut.print(" ");
      // DebugOut.print(dex);
      digitalWrite(INclockPin, HIGH); // Shift out the next bit
      digitalWrite(INclockPin, LOW);
    }
    if (pinValues != oldPinValues) {
      oldPinValues = pinValues;
      DebugOut.println(pinValues, BIN);
    }
  }

  if (button1Engaged = read_button(PIN_RE1[2], button1Engaged, 0))
    state_changed = 1;

  if (button2Engaged = read_button(PIN_RE2[2], button2Engaged, togglePlaying))
    state_changed = 1;

  if (state_changed)
    printLCD();

  if (time % 100 == 0) {
    dfmp3.loop();
  }

  // delay(beat_time);
}
