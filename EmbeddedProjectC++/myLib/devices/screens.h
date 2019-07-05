/*
Reference template for 4x20 char display

const uint8_t example[][20] = 
{
///01234567890123456789 <- Reference
  "                    ",
///01234567890123456789 <- Reference
  "                    ",
///01234567890123456789 <- Reference
  "                    ",
///01234567890123456789 <- Reference
  "                    ",
};

*/

extern "C"
{
  const uint8_t clearLine[21] = 
  {"                    "};
  
  const uint8_t splash[4][21] = 
  {
    ///01234567890123456789 <- Reference
    "     Diesel Tech    ",
    ///01234567890123456789 <- Reference
    "      Industries    ",
    ///01234567890123456789 <- Reference
    "       2 0 1 6      ",
    ///01234567890123456789 <- Reference
    "                    ",
  };
}
