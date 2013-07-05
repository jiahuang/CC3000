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

int connected = -1;

#define CC3000_APP_BUFFER_SIZE                  (5)
#define CC3000_RX_BUFFER_OVERHEAD_SIZE          (20)

unsigned char pucCC3000_Rx_Buffer[CC3000_APP_BUFFER_SIZE + CC3000_RX_BUFFER_OVERHEAD_SIZE] = { 0 };

void tm_net_block_until_dhcp ()
{
  while ((ulCC3000DHCP == 0) || (ulCC3000Connected == 0)) {
    delayMicroseconds(100);
  }
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

const char *tm_net_udp_receive (int ulSocket)
{
	// the family is always AF_INET
	sockaddr remoteSocketAddr;
	remoteSocketAddr.sa_family = AF_INET;
	remoteSocketAddr.sa_data[0] = (4444 & 0xFF00) >> 8; 
	remoteSocketAddr.sa_data[1] = (4444 & 0x00FF);
	remoteSocketAddr.sa_data[2] = 10;
	remoteSocketAddr.sa_data[3] = 1;
	remoteSocketAddr.sa_data[4] = 90;
	remoteSocketAddr.sa_data[5] = 135;

	socklen_t tRxPacketLength = 8;
	signed long iReturnValue = recvfrom(ulSocket, pucCC3000_Rx_Buffer, CC3000_APP_BUFFER_SIZE, 0, &remoteSocketAddr, &tRxPacketLength);
	if (iReturnValue <= 0)
	{
		TM_DEBUG("No data recieved\n");
	}
	else
	{
		Serial.print("Recieved with flag: ");
		Serial.println(iReturnValue, BIN);
	}

	return (const char *) pucCC3000_Rx_Buffer;
}

void tm_net_udp_send (int ulSocket, uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3, int port, uint8_t *buf, unsigned long buf_len)
{
	sockaddr tSocketAddr;

	tSocketAddr.sa_family = AF_INET;
	tSocketAddr.sa_data[0] = (port & 0xFF00) >> 8; 
	tSocketAddr.sa_data[1] = (port & 0x00FF);
	tSocketAddr.sa_data[2] = ip0;
	tSocketAddr.sa_data[3] = ip1;
	tSocketAddr.sa_data[4] = ip2;
	tSocketAddr.sa_data[5] = ip3;
	
	sendto(ulSocket, buf, buf_len, 0, &tSocketAddr, sizeof(sockaddr));
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

int tm_net_tcp_write (int ulSocket, uint8_t *buf, unsigned long buf_len)
{
  int sentLen = send(ulSocket, buf, buf_len, 0);
  TM_DEBUG("Wrote %d bytes to TCP socket.\n", sentLen);
  return sentLen;
}

int tm_net_tcp_is_readable (int ulSocket) 
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

int tm_net_tcp_block_until_readable (int ulSocket, int timeout) {
  while (true) {
    if (tm_net_tcp_is_readable(ulSocket)) {
      break;
    }
    if (timeout > 0 && --timeout == 0) {
      return -1;
    }
    delay(100);
  }
  return 0;
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
  if (tm_net_tcp_block_until_readable(ulSocket, 10) < 0) {
    return;
  }

  // Dump bytes.
  int tcpc = 0;
  while (tm_net_tcp_is_readable(ulSocket)) {
    int count = tm_net_tcp_read(ulSocket, buf, 30);
    tcpc += count;
    for (int i = 0; i < count; i++) {
      TM_DEBUG("%c", buf[i]);
    }
  }
  TM_DEBUG("\n(Read %d bytes.)\n", tcpc);
}


/**
 * Startup
 */

unsigned long tm_net_local_ip ()
{
  tNetappIpconfigRetArgs ipinfo;
  netapp_ipconfig(&ipinfo);
  return (unsigned long) ipinfo.aucIP;
  // iptostring(ipinfo.aucIP, ipBuffer);
  // DispatcherUartSendPacket("Ip = ", 5);
  // DispatcherUartSendPacket((unsigned char*) ipvalue, length);
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

	TM_DEBUG("config wlan\n");
	wlan_ioctl_set_connection_policy(0, 0, 0);

  unsigned char version[2];
  if (!nvmem_read_sp_version(version)) {
    TM_DEBUG("Firmware version: %d.%d\n", version[0], version[1]);
  } else {
    TM_DEBUG("Failed to read version\n");
  }
}

void tm_net_connect_wpa2 (const char *ssid, const char *keys)
{
	TM_DEBUG("Attempting to connect...\n");
	int connected = -1;
	connected = wlan_connect(WLAN_SEC_WPA2, (char *) ssid, strlen(ssid), 0, (unsigned char *) keys, strlen(keys));
	TM_DEBUG("Connected: %d\n", connected);
}







/**
 * HTTP
 */



enum http_header_status
{
    http_header_status_done,
    http_header_status_continue,
    http_header_status_version_character,
    http_header_status_code_character,
    http_header_status_status_character,
    http_header_status_key_character,
    http_header_status_value_character,
    http_header_status_store_keyvalue
};

static unsigned char http_header_state[] = {
/*     *    \t    \n   \r    ' '     ,     :   PAD */
    0x80,    1, 0xC1, 0xC1,    1, 0x80, 0x80, 0xC1, /* state 0: HTTP version */
    0x81,    2, 0xC1, 0xC1,    2,    1,    1, 0xC1, /* state 1: Response code */
    0x82, 0x82,    4,    3, 0x82, 0x82, 0x82, 0xC1, /* state 2: Response reason */
    0xC1, 0xC1,    4, 0xC1, 0xC1, 0xC1, 0xC1, 0xC1, /* state 3: HTTP version newline */
    0x84, 0xC1, 0xC0,    5, 0xC1, 0xC1,    6, 0xC1, /* state 4: Start of header field */
    0xC1, 0xC1, 0xC0, 0xC1, 0xC1, 0xC1, 0xC1, 0xC1, /* state 5: Last CR before end of header */
    0x87,    6, 0xC1, 0xC1,    6, 0x87, 0x87, 0xC1, /* state 6: leading whitespace before header value */
    0x87, 0x87, 0xC4,   10, 0x87, 0x88, 0x87, 0xC1, /* state 7: header field value */
    0x87, 0x88,    6,    9, 0x88, 0x88, 0x87, 0xC1, /* state 8: Split value field value */
    0xC1, 0xC1,    6, 0xC1, 0xC1, 0xC1, 0xC1, 0xC1, /* state 9: CR after split value field */
    0xC1, 0xC1, 0xC4, 0xC1, 0xC1, 0xC1, 0xC1, 0xC1, /* state 10:CR after header value */
};

int http_parse_header_char(int* state, char ch)
{
    int newstate, code = 0;
    switch (ch) {
    case '\t': code = 1; break;
    case '\n': code = 2; break;
    case '\r': code = 3; break;
    case  ' ': code = 4; break;
    case  ',': code = 5; break;
    case  ':': code = 6; break;
    }

    newstate = http_header_state[*state * 8 + code];
    *state = (newstate & 0xF);

    switch (newstate) {
    case 0xC0: return http_header_status_done;
    case 0xC1: return http_header_status_done;
    case 0xC4: return http_header_status_store_keyvalue;
    case 0x80: return http_header_status_version_character;
    case 0x81: return http_header_status_code_character;
    case 0x82: return http_header_status_status_character;
    case 0x84: return http_header_status_key_character;
    case 0x87: return http_header_status_value_character;
    case 0x88: return http_header_status_value_character;
    }

    return http_header_status_continue;
}

int http_parse_header (int ulSocket, char (*http_receive_byte)(int), void (*on_header)(char *, int), void (*on_body)(int status, int content_len))
{
  int code = 0, content_len = 0;

  int state = 0;
  char buf[32] = { 0 };
  int keyn = 0, valn = 0;
  uint8_t is_content_len_header = 0;

  while (true) {
    char ch = http_receive_byte(ulSocket);

    switch (http_parse_header_char(&state, ch)) {
      case http_header_status_done:
        on_body(code, content_len);
        return content_len;
        break;

      case http_header_status_code_character:
        code = code * 10 + ch - '0';
        break;

      case http_header_status_key_character:
        if (keyn + 1 < sizeof(buf)) {
          buf[keyn] = tolower(ch);
          keyn++;
        }
        break;

      case http_header_status_value_character:
        if (keyn > 0) {
          TM_DEBUG("keyn %d compare %d\n", keyn, strncmp(buf, "content-length", keyn));
          buf[keyn] = '\0';
          if (keyn == 14 && 0 == strncmp(buf, "content-length", keyn)) {
            is_content_len_header = 1;
          }
          on_header(buf, keyn);
          keyn = 0;
        }

        buf[valn] = ch;
        if (valn + 2 < sizeof(buf)) {
          valn++;
        }
        break;

      case http_header_status_store_keyvalue:
        if (is_content_len_header) {
          for (int i = 0; i < valn; i++) {
            content_len = content_len * 10 + buf[i] - '0';
          }
          is_content_len_header = 0;
        }
        valn = keyn = 0;
        break;
    }
  }
}





char http_receive_byte (int ulSocket)
{
  while (recv(ulSocket, pucCC3000_Rx_Buffer, 1, 0) == 0) {
    delay(100);
  }
  return pucCC3000_Rx_Buffer[0];
}

void on_http_header (char *buf, int len)
{
  TM_DEBUG("HEADER matched %s\n", buf);
}

void on_http_body (int status, int len)
{
  TM_DEBUG("BODY %d length %d\n", status, len);
}

// int crudeReadTCP (int ulSocket)
// {
//   if (tm_net_tcp_block_until_readable(ulSocket) < 0) {
//     return -1;
//   }
//   delay(100);

//   http_parse_header(ulSocket, http_receive_byte, on_http_header, on_http_body);

//   // int totalbytes = 0, bytesReceived = 0;
//   // while (block_readable_socket(ulSocket) == 0) {
//   //   bytesReceived = recv(ulSocket, pucCC3000_Rx_Buffer, CC3000_APP_BUFFER_SIZE, 0);
//   //   if (bytesReceived <= 0) {
//   //     break;
//   //   }
//   //   totalbytes += bytesReceived;
//   //   pucCC3000_Rx_Buffer[bytesReceived] = '\0';
//   //   TM_DEBUG("Result of TCP %d bytes: \"%s\"\n", bytesReceived, pucCC3000_Rx_Buffer);
//   // } 

//   // return bytesReceived;
// }
