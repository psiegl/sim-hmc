#ifndef _HMC_MACROS_H_
#define _HMC_MACROS_H_

#define ALWAYS_INLINE             inline __attribute__((always_inline))
#define elemsof( x )              (sizeof(x) / sizeof((x)[0]))

#endif /* #ifndef _HMC_MACROS_H_ */
