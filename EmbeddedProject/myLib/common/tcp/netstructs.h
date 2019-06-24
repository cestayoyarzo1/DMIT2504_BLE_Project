/*
  FILE: $FILE_FNAME$
  Created on: 9/1/2017, by Tom Diaz
*/
#ifndef NETSTRUCTS_H
#define NETSTRUCTS_H

#define	CONSTANT	const 					//Define used for this as some compilers require an additional qualifier such as 'rom' to signify that a constant should be stored in program memory

#undef BOOL
#undef TRUE
#undef FALSE
#undef BYTE
#undef SIGNED_BYTE
#undef WORD
#undef SIGNED_WORD
#undef DWORD
#undef SIGNED_DWORD

//BOOLEAN - 1 bit:
typedef enum _BOOL { FALSE = 0, TRUE } BOOL;
//BYTE - 8 bit unsigned:
typedef uint8_t BYTE;
//SIGNED_BYTE - 8 bit signed:
typedef int8_t SIGNED_BYTE;
//WORD - 16 bit unsigned:
typedef uint16_t WORD;
//SIGNED_WORD - 16 bit signed:
typedef int16_t SIGNED_WORD;
//DWORD - 32 bit unsigned:
typedef uint32_t DWORD;
//SIGNED_DWORD - 32 bit signed:
typedef int32_t SIGNED_DWORD;

//BYTE BIT ACCESS:
typedef union _BYTE_VAL
{
    struct
    {
        unsigned char b0:1;
        unsigned char b1:1;
        unsigned char b2:1;
        unsigned char b3:1;
        unsigned char b4:1;
        unsigned char b5:1;
        unsigned char b6:1;
        unsigned char b7:1;
    } bits;
    BYTE Val;
} BYTE_VAL;

//WORD ACCESS
typedef union _WORD_VAL
{
    WORD Val;
    struct
    {
        BYTE LSB;
        BYTE MSB;
    } byte;
    BYTE v[2];
} WORD_VAL;
#define LSB(a)          ((a).v[0])
#define MSB(a)          ((a).v[1])

//DWORD ACCESS:
typedef union _DWORD_VAL
{
    DWORD Val;
    struct
    {
        BYTE LOLSB;
        BYTE LOMSB;
        BYTE HILSB;
        BYTE HIMSB;
    } byte;
    struct
    {
        WORD LSW;
        WORD MSW;
    } word;
    BYTE v[4];
} DWORD_VAL;
#define LOWER_LSB(a)    ((a).v[0])
#define LOWER_MSB(a)    ((a).v[1])
#define UPPER_LSB(a)    ((a).v[2])
#define UPPER_MSB(a)    ((a).v[3])

//EXAMPLE OF HOW TO USE THE DATA TYPES:-
//	WORD_VAL variable_name;				//Define the variable
//	variable_name = 0xffffffff;			//Writing 32 bit value
//	variable_name.LSW = 0xffff;			//Writing 16 bit value to the lower word 
//	variable_name.LOLSB = 0xff;			//Writing 8 bit value to the low word least significant byte
//	variable_name.v[0] = 0xff;			//Writing 8 bit value to byte 0 (least significant byte)


//MAC ADDRESS:
typedef struct _MAC_ADDR
{
  uint8_t		v[6];
} MAC_ADDR;
#define		MAC_ADDR_LENGTH		6		//Use this define instead of sizeof to deal with compilers that pad this size to a 32 bit boundary (8 bytes)

//----- DATA TYPE DEFINITIONS -----
typedef struct _ETHERNET_HEADER
{
  MAC_ADDR		destination_mac_address;
  MAC_ADDR		source_mac_address;
  WORD_VAL		type;
} ETHERNET_HEADER;
#define	ETHERNET_HEADER_LENGTH		14		//Can't use sizeof as some compilers will add pad bytes within the struct


#define IP_PROTOCOL_ICMP                1
#define IP_PROTOCOL_TCP                 6
#define IP_PROTOCOL_UDP                 17

#define IP_VERSION                      0x04

#define MAX_IP_OPTIONS_LENGTH		20

#define IP_TYPE_OF_SERVICE_STD		0

#define	IP_DEFAULT_TIME_TO_LIVE		100

//----- ETHERNET PACKET DEFINITIONS -----
#define	ETHERNET_HARDWARE_TYPE			0x0001
#define	ETHERNET_TYPE_ARP			0x0806
#define ETHERNET_TYPE_IP			0x0800

// IP ADDRESS:
typedef union _IP_ADDR
{
  uint8_t        v[4];
  uint32_t       Val;
} IP_ADDR;
#define		IP_ADDR_LENGTH		4

// STATIC ADDRESS INFO
typedef struct _STATIC_ADDR
{
  _IP_ADDR LocalIP;
  _IP_ADDR LocalGateway;
  _IP_ADDR LocalSubnet;
} STATIC_ADDR;
#define		STATIC_ADDR_LENGTH      IP_ADDR_LENGTH * 3

// _IP_HEADER
typedef struct _IP_HEADER
{
  uint8_t		version_header_length;
  uint8_t		type_of_service;
  uint16_t		length;
  uint16_t		ident;
  uint16_t		flags;
  uint8_t		time_to_live;
  uint8_t		protocol;		//(ICMP=1, TCP=6, EGP=8, UDP=17)
  uint16_t		header_checksum;
  IP_ADDR		source_ip_address;
  IP_ADDR		destination_ip_address;
} IP_HEADER;
//Defined to avoid sizeof problemms with compilers that add padd bytes
#define	IP_HEADER_LENGTH			20

// REMOTE DEVICE INFO:
typedef struct _DEVICE_INFO
{
  MAC_ADDR    mac_address;
  IP_ADDR     ip_address;
} DEVICE_INFO;

#endif /* NETSTRUCTS_H */
