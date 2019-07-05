/*
  FILE: nfccard.h
  Created on: 9/18/2016, by Tom Diaz
*/

#ifndef NFCCARD_H
#define NFCCARD_H

class NfcCard
{
public:
  bool operator ~ ();
  NfcCard& operator = (NfcCard src);
  NfcCard& operator = (NfcCard* src);
  bool operator == (NfcCard card);
  bool operator == (NfcCard* card);
  bool operator == (char const* uid_);
  void Serialize(BufferHandler* buff);
  bool Init(uint8_t* buffer); // Populate from a raw buffer
  void AddExtraData(uint8_t* data);
  CardType::Value GetType();
  uint8_t* GetUID();
  uint8_t GetUIDLength();
  bool Validate(uint8_t* k);
  void AddIDs(uint8_t num, uint8_t* idbuff);
  
private:
  uint8_t numberOfApplications;
  CardType::Value type;
  uint8_t uidLength;
  uint8_t uid[7];
  uint8_t extraDataIndex;
  uint16_t hwVer;
  uint16_t swVer;
  uint16_t year;
  uint16_t week;
  uint16_t size;
};

#endif /* NFCCARD_H */
