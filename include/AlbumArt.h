#ifndef __ALBUM_ART_H
#define __ALBUM_ART_H

#include <Arduino.h>

int getImageForStreamTitle(const char *streamTitle, uint8_t **data);

#endif