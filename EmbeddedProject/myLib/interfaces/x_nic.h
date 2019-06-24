/*
  FILE: $FILE_FNAME$
  Created on: 8/25/2017, by Tom Diaz
*/
#ifndef X_NIC_H
#define X_NIC_H

#include "stdint.h"
#include <netstructs.h>

class x_NIC
{
public:
  virtual void Initialise() = 0;
  
  virtual uint16_t CheckForRx() = 0;
  
  virtual uint8_t ReadNextByte(uint8_t* data) = 0;
  
  virtual uint8_t ReadArray(uint8_t* array_buffer, uint16_t array_length) = 0;
  
  virtual void MovePointer(uint16_t move_pointer_to_ethernet_byte) = 0;
  
  virtual void DumpPacket() = 0;
  
  virtual uint8_t SetupTx() = 0;
  
  virtual void WriteEthernetHeader(MAC_ADDR *remote_mac_address, uint16_t ethernet_packet_type) = 0;
  
  virtual void WriteNextByte(uint8_t data) = 0;
  
  virtual void WriteArray(uint8_t *array_buffer, uint16_t array_length) = 0;
  
  virtual void WriteTxWordAtLocation(uint16_t byte_address, uint16_t data) = 0;
  
  virtual void TxPacket() = 0;
  
  virtual uint8_t OkToDoTx() = 0;

  uint8_t Linked() { return linked; }
  
  uint16_t TxLen() { return txLen; }
  
  uint8_t RxPacketWaitingToBeDumped;
  
  MAC_ADDR* MAC() { return mac; }
protected:
  uint8_t linked;
  uint16_t txLen;
  MAC_ADDR* mac;
};

#endif /* X_NIC_H */
