/*****************************************************************************
* 
*
*
*           Example Program
*
*
*
*****************************************************************************/


#include "wlan.h" 
#include "spi.h"


char ssid[] = "yourNetwork";                     // your network SSID (name) 
char key[] = "D0D0DEADF00DABBADEAFBEADED";       // your network key
int keyIndex = 0;                                // your network key Index number

main(void)
{

	//
	//  Board Initialization
	//
	init_spi();

    //
    // Initialize WLAN. Needs to be run before anything else
    //
    wlan_init( CC3000_UsynchCallback, NULL, NULL, NULL, ReadWlanIRQ, WlanIRQEnable, WlanIRQDisable, WriteWlanCCEN);
    
    //
    // Trigger a WLAN device
    //
    wlan_start(0);

    //
    // Mask out all non-required events from CC3000
    //
    wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE|HCI_EVNT_WLAN_UNSOL_INIT|HCI_EVNT_WLAN_ASYNC_PING_REPORT);


    wlan_connect(ssid,ssidLen,key,key_len);


}

