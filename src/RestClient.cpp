#include "RestClient.h"

// Uncomment to debug
#define ESP32_RESTCLIENT_DEBUG

#ifdef ESP32_RESTCLIENT_DEBUG
#define DEBUG_PRINT(x) Serial.print(x);
#else
#define DEBUG_PRINT(x)
#endif
// TODO: Add Timeout to send and read requests

const char *contentType = "text/plain";

RestClient::RestClient(const char *_host, const int _port)
{
   port = _port;
   host = _host;
   num_headers = 0;
   httpClient = new HTTPClient();
}

RestClient::~RestClient()
{
   delete httpClient;
}

int RestClient::get(String path)
{
   return this->request("GET", path);
}

void RestClient::setHeader(const char *headerName, const char *headerValue)
{
   headerNames[num_headers] = headerName;
   headerValues[num_headers] = headerValue;
   num_headers++;
}

void RestClient::setContentType(const char *contentTypeValue)
{
   contentType = contentTypeValue;
}

int RestClient::request(const char *method, String path)
{
   return request(method, path, "");
}

int RestClient::request(const char *method, String path, String body)
{
   DEBUG_PRINT("RestClient::request()\n");
   DEBUG_PRINT("method: ");
   DEBUG_PRINT(method);
   DEBUG_PRINT("\n");
   DEBUG_PRINT("path: ");
   DEBUG_PRINT(path);
   DEBUG_PRINT("\n");
   String url = "https://" + String(host) + ":" + String(port) + this->urlencode(path);
   DEBUG_PRINT("url: ");
   DEBUG_PRINT(url);
   DEBUG_PRINT("\n");
   httpClient->begin(url);
   httpClient->addHeader("Content-Type", contentType);
   for (int i = 0; i < num_headers; i++)
   {
      httpClient->addHeader(headerNames[i], headerValues[i]);
   }
   this->statusCode = httpClient->GET();
   return this->statusCode;
}

String RestClient::getResponseBody()
{
   if (this->statusCode != HTTP_CODE_OK)
   {
      DEBUG_PRINT("Error on HTTP request: ");
      DEBUG_PRINT(this->statusCode);
      DEBUG_PRINT("\n");
      return "";
   }
   return httpClient->getString();
}

String RestClient::urlencode(String originalText)
{
   originalText.replace(" ", "%20");
   return originalText;
}