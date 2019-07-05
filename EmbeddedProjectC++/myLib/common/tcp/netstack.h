/*
  FILE: NetStack.h
  Created on: 9/1/2017, by Tom Diaz
*/
#ifndef NETSTACK_H
#define NETSTACK_H

#include <x_nic.h>
#include <ip.h>
#include <arp.h>
#include <udp.h>
#include <dns.h>
#include <dhcp.h>
#include <tcp.h>
#include <httpclient.h>

#include <x_network.h>

//----- ETHERNET PACKET DEFINITIONS -----
#define	ETHERNET_HARDWARE_TYPE			0x0001
#define	ETHERNET_TYPE_ARP			0x0806
#define ETHERNET_TYPE_IP			0x0800

//----- STACK STATE MACHINE STATES -----
typedef enum _ETH_STACK_STATE
{
  SM_ETH_STACK_IDLE,
  SM_ETH_STACK_ARP,
  SM_ETH_STACK_IP,
  SM_ETH_STACK_ICMP,
  SM_ETH_STACK_ICMP_REPLY,
  SM_ETH_STACK_UDP,
  SM_ETH_STACK_TCP
    
} ETH_STACK_STATE;

class NetStack : public x_Network
{
public:
  NetStack(){};
  ~NetStack(){};
  void tcp_ip_Initialise (x_NIC* nic_);
  void tcp_ip_process_stack (void);
  int8_t DhcpState();
  void* GetObject(StackObject o);
  void SetObject(StackObject o, void* object);
protected:
  uint32_t ethernet_10ms_clock_timer_working;
  uint8_t sm_ethernet_stack;
  x_NIC* nic;
  IP ip;
  ARP arp;
  UDP udp;
  DHCP dhcp;
  DNS dns;
  TCP* tcp;
  HttpClient* httpClient;
private:
  void State_IDLE();
  void State_ARP();
  void State_IP();
  void State_ICMP();
  void State_ICMP_REPLY();
  void State_UDP();
  void State_TCP();
  DEVICE_INFO remoteDevice;
  MAC_ADDR rx_destination_mac_address;
  uint16_t rx_len;
  uint16_t data_remaining_bytes;
  IP_ADDR destination_ip_address;
  uint8_t process_stack_again;
};

#endif /* NETSTACK_H */

