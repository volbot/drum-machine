// this example will play a random track from all on the sd
//
// it expects the sd card to contain some mp3 files

#include <DFMiniMp3.h>

// define a handy type using hardware serial with no notifications
//
// typedef DFMiniMp3<HardwareSerial> DfMp3;

// instance a DfMp3 object,
//
// DfMp3 dfmp3(Serial1);

// Some arduino boards only have one hardware serial port, so a software serial
// port is needed instead. comment out the above definitions and use these
#include <SoftwareSerial.h>
SoftwareSerial secondarySerial(7, 6); // RX, TX

#define DebugOut Serial
#define DfMiniMp3Debug DebugOut

class Mp3Notify;

// define a handy type using serial and our notify class
//
typedef DFMiniMp3<SoftwareSerial, Mp3Notify, Mp3ChipOriginal, 1600> DfMp3;

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
  // for boards that support hardware arbitrary pins
  // dfmp3.begin(10, 11); // RX, TX

  DebugOut.println("initializing...2");
  // during development, it's a good practice to put the module
  // into a known state by calling reset().
  // You may hear popping when starting and you can remove this
  // call to reset() once your project is finalized
  // dfmp3.reset();

  DebugOut.println("initializing...3");
  uint16_t version = dfmp3.getSoftwareVersion();
  DebugOut.print("version ");
  DebugOut.println(version);

  // show some properties and set the volume
  uint16_t volume = dfmp3.getVolume();
  DebugOut.print("volume ");
  DebugOut.println(volume);
  dfmp3.setVolume(24);

  uint16_t count = dfmp3.getTotalTrackCount(DfMp3_PlaySource_Sd);
  DebugOut.print("files ");
  DebugOut.println(count);

  uint16_t mode = dfmp3.getPlaybackMode();
  DebugOut.print("playback mode ");
  DebugOut.println(mode);

  DebugOut.println("starting...");

  // dfmp3.playRandomTrackFromAll(); // random of all folders on sd
  // dfmp3.playMp3FolderTrack(1);
}

void setup() { setupMp3(); }

void loop() {
  // calling dfmp3.loop() periodically allows for notifications
  // to be handled without interrupts
  dfmp3.loop();

  dfmp3.playMp3FolderTrack(1);

  delay(400);
}
