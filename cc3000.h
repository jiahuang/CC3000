
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

#define ConnLED 7
#define ErrorLED 6
#define CC3000_UNENCRYPTED_SMART_CONFIG 1

//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************
extern long ulSocket;

extern int test(void);

extern void initialize(void);
extern void sendUDP(void);
extern void connectUDP(void);
extern void closeUDP(void);
extern void listenUDP(void);
extern const char *receiveUDP(void);
extern void StartSmartConfig(void);


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

