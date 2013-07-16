/* Copyright 2013 - Elerson Rubens da S. Santos <elerss@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <msp430g2553.h>
#include "zigbee.h"
#include "rpl.h"

int main(void) {



      WDTCTL = WDTPW + WDTHOLD;               //
	  DCOCTL = 0x00;                          //
	  BCSCTL1 = CALBC1_8MHZ;
	  DCOCTL = CALDCO_8MHZ;

	  P2OUT = 0x00;
	  P2SEL = UART_TXD + UART_RXD;            //  uart TX/RX pinos
	  P2DIR = 0xFF & ~UART_RXD;               //
	  P2OUT = 0XFF;

	  P1DIR = 0x01 + 0x40;        // Define pinos 1.0 e 1.6 como saída (0100 0001)
	  P1REN = 0x08;               // Habilita pullup/pulldown do pino 1.3 (0000 1000)
	  P1OUT = 0x08;               // Define pullup para o pino 1.3 (0000 1000)
	  P1SEL |= TX + RX;           // Timer function for TXD/RXD pins


	  zigbeeInit();
	  resetTimer();
	  __enable_interrupt();
	  NetworkProtocolConfigure();


	  for (;;){
		  execNetworkProtocol();
	  }
}



#pragma vector = WDT_VECTOR, NMI_VECTOR//,RESET_VECTOR
__interrupt void ISR_trap(void)
{
	if(FCTL3 & ACCVIFG)
	{
		P1DIR |= 0x01;					// Set P1.0 to output direction
		for(;;) {
			volatile unsigned int i;	// volatile to prevent optimization

			P1OUT ^= 0x0;				// Toggle P1.0 using exclusive-OR

			i = 10000;					// SW Delay
			do i--;
			while(i != 0);
		}
	}

}

/*
HARDWARE UART for USB - 9600 bps - don't change that
*/
void USBInit()
{
  BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
  DCOCTL = CALDCO_1MHZ;
  P1SEL |= BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
  P1SEL2 |= BIT1 + BIT2 ;                    // P1.1 = RXD, P1.2=TXD
  UCA0CTL1 |= UCSSEL_2;                     // SMCLK
  UCA0BR0 = 104;                            // 1MHz 19200
  UCA0BR1 = 0;                              // 1MHz 19200
  UCA0MCTL = UCBRS1;                        // Modulation UCBRSx = 1
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt

}

//------------------------------------------------------------------------------
// Outputs one byte using the hardware UART
//------------------------------------------------------------------------------

void USBPutChar(unsigned char c)
{
  while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
  UCA0TXBUF = c;
}

//------------------------------------------------------------------------------
// Outputs one string using the hardware UART
//------------------------------------------------------------------------------


void USBPutString(char *s)
{
  while(*s)
    USBPutChar(*s++);
}

//------------------------------------------------------------------------------
// Input one byte using the hardware UART
//------------------------------------------------------------------------------


#pragma vector=USCIAB0RX_VECTOR
__interrupt void USBGetChar_ISR(void)
{
  zigbeePutChar(UCA0RXBUF);
}

