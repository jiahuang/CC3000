#ifndef __CC3000_H__
#define __CC3000_H__

#include <inttypes.h>

extern "C" {
  #include "utility/arduino_wl_definitions.h"
  #include "utility/arduino_wl_types.h"
}

#include "IPAddress.h"
#include "WifiClass.h"
#include "WiFiDatagram.h"
#include "WiFiClient.h"
#include "WiFiServer.h"

#define NO_SOCKET_AVAIL  255
#define CLOSED 255

#endif