#ifndef __TM_NET_H__
#define __TM_NET_H__

#include "utility/socket.h"
#include "utility/cc3000_common.h"

extern "C" {

// Wifi

extern unsigned long ulSmartConfigFinished, ulCC3000Connected,ulCC3000DHCP, OkToDoShutDown, ulCC3000DHCP_configured;

extern void tm_net_initialize (void);
extern void tm_net_connect_wpa2 (const char *ssid, const char *keys);
extern unsigned long tm_net_local_ip ();
extern int tm_net_mac (uint8_t mac[MAC_ADDR_LEN]);

extern void tm_net_block_until_dhcp (void);

// UDP

extern int tm_net_udp_open_socket (void);
extern int tm_net_udp_close_socket (int socket);

extern void tm_net_udp_send (int ulSocket, uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3, int port, uint8_t *buf, unsigned long buf_len);

extern int tm_net_udp_listen (int socket, int port);
extern const char *tm_net_udp_receive (int ulSocket);

// TCP

extern int tm_net_tcp_open_socket ();
extern int tm_net_tcp_close_socket (int ulSocket);

extern int tm_net_tcp_connect (int ulSocket, uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3, int port);
extern int tm_net_tcp_write (int ulSocket, uint8_t *buf, unsigned long buf_len);

extern int tm_net_tcp_is_readable (int ulSocket);
extern int tm_net_tcp_block_until_readable (int ulSocket, int tries);

extern int tm_net_tcp_read (int ulSocket, uint8_t buf, int buf_len);
extern int tm_net_tcp_read_byte (int ulSocket);
extern void tm_net_tcp_read_debug (int ulSocket);

extern int tm_net_tcp_listen (int ulSocket, int port);
extern int tm_net_tcp_accept (int ulSocket, sockaddr *addrClient, socklen_t *addrLen);
extern int tm_net_tcp_block_until_accepting (int ulSocket, sockaddr *addrClient, socklen_t *addrLen);

};

#endif

