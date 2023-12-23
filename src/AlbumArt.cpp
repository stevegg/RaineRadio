#include <Arduino.h>
#include <ArduinoJson.h>
#include "RestClient.h"
#include "config.h"
#include <DebugLog.h>

// // You can control the scope depending on your own flag
// #ifdef MYCLASS_ENABLE_DEBUGLOG
// #include <DebugLogEnable.h>
// #else
// #include <DebugLogDisable.h>
// #endif // MYCLASS_ENABLE_DEBUGLOG

const char *getAlbumArtMBID(RestClient *restClient, const char *streamTitle)
{
   const char *mbid = NULL;
   LOG_INFO("Getting album art for stream title: ", streamTitle);
   // Serial.println(streamTitle);
   String s = streamTitle;
   // Remove StreamTitle: and the preceeding '
   s = s.substring(13);
   // Remove trailing '
   s = s.substring(0, s.length() - 1);

   // Now we need the artist and song.  These are separated by a -
   int pos = s.indexOf("-");
   String artist = s.substring(0, pos - 2);
   String song = s.substring(pos + 2);
   String url = "http://musicbrainz.org/ws/2/recording/?query=artist:" + artist + "+recording:" + song;

   restClient->setContentType("application/json");
   restClient->setHeader("Accept", "application/json");
   int statusCode = restClient->get(url.c_str());
   if (statusCode == 200)
   {
      // Parse JSON
      DynamicJsonDocument doc(4096);
      deserializeJson(doc, restClient->getStream());
      mbid = doc["recordings"][0]["releases"][0]["id"].as<const char *>();
      LOG_INFO("Album Art -> MBID: ", mbid);
   }
   else
   {
      LOG_ERROR("Album Art -> Error: ", statusCode);
   }
   // Close the REST client connection
   restClient->close();

   return mbid;
}

const char *getAlbumArtUrl(RestClient *restClient, const char *streamTitle)
{
   LOG_INFO("Getting album art for stream title: ", streamTitle);
   const char *mbid = getAlbumArtMBID(restClient, streamTitle);
   if (mbid != NULL)
   {
      LOG_INFO("Getting album art for MBID: ", mbid);

      restClient->reset();
      restClient->setContentType("application/json");
      restClient->setHeader("Accept", "application/json");
      char url[128];
      sprintf(url, "http://coverartarchive.org/release/%s", mbid);

      int statusCode = restClient->get(url);
      if (statusCode == 200)
      {
         DynamicJsonDocument doc(1024);
         deserializeJson(doc, restClient->getStream());
         const char *url = doc["images"][0]["thumbnails"]["small"].as<const char *>();
         return url;
      }
      else
      {
         if (statusCode == 307)
         {
            LOG_DEBUG("Album Art -> Redirect");
         }
         else
         {
            LOG_ERROR("Album Art -> Error: ", statusCode);
         }
      }
   }
   return NULL;
}

#define BUFF_SIZE 128

int readFileFromUrl(const char *url, uint8_t **data)
{
   HTTPClient http;
   http.begin(url);
   int code = http.sendRequest("GET");
   int index = 0;
   if (code > 0)
   {
      int len = http.getSize();
      LOG_DEBUG("[HTTP] GET... code: ", code);
      LOG_DEBUG("[HTTP] GET... size: ", len);
      int headerCount = http.headers();
      if (headerCount > 0)
      {
         for (int i = 0; i < headerCount; i++)
         {
            String headerName = http.headerName(i);
            String headerValue = http.header(i);
            LOG_DEBUG("[HTTP] GET... header: ", headerName.c_str(), " : ", headerValue.c_str());
         }
      }
      else
      {
         LOG_DEBUG("[HTTP] GET... no headers");
      }
      *data = (uint8_t *)malloc(len + BUFF_SIZE);

      // create buffer for read
      uint8_t buff[BUFF_SIZE] = {0};
      WiFiClient *stream = http.getStreamPtr();
      while (http.connected() && stream->available() > 0)
      {
         // get available data size
         size_t size = stream->available();
         if (size + index > len)
         {
            len += size + index;
            // Reallocate the buffer
            *data = (uint8_t *)realloc(*data, len + BUFF_SIZE);
         }

         if (size)
         {
            // read up to BUFF_SIZE byte
            int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
            if (c > len)
            {
               c = len;
            }
            memcpy(*data + index, buff, c);
            index += c;
            if (len > 0)
            {
               len -= c;
            }
         }
         delay(1);
      }
      http.end();
   }
   else
   {
      LOG_ERROR("[HTTP] GET... failed, error: ", http.errorToString(code).c_str());
   }

   return index;
}

int getImageForStreamTitle(const char *streamTitle, uint8_t **data)
{
   RestClient *restClient = new RestClient();
   const char *url = getAlbumArtUrl(restClient, streamTitle);
   LOG_INFO("Album Art URL: ", url);

   // Read the data from the url
   int bytes = readFileFromUrl(url, data);
   LOG_DEBUG("Read :", bytes);

   delete restClient;
   return bytes;
}