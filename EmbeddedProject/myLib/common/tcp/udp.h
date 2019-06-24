/*
FILE: udp.h
Created on: 8/28/2017, by Tom Diaz
*/
#ifndef UDP_H
#define UDP_H

#include <x_network.h>

//----- UDP SETUP -----
#define UDP_NO_OF_AVAILABLE_SOCKETS		4		//<<<<< SETUP FOR A NEW APPLICATION
//The number of UDP sockets available to the application (each socket uses sizeof(UDP_SOCKET_INFO) bytes of ram)
//Note that other stack components that use UDP will use sockets from this pool.

#if (UDP_NO_OF_AVAILABLE_SOCKETS <= 0 || UDP_NO_OF_AVAILABLE_SOCKETS > 255 )
#error UDP_NO_OF_AVAILABLE_SOCKETS value is out of range!
#endif


#define	UDP_CHECKSUMS_ENABLED					//Comment out if you do not want to use checksums for UDP tx and rx


//UDP PORTS:-
#define UDP_PORT_NULL			0xffff
#define	UDP_INVALID_SOCKET		0xff		//Value returned when UDP socket not found


//----- DATA TYPE DEFINITIONS -----
typedef struct _UDP_SOCKET_INFO
{
  DEVICE_INFO		remoteDevice;
  uint16_t			local_port;
  uint16_t			remote_port;
  uint16_t			rx_data_bytes_remaining;
  IP_ADDR			destination_ip_address;		//Contains the IP address the last received packet was sent to (not used for tx)
} UDP_SOCKET_INFO;


typedef struct _UDP_HEADER
{
  uint16_t		source_port;
  uint16_t		destination_port;
  uint16_t		length;
  uint16_t		checksum;
} UDP_HEADER;
#define	UDP_HEADER_LENGTH			8			//Defined to avoid sizeof problemms with compilers that add padd bytes


class UDP
{
public:
  UDP(){};
  ~UDP(){};
  UDP_SOCKET_INFO udp_socket[UDP_NO_OF_AVAILABLE_SOCKETS];
  uint8_t udp_rx_packet_is_waiting_to_be_processed;
  void udp_Initialise(x_Network* net_);
  uint8_t udp_process_rx(DEVICE_INFO *sender_device_info, 
                          IP_ADDR *destination_ip_address, uint16_t ip_data_area_bytes);
  uint8_t udp_open_socket(DEVICE_INFO *device_info, uint16_t local_port, uint16_t remote_port);
  void udp_close_socket(uint8_t *socket);
  uint8_t udp_check_socket_for_rx(uint8_t socket);
  uint8_t udp_read_next_rx_byte (uint8_t *data);
  uint8_t udp_read_rx_array(uint8_t *array_buffer, uint16_t array_length);
  void udp_dump_rx_packet(void);
  uint8_t udp_setup_tx(uint8_t socket);
  void udp_write_next_byte(uint8_t data);
  void udp_write_array(uint8_t *array_buffer, uint16_t array_length);
  void udp_tx_packet(void);
protected:
  uint8_t udp_rx_check_for_matches_socket(UDP_HEADER *rx_udp_header, DEVICE_INFO *rx_device_info);
  uint8_t udp_rx_active_socket;
  uint16_t udp_tx_checksum;
  uint8_t udp_tx_checksum_next_byte_low;
  x_Network* net;
private:
};

#endif /* UDP_H */
