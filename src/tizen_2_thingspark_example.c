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
 Filename:   tizen_2_thingspark_example.c
 Author: theksystem
 ************************************************************************/


#include <tizen.h>
#include <service_app.h>
#include <Ecore.h>
#include <thingspark.h>
#include <tizen_2_thingspark_example.h>
#include "log.h"

#include <json-glib/json-glib.h>
#include <curl/curl.h>

/* holder for curl fetch */
struct curl_fetch_st {
    char *payload;
    size_t size;
};

Ecore_Timer *timer = NULL;

Eina_Bool _get_sensor_value(void *data) {

	int ret = 0;
	tp_handle_h handle = NULL;

	// TODO API_KEY 변경
	// thingspark.kr 에서 채널별로 부여되는 api-key를 입력합니다.
	ret = tp_initialize("----------", &handle);
	retv_if(ret != 0, -1);

	char s1[10];               // 변환한 문자열을 저장할 배열
	double rdata = 0.0f; //실수 난수를 저장할 배열공간

	srand(time(NULL)); //랜덤한 숫자

	rdata = rand() % 100 + rand() % 100 / 100.0; //00.00~99.99 난수구하기
	sprintf(s1, "%.2f", rdata);   // %f를 지정하여 실수를 문자열로 저장

	ret = tp_set_field_value(handle, 1, s1);
	retv_if(ret != 0, -1);

	/*
	for (int i = 2; i < 5; i++) {
		rdata = rand() % 100 + rand() % 100 / 100.0;
		sprintf(s1, "%.2f", rdata);

		ret = tp_set_field_value(handle, i, s1);
		retv_if(ret != 0, -1);
	}
	*/

	ret = tp_send_data(handle);
	retv_if(ret != 0, -1);

	ret = tp_finalize(handle);
	retv_if(ret != 0, -1);

	return ECORE_CALLBACK_RENEW;
}



/* callback for curl fetch */
size_t curl_callback (void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;                             /* calculate buffer size */
    struct curl_fetch_st *p = (struct curl_fetch_st *) userp;   /* cast pointer to fetch struct */

    /* expand buffer */
    p->payload = (char *) realloc(p->payload, p->size + realsize + 1);

    /* check buffer */
    if (p->payload == NULL) {
      /* this isn't good */
      fprintf(stderr, "ERROR: Failed to expand buffer in curl_callback");
      /* free buffer */
      free(p->payload);
      /* return */
      return 1;
    }

    /* copy contents to buffer */
    memcpy(&(p->payload[p->size]), contents, realsize);

    /* set new buffer size */
    p->size += realsize;

    /* ensure null termination */
    p->payload[p->size] = 0;

    /* return size */
    return realsize;
}

CURLcode curl_fetch_url(CURL *ch, const char *url, struct curl_fetch_st *fetch) {
    CURLcode rcode;                   /* curl result code */

    /* init payload */
    fetch->payload = (char *) calloc(1, sizeof(fetch->payload));

    /* check payload */
    if (fetch->payload == NULL) {
        /* log error */
        fprintf(stderr, "ERROR: Failed to allocate payload in curl_fetch_url");
        /* return error */
        return CURLE_FAILED_INIT;
    }

    /* init size */
    fetch->size = 0;

    /* set url to fetch */
    curl_easy_setopt(ch, CURLOPT_URL, url);

    /* set calback function */
    curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, curl_callback);

    /* pass fetch struct pointer */
    curl_easy_setopt(ch, CURLOPT_WRITEDATA, (void *) fetch);

    /* set default user agent */
    curl_easy_setopt(ch, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    /* set timeout */
    curl_easy_setopt(ch, CURLOPT_TIMEOUT, 15);

    /* enable location redirects */
    curl_easy_setopt(ch, CURLOPT_FOLLOWLOCATION, 1);

    /* set maximum allowed redirects */
    curl_easy_setopt(ch, CURLOPT_MAXREDIRS, 1);

    /* fetch the url */
    rcode = curl_easy_perform(ch);

    /* return */
    return rcode;
}

Eina_Bool _get_thingspark_last_channel_entry(void *data) {

	CURL *ch;                                               /* curl handle */
	CURLcode rcode;                                         /* curl result code */

	JsonParser *parser = json_parser_new ();
	GError *gerror = NULL;

	struct curl_fetch_st curl_fetch;                        /* curl fetch struct */
	struct curl_fetch_st *cf = &curl_fetch;                 /* pointer to fetch struct */
	struct curl_slist *headers = NULL;                      /* http headers to send with request */

	/* url to test site */
	char *url = "https://api.thingspark.kr/channels/65688/last";

	/* init curl handle */
	if ((ch = curl_easy_init()) == NULL) {
		/* log error */
		fprintf(stderr, "ERROR: Failed to create curl handle in fetch_session");
		/* return error */
		return 1;
	}

	/* set content type */
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");

	/* set curl options */
	curl_easy_setopt(ch, CURLOPT_CUSTOMREQUEST, "GET");
	curl_easy_setopt(ch, CURLOPT_HTTPHEADER, headers);


	_D("url = %s", url);

	/* fetch page and capture return code */
	rcode = curl_fetch_url(ch, url, cf);

	/* cleanup curl handle */
	curl_easy_cleanup(ch);

	/* free headers */
	curl_slist_free_all(headers);


	/* check return code */
	if (rcode != CURLE_OK || cf->size < 1) {
		/* log error */
		fprintf(stderr, "ERROR: Failed to fetch url (%s) - curl said: %s",
			url, curl_easy_strerror(rcode));
		/* return error */
		return 1;
	}

	/* check payload */
	if (cf->payload != NULL) {
		/* print result */
		_D("CURL Returned: \n%s\n", cf->payload);
		/* parse return */
		//json = json_tokener_parse_verbose(cf->payload, &jerr);
		json_parser_load_from_data (parser, cf->payload, -1, &gerror);

		JsonReader *reader = json_reader_new (json_parser_get_root (parser));

		JsonNode *jnode = json_parser_get_root (parser);
		JsonObject *jobject;

		jobject = json_node_get_object (jnode);

		const char *create_at = json_object_get_string_member (jobject, "created_at");
		const char *field2 = json_object_get_string_member (jobject, "field2");

		_D("Read Value : \n%s\n", create_at);
		_D("Read Value : \n%s\n", field2);

		/* Clean up */
		g_object_unref (reader);
		g_object_unref (parser);

		/* free payload */
		free(cf->payload);
	} else {
		/* error */
		fprintf(stderr, "ERROR: Failed to populate payload");
		/* free payload */
		free(cf->payload);
		/* return */
		return 1;
	}

	return ECORE_CALLBACK_RENEW;
}

bool service_app_create(void *data) {
	// Todo: add your code here.

	_D("===== start thingspark =====");

	// Ecore_Timer* ecore_timer_add(double interval, Ecore_Task_Cb func, const void *data)
	// 30초에 1번씩 _get_sensor_value 를 호출

	timer = ecore_timer_add(30.0f, _get_thingspark_last_channel_entry, NULL);

	//timer = ecore_timer_add(30.0f, _get_sensor_value, NULL);
	if (!timer)
		_E("cannot add a timer(%s)", "hello");

	return true;
}

void service_app_terminate(void *data) {
	// Todo: add your code here.
	if (timer) {
		ecore_timer_del(timer);
		timer = NULL;
	}
	return;
}

void service_app_control(app_control_h app_control, void *data) {
	// Todo: add your code here.
	return;
}

static void service_app_lang_changed(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_LANGUAGE_CHANGED*/
	return;
}

static void service_app_region_changed(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void service_app_low_battery(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_LOW_BATTERY*/
}

static void service_app_low_memory(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_LOW_MEMORY*/
}

int main(int argc, char* argv[]) {
	char ad[50] = { 0, };
	service_app_lifecycle_callback_s event_callback;
	app_event_handler_h handlers[5] = { NULL, };

	event_callback.create = service_app_create;
	event_callback.terminate = service_app_terminate;
	event_callback.app_control = service_app_control;

	service_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, service_app_low_battery, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, service_app_low_memory, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, service_app_lang_changed, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, service_app_region_changed, &ad);

	return service_app_main(argc, argv, &event_callback, ad);
}


