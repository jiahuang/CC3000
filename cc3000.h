
//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifndef __CC3000_H__
#define __CC3000_H__

#include "utility/socket.h"

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

extern void cc_initialize (void);

extern void cc_block_until_dhcp (void);

// UDP

extern int cc_udp_open_socket (void);
extern int cc_udp_close_socket (int socket);

extern void cc_udp_send (int ulSocket, uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3, int port, uint8_t *buf, unsigned long buf_len);

extern int cc_udp_listen (int socket, int port);

extern const char *receiveUDP (int ulSocket);

// TCP

extern int cc_tcp_open_socket ();
extern int cc_tcp_close_socket (int ulSocket);

extern int cc_tcp_connect (int ulSocket, uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3, int port);
extern int cc_tcp_write (int ulSocket, uint8_t *buf, unsigned long buf_len);

extern int cc_tcp_is_readable (int ulSocket);
extern int cc_tcp_block_until_readable (int ulSocket, int tries);

extern int cc_tcp_read (int ulSocket, uint8_t buf, int buf_len);
extern int cc_tcp_read_byte (int ulSocket);
extern void cc_tcp_read_dump (int ulSocket);

extern int cc_tcp_listen (int ulSocket, int port);
extern int cc_tcp_accept (int ulSocket, sockaddr *addrClient, socklen_t *addrLen);
extern int cc_tcp_block_until_accepting (int ulSocket, sockaddr *addrClient, socklen_t *addrLen);

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

