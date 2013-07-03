
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
extern unsigned long ulSmartConfigFinished, ulCC3000Connected,ulCC3000DHCP, OkToDoShutDown, ulCC3000DHCP_configured;
extern uint8_t ulCC3000DHCPIP[4];

extern void initializeCC3000(void);

extern int openUDP (void);
extern int closeUDP (int socket);
extern void sendUDP (int ulSocket, uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3, int port, uint8_t *buf, unsigned long buf_len);
extern void listenUDP (int socket);
extern const char *receiveUDP (int ulSocket);

extern int openTCP ();
extern int closeTCP (int ulSocket);
extern int requestTCP (int ulSocket, uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3, int port, uint8_t *buf, unsigned long buf_len);
extern int pollReadTCP (int ulSocket);
extern int crudeReadTCP (int ulSocket);

extern int listenTCP (int ulSocket, int port);
extern void receiveTCP (int ulSocket);

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

