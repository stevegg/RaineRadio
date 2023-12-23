#include "RestClient.h"
#include "wifiFix.h"
#include "config.h"
#include <DebugLog.h>

// // You can control the scope depending on your own flag
// #ifdef MYCLASS_ENABLE_DEBUGLOG
// #include <DebugLogEnable.h>
// #else
// #include <DebugLogDisable.h>
// #endif // MYCLASS_ENABLE_DEBUGLOG

const char *contentType = "text/plain";

RestClient::RestClient()
{
   num_headers = 0;
   httpClient = new HTTPClient();
   wifi = new WiFiClientFixed();
}

RestClient::~RestClient()
{
   httpClient->end();
   delete httpClient;
}

void RestClient::reset()
{
   num_headers = 0;
}

int RestClient::get(const char *url)
{
   return this->request(url);
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

char requestUrl[250];
int RestClient::request(const char *requestUrl)
{
   // URLEncode path
   char *encoded = this->urlencode(requestUrl);

   LOG_DEBUG("RestClient -> url: ", encoded);
   httpClient->useHTTP10(true);
   httpClient->setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
   httpClient->begin(*this->wifi, encoded);
   httpClient->addHeader("Content-Type", contentType);
   httpClient->addHeader("Accept", "application/json");
   for (int i = 0; i < num_headers; i++)
   {
      httpClient->addHeader(headerNames[i], headerValues[i]);
   }
   this->statusCode = httpClient->GET();
   // Free memory used by the encoded path
   free(encoded);

   LOG_DEBUG("RestClient -> statusCode: ", this->statusCode);

   if (statusCode < 0)
   {
      LOG_ERROR("HTTP error: ", httpClient->errorToString(statusCode).c_str());
   }

   // Return the status code
   return this->statusCode;
}

WiFiClient &RestClient::getStream()
{
   return httpClient->getStream();
}

void RestClient::close()
{
   httpClient->end();
}

char *RestClient::urlencode(const char *originalText)
{
   int count = 0;
   // First count how many spaces are in the originalText
   for (int i = 0; i < strlen(originalText); i++)
   {
      if (originalText[i] == ' ')
      {
         count++;
      }
   }
   int computedLength = strlen(originalText) + (count * 3) + 1;
   // Allocate a new string with enough space for the %20
   char *newText = (char *)malloc(computedLength);
   memset(newText, 0, computedLength);
   // Now copy the originalText to the newText and replace spaces with %20
   for (int i = 0, j = 0; i < strlen(originalText); i++)
   {
      if (originalText[i] == ' ')
      {
         newText[j++] = '%';
         newText[j++] = '2';
         newText[j++] = '0';
      }
      else
      {
         newText[j++] = originalText[i];
      }
   }

   return newText;
}