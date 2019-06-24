/*
  FILE: dns.cpp
  Created on: 8/29/2017, by Tom Diaz
*/

#include <dns.h>
#include <iputils.h>
#include <udp.h>
#include <arp.h>

void DNS::Init(x_Network* net_)
{
  net = net_;
}

//**************************************
//**************************************
//********** SEND DNS REQUEST **********
//**************************************
//**************************************
//requested_domain_name
//	= 'www.###.com' etc for a web address (non case sensitive)
//	= ###.com etc for a mail server mx address
//qtype
//	= 1 for host (www lookup)
//	= 15 for mx mailserver (email lookup)
void DNS::sendRequest (uint8_t *requested_domain_name, uint8_t qtype)
{
  uint8_t name_character_number;
  uint8_t name_length;
  uint8_t name_string[64];
  uint8_t loop_count;
  UDP* udp = (UDP*)net->GetObject(StackObject::UDP);
  
  //UDP PACKET HAS ALREADY BEEN SETUP
  
  //---------------------------
  //----- SEND THE HEADER -----
  //---------------------------
  
  //----- ID [2] -----
  //(Message ID number)
  udp->udp_write_next_byte(0x12);
  udp->udp_write_next_byte(0x23);
  
  //----- FLAGS [2] -----
  //QR (Bit 15)
  //	0 = Query 
  //	1 = response
  //OPCODE (Bits 14:11)
  //	0 = standard query (QUERY)
  //	1 = inverse query (IQUERY)
  //	2 = server status request (STATUS)
  //	3-15 =unused
  //AA (Bit 10)
  //	Authoritative Answer - valid in responses - specifies that the responding name server is an authority for the domain name in question section
  //	(May have multiple owner names because of aliases.  The AA bit corresponds to the name which matches the query name, or the first owner name
  //	in the answer section)
  //TC (Bit 9)
  //	TrunCation - specifies that this message was truncated due to length greater than that permitted on the transmission channel
  //RD (Bit 8)
  //	Recursion Desired - may be set in a query and is copied into the response. 
  //	1 = pursue the query recursively (carry out DNS and return the IP address instead of simply responding with a list of name servers if IP is unknown)
  udp->udp_write_next_byte(0x01);		//Recursion desired - we rely on the gateway will carry out recursion for us
  
  //RA (Bit 7)
  //	Recursion Available - set or cleared in a response, and denotes whether recursive query support is available in the name server
  //Z (Bits 6:4)
  //	Reserved for future use.  Must be zero in all queries and responses
  //RCODE (Bits 3:0)
  //	Response code
  //	0 = No error condition
  //	1 = Format error - The name server was unable to interpret the query
  //	2 = Server failure
  //	3 = Name Error (Meaningful only for responses from an authoritative name server, this code signifies that the domain name referenced in the
  //	query does not exist)
  //	4 = Not Implemented (The name server does not support the requested kind of query)
  //	5 = Refused
  //	6-15 = unused
  udp->udp_write_next_byte(0x00);
  
  
  //----- QDCOUNT [2] -----
  //(The number of entries in the question section)
  udp->udp_write_next_byte(0x00);
  udp->udp_write_next_byte(0x01);
  
  //----- ANCOUNT [2] -----
  //(The number of resource records in the answer section)
  udp->udp_write_next_byte(0x00);
  udp->udp_write_next_byte(0x00);
  
  //----- NSCOUNT [2] -----
  //(The number of name server resource records in the authority records section)
  udp->udp_write_next_byte(0x00);
  udp->udp_write_next_byte(0x00);
  
  //----- ARCOUNT [2] -----
  //(The number of resource records in the additional records section)
  udp->udp_write_next_byte(0x00);
  udp->udp_write_next_byte(0x00);
  
  
  //-----------------------------
  //----- SEND THE QUESTION -----
  //-----------------------------
  
  //----- QNAME [#] -----
  //A domain name represented as a sequence of labels, where each label consists of a length octet followed by that
  //number of bytes.  The domain name terminates with the zero length octet for the null label of the root.  Note
  //that this field may be an odd number of bytes and no padding is used.
  //An example:-
  //0x03, 'W', 'W', 'W',
  //0x05, 'Y', 'A', 'H', 'O', 'O',
  //0x03, 'C', 'O', 'M',
  //0x00
  
  name_character_number = 0;
  while (1)
  {
    //GET THE NEXT LABEL SECTION OF THE NAME
    name_length = 0;
    while (1)
    {
      name_string[name_length] = dns_requested_url[name_character_number];
      if (name_string[name_length] == '.')		//Period name seperator?
        break;
      if (name_string[name_length] == 0x00)		//Null end of name?
        break;
      if (name_length >= 64)						//Check for overflow
        break;
      
      name_length++;
      name_character_number++;
    }
    name_character_number++;
    
    //Write the next label section of the name
    udp->udp_write_next_byte(name_length);
    for (loop_count = 0; loop_count < name_length; loop_count++)
      udp->udp_write_next_byte(name_string[loop_count]);
    
    if (name_string[name_length] == 0x00)		//Exit if reached null end of name?
      break;
  }
  udp->udp_write_next_byte(0);		//Null termination
  
  //----- QTYPE [2] -----
  //A two octet code which specifies the type of the query.  The values for this field include all codes valid for a
  //TYPE field, together with some more general codes which can match more than one type of RR
  //Value 1 = a host address (IP v4 address), 15 = mail mx
  udp->udp_write_next_byte(0);
  udp->udp_write_next_byte(qtype);
  
  //----- QCLASS [2] -----
  //A two octet code that specifies the class of the query.  For example, the QCLASS field is IN for the Internet
  udp->udp_write_next_byte(0);			//Value 1 = The internet
  udp->udp_write_next_byte(1);
  
  
  //------------------
  //----- ANSWER -----
  //------------------
  //----- NAME [#] -----
  //A domain name to which this resource record pertains
  
  //----- TYPE [2] -----
  //Two bytes containing one of the RR type codes.  This field specifies the meaning of the data in the RDATA field
  
  //----- CLASS [2] -----
  //Two bytes which specify the class of the data in the RDATA field
  
  //----- TTL [4] -----
  //The time interval in seconds that the resource record may be cached before it should be discarded.  Zero values are
  //interpreted to mean that the RR can only be used for the transaction in progress, and should not be cached
  
  //----- RDLENGTH [2] -----
  //The length in bytes of the RDATA field
  
  //----- RDATA [#] -----
  //A variable length string of bytes that describes the resource.  The format of this information varies
  //according to the TYPE and CLASS of the resource record.  For example, the if the TYPE is A and the CLASS is IN,
  //the RDATA field is a 4 octet ARPA Internet address
  
  
  //---------------------
  //----- AUTHORITY -----
  //---------------------
  //----- NAME [#] -----
  //A domain name to which this resource record pertains
  
  //----- TYPE [2] -----
  //Two bytes containing one of the RR type codes.  This field specifies the meaning of the data in the RDATA field
  
  //----- CLASS [2] -----
  //Two bytes which specify the class of the data in the RDATA field
  
  //----- TTL [4] -----
  //The time interval (in seconds) that the resource record may be cached before it should be discarded.  Zero values are
  //interpreted to mean that the RR can only be used for the transaction in progress, and should not be cached
  
  //----- RDLENGTH [2] -----
  //The length in bytes of the RDATA field
  
  //----- RDATA [#] -----
  //A variable length string of bytes that describes the resource.  The format of this information varies
  //according to the TYPE and CLASS of the resource record.  For example, the if the TYPE is A and the CLASS is IN,
  //the RDATA field is a 4 octet ARPA Internet address
  
  
  //----------------------
  //----- ADDITIONAL -----
  //----------------------
  
  //----- NAME [#] -----
  //A domain name to which this resource record pertains
  
  //----- TYPE [2] -----
  //Two bytes containing one of the RR type codes.  This field specifies the meaning of the data in the RDATA field
  
  //----- CLASS [2] -----
  //Two bytes which specify the class of the data in the RDATA field
  
  //----- TTL [4] -----
  //The time interval (in seconds) that the resource record may be cached before it should be discarded.  Zero values are
  //interpreted to mean that the RR can only be used for the transaction in progress, and should not be cached
  
  //----- RDLENGTH [2] -----
  //The length in bytes of the RDATA field
  
  //----- RDATA [#] -----
  //A variable length string of bytes that describes the resource.  The format of this information varies
  //according to the TYPE and CLASS of the resource record.  For example, the if the TYPE is A and the CLASS is IN,
  //the RDATA field is a 4 octet ARPA Internet address
  
  
  //---------------------------
  //----- SEND THE PACKET -----
  //---------------------------
  udp->udp_tx_packet();
}

//*********************************************
//*********************************************
//********** GET RECEIVED DNS PACKET **********
//*********************************************
//*********************************************
//We request that the network gateway device does a recursive search (i.e. it goes through the tree of name servers to find the IP address).  As
//the DNS response can use compression (compression of names by repeated names or sections of names being re-used later on in the packet) we don't
//check the response names (as this is unecessary and would involve a great deal of complexity and ram space).  Instead we check the answer and
//additional sections looking for an IP address.
//DNS may return several IP addresses (for instance for large sites) and in this case we just use the first one.
//
//Returns
//	1 = good response received and resolved_ip_address has been loaded
//	0 = DNS failed
uint8_t DNS::checkResponse(uint8_t *requested_domain_name, uint8_t qtype, IP_ADDR *resolved_ip_address)
{
  uint8_t loop_count;
  uint8_t received_byte;
  WORD_VAL qd_count;
  WORD_VAL an_count;
  WORD_VAL ns_count;
  WORD_VAL ar_count;
  uint8_t name_character_number = 0;
  uint8_t label_length;
  uint16_t data_length;
  uint8_t response_is_good;
  UDP* udp = (UDP*)net->GetObject(StackObject::UDP);
  
  
  //---------------------------
  //----- GET THE HEADER -----
  //---------------------------
  
  //----- ID [2] -----
  //(Message ID number)
  udp->udp_read_next_rx_byte(&received_byte);
  if (received_byte != 0x12)				//Should match the number we sent
    goto dns_check_response_failed;
  udp->udp_read_next_rx_byte(&received_byte);
  if (received_byte != 0x23)
    goto dns_check_response_failed;
  
  //----- FLAGS [2] -----
  //QR (Bit 15)
  //	0 = Query 
  //	1 = response
  //OPCODE (Bits 14:11)
  //	0 = standard query (QUERY)
  //	1 = inverse query (IQUERY)
  //	2 = server status request (STATUS)
  //	3-15 =unused
  //AA (Bit 10)
  //	Authoritative Answer - valid in responses - specifies that the responding name server is an authority for the domain name in question section
  //	(May have multiple owner names because of aliases.  The AA bit corresponds to the name which matches the query name, or the first owner name
  //	in the answer section)
  //TC (Bit 9)
  //	TrunCation - specifies that this message was truncated due to length greater than that permitted on the transmission channel
  //RD (Bit 8)
  //	Recursion Desired - may be set in a query and is copied into the response. 
  //	1 = pursue the query recursively (carry out DNS and return the IP address instead of simply responding with a list of name servers if IP is unknown)
  udp->udp_read_next_rx_byte(&received_byte);
  if ((received_byte & 0x80)  == 0)		//Check message is a response
    goto dns_check_response_failed;
  
  //RA (Bit 7)
  //	Recursion Available - set or cleared in a response, and denotes whether recursive query support is available in the name server
  //Z (Bits 6:4)
  //	Reserved for future use.  Must be zero in all queries and responses
  //RCODE (Bits 3:0)
  //	Response code
  //	0 = No error condition
  //	1 = Format error - The name server was unable to interpret the query
  //	2 = Server failure
  //	3 = Name Error (Meaningful only for responses from an authoritative name server, this code signifies that the domain name referenced in the
  //	query does not exist)
  //	4 = Not Implemented (The name server does not support the requested kind of query)
  //	5 = Refused
  //	6-15 = unused
  udp->udp_read_next_rx_byte(&received_byte);
  if ((received_byte & 0x80)  == 0)		//Check server supports recursion
    goto dns_check_response_failed;
  
  if ((received_byte & 0x0f)  != 0)		//Response code should be zero for no error
    goto dns_check_response_failed;
  
  //----- QDCOUNT [2] -----
  //(The number of entries in the question section)
  udp->udp_read_next_rx_byte(&qd_count.v[1]);
  udp->udp_read_next_rx_byte(&qd_count.v[0]);
  
  //----- ANCOUNT [2] -----
  //(The number of resource records in the answer section)
  udp->udp_read_next_rx_byte(&an_count.v[1]);
  udp->udp_read_next_rx_byte(&an_count.v[0]);
  
  //----- NSCOUNT [2] -----
  //(The number of name server resource records in the authority records section)
  udp->udp_read_next_rx_byte(&ns_count.v[1]);
  udp->udp_read_next_rx_byte(&ns_count.v[0]);
  
  //----- ARCOUNT [2] -----
  //(The number of resource records in the additional records section)
  udp->udp_read_next_rx_byte(&ar_count.v[1]);
  udp->udp_read_next_rx_byte(&ar_count.v[0]);
  
  
  //------------------------------------
  //----- GET THE QUESTION ENTRIES -----
  //------------------------------------
  
  //----- QNAME [#] -----
  //(A domain name represented as a sequence of labels, where each label consists of a length octet followed by that
  //number of bytes.  The domain name terminates with the zero length octet for the null label of the root.  Note
  //that this field may be an odd number of bytes; no padding is used)
  
  if (qd_count.Val !=1)				//There should only be 1 question entry as we only asked 1 question
    goto dns_check_response_failed;
  
  name_character_number = 0;
  while(1)
  {
    //GET THE LENGTH OF THE NEXT LABEL
    udp->udp_read_next_rx_byte(&label_length);
    if ((label_length == 0) && (name_character_number > 0))				//If the length is 0 then name is complete
      break;
    
    //THE NEXT CHARACTER IN THE REQUESTED NAME SHOULD BE '.'
    if (name_character_number)
    {
      if (*(requested_domain_name + name_character_number) != '.')
        goto dns_check_response_failed;
      
      name_character_number++;
    }
    
    //GET AND CHECK EACH CHARACTER MATCHES OUR QUESTION
    for (loop_count = 0; loop_count < label_length; loop_count++)
    {
      udp->udp_read_next_rx_byte(&received_byte);
      if (IPUtils::convert_character_to_lower_case(received_byte) != 
          IPUtils::convert_character_to_lower_case(*(requested_domain_name + name_character_number)))
        goto dns_check_response_failed;
      name_character_number++;
    }
  }
  
  //----- QTYPE [2] -----
  //(A two octet code which specifies the type of the query.  The values for this field include all codes valid for a
  //TYPE field, together with some more general codes which can match more than one type of RR)
  //Should match the request we sent
  udp->udp_read_next_rx_byte(&received_byte);
  if (received_byte != 0)					//Check high byte
    goto dns_check_response_failed;
  
  udp->udp_read_next_rx_byte(&received_byte);
  if (received_byte != qtype)				//Check low byte
    goto dns_check_response_failed;
  
  //----- QCLASS [2] -----
  //(A two octet code that specifies the class of the query. Value 1 = The internet
  //Should match the request we sent
  udp->udp_read_next_rx_byte(&received_byte);
  if (received_byte != 0)					//Check high byte
    goto dns_check_response_failed;
  
  udp->udp_read_next_rx_byte(&received_byte);
  if (received_byte != 1)					//Check low byte
    goto dns_check_response_failed;
  
  
  //------------------
  //----- ANSWER -----
  //------------------
  // THIS SECTION MAY CONTAIN THE IP ADDRESS WE REQUESTED
  //If doing a type 1 'host address' dns then this section will contain the IP address
  //If doing a type 15 'mx' mailserver address the additional section will contain the IP address if its not here
  while (an_count.Val)
  {
    response_is_good = 1;		//Default to good record
    
    //NAME [#]
    //(A domain name to which this resource record pertains)
    //As we're not buffering the entire packet to deal with the possible compression we don't bother to check this
    while (1)
    {
      udp->udp_read_next_rx_byte(&received_byte);
      
      if (received_byte == 0x00)
      {
        //Null - name is done
        break;
      }
      
      if (received_byte & 0xc0)
      {
        //Using a compressed name
        //Get the next byte of the name location offset and exit
        udp->udp_read_next_rx_byte(&received_byte);
        break;
      }
      else
      {
        //Using a normal name
        //Get all of the name characters
        label_length = received_byte;
        while (label_length)
        {
          udp->udp_read_next_rx_byte(&received_byte);
          label_length--;
        }
      }
    }
    
    //----- TYPE [2] -----
    //(Two bytes containing one of the RR type codes.  This field specifies the meaning of the data in the RDATA field)
    //Should be type 'A' = host address (value = 0x0001)
    udp->udp_read_next_rx_byte(&received_byte);		//Get and check high byte
    if (received_byte != 0x00)
      response_is_good = 0;
    
    udp->udp_read_next_rx_byte(&received_byte);		//Get and check low byte
    if (received_byte != 0x01)
      response_is_good = 0;
    
    //----- CLASS [2] -----
    //(Two bytes which specify the class of the data in the RDATA field)
    //Should be class 'IN' = internet (value = 0x0001)
    udp->udp_read_next_rx_byte(&received_byte);		//Get and check high byte
    if (received_byte != 0x00)
      response_is_good = 0;
    
    udp->udp_read_next_rx_byte(&received_byte);		//Get and check low byte
    if (received_byte != 0x01)
      response_is_good = 0;
    
    //----- TTL [4] -----
    //(The time interval (in seconds) that the resource record may be cached before it should be discarded.  Zero values are
    //interpreted to mean that the RR can only be used for the transaction in progress, and should not be cached)
    udp->udp_read_next_rx_byte(&received_byte);
    udp->udp_read_next_rx_byte(&received_byte);
    udp->udp_read_next_rx_byte(&received_byte);
    udp->udp_read_next_rx_byte(&received_byte);
    
    //----- RDLENGTH [2] -----
    //(The length in bytes of the RDATA field)
    udp->udp_read_next_rx_byte(&received_byte);		//Get and check high byte
    if (received_byte != 0x00)
      goto dns_check_response_failed;
    
    udp->udp_read_next_rx_byte(&received_byte);		//Get and check low byte
    label_length = received_byte;
    if (received_byte != 0x04)
      response_is_good = 0;
    
    //----- RDATA [#] -----
    //(A variable length string of bytes that describes the resource.  The format of this information varies
    //according to the TYPE and CLASS of the resource record.  For example, the if the TYPE is A and the CLASS is IN,
    //the RDATA field is a 4 octet ARPA Internet address)
    for (loop_count = 0; loop_count < label_length; loop_count++)
    {
      udp->udp_read_next_rx_byte(&received_byte);
      if (response_is_good)
        resolved_ip_address->v[loop_count] = received_byte;
    }
    
    if (response_is_good)
      goto dns_check_response_good;
    
    ar_count.Val--;
  }
  
  
  //---------------------
  //----- AUTHORITY -----
  //---------------------
  // THIS SECTION WON'T CONTAIN THE IP ADDRESS WE REQUESTED - MOVE GET PAST IT
  while (ns_count.Val)
  {
    //----- NAME [#] -----
    //(A domain name to which this resource record pertains)
    while (1)
    {
      udp->udp_read_next_rx_byte(&received_byte);
      
      if (received_byte == 0x00)
      {
        //Null - name is done
        break;
      }
      
      if (received_byte & 0xc0)
      {
        //Using a compressed name
        //Get the next byte of the name location offset and exit
        udp->udp_read_next_rx_byte(&received_byte);
        break;
      }
      else
      {
        //Using a normal name
        //Get all of the name characters
        label_length = received_byte;
        while (label_length)
        {
          udp->udp_read_next_rx_byte(&received_byte);
          label_length--;
        }
      }
    }
    
    //----- TYPE [2] -----
    //(Two bytes containing one of the RR type codes.  This field specifies the meaning of the data in the RDATA field)
    udp->udp_read_next_rx_byte(&received_byte);
    udp->udp_read_next_rx_byte(&received_byte);
    
    //----- CLASS [2] -----
    //(Two bytes which specify the class of the data in the RDATA field)
    udp->udp_read_next_rx_byte(&received_byte);
    udp->udp_read_next_rx_byte(&received_byte);
    
    //----- TTL [4] -----
    //(The time interval (in seconds) that the resource record may be cached before it should be discarded.  Zero values are
    //interpreted to mean that the RR can only be used for the transaction in progress, and should not be cached)
    udp->udp_read_next_rx_byte(&received_byte);
    udp->udp_read_next_rx_byte(&received_byte);
    udp->udp_read_next_rx_byte(&received_byte);
    udp->udp_read_next_rx_byte(&received_byte);
    
    //----- RDLENGTH [2] -----
    //(The length in bytes of the RDATA field)
    udp->udp_read_next_rx_byte(&received_byte);
    data_length = (received_byte << 8);
    udp->udp_read_next_rx_byte(&received_byte);
    data_length += received_byte;
    
    //----- RDATA [#] -----
    //(A variable length string of bytes that describes the resource.  The format of this information varies
    //according to the TYPE and CLASS of the resource record.  For example, the if the TYPE is A and the CLASS is IN,
    //the RDATA field is a 4 octet ARPA Internet address)
    while (data_length)
    {
      udp->udp_read_next_rx_byte(&received_byte);
      data_length--;
    }
    
    ns_count.Val--;
  }
  
  
  //----------------------
  //----- ADDITIONAL -----
  //----------------------
  // THIS SECTION MAY CONTAIN THE IP ADDRESS WE REQUESTED
  while (ar_count.Val)
  {
    response_is_good = 1;		//Default to good record
    
    //----- NAME [#] -----
    //(A domain name to which this resource record pertains)
    //As we're not buffering the entire packet to deal with the possible compression we don't bother to check this
    while (1)
    {
      udp->udp_read_next_rx_byte(&received_byte);
      
      if (received_byte == 0x00)
      {
        //Null - name is done
        break;
      }
      
      if (received_byte & 0xc0)
      {
        //Using a compressed name
        //Get the next byte of the name location offset and exit
        udp->udp_read_next_rx_byte(&received_byte);
        break;
      }
      else
      {
        //Using a normal name
        //Get all of the name characters
        label_length = received_byte;
        while (label_length)
        {
          udp->udp_read_next_rx_byte(&received_byte);
          label_length--;
        }
      }
    }
    
    //----- TYPE [2] -----
    //(Two bytes containing one of the RR type codes.  This field specifies the meaning of the data in the RDATA field)
    //Should be type 'A' = host address (value = 0x0001)
    udp->udp_read_next_rx_byte(&received_byte);		//Get and check high byte
    if (received_byte != 0x00)
      response_is_good = 0;
    
    udp->udp_read_next_rx_byte(&received_byte);		//Get and check low byte
    if (received_byte != 0x01)
      response_is_good = 0;
    
    //----- CLASS [2] -----
    //(Two bytes which specify the class of the data in the RDATA field)
    //Should be class 'IN' = internet (value = 0x0001)
    udp->udp_read_next_rx_byte(&received_byte);		//Get and check high byte
    if (received_byte != 0x00)
      response_is_good = 0;
    
    udp->udp_read_next_rx_byte(&received_byte);		//Get and check low byte
    if (received_byte != 0x01)
      response_is_good = 0;
    
    //----- TTL [4] -----
    //(The time interval (in seconds) that the resource record may be cached before it should be discarded.  Zero values are
    //interpreted to mean that the RR can only be used for the transaction in progress, and should not be cached)
    udp->udp_read_next_rx_byte(&received_byte);			//We don't care as we're not caching responses
    udp->udp_read_next_rx_byte(&received_byte);
    udp->udp_read_next_rx_byte(&received_byte);
    udp->udp_read_next_rx_byte(&received_byte);
    
    //----- RDLENGTH [2] -----
    //(The length in bytes of the RDATA field)
    //Should be 4
    udp->udp_read_next_rx_byte(&received_byte);		//Get and check high byte
    if (received_byte != 0x00)
      goto dns_check_response_failed;
    
    udp->udp_read_next_rx_byte(&received_byte);		//Get and check low byte
    label_length = received_byte;
    if (received_byte != 0x04)
      response_is_good = 0;
    
    //----- RDATA [#] -----
    //(A variable length string of bytes that describes the resource.  The format of this information varies
    //according to the TYPE and CLASS of the resource record.  For example, the if the TYPE is A and the CLASS is IN,
    //the RDATA field is a 4 octet ARPA Internet address)
    for (loop_count = 0; loop_count < label_length; loop_count++)
    {
      udp->udp_read_next_rx_byte(&received_byte);
      if (response_is_good)
        resolved_ip_address->v[loop_count] = received_byte;
    }
    
    if (response_is_good)
      goto dns_check_response_good;
    
    ar_count.Val--;
  }
  
  
  //------------------------
  //----- BAD RESPONSE -----
  //------------------------
dns_check_response_failed:
  udp->udp_dump_rx_packet();                             //We are done with this packet
  
  return (0);
  
  
  //-------------------------
  //----- GOOD RESPONSE -----
  //-------------------------
dns_check_response_good:
  udp->udp_dump_rx_packet();                             //We are done with this packet
  
  return (1);
}

//**********************************
//**********************************
//********** DO DNS QUERY **********
//**********************************
//**********************************
//url_pointer
//	= 'www.###.com' etc for a web address (non case sensitive)
//	= ###.com etc for a mail server mx address
//dns_type
//	= 1 for host (www lookup)
//	= 15 for mx mailserver (email lookup)
//Returns:
//	1 = DNS query started
//	0 = unable to start query currently - try again later
uint8_t DNS::Query (uint8_t *url_pointer, uint8_t dns_type)
{
  uint8_t count;
  
  //----- CHECK WE ARE CONNECTED -----
  if (!net->NicLinkedAndIpValid)
    return(0);
  
  //----- CHECK WE ARE NOT ALREADY DOING A QUERY -----
  if (dns_state != SM_DNS::SM_DNS_IDLE)
  {
    //We can't currently do a new dns query as DNS is already active or waiting on previous user to get the result
    return (0);
  }
  
  //----- START NEW DNS QUERY -----
  //Copy the name
  for (count = 0; count < DNS_MAX_URL_LENGTH; count++)
    dns_requested_url[count] = url_pointer[count];
  
  //Store the type
  dns_requested_qtype = dns_type;
  
  dns_state = SM_DNS::SM_DNS_WAITING_TO_SEND;			//The 'FSM' state machine will do the dns query
  return (1);
}
//****************************************
//****************************************
//********** CHECK DNS RESPONSE **********
//****************************************
//****************************************
//Returns IP address
//	0 				= dns not yet complete
//	0xFFFFFFFF		= dns failed1
//	Anything else	= IP address of requested URL
IP_ADDR DNS::CheckResponse (void)
{
  IP_ADDR return_ip;
  
  if (dns_state == SM_DNS::SM_DNS_SUCCESS)
  {
    //----- DNS IS COMPLETE AND WAS SUCESSFUL -----
    dns_state = SM_DNS::SM_DNS_IDLE;
    return (dns_resolved_ip_address);
  }
  else if ((dns_state == SM_DNS::SM_DNS_FAILED) || (dns_state == SM_DNS::SM_DNS_IDLE))
  {
    //----- DNS IS COMPLETE BUT FAILED -----
    //(Or has already been returned)
    dns_state = SM_DNS::SM_DNS_IDLE;
    dns_resolved_ip_address.Val = 0xffffffff;
    return (dns_resolved_ip_address);
  }
  
  //----- DNS IS NOT YET COMPLETE -----
  return_ip.Val = 0;
  return (return_ip);
}

//*********************************
//*********************************
//********** PROCESS DNS **********
//*********************************
//*********************************
//This function is called reguarly by tcp_ip_process_stack
void DNS::FSM (void)
{  
  
  ARP* arp = (ARP*)net->GetObject(StackObject::ARP);
  UDP* udp = (UDP*)net->GetObject(StackObject::UDP);
  
  //-----------------------------------
  //----- CHECK FOR UPDATE TIMERS -----
  //-----------------------------------
  if ((uint8_t)((uint8_t)(net->ethernet_10ms_clock_timer & 0x000000ff) - eth_dns_10ms_clock_timer_last) >=	10)
  {
    eth_dns_10ms_clock_timer_last = (uint8_t)(net->ethernet_10ms_clock_timer & 0x000000ff);
    
    //TIMEOUT TIMER
    if (dns_100ms_timeout_timer)
      dns_100ms_timeout_timer--;
  }
  
  
  
  switch(dns_state)
  {
  case SM_DNS::SM_DNS_IDLE:
    //--------------------------------------------
    //----- IDLE - NO DNS REQUEST IS PENDING -----
    //--------------------------------------------
    
    break;
    
    
  case SM_DNS::SM_DNS_WAITING_TO_SEND:
    //------------------------------------
    //----- A DNS REQUEST IS PENDING -----
    //------------------------------------
    
    //----- USE ARP TO GET THE MAC ADDRESS OF OUR GATEWAY DEVICE -----
    if (arp->ResolveIP(&net->GatewayIpAddress))
    {
      //REQUEST WAS SENT
      dns_100ms_timeout_timer = DNS_GATEWAY_DO_ARP_TIMEOUT_x100MS;
      
      dns_state = SM_DNS::SM_DNS_WAITING_FOR_ARP_RESPONSE;
      break;
    }
    //Request cannot be sent right now - try again next time
    break;
    
    
  case SM_DNS::SM_DNS_WAITING_FOR_ARP_RESPONSE:
    //-------------------------------------
    //----- WAIT FOR THE ARP RESPONSE -----
    //-------------------------------------
    //Check for timeout
    if (dns_100ms_timeout_timer == 0)
      goto FSM_failed;
    
    //Wait for the ARP response
    if (arp->ResolveIsComplete(&net->LocalIpAddress, &DNSServerNode.mac_address) )
    {
      //----- ARP RESPONSE RECEVIED FROM GATEWAY -----
      
      //DNSServerNode.MACAddr has been set in the function above
      
      //Set the IP address
      DNSServerNode.ip_address.Val = net->GatewayIpAddress.Val;
      
      //Send DNS request
      dns_state = SM_DNS::SM_DNS_SEND_REQUEST;
    }
    break;
    
    
  case SM_DNS::SM_DNS_SEND_REQUEST:
    //----------------------------
    //----- SEND DNS REQUEST -----
    //----------------------------
    //Check for timeout
    if (dns_100ms_timeout_timer == 0)
      goto FSM_failed;
    
    //Open a UDP socket
    dns_socket = udp->udp_open_socket (&DNSServerNode, DNS_CLIENT_PORT, DNS_SERVER_PORT);
    
    //Check a socket was allocated
    if (dns_socket == UDP_INVALID_SOCKET)
      break;							//No UDP socket currently available - try again next time
    
    //Start the TX
    if (udp->udp_setup_tx(dns_socket))
    {
      //TX setup - send the packet
      sendRequest(dns_requested_url, dns_requested_qtype);
      dns_100ms_timeout_timer = DNS_DO_REQUEST_TIMEOUT_x100MS;
      dns_state = SM_DNS::SM_DNS_WAITING_FOR_DNS_RESPONSE;
    }
    else
    {
      //Can't tx right now - close the socket ready to try again next time
      udp->udp_close_socket(&dns_socket);
    }
    break;
    
    
  case SM_DNS::SM_DNS_WAITING_FOR_DNS_RESPONSE:
    //----------------------------------------------------------------
    //----- A DNS REQUEST HAS BEEN SENT - WAITING FOR A RESPONSE -----
    //----------------------------------------------------------------
    //Check for timeout
    if (dns_100ms_timeout_timer == 0)
    {
      //TIMED OUT - DNS FAILED
      udp->udp_close_socket(&dns_socket);
      goto FSM_failed;
    }
    
    if (udp->udp_check_socket_for_rx(dns_socket))
    {
      if (checkResponse(dns_requested_url, dns_requested_qtype, &dns_resolved_ip_address))
      {
        //A VALID DNS RESPONSE HAS BEEN RECEIVED
        dns_state = SM_DNS::SM_DNS_SUCCESS;
      }
      else
      {
        //AN INVALID DNS RESPONSE HAS BEEN RECEIVED
        dns_state = SM_DNS::SM_DNS_FAILED;
      }
      udp->udp_close_socket(&dns_socket);				//Close the UDP socket
    }
    break;
    
    
  case SM_DNS::SM_DNS_SUCCESS:
    //------------------------------------------
    //----- THE LAST REQUEST WAS SUCESSFUL -----
    //------------------------------------------
    //We stay in this state until the calling function next cheks for the outcome
    break;
    
    
  case SM_DNS::SM_DNS_FAILED:
    //-----------------------------------
    //----- THE LAST REQUEST FAILED -----
    //-----------------------------------
    //We stay in this state until the calling function next cheks for the outcome
    udp->udp_close_socket(&dns_socket);				//Close the UDP socket   
    break;
    
  }
  
  //----------------
  //----- EXIT -----
  //----------------
  return;
  
  //----------------------
  //----- DNS FAILED -----
  //----------------------
FSM_failed:
  dns_state = SM_DNS::SM_DNS_FAILED;
  return;
}



