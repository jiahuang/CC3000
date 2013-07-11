/*
  smart config example
 */
#include <SPI.h>
#include <cc3000.h>

#define SmartConfig 6
int SmartConfigProcess = 0;
void setup() {
//  pinMode(2, OUTPUT);

  Serial.begin(9600);
  delayMicroseconds(100);
  Serial.println("setting up"); 
//  test();
//  StartSmartConfig();
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
    } else if ( c == 'X') {
      Serial.println("smart config");
      StartSmartConfig();
    }
  } 

}