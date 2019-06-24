/*
FILE: tcp.h
Created on: 8/29/2017, by Tom Diaz
*/
#ifndef TCP_H
#define TCP_H

#include <x_network.h>

//----- TCP SETUP -----
#define TCP_NO_OF_AVAILABLE_SOCKETS		6		//<<<<< SETUP FOR A NEW APPLICATION
//The number of TCP sockets available to the application (each socket uses sizeof(TCP_SOCKET_INFO) bytes of ram)
//Note that other stack components that use TCP will use sockets from this pool.

#if (TCP_NO_OF_AVAILABLE_SOCKETS <= 0 || TCP_NO_OF_AVAILABLE_SOCKETS > 255 )
#error TCP_NO_OF_AVAILABLE_SOCKETS value is out of range!
#endif

#define TCP_INITIAL_TIMEOUT_VALUE_10MS		(uint32_t)300		//Initial re-transmission timeout time, doubled after each timeout to provide a good compromise between local network and internet communications)
#define TCP_MAX_NUM_OF_RETRIES				3				//The number of retries to attempt before closing a TCP connection
#define	TCP_MAX_TIMEOUT_VALUE_10MS			(uint32_t)3000		//The maximum timeout value we should use

#define	TCP_USE_SOCKET_INACTIVITY_TIMOUT			//Comment out to allow a socket to be opened and remain open indefinately even if there is no activity

#define MAX_TCP_DATA_LEN    		(1500 - 40)		//Maximum size of the tcp packet data area (-40 allows for 20 byte tcp header and 20 byte ip header)
#define	TCP_MAX_SEGMENT_SIZE		(1500 - 40)		//Maximum size of tcp packet data area this device is willing to accept.  We don't use a variable window
//size as we're not buffering to memory.  We use a fixed window size of 1 packet to avoid a remote device
//sending lots of packets which we can't process fast enough and therefore get lost, to then be re-sent by
//the remote device after a delay.  -40 allows for 20 byte TCP header and 20 byte IP header
//TCP HEADER FLAGS
#define TCP_FIN						0x01
#define TCP_SYN						0x02
#define TCP_RST						0x04
#define TCP_PSH						0x08			//We don't use PSH as its use isn't properly defined anyway
#define TCP_ACK						0x10
#define TCP_URG						0x20			//We don't use URG, but it could easily be used if requried by an application

//TCP OPTIONS
#define TCP_OPTIONS_MAX_SEG_SIZE	0x02

#define TCP_LOCAL_PORT_NUMBER_START	1024			//Port number range available for automatic use as the local TCP port.
#define TCP_LOCAL_PORT_NUMBER_END	5000

#define TCP_INVALID_SOCKET			0xfe
#define TCP_INVALID_TX_BUFFER		0xff



//----- TCP STATE MACHINE STATES FOR EACH SOCKET -----
typedef enum _TCP_STATE
{
  SM_TCP_CLOSED,
  SM_TCP_LISTEN,
  SM_TCP_SYN_REQUEST_RECEIVED,
  SM_TCP_CONNECT_SEND_ARP_REQUEST,
  SM_TCP_CONNECT_WAIT_ARP_RESPONSE,
  SM_TCP_SYN_REQUEST_SENT,
  SM_TCP_CONNECTION_ESTABLISHED,
  SM_TCP_FIN_WAIT_1,
  SM_TCP_CLOSING,
  SM_TCP_LAST_ACK_SENT
} TCP_STATE;



//----- TCP SOCKET STRUCTURE -----
typedef struct _TCP_SOCKET_INFO
{
  uint8_t sm_socket_state;
  DEVICE_INFO remoteDevice;
  uint16_t local_port;
  uint16_t remote_port;
  uint8_t tx_last_tx_flags;
  uint32_t send_sequence_number;
  uint32_t send_acknowledgement_number;
  uint16_t next_segment_sequence_increment_value;
  uint8_t retry_count;
  uint16_t rx_data_bytes_remaining;
  uint8_t waiting_command_flags;
  uint32_t start_time;
  uint32_t time_out_value;
  struct
  {
    uint8_t ready_for_tx				:1;
    uint8_t tx_last_tx_awaiting_ack	:1;
    uint8_t tx_last_tx_had_data		:1;
    uint8_t tx_resend_last_tx			:1;			//If set the application function that owns the socket needs to re-send the last transmission
    uint8_t tx_send_waiting_command	:1;
    uint8_t socket_is_server			:1;
  } flags;
  
} TCP_SOCKET_INFO;



//----- TCP PACKET HEADER -----
typedef struct _TCP_HEADER
{
  uint16_t source_port;
  uint16_t destination_port;
  uint32_t sequence_number;
  uint32_t acknowledgment_number;
  
  union
  {
    struct
    {
      uint8_t reserved		:4;
      uint8_t val			:4;
    } bits;
    uint8_t byte;
  } header_length;
  
  union
  {
    struct
    {
      uint8_t flag_fin	:1;
      uint8_t flag_syn	:1;
      uint8_t flag_rst	:1;
      uint8_t flag_psh	:1;
      uint8_t flag_ack	:1;
      uint8_t flag_urg	:1;
      uint8_t reserved	:2;
    } bits;
    uint8_t byte;
  } flags;
  
  uint16_t window;
  uint16_t checksum;
  uint16_t urgent_pointer;
} TCP_HEADER;
#define	TCP_HEADER_LENGTH			20			//Defined to avoid sizeof problemms with compilers that add padd bytes


typedef struct _TCP_OPTIONS
{
  uint8_t id;
  uint8_t length;
  WORD_VAL max_seg_size;
} TCP_OPTIONS;
#define	TCP_OPTIONS_LENGTH			4			//Defined to avoid sizeof problemms with compilers that add padd bytes


//PSEUDO HEADER AS DEFINED BY RFC793
typedef struct _PSEUDO_HEADER
{
  IP_ADDR source_address;
  IP_ADDR destination_address;
  uint8_t zero;
  uint8_t protocol;
  uint16_t tcp_length;
} PSEUDO_HEADER;
#define	PSEUDO_HEADER_LENGTH		12			//Defined to avoid sizeof problemms with compilers that add padd bytes


class TCP
{
public:
  TCP(){};
  ~TCP(){};
  
  void tcp_Initialise(x_Network* net_);
  
  uint8_t tcp_process_rx(DEVICE_INFO *sender_device_info, 
                         IP_ADDR *destination_ip_address, uint16_t ip_data_area_bytes);
  
  void tcp_request_disconnect_socket(uint8_t socket);
  
  uint8_t tcp_open_socket_to_listen(uint16_t port);
  
  uint8_t tcp_connect_socket(DEVICE_INFO *remoteDevice, uint16_t port);
  
  void tcp_connect_socket_send_syn_request(uint8_t socket);
  
  void tcp_close_socket_from_listen(uint8_t socket);
  
  void tcp_close_socket(uint8_t socket);
  
  uint8_t tcp_is_socket_closed(uint8_t socket);
  
  uint8_t tcp_is_socket_connected(uint8_t socket);
  
  uint8_t tcp_is_socket_ready_to_tx_new_packet(uint8_t socket);
  
  uint8_t tcp_setup_socket_tx(uint8_t socket);
  
  uint8_t tcp_setup_tx(DEVICE_INFO *remoteDevice, uint16_t local_port, 
                       uint16_t remote_port, uint32_t tx_sequence_number, 
                       uint32_t tx_acknowledgment_number, uint8_t tx_flags);
  
  uint8_t tcp_write_next_byte(uint8_t data);
  
  uint8_t tcp_write_array(uint8_t *array_buffer, uint16_t array_length);
  
  void tcp_socket_tx_packet(uint8_t socket);
  
  void tcp_tx_packet(void);
  
  uint8_t tcp_does_socket_require_resend_of_last_packet(uint8_t socket);
  
  uint8_t tcp_check_socket_for_rx(uint8_t socket);
  
  uint8_t tcp_read_next_rx_byte(uint8_t *data);
  
  uint8_t tcp_read_rx_array(uint8_t *array_buffer, uint16_t array_length);
  
  void tcp_dump_rx_packet(void);
  
  void process_tcp(void);  
  
  uint8_t tcp_rx_packet_is_waiting_to_be_processed;
  TCP_SOCKET_INFO tcp_socket[TCP_NO_OF_AVAILABLE_SOCKETS];
  
protected:
  
  uint8_t tcp_rx_check_for_matches_socket(TCP_HEADER *rx_tcp_header, 
                                          DEVICE_INFO *rx_device_info);
  
  uint8_t process_tcp_segment(uint8_t socket, DEVICE_INFO *remoteDevice, 
                              TCP_HEADER *tcp_header,  uint16_t tcp_data_length);
  
  void tcp_send_command_packet_no_socket(DEVICE_INFO *remoteDevice, 
                                         uint16_t local_port, uint16_t remote_port, 
                                         uint32_t tx_sequence_number,	
                                         uint32_t tx_acknowledgment_number, 
                                         uint8_t flags);
  
  void tcp_send_command_packet_from_socket(uint8_t socket, uint8_t flags);
  
  void swap_tcp_header(TCP_HEADER* header);
  
  x_Network* net;
  
  uint8_t tcp_rx_active_socket;
  uint16_t next_local_tcp_port_to_use;
  uint32_t next_connection_start_sequence_number;
  uint16_t tcp_tx_checksum;
  uint8_t tcp_tx_checksum_next_byte_low;
  uint16_t tcp_tx_data_byte_length;
  uint8_t tcp_tx_resp_after_proc_rx_socket;
  uint8_t tcp_tx_resp_after_proc_rx_resp_flags;
private:
};

#endif /* TCP_H */

