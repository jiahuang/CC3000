#include <Arduino.h>
#include <SPI.h>
#include "cc3000.h"          
#include "utility/wlan.h"


#include "utility/hci.h"
#include "utility/os.h"
#include "utility/evnt_handler.h"


#define READ                    0x03
#define WRITE                   0x01

#define HI(value)               (((value) & 0xFF00) >> 8)
#define LO(value)               ((value) & 0x00FF)

// #define ASSERT_CS()          (P1OUT &= ~BIT3)

// #define DEASSERT_CS()        (P1OUT |= BIT3)

#define HEADERS_SIZE_EVNT       (SPI_HEADER_SIZE + 5)

#define SPI_HEADER_SIZE			(5)

#define 	eSPI_STATE_POWERUP 				 (0)
#define 	eSPI_STATE_INITIALIZED  		 (1)
#define 	eSPI_STATE_IDLE					 (2)
#define 	eSPI_STATE_WRITE_IRQ	   		 (3)
#define 	eSPI_STATE_WRITE_FIRST_PORTION   (4)
#define 	eSPI_STATE_WRITE_EOT			 (5)
#define 	eSPI_STATE_READ_IRQ				 (6)
#define 	eSPI_STATE_READ_FIRST_PORTION	 (7)
#define 	eSPI_STATE_READ_EOT				 (8)

#define CC3000_nIRQ 	(2)
#define HOST_nCS		(10)
#define HOST_VBAT_SW_EN (9)
#define LED 			(6)

#define DEBUG_MODE		(1)


//foor spi bus loop
int loc = 0; 

char ssid[] = "KYD03";                     // your network SSID (name) 
unsigned char keys[] = "bellacody";       // your network key
unsigned char bssid[] = "KYD03";       // your network key
int keyIndex = 0; 


typedef struct
{
	gcSpiHandleRx  SPIRxHandler;
	unsigned short usTxPacketLength;
	unsigned short usRxPacketLength;
	unsigned long  ulSpiState;
	unsigned char *pTxPacket;
	unsigned char *pRxPacket;

}tSpiInformation;


tSpiInformation sSpiInformation;

//
// Static buffer for 5 bytes of SPI HEADER
//
unsigned char tSpiReadHeader[] = {READ, 0x00, 0x00};


char SpiWriteDataSynchronous(unsigned char *data, unsigned short size);
void SpiWriteAsync(const unsigned char *data, unsigned short size);
void SpiPauseSpi(void);
void SpiResumeSpi(void);
void SSIContReadOperation(void);
void SpiReadHeader(void);


// The magic number that resides at the end of the TX/RX buffer (1 byte after the allocated size)
// for the purpose of detection of the overrun. The location of the memory where the magic number 
// resides shall never be written. In case it is written - the overrun occured and either recevie function
// or send function will stuck forever.
#define CC3000_BUFFER_MAGIC_NUMBER (0xDE)

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//#pragma is used for determine the memory location for a specific variable.                            ///        ///
//__no_init is used to prevent the buffer initialization in order to prevent hardware WDT expiration    ///
// before entering to 'main()'.                                                                         ///
//for every IDE, different syntax exists :          1.   __CCS__ for CCS v5                    ///
//                                                  2.  __IAR_SYSTEMS_ICC__ for IAR Embedded Workbench  ///
// *CCS does not initialize variables - therefore, __no_init is not needed.                             ///
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// #ifdef __CCS__
// #pragma DATA_SECTION(spi_buffer, ".FRAM_DATA")
// char spi_buffer[CC3000_RX_BUFFER_SIZE];

// #elif __IAR_SYSTEMS_ICC__
// #pragma location = "FRAM_DATA"
// __no_init char spi_buffer[CC3000_RX_BUFFER_SIZE];
// #endif



//#ifdef __CCS__
//#pragma DATA_SECTION(wlan_tx_buffer, ".FRAM_DATA")
unsigned char wlan_tx_buffer[CC3000_TX_BUFFER_SIZE];
unsigned char spi_buffer[CC3000_RX_BUFFER_SIZE];


//#elif __IAR_SYSTEMS_ICC__
//#pragma location = "FRAM_DATA"
//__no_init unsigned char wlan_tx_buffer[CC3000_TX_BUFFER_SIZE];
//#endif
//*****************************************************************************
// 
//!  This function get the reason for the GPIO interrupt and clear cooresponding
//!  interrupt flag
//! 
//!  \param  none
//! 
//!  \return none
//! 
//!  \brief  This function This function get the reason for the GPIO interrupt
//!          and clear cooresponding interrupt flag
// 
//*****************************************************************************
void
SpiCleanGPIOISR(void)
{
	if (DEBUG_MODE)
	{
		Serial.println("SpiCleanGPIOISR");
	}

	//add code
}
 


//*****************************************************************************
//
//!  SpiClose
//!
//!  \param  none
//!
//!  \return none
//!
//!  \brief  Cofigure the SSI
//
//*****************************************************************************
void
SpiClose(void)
{
	if (DEBUG_MODE)
	{
		Serial.println("SpiClose");
	}

	if (sSpiInformation.pRxPacket)
	{
		sSpiInformation.pRxPacket = 0;
	}

	// //
	// //	Disable Interrupt in GPIOA module...
	// //
	tSLInformation.WlanInterruptDisable();
}


//*****************************************************************************
//
//!  SpiClose
//!
//!  \param  none
//!
//!  \return none
//!
//!  \brief  Cofigure the SSI
//
//*****************************************************************************
void SpiOpen(gcSpiHandleRx pfRxHandler)
{

	if (DEBUG_MODE)
	{
		Serial.println("SpiOpen");
	}

    pinMode(CC3000_nIRQ, INPUT);
    pinMode(HOST_nCS, OUTPUT);
    pinMode(HOST_VBAT_SW_EN, OUTPUT);

    //Initialize SPI
    SPI.begin();

    //Set bit order to MSB first
    SPI.setBitOrder(MSBFIRST);

    //Set data mode to CPHA 0 and CPOL 0
    SPI.setDataMode(SPI_MODE0);

    //Set clock divider.  This will be different for each board

    //For Due, this sets to 4MHz.  CC3000 can go up to 26MHz
    //SPI.setClockDivider(SS, SPI_CLOCK_DIV21);

    //For other boards, cant select SS pin. Only divide by 4 to get 4MHz
   	SPI.setClockDivider(SPI_CLOCK_DIV4);


	sSpiInformation.ulSpiState = eSPI_STATE_POWERUP;

	sSpiInformation.SPIRxHandler = pfRxHandler;
	sSpiInformation.usTxPacketLength = 0;
	sSpiInformation.pTxPacket = NULL;
	sSpiInformation.pRxPacket = (unsigned char *)spi_buffer;
	sSpiInformation.usRxPacketLength = 0;
	spi_buffer[CC3000_RX_BUFFER_SIZE - 1] = CC3000_BUFFER_MAGIC_NUMBER;
	wlan_tx_buffer[CC3000_TX_BUFFER_SIZE - 1] = CC3000_BUFFER_MAGIC_NUMBER;


	 
//	Enable interrupt on the GPIOA pin of WLAN IRQ
	
	tSLInformation.WlanInterruptEnable();


	if (DEBUG_MODE)
	{
		Serial.println("Completed SpiOpen");
	}
}




//*****************************************************************************
//
//! This function: init_spi
//!
//!  \param  buffer
//!
//!  \return none
//!
//!  \brief  initializes an SPI interface
//
//*****************************************************************************

long SpiFirstWrite(unsigned char *ucBuf, unsigned short usLength)
{
 
	//
    // workaround for first transaction
    //
	if (DEBUG_MODE)
	{
		Serial.println("SpiFirstWrite");
	}

   	digitalWrite(HOST_nCS, LOW);

	delayMicroseconds(50);
	
    // SPI writes first 4 bytes of data
    SpiWriteDataSynchronous(ucBuf, 4);

    delayMicroseconds(50);
	
    SpiWriteDataSynchronous(ucBuf + 4, usLength - 4);

    // From this point on - operate in a regular way
	sSpiInformation.ulSpiState = eSPI_STATE_IDLE;

	digitalWrite(HOST_nCS, HIGH);

    return(0);
}

long SpiWrite(unsigned char *pUserBuffer, unsigned short usLength)
{
  
	if (DEBUG_MODE)
	{
		Serial.println("SpiWrite");
	}

    unsigned char ucPad = 0;

	//
	// Figure out the total length of the packet in order to figure out if there is padding or not
	//
    if(!(usLength & 0x0001))
    {
        ucPad++;
    }
	
    pUserBuffer[0] = WRITE;
    pUserBuffer[1] = HI(usLength + ucPad);
    pUserBuffer[2] = LO(usLength + ucPad);
    pUserBuffer[3] = 0;
    pUserBuffer[4] = 0;

	usLength += (SPI_HEADER_SIZE);

        	
        // The magic number that resides at the end of the TX/RX buffer (1 byte after the allocated size)
        // for the purpose of detection of the overrun. If the magic number is overriten - buffer overrun 
        // occurred - and we will stuck here forever!
	if (wlan_tx_buffer[CC3000_TX_BUFFER_SIZE - 1] != CC3000_BUFFER_MAGIC_NUMBER)
	{
		while (1)
			;
	}


	if (sSpiInformation.ulSpiState == eSPI_STATE_POWERUP)
	{
		while (sSpiInformation.ulSpiState != eSPI_STATE_INITIALIZED)
			;
	}
	
	if (sSpiInformation.ulSpiState == eSPI_STATE_INITIALIZED)
	{
		
		//
		// This is time for first TX/RX transactions over SPI: the IRQ is down - so need to send read buffer size command
		//
		SpiFirstWrite(pUserBuffer, usLength);
	}
	else 
	{
		
		while (sSpiInformation.ulSpiState != eSPI_STATE_IDLE)
		{
			;
		}

		sSpiInformation.ulSpiState = eSPI_STATE_WRITE_IRQ;
		sSpiInformation.pTxPacket = pUserBuffer;
		sSpiInformation.usTxPacketLength = usLength;
		

   		digitalWrite(HOST_nCS, LOW);

	}
	
	//
	// Due to the fact that we are currently implementing a blocking situation
	// here we will wait till end of transaction
	//

	while (eSPI_STATE_IDLE != sSpiInformation.ulSpiState)
		;
	
    return(0);

}

char SpiWriteDataSynchronous(unsigned char *data, unsigned short size)
{
	tSLInformation.WlanInterruptDisable();
	if (DEBUG_MODE)
	{
		Serial.println("SpiWriteDataSynchronous");
	}
	
	char result;

	for (loc = 0; loc < size; loc ++) 
	{

		SPDR = data[loc];                    // Start the transmission
		while (!(SPSR & (1<<SPIF)))     // Wait the end of the transmission
	    	;
		delayMicroseconds(50);
	    result = SPDR;
		if (DEBUG_MODE)
		{
			if (!(loc==size-1))
			{
				Serial.print(data[loc]); 
				Serial.print(" ");
			}
			else
			{
				Serial.println(data[loc]);
			}
		}

	}

	if (DEBUG_MODE)
	{
		Serial.println("SpiWriteDataSynchronous done.");
		delayMicroseconds(50);
	}
	tSLInformation.WlanInterruptEnable();
	return result;

}


void SpiReadDataSynchronous(unsigned char *data, unsigned short size)
{

	
	if (DEBUG_MODE)
	{
		Serial.println("SpiReadDataSynchronous");
	}

	

	int i = 0;
	
	for (i = 0; i < size; i ++)
	{
		data[i] = SPI.transfer(0x00);
		Serial.println(data[i]);
    }
	


	/*
	delayMicroseconds(50);
    unsigned char result = SPDR;
	delayMicroseconds(50);
	
	int i = 0;
	while (!(result))
	{
		if (DEBUG_MODE)
		{
			Serial.print(result);
			Serial.print(" ");
		}
		SPDR = i;
		delayMicroseconds(50);
		result = SPDR;

	}

	Serial.println(" ");
	Serial.print("Result: ");
	Serial.println(result);
	*/
	//while (!(SPSR & (1<<SPIF)))
	//	;    // Wait the end of the transmission

	/*for (int i = 0; i < 15; i ++)
    {
		//while (!(SPSR & (1<<SPIF)))     // Wait the end of the transmission
			//Serial.print(digitalRead(CC3000_nIRQ));
			
			//;
		
		if (!(i==14))
		{
			data[i] = SPDR;
			Serial.print(data[i]);
		}
		else
		{
			Serial.println(data[i]);
		}
    }*/

}

void SpiReadHeader(void)
{

	if (DEBUG_MODE)
	{
		Serial.println("SpiReadHeader");
	}

	SpiWriteDataSynchronous(tSpiReadHeader, 3);

	SpiReadDataSynchronous(sSpiInformation.pRxPacket, 10);

}



long SpiReadDataCont(void)
{

	if (DEBUG_MODE)
	{
		Serial.println("SpiReadDataCont");
	}
   
    long data_to_recv;
	unsigned char *evnt_buff, type;

	
    //
    //determine what type of packet we have
    //
    evnt_buff =  sSpiInformation.pRxPacket;
    data_to_recv = 0;
	STREAM_TO_UINT8((char *)(evnt_buff + SPI_HEADER_SIZE), HCI_PACKET_TYPE_OFFSET, type);
	
    switch(type)
    {
        case HCI_TYPE_DATA:
        {
			//
			// We need to read the rest of data..
			//
			STREAM_TO_UINT16((char *)(evnt_buff + SPI_HEADER_SIZE), HCI_DATA_LENGTH_OFFSET, data_to_recv);
			if (!((HEADERS_SIZE_EVNT + data_to_recv) & 1))
			{	
    	        data_to_recv++;
			}

			if (data_to_recv)
			{
            	SpiReadDataSynchronous(evnt_buff + 10, data_to_recv);
			}
            break;
        }
        case HCI_TYPE_EVNT:
        {
			// 
			// Calculate the rest length of the data
			//
            STREAM_TO_UINT8((char *)(evnt_buff + SPI_HEADER_SIZE), HCI_EVENT_LENGTH_OFFSET, data_to_recv);
			data_to_recv -= 1;
			
			// 
			// Add padding byte if needed
			//
			if ((HEADERS_SIZE_EVNT + data_to_recv) & 1)
			{
				
	            data_to_recv++;
			}
			
			if (data_to_recv)
			{
            	SpiReadDataSynchronous(evnt_buff + 10, data_to_recv);
			}

			sSpiInformation.ulSpiState = eSPI_STATE_READ_EOT;
            break;
        }
    }
	
     return (0);
}

void SpiPauseSpi(void)
{

	if (DEBUG_MODE)
	{
		Serial.println("SpiPauseSpi");
	}
	// SPI_IRQ_PORT &= ~SPI_IRQ_PIN;
}

void SpiResumeSpi(void)
{

	if (DEBUG_MODE)
	{
		Serial.println("SpiResumeSpi");
	}
	// SPI_IRQ_PORT |= SPI_IRQ_PIN;
}

void SpiTriggerRxProcessing(void)
{

	if (DEBUG_MODE)
	{
		Serial.println("SpiTriggerRxProcessing");
	}
	// //
	// // Trigger Rx processing
	// //
	// SpiPauseSpi();
	// DEASSERT_CS();
        
 //        // The magic number that resides at the end of the TX/RX buffer (1 byte after the allocated size)
 //        // for the purpose of detection of the overrun. If the magic number is overriten - buffer overrun 
 //        // occurred - and we will stuck here forever!
	// if (sSpiInformation.pRxPacket[CC3000_RX_BUFFER_SIZE - 1] != CC3000_BUFFER_MAGIC_NUMBER)
	// {
	// 	while (1)
	// 		;
	// }
	
	// sSpiInformation.ulSpiState = eSPI_STATE_IDLE;
	// sSpiInformation.SPIRxHandler(sSpiInformation.pRxPacket + SPI_HEADER_SIZE);
}

int test(void)
{

	Serial.println("Calling wlan_init");
	wlan_init(CC3000_UsynchCallback, NULL, NULL, NULL, ReadWlanInterruptPin, WlanInterruptEnable, WlanInterruptDisable, WriteWlanPin);


	Serial.println("Calling wlan_start...");
	wlan_start(0);

	Serial.println("Attempting to connect...");
	wlan_connect(WLAN_SEC_UNSEC,ssid,10, bssid, keys, 16);

	return(0);
}


//*****************************************************************************
//
//! Returns state of IRQ pin
//!
//
//*****************************************************************************

long ReadWlanInterruptPin(void)
{

	if (DEBUG_MODE)
	{
		Serial.println("ReadWlanInterruptPin");
	}

	return(digitalRead(CC3000_nIRQ));

}


void WlanInterruptEnable()
{

	if (DEBUG_MODE)
	{
		Serial.println("WlanInterruptEnable.");
		delayMicroseconds(50);
	}
	Serial.println(digitalRead(CC3000_nIRQ));
	attachInterrupt(1, SPI_IRQ, FALLING); //Attaches Pin 3 to interrupt 1
}


void WlanInterruptDisable()
{
	if (DEBUG_MODE)
	{
		Serial.println("WlanInterruptDisable");
	}

	detachInterrupt(1);	//Detaches Pin 3 from interrupt 1
}

void SPI_IRQ(void)
{
	if (sSpiInformation.ulSpiState == eSPI_STATE_POWERUP)
	{
		/* This means IRQ line was low call a callback of HCI Layer to inform on event */
		sSpiInformation.ulSpiState = eSPI_STATE_INITIALIZED;
	}
	else if (sSpiInformation.ulSpiState == eSPI_STATE_IDLE)
	{
		sSpiInformation.ulSpiState = eSPI_STATE_READ_IRQ;
		digitalWrite(LED,HIGH);
		/* IRQ line goes down - we are start reception */
		digitalWrite(HOST_nCS, LOW);
		digitalWrite(LED,LOW);
		//
		// Wait for TX/RX Compete which will come as DMA interrupt
		// 
		SpiReadHeader();


		sSpiInformation.ulSpiState = eSPI_STATE_READ_EOT;
			
			//
			//
			//
		SSIContReadOperation();
	}
	else if (sSpiInformation.ulSpiState == eSPI_STATE_WRITE_IRQ)
	{

		SpiWriteDataSynchronous(sSpiInformation.pTxPacket, sSpiInformation.usTxPacketLength);

		sSpiInformation.ulSpiState = eSPI_STATE_IDLE;

		digitalWrite(HOST_nCS, HIGH);
	}

	return;

}

void print_spi_state(void)
{
	if (DEBUG_MODE)
	{

		switch (sSpiInformation.ulSpiState)
		{
			case eSPI_STATE_POWERUP:
				Serial.println("POWERUP");
				break;
			case eSPI_STATE_INITIALIZED:
				Serial.println("INITIALIZED");
				break;
			case eSPI_STATE_IDLE:
				Serial.println("IDLE");
				break;
			case eSPI_STATE_WRITE_IRQ:
				Serial.println("WRITE_IRQ");
				break;
			case eSPI_STATE_WRITE_FIRST_PORTION:
				Serial.println("WRITE_FIRST_PORTION");
				break;
			case eSPI_STATE_WRITE_EOT:
				Serial.println("WRITE_EOT");
				break;
			case eSPI_STATE_READ_IRQ:
				Serial.println("READ_IRQ");
				break;
			case eSPI_STATE_READ_FIRST_PORTION:
				Serial.println("READ_FIRST_PORTION");
				break;
			case eSPI_STATE_READ_EOT:
				Serial.println("STATE_READ_EOT");
				break;
			default:
				break;
		}
	}

	return;
}


void WriteWlanPin( unsigned char val )
{
    
    if (DEBUG_MODE)
	{
		Serial.print("WriteWlanPin: ");
		Serial.println(val);
		delayMicroseconds(50);
	}
	if (val)
	{
		digitalWrite(HOST_VBAT_SW_EN, HIGH);
	}
	else
	{
		digitalWrite(HOST_VBAT_SW_EN, LOW);
	}
		

}


//*****************************************************************************
//
//  The function handles asynchronous events that come from CC3000 device 
//!		  
//
//*****************************************************************************

void CC3000_UsynchCallback(long lEventType, char * data, unsigned char length)
{

	if (DEBUG_MODE)
	{
		Serial.println("CC3000_UsynchCallback");
	}

  
	// if (lEventType == HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE)
	// {
	// 	//config complete
	// }

	// if (lEventType == HCI_EVNT_WLAN_UNSOL_CONNECT)
	// {
	// 	//connected
	// }

	// if (lEventType == HCI_EVNT_WLAN_UNSOL_DISCONNECT)
	// {		
 //            //disconnected        
	// }
        
	// if (lEventType == HCI_EVNT_WLAN_UNSOL_DHCP)
	// {
 //             //dhcp
	// }
        
	// if (lEventType == HCI_EVENT_CC3000_CAN_SHUT_DOWN)
	// {
	// 	//ready to shut down
	// }

}



// *****************************************************************************

// ! This function enter point for write flow
// !
// !  \param  SSIContReadOperation
// !
// !  \return none
// !
// !  \brief  The function triggers a user provided callback for 

// *****************************************************************************

void
SSIContReadOperation(void)
{
	//
	// The header was read - continue with  the payload read
	//
	if (!SpiReadDataCont())
	{
		
		
		//
		// All the data was read - finalize handling by switching to teh task
		//	and calling from task Event Handler
		//
		SpiTriggerRxProcessing();
	}
}
