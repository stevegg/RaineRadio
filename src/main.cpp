#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <Audio.h>
#include <ESP32Encoder.h>
#include <ezButton.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <SD_MMC.h>
#include <SPIFFS.h>
#include <FFat.h>
#include <LittleFS.h>
#include <EEPROM.h>
#include "RadioStateMachine.h"
#include "RestClient.h"
#include "streams.h"
#include "config.h"
#include "FreeSansBold10pt7b.h"
#include "FreeSansBold18pt.h"

#define LIGHTBLUE RGB565(3, 140, 252)
// define the number of bytes you want to access
#define EEPROM_SIZE 1

typedef enum
{
  STATE_INIT,
  STATE_CONNECTING,
  STATE_PLAYING,
  STATE_SELECTING,
  STATE_CONFIG
} State_t;

Arduino_ESP32SPI bus = Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO);
Arduino_ILI9341 display = Arduino_ILI9341(&bus, TFT_RESET);
Audio audio;
ESP32Encoder encoder;
ezButton button(SW_PIN);
int GV_streamIndex = 3;
State_t GV_state = STATE_INIT;
int GV_lastPosition = 0;
bool GV_buttonPressed = false;
RadioStateMachine machine = RadioStateMachine();
RadioState *S0;
RadioState *S1;
RadioState *S2;
RadioState *S3;
int GV_volume = 0;
String GV_artist;
String GV_song;
boolean GV_updateStream = false;
boolean GV_slowStream = false;
boolean GV_lastSlowStream = false;
boolean GV_ConnectFailure = false;

int checkEncoder()
{
  long newPosition = encoder.getCount() / 2;
  if (newPosition != GV_lastPosition)
  {
    if (newPosition < GV_lastPosition)
    {
      GV_lastPosition = newPosition;
      return 1;
    }
    GV_lastPosition = newPosition;
    return -1;
  }
  // Return 0 for no change
  return 0;
}

void updateDisplay(int newState)
{
  Serial.print("Update display :");
  Serial.println(newState);
  if (newState == 0)
  {
    display.fillScreen(BLACK);
    display.setCursor(10, 20);
    display.println("Connecting to Wifi...");
  }
  else if (newState == 1)
  {
    display.fillScreen(BLACK);
    display.setCursor(10, 20);
    display.println("Connecting to stream...");
    display.setCursor(10, 40);
    display.println(streams[GV_streamIndex].name);
  }
  else if (newState == 2)
  {
    display.fillScreen(BLACK);
    display.setFont(&FreeSansBold18pt7b);
    display.setTextColor(LIGHTBLUE);
    display.setCursor(10, 30);
    display.println(streams[GV_streamIndex].name);
    display.setFont(&FreeSansBold10pt7b);
    display.setTextColor(WHITE);
    display.setCursor(40, 100);
    display.println(GV_artist);
    display.setCursor(40, 120);
    display.println(GV_song);
    if (GV_slowStream)
    {
      display.setCursor(130, 220);
      display.setTextColor(RED);
      display.println("Slow Stream...");
      display.setTextColor(WHITE);
    }
  }
  else if (newState == 3)
  {
    display.fillRect(0, 0, 320, 160, BLACK);
    display.setCursor(10, 20);
    display.println("Configuring...");
    display.setCursor(10, 40);
    display.println(streams[GV_streamIndex].name);
  }
}

// Forward declarations of state functions
void connectingWifiState()
{
  if (machine.executeOnce)
  {
    long startTime = millis();
    int myCount = 0;
    Serial.print("Connecting to Wifi: ");
    Serial.println(SSID);
    WiFi.mode(WIFI_STA); // Optional
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED && myCount < 31)
    {
      yield();
      if (millis() - startTime > 500)
      {
        Serial.print(".");
        startTime = millis();
        myCount++;
        if (myCount > 30)
        {
          ESP.restart();
        }
      }
    }
  }
}

void connectingStreamState()
{
  if (machine.executeOnce || GV_ConnectFailure)
  {
    GV_ConnectFailure = false;
    GV_artist = "";
    GV_song = "";
    Serial.println(streams[GV_streamIndex].name);
    audio.setPinout(MAX98357A_I2S_BCLK, MAX98357A_I2S_LRC, MAX98357A_I2S_DOUT);
    audio.setVolume(GV_volume);
    audio.connecttohost(streams[GV_streamIndex].url);
  }
}

void playingState()
{
  if (machine.executeOnce)
  {
    Serial.println("Playing");
    button.resetCount();
    delay(500);
  }
  if (GV_updateStream)
  {
    GV_updateStream = false;
    updateDisplay(STATE_PLAYING);
  }
  if (GV_slowStream && !GV_lastSlowStream)
  {
    GV_lastSlowStream = true;
    Serial.println("Slow Stream");
    updateDisplay(STATE_PLAYING);
  }
}

void configuringState()
{
  if (machine.executeOnce)
  {
    Serial.println("Configuring");
    button.resetCount();
    delay(500);
  }
  int value = checkEncoder();
  if (value > 0)
  {
    GV_streamIndex++;
    if (GV_streamIndex > StreamCount - 1)
    {
      GV_streamIndex = 0;
    }
  }
  else if (value < 0)
  {
    GV_streamIndex--;
    if (GV_streamIndex < 0)
    {
      GV_streamIndex = StreamCount - 1;
    }
  }
  if (value != 0)
  {
    // Write to non-volatile memory
    EEPROM.write(0, GV_streamIndex);
    EEPROM.commit();
    Serial.print("Stream: ");
    Serial.println(streams[GV_streamIndex].name);
    updateDisplay(3);
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("Initializing EEPROM...");
  EEPROM.begin(EEPROM_SIZE);
  GV_streamIndex = EEPROM.read(0);
  if (GV_streamIndex >= StreamCount)
  {
    GV_streamIndex = 0;
  }

  Serial.println("Initializing display...");

  // Initialize the display
  display.begin();
  display.setRotation(1);
  display.fillScreen(BLACK);
  display.setFont(&FreeSansBold10pt7b);
  display.setTextColor(WHITE);
  display.setCursor(10, 20);
  display.println("Setting things up...");
  updateDisplay(0);

  Serial.println("Setting up encoder...");

  // Enable the weak pull up resistors
  ESP32Encoder::useInternalWeakPullResistors = UP;

  encoder = new ESP32Encoder();
  encoder.attachHalfQuad(CLK_PIN, DT_PIN);
  button.resetCount();

  // set starting count value after attaching
  encoder.clearCount();

  Serial.println("Setting up State Machine...");
  machine.setTransitionCallback(updateDisplay);
  S0 = machine.addState(&connectingWifiState);
  S1 = machine.addState(&connectingStreamState);
  S2 = machine.addState(&playingState);
  S3 = machine.addState(&configuringState);

  // Setup State transitions
  // Transition from S0 to S1 when WiFi is connected
  S0->addTransition([]()
                    { 
                      if (WiFi.status() != WL_CONNECTED) {
                        return false;
                      }
                      Serial.println("Connected to WiFi");
                      Serial.print("IP address: ");
                      Serial.println(WiFi.localIP());
                      return true; },
                    S1);

  // Transition from S1 to S2 when audio stream is connected
  S1->addTransition([]()
                    {
                      if (!audio.isRunning() || GV_ConnectFailure)
                      {
                        return false;
                      }
                      return true; },
                    S2);
  S1->addTransition([]()
                    { return (button.isPressed()); },
                    S3);
  S2->addTransition([]()
                    { return (button.isPressed()); },
                    S3);
  S3->addTransition([]()
                    { return (button.isPressed()); },
                    S1);
}

void loop()
{
  audio.loop();
  // Get the volume level
  GV_volume = map((analogRead(VOLUME_INPUT)), 0, 4095, 0, 20);
  audio.setVolume(GV_volume);
  // Check the encoder
  button.loop();
  // Run the state machine
  machine.run();
}

uint8_t buffer[150];
char lastInfo[255];

void processStreamTitle(const char *info)
{
  if (strcmp(info, lastInfo) == 0)
  {
    return;
  }
  strcpy(lastInfo, info);
  // Remove StreamTitle: and the preceeding '
  char *sinfo = (char *)(info + 13);
  // Remove trailing '
  sinfo[strlen(sinfo) - 1] = 0;

  // Now we need the artist and song
  char *artist = strtok(sinfo, "-");
  char *song = strtok(NULL, "-");
  if (song != NULL)
  {
    // Remove trailing space from artist
    // artist[strlen(artist) - 1] = 0;
    // Remove leading space from song
    song++;
    GV_artist = String(artist);
    GV_song = String(song);
  }
  else if (artist != NULL)
  {
    GV_artist = String(artist);
    GV_song = "";
  }
  else
  {
    GV_artist = "";
    GV_song = "";
  }
  if (GV_artist.length() > 20)
  {
    GV_artist = GV_artist.substring(0, 20) + "...";
  }
  if (GV_song.length() > 20)
  {
    GV_song = GV_song.substring(0, 20) + "...";
  }

  Serial.print("StreamTitle Artist :");
  Serial.println(artist);
  Serial.print("StreamTitle Song :");
  Serial.println(song);
  audio_showstation(info + 10);
  GV_updateStream = true;
}

//   // construct the url
//   char *url = (char *)malloc(strlen(artist) + strlen(song) + 100);
//   sprintf(url, "/ws/2/recording/?query=artist:%s+recording:%s", artist, song);
//   albumArtRestClient.setContentType("application/json");
//   albumArtRestClient.setHeader("Accept", "application/xml");
//   int statusCode = albumArtRestClient.get(url);
//   if (statusCode == 200)
//   {
//     JsonDocument &doc = albumArtRestClient.getJsonDoc();
//     const char *mbid = doc["recordings"][0]["releases"][0]["id"].as<const char *>();
//     Serial.print("Album Art -> MBID: ");
//     Serial.println(mbid);
//   }
//   else
//   {
//     Serial.print("Album Art -> Error: ");
//     Serial.println(statusCode);
//   }
// }

//**************************************************************************************************
//                                           E V E N T S                                           *
//**************************************************************************************************
void audio_info(const char *info)
{
  GV_slowStream = false;
  Serial.print("audio_info: ");
  Serial.println(info);
  if (strstr(info, "StreamTitle="))
  {
    processStreamTitle(info);
  }
  if (strstr(info, "slow stream,"))
  {
    GV_slowStream = true;
  }
  if (!GV_slowStream)
  {
    GV_lastSlowStream = false;
  }
  // Check and see if the connect request failed
  if (strstr(info, "failed!"))
  {
    Serial.println("Connection failed");
    machine.executeOnce = true;
    GV_ConnectFailure = true;
    machine.transitionTo(STATE_CONNECTING);
  }
}
void audio_showstation(const char *info)
{
  Serial.print("Station Name:");
  Serial.println(String(info));
}
void audio_showstreamtitle(const char *info)
{
  String sinfo = String(info);
  sinfo.replace("|", "\n");
  Serial.println(sinfo);
}

void audio_id3data(const char *info)
{
  Serial.print("ID3 Data: ");
  Serial.println(String(info));
}
