#include <Arduino.h>
#include <string.h>
#include "utility/cc3000_spi.h"
#include "utility/nvmem.h"
#include "utility/wlan.h"
#include "utility/hci.h"
#include "cc3000.h"

char ssid[] = "HCPGuest";                     // your network SSID (name) 
unsigned char keys[] = "kendall!";       // your network key
int connected = -1;

void printVersion(void){
  Serial.print("version: ");
  unsigned char version[3];
  if (!nvmem_read_sp_version(version))
  {
    Serial.print(version[0]);
    Serial.print(".");
    Serial.println(version[1]);
    Serial.print(".");
    Serial.print(version[2]);

  } else {
    Serial.println("Failed to read version");
  }
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

void initialize(void){
  Serial.println("Calling wlan_init");
  wlan_init(CC3000_UsynchCallback, NULL, NULL, NULL, ReadWlanInterruptPin, 
    WlanInterruptEnable, WlanInterruptDisable, WriteWlanPin);

  Serial.println("Calling wlan_start...");
  wlan_start(0);

  printVersion();
  printMAC();
  
  Serial.println("setting event mask");
  wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE|HCI_EVNT_WLAN_UNSOL_INIT|HCI_EVNT_WLAN_ASYNC_PING_REPORT);

  Serial.println("config wlan");
  wlan_ioctl_set_connection_policy(0, 0, 0);

  Serial.println("Attempting to connect...");
  connected = wlan_connect(WLAN_SEC_WPA2,ssid,8, 0, keys, 8);
  Serial.println(connected);
}

int test(void)
{
  
  SpiInit();

  delayMicroseconds(100);
  initialize();

  Serial.println("done testing");
  return(0);
}
