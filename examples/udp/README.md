#CC3000 UDP Example

This example makes the CC3000 connect to Google and returns the webpage

## Setup

The CC3000 can connect with smartconfig or by entering in the network credentials. The default example sets it up using smartconfig.

### SmartConfig

1. Download the SmartConfig app. 
	* iphone: search TI cc3000 in the app store
	* android: gotta build your own app from [http://processors.wiki.ti.com/index.php/CC3000_First_Time_Configuration](http://processors.wiki.ti.com/index.php/CC3000_First_Time_Configuration)
2. Fill in your network credntials and password
3. Device name is CC3000
4. Arduinos can't run AES encryption, so don't check the encryption key

### Network credentials

1. Fill in the `ssid` and `pass` with your network credentials.
2. Uncomment out the `WiFi.begin(ssid, pass);`
3. Comment out the `WiFi.begin()` line

## Example output
When the Arduino connects, it should output something like

```
Calling wlan_init
Calling wlan_start...
SpiOpen
Completed SpiOpen
SpiFirstWrite
setting event mask
Firmware version: 1.19
Attempting to connect...
Connected: 0
disconnected
connected
dhcp
DHCP Connected with IP: 192.168.1.20
Binding local socket...
Listening on local socket...
SSID: redchairs
IP Address: 192.168.1.20
signal strength (RSSI):0 dBm
```

Use netcat in order to start up a UDP socket. On the Ip address and the port specified in the example (default is 2390)

```
>>> nc -ul 192.168.1.20 2390
```

Once netcat has connected, you should be able to send packets back and forth from your computer to the CC3000.
