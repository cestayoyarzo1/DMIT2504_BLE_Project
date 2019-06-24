/*
FILE: ip.cpp
Created on: 8/28/2017, by Tom Diaz
*/

#include <ip.h>
#include <iputils.h>

IP::IP()
{
  ip_packet_identifier = 0;
}
//------------------------------------------------------------------------------
void IP::Init(x_Network* net_)
{
  net = net_;
}
//***********************************
//***********************************
//********** GET IP HEADER **********
//***********************************
//***********************************
//Returns 1 if IP header OK, 0 if invalid packet (packet will have been dumped)
//destination_ip = the ip address this packet was sent to (checking for part of our subnet is done by this function)
//remoteDevice = the IP address field is loaded (mac address field is not altered)
//ip_protocol = the ip protocol byte
//length = the length of the data area of this packet

uint8_t IP::ip_get_header(IP_ADDR *destination_ip, DEVICE_INFO *remoteDevice, 
                          uint8_t *ip_protocol, uint16_t *length)
{
  IP_HEADER ip_header;
  uint8_t options_length;
  uint8_t header_options_bytes[MAX_IP_OPTIONS_LENGTH];
  uint16_t received_checksum;
  uint16_t calculated_checksum;
  uint8_t calculated_checksum_next_byte_is_low;
  bool dhcpBool = false;
  x_NIC* nic = (x_NIC*)net->GetObject(StackObject::NIC);
  
  
  //----- GET THE IP HEADER -----
  //if (ReadArray((uint8_t*)&ip_header, IP_HEADER_LENGTH) == 0)
  //	goto ip_get_header_dump_packet;							//Error - packet was too small - dump
  
  if (!nic->ReadNextByte(&ip_header.version_header_length))
    return(1);												//Error - packet was too small - dump it
  if (!nic->ReadNextByte(&ip_header.type_of_service))
    return(1);												//Error - packet was too small - dump it
  if (!(nic->ReadArray((uint8_t*)&ip_header.length, 2)))
    goto ip_get_header_dump_packet;							//Error - packet was too small - dump
  if (!(nic->ReadArray((uint8_t*)&ip_header.ident, 2)))
    goto ip_get_header_dump_packet;							//Error - packet was too small - dump
  if (!(nic->ReadArray((uint8_t*)&ip_header.flags, 2)))
    goto ip_get_header_dump_packet;							//Error - packet was too small - dump
  if (!nic->ReadNextByte(&ip_header.time_to_live))
    return(1);												//Error - packet was too small - dump it
  if (!nic->ReadNextByte(&ip_header.protocol))
    return(1);												//Error - packet was too small - dump it
  if (!(nic->ReadArray((uint8_t*)&ip_header.header_checksum, 2)))
    goto ip_get_header_dump_packet;							//Error - packet was too small - dump
  if (!(nic->ReadArray((uint8_t*)&ip_header.source_ip_address, 4)))
    goto ip_get_header_dump_packet;							//Error - packet was too small - dump
  if (!(nic->ReadArray((uint8_t*)&ip_header.destination_ip_address, 4)))
    goto ip_get_header_dump_packet;							//Error - packet was too small - dump
  
  
  //IP Version should be V4
  if ((ip_header.version_header_length & 0xf0) != (IP_VERSION << 4))
    goto ip_get_header_dump_packet;
  
  //Get the options length (if there are any)
  options_length = ((ip_header.version_header_length & 0x0f) << 2) - IP_HEADER_LENGTH;		// x4 as the length field is in Duint16_tS
  
  //Read the options bytes if there are any, checking for maximum we can accept
  if (options_length > MAX_IP_OPTIONS_LENGTH)
    goto ip_get_header_dump_packet;
  
  if (options_length > 0)
  {
    //Get the option bytes
    if (nic->ReadArray((uint8_t*)&header_options_bytes, options_length) == 0)
      goto ip_get_header_dump_packet;
  }
  
  
  //----- CALCULATE THE HEADER CHECKSUM AND CHECK IT MATCHES -----
  calculated_checksum = 0;
  calculated_checksum_next_byte_is_low = 0;
  
  //Store the received checksum and then set to zero in the header ready for checksum calculation
  received_checksum = ip_header.header_checksum;
  ip_header.header_checksum = 0;
  
  //Get checksum of IP header
  //ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (uint8_t*)&ip_header, IP_HEADER_LENGTH);
  ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (uint8_t*)&ip_header.version_header_length, 1);
  ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (uint8_t*)&ip_header.type_of_service, 1);
  ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (uint8_t*)&ip_header.length, 2);
  ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (uint8_t*)&ip_header.ident, 2);
  ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (uint8_t*)&ip_header.flags, 2);
  ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (uint8_t*)&ip_header.time_to_live, 1);
  ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (uint8_t*)&ip_header.protocol, 1);
  ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (uint8_t*)&ip_header.header_checksum, 2);
  ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (uint8_t*)&ip_header.source_ip_address, 4);
  ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (uint8_t*)&ip_header.destination_ip_address, 4);
  
  
  //Add checksum of any options bytes
  if (options_length > 0)
  {
    ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (uint8_t*)&header_options_bytes, options_length);
  }	
  
  //Ensure checksums match
  received_checksum = ~received_checksum;
  received_checksum = IPUtils::swap_word_bytes(received_checksum);
  
  if (received_checksum != calculated_checksum)
    goto ip_get_header_dump_packet;
  
  //SWAP LENGTH, IDENT & CHECKSUM uint16_tS
  ip_header.length = IPUtils::swap_word_bytes(ip_header.length);
  
  dhcpBool = net->ManualIp ?
    false : (((net->DhcpState() == 2) || (net->DhcpState() == 5)) && (net->LocalIpAddress.Val == 0));
    
  //----- CHECK THE DESTINATION IP ADDRESS MATCHES OURS OR IS BROADCAST ON OUR SUBNET -----
  /*if (
//#ifdef STACK_USE_DHCP
      (((net->DhcpState() == 2) || (net->DhcpState() == 5)) && (net->LocalIpAddress.Val == 0)) ||	//Doing DHCP discover and we don't currently have an IP address?
//#endif
        (ip_header.destination_ip_address.Val == net->LocalIpAddress.Val) ||								//Our IP address?
          (
           ((ip_header.destination_ip_address.Val & net->LocalSubnetMask.Val) == (net->LocalIpAddress.Val & net->LocalSubnetMask.Val)) &&
             ((ip_header.destination_ip_address.Val & ~net->LocalSubnetMask.Val) == ~net->LocalSubnetMask.Val)		//Broadcast on our subnet?
               ) ||
            (ip_header.destination_ip_address.Val == 0xffffffff)										//Broadcast?
              )*/
  if ( dhcpBool || (ip_header.destination_ip_address.Val == net->LocalIpAddress.Val) ||					//Our IP address?
          (
           ((ip_header.destination_ip_address.Val & net->LocalSubnetMask.Val) == (net->LocalIpAddress.Val & net->LocalSubnetMask.Val)) &&
             ((ip_header.destination_ip_address.Val & ~net->LocalSubnetMask.Val) == ~net->LocalSubnetMask.Val)		//Broadcast on our subnet?
               ) ||
            (ip_header.destination_ip_address.Val == 0xffffffff)										//Broadcast?
              )
  {
    //DESTINATION IP IS OK
  }
  else
  {
    //DESTINATION IP IS NOT VALID FOR US
    goto ip_get_header_dump_packet;
  }
  
  
  //----- LOAD THE CALLERS REGISTERS WITH THE PACKET INFO -----
  //Store the destination IP address
  destination_ip->Val = ip_header.destination_ip_address.Val;
  
  //Store the remote device IP address
  remoteDevice->ip_address.Val = ip_header.source_ip_address.Val;
  
  //Store the protocol
  *ip_protocol = ip_header.protocol;
  
  //Store the length of the data area of the packet
  *length = (ip_header.length - options_length - IP_HEADER_LENGTH);
  
  return (1);
  
  
  
ip_get_header_dump_packet:
  //----------------------------------
  //----- BAD HEADER DUMP PACKET -----
  //----------------------------------
  nic->DumpPacket();
  return(0);
}

//*************************************
//*************************************
//********** WRITE IP HEADER **********
//*************************************
//*************************************
//The Ethernet header is also written, before the IP header
void IP::ip_write_header(DEVICE_INFO *remoteDevice, uint8_t ip_protocol)
{
  IP_HEADER ip_header;
  x_NIC* nic = (x_NIC*)net->GetObject(StackObject::NIC);
  
  //----- SETUP THE IP HEADER -----
  ip_header.version_header_length = (IP_VERSION << 4) | (IP_HEADER_LENGTH >> 2);		//Header length is in DWORDs
  ip_header.type_of_service = IP_TYPE_OF_SERVICE_STD;
  ip_header.length = 0;									//This will be written before packet is sent (header length + data length)
  ip_header.ident = ++ip_packet_identifier;				//Incrementing packet ID number)
  ip_header.flags = 0;
  ip_header.time_to_live = IP_DEFAULT_TIME_TO_LIVE;
  ip_header.protocol = ip_protocol;
  ip_header.header_checksum = 0;							//The real value will be written before packet is sent as until the length is known we can't do the checksum
  ip_header.source_ip_address.Val = net->LocalIpAddress.Val;
  ip_header.destination_ip_address.Val = remoteDevice->ip_address.Val;
  
  //SWAP LENGTH, IDENT & CHECKSUM WORDS
  //ip_header.length = swap_word_bytes(ip_header.length);		//Not needed
  ip_header.ident = IPUtils::swap_word_bytes(ip_header.ident);
  //ip_header.header_checksum = swap_word_bytes(ip_header.header_checksum);	//Done later
  
  
  //----- START CALCULATION OF IP HEADER CHECKSUM -----
  ip_tx_ip_header_checksum = 0;
  ip_tx_ip_header_checksum_next_byte_low = 0;
  //ip_add_bytes_to_ip_checksum (&ip_tx_ip_header_checksum, &ip_tx_ip_header_checksum_next_byte_low, (uint8_t*)&ip_header, IP_HEADER_LENGTH);
  ip_add_bytes_to_ip_checksum (&ip_tx_ip_header_checksum, &ip_tx_ip_header_checksum_next_byte_low, (uint8_t*)&ip_header.version_header_length, 1);
  ip_add_bytes_to_ip_checksum (&ip_tx_ip_header_checksum, &ip_tx_ip_header_checksum_next_byte_low, (uint8_t*)&ip_header.type_of_service, 1);
  ip_add_bytes_to_ip_checksum (&ip_tx_ip_header_checksum, &ip_tx_ip_header_checksum_next_byte_low, (uint8_t*)&ip_header.length, 2);
  ip_add_bytes_to_ip_checksum (&ip_tx_ip_header_checksum, &ip_tx_ip_header_checksum_next_byte_low, (uint8_t*)&ip_header.ident, 2);
  ip_add_bytes_to_ip_checksum (&ip_tx_ip_header_checksum, &ip_tx_ip_header_checksum_next_byte_low, (uint8_t*)&ip_header.flags, 2);
  ip_add_bytes_to_ip_checksum (&ip_tx_ip_header_checksum, &ip_tx_ip_header_checksum_next_byte_low, (uint8_t*)&ip_header.time_to_live, 1);
  ip_add_bytes_to_ip_checksum (&ip_tx_ip_header_checksum, &ip_tx_ip_header_checksum_next_byte_low, (uint8_t*)&ip_header.protocol, 1);
  ip_add_bytes_to_ip_checksum (&ip_tx_ip_header_checksum, &ip_tx_ip_header_checksum_next_byte_low, (uint8_t*)&ip_header.header_checksum, 2);
  ip_add_bytes_to_ip_checksum (&ip_tx_ip_header_checksum, &ip_tx_ip_header_checksum_next_byte_low, (uint8_t*)&ip_header.source_ip_address, 4);
  ip_add_bytes_to_ip_checksum (&ip_tx_ip_header_checksum, &ip_tx_ip_header_checksum_next_byte_low, (uint8_t*)&ip_header.destination_ip_address, 4);
  
  
  //----- WRITE THE ETHERNET HEADER -----
  nic->WriteEthernetHeader(&remoteDevice->mac_address, ETHERNET_TYPE_IP);
  
  
  //----- WRITE THE IP HEADER -----
  //WriteArray((uint8_t*)&ip_header, IP_HEADER_LENGTH);
  nic->WriteNextByte(ip_header.version_header_length);
  nic->WriteNextByte(ip_header.type_of_service);
  nic->WriteArray((uint8_t*)&ip_header.length, 2);
  nic->WriteArray((uint8_t*)&ip_header.ident, 2);
  nic->WriteArray((uint8_t*)&ip_header.flags, 2);
  nic->WriteNextByte(ip_header.time_to_live);
  nic->WriteNextByte(ip_header.protocol);
  nic->WriteArray((uint8_t*)&ip_header.header_checksum, 2);
  nic->WriteArray((uint8_t*)&ip_header.source_ip_address, 4);
  nic->WriteArray((uint8_t*)&ip_header.destination_ip_address, 4);
  
  
  //----- NOW JUST WRITE THE IP DATA -----
  
}

//**********************************
//**********************************
//********** IP TX PACKET **********
//**********************************
//**********************************
void IP::ip_tx_packet (void)
{
  uint16_t ip_length;
  x_NIC* nic = (x_NIC*)net->GetObject(StackObject::NIC);
  
  //GET THE IP PACKET LENGTH FOR THE IP HEADER
  ip_length = (nic->TxLen() - ETHERNET_HEADER_LENGTH);
  ip_length = IPUtils::swap_word_bytes(ip_length);
  
  //ADD IT TO THE IP HEADER CHECKSUM
  ip_add_bytes_to_ip_checksum (&ip_tx_ip_header_checksum, &ip_tx_ip_header_checksum_next_byte_low, (BYTE*)&ip_length, 2);
  
  //WRITE THE IP LENGTH FIELD
  nic->WriteTxWordAtLocation ((ETHERNET_HEADER_LENGTH + 2), ip_length);
  
  //WRITE THE IP CHECKSUM FIELD
  ip_tx_ip_header_checksum = IPUtils::swap_word_bytes(~ip_tx_ip_header_checksum);
  nic->WriteTxWordAtLocation ((ETHERNET_HEADER_LENGTH + 10), ip_tx_ip_header_checksum);
  
  //TX THE PACKET
  nic->TxPacket();
}

//**********************************************
//**********************************************
//********** ADD uint8_tS TO IP CHECKSUM **********
//**********************************************
//**********************************************
//To do before starting a new checksum:-
//checksum = 0;
//checksum_next_byte_is_low = 0;
void IP::ip_add_bytes_to_ip_checksum (uint16_t *checksum, uint8_t *checksum_next_byte_is_low, uint8_t *next_byte, uint8_t no_of_bytes_to_add)
{
  DWORD dw_temp;
  uint8_t count;
  
  
  dw_temp = (DWORD)*checksum;
  
  for (count = 0; count < no_of_bytes_to_add; count++)
  {
    if (*checksum_next_byte_is_low)
    {
      dw_temp += (DWORD)next_byte[count];
      *checksum_next_byte_is_low = 0;
    }
    else
    {
      dw_temp += ((DWORD)next_byte[count] << 8);
      *checksum_next_byte_is_low = 1;
    }
    
    //Do one's complement (overflow bit is added to checksum)
    if (dw_temp > 0x0000ffff)
    {
      dw_temp &= 0x0000ffff;
      dw_temp++;
    }
  }
  *checksum = (WORD)dw_temp;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------