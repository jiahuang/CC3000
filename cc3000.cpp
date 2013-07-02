#include <Arduino.h>
#include <string.h>

#include "utility/cc3000_spi.h"
#include "utility/nvmem.h"
#include "utility/wlan.h"
#include "utility/hci.h"
#include "utility/security.h"
#include "utility/os.h"
#include "utility/netapp.h"
#include "utility/evnt_handler.h"
#include "utility/debug.h"
#include "cc3000.h"

/**
 * Configuration
 */

char ssid[] = "HCPGuest";                     // your network SSID (name) 
unsigned char keys[] = "kendall!";       // your network key
int connected = -1;

#define CC3000_APP_BUFFER_SIZE                  (128)
#define CC3000_RX_BUFFER_OVERHEAD_SIZE          (20)

unsigned char pucCC3000_Rx_Buffer[CC3000_APP_BUFFER_SIZE + CC3000_RX_BUFFER_OVERHEAD_SIZE] = { 0 };

void wait_dhcp ()
{
  while ((ulCC3000DHCP == 0) || (ulCC3000Connected == 0)) {
    continue;
  }
}

/** 
 * UDP
 */

long openUDP ()
{
	wait_dhcp();

	// open a socket
	return socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

long closeUDP (long ulSocket)
{
	closesocket(ulSocket);
	return 0xFFFFFFFF;
}

void listenUDP (long ulSocket)
{
	sockaddr localSocketAddr;
	localSocketAddr.sa_family = AF_INET;
	localSocketAddr.sa_data[0] = (4444 & 0xFF00) >> 8;
	localSocketAddr.sa_data[1] = (4444 & 0x00FF); 
	localSocketAddr.sa_data[2] = 0;
	localSocketAddr.sa_data[3] = 0;
	localSocketAddr.sa_data[4] = 0;
	localSocketAddr.sa_data[5] = 0;

	// Bind socket
	int sockStatus;
	if ( (sockStatus = bind(ulSocket, &localSocketAddr, sizeof(sockaddr)) ) != 0) {
		Serial.print("binding failed: ");
		Serial.println(sockStatus, BIN);
		return;
	}
}

const char *receiveUDP (long ulSocket)
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
		_DEBUG("No data recieved\n");
	}
	else
	{
		Serial.print("Recieved with flag: ");
		Serial.println(iReturnValue, BIN);
	}

	return (const char *) pucCC3000_Rx_Buffer;
}

void sendUDP (long ulSocket, uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3, int port, uint8_t *buf, unsigned long buf_len)
{
	sockaddr tSocketAddr;

	wait_dhcp();

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

long openTCP ()
{
  wait_dhcp();
  // open a socket
  return socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

long closeTCP (long ulSocket)
{
  closesocket(ulSocket);
  return 0xFFFFFFFF;
}


int requestTCP (long ulSocket, uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3, int port, uint8_t *buf, unsigned long buf_len)
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

  if (lerr != ESUCCESS)
  {
    _DEBUG("Error Connecting\r\n");
    return -1;
  }

  int sentLen = send(ulSocket, buf, buf_len, 0);
  _DEBUG("Sent length of TCP: %d\n", sentLen);
  int bytesReceived = recv(ulSocket, pucCC3000_Rx_Buffer, CC3000_APP_BUFFER_SIZE, 0);
  _DEBUG("Result of TCP: %d\n", bytesReceived);
  return bytesReceived;
}

int listenTCP (long ulSocket, int port)
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
  _DEBUG("Binding local socket...\n");
  int sockStatus;
  if ((sockStatus = bind(ulSocket, &localSocketAddr, sizeof(sockaddr))) != 0) {
    _DEBUG("binding failed: %d\n", sockStatus);
    return -1;
  }

  _DEBUG("Listening on local socket...\n");
  int listenStatus = listen(ulSocket, 1);
  if (listenStatus != 0) {
    _DEBUG("cannot listen to socket: %d\n", listenStatus);
    return -1;
  }

  return 0;
}

void receiveTCP (long ulSocket)
{
  long clientDescriptor = -1;
  sockaddr addrClient;
  socklen_t addrlen = sizeof(sockaddr);

  setsockopt(ulSocket, SOL_SOCKET, SOCKOPT_ACCEPT_NONBLOCK, SOCK_ON, 4);

  while (1) {
    clientDescriptor = accept(ulSocket, &addrClient, &addrlen);

    if (clientDescriptor >= 0 ) {
      _DEBUG("Client discovered: %ld\n", clientDescriptor);

      int recvStatus = recv(clientDescriptor, pucCC3000_Rx_Buffer, CC3000_APP_BUFFER_SIZE, 0);
      _DEBUG("Received packet from client. %d\n", recvStatus);

      const char *res = "HTTP/1.0 200 OK\r\nContent-Length: 19\r\nContent-Type: text/html\r\n\r\n<h1>HI BITCHES</h1>";
      int sendStatus = send(clientDescriptor, res, strlen(res), 0);
      _DEBUG("Response sent: %d\n", sendStatus);

      int closeStatus = closesocket(clientDescriptor);
      _DEBUG("Socket close status: %d\n", closeStatus);
      delayMicroseconds(100);
    } else if (clientDescriptor == -57) {
      // BUG: Socket inactive so reopen socket
      // Inactive Socket, close and reopen it
      ulSocket = closesocket(ulSocket);

      ulSocket = openTCP();
      listenTCP(ulSocket, 4444);
      _DEBUG("Had to re-open server due to bug.\n");
    }
    // hci_unsolicited_event_handler();

    delayMicroseconds(100);
  }
}


/**
 * Startup
 */

void getIpAddr (char * ipBuffer)
{
  tNetappIpconfigRetArgs ipinfo;
  netapp_ipconfig(&ipinfo);
  // iptostring(ipinfo.aucIP, ipBuffer);
  // DispatcherUartSendPacket("Ip = ", 5);
  // DispatcherUartSendPacket((unsigned char*) ipvalue, length);
}

void printMAC (void)
{
  unsigned char cMacFromEeprom[MAC_ADDR_LEN];

  if (nvmem_get_mac_address(cMacFromEeprom)) {
    _DEBUG("No mac address found\n");
  } else {
    _DEBUG("MAC: %d", cMacFromEeprom[0]);
    for(int i = 1; i < MAC_ADDR_LEN; i++){
      _DEBUG(".%d", cMacFromEeprom[i]);
    }
    _DEBUG("\n");
  }
}

void initializeCC3000 (void)
{
  SpiInit();
  delayMicroseconds(100);

  _DEBUG("Calling wlan_init\n");
  wlan_init(CC3000_UsynchCallback, NULL, NULL, NULL, ReadWlanInterruptPin, 
    WlanInterruptEnable, WlanInterruptDisable, WriteWlanPin);

  _DEBUG("Calling wlan_start...\n");
  wlan_start(0);

  printMAC();
  
  _DEBUG("setting event mask\n");
  wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE|HCI_EVNT_WLAN_UNSOL_INIT|HCI_EVNT_WLAN_ASYNC_PING_REPORT);

	_DEBUG("config wlan\n");
	wlan_ioctl_set_connection_policy(0, 0, 0);

	_DEBUG("Attempting to connect...\n");
	int connected = -1;
	connected = wlan_connect(WLAN_SEC_WPA2,ssid,8, 0, keys, 8);
	_DEBUG("Connected: %d\n", connected);
	
  unsigned char version[2];
	if (!nvmem_read_sp_version(version)) {
		_DEBUG("Firmware version: %d.%d\n", version[0], version[1]);
	} else {
		_DEBUG("Failed to read version\n");
	}
}
