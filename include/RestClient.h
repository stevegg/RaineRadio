#ifndef __RESTCLIENT_H__
#define __RESTCLIENT_H__

#include <Arduino.h>
#include <HTTPClient.h>

class RestClient
{
public:
   RestClient(const char *host, const int port);
   ~RestClient();

   int get(String path);
   String getResponseBody();

   void setHeader(const char *headerName, const char *headerValue);
   void setContentType(const char *contentTypeValue);

private:
   int request(const char *method, String path, String body);
   int request(const char *method, String path);
   String urlencode(String originalText);

   int port;
   int num_headers;
   const char *host;
   const char *headerNames[10];
   const char *headerValues[10];
   const char *contentType;
   HTTPClient *httpClient;
   int statusCode;
};

#endif