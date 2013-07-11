#include <SPI.h>
#include <cc3000.h>
/*
 * This is a UDP example for the cc3000
 * In order to test this:
 * 1. fill in the ssid (network name)
 * 2. fill in the network password
 * 3. Wait until the Connection LED turns on 
 * 4. You should get an output that looks like
 *    DHCP Connected with IP: 10.1.90.49
 * 5. Use netcat to open up a UDP connection. Type in the following into the terminal
 *    terminal >> nc -u 10.1.90.49 2390
 *    where 2390 is the specified localPort (2390 by default)
 * 6. Type into the terminal once the UDP connection is opened. Mesages should flow between the cc3000 and your computer
 *    terminal >> nc -u 10.1.90.49 2390
 *                okok
 *                acknowledged
 *    cc3000 >> Recieved 4 UDP bytes
 *              Received packet from 10.1.90.138, port 4444
 *              Contents:
 *              okok
 */
int status = WL_IDLE_STATUS;
char ssid[] = "...";                 // your network SSID (name) 
char pass[] = "...";                 // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                    // your network key Index number (needed only for WEP)

unsigned int localPort = 2390;       // local port to listen on

char inBuffer[255];                  // buffer to hold incoming packet
char outBuffer[] = "acknowledged";   // a string to send back

WiFiDatagram Udp;

void setup ()
{
  //Initialize serial and wait for port to open:
  Serial.begin(9600); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present"); 
    // don't continue:
    while(true);
  } 
  
  // attempt to connect to Wifi network:
  // select smart config or regular connection
  while ( status != WL_CONNECTED) { 
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:    
    status = WiFi.begin(ssid, pass);
    
    // uncomment this if you want to auto start smart config
    // status = WiFi.beginSmartConfig();
  } 
  
  Serial.println("Connected to wifi");
  printWifiStatus();
  
  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  Udp.begin(localPort);  

}

void loop ()
{    
  // if there's data available, read a packet
  if (Udp.available())
  {   
    // read the packet into packetBufffer
    int len = Udp.receive(inBuffer,255);
    if (len >0) inBuffer[len]=0;
    
    Serial.print("Received packet from ");
    IPAddress remoteIp = Udp.remoteIP();
    Serial.print(remoteIp);
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    Serial.println("Contents:");
    Serial.println(inBuffer);
    
    // send a reply, to the IP address and port that sent us the packet we received
    Udp.send(Udp.remoteIP(), Udp.remotePort(), outBuffer, strlen(outBuffer));
   }
}


void printWifiStatus ()
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}