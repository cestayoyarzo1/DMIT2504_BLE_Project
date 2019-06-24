/*
  FILE: dns.h
  Created on: 8/29/2017, by Tom Diaz
*/
#ifndef DNS_H
#define DNS_H

//#include <udp.h>
//#include <dhcp.h>
//#include <arp.h>
#include <x_network.h>

#define	DNS_GATEWAY_DO_ARP_TIMEOUT_x100MS	40
#define	DNS_DO_REQUEST_TIMEOUT_x100MS		100		//Allow time for a router to connect to the internet if necessary

#define	DNS_MAX_URL_LENGTH			64

//UDP PORTS
#define	DNS_CLIENT_PORT				53
#define	DNS_SERVER_PORT				53

//QUERY TYPE VALUES
#define	QNS_QUERY_TYPE_HOST		        1
#define	QNS_QUERY_TYPE_MX		        15

/*typedef enum _SM_DNS
{
  SM_DNS_IDLE,
  SM_DNS_WAITING_TO_SEND,
  SM_DNS_WAITING_FOR_ARP_RESPONSE,
  SM_DNS_SEND_REQUEST,
  SM_DNS_WAITING_FOR_DNS_RESPONSE,
  SM_DNS_SUCCESS,
  SM_DNS_FAILED
} SM_DNS;*/

enum class SM_DNS
{
  SM_DNS_IDLE,
  SM_DNS_WAITING_TO_SEND,
  SM_DNS_WAITING_FOR_ARP_RESPONSE,
  SM_DNS_SEND_REQUEST,
  SM_DNS_WAITING_FOR_DNS_RESPONSE,
  SM_DNS_SUCCESS,
  SM_DNS_FAILED
};

class DNS
{
public:
  DNS(){};
  ~DNS(){};
  void Init(x_Network* net_);
  uint8_t Query(uint8_t *url_pointer, uint8_t dns_type);
  IP_ADDR CheckResponse(void);
  void FSM(void);
protected:
  void sendRequest(uint8_t *requested_domain_name, uint8_t qtype);
  uint8_t checkResponse(uint8_t *requested_domain_name, uint8_t qtype, 
                             IP_ADDR *resolved_ip_address);
  x_Network* net;
  uint8_t dns_100ms_timeout_timer;
  SM_DNS  dns_state = SM_DNS::SM_DNS_IDLE;
  uint8_t dns_socket = 0xff; // UDP Invalid Socket
  DEVICE_INFO DNSServerNode;
  uint8_t dns_requested_qtype;
  IP_ADDR dns_resolved_ip_address;
  uint8_t dns_requested_url[DNS_MAX_URL_LENGTH];
  uint8_t eth_dns_10ms_clock_timer_last;
private:
};

#endif /* DNS_H */

