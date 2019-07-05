/*
FILE: arp.h
Created on: 8/28/2017, by Tom Diaz
*/
#ifndef ARP_H
#define ARP_H

#define	ETHERNET_PROTOCOL_ARP		0x0800
#define ARP_OPCODE_REQUEST 		0x0001
#define ARP_OPCODE_RESPONSE 		0x0002

#include <stdint.h>
#include <netstructs.h>
#include <x_network.h>

//----- ARP STATE MACHINE STATES -----
enum class ARP_STATE
{
  SM_ARP_IDLE,
  SM_ARP_SEND_REPLY
};

//----- DATA TYPE DEFINITIONS -----
//ARP PACKET
typedef struct _ARP_PACKET
{
  uint16_t		hardware_type;
  uint16_t		protocol;
  uint8_t		mac_addr_len;
  uint8_t		protocol_len;
  uint16_t		op_code;
  MAC_ADDR	        sender_mac_addr;
  IP_ADDR		sender_ip_addr;
  MAC_ADDR	        target_mac_addr;
  IP_ADDR		target_ip_addr;
} ARP_PACKET;

class ARP
{
public:
  ARP(){};
  ~ARP(){};
  void Init(x_Network* net_);
  uint8_t ResolveIP(IP_ADDR *ip_address_to_resolve);
  uint8_t ResolveIsComplete(IP_ADDR *ip_address_being_resolved, 
                                   MAC_ADDR *resolved_mac_address);
  uint8_t Rx(void);
protected:
  ARP_PACKET arp_packet;
  void txPacket(DEVICE_INFO *remoteDevice, uint16_t op_code);
  ARP_STATE fsmState;
  DEVICE_INFO lastResponse;
  DEVICE_INFO remoteDevice;
  x_Network* net;
private:
};

#endif /* ARP_H */
