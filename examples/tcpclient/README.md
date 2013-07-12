#CC3000 TCP Client

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

The arduino should send the following:

```
Send anything.
```

Type in anything in order to start up the example. Afterwards you should see the following as the CC3000 attemps to connect

```
Attempting to connect to SSID: redchairs
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
Connected to wifi
SSID: redchairs
IP Address: 192.168.1.20
signal strength (RSSI):0 dBm
```

After this, the CC3000 will begin to stream the html page from Google

```
Starting connection to server...
connected to server
HTTP/1.1 200 OK
Date: Fri, 12 Jul 2013 13:51:47 GMT
Expires: -1
Cache-Control: private, max-age=0
Content-Type: text/html; charset=ISO-8859-1
Set-Cookie: PREF=ID=20fb20cf4469a73d:FF=0:TM=1373637107:LM=1373637107:S=hVab2L0C87ZeLavo; expires=Sun, 12-Jul-2015 13:51:47 GMT; path=/; domain=.google.com
Set-Cookie: NID=67=L-1tQA0YryzvZDkApc53UmaUs49moJukLl3UH0CLWIWTvEM8hhF2-23mP4rHZtgf42bUe_mKsuzswLCyBMJMeePd5-jL5TI74_DhedcgxXDANJrLQtWl4_Gwg4MEkJ_B; expires=Sat, 11-Jan-2014 13:51:47 GMT; path=/; domain=.google.com; HttpOnly
P3P: CP="This is not a P3P policy! See http://www.google.com/support/accounts/bin/answer.py?hl=en&answer=151657 for more info."
Server: gws
X-XSS-Protection: 1; mode=block
X-Frame-Options: SAMEORIGIN
Connection: close

<!doctype html><html itemscope="itemscope" itemtype="http://schema.org/WebPage"><head><meta http-equiv="Content-Type" content="text/html; charset=UTF-8"><meta itemprop="image" content="/images/google_favicon_128.png"><title>arduino - Google Search</title>...
```