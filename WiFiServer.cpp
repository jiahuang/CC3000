#include <string.h>

extern "C" {
  #include "tm_debug.h"
}

#include "CC3000.h"
#include "WiFiClient.h"
#include "WiFiServer.h"

WiFiServer::WiFiServer(uint8_t sock, uint16_t port) : _sock(sock)
{
  _port = port;
}

WiFiServer::WiFiServer(uint16_t port) : _sock(NO_SOCKET_AVAIL)
{
  _port = port;
}

void WiFiServer::begin()
{
  _sock = tm_net_tcp_open_socket();
  if (_sock != NO_SOCKET_AVAIL) {
    tm_net_tcp_listen(_sock, _port);
  }
}

WiFiClient WiFiServer::available(byte* status)
{
  sockaddr addrClient;
  socklen_t addrLen;
  int clientsock = tm_net_tcp_block_until_accepting(_sock, &addrClient, &addrLen);
  delayMicroseconds(100);

  // Negative client may imply closed socket.
  if (clientsock < 0) {
    _sock = tm_net_tcp_close_socket(_sock);
    begin();
    return available(status);
  }

  return WiFiClient(clientsock);
}

uint8_t WiFiServer::status()
{
  // TODO
  return 1;
}

size_t WiFiServer::write (uint8_t b)
{
  return write(&b, 1);
}

size_t WiFiServer::write (const uint8_t *buffer, size_t size)
{
  // TODO
  return 0;
}
