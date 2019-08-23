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

	for (int i = 2; i < 5; i++) {
		rdata = rand() % 100 + rand() % 100 / 100.0;
		sprintf(s1, "%.2f", rdata);

		ret = tp_set_field_value(handle, i, s1);
		retv_if(ret != 0, -1);
	}

	ret = tp_send_data(handle);
	retv_if(ret != 0, -1);

	ret = tp_finalize(handle);
	retv_if(ret != 0, -1);

	return ECORE_CALLBACK_RENEW;
}

bool service_app_create(void *data) {
	// Todo: add your code here.

	_D("===== start thingspark =====");

	// Ecore_Timer* ecore_timer_add(double interval, Ecore_Task_Cb func, const void *data)
	// 30초에 1번씩 _get_sensor_value 를 호출

	timer = ecore_timer_add(30.0f, _get_sensor_value, NULL);
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
