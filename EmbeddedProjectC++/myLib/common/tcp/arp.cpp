/*
FILE: arp.cpp
Created on: 8/28/2017, by Tom Diaz
*/

#include <arp.h>
#include <iputils.h>

//************************************
//************************************
//********** Initialise ARP **********
//************************************
//************************************
//Called from the tcp_ip_Initialise function
void ARP::Init(x_Network* net_)
{
  net = net_;
  
  fsmState = ARP_STATE::SM_ARP_IDLE;
  
  lastResponse.mac_address.v[0] = 0;
  lastResponse.mac_address.v[1] = 0;
  lastResponse.mac_address.v[2] = 0;
  lastResponse.mac_address.v[3] = 0;
  lastResponse.mac_address.v[4] = 0;
  lastResponse.mac_address.v[5] = 0;
  lastResponse.ip_address.Val = 0x00000000;
}
//********************************************
//********************************************
//********** ARP RESOLVE IP ADDRESS **********
//********************************************
//********************************************
//Returns 1 if request was sent, 0 if request can't be sent right now
uint8_t ARP::ResolveIP(IP_ADDR *ip_address_to_resolve)
{
  DEVICE_INFO remoteDevice;
  x_NIC* nic = (x_NIC*)net->GetObject(StackObject::NIC);
  
  //Setup ready to tx a packet
  if (nic->SetupTx() == 0)
    return(0);		//nic is not currently ready for a new tx packet
  
  //Clear the last resolved arp ip address
  lastResponse.ip_address.Val = 0x00000000;
  
  //Setup packet to tx
  remoteDevice.mac_address.v[0] = 0xff;
  remoteDevice.mac_address.v[1] = 0xff;
  remoteDevice.mac_address.v[2] = 0xff;
  remoteDevice.mac_address.v[3] = 0xff;
  remoteDevice.mac_address.v[4] = 0xff;
  remoteDevice.mac_address.v[5] = 0xff;
  
  remoteDevice.ip_address = *ip_address_to_resolve;
  
  //Send the packet
  txPacket(&remoteDevice, ARP_OPCODE_REQUEST);
  
  return(1);
}
//*********************************************
//*********************************************
//********** CHECK ARP RESOLVE STATE **********
//*********************************************
//*********************************************
//ip_address_being_resolved = The IP address that is waiting to be resolved
//resolved_mac_address = The mac address for the resolved IP address will be stored to here
//Returns 1 if resolved, 0 if not yet resolved
uint8_t ARP::ResolveIsComplete(IP_ADDR *ip_address_being_resolved, 
                                     MAC_ADDR *resolved_mac_address)
{
  
  if (
      (lastResponse.ip_address.Val == ip_address_being_resolved->Val) ||											//If IP address matches last returned ARP response then arp is resolved
        ((net->GatewayIpAddress.Val != 0x00000000) && (lastResponse.ip_address.Val == net->GatewayIpAddress.Val))	//or if the last returned IP address is the gateway IP address then arp
          )																																//is resolved (this would mean the target IP is on a different subnet so
  {
    resolved_mac_address->v[0] = lastResponse.mac_address.v[0];
    resolved_mac_address->v[1] = lastResponse.mac_address.v[1];
    resolved_mac_address->v[2] = lastResponse.mac_address.v[2];
    resolved_mac_address->v[3] = lastResponse.mac_address.v[3];
    resolved_mac_address->v[4] = lastResponse.mac_address.v[4];
    resolved_mac_address->v[5] = lastResponse.mac_address.v[5];
    return(1);
  }
  return(0);
}
//*******************************************
//*******************************************
//********** PROCESS ARP RX PACKET **********
//*******************************************
//*******************************************
//Called from the recevied packet ethernet stack state machine
//Returns 1 if ARP was processed, 0 if couldn't do it this time
uint8_t ARP::Rx (void)
{
  x_NIC* nic = (x_NIC*)net->GetObject(StackObject::NIC);
  
  switch(fsmState)
  {
  case ARP_STATE::SM_ARP_IDLE:
    //-----------------------------------------------------------------------------
    //-----------------------------------------------------------------------------
    //----- AN ARP PACKET IS WAITING TO BE READ OUT OF THE NIC RECEIVE BUFFER -----
    //-----------------------------------------------------------------------------
    //-----------------------------------------------------------------------------
    
    //READ THE PACKET INTO OUR BUFFER
    //We can't do this as the ARP_PACKET structure is not aligned for 32bit architectures so will often be padded:
    //if (!ReadArray((uint8_t*)&arp_packet, sizeof(arp_packet)))
    //	return(1);									//Error - packet was too small - dump it
    
    //Instead we do this to avoid padding problems:
    if (!nic->ReadArray((uint8_t*)&arp_packet.hardware_type, 2))
      return(1);									//Error - packet was too small - dump it
    if (!nic->ReadArray((uint8_t*)&arp_packet.protocol, 2))
      return(1);									//Error - packet was too small - dump it
    if (!nic->ReadNextByte(&arp_packet.mac_addr_len))
      return(1);									//Error - packet was too small - dump it
    if (!nic->ReadNextByte(&arp_packet.protocol_len))
      return(1);									//Error - packet was too small - dump it
    if (!nic->ReadArray((uint8_t*)&arp_packet.op_code, 2))
      return(1);									//Error - packet was too small - dump it
    if (!nic->ReadArray((uint8_t*)&arp_packet.sender_mac_addr, 6))
      return(1);									//Error - packet was too small - dump it
    if (!nic->ReadArray((uint8_t*)&arp_packet.sender_ip_addr, 4))
      return(1);									//Error - packet was too small - dump it
    if (!nic->ReadArray((uint8_t*)&arp_packet.target_mac_addr, 6))
      return(1);									//Error - packet was too small - dump it
    if (!nic->ReadArray((uint8_t*)&arp_packet.target_ip_addr, 4))
      return(1);									//Error - packet was too small - dump it
    
    //Swap the uint8_ts in hardware type, protocol & operation
    arp_packet.hardware_type = IPUtils::swap_word_bytes(arp_packet.hardware_type);
    arp_packet.protocol = IPUtils::swap_word_bytes(arp_packet.protocol);
    arp_packet.op_code = IPUtils::swap_word_bytes(arp_packet.op_code);
    
    //DUMP THE PACKET FROM THE NIC BUFFER
    nic->DumpPacket();
    
    //Check hardware type
    //(Don't care about the high uint8_t, the low uint8_t should be 0x01)
    if ((arp_packet.hardware_type & 0x00ff) != 0x0001)
      return(1);
    
    //Check protocol
    //(Should be 0x0800)
    if (arp_packet.protocol != 0x0800)
      return(1);
    
    //Check the mac address length
    if (arp_packet.mac_addr_len != MAC_ADDR_LENGTH)
      return(1);
    
    //Check the IP address length
    if (arp_packet.protocol_len != IP_ADDR_LENGTH)
      return(1);
    
    //CHECK THE TARGET IP ADDRESS MATCHES US
    if ((arp_packet.target_ip_addr.v[0] != net->LocalIpAddress.v[0]) ||
        (arp_packet.target_ip_addr.v[1] != net->LocalIpAddress.v[1]) ||
          (arp_packet.target_ip_addr.v[2] != net->LocalIpAddress.v[2]) ||
            (arp_packet.target_ip_addr.v[3] != net->LocalIpAddress.v[3]))
    {
      //----- PACKET IS NOT FOR US -----
      return(1);
    }
    
    //----- CHECK THE OP CODE -----
    //(Don't care about the high uint8_t)
    if ((arp_packet.op_code & 0x00ff) == ARP_OPCODE_REQUEST)
    {
      //--------------------------------
      //----- ARP REQUEST RECEIVED -----
      //--------------------------------
      //Store the remote device details in case we can't tx our response now
      remoteDevice.mac_address.v[0] = arp_packet.sender_mac_addr.v[0];
      remoteDevice.mac_address.v[1] = arp_packet.sender_mac_addr.v[1];
      remoteDevice.mac_address.v[2] = arp_packet.sender_mac_addr.v[2];
      remoteDevice.mac_address.v[3] = arp_packet.sender_mac_addr.v[3];
      remoteDevice.mac_address.v[4] = arp_packet.sender_mac_addr.v[4];
      remoteDevice.mac_address.v[5] = arp_packet.sender_mac_addr.v[5];
      remoteDevice.ip_address.Val = arp_packet.sender_ip_addr.Val;
      
      fsmState = ARP_STATE::SM_ARP_SEND_REPLY;
      //Don't break so that we attempt to reply now;
    }
    else if ((arp_packet.op_code & 0x00ff) == ARP_OPCODE_RESPONSE)
    {
      //---------------------------------
      //----- ARP RESPONSE RECEIVED -----
      //---------------------------------
      //Store the response ready for the requesting function to check when its next called
      lastResponse.mac_address.v[0] = arp_packet.sender_mac_addr.v[0];
      lastResponse.mac_address.v[1] = arp_packet.sender_mac_addr.v[1];
      lastResponse.mac_address.v[2] = arp_packet.sender_mac_addr.v[2];
      lastResponse.mac_address.v[3] = arp_packet.sender_mac_addr.v[3];
      lastResponse.mac_address.v[4] = arp_packet.sender_mac_addr.v[4];
      lastResponse.mac_address.v[5] = arp_packet.sender_mac_addr.v[5];
      lastResponse.ip_address.Val = arp_packet.sender_ip_addr.Val;
      return(1);
    }
    else
    {
      //---------------------------
      //----- INVALID OP CODE -----
      //---------------------------
      return(1);
    }
    
    
    
  case ARP_STATE::SM_ARP_SEND_REPLY:		//case SM_ARP_IDLE will fall into this case if we want to try to reply to a received request
    //-----------------------------
    //-----------------------------
    //----- SEND ARP RESPONSE -----
    //-----------------------------
    //-----------------------------
    if (nic->SetupTx())
    {
      //--------------------------------------------
      //----- WE CAN TRANSMIT OUR RESPONSE NOW -----
      //--------------------------------------------
      txPacket(&remoteDevice, ARP_OPCODE_RESPONSE);
      fsmState = ARP_STATE::SM_ARP_IDLE;
      return(1);
    }
    else
    {
      //-----------------------------------------------------------
      //----- CAN'T TX OUR RESPONSE NOW - TRY AGAIN NEXT TIME -----
      //-----------------------------------------------------------
      return(0);
    }
    
  }
  return(0);
}
//***********************************
//***********************************
//********** ARP TX PACKET **********
//***********************************
//***********************************
//SetupTx() must have been called first
void ARP::txPacket (DEVICE_INFO *remoteDevice, uint16_t op_code)
{
  x_NIC* nic = (x_NIC*)net->GetObject(StackObject::NIC);
  //----- WRITE THE ETHERNET HEADER -----
  nic->WriteEthernetHeader(&remoteDevice->mac_address, ETHERNET_TYPE_ARP);
  
  //----- WRITE THE ARP PACKET -----
  //SEND HARDWARE TYPE [2]
  nic->WriteNextByte((BYTE)(ETHERNET_HARDWARE_TYPE >> 8));
  nic->WriteNextByte((BYTE)(ETHERNET_HARDWARE_TYPE & 0x00ff));
  
  //SEND PROTOCOL TYPE [2]
  nic->WriteNextByte((BYTE)(ETHERNET_PROTOCOL_ARP >> 8));
  nic->WriteNextByte((BYTE)(ETHERNET_PROTOCOL_ARP & 0x00ff));
  
  //SEND MAC ADDRESS LENGTH [1]
  nic->WriteNextByte((BYTE)MAC_ADDR_LENGTH);
  
  //SEND IP ADDRESS LENGTH [1]
  nic->WriteNextByte((BYTE)IP_ADDR_LENGTH);
  
  //SEND OP CODE [2]
  nic->WriteNextByte((BYTE)(op_code >> 8));
  nic->WriteNextByte((BYTE)(op_code & 0x00ff));
  
  //SEND OUR MAC ADDRESS [6]
  nic->WriteNextByte(nic->MAC()->v[0]);
  nic->WriteNextByte(nic->MAC()->v[1]);
  nic->WriteNextByte(nic->MAC()->v[2]);
  nic->WriteNextByte(nic->MAC()->v[3]);
  nic->WriteNextByte(nic->MAC()->v[4]);
  nic->WriteNextByte(nic->MAC()->v[5]);
  
  //SEND OUR IP ADDRESS [4]
  nic->WriteNextByte(net->LocalIpAddress.v[0]);
  nic->WriteNextByte(net->LocalIpAddress.v[1]);
  nic->WriteNextByte(net->LocalIpAddress.v[2]);
  nic->WriteNextByte(net->LocalIpAddress.v[3]);
  
  //SEND DESTINATION MAC ADDRESS [6]
  if (op_code == ARP_OPCODE_RESPONSE)
  {
    //Sending a response - return to senders mac
    nic->WriteNextByte(remoteDevice->mac_address.v[0]);
    nic->WriteNextByte(remoteDevice->mac_address.v[1]);
    nic->WriteNextByte(remoteDevice->mac_address.v[2]);
    nic->WriteNextByte(remoteDevice->mac_address.v[3]);
    nic->WriteNextByte(remoteDevice->mac_address.v[4]);
    nic->WriteNextByte(remoteDevice->mac_address.v[5]);
  }
  else
  {
    //Sending a request - broadcast
    nic->WriteNextByte(0xff);
    nic->WriteNextByte(0xff);
    nic->WriteNextByte(0xff);
    nic->WriteNextByte(0xff);
    nic->WriteNextByte(0xff);
    nic->WriteNextByte(0xff);
  }
  
  //SEND DESTINATION IP ADDRESS [4]
  //If destination is not on the same subnet as us then send this to our gateway instead
  //Once the gateway returns its mac address the pending TCP/IP communication will automatically get sent to the
  //gateway which will do its job and forward packets on.
  if (((net->LocalIpAddress.v[0] ^ remoteDevice->ip_address.v[0]) & net->LocalSubnetMask.v[0]) ||
      ((net->LocalIpAddress.v[1] ^ remoteDevice->ip_address.v[1]) & net->LocalSubnetMask.v[1]) ||
        ((net->LocalIpAddress.v[2] ^ remoteDevice->ip_address.v[2]) & net->LocalSubnetMask.v[2]) ||
          ((net->LocalIpAddress.v[3] ^ remoteDevice->ip_address.v[3]) & net->LocalSubnetMask.v[3]) )
  {
    //TARGET IS NOT ON OUR SUBNET - USE THE GATEWAY IP ADDRESS
    nic->WriteNextByte(net->GatewayIpAddress.v[0]);
    nic->WriteNextByte(net->GatewayIpAddress.v[1]);
    nic->WriteNextByte(net->GatewayIpAddress.v[2]);
    nic->WriteNextByte(net->GatewayIpAddress.v[3]);
  }
  else
  {
    //TARET IS ON OUR SUBNET - USE THE TARGET IP ADDRESS
    nic->WriteNextByte(remoteDevice->ip_address.v[0]);
    nic->WriteNextByte(remoteDevice->ip_address.v[1]);
    nic->WriteNextByte(remoteDevice->ip_address.v[2]);
    nic->WriteNextByte(remoteDevice->ip_address.v[3]);
  }
  
  
  
  //----- SEND THE PACKET -----
  nic->TxPacket();
  
  return;
}