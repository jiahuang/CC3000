#ifndef wifiserver_h
#define wifiserver_h

extern "C" {
  #include "utility/arduino_wl_definitions.h"
  #include <utility/socket.h>
}

#include "Server.h"
#include "WiFiClient.h"

class WiFiServer : public Server {
private:
  uint16_t _port;
  uint8_t _sock;
  void*     pcb;

public:
  WiFiServer(uint16_t);
  WiFiServer(uint8_t, uint16_t);
  WiFiClient available(uint8_t* status = NULL);
  void begin();
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);
  uint8_t status();

  using Print::write;
};

#endif
