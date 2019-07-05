/*
FILE: dhcp.cpp
Created on: 8/28/2017, by Tom Diaz
*/

#include <dhcp.h>
#include <udp.h>
#include <x_nic.h>


//extern IP_ADDR our_gateway_ip_address; // eth-main, get ridof this after encapsulating eth-main into stack class
//extern BYTE nic_linked_and_ip_address_valid; // same

//*************************************
//*************************************
//********** DHCP Initialise **********
//*************************************
//*************************************
//Called from the tcp_ip_Initialise function
void DHCP::Init(x_Network* net_)
{
  net = net_;
  
  sm_dhcp = DHCP_STATE::DHCP_INIT;
  net->NicLinkedAndIpValid = 0;
  
  //If configured to use DHCP then Initialise the IP settings
  if (!net->ManualIp)
  {
    net->LocalIpAddress.Val = 0;
    net->LocalSubnetMask.Val = 0;
    net->GatewayIpAddress.Val = 0;
  }
}

//**********************************
//**********************************
//********** PROCESS DHCP **********
//**********************************
//**********************************
void DHCP::FSM (void)
{
  DEVICE_INFO remoteDevice;
  x_NIC* nic = (x_NIC*)net->GetObject(StackObject::NIC);
  UDP* udp = (UDP*)net->GetObject(StackObject::UDP);
  //----------------------------------------------
  //----- CHECK FOR USING MANUAL IP SETTINGS -----
  //----------------------------------------------
  if (net->ManualIp)
  {
    //----- WE ARE CURRENTLY CONFIGURED TO USE MANUAL SETTINGS, NOT DHCP -----
    sm_dhcp = DHCP_STATE::DHCP_BOUND;
    eth_dhcp_1sec_renewal_timer = 1000;				//If our application changes our state to use DHCP cause a request to happen quickly
    eth_dhcp_1sec_lease_timer = 2;
  }
  
  
  //------------------------------------
  //----- CHECK FOR NIC NOT LINKED -----
  //------------------------------------
  if (nic->Linked() == 0)
  {
    //----- WE HAVE NO ETHERNET LINK - PUT DHCP INTO Initialise STATE WAITING FOR NIC TO BE LINKED -----
    sm_dhcp = DHCP_STATE::DHCP_INIT;
  }
  
  
  //------------------------
  //----- PROCESS DHCP -----
  //------------------------
  switch (sm_dhcp)
  {
  case DHCP_STATE::DHCP_INIT:
    //----------------------
    //----- Initialise -----
    //----------------------
    
    net->NicLinkedAndIpValid = 0;				//Flag that we do not have valid IP settings
    
    //ENSURE OUR SOCKET IS NOT OPEN
    if (dhcp_socket != UDP_INVALID_SOCKET)
      udp->udp_close_socket(&dhcp_socket);
    
    if (!net->ManualIp)
    {
      //----- WE ARE CURRENTLY CONFIGURED TO USE DHCP (NOT MANUAL SETTINGS) -----
      net->LocalIpAddress.Val = 0;
      net->LocalSubnetMask.Val = 0;
      net->GatewayIpAddress.Val = 0;
      eth_dhcp_1sec_lease_timer = 100;			//Set a value so that the discover stage doesn't immediatly return to here
      
      //GENERATE A RANDOM DELAY OF UP TO 10 SECONDS BEFORE CONTACTING THE DHCP SERVER
      //(Good practice to avoid lots of devices hitting a server at once when connectivety returns)
#ifdef DHCP_DO_POWERUP_RANDOM_DELAY
      eth_dhcp_1ms_timer = (((uint16_t)our_mac_address.v[0]) << 8) + (uint16_t)our_mac_address.v[1];		//Use our mac address as the random variable
      while (eth_dhcp_1ms_timer > 10000)			//Limit to 10 seconds
        eth_dhcp_1ms_timer -= 10000;
      if (eth_dhcp_1ms_timer < 1000)				//Force to at least 1 second
        eth_dhcp_1ms_timer += 1000;
#else
      eth_dhcp_1ms_timer = 100;
#endif
      
      //Do discovery next
      sm_dhcp = DHCP_STATE::DHCP_DISCOVER;
    }
    break;
    
    
    
  case DHCP_STATE::DHCP_DISCOVER:
    //-------------------------
    //----- DHCP DISCOVER -----
    //-------------------------
    //Send a DHCP discover request
    
    //Wait to do action (if coming from another state that set a delay to occur first)
    if (eth_dhcp_1ms_timer)
    {
      eth_dhcp_1ms_timer--;
      break;
    }
    
    if (eth_dhcp_1sec_lease_timer == 0)
    {
      //----- OUR LEASE HAS EXPIRED - IP SETTINGS NO LONGER VALID -----
      sm_dhcp = DHCP_STATE::DHCP_INIT;
      break;
    }
    
    
    //TRY AND OPEN A UDP SOCKET
    remoteDevice.ip_address.Val = 0xffffffff;			//Set to broadcast
    remoteDevice.mac_address.v[0] = 0xff;
    remoteDevice.mac_address.v[1] = 0xff;
    remoteDevice.mac_address.v[2] = 0xff;
    remoteDevice.mac_address.v[3] = 0xff;
    remoteDevice.mac_address.v[4] = 0xff;
    remoteDevice.mac_address.v[5] = 0xff;
    dhcp_socket = udp->udp_open_socket(&remoteDevice, DHCPCLIENT_PORT, DHCPSERVER_PORT);
    
    if (dhcp_socket != UDP_INVALID_SOCKET)
    {
      //WE HAVE OBTAINED A UDP SOCKET - SETUP FOR TX
      if (udp->udp_setup_tx(dhcp_socket) == 0)
      {
        //TX IS NOT CURRENTLY AVAILABLE
        //Try again next time
        udp->udp_close_socket(&dhcp_socket);
        break;
      }
      //----- SEND DISCOVER PACKET -----
      txPacket(DHCP_MESSAGE_DISCOVER);
      
      eth_dhcp_1ms_timer = DHCP_DISCOVER_TIMEOUT;
      sm_dhcp = DHCP_STATE::DHCP_WAIT_FOR_OFFER_RESPONSE;
      break;
    }
    //CANNOT OPEN A UDP SOCKET
    //There is no free UDP socket currently - Try again next time
    break;
    
    
    
  case DHCP_STATE::DHCP_WAIT_FOR_OFFER_RESPONSE:
    //-----------------------------------
    //----- WAIT FOR OFFER RESPONSE -----
    //-----------------------------------
    if (udp->udp_check_socket_for_rx(dhcp_socket))
    {
      //----- RESPONSE RECEVIED -----
      
      //Process the response packet
      if (rxPacket())
      {
        //PACKET PROCESSED AND OUR STATE MACHINE HAS BEEN UPDATED - DO NOTHING
        udp->udp_close_socket(&dhcp_socket);
        break;
      }
      else
      {
        //PACKET WAS NOT VALID FOR SOME REASON
        //Try again after a short delay
        udp->udp_close_socket(&dhcp_socket);
        eth_dhcp_1ms_timer = 1000;
        sm_dhcp = DHCP_STATE::DHCP_DISCOVER;
        break;				
      }
      
    }
    else if (eth_dhcp_1ms_timer == 0)
    {
      //------ TIMED OUT - NO RESPONSE FROM DHCP SERVER -----
      //Setup to try again after a delay
      eth_dhcp_1ms_timer = (((uint16_t)nic->MAC()->v[0]) << 8) + (uint16_t)nic->MAC()->v[1];		//Set timer to a random delay based on our mac address
      while (eth_dhcp_1ms_timer > 8000)
        eth_dhcp_1ms_timer -= 8000;
      if (eth_dhcp_1ms_timer < 4000)
        eth_dhcp_1ms_timer += 4000;
      
      udp->udp_close_socket(&dhcp_socket);
      sm_dhcp = DHCP_STATE::DHCP_DISCOVER;
    }
    break;
    
    
    
  case DHCP_STATE::DHCP_REQUEST:		//(These 2 states do the same thing)
  case DHCP_STATE::DHCP_RENEWING:
    //-----------------------------------------
    //----- REQUEST OR RENEW DHCP ADDRESS -----
    //-----------------------------------------
    //Wait to do action
    if (eth_dhcp_1ms_timer)
      break;
    
    //TRY AND OPEN A UDP SOCKET
    //Address must be broadcast for a request (after discover) as a DHCP server will not reply if addressed by its IP address until it has ACK'ed the first request
    //When renewing the individual IP address can be used but there isn't any reason to so we always send broadcast
    remoteDevice.ip_address.Val = 0xffffffff;			//Set to broadcast
    remoteDevice.mac_address.v[0] = 0xff;
    remoteDevice.mac_address.v[1] = 0xff;
    remoteDevice.mac_address.v[2] = 0xff;
    remoteDevice.mac_address.v[3] = 0xff;
    remoteDevice.mac_address.v[4] = 0xff;
    remoteDevice.mac_address.v[5] = 0xff;
    
    dhcp_socket = udp->udp_open_socket(&remoteDevice, DHCPCLIENT_PORT, DHCPSERVER_PORT);
    
    if (dhcp_socket != UDP_INVALID_SOCKET)
    {
      //WE HAVE OBTAINED A UDP SOCKET - SETUP FOR TX
      if (udp->udp_setup_tx(dhcp_socket) == 0)
      {
        //TX IS NOT CURRENTLY AVAILABLE
        //Try again next time
        udp->udp_close_socket(&dhcp_socket);
        break;
      }
      //----- SEND REQUEST PACKET -----
      txPacket(DHCP_MESSAGE_REQUEST);
      
      eth_dhcp_1ms_timer = DHCP_REQUEST_TIMEOUT;
      sm_dhcp = DHCP_STATE::DHCP_WAIT_FOR_REQUEST_RESPONSE;
      break;
    }
    //CANNOT OPEN A UDP SOCKET
    //There is no free UDP socket currently - Try again next time
    break;
    
    
    
  case DHCP_STATE::DHCP_WAIT_FOR_REQUEST_RESPONSE:
    //-------------------------------------
    //----- WAIT FOR REQUEST RESPONSE -----
    //-------------------------------------
    
    if (udp->udp_check_socket_for_rx(dhcp_socket))
    {
      //----- RESPONSE RECEVIED -----
      
      //Process the response packet
      if (rxPacket())
      {
        //PACKET PROCESSED AND OUR STATE MACHINE HAS BEEN UPDATED - DO NOTHING
        udp->udp_close_socket(&dhcp_socket);
        break;
      }
      else
      {
        //PACKET WAS NOT VALID FOR SOME REASON
        //Try again after a short delay
        udp->udp_close_socket(&dhcp_socket);
        eth_dhcp_1ms_timer = 1000;
        sm_dhcp = DHCP_STATE::DHCP_DISCOVER;
        break;				
      }
      
    }
    else if (eth_dhcp_1ms_timer == 0)
    {
      //------ TIMED OUT - NO RESPONSE FROM DHCP SERVER -----
      //Setup to try again after a delay
      eth_dhcp_1ms_timer = (((uint16_t)nic->MAC()->v[0]) << 8) + (uint16_t)nic->MAC()->v[1];		//Set timer for a retry if this doesn't work
      while (eth_dhcp_1ms_timer > 8000)
        eth_dhcp_1ms_timer -= 8000;
      if (eth_dhcp_1ms_timer < 4000)
        eth_dhcp_1ms_timer += 4000;
      
      udp->udp_close_socket(&dhcp_socket);
      sm_dhcp = DHCP_STATE::DHCP_DISCOVER;
    }
    break;
    
    
    
    
  case DHCP_STATE::DHCP_BOUND:
    //----------------------
    //----- DHCP BOUND -----
    //----------------------
    //We have our IP settings - do we need to renew with the DHCP server?
    
    net->NicLinkedAndIpValid = 1;				//Flag that we have valid IP settings
    
    if (eth_dhcp_1sec_renewal_timer == 0)
    {
      //----- WE NEED TO RENEW OUR ADDRES -----
      sm_dhcp = DHCP_STATE::DHCP_RENEWING;			
      break;
    }
    
    if (eth_dhcp_1sec_lease_timer == 0)
    {
      //----- OUR LEASE HAS EXPIRED - IP SETTINGS NO LONGER VALID -----
      sm_dhcp = DHCP_STATE::DHCP_INIT;
      break;
    }
    
    break;
    
    //-------------------------
    //----- END OF SWITCH -----
    //-------------------------
  }	//switch (sm_dhcp)
  
  
  
}

//*****************************************
//*****************************************
//********** RECEIVE DHCP PACKET **********
//*****************************************
//*****************************************
//Returns 1 if a DHCP packet was sucessfully processed, 0 if not
//If return value is 1 then the DHCP state machine has been updated
uint8_t DHCP::rxPacket (void)
{
  uint8_t data;
  uint8_t w_temp;
  IP_ADDR received_ip;
  uint8_t option_code;
  uint8_t option_len;
  uint32_t option_data;
  uint8_t option_message_type = 0xff;
  uint32_t option_lease_time;
  IP_ADDR option_subnet_mask;
  IP_ADDR option_gateway;
  x_NIC* nic = (x_NIC*)net->GetObject(StackObject::NIC);
  UDP* udp = (UDP*)net->GetObject(StackObject::UDP);
  //IP_ADDR option_server_ip_addr;
  
  received_ip.Val = 0;
  option_subnet_mask.Val = 0;
  option_gateway.Val = 0;
  serverIP.Val = 0;
  
  //GET OPCODE
  udp->udp_read_next_rx_byte(&data);
  if (data != 2)
    goto rxPacket_dump;
  
  //GET ADDRESS TYPE
  udp->udp_read_next_rx_byte(&data);
  
  //GET ADDRESS LENGTH
  udp->udp_read_next_rx_byte(&data);
  if (data != 6)
    goto rxPacket_dump;
  
  //GET HOPS [1]
  //GET TRANSACTION ID [4]
  //GET ELAPSED TIME [2]
  //GET FLAGS [2]
  //GET CLIENT IP [4]
  for(w_temp = 0; w_temp < 13; w_temp++)
    udp->udp_read_next_rx_byte(&data);
  
  //GET YOUR (CLIENT) IP ADDRESS (The IP address being assigned to us)
  udp->udp_read_next_rx_byte(&received_ip.v[0]);
  udp->udp_read_next_rx_byte(&received_ip.v[1]);
  udp->udp_read_next_rx_byte(&received_ip.v[2]);
  udp->udp_read_next_rx_byte(&received_ip.v[3]);
  
  //GET SERVER IP [4]
  //GET RELAY AGENT IP [4]
  for(w_temp = 0; w_temp < 8; w_temp++)
    udp->udp_read_next_rx_byte(&data);
  
  //GET MAC ADDRESS
  udp->udp_read_next_rx_byte(&data);
  if (data != nic->MAC()->v[0])
    goto rxPacket_dump;
  
  udp->udp_read_next_rx_byte(&data);
  if (data != nic->MAC()->v[1])
    goto rxPacket_dump;
  
  udp->udp_read_next_rx_byte(&data);
  if (data != nic->MAC()->v[2])
    goto rxPacket_dump;
  
  udp->udp_read_next_rx_byte(&data);
  if (data != nic->MAC()->v[3])
    goto rxPacket_dump;
  
  udp->udp_read_next_rx_byte(&data);
  if (data != nic->MAC()->v[4])
    goto rxPacket_dump;
  
  udp->udp_read_next_rx_byte(&data);
  if (data != nic->MAC()->v[5])
    goto rxPacket_dump;
  
  //UNUSED uint8_tS
  for(w_temp = 0; w_temp < (10 + 64 + 128); w_temp++)
    udp->udp_read_next_rx_byte(&data);
  
  //GET MAGIC COOKIE
  udp->udp_read_next_rx_byte(&data);
  if (data != 0x63)
    goto rxPacket_dump;
  
  udp->udp_read_next_rx_byte(&data);
  if (data != 0x82)
    goto rxPacket_dump;
  
  udp->udp_read_next_rx_byte(&data);
  if (data != 0x53)
    goto rxPacket_dump;
  
  udp->udp_read_next_rx_byte(&data);
  if (data != 0x63)
    goto rxPacket_dump;
  
  
  //----- BELOW HERE ARE THE DHCP OPTIONS -----
  
  //GET EACH OPTION
  while (
         udp->udp_read_next_rx_byte(&option_code) && 
           (option_code != 0xff) &&
             udp->udp_read_next_rx_byte(&option_len)
               )
  {
    //----- GET THE OPTION DATA -----
    option_data = 0;
    for (w_temp = 0; w_temp < option_len; w_temp++)
    {
      option_data <<= 8;
      udp->udp_read_next_rx_byte(&data);
      option_data += (uint32_t)data;
    }
    
    //----- PROCESS THE OPTION -----
    
    
    if ((option_code == 53) && (option_len == 1))
    {
      //----- MESSAGE TYPE -----
      option_message_type = (uint8_t)option_data;
    }	
    else if ((option_code == 51) && (option_len == 4))
    {
      //----- LEASE TIME -----
      option_lease_time = option_data;
    }	
    else if ((option_code == 1) && (option_len == 4) && (option_message_type == DHCP_MESSAGE_ACK))
    {
      //----- SUBNET MASK -----
      option_subnet_mask.v[0] = (uint8_t)(option_data >> 24);
      option_subnet_mask.v[1] = (uint8_t)((option_data & 0x00ff0000) >> 16);
      option_subnet_mask.v[2] = (uint8_t)((option_data & 0x0000ff00) >> 8);
      option_subnet_mask.v[3] = (uint8_t)(option_data & 0x000000ff);
    }
    else if ((option_code == 3) && (option_len == 4) && (option_message_type == DHCP_MESSAGE_ACK))
    {
      //----- GATEWAY -----
      option_gateway.v[0] = (uint8_t)(option_data >> 24);
      option_gateway.v[1] = (uint8_t)((option_data & 0x00ff0000) >> 16);
      option_gateway.v[2] = (uint8_t)((option_data & 0x0000ff00) >> 8);
      option_gateway.v[3] = (uint8_t)(option_data & 0x000000ff);
    }
    else if ((option_code == 54) && (option_len == 4))		//DHCP_MESSAGE_OFFER or DHCP_MESSAGE_ACK
    {
      //----- DHCP SERVER IP ADDRESS ----- (Enable this if you want it for some reason - we get the server IP address from the packet header)
      serverIP.v[0] = (uint8_t)(option_data >> 24);
      serverIP.v[1] = (uint8_t)((option_data & 0x00ff0000) >> 16);
      serverIP.v[2] = (uint8_t)((option_data & 0x0000ff00) >> 8);
      serverIP.v[3] = (uint8_t)(option_data & 0x000000ff);
    }
    
    
    //<<<< ADD ANY OTHER OPTIONS REQURIED HERE
    
    
  }
  
  
  //----- STORE THE DHCP SERVER IP ADDRESS -----
  if (serverIP.Val == 0)
    serverIP.Val = udp->udp_socket[dhcp_socket].remoteDevice.ip_address.Val;
  
  
  //----- PROCESS PACKET -----
  if (option_message_type == DHCP_MESSAGE_NAK)
  {
    //----- NAK RECEIVED -----
    //This should not happen - restart DHCP in case the DHCP server want's to give us a new address
    
    eth_dhcp_1ms_timer = 0;
    sm_dhcp = DHCP_STATE::DHCP_INIT;
  }
  else if ((sm_dhcp == DHCP_STATE::DHCP_WAIT_FOR_OFFER_RESPONSE) && (option_message_type == DHCP_MESSAGE_OFFER))
  {
    //----- WE ARE WAITING FOR AN OFFER AND IT HAS BEEN RECEIVED -----
    //Store the DHCP server mac address
    serverMAC.v[0] = udp->udp_socket[dhcp_socket].remoteDevice.mac_address.v[0];
    serverMAC.v[1] = udp->udp_socket[dhcp_socket].remoteDevice.mac_address.v[1];
    serverMAC.v[2] = udp->udp_socket[dhcp_socket].remoteDevice.mac_address.v[2];
    serverMAC.v[3] = udp->udp_socket[dhcp_socket].remoteDevice.mac_address.v[3];
    serverMAC.v[4] = udp->udp_socket[dhcp_socket].remoteDevice.mac_address.v[4];
    serverMAC.v[5] = udp->udp_socket[dhcp_socket].remoteDevice.mac_address.v[5];
    
    //Store the offered IP address
    offerIP.Val = received_ip.Val;
    
    //Set DHCP state machine to new state
    eth_dhcp_1ms_timer = 0;
    sm_dhcp = DHCP_STATE::DHCP_REQUEST;
  }
  else if (
           (sm_dhcp == DHCP_STATE::DHCP_WAIT_FOR_REQUEST_RESPONSE) &&
             (option_message_type == DHCP_MESSAGE_ACK)
               )
  {
    //----- WE HAVE SENT A REQUEST OR RENEWAL REQUEST AND WE HAVE RECEIVED THE RESPONSE -----
    
    //Get the lease time	
    if (option_lease_time > DHCP_LEASE_MAX_SECS)
      eth_dhcp_1sec_lease_timer = DHCP_LEASE_MAX_SECS;
    else if (option_lease_time < DHCP_LEASE_MIN_SECS)
      eth_dhcp_1sec_lease_timer = DHCP_LEASE_MIN_SECS;
    else
      eth_dhcp_1sec_lease_timer = option_lease_time;
    
    eth_dhcp_1sec_renewal_timer = (eth_dhcp_1sec_lease_timer >> 1);			//We will ask to renew the lease half way through our lease time
    
    //Store our new IP address
    net->LocalIpAddress.Val = received_ip.Val;
    
    //Store our new subnet mask
    if (option_subnet_mask.Val)
      net->LocalSubnetMask.Val = option_subnet_mask.Val;
    
    //Store our new gateway address
    if (option_gateway.Val)
      net->GatewayIpAddress.Val = option_gateway.Val;
    
    //Set DHCP state machine to new state
    sm_dhcp = DHCP_STATE::DHCP_BOUND;
    
  }
  else
  {
    goto rxPacket_dump;
  }
  
  //-----------------------------------------------------
  //----- GOOD DHCP RESPONSE RECEIVED AND PROCESSED -----
  //-----------------------------------------------------
  udp->udp_dump_rx_packet();
  return(1);
  
rxPacket_dump:
  //-------------------------------------------
  //----- RESPONSE RECEIVED BUT NOT VALID -----
  //-------------------------------------------
  udp->udp_dump_rx_packet();
  return(0);
}

//**************************************
//**************************************
//********** SEND DHCP PACKET **********
//**************************************
//**************************************
void DHCP::txPacket (uint8_t message_type)
{
  uint16_t w_temp;
  x_NIC* nic = (x_NIC*)net->GetObject(StackObject::NIC);
  UDP* udp = (UDP*)net->GetObject(StackObject::UDP);
  //----- A UDP PACKET HAS BEEN SETUP AND WE ARE READY TO WRITE THE UDP DATA AREA -----
  
  //Send Boot Request
  udp->udp_write_next_byte(1);
  
  //Send Hardware Address Type = Ethernet
  udp->udp_write_next_byte(1);
  
  //Address Length
  udp->udp_write_next_byte(6);
  
  //Hops
  udp->udp_write_next_byte(0);
  
  //Transaction ID (Random number - use our mac address)
  udp->udp_write_next_byte(nic->MAC()->v[2]);
  udp->udp_write_next_byte(nic->MAC()->v[3]);
  udp->udp_write_next_byte(nic->MAC()->v[4]);
  udp->udp_write_next_byte(nic->MAC()->v[5]);
  
  //Elapsed Time
  udp->udp_write_next_byte(0);
  udp->udp_write_next_byte(0);
  
  //Flags
  udp->udp_write_next_byte(0);
  udp->udp_write_next_byte(0);
  //Client IP Address
  udp->udp_write_next_byte(net->LocalIpAddress.v[0]);
  udp->udp_write_next_byte(net->LocalIpAddress.v[1]);
  udp->udp_write_next_byte(net->LocalIpAddress.v[2]);
  udp->udp_write_next_byte(net->LocalIpAddress.v[3]);
  
  //Your (Client) IP Address
  udp->udp_write_next_byte(0);
  udp->udp_write_next_byte(0);
  udp->udp_write_next_byte(0);
  udp->udp_write_next_byte(0);
  
  //Server IP Address
  udp->udp_write_next_byte(0);
  udp->udp_write_next_byte(0);
  udp->udp_write_next_byte(0);
  udp->udp_write_next_byte(0);
  
  //Relay agent IP
  udp->udp_write_next_byte(0);
  udp->udp_write_next_byte(0);
  udp->udp_write_next_byte(0);
  udp->udp_write_next_byte(0);
  
  //Our MAC address
  udp->udp_write_next_byte(nic->MAC()->v[0]);
  udp->udp_write_next_byte(nic->MAC()->v[1]);
  udp->udp_write_next_byte(nic->MAC()->v[2]);
  udp->udp_write_next_byte(nic->MAC()->v[3]);
  udp->udp_write_next_byte(nic->MAC()->v[4]);
  udp->udp_write_next_byte(nic->MAC()->v[5]);
  
  //Unused bytes
  for (w_temp = 0; w_temp < 202; w_temp++)
    udp->udp_write_next_byte(0);
  
  //DHCP Magic Cookie
  udp->udp_write_next_byte(0x63);
  udp->udp_write_next_byte(0x82);
  udp->udp_write_next_byte(0x53);
  udp->udp_write_next_byte(0x63);
  
  //----- BELOW HERE ARE THE DHCP OPTIONS -----
  //(These may be changed if required)
  
  //Send Option - DHCP Message Type
  udp->udp_write_next_byte(53);					//Option
  udp->udp_write_next_byte(1);						//Length
  udp->udp_write_next_byte(message_type);			//Data
  
  //Send Option - Client ID
  udp->udp_write_next_byte(61);					//Option
  udp->udp_write_next_byte(7);						//Length
  udp->udp_write_next_byte(1);						//Data - Hardware type (Ethernet)
  udp->udp_write_next_byte(nic->MAC()->v[0]);	//Data - our client ID (MAC address)
  udp->udp_write_next_byte(nic->MAC()->v[1]);
  udp->udp_write_next_byte(nic->MAC()->v[2]);
  udp->udp_write_next_byte(nic->MAC()->v[3]);
  udp->udp_write_next_byte(nic->MAC()->v[4]);
  udp->udp_write_next_byte(nic->MAC()->v[5]);
  
  
  //If doing DHCP request then send option - dhcp server IP, and send option - our requested IP
  if (sm_dhcp == DHCP_STATE::DHCP_REQUEST)
  {
    udp->udp_write_next_byte(54);						//Option
    udp->udp_write_next_byte(4);							//Length
    udp->udp_write_next_byte(serverIP.v[0]);	//Data
    udp->udp_write_next_byte(serverIP.v[1]);
    udp->udp_write_next_byte(serverIP.v[2]);
    udp->udp_write_next_byte(serverIP.v[3]);
    
    udp->udp_write_next_byte(50);						//Option
    udp->udp_write_next_byte(4);							//Length
    udp->udp_write_next_byte(offerIP.v[0]);	//Data
    udp->udp_write_next_byte(offerIP.v[1]);
    udp->udp_write_next_byte(offerIP.v[2]);
    udp->udp_write_next_byte(offerIP.v[3]);
  }
  
  //Send our name if we have one assigned to the pointer variable
  if (eth_dhcp_our_name_pointer)
  {
    udp->udp_write_next_byte(12);					//Option
    udp->udp_write_next_byte(16);					//Length
    udp->udp_write_next_byte(eth_dhcp_our_name_pointer[0]);					//Data - our ascii name
    udp->udp_write_next_byte(eth_dhcp_our_name_pointer[1]);
    udp->udp_write_next_byte(eth_dhcp_our_name_pointer[2]);
    udp->udp_write_next_byte(eth_dhcp_our_name_pointer[3]);
    udp->udp_write_next_byte(eth_dhcp_our_name_pointer[4]);
    udp->udp_write_next_byte(eth_dhcp_our_name_pointer[5]);
    udp->udp_write_next_byte(eth_dhcp_our_name_pointer[6]);
    udp->udp_write_next_byte(eth_dhcp_our_name_pointer[7]);
    udp->udp_write_next_byte(eth_dhcp_our_name_pointer[8]);
    udp->udp_write_next_byte(eth_dhcp_our_name_pointer[9]);
    udp->udp_write_next_byte(eth_dhcp_our_name_pointer[10]);
    udp->udp_write_next_byte(eth_dhcp_our_name_pointer[11]);
    udp->udp_write_next_byte(eth_dhcp_our_name_pointer[12]);
    udp->udp_write_next_byte(eth_dhcp_our_name_pointer[13]);
    udp->udp_write_next_byte(eth_dhcp_our_name_pointer[14]);
    udp->udp_write_next_byte(0x00);
  }
  
  
  //End of options marker
  udp->udp_write_next_byte(0xff);
  
  
  //----- SEND THE PACKET -----
  udp->udp_tx_packet();
}
//------------------------------------------------------------------------------
