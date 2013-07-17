#include <Arduino.h>
#include <string.h>

#include "tm_net.h"
#include "tm_debug.h"
#include "host_spi.h"
#include "utility/nvmem.h"
#include "utility/wlan.h"
#include "utility/hci.h"
#include "utility/security.h"
#include "utility/os.h"
#include "utility/netapp.h"
#include "utility/evnt_handler.h"



/**
 * Configuration
 */

int tm_net_is_connected ()
{
  return ulCC3000Connected;
}

int tm_net_has_ip ()
{
  return ulCC3000DHCP;
}

void tm_net_block_until_dhcp ()
{
  // Wait for interrupts
  while (!tm_net_is_connected() || !tm_net_has_ip()) {
    delayMicroseconds(100);
  }
}

int tm_net_block_until_dhcp_wait (int waitLength)
{
  // Wait for interrupts
  int i = 0;
  while (i < waitLength && (!tm_net_is_connected() || !tm_net_has_ip())) {
    delayMicroseconds(100);
    i++;
  }

  if (tm_net_is_connected() && tm_net_has_ip()){
    return 1;
  }
  return 0;
}

int tm_net_ssid (char ssid[33])
{
  tNetappIpconfigRetArgs ipinfo;
  netapp_ipconfig(&ipinfo);
  memset(ssid, 0, 33);
  memcpy(ssid, ipinfo.uaSSID, 32);
  return 0;
}

// int tm_net_rssi ()
// {
//   uint8_t results[64];
//   int res = wlan_ioctl_get_scan_results(10000, results);
//   if (res == 0) {
//     return results[4 + 4] & 0x7F; // lower 7 bits
//   }
//   return res;
// }


uint32_t tm_net_local_ip ()
{
  tNetappIpconfigRetArgs ipinfo;
  netapp_ipconfig(&ipinfo);
  return *((uint32_t *) ipinfo.aucIP);
}

int tm_net_mac (uint8_t mac[MAC_ADDR_LEN])
{
  return nvmem_get_mac_address(mac);
}


void tm_net_initialize_dhcp_server (void) 
{
  // Added by Hai Ta
  //
  // Network mask is assumed to be 255.255.255.0
  //

  uint8_t pucSubnetMask[4], pucIP_Addr[4], pucIP_DefaultGWAddr[4];

  unsigned long pucDNS = 0x04040808;

  pucSubnetMask[0] = 0;
  pucSubnetMask[1] = 0;
  pucSubnetMask[2] = 0;
  pucSubnetMask[3] = 0;
  pucIP_Addr[0] = 0;
  pucIP_Addr[1] = 0;
  pucIP_Addr[2] = 0;
  pucIP_Addr[3] = 0;
  // Use default gateway 192.168.1.1 here
  pucIP_DefaultGWAddr[0] = 0;
  pucIP_DefaultGWAddr[1] = 0;
  pucIP_DefaultGWAddr[2] = 0;
  pucIP_DefaultGWAddr[3] = 0;
                       
  // In order for gethostbyname( ) to work, it requires DNS server to be configured prior to its usage
  // so I am gonna add full static 

  // Netapp_Dhcp is used to configure the network interface, static or dynamic (DHCP).
  // In order to activate DHCP mode, aucIP, aucSubnetMask, aucDefaultGateway must be 0.The default mode of CC3000 is DHCP mode. 
  netapp_dhcp((unsigned long *)pucIP_Addr, (unsigned long *)pucSubnetMask, (unsigned long *)pucIP_DefaultGWAddr, &pucDNS);
}

int tm_net_is_readable (int ulSocket) 
{
  fd_set readSet;        // Socket file descriptors we want to wake up for, using select()
  FD_ZERO(&readSet);
  FD_SET(ulSocket, &readSet); 
  struct timeval timeout;

  timeout.tv_sec = 0;
  timeout.tv_usec = 0;

  int rcount = select( ulSocket+1, &readSet, (fd_set *) 0, (fd_set *) 0, &timeout );
  int flag = FD_ISSET(ulSocket, &readSet);
  return flag;
}

int tm_net_block_until_readable (int ulSocket, int timeout) {
  while (true) {
    if (tm_net_is_readable(ulSocket)) {
      break;
    }
    if (timeout > 0 && --timeout == 0) {
      return -1;
    }
    delay(100);
  }
  return 0;
}

void tm_net_initialize (void)
{
  SpiInit();
  delayMicroseconds(100);

  TM_DEBUG("Calling wlan_init\n");
  wlan_init(CC3000_UsynchCallback, NULL, NULL, NULL, ReadWlanInterruptPin, 
    WlanInterruptEnable, WlanInterruptDisable, WriteWlanPin);

  TM_DEBUG("Calling wlan_start...\n");
  wlan_start(0);

  // tm_net_initialize_dhcp_server();
  // wlan_stop();
  // delay(2000);
  // wlan_start(0);
  
  TM_DEBUG("setting event mask\n");
  wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE|HCI_EVNT_WLAN_UNSOL_INIT|HCI_EVNT_WLAN_ASYNC_PING_REPORT);

  // TM_DEBUG("config wlan\n");
  // wlan_ioctl_set_connection_policy(0, 1, 0);

  unsigned char version[2];
  if (!nvmem_read_sp_version(version)) {
    TM_DEBUG("Firmware version: %d.%d\n", version[0], version[1]);
  } else {
    TM_DEBUG("Failed to read version\n");
  }
}

void tm_net_set_policy(){
  wlan_ioctl_set_connection_policy(0, 1, 0); 
}

void tm_net_smartconfig_initialize(void){
  StartSmartConfig();
}

void tm_net_disconnect (void)
{
  wlan_stop();
}

int tm_net_connect_wpa2 (const char *ssid, const char *keys)
{
  TM_DEBUG("Attempting to connect...\n");
  int connected = wlan_connect(WLAN_SEC_WPA2, (char *) ssid, strlen(ssid), 0, (unsigned char *) keys, strlen(keys));
  TM_DEBUG("Connected: %d\n", connected);
  return connected;
}


/** 
 * UDP
 */

int tm_net_udp_open_socket ()
{
	return socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

int tm_net_udp_close_socket (int ulSocket)
{
	closesocket(ulSocket);
	return 0xFFFFFFFF;
}

int tm_net_udp_listen (int ulSocket, int port)
{
	sockaddr localSocketAddr;
	localSocketAddr.sa_family = AF_INET;
	localSocketAddr.sa_data[0] = (port & 0xFF00) >> 8;
	localSocketAddr.sa_data[1] = (port & 0x00FF); 
	localSocketAddr.sa_data[2] = 0;
	localSocketAddr.sa_data[3] = 0;
	localSocketAddr.sa_data[4] = 0;
	localSocketAddr.sa_data[5] = 0;

	// Bind socket
	int sockStatus;
	if ( (sockStatus = bind(ulSocket, &localSocketAddr, sizeof(sockaddr)) ) != 0) {
		TM_DEBUG("Binding UDP socket failed: %d\n", sockStatus);
	}
  return sockStatus;
}

int tm_net_udp_receive (int ulSocket, uint8_t *buf, unsigned long buf_len, sockaddr *from, socklen_t *from_len)
{
	signed long ret = recvfrom(ulSocket, buf, buf_len, 0, from, from_len);
	if (ret <= 0) {
		TM_DEBUG("No data recieved\n");
	} else {
		TM_DEBUG("Recieved %d UDP bytes\n", ret);
	}
  return ret;
}

int tm_net_udp_send (int ulSocket, uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3, int port, uint8_t *buf, unsigned long buf_len)
{
	sockaddr tSocketAddr;

	tSocketAddr.sa_family = AF_INET;
	tSocketAddr.sa_data[0] = (port & 0xFF00) >> 8; 
	tSocketAddr.sa_data[1] = (port & 0x00FF);
	tSocketAddr.sa_data[2] = ip0;
	tSocketAddr.sa_data[3] = ip1;
	tSocketAddr.sa_data[4] = ip2;
	tSocketAddr.sa_data[5] = ip3;
	
	return sendto(ulSocket, buf, buf_len, 0, &tSocketAddr, sizeof(sockaddr));
}


/**
 * TCP
 */

int tm_net_tcp_open_socket ()
{
  int ulSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  setsockopt(ulSocket, SOL_SOCKET, SOCKOPT_ACCEPT_NONBLOCK, SOCK_ON, 4);
  return ulSocket;
}

int tm_net_tcp_close_socket (int ulSocket)
{
  closesocket(ulSocket);
  return 0xFFFFFFFF;
}

int tm_net_tcp_connect (int ulSocket, uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3, int port)
{
  // the family is always AF_INET
  sockaddr remoteSocketAddr;
  remoteSocketAddr.sa_family = AF_INET;
  remoteSocketAddr.sa_data[0] = (port & 0xFF00) >> 8;
  remoteSocketAddr.sa_data[1] = (port & 0x00FF);
  remoteSocketAddr.sa_data[2] = ip0;
  remoteSocketAddr.sa_data[3] = ip1;
  remoteSocketAddr.sa_data[4] = ip2;
  remoteSocketAddr.sa_data[5] = ip3;

  int lerr = connect(ulSocket, &remoteSocketAddr, sizeof(sockaddr));
  if (lerr != ESUCCESS) {
    TM_DEBUG("Error Connecting\r\n");
  }
  return lerr;
}

int tm_net_tcp_write (int ulSocket, const uint8_t *buf, unsigned long buf_len)
{
  int sentLen = send(ulSocket, buf, buf_len, 0);
  // TM_DEBUG("Wrote %d bytes to TCP socket.\n", sentLen);
  return sentLen;
}

int tm_net_tcp_read (int ulSocket, uint8_t *buf, int buf_len)
{
  return recv(ulSocket, buf, buf_len, 0);
}

int tm_net_tcp_read_byte (int ulSocket)
{
  uint8_t buf[1];
  int status = tm_net_tcp_read(ulSocket, buf, 1);
  if (status <= 0) {
    return -1;
  }
  return buf[0];
}

int tm_net_tcp_listen (int ulSocket, int port)
{
  sockaddr localSocketAddr;
  localSocketAddr.sa_family = AF_INET;
  localSocketAddr.sa_data[0] = (port & 0xFF00) >> 8; //ascii_to_char(0x01, 0x01);
  localSocketAddr.sa_data[1] = (port & 0x00FF); //ascii_to_char(0x05, 0x0c);
  localSocketAddr.sa_data[2] = 0;
  localSocketAddr.sa_data[3] = 0;
  localSocketAddr.sa_data[4] = 0;
  localSocketAddr.sa_data[5] = 0;

  // Bind socket
  TM_DEBUG("Binding local socket...\n");
  int sockStatus;
  if ((sockStatus = bind(ulSocket, &localSocketAddr, sizeof(sockaddr))) != 0) {
    TM_DEBUG("binding failed: %d\n", sockStatus);
    return -1;
  }

  TM_DEBUG("Listening on local socket...\n");
  int listenStatus = listen(ulSocket, 1);
  if (listenStatus != 0) {
    TM_DEBUG("cannot listen to socket: %d\n", listenStatus);
    return -1;
  }

  return 0;
}

int tm_net_tcp_accept (int ulSocket, sockaddr *addrClient, socklen_t *addrlen)
{
  return accept(ulSocket, addrClient, addrlen);
}

int tm_net_tcp_block_until_accepting (int ulSocket, sockaddr *addrClient, socklen_t *addrlen)
{
  int result = 0;
  *addrlen = sizeof(sockaddr);
  while ((result = tm_net_tcp_accept(ulSocket, addrClient, addrlen)) < 0) {
    delayMicroseconds(100);
  }
  return result;
}

void tm_net_tcp_read_debug (int ulSocket)
{
  uint8_t buf[30];

  // If no content is readable, stop blocking.
  if (tm_net_block_until_readable(ulSocket, 10) < 0) {
    return;
  }

  // Dump bytes.
  int tcpc = 0;
  while (tm_net_is_readable(ulSocket)) {
    int count = tm_net_tcp_read(ulSocket, buf, 30);
    tcpc += count;
    for (int i = 0; i < count; i++) {
      TM_DEBUG("%c", buf[i]);
    }
  }
  TM_DEBUG("\n(Read %d bytes.)\n", tcpc);
}
