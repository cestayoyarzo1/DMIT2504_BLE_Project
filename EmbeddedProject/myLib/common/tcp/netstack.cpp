/*
FILE: netstack.cpp
Created on: 9/1/2017, by Tom Diaz
*/

#include <netstack.h>
//*****************************************
//*****************************************
//********** Initialise ETHERNET **********
//*****************************************
//*****************************************
//(This function must be called before any of the stack or its component routines are used.
void NetStack::tcp_ip_Initialise (x_NIC* nic_)
{
  ManualIp = false;
  
  nic = nic_;
  
  nic->Initialise();
  
  ip.Init(this);
  
  arp.Init(this);
  
  udp.udp_Initialise(this);
  
  dhcp.Init(this);
  
  dns.Init(this);
  
  /*if(tcp)
    tcp->tcp_Initialise(this);
  
  if(httpClient)
    httpClient->Init(this);*/
  
  sm_ethernet_stack = SM_ETH_STACK_IDLE;
}

void* NetStack::GetObject(StackObject o)
{
  switch(o)
  {
  case StackObject::NIC:
    return nic;
  case StackObject::IP:
    return &ip;
  case StackObject::UDP:
    return &udp;
  case StackObject::ARP:
    return &arp;
  case StackObject::DHCP:
    return &dhcp;
  case StackObject::TCP:
    return tcp;
  case StackObject::DNS:
    return &dns;
  default:
    return 0;
  }
}

void NetStack::SetObject(StackObject o, void* object)
{
  switch(o)
  {
  case StackObject::HTTP_CLIENT:
    httpClient = (HttpClient*)object;
    httpClient->Init(this);
    break;
  case StackObject::TCP:
    tcp = (TCP*)object;
    tcp->tcp_Initialise(this);
    break;
  }
}

int8_t NetStack::DhcpState()
{
  if(!this->ManualIp)
    return (int8_t)dhcp.sm_dhcp;
  else
    return -1;
}

//***************************************
//***************************************
//********** PROCESS THE STACK **********
//***************************************
//***************************************
//This state machine checks for new incoming packets, and routes them to appropriate stack functions.  It also performs timed operations.
//This function must be called called reguarly (typically as part of the main loop of your application).
void NetStack::tcp_ip_process_stack (void)
{
  //-----------------------------------------------
  //----- GET THE CURRENT ETHERNET TIMER TIME -----
  //-----------------------------------------------
  ethernet_10ms_clock_timer = ethernet_10ms_clock_timer_working;
  //CHECK THAT TIMER HASN'T JUST CHANGED IN WHICH CASE WE COULD HAVE READ A CORRUPT TIME AS THE VALUE IS PROBABLY CHANGED IN AN INTERRUPT
  while (ethernet_10ms_clock_timer != ethernet_10ms_clock_timer_working)
    ethernet_10ms_clock_timer = ethernet_10ms_clock_timer_working;
  
  
  //------------------------------------------------
  //----- IF NIC IS NOT LINKED RESET THE STACK -----
  //------------------------------------------------
  if (!nic->Linked())
    sm_ethernet_stack = SM_ETH_STACK_IDLE;
  
  //-------------------------------------------------------------
  //----- IF DHCP IS NOT INCLUDED THEN DO DHCP GENERAL FLAG -----
  //-------------------------------------------------------------
  if(!this->ManualIp)
    this->NicLinkedAndIpValid = false;
  else
    this->NicLinkedAndIpValid = nic->Linked();
  //------------------------------------------
  //----- CHECK FOR DUMP RECEIVED PACKET -----
  //------------------------------------------
  //(Backup in case an application function has opened a socket but doesn't process a received packet for that socket for some reason)
  if (nic->RxPacketWaitingToBeDumped)
  {
    nic->RxPacketWaitingToBeDumped--;
    if (nic->RxPacketWaitingToBeDumped == 0)
    {
      udp.udp_rx_packet_is_waiting_to_be_processed = 0;
      tcp->tcp_rx_packet_is_waiting_to_be_processed = 0;
      nic->DumpPacket();
    }
  }
  //-----------------------------
  //-----------------------------
  //----- PROCESS THE STACK -----
  //-----------------------------
  //-----------------------------
  process_stack_again = 1;
  while(process_stack_again)
  {
    process_stack_again = 0;			//Default to no need to run through the stack checks again for a subsequent task
    
    switch(sm_ethernet_stack)
    {
    case SM_ETH_STACK_IDLE:
      State_IDLE();
      break;
    case SM_ETH_STACK_ARP:
      State_ARP();
      break;    
    case SM_ETH_STACK_IP:
      State_IP();
      break;
    case SM_ETH_STACK_ICMP:
      State_ICMP();
      break;
    case SM_ETH_STACK_ICMP_REPLY:
      State_ICMP_REPLY();
      break;
    case SM_ETH_STACK_UDP:
      State_UDP();
      break;
    case SM_ETH_STACK_TCP:
      State_TCP();
      break;
    }
  }
  
  dhcp.FSM();
  
  if(tcp)
    tcp->process_tcp();

  dns.FSM();
  
  if(httpClient)
    httpClient->process_http_client();
}

//----------------------State Machine States------------------------------------
void NetStack::State_IDLE()
{
  WORD_VAL rx_ethernet_packet_type;
  //-----------------------------
  //-----------------------------
  //----- THE STACK IS IDLE -----
  //-----------------------------
  //-----------------------------
  
  //----- CHECK FOR A PACKET RECEIVED (AND DO ANY OTHER NIC BACKGROUND TASKS) -----
  //(This must happen reguarly)
  rx_len = nic->CheckForRx();
  
  if (rx_len == 0)
  {
    //-----------------------------------------------
    //----- THERE IS NO RECEIVED PACKET WAITING -----
    //-----------------------------------------------
    
    return;
  }
  else
  {
    //-------------------------------------
    //----- NIC HAS RECEIVED A PACKET -----
    //-------------------------------------
    //(The nic function has read the packet length from the nic IC and the next byte to read is the first byte of the data area)
    
    process_stack_again = 1;				//Process the stack again for this received packet or if this packet is dumped for any other packet that may be waiting
    
    //----- GET THE ETHERNET HEADER -----
    //Get the destination mac address [6]
    nic->ReadNextByte(&rx_destination_mac_address.v[0]);
    nic->ReadNextByte(&rx_destination_mac_address.v[1]);
    nic->ReadNextByte(&rx_destination_mac_address.v[2]);
    nic->ReadNextByte(&rx_destination_mac_address.v[3]);
    nic->ReadNextByte(&rx_destination_mac_address.v[4]);
    nic->ReadNextByte(&rx_destination_mac_address.v[5]);
    
    //Get the source mac address [6]
    nic->ReadNextByte(&remoteDevice.mac_address.v[0]);
    nic->ReadNextByte(&remoteDevice.mac_address.v[1]);
    nic->ReadNextByte(&remoteDevice.mac_address.v[2]);
    nic->ReadNextByte(&remoteDevice.mac_address.v[3]);
    nic->ReadNextByte(&remoteDevice.mac_address.v[4]);
    nic->ReadNextByte(&remoteDevice.mac_address.v[5]);
    
    //Get the header type [2]
    nic->ReadNextByte(&rx_ethernet_packet_type.v[1]);
    nic->ReadNextByte(&rx_ethernet_packet_type.v[0]);
    
    
    //----- DECIDE WHAT TO DO FROM THE PACKET TYPE VALUE -----
    if (rx_ethernet_packet_type.Val == 0x0806)
    {
      //-------------------------
      //----- PACKET IS ARP -----
      //-------------------------
      sm_ethernet_stack = SM_ETH_STACK_ARP;
    }
    else if (rx_ethernet_packet_type.Val == 0x0800)
    {
      //------------------------
      //----- PACKET IS IP -----
      //------------------------
      sm_ethernet_stack = SM_ETH_STACK_IP;
    }
    
    //<<<< ADD ANY OTHER ETHERNET PACKET TYPES HERE
    
    else
    {
      //----------------------------------
      //----- PACKET TYPE IS UNKNOWN -----
      //----------------------------------
      nic->DumpPacket();
    }
  }
}

void NetStack::State_ARP()
{
  //---------------------------------------
  //---------------------------------------
  //----- PROCESS RECEIVED ARP PACKET -----
  //---------------------------------------
  //---------------------------------------
  if (arp.Rx())
  {
    //----- ARP PACKET HAS BEEN PROCESSED -----
    sm_ethernet_stack = SM_ETH_STACK_IDLE;
  }
  //else
  //{
  //----- ARP PACKET HAS BEEN READ BUT ARP COULDN'T TRANSMIT A RESPONSE RIGHT NOW - WE'LL KEEP CALLING ARP_PROCESS_RX UNTIL IT CAN -----
  //}
  nic->DumpPacket();		//Ensure packet has been dumped
}

void NetStack::State_IP()
{
  uint8_t ip_protocol;
  //--------------------------------------
  //--------------------------------------
  //----- PROCESS RECEIVED IP PACKET -----
  //--------------------------------------
  //--------------------------------------
  
  //----- GET THE IP HEADER -----
  if (ip.ip_get_header(&destination_ip_address, &remoteDevice, &ip_protocol, &data_remaining_bytes))
  {
    //-------------------------------
    //----- IP HEADER RETREIVED -----
    //-------------------------------
    process_stack_again = 1;		//Process the stack again for this packet once we've set the state to handle it below
    
    if (ip_protocol == IP_PROTOCOL_ICMP)
    {
      //----- PACKET IS ICMP -----
      sm_ethernet_stack = SM_ETH_STACK_ICMP;
    }
    
    else if (ip_protocol == IP_PROTOCOL_UDP)
    {
      //----- PACKET IS UDP -----
      sm_ethernet_stack = SM_ETH_STACK_UDP;
    }
    
    else if (ip_protocol == IP_PROTOCOL_TCP)
    {
      //----- PACKET IS TCP -----
      sm_ethernet_stack = SM_ETH_STACK_TCP;
    }
    
    //<<<<< ADD ANY OTHER IP PROTOCOL TYPES HERE
    
    else
    {
      //----- UNKNOWN IP PACKET TYPE -----
      nic->DumpPacket();
      process_stack_again = 0;
      sm_ethernet_stack = SM_ETH_STACK_IDLE;
    }
  }
  else
  {
    //-------------------------
    //----- BAD IP HEADER -----
    //-------------------------
    //(The packet has already been dumped by ip_get_header)
    sm_ethernet_stack = SM_ETH_STACK_IDLE;
  }
}

void NetStack::State_ICMP()
{
  //----------------------------------------
  //----------------------------------------
  //----- PROCESS RECEIVED ICMP PACKET -----
  //----------------------------------------
  //----------------------------------------
  //(ICMP is used by 'ping')
  /*#ifdef STACK_USE_ICMP
  //Check the packet is for us (broadcast is not allowed)
  if (destination_ip_address.Val == our_ip_address.Val)
  {
  //Process the packet as long as its length is OK
  if (data_remaining_bytes <= (ICMP_MAX_DATA_LENGTH + ICMP_HEADER_LENGTH))
  {
  if (icmp_process_received_echo_request(&icmp_id, &icmp_sequence, &icmp_data_buffer[0], data_remaining_bytes))
  {
  //----- VALID ICMP ECHO REQUEST RECEIVED -----
  process_stack_again = 1;
  sm_ethernet_stack = SM_ETH_STACK_ICMP_REPLY;			//(Will be processed when loop runs again)
}
          else
  {
  //The request was not valid
  sm_ethernet_stack = SM_ETH_STACK_IDLE;
}
}
        else
  {
  //Packet is too long
  sm_ethernet_stack = SM_ETH_STACK_IDLE;
}
}
      else
  {
  //Packet was not sent to our IP address so ignore it
  sm_ethernet_stack = SM_ETH_STACK_IDLE;
}
#else*/
  sm_ethernet_stack = SM_ETH_STACK_IDLE;
  //#endif
  
  //Dump the packet if it hasn't been already
  nic->DumpPacket();
}

void NetStack::State_ICMP_REPLY()
{
  //--------------------------------
  //--------------------------------
  //----- SEND ICMP ECHO REPLY -----
  //--------------------------------
  //--------------------------------
  /*#ifdef STACK_USE_ICMP
  if (nic->SetupTx())
  {
  //----- WE CAN TRANSMIT OUR RESPONSE NOW -----
  icmp_send_packet(&remoteDevice,ICMP_ECHO_REPLY, &icmp_data_buffer[0], (data_remaining_bytes - ICMP_HEADER_LENGTH),
  &icmp_id, &icmp_sequence);
  
  sm_ethernet_stack = SM_ETH_STACK_IDLE;
  break;;
}
      else
  {
  //----- CAN'T TX OUR RESPONSE NOW - TRY AGAIN NEXT TIME -----
  break;
}
#else*/
  sm_ethernet_stack = SM_ETH_STACK_IDLE;
  //#endif
}

void NetStack::State_UDP() //----- PROCESS RECEIVED UDP PACKET -----
{
  
  if (udp.udp_process_rx(&remoteDevice, &destination_ip_address, data_remaining_bytes))
  {
    //----- UDP PACKET HAS BEEN PROCESSED -----
    sm_ethernet_stack = SM_ETH_STACK_IDLE;
  }
  process_stack_again = 0;
}

void NetStack::State_TCP() //----- PROCESS RECEIVED TCP PACKET -----
{
  if(tcp) // If TCP object was provided, ie not null
    if (tcp->tcp_process_rx(&remoteDevice, &destination_ip_address, data_remaining_bytes))
    {
      //----- TCP PACKET HAS BEEN PROCESSED -----
      sm_ethernet_stack = SM_ETH_STACK_IDLE;
    }
  process_stack_again = 0;
}