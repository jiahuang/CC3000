/*
  cc3000 example
 */
#include <SPI.h>
#include <cc3000.h>


void setup() {
  Serial.begin(9600);
  Serial.println("setting up");
  test();
}

void loop() {
 if ( Serial.available() )
  {
    char c = toupper(Serial.read());
    if ( c == 'S' )
    {
      Serial.println("S");
      sendUDP();
      //StartSmartConfig();
    }
  }
}