/*
  FILE: $FILE_FNAME$
  Created on: 8/29/2017, by Tom Diaz
*/
#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

//-------------------------------------------------
//----- FUNCTION TO PROCESS HTTP CLIENT FILES -----
//-------------------------------------------------
//The function name in your application that will be called with each sequential packet of requested HTTP files:
#define HTTP_CLIENT_REQUEST_RECEIVE_FUNCTION	process_http_client_request

//Your function definition needs to be:
//		void my_function_name (uint8_t op_code, uint32_t content_length, uint8_t *requested_host_url, uint8_t *requested_filename)
//op_code
//	0 = error - http client failed.
//	1 = OK.  First section of TCP file data ready to be read.  Function should use the tcp_read_next_rx_byte or tcp_read_rx_array functions to read all of the data from this TCP packet before returning.
//	2 = Next section of TCP file data ready to be read.  Function should use the tcp_read_next_rx_byte or tcp_read_rx_array functions to read all of the data from this TCP packet before returning.
//	0xff = The remote server has closed the connection (this will mark the end of the file if content-length was not provided by the server)
//content_length
//	The file length specified by the server at the start of the response.  Note that the server is not requried to specify this (and many don't) and in this instance the value will be 0xffffffff;
//requested_host_url
//	Pointer to a null terminated string containing the host url that was originally requested
//requested_filename
//	Pointer to a null terminated string containing the filename that was originally requested
//
//Read each data byte in the packet using:
//	while (tcp_read_next_rx_byte(&my_byte_variable))
//	{
//
//This function is always called after a request, either to either indicate the request failed, or with 1 or more packets of file data.  If the request was sucessful the file data packets will be received
//in the correct order and the tcp data may simply be stored as it is read.  The HTTP Client get request headers specify that no encoding may be used by the remote server so the file will be received exactly
//as it is stored on the server.

#include <x_network.h>

#define	HTTP_CLIENT_MAX_URL_LENGTH              128
#define	HTTP_CLIENT_MAX_FILENAME_LENGTH		128

const uint8_t http_client_request_text_get_start[] = {"GET /"};
const uint8_t http_client_request_text_get_end[] = {" HTTP/1.0\r\n"};
const uint8_t http_client_request_text_host_start[] = {"Host: "};
//Accept-Encoding blank = server must send using identity encoding (no encoding is permissible). End with blank linee
const uint8_t http_client_request_text_remainder[] = {"\r\nAccept-Encoding: \r\n\r\n"};	
const uint8_t http_client_content_length_text[] = {"content-length:"};



enum class SM_HTTP_CLIENT
{
    IDLE,
    RESOLVE_URL_START,
    RESOLVE_URL_WAIT,
    OPEN_SOCKET,
    WAIT_FOR_CONNECTION,
    WAIT_FOR_RESPONSE,
    WAIT_FOR_RESPONSE_1,
    TX_REQUEST,
    REQUEST_DISCONNECT,
    WAIT_FOR_DISCONNECT,
    FAIL
}; //SM_HTTP_CLIENT;

class HttpClient
{
public:
  HttpClient(){};
  ~HttpClient(){};
  void Init(x_Network* net_);
  uint8_t start_http_client_request (const uint8_t *host_url, const uint8_t *filename);
  void process_http_client (void);
protected:
  void http_client_send_const_string (const uint8_t *string_to_send);
  void process_http_client_request(uint8_t op_code, uint32_t content_length, 
                                   uint8_t *requested_host_url, uint8_t *requested_filename);
  x_Network* net;
  uint8_t our_tcp_client_socket = 0xfe; //TCP Invalid socket
  uint16_t our_tcp_client_local_port;
  DEVICE_INFO remoteDevice;
  uint8_t eth_http_client_10ms_clock_timer_last;
  uint32_t http_client_content_length;
  uint32_t http_client_content_received;
  uint8_t header_started;
  uint8_t header_ended;
  uint8_t data_buffer[4];
  uint8_t content_length_match_char;
  SM_HTTP_CLIENT http_client_state = SM_HTTP_CLIENT::IDLE;
  uint8_t http_client_host_url[HTTP_CLIENT_MAX_URL_LENGTH];
  uint8_t http_client_filename[HTTP_CLIENT_MAX_FILENAME_LENGTH];
  uint8_t http_client_100ms_timeout_timer;
  uint8_t http_client_request_ok;
private:
};


#endif /* HTTPCLIENT_H */
