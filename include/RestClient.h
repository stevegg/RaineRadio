#ifndef __RESTCLIENT_H__
#define __RESTCLIENT_H__

#include <Arduino.h>
#include <HTTPClient.h>
#include "wifiFix.h"

class RestClient
{
public:
   RestClient();
   ~RestClient();

   void reset();
   int get(const char *path);
   const char *getResponseBody();
   char getNextResponseBodyChar();

   void setHeader(const char *headerName, const char *headerValue);
   void setContentType(const char *contentTypeValue);
   WiFiClient &getStream();
   void close();

private:
   int request(const char *url);
   char *urlencode(const char *original);

   int num_headers;
   const char *headerNames[10];
   const char *headerValues[10];
   const char *contentType;
   HTTPClient *httpClient;
   int statusCode;
   WiFiClientFixed *wifi;
};

#endif