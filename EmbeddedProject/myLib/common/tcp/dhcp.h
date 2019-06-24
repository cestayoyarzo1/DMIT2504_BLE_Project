/*
FILE: dhcp.h
Created on: 8/28/2017, by Tom Diaz
*/
#ifndef DHCP_H
#define DHCP_H

#include <x_network.h>

//----- USER OPTIONS -----
//#define	DHCP_DO_POWERUP_RANDOM_DELAY	//Comment out to cause a DHCP request to be sent immediatly on powerup without a random delay (random delays should
//be used if a large number of devices could powerup at the same time)
//----- TIMEOUTS -----
#define	DHCP_DISCOVER_TIMEOUT		10000	//x1mS
#define	DHCP_REQUEST_TIMEOUT		10000	//x1mS
#define DHCP_LEASE_MIN_SECS 		60		//Minimum lease renewal time for us in seconds
#define DHCP_LEASE_MAX_SECS 		86400	//Maximum lease renewal time for us in seconds


//----- DHCP MESSAGE TYPES -----
#define DHCP_MESSAGE_DISCOVER		1
#define DHCP_MESSAGE_OFFER		2
#define DHCP_MESSAGE_REQUEST		3
#define DHCP_MESSAGE_DECLINE		4
#define DHCP_MESSAGE_ACK		5
#define DHCP_MESSAGE_NAK		6
#define DHCP_MESSAGE_RELEASE		7
#define DHCP_MESSAGE_INFORM		8

//----- ETHERNET VALUES -----
#define DHCPSERVER_PORT		67
#define DHCPCLIENT_PORT		68

enum class DHCP_STATE
{
  DHCP_INIT,
  DHCP_DISCOVER,
  DHCP_WAIT_FOR_OFFER_RESPONSE,
  DHCP_REQUEST,
  DHCP_RENEWING,
  DHCP_WAIT_FOR_REQUEST_RESPONSE,
  DHCP_BOUND
};

class DHCP
{
public:
  DHCP(){};
  ~DHCP(){};
  void Init(x_Network* net_);
  void FSM(void);
  DHCP_STATE sm_dhcp;
protected:
private:
  void txPacket(uint8_t message_type);
  uint8_t rxPacket(void);
  x_Network* net;
  uint8_t dhcp_socket;
  IP_ADDR serverIP;
  MAC_ADDR serverMAC;
  IP_ADDR offerIP;
  uint8_t *eth_dhcp_our_name_pointer;
  DWORD eth_dhcp_1sec_renewal_timer;
  DWORD eth_dhcp_1sec_lease_timer;
  WORD eth_dhcp_1ms_timer;
};

#endif /* DHCP_H */
