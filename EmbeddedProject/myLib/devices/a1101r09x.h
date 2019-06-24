/*
  FILE: $FILE_FNAME$
  Created on: 6/30/2017, by Tom Diaz
*/

#ifndef A1101R09x_H
#define A1101R09x_H

#include <x_spiport.h>
#include <x_gpio.h>

#define PACKET_BUFFER_SIZE 72
#define MAX_PAYLOAD_SIZE 58
#define HEADER_BYTES 2

class CC1101Config
{
public:
  CC1101Config()
  {
    R00_IOCFG2 = 0x06;  // Asserts on Sync sent deasserts in packet complete
    R01_IOCFG1 = 0x0C;  // SPI MISO
    R02_IOCFG0 = 0x63;  // CLK_XOSC / 192
    R03_FIFOTHR = 0x07; // 0db Rx Attenuation, 33 Bytes Tx / 32 Bytes Rx Fifo 
    R04_SYNC1 = 0xD3;   // Datasheet default
    R05_SYNC0 = 0x91;   // Datasheet default
    R06_PKTLEN = 0xFF;  // Datasheet default
    R07_PKTCTRL1 = 0x00;// No auto flush, no status append, No address check
    R08_PKTCTRL0 = 0x05;// CRC enabled, variable paclket length
    R09_ADDR = 0x01;    // Datasheet default
    R0A_CHANNR = 0x00;  // Datasheet default 
    R0B_FSCTRL1 = 0x0F; // Datasheet default
    R0C_FSCTRL0 = 0x00; // Datasheet default 
    R0D_FREQ2 = 0x23;   // Default working, calculated
    R0E_FREQ1 = 0x31;   // Default working, calculated
    R0F_FREQ0 = 0x3B;   // Default working, calculated
    R10_MDMCFG4 = 0x2B;
    R11_MDMCFG3 = 0x22;
    R12_MDMCFG2 = 0x73;
    R13_MDMCFG1 = 0x62;
    R14_MDMCFG0 = 0xF8;
    R15_DEVIATN = 0x00;
    R16_MCSM2 = 0x07;
    R17_MCSM1 = 0x00;
    R18_MCSM0 = 0x18;
    R19_FOCCFG = 0x1D;
    R1A_BSCFG = 0x1C;
    R1B_AGCCTRL2 = 0xC7;
    R1C_AGCCTRL1 = 0x00;
    R1D_AGCCTRL0 = 0xB2;
    R1E_WOREVT1 = 0x87;
    R1F_WOREVT0 = 0x6B;
    R20_WORCTRL = 0xF8;
    R21_FREND1 = 0xB6;
    R22_FREND0 = 0x10;
    R23_FSCAL3 = 0xEA;
    R24_FSCAL2 = 0x0A;
    R25_FSCAL1 = 0x00;
    R26_FSCAL0 = 0x11;
    R27_RCCTRL1 = 0x41;
    R28_RCCTRL0 = 0x00;
    R29_FSTEST = 0x59;
    R2A_PTEST = 0x7F;
    R2B_AGCTEST = 0x3F;
    R2C_TEST2 = 0x88;
    R2D_TEST1 = 0x31;
    R2E_TEST0 = 0x0B;
  }	
  uint8_t R00_IOCFG2;		// GDO2 output pin configuration
  uint8_t R01_IOCFG1;		// GDO1 output pin configuration
  uint8_t R02_IOCFG0;		// GDO0 output pin configuration
  uint8_t R03_FIFOTHR;		// RX FIFO and TX FIFO thresholds
  uint8_t R04_SYNC1;		// Sync word, high byte
  uint8_t R05_SYNC0;		// Sync word, low byte
  uint8_t R06_PKTLEN;		// Packet length
  uint8_t R07_PKTCTRL1;		// Packet automation control
  uint8_t R08_PKTCTRL0;		// Packet automation control
  uint8_t R09_ADDR;		// Device address
  uint8_t R0A_CHANNR;		// Channel number
  uint8_t R0B_FSCTRL1;		// Frequency synthesizer control
  uint8_t R0C_FSCTRL0;		// Frequency synthesizer control
  uint8_t R0D_FREQ2;		// Frequency control word, high byte
  uint8_t R0E_FREQ1;		// Frequency control word, middle byte
  uint8_t R0F_FREQ0;		// Frequency control word, low byte
  uint8_t R10_MDMCFG4;		// Modem configuration
  uint8_t R11_MDMCFG3;		// Modem configuration
  uint8_t R12_MDMCFG2;		// Modem configuration
  uint8_t R13_MDMCFG1;		// Modem configuration
  uint8_t R14_MDMCFG0;		// Modem configuration
  uint8_t R15_DEVIATN;		// Modem deviation setting
  uint8_t R16_MCSM2;		// Main Radio Cntrl State Machine config
  uint8_t R17_MCSM1;		// Main Radio Cntrl State Machine config
  uint8_t R18_MCSM0;		// Main Radio Cntrl State Machine config
  uint8_t R19_FOCCFG;		// Frequency Offset Compensation config
  uint8_t R1A_BSCFG;		// Bit Synchronization configuration
  uint8_t R1B_AGCCTRL2;		// AGC control
  uint8_t R1C_AGCCTRL1;		// AGC control
  uint8_t R1D_AGCCTRL0;		// AGC control
  uint8_t R1E_WOREVT1;		// High byte Event 0 timeout
  uint8_t R1F_WOREVT0;		// Low byte Event 0 timeout
  uint8_t R20_WORCTRL;		// Wake On Radio control
  uint8_t R21_FREND1;		// Front end RX configuration
  uint8_t R22_FREND0;		// Front end TX configuration
  uint8_t R23_FSCAL3;		// Frequency synthesizer calibration
  uint8_t R24_FSCAL2;		// Frequency synthesizer calibration
  uint8_t R25_FSCAL1;		// Frequency synthesizer calibration
  uint8_t R26_FSCAL0;		// Frequency synthesizer calibration
  uint8_t R27_RCCTRL1;		// RC oscillator configuration
  uint8_t R28_RCCTRL0;		// RC oscillator configuration
  uint8_t R29_FSTEST;		// Frequency synthesizer cal control
  uint8_t R2A_PTEST;		// Production test
  uint8_t R2B_AGCTEST;		// AGC test
  uint8_t R2C_TEST2;		// Various test settings
  uint8_t R2D_TEST1;		// Various test settings
  uint8_t R2E_TEST0;		// Various test settings
};

class MarcState
{
public:
  enum Value
  {
    SLEEP,
    IDLE,
    XOFF,
    VCOON_MC,
    REGON_MC,
    MANCAL,
    VCOON,
    REGON,
    STARTCAL,
    BWBOOST,
    FS_LOCK,
    IFDACON,
    ENDCAL,
    RX,
    RX_END,
    RX_RST,
    TXRX_SWITCH,
    RXFIFO_OVERFLOW,
    FSTXON,
    TX,
    TX_END,
    RXTX_SWITCH,
    TXFIFO_UNDERFLOW,
    UNDEFINED
  };
  static Value Validate(uint8_t val)
  {
    val &= 0x1f;
    if(val < UNDEFINED)
      return (Value)val;
    else
      return UNDEFINED;
  }
};

class A1101R09x
{
public:
  A1101R09x(){ };
  ~A1101R09x(){};
  void Init(x_SPIPort* spi_, x_GPIO* cs_, x_GPIO* led_, x_Callback* cb_);
  bool SendPacket(uint8_t* msg, uint16_t len);
  void SendPacket(BufferHandler* b);
  void ISR();
  BufferHandler Buffer;
  enum class State
  {
    Init,
    Idle,
    Transmitting,
    TxWait,
    TxEnded,
    Receiving,
    RxEnded,
    Exception,
  } FsmState;
  void FSM();
protected:
  bool receivePacket();
  void reset();
  void tx();
  bool unpack();
  void writeStrobe(uint8_t strobe);
  bool writeReg(uint8_t reg, uint8_t val);
  uint8_t readReg(uint8_t reg);
  void configure(CC1101Config* c);
  void writeBurst(uint8_t addr, uint8_t* buff, uint16_t len);
  void readBurst(uint8_t addr, BufferHandler* b, uint16_t len);
  uint8_t readStatus(uint8_t addr);
  uint8_t buffer[COM_PORT_RX_BUFFER_SIZE];
  uint8_t pbuffer[PACKET_BUFFER_SIZE];
  bool isChannelClear();
  void calcTxPackets(uint16_t len);
  BufferHandler pb; // Packet Buffer
  CC1101Config defaults;
  x_GPIO* cs;
  //x_GPIO* l;
  GPIOEx led;
  x_SPIPort* spi;
  x_Callback* cb;
  uint8_t version;
  uint8_t partNo;
  uint8_t txPackets;
  uint8_t txPacketNo;
  MarcState::Value marcState;
  bool packetReady;
private:
};

//----------------------------------------------------------------------------TI
#define TI_CC_RF_FREQ 915
// Configuration Registers
#define TI_CCxxx0_IOCFG2       0x00        // GDO2 output pin configuration
#define TI_CCxxx0_IOCFG1       0x01        // GDO1 output pin configuration
#define TI_CCxxx0_IOCFG0       0x02        // GDO0 output pin configuration
#define TI_CCxxx0_FIFOTHR      0x03        // RX FIFO and TX FIFO thresholds
#define TI_CCxxx0_SYNC1        0x04        // Sync word, high byte
#define TI_CCxxx0_SYNC0        0x05        // Sync word, low byte
#define TI_CCxxx0_PKTLEN       0x06        // Packet length
#define TI_CCxxx0_PKTCTRL1     0x07        // Packet automation control
#define TI_CCxxx0_PKTCTRL0     0x08        // Packet automation control
#define TI_CCxxx0_ADDR         0x09        // Device address
#define TI_CCxxx0_CHANNR       0x0A        // Channel number
#define TI_CCxxx0_FSCTRL1      0x0B        // Frequency synthesizer control
#define TI_CCxxx0_FSCTRL0      0x0C        // Frequency synthesizer control
#define TI_CCxxx0_FREQ2        0x0D        // Frequency control word, high byte
#define TI_CCxxx0_FREQ1        0x0E        // Frequency control word, middle byte
#define TI_CCxxx0_FREQ0        0x0F        // Frequency control word, low byte
#define TI_CCxxx0_MDMCFG4      0x10        // Modem configuration
#define TI_CCxxx0_MDMCFG3      0x11        // Modem configuration
#define TI_CCxxx0_MDMCFG2      0x12        // Modem configuration
#define TI_CCxxx0_MDMCFG1      0x13        // Modem configuration
#define TI_CCxxx0_MDMCFG0      0x14        // Modem configuration
#define TI_CCxxx0_DEVIATN      0x15        // Modem deviation setting
#define TI_CCxxx0_MCSM2        0x16        // Main Radio Cntrl State Machine config
#define TI_CCxxx0_MCSM1        0x17        // Main Radio Cntrl State Machine config
#define TI_CCxxx0_MCSM0        0x18        // Main Radio Cntrl State Machine config
#define TI_CCxxx0_FOCCFG       0x19        // Frequency Offset Compensation config
#define TI_CCxxx0_BSCFG        0x1A        // Bit Synchronization configuration
#define TI_CCxxx0_AGCCTRL2     0x1B        // AGC control
#define TI_CCxxx0_AGCCTRL1     0x1C        // AGC control
#define TI_CCxxx0_AGCCTRL0     0x1D        // AGC control
#define TI_CCxxx0_WOREVT1      0x1E        // High byte Event 0 timeout
#define TI_CCxxx0_WOREVT0      0x1F        // Low byte Event 0 timeout
#define TI_CCxxx0_WORCTRL      0x20        // Wake On Radio control
#define TI_CCxxx0_FREND1       0x21        // Front end RX configuration
#define TI_CCxxx0_FREND0       0x22        // Front end TX configuration
#define TI_CCxxx0_FSCAL3       0x23        // Frequency synthesizer calibration
#define TI_CCxxx0_FSCAL2       0x24        // Frequency synthesizer calibration
#define TI_CCxxx0_FSCAL1       0x25        // Frequency synthesizer calibration
#define TI_CCxxx0_FSCAL0       0x26        // Frequency synthesizer calibration
#define TI_CCxxx0_RCCTRL1      0x27        // RC oscillator configuration
#define TI_CCxxx0_RCCTRL0      0x28        // RC oscillator configuration
#define TI_CCxxx0_FSTEST       0x29        // Frequency synthesizer cal control
#define TI_CCxxx0_PTEST        0x2A        // Production test
#define TI_CCxxx0_AGCTEST      0x2B        // AGC test
#define TI_CCxxx0_TEST2        0x2C        // Various test settings
#define TI_CCxxx0_TEST1        0x2D        // Various test settings
#define TI_CCxxx0_TEST0        0x2E        // Various test settings

// Strobe commands
#define TI_CCxxx0_SRES         0x30        // Reset chip.
#define TI_CCxxx0_SFSTXON      0x31        // Enable/calibrate freq synthesizer
#define TI_CCxxx0_SXOFF        0x32        // Turn off crystal oscillator.
#define TI_CCxxx0_SCAL         0x33        // Calibrate freq synthesizer & disable
#define TI_CCxxx0_SRX          0x34        // Enable RX.
#define TI_CCxxx0_STX          0x35        // Enable TX.
#define TI_CCxxx0_SIDLE        0x36        // Exit RX / TX
#define TI_CCxxx0_SAFC         0x37        // AFC adjustment of freq synthesizer
#define TI_CCxxx0_SWOR         0x38        // Start automatic RX polling sequence
#define TI_CCxxx0_SPWD         0x39        // Enter pwr down mode when CSn goes hi
#define TI_CCxxx0_SFRX         0x3A        // Flush the RX FIFO buffer.
#define TI_CCxxx0_SFTX         0x3B        // Flush the TX FIFO buffer.
#define TI_CCxxx0_SWORRST      0x3C        // Reset real time clock.
#define TI_CCxxx0_SNOP         0x3D        // No operation.

// Status registers
#define TI_CCxxx0_PARTNUM      0x30        // Part number
#define TI_CCxxx0_VERSION      0x31        // Current version number
#define TI_CCxxx0_FREQEST      0x32        // Frequency offset estimate
#define TI_CCxxx0_LQI          0x33        // Demodulator estimate for link quality
#define TI_CCxxx0_RSSI         0x34        // Received signal strength indication
#define TI_CCxxx0_MARCSTATE    0x35        // Control state machine state
#define TI_CCxxx0_WORTIME1     0x36        // High byte of WOR timer
#define TI_CCxxx0_WORTIME0     0x37        // Low byte of WOR timer
#define TI_CCxxx0_PKTSTATUS    0x38        // Current GDOx status and packet status
#define TI_CCxxx0_VCO_VC_DAC   0x39        // Current setting from PLL cal module
#define TI_CCxxx0_TXBYTES      0x3A        // Underflow and # of bytes in TXFIFO
#define TI_CCxxx0_RXBYTES      0x3B        // Overflow and # of bytes in RXFIFO
#define TI_CCxxx0_NUM_RXBYTES  0x7F        // Mask "# of bytes" field in _RXBYTES

// Other memory locations
#define TI_CCxxx0_PATABLE      0x3E
#define TI_CCxxx0_TXFIFO       0x3F
#define TI_CCxxx0_RXFIFO       0x3F

// Masks for appended status bytes
#define TI_CCxxx0_LQI_RX       0x01        // Position of LQI byte
#define TI_CCxxx0_CRC_OK       0x80        // Mask "CRC_OK" bit within LQI byte

// Definitions to support burst/single access:
#define TI_CCxxx0_WRITE_BURST  0x40
#define TI_CCxxx0_READ_SINGLE  0x80
#define TI_CCxxx0_READ_BURST   0xC0
//------------------------------------------------------------------------------

#endif /* A1101R09x_H */
