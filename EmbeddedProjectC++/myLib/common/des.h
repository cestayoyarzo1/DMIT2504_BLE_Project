/*
  FILE: des.h
  Created on: 9/18/2016, by Tom Diaz
*/

#ifndef DES_H
#define DES_H

class DES
{
public:
  DES();
  void Encrypt(uint8_t data[8]);
  void Decrypt(uint8_t data[8]);
  void SetKey(uint8_t k[8]);
  void des_crypt(uint8_t in[], uint8_t out[], uint8_t key[][6]);
  void key_schedule(uint8_t key[], uint8_t schedule[][6], bool mode);
private:
  void InvIP(uint32_t state[], uint8_t in[]);
  uint32_t f(uint32_t state, uint8_t key[]);
  void IP(uint32_t state[], uint8_t in[]);
  uint8_t out[8];
  uint8_t key[8];
  uint8_t schedule[16][6];
};

#endif /* DES_H */
