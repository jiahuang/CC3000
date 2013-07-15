/*
 * cc3000 examples
 */
 
#include <SPI.h>
#include <CC3000.h>
#include <utility/debug.h>
#include <utility/socket.h>

void setup() {
//  pinMode(13, OUTPUT);
//  digitalWrite(13, LOW);
  Serial.begin(9600);
  Serial.println(F("Send \"C\" to run.\n")); 
}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void loop ()
{ 
  if ( Serial.available() )
  { 
    char c = toupper(Serial.read());
    
    if (c == 'C') {
      Serial.println(F("Connecting..."));
      cc_initialize();
      cc_block_until_dhcp();
      Serial.println(F("\nOptions:\n"
        "  S   Send UDP packet\n"
        "  R   Receive UDP packet\n"
        "  G   Open TCP server\n"
        "  T   Send TCP request\n"));
    }
    
    else if ( c == 'S' )
    {
      // Send UDP packet
      long socket = cc_udp_open_socket();
      Serial.println(F("S"));
      cc_udp_send(socket, 10, 1, 90, 138, 4444, (uint8_t *) "haha", 4);
      socket = cc_udp_close_socket(socket);
      Serial.println(F("Sent"));
    }
    
    else if ( c == 'R') {
      // Receive UDP packet
      Serial.println(F("R"));
      long socket = cc_udp_open_socket();
      cc_udp_listen(socket, 4444);
      const char *ret = receiveUDP(socket);
      while(*ret != '\0'){
        Serial.print(*ret++);
      }
      socket = cc_udp_close_socket(socket);
    }
    
    else if ( c == 'G')
    {
      // Start TCP Server.
      CC_DEBUG(">>> START TCP SERVER\n");
      int socket = cc_tcp_open_socket();
      cc_tcp_listen(socket, 4444);
    
      // Wait for client.
      sockaddr addrClient;
      socklen_t addrLen;
      while (true) {
        long client = cc_tcp_block_until_accepting(socket, &addrClient, &addrLen);
        delayMicroseconds(100);
        
        // Negative client may imply closed socket.
        CC_DEBUG("Client discovered: %ld\n", client);
        if (client < 0) {
          break;
        }
      
        // Read incoming.
        cc_tcp_read_dump(client);
      
        // Output simple response.
        const char *res = "HTTP/1.0 200 OK\r\nContent-Length: 19\r\nContent-Type: text/html\r\n\r\n<h1>HI ERRBODY</h1>";
        int sendStatus = cc_tcp_write(client, (uint8_t *) res, strlen(res));
        
        // Close client.
        cc_tcp_close_socket(client);
        delayMicroseconds(100);
      }
    
      socket = cc_tcp_close_socket(socket);
      CC_DEBUG("<<< END TCP SERVER\n");
    }
    
    // Request TCP
    else if ( c == 'T')
    {
      CC_DEBUG(">>> START TCP REQUEST\n");
      
      long socket = cc_tcp_open_socket();
      CC_DEBUG("Socket #%d\n", socket);
      
//      char *hostname = "www.google.com";
//      long unsigned ip = 0;
//      int gethostret = -1;
//      while (gethostret < 0 && ip == 0) {
//        gethostret = gethostbyname(hostname, strlen(hostname), &ip);
//        CC_DEBUG("Host lookup returned %d, ip is %x\n", gethostret, ip);
//        delay(1000);
//      }
      
      //long unsigned ip = (10 << 24) + (1 << 16) + (90 << 8) + (138 << 0);
      const char *req = "GET / HTTP/1.0\r\nAccept: *" "/" "*\r\n\r\n";
      cc_tcp_connect(socket, 10, 1, 90, 138, 4444);
      cc_tcp_write(socket, (uint8_t *) req, strlen(req));
      cc_tcp_read_dump(socket);
      
      socket = cc_tcp_close_socket(socket);
      CC_DEBUG("<<< END TCP REQUEST\n");
    }
    
    else {
      return;
    }
    
    Serial.print(F("Available RAM: "));
    Serial.println(freeRam(), DEC);
  }
}