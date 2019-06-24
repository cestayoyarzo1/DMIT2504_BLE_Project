/*
  FILE: x_network.h
  Created on: 9/1/2017, by Tom Diaz
*/
#ifndef X_NETWORK_H
#define X_NETWORK_H

#include <stdint.h>
#include <netstructs.h>

enum class StackObject
{
  NIC,
  IP,
  ARP,
  UDP,
  DHCP,
  TCP,
  DNS,
  HTTP_CLIENT,
  HTTP_SERVER,
};

class x_Network
{
public:
  virtual int8_t DhcpState() = 0;
  virtual void* GetObject(StackObject o) = 0;
  virtual void SetObject(StackObject o, void* object) = 0;
  void SetLocalIP(STATIC_ADDR* info)
  {
    ManualIp = false;
    if(info)
    {
      ManualIp = true;
      LocalIpAddress.Val = info->LocalIP.Val;
      GatewayIpAddress.Val = info->LocalGateway.Val;
      LocalSubnetMask.Val = info->LocalSubnet.Val;
    }
  }
  uint32_t ethernet_10ms_clock_timer;
  IP_ADDR LocalIpAddress;
  IP_ADDR GatewayIpAddress;
  IP_ADDR LocalSubnetMask;
  bool NicLinkedAndIpValid;
  bool ManualIp;
protected:
  //bool dhcpEnabled;
};

#endif /* X_NETWORK_H */
