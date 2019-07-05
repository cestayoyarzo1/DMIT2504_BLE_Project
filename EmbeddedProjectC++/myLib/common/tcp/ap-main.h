/*
  FILE: ap-main.h
  Created on: 8/27/2017, by Tom Diaz
*/
#ifndef AP_MAIN_H
#define AP_MAIN_H

#include <stdint.h>

void process_http_multipart_form_data(uint8_t data);
void process_http_multipart_form_last_section_done(void);
void process_http_multipart_form_header(const uint8_t *input_name, uint8_t *input_value, uint8_t *requested_filename, uint8_t *requested_filename_extension, uint8_t tcp_socket_number);
void process_http_inputs(uint8_t *input_name, uint8_t *input_value, uint8_t *requested_filename, uint8_t *requested_filename_extension, uint8_t tcp_socket_number);
#endif /* AP_MAIN_H */
