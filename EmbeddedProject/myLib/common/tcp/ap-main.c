/*
  FILE: ap-main.c
  Created on: 8/27/2017, by Tom Diaz
*/

#include "ap-main.h"

//----- RECEIVE EACH BYTE OF THIS MULTIPART SECTION -----
//#define	HTTP_POST_MULTIPART_NEXT_BYTE_FUNCTION		process_http_multipart_form_data
//Your function definition needs to be:
//	void my_function_name (BYTE data)
//This function is called with each decoded byte in turn of a multipart section.  The data you get
//here is the same as the data submitted by the user (the driver deals with all decoding).
void process_http_multipart_form_data(uint8_t data)
{
}
//#define	HTTP_POST_LAST_MULTIPART_DONE_FUNCTION		process_http_multipart_form_last_section_done
//Your function definition needs to be:
//	void my_function_name (void)
//This function is called after the last byte has been received for a multipart section to allow you to carry out any
//operations with the data just received before the next multipart section is started or the end of the form post.
void process_http_multipart_form_last_section_done(void)
{
}
//----- PROCESS MULTIPART HEADER FOR NEXT SECTION FUNCTION -----
//#define	HTTP_POST_MULTIPART_HEADER_FUNCTION 	process_http_multipart_form_header

//Your function definition needs to be:
//	void my_function_name (CONSTANT BYTE *input_name, BYTE *input_value, BYTE *requested_filename, BYTE *requested_filename_extension, BYTE tcp_socket_number)
//input_name
//	Pointer to the null termianted header name string
//input_value
//	Pointer to the null termianted value string (converted to lowercase)
//requested_filename
//	Pointer to the null termianted string containing the filename, in case its useful
//requested_filename_extension
//	Pointer to the 3 byte string containing the filename extension, in case its useful
//tcp_socket_number
//	Included in case it is helpful for your application to identify a user (e.g. by their mac or IP address, tcp_socket[tcp_socket_number].remote_device_info.ip_address)
//
//This function is called for each header found for a new multipart section, of a multipart form post
//It will be called 1 or more times, and this signifies that when HTTP_POST_MULTIPART_NEXT_BYTE_FUNCTION is next called it will
//be with the data for this new section of the multipart message (i.e. any call to this function means your about to receive data
//for a new multipart section, so reset whatever your application needs to reset to start dealing with the new data).
//The following are the possible values that this function can be called with (always lower case):-
//	content-disposition		Value will be 'form-data' or 'file'
//	name					Name of the corresponding form control
//	filename				Name of the file when content-disposition = file (note that the client is not requried to
//							provide this, but usually will).  May or may not include full path depending on browser.
//							If its important to you to read filename with potentially long paths ensure you set
//							HTTP_MAX_POST_LINE_LENGTH to a value that not cause the end to be cropped off long paths.
//	content-type			Value dependant on the content.  e.g. text/plain, image/gif, etc.
//							If not present then you must assume content-type = text/plain; charset=us-ascii

void process_http_multipart_form_header(const uint8_t *input_name, uint8_t *input_value, uint8_t *requested_filename, uint8_t *requested_filename_extension, uint8_t tcp_socket_number)
{
}
//The function name in your application that will receive and process http inputs (typically but not necessarily from forms).  Comment out if not required.
//#define	HTTP_PROCESS_INPUT_FUNCTION			process_http_inputs

//Your function definition needs to be:
//		void my_function_name (BYTE *input_name, BYTE *input_value, BYTE *requested_filename, BYTE *requested_filename_extension, BYTE tcp_socket_number)
//input_name
//	Pointer to a null terminated string containing the input name sent by the client (i.e. the name of the form item in your HTML page)
//input_value
//	Pointer to a null terminated string containing the value returned for this item
//requested_filename
//	Pointer to a null terminated string containing the filename that will be returned to the client after the driver has finished reading all of the input
//	data.  Your application may alter this if desired (max length = HTTP_MAX_FILENAME_LENGTH).  Can be ignored if you wish.
//requested_filename_extension
//	Pointer to 3 byte string containing the filename extension.  Your application may alter this if desired or it can be ignored.
//tcp_socket_number
//	Included in case it is helpful for your application to identify a user (e.g. by their mac or IP address, tcp_socket[tcp_socket_number].remote_device_info.ip_address)
//	Can be ignored if you wish.
//You need to #include the file that contains the function definition at the top of eth-http.c for the compiler

void process_http_inputs(uint8_t *input_name, uint8_t *input_value, uint8_t *requested_filename, uint8_t *requested_filename_extension, uint8_t tcp_socket_number)
{
}
