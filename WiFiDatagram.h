#ifndef wifiudp_h
#define wifiudp_h

#include "IPAddress.h"

#define UDP_TX_PACKET_MAX_SIZE 24

class WiFiDatagram {
private:
  uint8_t _sock;  // socket ID
  uint16_t _port; // local port to listen on

  IPAddress _remoteIP;
  uint16_t _remotePort;

public:
  WiFiDatagram();  // Constructor
  virtual uint8_t begin(uint16_t);  // initialize, start listening on specified port. Returns 1 if successful, 0 if there are no sockets available to use
  virtual void stop();  // Finish with the UDP socket

  // Sending UDP packets
  
  // Start building up a packet to send to the remote host specific in ip and port
  // Returns 1 if successful, 0 if there was a problem with the supplied IP address or port
  virtual size_t send(IPAddress ip, uint16_t port, const unsigned char *buffer, size_t size);
  virtual size_t send(IPAddress ip, uint16_t port, const char *buffer, size_t size)
    { return send(ip, port, (unsigned char*)buffer, size); };
  // Start building up a packet to send to the remote host specific in host and port
  // Returns 1 if successful, 0 if there was a problem resolving the hostname or port
  virtual size_t send(const char *host, uint16_t port, const unsigned char *buffer, size_t size);
  virtual size_t send(const char *host, uint16_t port, const char *buffer, size_t size)
    { return send(host, port, (unsigned char*)buffer, size); };

  // Start processing the next available incoming packet
  // Returns number of bytes remaining in the current packet
  virtual int available();
  // Read up to len bytes from the current packet and place them into buffer
  // Returns the number of bytes read, or 0 if none are available
  virtual size_t receive(unsigned char* buffer, size_t len);
  // Read up to len characters from the current packet and place them into buffer
  // Returns the number of characters read, or 0 if none are available
  virtual size_t receive(char* buffer, size_t len)
    { return receive((unsigned char*)buffer, len); };

  // Return the IP address of the host who sent the current incoming packet
  virtual IPAddress remoteIP();
  // Return the port of the host who sent the current incoming packet
  virtual uint16_t remotePort();
};

#endif