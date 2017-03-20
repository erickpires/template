#ifndef DEFAULT_DEFINITIONS_H
#define DEFAULT_DEFINITIONS_H 1

#include <stdint.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef unsigned int uint;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int bool;
#define FALSE 0
#define TRUE  1

#define KILO(n) (1024 * n)
#define MEGA(n) (KILO(n))

#endif
