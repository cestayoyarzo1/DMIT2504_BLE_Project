/*
  FILE: enc28j60x.h
  Created on: 8/23/2017, by Tom Diaz
*/
#ifndef ENC28J60X_H
#define ENC28J60X_H

#include <x_spiport.h>
#include <x_gpio.h>
#include <x_nic.h>
#include <enc28j60x_c.h>

class ENC28J60x : public x_NIC
{
public:
  ENC28J60x(){};
  ~ENC28J60x(){};
  void Init(MAC_ADDR* mac_, x_GPIO* cs_, x_GPIO* res_, x_GPIO* intr_, x_SPIPort* spi_);
  void Initialise();
  uint16_t CheckForRx();
  uint8_t ReadNextByte(uint8_t* data);
  uint8_t ReadArray(uint8_t* array_buffer, uint16_t array_length);  
  void MovePointer(uint16_t move_pointer_to_ethernet_byte);  
  void DumpPacket();  
  uint8_t SetupTx();  
  void WriteEthernetHeader(MAC_ADDR *remote_mac_address, uint16_t ethernet_packet_type);  
  void WriteNextByte(uint8_t data);  
  void WriteArray(uint8_t *array_buffer, uint16_t array_length);  
  void WriteTxWordAtLocation(uint16_t byte_address, uint16_t data);  
  void TxPacket();  
  uint8_t OkToDoTx();
protected:
  void reset(void);
  NIC_REG nic_read(uint8_t address);
  void nic_write(uint8_t address, uint8_t data);
  void nic_write_buffer(uint8_t data);
  void nic_select_bank(uint16_t reg);
  void nic_bit_field_clear_register(uint8_t address, uint8_t data);
  void nic_bit_field_set_register (uint8_t address, uint8_t data);
  void nic_write_phy_register(uint8_t register, uint16_t data);
  NIC_REG nic_read_mac_mii_register(uint8_t address);
  PHYREG nic_read_phy_register(uint8_t reg);
  void nic_setup_read_data(void);
  WORD_VAL nic_next_packet_location;
  WORD_VAL nic_current_packet_location;
  uint8_t enc28j60_revision_id;
  uint16_t nic_rx_bytes_remaining;
  uint16_t nic_rx_packet_total_ethernet_bytes;
  //uint16_t TxLen;
  //PHYREG phyReg;
  x_GPIO* cs;
  x_GPIO* res;
  x_GPIO* intr;
  x_SPIPort* spi;
private:
};

#endif /* ENC28J60X_H */
