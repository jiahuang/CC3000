
extern "C" {
  #include "tm_debug.h"
}
#include <string.h>

#include "cc3000.h"
#include "tm_net.h"
#include "WiFiUDPQueue.h"

/* Constructor */
WiFiUDPQueue::WiFiUDPQueue() : _sock(NO_SOCKET_AVAIL) {}

/* Start WiFiUDPQueue socket, listening at local port PORT */
uint8_t WiFiUDPQueue::begin(uint16_t port)
{
  _sock = tm_net_udp_open_socket();
  if (_sock >= 0 && tm_net_udp_listen(_sock, port) == 0) {
    _port = port;
    return 1;
  }
  _sock = NO_SOCKET_AVAIL;
  return 0;
}

/* return number of bytes available in the current packet,
   will return zero if parsePacket hasn't been called yet */
int WiFiUDPQueue::available()
{
  if (_sock != NO_SOCKET_AVAIL) {
    return tm_net_is_readable(_sock);
  }
  return 0;
}

/* Release any resources being used by this WiFiUDPQueue instance */
void WiFiUDPQueue::stop()
{
  if (_sock == NO_SOCKET_AVAIL)
    return;

  tm_net_udp_close_socket(_sock);

  _sock = NO_SOCKET_AVAIL;
}

size_t WiFiUDPQueue::send(const char *host, uint16_t port, const unsigned char *buffer, size_t size)
{
  // Look up the host first
  int ret = 0;
  IPAddress remote_addr;
  if (WiFi.hostByName(host, remote_addr))
  {
    ret = send(remote_addr, port, buffer, size);
  }
  return ret;
}

size_t WiFiUDPQueue::send(IPAddress ip, uint16_t port, const unsigned char *buffer, size_t size)
{
  return tm_net_udp_send(_sock, ip[0], ip[1], ip[2], ip[3], port, (uint8_t *) buffer, size);
}

size_t WiFiUDPQueue::receive(unsigned char* buffer, size_t len)
{
  if (available()) {
    sockaddr from;
    socklen_t from_len;
    int recv_len = 0;
    if ((recv_len = tm_net_udp_receive(_sock, buffer, len, &from, &from_len))) {
      _remoteIP[0] = from.sa_data[2];
      _remoteIP[1] = from.sa_data[3];
      _remoteIP[2] = from.sa_data[4];
      _remoteIP[3] = from.sa_data[5];
      _remotePort = ((long) from.sa_data[0] << 8) + (long) from.sa_data[1];
      return recv_len;
    }
  }
  return -1;
}

IPAddress  WiFiUDPQueue::remoteIP()
{
  return _remoteIP;
}

uint16_t  WiFiUDPQueue::remotePort()
{
  return _remotePort;
}
