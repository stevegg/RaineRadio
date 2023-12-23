#ifndef __CONFIG_H
#define __CONFIG_H

// Encoder
#define CLK_PIN 15 // ESP32 pin GPIO15 connected to the rotary encoder's CLK pin
#define DT_PIN 32  // 2   // ESP32 pin GPIO2 connected to the rotary encoder's DT pin
#define SW_PIN 14  // 4   // ESP32 pin GPIO4 connected to the rotary encoder's SW pin

#define DIRECTION_CW 0  // clockwise direction
#define DIRECTION_CCW 1 // counter-clockwise direction

// Display
#define TFT_SCK 18
#define TFT_MOSI 23
#define TFT_MISO 19
#define TFT_CS 22
#define TFT_DC 21
#define TFT_RESET 17

// Audio
#define MAX98357A_I2S_DOUT 25
#define MAX98357A_I2S_BCLK 27
#define MAX98357A_I2S_LRC 26
#define VOLUME_INPUT 39

#define SSID "GIoT"
#define PASSWORD "IC7610B4by!"

#define DEBUGLOG_DEFAULT_LOG_LEVEL_DEBUG

#endif