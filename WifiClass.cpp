#include <stdio.h>

#include "CC3000.h"
#include "tm_net.h"
#include "tm_debug.h"
#include "utility/arduino_wl_definitions.h"
#include "utility/arduino_wl_types.h"

WiFiClass::WiFiClass()
{
  // Driver initialization

  init();
}

void WiFiClass::init()
{
}

uint8_t WiFiClass::getSocket()
{
  // Not implemented
  return NO_SOCKET_AVAIL;
}

char *WiFiClass::firmwareVersion()
{
  // uint8_t major, minor;
  // tm_net_firmware_version(&major, &minor);
  snprintf(_firmware_version, 8, "%d.%d", 0, 0);
  return _firmware_version;
}

void WiFiClass::begin()
{
  tm_net_initialize();
  delay(500);
}

int WiFiClass::begin(char* ssid)
{
  // TODO
  return -1;
}

int WiFiClass::begin(char* ssid, uint8_t key_idx, const char *key)
{
  // TODO
  return -1;
}

int WiFiClass::begin(char* ssid, const char *passphrase)
{
  tm_net_initialize();
  tm_net_set_policy();
  
  uint8_t status = WL_IDLE_STATUS;
  uint8_t attempts = WL_MAX_ATTEMPT_CONNECTION;

  if (tm_net_connect_wpa2(ssid, passphrase) == 0) {
    tm_net_block_until_dhcp();
    status = WL_CONNECTED;
  } else {
    status = WL_CONNECT_FAILED;
  }
  return status;
}

int WiFiClass::beginSmartConfig()
{
  tm_net_initialize();
  tm_net_smartconfig_initialize();
  tm_net_block_until_dhcp();
  return WL_CONNECTED;
}
// void WiFiClass::config(IPAddress local_ip)
// {
//   WiFiDrv::config(1, (uint32_t)local_ip, 0, 0);
// }

// void WiFiClass::config(IPAddress local_ip, IPAddress dns_server)
// {
//   WiFiDrv::config(1, (uint32_t)local_ip, 0, 0);
//   WiFiDrv::setDNS(1, (uint32_t)dns_server, 0);
// }

// void WiFiClass::config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway)
// {
//   WiFiDrv::config(2, (uint32_t)local_ip, (uint32_t)gateway, 0);
//   WiFiDrv::setDNS(1, (uint32_t)dns_server, 0);
// }

// void WiFiClass::config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet)
// {
//   WiFiDrv::config(3, (uint32_t)local_ip, (uint32_t)gateway, (uint32_t)subnet);
//   WiFiDrv::setDNS(1, (uint32_t)dns_server, 0);
// }

// void WiFiClass::setDNS(IPAddress dns_server1)
// {
//   WiFiDrv::setDNS(1, (uint32_t)dns_server1, 0);
// }

// void WiFiClass::setDNS(IPAddress dns_server1, IPAddress dns_server2)
// {
//   WiFiDrv::setDNS(2, (uint32_t)dns_server1, (uint32_t)dns_server2);
// }

int WiFiClass::disconnect()
{
  tm_net_disconnect();
  return 0;
}

uint8_t* WiFiClass::macAddress(uint8_t* mac)
{
  // tm_net_mac(mac);
  return mac;
}
   
IPAddress WiFiClass::localIP()
{
  IPAddress ret;
  uint32_t ip = tm_net_local_ip();
  ret[0] = (ip >> 24) & 0xFF;
  ret[1] = (ip >> 16) & 0xFF;
  ret[2] = (ip >> 8) & 0xFF;
  ret[3] = (ip >> 0) & 0xFF;
  return ret;
}

// IPAddress WiFiClass::subnetMask()
// {
//   IPAddress ret;
//   WiFiDrv::getSubnetMask(ret);
//   return ret;
// }

// IPAddress WiFiClass::gatewayIP()
// {
//   IPAddress ret;
//   WiFiDrv::getGatewayIP(ret);
//   return ret;
// }

char *WiFiClass::SSID()
{
  tm_net_ssid(_ssid_ref);
  return _ssid_ref;
}

// uint8_t* WiFiClass::BSSID(uint8_t* bssid)
// {
//   uint8_t* _bssid = WiFiDrv::getCurrentBSSID();
//   memcpy(bssid, _bssid, WL_MAC_ADDR_LENGTH);
//     return bssid;
// }

int32_t WiFiClass::RSSI()
{
  // return tm_net_rssi(ssid());
  // TODO
  return 0;
}

// uint8_t WiFiClass::encryptionType()
// {
//     return WiFiDrv::getCurrentEncryptionType();
// }


// int8_t WiFiClass::scanNetworks()
// {
//   uint8_t attempts = 10;
//   uint8_t numOfNetworks = 0;

//   if (WiFiDrv::startScanNetworks() == WL_FAILURE)
//     return WL_FAILURE;
//   do
//   {
//     delay(2000);
//     numOfNetworks = WiFiDrv::getScanNetworks();
//   }
//   while (( numOfNetworks == 0)&&(--attempts>0));
//   return numOfNetworks;
// }

// char* WiFiClass::SSID(uint8_t networkItem)
// {
//   // TODO
//   return _ssid_buffer;
//   // return WiFiDrv::getSSIDNetoworks(networkItem);
// }

// int32_t WiFiClass::RSSI(uint8_t networkItem)
// {
//   // TODO
//   return 0;
//   // return WiFiDrv::getRSSINetoworks(networkItem);
// }

// uint8_t WiFiClass::encryptionType(uint8_t networkItem)
// {
//     return WiFiDrv::getEncTypeNetowrks(networkItem);
// }

uint8_t WiFiClass::status()
{
  return tm_net_is_connected() && tm_net_has_ip() ? WL_CONNECTED : WL_IDLE_STATUS;
}

int WiFiClass::hostByName(const char* aHostname, IPAddress& aResult)
{
  // TODO
  return -1;
  //return WiFiDrv::getHostByName(aHostname, aResult);
}

WiFiClass WiFi;
