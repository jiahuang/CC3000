extern "C" {
  #include "utility/arduino_wl_definitions.h"
  #include "utility/arduino_wl_types.h"
  #include "socket.h"
  #include "string.h"
  #include "tm_debug.h"
}

#include "CC3000.h"
#include "WiFiClient.h"
// #include "WiFiServer.h"


uint16_t WiFiClient::_srcport = 1024;

WiFiClient::WiFiClient() : _sock(NO_SOCKET_AVAIL) {
}

WiFiClient::WiFiClient(uint8_t sock) : _sock(sock) {
}

int WiFiClient::connect (const char* host, uint16_t port)
{
  Serial.println("Hostname connecting.");
// 	IPAddress remote_addr;
// 	if (WiFi.hostByName(host, remote_addr))
// 	{
// 		return connect(remote_addr, port);
// 	}
	return 0;
}

int WiFiClient::connect (IPAddress ip, uint16_t port)
{
  _sock = tm_net_tcp_open_socket();
  if (_sock == NO_SOCKET_AVAIL) {
    Serial.println("No Socket available");
    return 0;
  }

  if (tm_net_tcp_connect(_sock, ip[0], ip[1], ip[2], ip[3], port) != 0) {
    return 0;
  }

  return 1;
}

size_t WiFiClient::write (uint8_t b)
{
	return write(&b, 1);
}

size_t WiFiClient::write (const uint8_t *buf, size_t size)
{
  if (_sock == NO_SOCKET_AVAIL) {
	  setWriteError();
	  return 0;
  }
  
  if (size == 0) {
	  setWriteError();
    return 0;
  }

  if ((size = tm_net_tcp_write(_sock, (uint8_t *) buf, size)) == 0) {
    setWriteError();
    return 0;
  }

  return size;
}

int WiFiClient::available () {
  return tm_net_is_readable(_sock);
}

int WiFiClient::read ()
{
  uint8_t b;
  if (!available()) {
    return -1;
  }

  tm_net_tcp_read(_sock, &b, 1);
  return b;
}


int WiFiClient::read (uint8_t* buf, size_t size)
{
  if (!tm_net_tcp_read(_sock, buf, size)) {
    return -1;
  }
  return 0;
}

int WiFiClient::peek() {
  Serial.println("Peek not implemented.");
  // uint8_t b;
  // if (!available())
    return -1;

  // ServerDrv::getData(_sock, &b, 1);
  // return b;
}

void WiFiClient::flush ()
{
  while (available()) {
    read();
  }
}

void WiFiClient::stop()
{
  if (_sock == NO_SOCKET_AVAIL) {
    return;
  }

  tm_net_tcp_close_socket(_sock);
  _sock = NO_SOCKET_AVAIL;
}

uint8_t WiFiClient::connected ()
{
  return _sock != NO_SOCKET_AVAIL;
}

uint8_t WiFiClient::status ()
{
  if (_sock == NO_SOCKET_AVAIL) {
    return CLOSED;
  } else {
    return 1;
  }
}

WiFiClient::operator bool() {
  return _sock != NO_SOCKET_AVAIL;
}
