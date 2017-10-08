/*
 * _HMC_SIM_MACROS_H_
 *
 * HYBRID MEMORY CUBE SIMULATION LIBRARY
 *
 * MACROS HEADER FILE
 *
 */

#ifndef _HMC_SIM_MACROS_H_
#define _HMC_SIM_MACROS_H_


#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------- TRACE VALUES */
#define		HMC_TRACE_BANK		0x0001
#define		HMC_TRACE_QUEUE		0x0002
#define		HMC_TRACE_CMD		0x0004
#define		HMC_TRACE_STALL		0x0008
#define		HMC_TRACE_LATENCY	0x0010

/* -------------------------------------------- MACROS */
#define		HMC_MIN_VAULTS		32
#define		HMC_MIN_DRAMS		20

#ifdef __cplusplus
} /* extern C */
#endif

#endif	/* _HMC_SIM_MACROS_H_ */

/* EOF */
