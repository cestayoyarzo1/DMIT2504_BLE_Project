/*
  FILE: enc28j60x_c.h
  Created on: 8/25/2017, by Tom Diaz
*/
#ifndef ENC28J6X_C_H
#define ENC28J6X_C_H

//##################################
//##################################
//########## USING DRIVER ##########
//##################################
//##################################

//Check this header file for defines to setup and any usage notes
//Configure the IO pins as requried in your applications initialisation.

//#########################
//##### SETUP SPI BUS #####
//#########################
//CLK needs to idle in low state
//The ENC28J60 clocks in data on the rising edge, and outputs data on the falling edge
//SPI bus clock frequency up to 20MHz (rising edge to next rising edge of clk)


//#########################
//##### PROVIDE DELAY #####
//#########################
//Ensure the nic_delay_1ms() will provide a 1mS delay

//###########################
//##### OPERATION NOTES #####
//###########################
//Since the ENC28J60 doesn't support auto-negotiation, full-duplex mode is not compatible with most switches/routers.  If a dedicated network is used 
//where the duplex of the remote node can be manually configured, you may change this configuration.  Otherwise, half duplex should always be used.

//For further information please see the project technical manual


//*****************************
//*****************************
//********** DEFINES **********
//*****************************
//*****************************
#ifndef NIC_C_INIT		//Do only once the first time this file is used
#define	NIC_C_INIT

//#include "eth-main.h"
#include <generic.h>

//----------------------------------------------
//----- DEFINE TARGET COMPILER & PROCESSOR -----
//----------------------------------------------


//------------------------
//----- USER OPTIONS -----
//------------------------

//----- ETHERNET SPEED TO USE -----
#define	NIC_INIT_SPEED				0			//1 = force speed to 10 Mbps (only option available for this IC)



#ifdef NIC_USING_MICROCHIP_C18_COMPILER
//########################################
//########################################
//##### USING MICROCHIP C18 COMPILER #####
//########################################
//########################################

//----------------------
//----- IO DEFINES -----
//----------------------
//SPI BUS CONTROL REGISTERS:-
//#define	NIC_SPI_BUF_FULL			SSP1STATbits.BF			//>0 when the SPI receive buffer contains a received byte, also signifying that transmit is complete
//#define	NIC_SPI_TX_BYTE(data)		SSP1BUF = data			//Macro to write a byte and start transmission over the SPI bus
//#define	NIC_SPI_RX_BYTE_BUFFER		SSP1BUF					//Register to read last received byte from
//##### Due to silicon bugs on some PIC18's the following alternative defines are needed as workarounds #####
#define	NIC_SPI_BUF_FULL			PIR1bits.SSPIF
#define	NIC_SPI_TX_BYTE(data)		PIR1bits.SSPIF = 0; SSP1BUF = data
#define	NIC_SPI_RX_BYTE_BUFFER		SSP1BUF

//CONTROL PINS:-
#define	NIC_CS(state)			LATDbits.LATD3 = state		//0 = select IC
#define	NIC_RESET_PIN(state)	LATDbits.LATD2 = state		//0 = reset IC

//INPUT PINS:-
#define	NIC_INT_PIN				PORTBbits.RB0				//This pin is used as a signal input rather than an interrupt, so does not need to be connected to
//an interrupt input on the microcontroller / processor if processing of the stack will simply be
//carried out constantly from the user application main loop

//---------------------------------------
//----- INITIALISATION DELAY DEFINE -----
//---------------------------------------
//Resetting the nic requires minimum setup time.  This define should cause a delay of at least 400nS
//#define	NIC_DELAY_400NS			Nop(); Nop(); Nop(); Nop();		//('Nop();' is a single cycle null instruction for the C18 compiler, include multiple times if required)

//###############################################
//###############################################
//##### END OF USING MICROCHIP C18 COMPILER #####
//###############################################
//###############################################
#endif		//#ifdef NIC_USING_MICROCHIP_C18_COMPILER



#ifdef NIC_USING_MICROCHIP_C30_COMPILER
//########################################
//########################################
//##### USING MICROCHIP C30 COMPILER #####
//########################################
//########################################

//----------------------
//----- IO DEFINES -----
//----------------------
//SPI BUS CONTROL REGISTERS:-
#define	NIC_SPI_BUF_FULL				(IFS0bits.SPI1IF && !_RF6) 		//>0 when the SPI receive buffer contains a received byte, also signifying that transmit is complete
//Checks clock pin for silicon bug workaround
#define	NIC_SPI_TX_BYTE(data)			IFS0bits.SPI1IF = 0; SPI1BUF = data			//Macro to write a byte and start transmission over the SPI bus
#define	NIC_SPI_RX_BYTE_BUFFER			SPI1BUF					//Register to read last received byte from

//CONTROL PINS:-
#define	NIC_CS(state)			_LATD14 = state		//0 = select IC
#define	NIC_RESET_PIN(state)	_LATD15 = state		//0 = reset IC

//INPUT PINS:-
#define	NIC_INT_PIN				_RE9						//This pin is used as a signal input rather than an interrupt, so does not need to be connected to
//an interrupt input on the microcontroller / processor if processing of the stack will simply be
//carried out constantly from the user application main loop

//---------------------------------------
//----- INITIALISATION DELAY DEFINE -----
//---------------------------------------
//Resetting the nic requires minimum setup time.  This define should cause a delay of at least 400nS
#define	NIC_DELAY_400NS			Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop();		//('Nop();' is a single cycle null instruction for the C30 compiler, include multiple times if required)


//###############################################
//###############################################
//##### END OF USING MICROCHIP C30 COMPILER #####
//###############################################
//###############################################
#endif		//#ifdef NIC_USING_MICROCHIP_C30_COMPILER


#ifdef NIC_USING_MICROCHIP_C32_COMPILER
//########################################
//########################################
//##### USING MICROCHIP C32 COMPILER #####
//########################################
//########################################

//----------------------
//----- IO DEFINES -----
//----------------------
//SPI BUS CONTROL REGISTERS:-
#define	NIC_SPI_BUF_FULL		        SpiChnGetRxIntFlag(1)		//>0 when the SPI receive buffer contains a received byte, also signifying that transmit is complete
#define	NIC_SPI_TX_BYTE(data)			spi->WriteByte(data)			//Macro to write a byte and start transmission over the SPI bus
#define	NIC_SPI_RX_BYTE_BUFFER			SpiChnGetC(1)				//Macro to read last received byte from

//CONTROL PINS:-
#define	NIC_CS(state)			(state ? cs->Set() : cs->Clear())		//0 = select IC
#define	NIC_RESET_PIN(state)	(state ? mPORTDSetBits(BIT_15) : mPORTDClearBits(BIT_15))		//0 = reset IC

//INPUT PINS:-
#define	NIC_INT_PIN				!intr->State()		//This pin is used as a signal input rather than an interrupt, so does not need to be connected to
//an interrupt input on the microcontroller / processor if processing of the stack will simply be
//carried out constantly from the user application main loop

//---------------------------------------
//----- INITIALISATION DELAY DEFINE -----
//---------------------------------------
//Resetting the nic requires minimum setup time.  This define should cause a delay of at least 400nS
#define	NIC_DELAY_400NS			Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop();
//('Nop();' is a single cycle null instruction for the C32 compiler, include multiple times if required)

//###############################################
//###############################################
//##### END OF USING MICROCHIP C32 COMPILER #####
//###############################################
//###############################################
#endif		//#ifdef NIC_USING_MICROCHIP_C32_COMPILER



//GENERAL DEFINES
#define	PROCESS_NIC_CALLS_BEFORE_DUMP_RX	5	//The number of times the CheckForRx may be called with a received packet waiting to be processed before it
//just gets dumped.  This is a backup in case an application function has opened a socket but doesn't process a
//received packet for that socket for some reason.

//NIC RAM DEFINITIONS
#define NIC_RAMSIZE				8192
/*
#define NIC_TXSTART 			(NIC_RAMSIZE - (1 + 1514 + 7))
//#define NIC_TXSTOP                              8191
#define NIC_RXSTART				0				//Should be an even memory address; must be 0 for silicon errata workaround
#define	NIC_RXSTOP				((NIC_TXSTART - 2) | 0x0001)	//Odd for errata workaround
#define NIC_RXSIZE				(NIC_RXSTOP - NIC_RXSTART + 1)
*/

#define NIC_RXSIZE         0x0BFF
#define NIC_RXSTART        0x0000  // start of RX buffer, (must be zero, Rev. B4 Errata point 5)
#define NIC_RXSTOP         0x0BFF  // end of RX buffer, room for 2 packets

#define NIC_TXSTART        0x0C00  // start of TX buffer, room for 1 packet
#define NIC_TXSTOP         0x11FF  // end of TX buffer


#define NIC_BASE_TX_ADDR		(NIC_TXSTART + 1)


//----- NIC REGISTER DEFINITIONS -----
//- BANK 0 -
#define NIC_ERDPTL				0x0000
#define NIC_ERDPTH				0x0001
#define NIC_EWRPTL				0x0002
#define NIC_EWRPTH				0x0003
#define NIC_ETXSTL				0x0004
#define NIC_ETXSTH				0x0005
#define NIC_ETXNDL				0x0006
#define NIC_ETXNDH				0x0007
#define NIC_ERXSTL				0x0008
#define NIC_ERXSTH				0x0009
#define NIC_ERXNDL				0x000a
#define NIC_ERXNDH				0x000b
#define NIC_ERXRDPTL			0x000c
#define NIC_ERXRDPTH			0x000d
#define NIC_ERXWRPTL			0x000e
#define NIC_ERXWRPTH			0x000f
#define NIC_EDMASTL				0x0010
#define NIC_EDMASTH				0x0011
#define NIC_EDMANDL				0x0012
#define NIC_EDMANDH				0x0013
#define NIC_EDMADSTL			0x0014
#define NIC_EDMADSTH			0x0015
#define NIC_EDMACSL				0x0016
#define NIC_EDMACSH				0x0017
#define NIC_EIE					0x001b
#define NIC_EIR					0x001c
#define NIC_ESTAT				0x001d
#define NIC_ECON2				0x001e
#define NIC_ECON1				0x001f

//- BANK 1 -
#define NIC_EHT0				0x0100
#define NIC_EHT1				0x0101
#define NIC_EHT2				0x0102
#define NIC_EHT3				0x0103
#define NIC_EHT4				0x0104
#define NIC_EHT5				0x0105
#define NIC_EHT6				0x0106
#define NIC_EHT7				0x0107
#define NIC_EPMM0				0x0108
#define NIC_EPMM1				0x0109
#define NIC_EPMM2				0x010a
#define NIC_EPMM3				0x010b
#define NIC_EPMM4				0x010c
#define NIC_EPMM5				0x010d
#define NIC_EPMM6				0x010e
#define NIC_EPMM7				0x010f
#define NIC_EPMCSL				0x0110
#define NIC_EPMCSH				0x0111
#define NIC_EPMOL				0x0114
#define NIC_EPMOH				0x0115
#define NIC_ERXFCON				0x0118
#define NIC_EPKTCNT				0x0119
//#define NIC_EIE				0x011b
//#define NIC_EIR				0x011c
//#define NIC_ESTAT				0x011d
//#define NIC_ECON2				0x011e
//#define NIC_ECON1				0x011f

//- BANK 2 -
#define NIC_MACON1				0x0200
#define NIC_MACON3				0x0202
#define NIC_MACON4				0x0203
#define NIC_MABBIPG				0x0204
#define NIC_MAIPGL				0x0206
#define NIC_MAIPGH				0x0207
#define NIC_MACLCON1			0x0208
#define NIC_MACLCON2			0x0209
#define NIC_MAMXFLL				0x020a
#define NIC_MAMXFLH				0x020b
#define NIC_MICMD				0x0212
#define NIC_MIREGADR			0x0214
#define NIC_MIWRL				0x0216
#define NIC_MIWRH				0x0217
#define NIC_MIRDL				0x0218
#define NIC_MIRDH				0x0219
//#define NIC_EIE				0x021b
//#define NIC_EIR				0x021c
//#define NIC_ESTAT				0x021d
//#define NIC_ECON2				0x021e
//#define NIC_ECON1				0x021f

//- BANK 3 -
#define NIC_MAADR5				0x0300
#define NIC_MAADR6				0x0301
#define NIC_MAADR3				0x0302
#define NIC_MAADR4				0x0303
#define NIC_MAADR1				0x0304
#define NIC_MAADR2				0x0305
#define NIC_EBSTSD				0x0306
#define NIC_EBSTCON				0x0307
#define NIC_EBSTCSL				0x0308
#define NIC_EBSTCSH				0x0309
#define NIC_MISTAT				0x030a
#define NIC_EREVID				0x0312
#define NIC_ECOCON				0x0315
#define NIC_EFLOCON				0x0317
#define NIC_EPAUSL				0x0318
#define NIC_EPAUSH				0x0319
//#define NIC_EIE				0x031b
//#define NIC_EIR				0x031c
//#define NIC_ESTAT				0x031d
//#define NIC_ECON2				0x031e
//#define NIC_ECON1				0x031f


//----- OPCODES -----
//(ORed with a 5 bit address)
#define	NIC_WCR (0x2<<5)			//Write Control Register
#define NIC_BFS (0x4<<5)			//Bit Field Set
#define	NIC_BFC (0x5<<5)			//Bit Field Clear
#define	NIC_RCR (0x0<<5)			//Read Control Register
#define NIC_RBM ((0x1<<5) | 0x1a)	//Read Buffer Memory
#define	NIC_WBM ((0x3<<5) | 0x1a) 	//Write Buffer Memory
#define	NIC_SR  ((0x7<<5) | 0x1f)	//System Reset command does not use an address.  


//----- REGISTER STRUCTURES TYPEDEF -----
typedef union _NIC_REG 
{
  uint8_t Val;
  
  // EIE bits ----------
  struct {
    uint8_t RXERIE:1;
    uint8_t TXERIE:1;
  uint8_t :1;
  uint8_t TXIE:1;
  uint8_t LINKIE:1;
  uint8_t DMAIE:1;
  uint8_t PKTIE:1;
  uint8_t INTIE:1;
  } EIEbits;
  
  // EIR bits ----------
  struct {
    uint8_t RXERIF:1;
    uint8_t TXERIF:1;
  uint8_t :1;
  uint8_t TXIF:1;
  uint8_t LINKIF:1;
  uint8_t DMAIF:1;
  uint8_t PKTIF:1;
  uint8_t :1;
  } EIRbits;
  
  // ESTAT bits ---------
  struct {
    uint8_t CLKRDY:1;
    uint8_t TXABRT:1;
    uint8_t RXBUSY:1;
  uint8_t :1;
  uint8_t LATECOL:1;
  uint8_t :1;
  uint8_t BUFER:1;
  uint8_t INT:1;
  } ESTATbits;
  
  // ECON2 bits --------
  struct {
  uint8_t :3;
  uint8_t VRPS:1;
  uint8_t :1;
  uint8_t PWRSV:1;
  uint8_t PKTDEC:1;
  uint8_t AUTOINC:1;
  } ECON2bits;
  
  // ECON1 bits --------
  struct {
    uint8_t BSEL0:1;
    uint8_t BSEL1:1;
    uint8_t RXEN:1;
    uint8_t TXRTS:1;
    uint8_t CSUMEN:1;
    uint8_t DMAST:1;
    uint8_t RXRST:1;
    uint8_t TXRST:1;
  } ECON1bits;
  
  // ERXFCON bits ------
  struct {
    uint8_t BCEN:1;
    uint8_t MCEN:1;
    uint8_t HTEN:1;
    uint8_t MPEN:1;
    uint8_t PMEN:1;
    uint8_t CRCEN:1;
    uint8_t ANDOR:1;
    uint8_t UCEN:1;
  } ERXFCONbits;
  
  // MACON1 bits --------
  struct {
    uint8_t MARXEN:1;
    uint8_t PASSALL:1;
    uint8_t RXPAUS:1;
    uint8_t TXPAUS:1;
  uint8_t :4;
  } MACON1bits;
  
  
  // MACON3 bits --------
  struct {
    uint8_t FULDPX:1;
    uint8_t FRMLNEN:1;
    uint8_t HFRMEN:1;
    uint8_t PHDREN:1;
    uint8_t TXCRCEN:1;
    uint8_t PADCFG0:1;
    uint8_t PADCFG1:1;
    uint8_t PADCFG2:1;
  } MACON3bits;
  struct {
    uint8_t FULDPX:1;
    uint8_t FRMLNEN:1;
    uint8_t HFRMEN:1;
    uint8_t PHDREN:1;
    uint8_t TXCRCEN:1;
    uint8_t PADCFG:3;
  } MACON3bits2;
  
  // MACON4 bits --------
  struct {
  uint8_t :4;
  uint8_t NOBKOFF:1;
  uint8_t BPEN:1;
  uint8_t DEFER:1;
  uint8_t :1;
  } MACON4bits;
  
  // MICMD bits ---------
  struct {
    uint8_t MIIRD:1;
    uint8_t MIISCAN:1;
  uint8_t :6;
  } MICMDbits;
  
  // EBSTCON bits -----
  struct {
    uint8_t BISTST:1;
    uint8_t TME:1;
    uint8_t TMSEL0:1;
    uint8_t TMSEL1:1;
    uint8_t PSEL:1;
    uint8_t PSV0:1;
    uint8_t PSV1:1;
    uint8_t PSV2:1;
  } EBSTCONbits;
  struct {
    uint8_t BISTST:1;
    uint8_t TME:1;
    uint8_t TMSEL:2;
    uint8_t PSEL:1;
    uint8_t PSV:3;
  } EBSTCONbits2;
  
  // MISTAT bits --------
  struct {
    uint8_t BUSY:1;
    uint8_t SCAN:1;
    uint8_t NVALID:1;
  uint8_t :5;
  } MISTATbits;
  
  // ECOCON bits -------
  struct {
    uint8_t COCON0:1;
    uint8_t COCON1:1;
    uint8_t COCON2:1;
  uint8_t :5;
  } ECOCONbits;
  struct {
    uint8_t COCON:3;
  uint8_t :5;
  } ECOCONbits2;
  
  // EFLOCON bits -----
  struct {
    uint8_t FCEN0:1;
    uint8_t FCEN1:1;
    uint8_t FULDPXS:1;
  uint8_t :5;
  } EFLOCONbits;
  struct {
    uint8_t FCEN:2;
    uint8_t FULDPXS:1;
  uint8_t :5;
  } EFLOCONbits2;
} NIC_REG;



//----- PH Register Locations -----
#define NIC_PHCON1	0x00
#define NIC_PHSTAT1	0x01
#define NIC_PHID1	0x02
#define NIC_PHID2	0x03
#define NIC_PHCON2	0x10
#define NIC_PHSTAT2	0x11
#define NIC_PHIE	0x12
#define NIC_PHIR	0x13
#define NIC_PHLCON	0x14


typedef union {
  uint16_t val;
  WORD_VAL VAL;
  
  //PHCON1 bits
  struct {
  uint16_t :8;
  uint16_t PDPXMD:1;
  uint16_t :2;
  uint16_t PPWRSV:1;
  uint16_t :2;
  uint16_t PLOOPBK:1;
  uint16_t PRST:1;
  } PHCON1bits;
  
  //PHSTAT1 bits
  struct {
  uint16_t :1;
  uint16_t JBSTAT:1;
  uint16_t LLSTAT:1;
  uint16_t :5;
  uint16_t :3;
  uint16_t PHDPX:1;
  uint16_t PFDPX:1;
  uint16_t :3;
  } PHSTAT1bits;
  
  //PHID2 bits
  struct {
    uint16_t PREV0:1;
    uint16_t PREV1:1;
    uint16_t PREV2:1;
    uint16_t PREV3:1;
    uint16_t PPN0:1;
    uint16_t PPN1:1;
    uint16_t PPN2:1;
    uint16_t PPN3:1;
    uint16_t PPN4:1;
    uint16_t PPN5:1;
    uint16_t PID19:1;
    uint16_t PID20:1;
    uint16_t PID21:1;
    uint16_t PID22:1;
    uint16_t PID23:1;
    uint16_t PID24:1;
  } PHID2bits;
  struct {
    uint16_t PREV:4;
    uint16_t PPNL:4;
    uint16_t PPNH:2;
    uint16_t PID:6;
  } PHID2bits2;
  
  //PHCON2 bits
  struct {
  uint16_t :8;
  uint16_t HDLDIS:1;
  uint16_t :1;
  uint16_t JABBER:1;
  uint16_t :2;
  uint16_t TXDIS:1;
  uint16_t FRCLNK:1;
  uint16_t :1;
  } PHCON2bits;
  
  //PHSTAT2 bits
  struct {
  uint16_t :5;
  uint16_t PLRITY:1;
  uint16_t :2;
  uint16_t :1;
  uint16_t DPXSTAT:1;
  uint16_t LSTAT:1;
  uint16_t COLSTAT:1;
  uint16_t RXSTAT:1;
  uint16_t TXSTAT:1;
  uint16_t :2;
  } PHSTAT2bits;
  
  //PHIE bits
  struct {
  uint16_t :1;
  uint16_t PGEIE:1;
  uint16_t :2;
  uint16_t PLNKIE:1;
  uint16_t :3;
  uint16_t :8;
  } PHIEbits;
  
  //PHIR bits
  struct {
  uint16_t :2;
  uint16_t PGIF:1;
  uint16_t :1;
  uint16_t PLNKIF:1;
  uint16_t :3;
  uint16_t :8;
  } PHIRbits;
  
  //PHLCON bits
  struct {
  uint16_t :1;
  uint16_t STRCH:1;
  uint16_t LFRQ0:1;
  uint16_t LFRQ1:1;
  uint16_t LBCFG0:1;
  uint16_t LBCFG1:1;
  uint16_t LBCFG2:1;
  uint16_t LBCFG3:1;
  uint16_t LACFG0:1;
  uint16_t LACFG1:1;
  uint16_t LACFG2:1;
  uint16_t LACFG3:1;
  uint16_t :4;
  } PHLCONbits;
  struct {
  uint16_t :1;
  uint16_t STRCH:1;
  uint16_t LFRQ:2;
  uint16_t LBCFG:4;
  uint16_t LACFG:4;
  uint16_t :4;
  } PHLCONbits2;
} PHYREG;


//----- Individual Register Bits -----
// ETH/MAC/MII bits

//EIE bits
#define	EIE_INTIE		(1<<7)
#define	EIE_PKTIE		(1<<6)
#define	EIE_DMAIE		(1<<5)
#define	EIE_LINKIE		(1<<4)
#define	EIE_TXIE		(1<<3)
#define	EIE_TXERIE		(1<<1)
#define	EIE_RXERIE		(1)

//EIR bits
#define	EIR_PKTIF		(1<<6)
#define	EIR_DMAIF		(1<<5)
#define	EIR_LINKIF		(1<<4)
#define	EIR_TXIF		(1<<3)
#define	EIR_TXERIF		(1<<1)
#define	EIR_RXERIF		(1)

//ESTAT bits
#define	ESTAT_INT		(1<<7)
#define ESTAT_BUFER		(1<<6)
#define	ESTAT_LATECOL	(1<<4)
#define	ESTAT_RXBUSY	(1<<2)
#define	ESTAT_TXABRT	(1<<1)
#define	ESTAT_CLKRDY	(1)

//ECON2 bits
#define	ECON2_AUTOINC	(1<<7)
#define	ECON2_PKTDEC	(1<<6)
#define	ECON2_PWRSV		(1<<5)
#define	ECON2_VRPS		(1<<3)

//ECON1 bits
#define	ECON1_TXRST		(1<<7)
#define	ECON1_RXRST		(1<<6)
#define	ECON1_DMAST		(1<<5)
#define	ECON1_CSUMEN	(1<<4)
#define	ECON1_TXRTS		(1<<3)
#define	ECON1_RXEN		(1<<2)
#define	ECON1_BSEL1		(1<<1)
#define	ECON1_BSEL0		(1)

//ERXFCON bits
#define	ERXFCON_UCEN	(1<<7)
#define	ERXFCON_ANDOR	(1<<6)
#define	ERXFCON_CRCEN	(1<<5)
#define	ERXFCON_PMEN	(1<<4)
#define	ERXFCON_MPEN	(1<<3)
#define	ERXFCON_HTEN	(1<<2)
#define	ERXFCON_MCEN	(1<<1)
#define	ERXFCON_BCEN	(1)

//MACON1 bits
#define	MACON1_TXPAUS	(1<<3)
#define	MACON1_RXPAUS	(1<<2)
#define	MACON1_PASSALL	(1<<1)
#define	MACON1_MARXEN	(1)

//MACON3 bits
#define	MACON3_PADCFG2	(1<<7)
#define	MACON3_PADCFG1	(1<<6)
#define	MACON3_PADCFG0	(1<<5)
#define	MACON3_TXCRCEN	(1<<4)
#define	MACON3_PHDREN	(1<<3)
#define	MACON3_HFRMEN	(1<<2)
#define	MACON3_FRMLNEN	(1<<1)
#define	MACON3_FULDPX	(1)

//MACON4 bits
#define	MACON4_DEFER	(1<<6)
#define	MACON4_BPEN		(1<<5)
#define	MACON4_NOBKOFF	(1<<4)

//MICMD bits
#define	MICMD_MIISCAN	(1<<1)
#define	MICMD_MIIRD		(1)

//EBSTCON bits
#define	EBSTCON_PSV2	(1<<7)
#define	EBSTCON_PSV1	(1<<6)
#define	EBSTCON_PSV0	(1<<5)
#define	EBSTCON_PSEL	(1<<4)
#define	EBSTCON_TMSEL1	(1<<3)
#define	EBSTCON_TMSEL0	(1<<2)
#define	EBSTCON_TME		(1<<1)
#define	EBSTCON_BISTST	(1)

//MISTAT bits
#define	MISTAT_NVALID	(1<<2)
#define	MISTAT_SCAN		(1<<1)
#define	MISTAT_BUSY		(1)

//ECOCON bits
#define	ECOCON_COCON2	(1<<2)
#define	ECOCON_COCON1	(1<<1)
#define	ECOCON_COCON0	(1)

//EFLOCON bits
#define	EFLOCON_FULDPXS	(1<<2)
#define	EFLOCON_FCEN1	(1<<1)
#define	EFLOCON_FCEN0	(1)


//----- PHY bits -----
//PHCON1 bits
#define	PHCON1_PRST		((WORD)1<<15)
#define	PHCON1_PLOOPBK	((WORD)1<<14)
#define	PHCON1_PPWRSV	((WORD)1<<11)
#define	PHCON1_PDPXMD	((WORD)1<<8)

//PHSTAT1 bits
#define	PHSTAT1_PFDPX	((WORD)1<<12)
#define	PHSTAT1_PHDPX	((WORD)1<<11)
#define	PHSTAT1_LLSTAT	((WORD)1<<2)
#define	PHSTAT1_JBSTAT	((WORD)1<<1)

//PHID2 bits
#define	PHID2_PID24		((WORD)1<<15)
#define	PHID2_PID23		((WORD)1<<14)
#define	PHID2_PID22		((WORD)1<<13)
#define	PHID2_PID21		((WORD)1<<12)
#define	PHID2_PID20		((WORD)1<<11)
#define	PHID2_PID19		((WORD)1<<10)
#define	PHID2_PPN5		((WORD)1<<9)
#define	PHID2_PPN4		((WORD)1<<8)
#define	PHID2_PPN3		((WORD)1<<7)
#define	PHID2_PPN2		((WORD)1<<6)
#define	PHID2_PPN1		((WORD)1<<5)
#define	PHID2_PPN0		((WORD)1<<4)
#define	PHID2_PREV3		((WORD)1<<3)
#define	PHID2_PREV2		((WORD)1<<2)
#define	PHID2_PREV1		((WORD)1<<1)
#define	PHID2_PREV0		((WORD)1)

//PHCON2 bits
#define	PHCON2_FRCLNK	((WORD)1<<14)
#define	PHCON2_TXDIS	((WORD)1<<13)
#define	PHCON2_JABBER	((WORD)1<<10)
#define	PHCON2_HDLDIS	((WORD)1<<8)

//PHSTAT2 bits
#define	PHSTAT2_TXSTAT	((WORD)1<<13)
#define	PHSTAT2_RXSTAT	((WORD)1<<12)
#define	PHSTAT2_COLSTAT	((WORD)1<<11)
#define	PHSTAT2_LSTAT	((WORD)1<<10)
#define	PHSTAT2_DPXSTAT	((WORD)1<<9)
#define	PHSTAT2_PLRITY	((WORD)1<<5)

//PHIE bits
#define	PHIE_PLNKIE		((WORD)1<<4)
#define	PHIE_PGEIE		((WORD)1<<1)

//PHIR bits
#define	PHIR_PLNKIF		((WORD)1<<4)
#define	PHIR_PGIF		((WORD)1<<2)

//PHLCON bits
#define	PHLCON_LACFG3	((WORD)1<<11)
#define	PHLCON_LACFG2	((WORD)1<<10)
#define	PHLCON_LACFG1	((WORD)1<<9)
#define	PHLCON_LACFG0	((WORD)1<<8)
#define	PHLCON_LBCFG3	((WORD)1<<7)
#define	PHLCON_LBCFG2	((WORD)1<<6)
#define	PHLCON_LBCFG1	((WORD)1<<5)
#define	PHLCON_LBCFG0	((WORD)1<<4)
#define	PHLCON_LFRQ1	((WORD)1<<3)
#define	PHLCON_LFRQ0	((WORD)1<<2)
#define	PHLCON_STRCH	((WORD)1<<1)


typedef union {
  uint8_t v[7];
  struct {
    uint16_t	ByteCount;
    uint8_t	CollisionCount:4;
    uint8_t	CRCError:1;
    uint8_t	LengthCheckError:1;
    uint8_t	LengthOutOfRange:1;
    uint8_t	Done:1;
    uint8_t	Multicast:1;
    uint8_t	Broadcast:1;
    uint8_t	PacketDefer:1;
    uint8_t	ExcessiveDefer:1;
    uint8_t	MaximumCollisions:1;
    uint8_t	LateCollision:1;
    uint8_t	Giant:1;
    uint8_t	Underrun:1;
    uint16_t 	BytesTransmittedOnWire;
    uint8_t	ControlFrame:1;
    uint8_t	PAUSEControlFrame:1;
    uint8_t	BackpressureApplied:1;
    uint8_t	VLANTaggedFrame:1;
    uint8_t	Zeros:4;
  } bits;
} TXSTATUS;

typedef union {
  uint8_t v[4];
  struct {
    uint16_t	ByteCount;
    uint8_t	PreviouslyIgnored:1;
    uint8_t	RXDCPreviouslySeen:1;
    uint8_t	CarrierPreviouslySeen:1;
    uint8_t	CodeViolation:1;
    uint8_t	CRCError:1;
    uint8_t	LengthCheckError:1;
    uint8_t	LengthOutOfRange:1;
    uint8_t	ReceiveOk:1;
    uint8_t	Multicast:1;
    uint8_t	Broadcast:1;
    uint8_t	DribbleNibble:1;
    uint8_t	ControlFrame:1;
    uint8_t	PauseControlFrame:1;
    uint8_t	UnsupportedOpcode:1;
    uint8_t	VLANType:1;
    uint8_t	Zero:1;
  } bits;
} RXSTATUS;


//----- NIC IS TX READY STATE MACHINE -----
typedef enum _NIC_IS_TX_READY_STATE
{
  SM_NIC_IS_TX_RDY_IDLE,
  SM_NIC_IS_TX_RDY_ALLOCATED
} NIC_IS_TX_READY_STATE;

/*
//----- DATA TYPE DEFINITIONS -----
typedef struct _ETHERNET_HEADER
{
  MAC_ADDR		destination_mac_address;
  MAC_ADDR		source_mac_address;
  WORD_VAL		type;
} ETHERNET_HEADER;
#define	ETHERNET_HEADER_LENGTH		14		//Can't use sizeof as some compilers will add pad bytes within the struct
*/
typedef struct _ENC_PREAMBLE
{
  uint16_t	        next_packet_pointer;
  RXSTATUS		status_vector;
} ENC_PREAMBLE;


#endif		//NIC_C_INIT

#endif /* ENC28J6X_C_H */
