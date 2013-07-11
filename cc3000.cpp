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

#include "cc3000.h"


#define CC3000_APP_BUFFER_SIZE                      (256)
#define CC3000_RX_BUFFER_OVERHEAD_SIZE          (20)
#define DISABLE 0
#define ENABLE 1
#define NETAPP_IPCONFIG_MAC_OFFSET        (20)

char ssid[] = "HCPGuest";                     // your network SSID (name) 
unsigned char keys[] = "kendall!";       // your network key
int connected = -1;
const char aucCC3000_prefix[] = {'T', 'T', 'T'};
//AES key "smartconfigAES16"
const unsigned char smartconfigkey[] = {0x73,0x6d,0x61,0x72,0x74,0x63,0x6f,0x6e,0x66,0x69,0x67,0x41,0x45,0x53,0x31,0x36};
unsigned long ulSmartConfigFinished, ulCC3000Connected,ulCC3000DHCP, OkToDoShutDown, ulCC3000DHCP_configured;
unsigned char ucStopSmartConfig;

// unsigned long ulSmartConfigFinished, ulCC3000Connected,ulCC3000DHCP, OkToDoShutDown, ulCC3000DHCP_configured;
unsigned char pucCC3000_Rx_Buffer[CC3000_APP_BUFFER_SIZE + CC3000_RX_BUFFER_OVERHEAD_SIZE] = { 0 };

// unsigned char ucStopSmartConfig;
// long ulSocket;



//*****************************************************************************
//
//  The function handles asynchronous events that come from CC3000 device 
//!     
//
//*****************************************************************************

void CC3000_UsynchCallback(long lEventType, char * data, unsigned char length)
{
  
  // if (DEBUG_MODE)
  // {
  //  Serial.println("CC3000_UsynchCallback");
  // }

  if (lEventType == HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE)
  {
    
    ulSmartConfigFinished = 1;
    ucStopSmartConfig     = 1;  
    Serial.println("smart config finished");

    // if smart config is done, stop advertising
    //mdnsAdvertiser(1,device_name,strlen(device_name));

    // stop the smart config blinking
    // digitalWrite(ConnLED, LOW);
    // digitalWrite(ErrorLED, LOW);
  }
  
  if (lEventType == HCI_EVNT_WLAN_UNSOL_CONNECT)
  {
    ulCC3000Connected = 1;
    Serial.println("connected");
  }
  
  if (lEventType == HCI_EVNT_WLAN_UNSOL_DISCONNECT)
  {   
    ulCC3000Connected = 0;
    ulCC3000DHCP      = 0;
    ulCC3000DHCP_configured = 0;
    // printOnce = 1;
    
    Serial.println("disconnected");
    digitalWrite(ConnLED, LOW);    
  }
  
  if (lEventType == HCI_EVNT_WLAN_UNSOL_DHCP)
  {

    Serial.println("dhcp");
    // Notes: 
    // 1) IP config parameters are received swapped
    // 2) IP config parameters are valid only if status is OK, i.e. ulCC3000DHCP becomes 1
    
    // only if status is OK, the flag is set to 1 and the addresses are valid
    if ( *(data + NETAPP_IPCONFIG_MAC_OFFSET) == 0)
    {
      Serial.println("set dhcp");
      // Serial.print("Ip: ");
      // Serial.println(data[3], HEX);
      // Serial.println(data[2], HEX);
      // Serial.println(data[1], HEX);
      // Serial.println(data[0], HEX);

      
      ulCC3000DHCP = 1;
      digitalWrite(ConnLED, HIGH);
    }
    else
    {
      ulCC3000DHCP = 0;
      Serial.println("DHCP failed");
      digitalWrite(ErrorLED, HIGH);
    }
  }
  
  if (lEventType == HCI_EVENT_CC3000_CAN_SHUT_DOWN)
  {
    OkToDoShutDown = 1;
  }
}



void connectUDP () {
	while ((ulCC3000DHCP == 0) || (ulCC3000Connected == 0))
	{
	}

	// open a socket
	ulSocket= socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

void closeUDP() {
	closesocket(ulSocket);
	ulSocket = 0xFFFFFFFF;
}

void listenUDP () {
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

const char *receiveUDP () {
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
		Serial.println("No data recieved");
	}
	else
	{
		Serial.print("Recieved with flag: ");
		Serial.println(iReturnValue, BIN);
	}

	return (const char *) pucCC3000_Rx_Buffer;
}

void sendUDP(){
	sockaddr tSocketAddr;
	while ((ulCC3000DHCP == 0) || (ulCC3000Connected == 0))
	{
		delayMicroseconds(100);
	}

	tSocketAddr.sa_family = AF_INET;

	// port 4444
	tSocketAddr.sa_data[0] = 0x11;
	tSocketAddr.sa_data[1] = 0x5c;

	// the destination IP address
	tSocketAddr.sa_data[2] = 10;
	tSocketAddr.sa_data[3] = 1;
	tSocketAddr.sa_data[4] = 90;
	tSocketAddr.sa_data[5] = 135;
	
	sendto(ulSocket, "haha", 4, 0, &tSocketAddr, sizeof(sockaddr));
}

void printMAC(void){
  unsigned char cMacFromEeprom[MAC_ADDR_LEN];

  if (nvmem_get_mac_address(cMacFromEeprom)) {
    Serial.println("No mac address found");
  } else {
    Serial.println("MAC: ");
    for(int i = 0; i<MAC_ADDR_LEN; i++){
      Serial.print(cMacFromEeprom[i]);
      Serial.print(".");
    }
    Serial.println("");
  }
}

void printVersion(void){
	unsigned char version[2];
	if (!nvmem_read_sp_version(version))
	{
		Serial.print("Version: ");
		Serial.print(version[0]);
		Serial.print(".");
		Serial.println(version[1]);

	} else {
		Serial.println("Failed to read version");
	}
}

void initialize(void){
	pinMode(ConnLED, OUTPUT);
	pinMode(ErrorLED, OUTPUT);

  Serial.println("Calling wlan_init");
  wlan_init(CC3000_UsynchCallback, NULL, NULL, NULL, ReadWlanInterruptPin, 
    WlanInterruptEnable, WlanInterruptDisable, WriteWlanPin);

  Serial.println("Calling wlan_start...");
  wlan_start(0);

  printMAC();
  printVersion();
  
 //  Serial.println("setting event mask");
 //  wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE|HCI_EVNT_WLAN_UNSOL_INIT|HCI_EVNT_WLAN_ASYNC_PING_REPORT);

	// Serial.println("config wlan");
	// wlan_ioctl_set_connection_policy(DISABLE, DISABLE, DISABLE);

	// Serial.println("Attempting to connect...");
	// int connected = -1;
	// connected = wlan_connect(WLAN_SEC_WPA2,ssid,8, 0, keys, 8);
	// Serial.println(connected);

  StartSmartConfig();
}

void StartSmartConfig(void)
{

  // if (DEBUG_MODE) {
    Serial.println("Start Smart Config");
  // }
  ulSmartConfigFinished = 0;
  ulCC3000Connected = 0;
  ulCC3000DHCP = 0;
  OkToDoShutDown=0;

  // Reset all the previous configuration
  wlan_ioctl_set_connection_policy(DISABLE, DISABLE, DISABLE);

  // if (wlan_ioctl_set_connection_policy(DISABLE, DISABLE, DISABLE) != 0) {
  //   digitalWrite(ErrorLED, HIGH);
  //   return;
  // }

  wlan_ioctl_del_profile(255);
  // if (wlan_ioctl_del_profile(255) != 0) {
  //   digitalWrite(ErrorLED, HIGH);
  //   return;
  // }

  //Wait until CC3000 is disconnected
  while (ulCC3000Connected == 1)
  {
    delayMicroseconds(100);
  }

  // Serial.println("waiting for disconnect");

  // Trigger the Smart Config process
  // Start blinking LED6 during Smart Configuration process
  digitalWrite(ConnLED, HIGH);  
  wlan_smart_config_set_prefix((char*)aucCC3000_prefix);
  
  // if (wlan_smart_config_set_prefix((char*)aucCC3000_prefix) != 0){
  //   digitalWrite(ErrorLED, HIGH);
  //   return;
  // }
  Serial.println("set prefix");
  digitalWrite(ConnLED, LOW);

  // Start the SmartConfig start process
  wlan_smart_config_start(0);
  // if (wlan_smart_config_start(0) != 0){
  //   digitalWrite(ErrorLED, HIGH);
  //   return;
  // }
  Serial.println("smart config start");

  digitalWrite(ConnLED, HIGH);
  
  // Wait for Smartconfig process complete
  while (ulSmartConfigFinished == 0)
  {
    delay(500);
    digitalWrite(ConnLED, LOW);
    delay(500);
    digitalWrite(ConnLED, HIGH);
  }

  Serial.println("smart config finished");

  digitalWrite(ConnLED, LOW);


  
  // Configure to connect automatically to the AP retrieved in the 
  // Smart config process. Enabled fast connect.
  wlan_ioctl_set_connection_policy(DISABLE, DISABLE, ENABLE);
  // if (wlan_ioctl_set_connection_policy(DISABLE, DISABLE, ENABLE) != 0){
  //   digitalWrite(ErrorLED, HIGH);
  //   return;
  // }

  // reset the CC3000
  wlan_stop();

  delayMicroseconds(600);
  wlan_start(0);

  // Mask out all non-required events
  wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE|HCI_EVNT_WLAN_UNSOL_INIT|HCI_EVNT_WLAN_ASYNC_PING_REPORT);
  // if (DEBUG_MODE) {
    Serial.print("Config done");
  // }
  
}


int test(void)
{
  
  SpiInit();

  delayMicroseconds(100);
  initialize();

  Serial.println("done testing");
  return(0);
}
