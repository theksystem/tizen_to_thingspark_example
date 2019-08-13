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

#ifndef THINGSPARK_H_
#define THINGSPARK_H_

typedef struct tp_handle_s *tp_handle_h;
extern int tp_initialize(const char *api_key, tp_handle_h *handle);
extern int tp_set_field_value(tp_handle_h handle, int field, const char *value);
extern int tp_send_data(tp_handle_h handle);
extern int tp_finalize(tp_handle_h handle);

#endif /* THINGSPARK_H_ */
