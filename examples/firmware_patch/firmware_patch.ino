#include <SPI.h>
#include <CC3000.h>

/* Do the driver patch before the firmware patch
 */

int patched = 0;
void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
  Serial.println("Type 'X' to begin firmware patch.");
  Serial.println("Do the driver patch before the firmware patch.");

}

void loop() {
  if ( Serial.available() )
  { 
    char c = toupper(Serial.read());
    
    if (c == 'X' && patched == 0) {
      patched = 1;
      Serial.println("Beginning firmware patch...");
      firmware_patch();
      Serial.println("Done with firmware patch");
    } else if (c == 'X' && patched == 1){
      Serial.println("Already patched");
    }
  }
}
