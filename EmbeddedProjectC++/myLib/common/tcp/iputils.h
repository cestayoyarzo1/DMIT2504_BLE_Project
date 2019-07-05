/*
  FILE: iputils.h
  Created on: 8/28/2017, by Tom Diaz
*/
#ifndef IP_UTILS_H
#define IP_UTILS_H

#include <stdint.h>
#include <x_nic.h>

class IPUtils
{
public:
  static uint16_t swap_word_bytes(uint16_t data);
  static uint8_t convert_character_to_lower_case(uint8_t character);
  static uint32_t swap_dword_bytes(uint32_t data);
protected:
private:
};

#endif /* IP_UTILS_H */
