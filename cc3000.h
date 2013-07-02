#include <Arduino.h>
#include <SPI.h>
#include <string.h>
#include "utility/cc3000_spi.h"
#include "utility/nvmem.h"
#include "utility/wlan.h"
#include "utility/hci.h"


//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifndef __CC3000_H__
#define __CC3000_H__

#ifdef  __cplusplus
extern "C" {
#endif

//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************
extern int test(void);
extern void SPI_IRQ(void);
extern void SpiInit(void);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef  __cplusplus
}
#endif // __cplusplus

//#endif
#endif // __CC3000_H__

