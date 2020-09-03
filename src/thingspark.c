/*****************************************************************
 *
 * Copyright (c) 2019 theksystem Co., Ltd. All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/


/***********************************************************************
 Filename:   thingspark.c
 Author: theksystem

 send data to thingspark IoT Cloud server

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
#include "thingspark.h"

#include <json-glib/json-glib.h>


#define MAXIMUM_FIELD 10

// structure used by the libcurl write callback function
struct url_data {
	size_t size;
	char* data;
};

//=====================================================================
// write callback function needed by libCurl
static size_t write_data(void *ptr, size_t size, size_t nmemb,
		struct url_data *data) {
	size_t index = data->size;
	size_t n = (size * nmemb);
	char* tmp;

	data->size += (size * nmemb);
	tmp = realloc(data->data, data->size + 1); /* +1 for '\0' */

	if (tmp) {
		data->data = tmp;
	} else {
		if (data->data) {
			free(data->data);
		}
		_D("wuthread> write_data Failed to allocate memory.\n");
		return 0;
	}

	memcpy((data->data + index), ptr, n);
	data->data[data->size] = '\0';

	return size * nmemb;
}

struct tp_handle_s {
	//char url[256];
	char *value[MAXIMUM_FIELD];
	char *api_key;

	CURL *curl;
	CURLcode res;
};

typedef struct tp_handle_s *tp_handle_h;

int tp_initialize(const char *api_key, tp_handle_h *handle) {
	retv_if(!api_key, -1);
	retv_if(!handle, -1);

	tp_handle_h h = calloc(1, sizeof(struct tp_handle_s));
	retv_if(!h, -1);

	h->api_key = strdup(api_key);
	retv_if(!h->api_key, -1);

	h->curl = curl_easy_init();
	retv_if(!h->curl, -1); // api_key 등을 정리하기

	*handle = h;

	return 0;
}

int tp_set_field_value(tp_handle_h handle, int field, const char *value) {
	retv_if(!handle, -1);
	retv_if(field < 1, -1);
	retv_if(field > MAXIMUM_FIELD, -1);
	retv_if(!value, -1);

	handle->value[field - 1] = strdup(value);
	_D("handle->value[field - 1] = %s", handle->value[field - 1]);
	retv_if(!handle->value[field - 1], -1);

	return 0;
}

#define BUFSIZE 256
#define FIELD_NAME "field"
int tp_send_data(tp_handle_h handle) {
	char send_str[BUFSIZE] = { 0, };
	char temp_url_value[BUFSIZE] = { 0, };

	snprintf(send_str, BUFSIZE - 1, "https://api.thingspark.co.kr/update?apiKey=%s", handle->api_key);

	_D("URL 1 is: [%s]", send_str);
	for (int i = 0; i < MAXIMUM_FIELD; ++i) {
		if (handle->value[i]) {
			// build the URL string
			snprintf(temp_url_value, BUFSIZE - 1, "&%s%d=%s", FIELD_NAME, i + 1, handle->value[i]);
			_D("URL 2 is: [%s]", temp_url_value);

			strcat(send_str, temp_url_value);
		}
	}

	_D("URL 3 is: [%s]", send_str);
	// strcat(send_str, temp_url_value);

	struct url_data response;

	response.size = 0;
	response.data = malloc(4096); /* reasonable size initial buffer */
	response.data[0] = '\0';

	curl_easy_setopt(handle->curl, CURLOPT_URL, send_str);
	curl_easy_setopt(handle->curl, CURLOPT_CUSTOMREQUEST, "GET");
	curl_easy_setopt(handle->curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(handle->curl, CURLOPT_WRITEDATA, &response);

	handle->res = curl_easy_perform(handle->curl);
	if (handle->res != CURLE_OK) {
		_E("UpdateThingsPark> curl_easy_perform() failed: %s\n", curl_easy_strerror(handle->res));
		//_E("URL is: [%s]", handle->url);
	} else {
		_D("succeed to perform");
		_D("%s", response.data);
		retv_if(strcmp(response.data,"0")==0, -1);
	}

	free(response.data);

	return 0;
}

int tp_finalize(tp_handle_h handle) {
	retv_if(!handle, -1);
	retv_if(!handle->api_key, -1);
	retv_if(!handle->curl, -1);

	curl_easy_cleanup(handle->curl);

	free(handle->api_key);
	for (int i = 0; i < MAXIMUM_FIELD; ++i) {
		if (handle->value[i])
			free(handle->value[i]);
	}
	free(handle);

	return 0;
}

/*
 int main()
 {
 int ret = 0;
 tp_handle_h handle = NULL;

 ret = tp_initialize("api-key", &handle);
 retv_if(ret != 0, -1);

 ret = tp_set_field_value(handle, 1, "0.5");
 retv_if(ret != 0, -1);

 ret = tp_set_field_value(handle, 2, "5");
 retv_if(ret != 0, -1);

 ret = tp_set_field_value(handle, 6, "7");
 retv_if(ret != 0, -1);

 ret = tp_send_data(handle);
 retv_if(ret != 0, -1);

 ret = tp_finalize(handle);
 retv_if(ret != 0, -1);
 }
*/
