#include <avr/pgmspace.h>
#ifndef HEXIMAGE_H
#define HEXIMAGE_H

// In C++, the name of functions and variables are changed during compiling (name mangling), 
// to implement overloading (= multiple functions with the same name). 
// So, I think you should put extern in the variable name
// if you want to use the same function name in other cpp.
//extern const PROGMEM unsigned char large_digit[10][27*5];
//extern const PROGMEM unsigned char small_digit[10][18*3];
//extern const PROGMEM unsigned char background[128*8];

extern const unsigned char large_digit[10][27*5];
extern const unsigned char small_digit[10][18*3];
extern const unsigned char background[128*8];

#endif
