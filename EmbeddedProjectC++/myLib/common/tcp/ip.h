/*
FILE: ip.h
Created on: 8/28/2017, by Tom Diaz
*/
#ifndef IP_H
#define IP_H

#include <stdint.h>
#include <x_network.h>

class IP
{
public:
  IP();
  ~IP(){};
  void Init(x_Network* net_);
  uint8_t ip_get_header(IP_ADDR *destination_ip, DEVICE_INFO *remoteDevice, 
                        uint8_t *ip_protocol, uint16_t *length);
  void ip_write_header(DEVICE_INFO *remoteDevice, uint8_t ip_protocol);
  void ip_add_bytes_to_ip_checksum (uint16_t *checksum, 
                                    uint8_t *checksum_next_byte_is_low, 
                                    uint8_t *next_byte, uint8_t no_of_bytes_to_add);
  void ip_tx_packet (void);
protected:
  //x_NIC* nic;
  uint16_t ip_packet_identifier;
  uint16_t ip_tx_ip_header_checksum;
  uint8_t ip_tx_ip_header_checksum_next_byte_low;
  x_Network* net;
private:
};


#endif /* IP_H */
