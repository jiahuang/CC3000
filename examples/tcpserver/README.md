#CC3000 TCP Server

This example sends analog readings over to a client (web browser, netcat, etc)

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

Listening on the Arduino serial output should give you

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

After the DHCP has been set, navigate to the IP that it gives you on a browser. From your browser you should see something like

```
analog input 0 is 462
analog input 1 is 375
analog input 2 is 331
analog input 3 is 301
analog input 4 is 277
analog input 5 is 280
```

Once a client connnects, the Arduino should output some log messages

```
new client
GET / HTTP/1.1
Host: 192.168.1.20
Connection: keep-alive
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_8_3) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/27.0.1453.116 Safari/537.36
Accept-Encoding: gzip,deflate,sdch
Accept-Language: en-US,en;q=0.8

client disonnected
new client
GET /favicon.ico HTTP/1.1
Host: 192.168.1.20
Connection: keep-alive
Accept: */*
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_8_3) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/27.0.1453.116 Safari/537.36
Accept-Encoding: gzip,deflate,sdch
Accept-Language: en-US,en;q=0.8
```
