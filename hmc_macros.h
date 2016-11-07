#ifndef _HMC_MACROS_H_
#define _HMC_MACROS_H_

#include <cstdint>

#define ALWAYS_INLINE           inline __attribute__((always_inline))
#define HMC_PACKET_HEADER( x )  ( *((uint64_t*)(x)) )

#endif /* #ifndef _HMC_MACROS_H_ */
