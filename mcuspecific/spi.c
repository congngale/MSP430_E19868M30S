#include <msp430.h>
#include <stdint.h>
#include <driverlib.h>
#include "uart.h"
#include "spi.h"
#include "stdio.h"
#include <custom.h>		//to get the Pins for the module

volatile uint8_t spi_buf = 0;

void initSPI(){

    // Configure SPI pins

    /*
     * Select Port 5
     * Set Pin 2 to input Secondary Module Function, (UCB1CLK).
     */
    GPIO_setAsPeripheralModuleFunctionInputPin(SPIPORTCLK, SPICLK, GPIO_SECONDARY_MODULE_FUNCTION);

    /*
     * Select Port 5
     * Set Pin 0, 1 to input Secondary Module Function, (UCB1SIMO, UCB1SOMI).
     */
    GPIO_setAsPeripheralModuleFunctionInputPin(SPIPORTDATA, SPITX + SPIRX, GPIO_SECONDARY_MODULE_FUNCTION);

    //Initialize Master
    EUSCI_B_SPI_initMasterParam param = {0};
    param.selectClockSource = EUSCI_B_SPI_CLOCKSOURCE_SMCLK;
    param.clockSourceFrequency = CS_getSMCLK();
    param.desiredSpiClock = 8000000; // 8MHz
    param.msbFirst = EUSCI_B_SPI_MSB_FIRST;
    param.clockPhase = EUSCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;
    param.clockPolarity = EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW;
    param.spiMode = EUSCI_B_SPI_3PIN;
    EUSCI_B_SPI_initMaster(EUSCI_B1_BASE, &param);

    //Enable SPI module
    EUSCI_B_SPI_enable(EUSCI_B1_BASE);
    myUart_writeBuf( BACKCHANNEL_UART, "SPI Enabled", NULL, CRLF);

    EUSCI_B_SPI_clearInterrupt(EUSCI_B1_BASE, EUSCI_B_SPI_RECEIVE_INTERRUPT);

    // Enable USCI_B1 RX interrupt
    EUSCI_B_SPI_enableInterrupt(EUSCI_B1_BASE, EUSCI_B_SPI_RECEIVE_INTERRUPT);

    myUart_writeBuf( BACKCHANNEL_UART, "SPI RX interrupt Enabled", NULL, CRLF);

    // Enable USCI_B1 TX interrupt
    EUSCI_B_SPI_enableInterrupt(EUSCI_B1_BASE, EUSCI_B_SPI_TRANSMIT_INTERRUPT);

    myUart_writeBuf( BACKCHANNEL_UART, "SPI TX interrupt Enabled", NULL, CRLF);

#ifdef LEDS
  GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0); // Red LED
  GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN1); // Green LED
#endif

    //Wait for slave to initialize
    __delay_cycles(100);

//    TXData = 0x1;                             // Holds TX data

    myUart_writeBuf( BACKCHANNEL_UART, "SPI wait for TX ready", NULL, CRLF);

    //USCI_B1 TX buffer ready?
    while (!EUSCI_B_SPI_getInterruptStatus(EUSCI_B1_BASE, EUSCI_B_SPI_TRANSMIT_INTERRUPT)) ;
}

void spi_txready() {
    //USCI_B1 TX buffer ready?
    while (!EUSCI_B_SPI_getInterruptStatus(EUSCI_B1_BASE, EUSCI_B_SPI_TRANSMIT_INTERRUPT)) ;
}

void spi_rxready() {
    //USCI_B1 RX Received?
    while (!EUSCI_B_SPI_getInterruptStatus(EUSCI_B1_BASE, EUSCI_B_SPI_RECEIVE_INTERRUPT));
}

void spi_send(uint8_t data) {
  spi_txready();
  EUSCI_B_SPI_transmitData(EUSCI_B1_BASE, data);
}

void spi_recv() {
  spi_rxready();
  spi_buf = EUSCI_B_SPI_receiveData(EUSCI_B1_BASE);         // Store received data
}

void spi_transfer(uint8_t data) {
  spi_send(data);
  spi_recv();
}

void spi_transfer_short(uint8_t data) {
  EUSCI_B_SPI_transmitData(EUSCI_B1_BASE, data);
  //spi_buf = EUSCI_B_SPI_receiveData(EUSCI_B1_BASE);
}

void spi_chipEnable() {
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN3);
}

void spi_chipDisable() {
    GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN3);
}
