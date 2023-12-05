#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <WiFi.h>
#include <Audio.h>
#include <ESP32Encoder.h>
#include <ezButton.h>
#include "RadioStateMachine.h"
#include "streams.h"
#include "config.h"
#include "FreeSansBold10pt7b.h"

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
int streamIndex = 0;
State_t state = STATE_INIT;
int lastPosition = 0;
bool buttonPressed = false;
RadioStateMachine machine = RadioStateMachine();
RadioState *S0;
RadioState *S1;
RadioState *S2;
RadioState *S3;

int checkEncoder()
{
  long newPosition = encoder.getCount() / 2;
  if (newPosition != lastPosition)
  {
    if (newPosition < lastPosition)
    {
      lastPosition = newPosition;
      return 1;
    }
    lastPosition = newPosition;
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
    display.fillScreen(WHITE);
    display.setCursor(10, 20);
    display.println("Connecting to Wifi...");
  }
  else if (newState == 1)
  {
    display.fillScreen(WHITE);
    display.setCursor(10, 20);
    display.println("Connecting to stream...");
    display.setCursor(10, 40);
    display.println(streams[streamIndex].name);
  }
  else if (newState == 2)
  {
    display.fillScreen(WHITE);
    display.setCursor(10, 20);
    display.println("Playing...");
    display.setCursor(10, 40);
    display.println(streams[streamIndex].name);
  }
  else if (newState == 3)
  {
    display.fillRect(0, 0, 320, 60, WHITE);
    display.setCursor(10, 20);
    display.println("Configuring...");
    display.setCursor(10, 40);
    display.println(streams[streamIndex].name);
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
    Serial.println(ssid);
    WiFi.mode(WIFI_STA); // Optional
    WiFi.begin(ssid, password);
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
  audio.loop();
  if (machine.executeOnce)
  {
    Serial.print("Connecting to stream :");
    Serial.println(streams[streamIndex].name);
    audio.setPinout(MAX98357A_I2S_BCLK, MAX98357A_I2S_LRC, MAX98357A_I2S_DOUT);
    audio.setVolume(100);
    audio.connecttohost(streams[streamIndex].url);
  }
}

void playingState()
{
  audio.loop();
  if (machine.executeOnce)
  {
    Serial.println("Playing");
    button.resetCount();
    delay(500);
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
    streamIndex++;
    if (streamIndex > StreamCount - 1)
    {
      streamIndex = 0;
    }
  }
  else if (value < 0)
  {
    streamIndex--;
    if (streamIndex < 0)
    {
      streamIndex = StreamCount - 1;
    }
  }
  if (value != 0)
  {
    Serial.print("Stream: ");
    Serial.println(streams[streamIndex].name);
    updateDisplay(3);
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("Initializing display...");

  // Initialize the display
  display.begin();
  display.setRotation(1);
  display.fillScreen(WHITE);
  display.setFont(&FreeSansBold10pt7b);
  display.setTextColor(BLACK);
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
                    // { if (!audio.isRunning()) {
                    //     Serial.print(".");
                    //     return false;
                    //   }
                    //   Serial.println("Connected to stream");
                      return true; },
                    S2);
  S2->addTransition([]()
                    { return (button.isPressed()); },
                    S3);
  S3->addTransition([]()
                    { return (button.isPressed()); },
                    S1);
}

void loop()
{
  button.loop();
  machine.run();
}
