#include <Arduino.h>

#include "SPI.h"
#include "tm_debug.h"
#include "host_spi.h"
#include "utility/wlan.h"
#include "utility/nvmem.h"
#include "utility/security.h"
#include "utility/hci.h"
#include "utility/os.h"
#include "utility/netapp.h"
#include "utility/evnt_handler.h"

#define READ                    3
#define WRITE                   1

#define HI(value)               (((value) & 0xFF00) >> 8)
#define LO(value)               ((value) & 0x00FF)

// #define ASSERT_CS()          (P1OUT &= ~BIT3)

// #define DEASSERT_CS()        (P1OUT |= BIT3)

#define HEADERS_SIZE_EVNT       (SPI_HEADER_SIZE + 5)

#define SPI_HEADER_SIZE     (5)

#define eSPI_STATE_POWERUP                (0)
#define eSPI_STATE_INITIALIZED            (1)
#define eSPI_STATE_IDLE                   (2)
#define eSPI_STATE_WRITE_IRQ              (3)
#define eSPI_STATE_WRITE_FIRST_PORTION    (4)
#define eSPI_STATE_WRITE_EOT              (5)
#define eSPI_STATE_READ_IRQ               (6)
#define eSPI_STATE_READ_FIRST_PORTION     (7)
#define eSPI_STATE_READ_EOT               (8)


#define CC3000_nIRQ     (2)
#define HOST_nCS        (10)
#define HOST_VBAT_SW_EN (9)
#define LED             (6)

#define DISABLE                   (0)

#define ENABLE                    (1)

#define DEBUG_MODE    (1)
#define NETAPP_IPCONFIG_MAC_OFFSET        (20)
#define ConnLED (7)
#define ErrorLED (6)


unsigned char tSpiReadHeader[] = {READ, 0, 0, 0, 0};
int uart_have_cmd;

//foor spi bus loop
int loc = 0; 

char device_name[] = "CC3000";
const char aucCC3000_prefix[] = {'T', 'T', 'T'};
// const unsigned char smartconfigkey[] = {0x73,0x6d,0x61,0x72,0x74,0x63,0x6f,0x6e,0x66,0x69,0x67,0x41,0x45,0x53,0x31,0x36};

static int keyIndex = 0; 
unsigned char printOnce = 1;

volatile unsigned long ulSmartConfigFinished, ulCC3000Connected,ulCC3000DHCP, OkToDoShutDown, ulCC3000DHCP_configured;

unsigned char ucStopSmartConfig;

typedef struct
{
  gcSpiHandleRx  SPIRxHandler;
  unsigned short usTxPacketLength;
  unsigned short usRxPacketLength;
  unsigned long  ulSpiState;
  unsigned char *pTxPacket;
  unsigned char *pRxPacket;

} tSpiInformation;

sockaddr tSocketAddr;
tSpiInformation sSpiInformation;

void SpiWriteDataSynchronous(unsigned char *data, unsigned short size);
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
//                                                  2.  __IAR_SYSTEMS_ITM__ for IAR Embedded Workbench  ///
// *CCS does not initialize variables - therefore, __no_init is not needed.                             ///
///////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned char wlan_tx_buffer[CC3000_TX_BUFFER_SIZE];
unsigned char spi_buffer[CC3000_RX_BUFFER_SIZE];


void csn(int mode)
{
  digitalWrite(HOST_nCS,mode);
}

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
void SpiCleanGPIOISR(void)
{
  TM_DEBUG("SpiCleanGPIOISR\n");

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
  TM_DEBUG("SpiClose\n");

  if (sSpiInformation.pRxPacket)
  {
    sSpiInformation.pRxPacket = 0;
  }

  //
  // Disable Interrupt in GPIOA module...
  //
  tSLInformation.WlanInterruptDisable();
}


void SpiInit(){
  ulCC3000DHCP = 0;
  ulCC3000Connected = 0;
  ulSmartConfigFinished=0;

  pinMode(CC3000_nIRQ, INPUT);
  attachInterrupt(0, SPI_IRQ, FALLING); //Attaches Pin 2 to interrupt 1
  
  pinMode(HOST_nCS, OUTPUT);
  pinMode(HOST_VBAT_SW_EN, OUTPUT);
  pinMode(ConnLED, OUTPUT);
  pinMode(ErrorLED, OUTPUT);

  //Initialize SPI
  SPI.begin();

  csn(HIGH);
  //Set bit order to MSB first
  SPI.setBitOrder(MSBFIRST);

  //Set data mode to CPHA 0 and CPOL 0
  SPI.setDataMode(SPI_MODE1);

  //Set clock divider.  This will be different for each board

  //For Due, this sets to 4MHz.  CC3000 can go up to 26MHz
  //SPI.setClockDivider(SS, SPI_CLOCK_DIV21);

  //For other boards, cant select SS pin. Only divide by 4 to get 4MHz
  SPI.setClockDivider(SPI_CLOCK_DIV4);

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
  TM_DEBUG("SpiOpen\n");

  sSpiInformation.ulSpiState = eSPI_STATE_POWERUP;

  sSpiInformation.SPIRxHandler = pfRxHandler;
  sSpiInformation.usTxPacketLength = 0;
  sSpiInformation.pTxPacket = NULL;
  sSpiInformation.pRxPacket = (unsigned char *)spi_buffer;
  sSpiInformation.usRxPacketLength = 0;
  spi_buffer[CC3000_RX_BUFFER_SIZE - 1] = CC3000_BUFFER_MAGIC_NUMBER;
  wlan_tx_buffer[CC3000_TX_BUFFER_SIZE - 1] = CC3000_BUFFER_MAGIC_NUMBER;


   
//  Enable interrupt on the GPIOA pin of WLAN IRQ
  
  tSLInformation.WlanInterruptEnable();


  TM_DEBUG("Completed SpiOpen\n");
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
  TM_DEBUG("SpiFirstWrite\n");

  // digitalWrite(HOST_nCS, LOW);
  csn(LOW);
  delayMicroseconds(80);
  
  // SPI writes first 4 bytes of data
  SpiWriteDataSynchronous(ucBuf, 4);

  delayMicroseconds(80);

  SpiWriteDataSynchronous(ucBuf + 4, usLength - 4);
  // SpiWriteDataSynchronous(testData, usLength - 4);

  // From this point on - operate in a regular way
  sSpiInformation.ulSpiState = eSPI_STATE_IDLE;

  // digitalWrite(HOST_nCS, HIGH);
  csn(HIGH);
  return(0);
}

long SpiWrite(unsigned char *pUserBuffer, unsigned short usLength)
{
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

  usLength += (SPI_HEADER_SIZE + ucPad);

  // The magic number that resides at the end of the TX/RX buffer (1 byte after the allocated size)
  // for the purpose of detection of the overrun. If the magic number is overriten - buffer overrun 
  // occurred - and we will stuck here forever!
  if (wlan_tx_buffer[CC3000_TX_BUFFER_SIZE - 1] != CC3000_BUFFER_MAGIC_NUMBER)
  {
    while (1)
      ;
  }

  // Serial.println("Checking for state");
  // print_spi_state();
  if (sSpiInformation.ulSpiState == eSPI_STATE_POWERUP)
  {
    while (sSpiInformation.ulSpiState != eSPI_STATE_INITIALIZED){
    }
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
    
    // We need to prevent here race that can occur in case 2 back to back 
    // packets are sent to the  device, so the state will move to IDLE and once 
    //again to not IDLE due to IRQ
    tSLInformation.WlanInterruptDisable();

    while (sSpiInformation.ulSpiState != eSPI_STATE_IDLE)
    {
      ;
    }

    sSpiInformation.ulSpiState = eSPI_STATE_WRITE_IRQ;
    sSpiInformation.pTxPacket = pUserBuffer;
    sSpiInformation.usTxPacketLength = usLength;

    // assert CS
    //digitalWrite(HOST_nCS, LOW);
    csn(LOW);
    // reenable IRQ
    tSLInformation.WlanInterruptEnable();

  }

  // check for a missing interrupt between the CS assertion and enabling back the interrupts
  if (tSLInformation.ReadWlanInterruptPin() == 0)
  {
    // Serial.println("writing synchronous data");
    SpiWriteDataSynchronous(sSpiInformation.pTxPacket, sSpiInformation.usTxPacketLength);
    sSpiInformation.ulSpiState = eSPI_STATE_IDLE;

    //deassert CS
    csn(HIGH);
  }
  
  //
  // Due to the fact that we are currently implementing a blocking situation
  // here we will wait till end of transaction
  //

  while (eSPI_STATE_IDLE != sSpiInformation.ulSpiState)
    ;
  //Serial.println("done with spi write");
    return(0);

}

void SpiWriteDataSynchronous(unsigned char *data, unsigned short size)
{
  tSLInformation.WlanInterruptDisable();

  while (size) {
    SPI.transfer(*data);
    size--;
    data++;
  }

  tSLInformation.WlanInterruptEnable();
}


void SpiReadDataSynchronous(unsigned char *data, unsigned short size)
{

  unsigned int i = 0;
  for (i = 0; i < size; i ++)
  {
    data[i] = SPI.transfer(tSpiReadHeader[0]); 
  }
}

void SpiReadHeader(void)
{
  SpiReadDataSynchronous(sSpiInformation.pRxPacket, 10);
}



long SpiReadDataCont(void)
{
  long data_to_recv;
  unsigned char *evnt_buff, type;

  
  //
  //determine what type of packet we have
  //
  evnt_buff =  sSpiInformation.pRxPacket;
  data_to_recv = 0;
  STREAM_TO_UINT8((char *)(evnt_buff + SPI_HEADER_SIZE), 
    HCI_PACKET_TYPE_OFFSET, type);
  
  switch(type)
  {
    case HCI_TYPE_DATA:
    {
      //
      // We need to read the rest of data..
      //
      STREAM_TO_UINT16((char *)(evnt_buff + SPI_HEADER_SIZE), 
        HCI_DATA_LENGTH_OFFSET, data_to_recv);
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
      STREAM_TO_UINT8((char *)(evnt_buff + SPI_HEADER_SIZE), 
        HCI_EVENT_LENGTH_OFFSET, data_to_recv);
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
  detachInterrupt(0); //Detaches Pin 3 from interrupt 1
}

void SpiResumeSpi(void)
{
  attachInterrupt(0, SPI_IRQ, FALLING); //Attaches Pin 2 to interrupt 1
}

void SpiTriggerRxProcessing(void)
{

  
  // //
  // // Trigger Rx processing
  // //
  SpiPauseSpi();
  csn(HIGH);
  //DEASSERT_CS();
  //digitalWrite(HOST_nCS, HIGH);


  // The magic number that resides at the end of the TX/RX buffer (1 byte after 
  // the allocated size) for the purpose of detection of the overrun. If the 
  // magic number is overwritten - buffer overrun occurred - and we will stuck 
  // here forever!
  if (sSpiInformation.pRxPacket[CC3000_RX_BUFFER_SIZE - 1] != CC3000_BUFFER_MAGIC_NUMBER)
  {

    while (1) {
      ;
    }
  }
  
  sSpiInformation.ulSpiState = eSPI_STATE_IDLE;
  sSpiInformation.SPIRxHandler(sSpiInformation.pRxPacket + SPI_HEADER_SIZE);
}

void StartSmartConfig(void)
{

  ulSmartConfigFinished = 0;
  ulCC3000Connected = 0;
  ulCC3000DHCP = 0;
  OkToDoShutDown=0;

  // Reset all the previous configuration

  if (wlan_ioctl_set_connection_policy(0, 0, 0) != 0) {
    digitalWrite(ErrorLED, HIGH);
    return;
  }

  if (wlan_ioctl_del_profile(255) != 0) {
    digitalWrite(ErrorLED, HIGH);
    return;
  }

  //Wait until CC3000 is disconnected
  while (ulCC3000Connected == 1)
  {
    delayMicroseconds(100);
  }

  // Serial.println("waiting for disconnect");

  // Trigger the Smart Config process
  // Start blinking LED6 during Smart Configuration process
  digitalWrite(ConnLED, HIGH);  
  
  if (wlan_smart_config_set_prefix((char*)aucCC3000_prefix) != 0){
    digitalWrite(ErrorLED, HIGH);
    return;
  }

  digitalWrite(ConnLED, LOW);

  // Start the SmartConfig start process
  if (wlan_smart_config_start(0) != 0){
    digitalWrite(ErrorLED, HIGH);
    return;
  }
  if (DEBUG_MODE) {
    Serial.println("smart config start");
  }

  digitalWrite(ConnLED, HIGH);
  
  // Wait for Smartconfig process complete
  while (ulSmartConfigFinished == 0)
  {
    delay(500);
    digitalWrite(ConnLED, LOW);
    delay(500);
    digitalWrite(ConnLED, HIGH);
  }
  
  if (DEBUG_MODE) {
    Serial.println("smart config finished");
  }
  digitalWrite(ConnLED, LOW);

  
  // Configure to connect automatically to the AP retrieved in the 
  // Smart config process. Enabled fast connect.
  if (wlan_ioctl_set_connection_policy(0, 0, 1) != 0){
    digitalWrite(ErrorLED, HIGH);
    return;
  }
  
  // reset the CC3000
  wlan_stop();

  delayMicroseconds(500);
  wlan_start(0);

  // Mask out all non-required events
  wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE|HCI_EVNT_WLAN_UNSOL_INIT|HCI_EVNT_WLAN_ASYNC_PING_REPORT);
  if (DEBUG_MODE) {
    Serial.print("Config done");
  }
}


//*****************************************************************************
//
//! Returns state of IRQ pin
//!
//
//*****************************************************************************

long ReadWlanInterruptPin(void)
{
  return(digitalRead(CC3000_nIRQ));
}


void WlanInterruptEnable()
{
  attachInterrupt(0, SPI_IRQ, FALLING); //Attaches Pin 2 to interrupt 1
}


void WlanInterruptDisable()
{
  detachInterrupt(0); //Detaches Pin 3 from interrupt 1
}

void SPI_IRQ(void)
{

  if (sSpiInformation.ulSpiState == eSPI_STATE_POWERUP)
  {

    //This means IRQ line was low call a callback of HCI Layer to inform on event 
    sSpiInformation.ulSpiState = eSPI_STATE_INITIALIZED;
  }
  else if (sSpiInformation.ulSpiState == eSPI_STATE_IDLE)
  {

    sSpiInformation.ulSpiState = eSPI_STATE_READ_IRQ;
    //IRQ line goes down - we are start reception
    csn(LOW);
    //
    // Wait for TX/RX Compete which will come as DMA interrupt
    // 
    SpiReadHeader();

    sSpiInformation.ulSpiState = eSPI_STATE_READ_EOT;
    
    SSIContReadOperation();
    
  }
  else if (sSpiInformation.ulSpiState == eSPI_STATE_WRITE_IRQ)
  {

    SpiWriteDataSynchronous(sSpiInformation.pTxPacket, 
      sSpiInformation.usTxPacketLength);

    sSpiInformation.ulSpiState = eSPI_STATE_IDLE;
    csn(HIGH);
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
        TM_DEBUG("POWERUP\n");
        break;
      case eSPI_STATE_INITIALIZED:
        TM_DEBUG("INITIALIZED\n");
        break;
      case eSPI_STATE_IDLE:
        TM_DEBUG("IDLE\n");
        break;
      case eSPI_STATE_WRITE_IRQ:
        TM_DEBUG("WRITE_IRQ\n");
        break;
      case eSPI_STATE_WRITE_FIRST_PORTION:
        TM_DEBUG("WRITE_FIRST_PORTION\n");
        break;
      case eSPI_STATE_WRITE_EOT:
        TM_DEBUG("WRITE_EOT\n");
        break;
      case eSPI_STATE_READ_IRQ:
        TM_DEBUG("READ_IRQ\n");
        break;
      case eSPI_STATE_READ_FIRST_PORTION:
        TM_DEBUG("READ_FIRST_PORTION\n");
        break;
      case eSPI_STATE_READ_EOT:
        TM_DEBUG("STATE_READ_EOT\n");
        break;
      default:
        break;
    }
  }

  return;
}


void WriteWlanPin( unsigned char val )
{

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

  if (lEventType == HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE)
  {
    ulSmartConfigFinished = 1;
    ucStopSmartConfig     = 1;  
  }
  
  if (lEventType == HCI_EVNT_WLAN_UNSOL_CONNECT)
  {
    ulCC3000Connected = 1;
    TM_DEBUG("connected\n");
  }
  
  if (lEventType == HCI_EVNT_WLAN_UNSOL_DISCONNECT)
  {   
    ulCC3000Connected = 0;
    ulCC3000DHCP      = 0;
    ulCC3000DHCP_configured = 0;
    printOnce = 1;
    
    TM_DEBUG("disconnected\n");

    digitalWrite(ConnLED, LOW);
    // digitalWrite(ErrorLED, HIGH);
  }
  
  if (lEventType == HCI_EVNT_WLAN_UNSOL_DHCP)
  {

    TM_DEBUG("dhcp\n");
    // Notes: 
    // 1) IP config parameters are received swapped
    // 2) IP config parameters are valid only if status is OK, i.e. ulCC3000DHCP becomes 1
    
    // only if status is OK, the flag is set to 1 and the addresses are valid
    if ( *(data + NETAPP_IPCONFIG_MAC_OFFSET) == 0)
    {
      //sprintf( (char*)pucCC3000_Rx_Buffer,"IP:%d.%d.%d.%d\f\r", data[3],data[2], data[1], data[0] );

      ulCC3000DHCP = 1;

      TM_DEBUG("DHCP Connected with IP: %hhu.%hhu.%hhu.%hhu\n", (unsigned char) data[3], (unsigned char) data[2], (unsigned char) data[1], (unsigned char) data[0]);

      digitalWrite(ConnLED, HIGH);
    }
    else
    {
      ulCC3000DHCP = 0;
      TM_DEBUG("DHCP failed\n");
      digitalWrite(ConnLED, LOW);
    }
  }
  
  if (lEventType == HCI_EVENT_CC3000_CAN_SHUT_DOWN)
  {
    OkToDoShutDown = 1;
  }
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

void SSIContReadOperation(void)
{
  //Serial.println("SSIContReadOp");
  
  //
  // The header was read - continue with  the payload read
  //
  if (!SpiReadDataCont())
  {
    
    
    //
    // All the data was read - finalize handling by switching to teh task
    //  and calling from task Event Handler
    //
    SpiTriggerRxProcessing();
  }
}
