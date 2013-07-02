//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifndef __CC3000_SPI_H__
#define __CC3000_SPI_H__

#ifdef  __cplusplus
extern "C" {
#endif

typedef void (*gcSpiHandleRx)(void *p);
typedef void (*gcSpiHandleTx)(void);

extern unsigned char wlan_tx_buffer[];
extern unsigned char spi_buffer[];

// extern unsigned long ulSmartConfigFinished, ulCC3000Connected,ulCC3000DHCP, OkToDoShutDown, ulCC3000DHCP_configured;
// extern long ulSocket;

// #define MOSI_MISO_PORT_SEL    P1SEL1
// #define MOSI_MISO_PORT_SEL2   P1SEL0
// #define SPI_MISO_PIN         BIT6
// #define SPI_MOSI_PIN         BIT7

// #define SPI_CLK_PORT_SEL    P2SEL1
// #define SPI_CLK_PORT_SEL2   P2SEL0
// #define SPI_CLK_PIN         BIT2

// #define SPI_IRQ_PORT    P2IE
// #define SPI_IFG_PORT    P2IFG
// #define SPI_IRQ_PIN     BIT3
//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************
extern void SpiOpen(gcSpiHandleRx pfRxHandler);
extern void SpiClose(void);
extern long SpiWrite(unsigned char *pUserBuffer, unsigned short usLength);
extern void SpiResumeSpi(void);
extern void SpiConfigureHwMapping(  unsigned long ulPioPortAddress,
                  unsigned long ulPort, 
                  unsigned long ulSpiCs, 
                  unsigned long ulPortInt, 
                  unsigned long uluDmaPort,
                  unsigned long ulSsiPortAddress,
                  unsigned long ulSsiTx,
                  unsigned long ulSsiRx,
                  unsigned long ulSsiClck);
extern void SpiCleanGPIOISR(void);
extern long TXBufferIsEmpty(void);
extern long RXBufferIsEmpty(void);
extern void SPI_IRQ(void);
extern void SpiInit(void);

extern void CC3000_UsynchCallback(long lEventType, char * data, unsigned char length);
extern long ReadWlanInterruptPin(void);
extern void WlanInterruptEnable();
extern void WlanInterruptDisable();
extern void WriteWlanPin( unsigned char val );
extern void print_spi_state(void);

extern void StartSmartConfig(void);
extern void initialize(void);

extern unsigned char ascii_to_char(char b1, char b2);
extern unsigned char atoc(char data);
extern void SpiTriggerRxProcessing(void);

// info
extern void getIpAddr(char * ipBuffer);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef  __cplusplus
}
#endif // __cplusplus

//#endif
#endif // __CC3000_SPI_H__

