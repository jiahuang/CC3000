/*
  cc3000 example
 */
#include <SPI.h>
//#include <cc3000old.h>
#include <cc3000.h>

void setup() {
//  pinMode(13, OUTPUT);
//  digitalWrite(13, LOW);
  Serial.begin(9600);
  Serial.println("setting up"); 
  
}

void loop() {
 if ( Serial.available() )
  {
    char c = toupper(Serial.read());
    if (c == 'C'){
      Serial.println("connecting");
      test();
    } else if ( c == 'S' )
    {
      connectUDP();
      Serial.println("S");
      sendUDP();
      closeUDP();
    } else if ( c == 'R') {
      Serial.println("R");
      connectUDP();
      listenUDP();
      const char *ret = receiveUDP();
      while(*ret != '\0'){
        Serial.print(*ret++);
      }
    }
  }
}