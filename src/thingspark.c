/***********************************************************************
   Filename:   thingspark.c
   send data to ThingsPark IoT server

   Uses libcurl to send the data via HTTP

************************************************************************/

/* system includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <curl/curl.h>

#include "log.h"

#define TRUE 1

// the URL structure to update ThingsPark
// https://api.thingspark.co.kr/update?api_key=YOUR_CHANNEL_API_KEY&field1=7
char *URLtemplate = "https://api.thingspark.co.kr/update?apiKey=%s&%s=%s&%s=%s";

// structure used by the libcurl write callback function
struct url_data {
    size_t size;
    char* data;
};

//=====================================================================
// write callback function needed by libCurl
size_t write_data(void *ptr, size_t size, size_t nmemb, struct url_data *data) {
    size_t index = data->size;
    size_t n = (size * nmemb);
    char* tmp;

    data->size += (size * nmemb);
    tmp = realloc(data->data, data->size + 1); /* +1 for '\0' */

    if(tmp) {
        data->data = tmp;
    } else {
        if(data->data) {
            free(data->data);
        }
        _D("wuthread> write_data Failed to allocate memory.\n");
        return 0;
    }

    memcpy((data->data + index), ptr, n);
    data->data[data->size] = '\0';

    return size * nmemb;
}

//=====================================================================
// upload data to ThingsPark
int UpdateThingsPark(char *api_key, char *f1, char *v1, char *f2, char *v2)
{
    int   error = -1;
    time_t   now;
    struct tm  *dt;
    char   url[256];

    CURL  *curl;
    CURLcode res;
    struct url_data response;

    time(&now);
    dt = gmtime(&now);

    // build the URL string
    snprintf(url, sizeof(url)-1, URLtemplate, api_key, f1, v1, f2, v2);
    // guarantee null termination of string
    url[sizeof(url)-1] = 0;

    _D("UpdateThingsPark> send: [%s]",url);

    curl = curl_easy_init();
    if (curl) {
        response.size = 0;
        response.data = malloc(4096); /* reasonable size initial buffer */
        response.data[0] = '\0';
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
        {
            _D("UpdateThingsPark> curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
            _D("URL is: [%s]",url);
            error = -1;
        } else {
            // ThingsPark returns "0" for error, else returns sample number
            error = (strcmp(response.data,"0") == 0);
        }
        curl_easy_cleanup(curl);
        free (response.data);
    } else {
        _D("UpdateThingsPark> curl_easy_init failed\n");
        error = -1;
    }

    return error;
}
