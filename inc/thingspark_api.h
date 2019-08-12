/*
 * thingspark_api.h
 *
 *  Created on: Aug 9, 2019
 *      Author: master
 */

#ifndef THINGSPARK_API_H_
#define THINGSPARK_API_H_

typedef struct tp_handle_s *tp_handle_h;
extern int tp_initialize(const char *api_key, tp_handle_h *handle);
extern int tp_set_field_value(tp_handle_h handle, int field, const char *value);
extern int tp_send_data(tp_handle_h handle);
extern int tp_finalize(tp_handle_h handle);

#endif /* THINGSPARK_API_H_ */
