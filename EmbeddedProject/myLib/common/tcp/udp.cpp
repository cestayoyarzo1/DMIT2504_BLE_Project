/*
FILE: udp.cpp
Created on: 8/28/2017, by Tom Diaz
*/

#include <udp.h>
#include <ip.h>
#include <iputils.h>

//************************************
//************************************
//********** Initialise UDP **********
//************************************
//************************************
//Called from the tcp_ip_Initialise function
void UDP::udp_Initialise(x_Network* net_)
{
  uint8_t count;
  
  net = net_;
  //nic = (x_NIC*)net->GetObject(StackObject::NIC);
  //ip = (IP*)net->GetObject(StackObject::IP);
  
  //SETUP EACH UDP SOCKET AS UNUSED
  for (count = 0; count < UDP_NO_OF_AVAILABLE_SOCKETS; count++)
  {
    udp_socket[count].local_port = UDP_PORT_NULL;
  }
  
  udp_rx_packet_is_waiting_to_be_processed = 0;
}

//*************************************************
//*************************************************
//********** PROCESS RECEIVED UDP PACKET **********
//*************************************************
//*************************************************
//Called from the recevied packet ethernet stack state machine
//Returns 1 if udp packet processing is done, 0 if not yet done (waiting on application process that owns the socket to read the packet and discard it)
uint8_t UDP::udp_process_rx(DEVICE_INFO *sender_device_info, IP_ADDR *destination_ip_address, uint16_t ip_data_area_bytes)
{
  UDP_HEADER udp_header;
  uint8_t udp_socket_number;
  uint8_t b_data;
  uint16_t w_temp;
  uint16_t count;
  IP* ip = (IP*)net->GetObject(StackObject::IP);
  x_NIC* nic = (x_NIC*)net->GetObject(StackObject::NIC);
  
#ifdef UDP_CHECKSUMS_ENABLED
  uint16_t udp_rx_checksum;
  uint16_t udp_rx_checksum_recevied;
  uint8_t udp_rx_checksum_next_byte_low;
#endif
  
  if (udp_rx_packet_is_waiting_to_be_processed)
  {
    //--------------------------------------------------------------------------------------------------------------------------------
    //----- UDP PACKET HAS ALREADY BEEN RECEVEIVED AND WE ARE WAITING ON THE APPLICATION PROCESS USING THIS SOCKET TO PROCESS IT -----
    //--------------------------------------------------------------------------------------------------------------------------------
    if (udp_socket[udp_rx_active_socket].rx_data_bytes_remaining == 0)		//(The application process that owns this socket flags the packet is processed and dumped by writing 0 here)
    {
      //----- THE UDP PACKET HAS BEEN PROCESSED -----
      udp_rx_active_socket = UDP_INVALID_SOCKET;
      udp_rx_packet_is_waiting_to_be_processed = 0;
      return (1);
    }
    return(0);
  }
  
  
  //-----------------------------------------------
  //----- NEW RECEIVED PACKET TO BE PROCESSED -----
  //-----------------------------------------------
  
  //----- GET THE UDP HEADER -----
  //if (ReadArray((uint8_t*)&udp_header, UDP_HEADER_LENGTH) == 0)
  //	goto udp_process_rx_dump_packet;						//Error - packet was too small - dump
  if (nic->ReadArray((uint8_t*)&udp_header.source_port, 2) == 0)
    goto udp_process_rx_dump_packet;						//Error - packet was too small - dump
  if (nic->ReadArray((uint8_t*)&udp_header.destination_port, 2) == 0)
    goto udp_process_rx_dump_packet;						//Error - packet was too small - dump
  if (nic->ReadArray((uint8_t*)&udp_header.length, 2) == 0)
    goto udp_process_rx_dump_packet;						//Error - packet was too small - dump
  if (nic->ReadArray((uint8_t*)&udp_header.checksum, 2) == 0)
    goto udp_process_rx_dump_packet;						//Error - packet was too small - dump
  
  
  
  
  //----- SWAP THE SOURCE PORT, DEST PORT, LENGTH AND CHECKSUM uint16_tS -----
  udp_header.source_port = IPUtils::swap_word_bytes(udp_header.source_port);
  udp_header.destination_port = IPUtils::swap_word_bytes(udp_header.destination_port);
  udp_header.length = IPUtils::swap_word_bytes(udp_header.length);
  udp_header.checksum = IPUtils::swap_word_bytes(udp_header.checksum);
  
  
  //----- LOOK FOR A MATCHING SOCKET -----
  //(Do before checksumming as a network may have lots of broadcast traffic so we don't want to checksum every packet unless its for one of our sockets)
  udp_socket_number = udp_rx_check_for_matches_socket(&udp_header, sender_device_info);
  
  if (udp_socket_number == UDP_INVALID_SOCKET)
  {
    //----- THERE IS NO MATCHING SOCKET FOR THIS UDP PACKET - DUMP IT -----
    goto udp_process_rx_dump_packet;
  }
  //----- MATCHING SOCKET FOUND -----
  
  
  //----- CHECK CHEKSUM -----
#ifdef UDP_CHECKSUMS_ENABLED
  //----- SWAP THE SOURCE PORT, DEST PORT, LENGTH AND CHECKSUM uint16_tS FOR CHECKSUMMING -----
  udp_header.source_port = IPUtils::swap_word_bytes(udp_header.source_port);
  udp_header.destination_port = IPUtils::swap_word_bytes(udp_header.destination_port);
  udp_header.length = IPUtils::swap_word_bytes(udp_header.length);
  udp_header.checksum = IPUtils::swap_word_bytes(udp_header.checksum);
  
  udp_rx_checksum = 0;
  udp_rx_checksum_next_byte_low = 0;
  
  //----- ADD THE PSEUDO HEADER TO THE CHECKSUM -----
  //Psuedo header source address
  ip->ip_add_bytes_to_ip_checksum (&udp_rx_checksum, &udp_rx_checksum_next_byte_low, (uint8_t*)&sender_device_info->ip_address.Val, 4);
  
  //Psuedo header destination address
  ip->ip_add_bytes_to_ip_checksum (&udp_rx_checksum, &udp_rx_checksum_next_byte_low, (uint8_t*)&destination_ip_address->Val, 4);
  
  //Protocol
  w_temp = (uint16_t)IP_PROTOCOL_UDP;
  w_temp = IPUtils::swap_word_bytes(w_temp);
  ip->ip_add_bytes_to_ip_checksum (&udp_rx_checksum, &udp_rx_checksum_next_byte_low, (uint8_t*)&w_temp, 2);
  
  //Length
  ip->ip_add_bytes_to_ip_checksum (&udp_rx_checksum, &udp_rx_checksum_next_byte_low, (uint8_t*)&udp_header.length, 2);
  
  //----- ADD THE UDP HEADER TO THE CHECKSUM -----
  udp_rx_checksum_recevied = udp_header.checksum;
  udp_header.checksum = 0;
  //ip_add_bytes_to_ip_checksum (&udp_rx_checksum, &udp_rx_checksum_next_byte_low, (uint8_t*)&udp_header, UDP_HEADER_LENGTH);
  ip->ip_add_bytes_to_ip_checksum (&udp_rx_checksum, &udp_rx_checksum_next_byte_low, (uint8_t*)&udp_header.source_port, 2);
  ip->ip_add_bytes_to_ip_checksum (&udp_rx_checksum, &udp_rx_checksum_next_byte_low, (uint8_t*)&udp_header.destination_port, 2);
  ip->ip_add_bytes_to_ip_checksum (&udp_rx_checksum, &udp_rx_checksum_next_byte_low, (uint8_t*)&udp_header.length, 2);
  ip->ip_add_bytes_to_ip_checksum (&udp_rx_checksum, &udp_rx_checksum_next_byte_low, (uint8_t*)&udp_header.checksum, 2);
  
  //----- ADD THE UDP DATA AREA TO THE CHECKSUM -----
  for (count = 0; count < (ip_data_area_bytes - UDP_HEADER_LENGTH); count++)
  {
    nic->ReadNextByte(&b_data);
    ip->ip_add_bytes_to_ip_checksum (&udp_rx_checksum, &udp_rx_checksum_next_byte_low, (uint8_t*)&b_data, 1);
  }
  
  udp_rx_checksum = IPUtils::swap_word_bytes(~udp_rx_checksum);
  
  //----- CHECK THAT THE CHECKSUMS MATCH -----
  if (udp_rx_checksum_recevied != udp_rx_checksum)
    goto udp_process_rx_dump_packet;
  
  //----- MOVE NIC POINTER AND DATA uint8_t COUNT BACK TO START OF UDP DATA AREA -----
  nic->MovePointer (ETHERNET_HEADER_LENGTH + IP_HEADER_LENGTH + UDP_HEADER_LENGTH);
  
  //----- SWAP THE SOURCE PORT, DEST PORT, LENGTH AND CHECKSUM uint16_tS BACK AGAIN AFTER CHECKSUMMING -----
  udp_header.source_port = IPUtils::swap_word_bytes(udp_header.source_port);
  udp_header.destination_port = IPUtils::swap_word_bytes(udp_header.destination_port);
  udp_header.length = IPUtils::swap_word_bytes(udp_header.length);
  udp_header.checksum = IPUtils::swap_word_bytes(udp_header.checksum);
#endif
  
  
  //----- FLAG TO THE APPLICATION PROCESS THAT THIS SOCKET HAS DATA READY TO BE READ -----
  //Copy the packet info to the udp socket info
  if (udp_socket[udp_socket_number].remoteDevice.ip_address.Val == 0xffffffff)
  {
    //Socket has broadcast IP address so is waiting for communications from anyone - copy the senders mac and ip to the socket
    udp_socket[udp_socket_number].remoteDevice.mac_address.v[0] = sender_device_info->mac_address.v[0];
    udp_socket[udp_socket_number].remoteDevice.mac_address.v[1] = sender_device_info->mac_address.v[1];
    udp_socket[udp_socket_number].remoteDevice.mac_address.v[2] = sender_device_info->mac_address.v[2];
    udp_socket[udp_socket_number].remoteDevice.mac_address.v[3] = sender_device_info->mac_address.v[3];
    udp_socket[udp_socket_number].remoteDevice.mac_address.v[4] = sender_device_info->mac_address.v[4];
    udp_socket[udp_socket_number].remoteDevice.mac_address.v[5] = sender_device_info->mac_address.v[5];
    udp_socket[udp_socket_number].remoteDevice.ip_address.Val = sender_device_info->ip_address.Val;
  }
  
  //Store the remote port that should be replied to
  udp_socket[udp_socket_number].remote_port = udp_header.source_port;
  
  //Store the destination IP address of the packet (in case the application process wants to know if it was a broadcast packet)
  udp_socket[udp_socket_number].destination_ip_address.Val = destination_ip_address->Val;
  
  //Store number of UDP data bytes
  udp_socket[udp_socket_number].rx_data_bytes_remaining = (udp_header.length - UDP_HEADER_LENGTH);
  
  //Flag socket is pending processing (processing will be done by the application function that is using this socket)
  udp_rx_active_socket = udp_socket_number;
  udp_rx_packet_is_waiting_to_be_processed = 1;
  return (0);		//The remainder of the packet is left in the nic receive buffer ready for the application process using this socket
  //to process it and discard the packet
  
  
  
udp_process_rx_dump_packet:
  //-------------------------------
  //----- DUMP THE UDP PACKET -----
  //-------------------------------
  nic->DumpPacket();
  return(1);
}

//*************************************
//*************************************
//********** OPEN UDP SOCKET **********
//*************************************
//*************************************
//If device_info is null then it is set to broadcast 
//To open a socket to listen to communications from anyone set the device_info to broadcast (ip address = 0xffffffff)
//remote_port is only used when transmitting a packet, it is ignored for rx
//Returns the socket number assigned to the calling process - this should be saved and used to access the socket, or UDP_INVALID_SOCKET
//is returned if no socket is available.
//The calling process must check for rx on the socket reguarly until the socket is closed (if a packet is receveid to this socket
//the stack waits until it has been processed and dumped)

uint8_t UDP::udp_open_socket (DEVICE_INFO *device_info, uint16_t local_port, uint16_t remote_port)
{
  uint8_t socket;
  
  
  //----- LOOK FOR AN AVAILABLE UDP PORT -----
  for (socket = 0; socket < UDP_NO_OF_AVAILABLE_SOCKETS; socket++)
  {
    if (udp_socket[socket].local_port == UDP_PORT_NULL)
    {
      //----- THIS SOCKET IS AVAILABLE - ASSIGN IT TO THE CALLING PROCESS -----
      udp_socket[socket].local_port = local_port;
      udp_socket[socket].remote_port = remote_port;
      if (device_info)
      {
        //Use the supplied device info
        udp_socket[socket].remoteDevice.mac_address.v[0] = device_info->mac_address.v[0];
        udp_socket[socket].remoteDevice.mac_address.v[1] = device_info->mac_address.v[1];
        udp_socket[socket].remoteDevice.mac_address.v[2] = device_info->mac_address.v[2];
        udp_socket[socket].remoteDevice.mac_address.v[3] = device_info->mac_address.v[3];
        udp_socket[socket].remoteDevice.mac_address.v[4] = device_info->mac_address.v[4];
        udp_socket[socket].remoteDevice.mac_address.v[5] = device_info->mac_address.v[5];
        udp_socket[socket].remoteDevice.ip_address.Val = device_info->ip_address.Val;
      }
      else
      {
        //Device info not supplied so set to broadcast (tx to all, rx from anyone)
        udp_socket[socket].remoteDevice.mac_address.v[0] = 0xff;
        udp_socket[socket].remoteDevice.mac_address.v[1] = 0xff;
        udp_socket[socket].remoteDevice.mac_address.v[2] = 0xff;
        udp_socket[socket].remoteDevice.mac_address.v[3] = 0xff;
        udp_socket[socket].remoteDevice.mac_address.v[4] = 0xff;
        udp_socket[socket].remoteDevice.mac_address.v[5] = 0xff;
        udp_socket[socket].remoteDevice.ip_address.Val = 0xffffffff;
      }
      return (socket);
    }
  }
  
  //----- THERE IS NO UNUSED UDP SOCKET AVAILABLE -----
  //(Increase the UDP_NO_OF_AVAILABLE_SOCKETS define in the header file)
  return (UDP_INVALID_SOCKET);
}

//**************************************
//**************************************
//********** CLOSE UDP SOCKET **********
//**************************************
//**************************************
//Release this socket ready for use by another application process
void UDP::udp_close_socket(uint8_t *socket)
{
  //EXIT IF SOCKET IS ALREADY CLOSED OR IS INVALID
  if ((*socket == UDP_INVALID_SOCKET) || (*socket >= UDP_NO_OF_AVAILABLE_SOCKETS))
    return;
  
  //ENSURE THE SOCKET HAS NOT GOT A PACKET WAITING
  if (udp_check_socket_for_rx(*socket))
    udp_dump_rx_packet();
  
  //CLOSE THE SOCKET
  udp_socket[*socket].local_port = UDP_PORT_NULL;
  
  *socket = UDP_INVALID_SOCKET;
}

//*********************************************
//*********************************************
//********** UDP CHECK FOR SOCKET RX **********
//*********************************************
//*********************************************
//Every application process that has an open UDP socket must reguarly call this funciton to see if the 
//socket has recevied a packet and if so process and dump the packet (the ethernet stack is halted until the
//received packet is dumped)
uint8_t UDP::udp_check_socket_for_rx (uint8_t socket)
{
  
  if (udp_rx_packet_is_waiting_to_be_processed)
  {
    if (udp_rx_active_socket == socket)
      return(1);
  }
  return (0);
}

//****************************************
//****************************************
//********** UDP DUMP RX PACKET **********
//****************************************
//****************************************
void UDP::udp_dump_rx_packet (void)
{
  x_NIC* nic = (x_NIC*)net->GetObject(StackObject::NIC);
  
  nic->DumpPacket();
  
  udp_socket[udp_rx_active_socket].rx_data_bytes_remaining = 0;		//(Doing this flags to the udp_process_rx function that rx is complete and the stack can move on)
}

//******************************************************
//******************************************************
//********** UDP GET NEXT uint8_t FROM RX PACKET **********
//******************************************************
//******************************************************
//Returns 0 if no more data available, 1 if byte retreived
uint8_t UDP::udp_read_next_rx_byte (uint8_t *data)
{
  x_NIC* nic = (x_NIC*)net->GetObject(StackObject::NIC);
  
  //Check that there are bytes still to be retreived
  if (udp_socket[udp_rx_active_socket].rx_data_bytes_remaining == 0)
    return (0);
  
  //Decrement the byte count
  udp_socket[udp_rx_active_socket].rx_data_bytes_remaining--;
  
  //Get the next byte from the nic
  if (nic->ReadNextByte(data))
    return (1);
  else
    return(0);				//Error - nic routine says no more bytes available - the UDP header contained an invalid number of bytes
}

//***************************************************
//***************************************************
//********** UDP READ ARRAY FROM RX PACKET **********
//***************************************************
//***************************************************
//Returns 0 if no more data available, 1 if byte retreived
uint8_t UDP::udp_read_rx_array (uint8_t *array_buffer, uint16_t array_length)
{
  x_NIC* nic = (x_NIC*)net->GetObject(StackObject::NIC);
  
  for ( ;array_length > 0; array_length--)
  {
    //Check that there are bytes still to be retreived
    if (udp_socket[udp_rx_active_socket].rx_data_bytes_remaining == 0)
      return (0);
    
    //Decrement the byte count
    udp_socket[udp_rx_active_socket].rx_data_bytes_remaining--;
    
    //Get the next byte from the nic
    if (nic->ReadNextByte(array_buffer++) == 0)
      return(0);				//Error - nic routine says no more bytes available - the UDP header contained an invalid number of bytes
    
  }
  return(1);
}

//**********************************
//**********************************
//********** UDP SETUP TX **********
//**********************************
//**********************************
//Returns 1 if tx setup and ready to send data to nic, 0 if unable to setup tx at this time (no tx buffer available from the nic)
uint8_t UDP::udp_setup_tx (uint8_t socket)
{
  UDP_HEADER udp_header;
  uint16_t w_temp;
  IP* ip = (IP*)net->GetObject(StackObject::IP);
  x_NIC* nic = (x_NIC*)net->GetObject(StackObject::NIC);
  
  //----- EXIT IF NIC IS NOT CURRENTLY ABLE TO SEND A NEW PACKET -----
  if (!nic->OkToDoTx())
    return(0);
  
  //----- SETUP THE NIC READY TO TX A NEW PACKET -----
  if (!nic->SetupTx())
    return(0);
  
  
  //----- SETUP THE UDP HEADER -----
  udp_header.source_port = udp_socket[socket].local_port;
  udp_header.destination_port = udp_socket[socket].remote_port;
  udp_header.length = 0;					//This will be written before packet is sent
  udp_header.checksum = 0;				//This will be written before packet is sent if checksumming is enabled
  
  //SWAP THE SOURCE PORT, DEST PORT, LENGTH AND CHECKSUM uint16_tS
  udp_header.source_port = IPUtils::swap_word_bytes(udp_header.source_port);
  udp_header.destination_port = IPUtils::swap_word_bytes(udp_header.destination_port);
  //udp_header.length = swap_word_bytes(udp_header.length);		//No need
  //udp_header.checksum = swap_word_bytes(udp_header.checksum);	//No need
  
  
  //----- START CALCULATION OF UDP CHECKSUM -----
#ifdef UDP_CHECKSUMS_ENABLED
  udp_tx_checksum = 0;
  udp_tx_checksum_next_byte_low = 0;
  
  //----- ADD THE PSEUDO HEADER TO THE CHECKSUM -----
  //Psuedo header source address
  ip->ip_add_bytes_to_ip_checksum (&udp_tx_checksum, &udp_tx_checksum_next_byte_low, (uint8_t*)&net->LocalIpAddress.Val, 4); //???
  
  //Psuedo header destination address
  ip->ip_add_bytes_to_ip_checksum (&udp_tx_checksum, &udp_tx_checksum_next_byte_low, (uint8_t*)&udp_socket[socket].remoteDevice.ip_address.Val, 4);
  
  //Protocol
  w_temp = (uint16_t)IP_PROTOCOL_UDP;
  w_temp = IPUtils::swap_word_bytes(w_temp);
  ip->ip_add_bytes_to_ip_checksum (&udp_tx_checksum, &udp_tx_checksum_next_byte_low, (uint8_t*)&w_temp, 2);
  
  //Length
  // Unknown - will be added before packet is transmitted
  
  //----- ADD THE UDP HEADER TO THE CHECKSUM -----
  //ip_add_bytes_to_ip_checksum (&udp_tx_checksum, &udp_tx_checksum_next_byte_low, (uint8_t*)&udp_header, UDP_HEADER_LENGTH);
  ip->ip_add_bytes_to_ip_checksum (&udp_tx_checksum, &udp_tx_checksum_next_byte_low, (uint8_t*)&udp_header.source_port, 2);
  ip->ip_add_bytes_to_ip_checksum (&udp_tx_checksum, &udp_tx_checksum_next_byte_low, (uint8_t*)&udp_header.destination_port, 2);
  ip->ip_add_bytes_to_ip_checksum (&udp_tx_checksum, &udp_tx_checksum_next_byte_low, (uint8_t*)&udp_header.length, 2);
  ip->ip_add_bytes_to_ip_checksum (&udp_tx_checksum, &udp_tx_checksum_next_byte_low, (uint8_t*)&udp_header.checksum, 2);
#endif
  
  //----- WRITE THE ETHERNET & IP HEADER -----
  ip->ip_write_header(&udp_socket[socket].remoteDevice, IP_PROTOCOL_UDP);
  
  //----- WRITE THE UDP HEADER -----
  //WriteArray((uint8_t*)&udp_header, UDP_HEADER_LENGTH);
  nic->WriteArray((uint8_t*)&udp_header.source_port, 2);
  nic->WriteArray((uint8_t*)&udp_header.destination_port, 2);
  nic->WriteArray((uint8_t*)&udp_header.length, 2);
  nic->WriteArray((uint8_t*)&udp_header.checksum, 2);
  
  //----- NOW JUST WRITE THE UDP DATA ----
  
  return(1);
}

//*****************************************
//*****************************************
//********** UDP WRITE NEXT uint8_t **********
//*****************************************
//*****************************************
void UDP::udp_write_next_byte (uint8_t data)
{
  IP* ip = (IP*)net->GetObject(StackObject::IP);
  x_NIC* nic = (x_NIC*)net->GetObject(StackObject::NIC);
  
  nic->WriteNextByte(data);
  
#ifdef UDP_CHECKSUMS_ENABLED
  //ADD TO CHECKSUM
  ip->ip_add_bytes_to_ip_checksum (&udp_tx_checksum, &udp_tx_checksum_next_byte_low, (uint8_t*)&data, 1);
#endif
}

//*************************************
//*************************************
//********** UDP WRITE ARRAY **********
//*************************************
//*************************************
void UDP::udp_write_array (uint8_t *array_buffer, uint16_t array_length)
{
  IP* ip = (IP*)net->GetObject(StackObject::IP);
  x_NIC* nic = (x_NIC*)net->GetObject(StackObject::NIC);
  
  nic->WriteArray (array_buffer, array_length);
  
#ifdef UDP_CHECKSUMS_ENABLED
  //ADD TO CHECKSUM
  ip->ip_add_bytes_to_ip_checksum (&udp_tx_checksum, &udp_tx_checksum_next_byte_low, array_buffer, array_length);
#endif
}

//***********************************
//***********************************
//********** UDP TX PACKET **********
//***********************************
//***********************************
void UDP::udp_tx_packet (void)
{
  uint16_t udp_length;
  IP* ip = (IP*)net->GetObject(StackObject::IP);
  x_NIC* nic = (x_NIC*)net->GetObject(StackObject::NIC);
  
  //----- GET THE UDP PACKET LENGTH FOR THE UDP HEADER -----
  udp_length = (nic->TxLen() - ETHERNET_HEADER_LENGTH - IP_HEADER_LENGTH);
  udp_length = IPUtils::swap_word_bytes(udp_length);
  
  //----- WRITE THE UDP LENGTH FIELD -----
  nic->WriteTxWordAtLocation ((ETHERNET_HEADER_LENGTH + IP_HEADER_LENGTH + 4), udp_length);
  
#ifdef UDP_CHECKSUMS_ENABLED
  //----- ADD THE LENGTH TO THE UDP CHECKSUM -----
  //(In place of where it should have been for the pseudo header)
  udp_tx_checksum_next_byte_low = 0;
  ip->ip_add_bytes_to_ip_checksum (&udp_tx_checksum, &udp_tx_checksum_next_byte_low, (uint8_t*)&udp_length, 2);	//UDP Header
  ip->ip_add_bytes_to_ip_checksum (&udp_tx_checksum, &udp_tx_checksum_next_byte_low, (uint8_t*)&udp_length, 2);	//Pseudo header
  
  //----- WRITE THE UDP CHECKSUM FIELD -----
  udp_tx_checksum = IPUtils::swap_word_bytes(~udp_tx_checksum);
  nic->WriteTxWordAtLocation ((ETHERNET_HEADER_LENGTH + IP_HEADER_LENGTH + 6), udp_tx_checksum);
#endif
  
  //----- TX THE PACKET -----
  ip->ip_tx_packet();
}

//*******************************************
//*******************************************
//*********** FIND MATCHING SOCKET **********
//*******************************************
//*******************************************
uint8_t UDP::udp_rx_check_for_matches_socket (UDP_HEADER *rx_udp_header, DEVICE_INFO *rx_device_info)
{
  uint8_t udp_socket_number;
  
  //----- CHECK EACH UDP SOCKET LOOKING FOR A MATCH WITH THIS PACKET -----
  for (udp_socket_number = 0; udp_socket_number < UDP_NO_OF_AVAILABLE_SOCKETS; udp_socket_number++ )
  {
    if (udp_socket[udp_socket_number].local_port == rx_udp_header->destination_port)
    {
      //----- THIS PACKET HAS BEEN SENT TO THIS SOCKETS LOCAL PORT -----
      //Check this socket is set to broadcast ip address (to receive from anyone) or this remote devices IP address
      if (
          (udp_socket[udp_socket_number].remoteDevice.ip_address.Val == rx_device_info->ip_address.Val) ||
            ((udp_socket[udp_socket_number].remoteDevice.ip_address.Val == net->LocalIpAddress.Val) | ~net->LocalSubnetMask.Val) || //???
              (udp_socket[udp_socket_number].remoteDevice.ip_address.Val == 0xffffffff)
                )
      {
        //----- THE SENDER IP MATCHES THE SOCKET REMOTE DEVICE IP OR THE SOCKET REMOTE DEVICE IP IS SET TO BROADCAST (RX FROM ANYONE) -----
        //The packet is for this socket
        return(udp_socket_number);
      }
    }
  }
  return(UDP_INVALID_SOCKET);	
}