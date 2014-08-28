// main_SHT21_Temp.c
//
//****************************************************************************************************
// Author:
// 	Nipun Gunawardena
//
// Credits:
//	Borrows from Texas Instruments' sht21 library.
//
// Requirements:
//
//
// Description:
// 	Interface with Sensirion SHT21 for temp measurements without driverlib
//
// Notes:
//
// Todo:
//	Make more durable, timeouts, testing, etc.
//****************************************************************************************************




// Includes ------------------------------------------------------------------------------------------
#include <stdint.h>
#include <msp430.h>




// Defines -------------------------------------------------------------------------------------------
#define SHT21_I2C_ADDRESS  0x40
#define SHT21_TEMP_NOBLOCK 0xF3
#define SHT21_HUM_NOBLOCK  0xF5




// Global --------------------------------------------------------------------------------------------
volatile unsigned char g_rxArr[2];
volatile uint8_t g_rxCount;
float hum = 0.0f;



// Main ----------------------------------------------------------------------------------------------
int main(void)
{
  WDTCTL = WDTPW | WDTHOLD;       // Stop WDT

  // Configure GPIO
  P1OUT &= ~BIT0;                           // Clear P1.0 output latch
  P1DIR |= BIT0;                            // For LED
  P1SEL1 |= BIT6 | BIT7;                    // I2C pins
//  PJSEL0 |= BIT4 | BIT5;					// Set J.4 & J.5 to accept crystal input for ACLK

  // Disable the GPIO power-on default high-impedance mode to activate
  // previously configured port settings
  PM5CTL0 &= ~LOCKLPM5;

  // Clock setup
  CSCTL0_H = CSKEY >> 8;                    // Unlock CS registers
  CSCTL1 = DCOFSEL_0;                       // Set DCO to 1MHz
  CSCTL2 = SELA__LFXTCLK | SELS__DCOCLK | SELM__DCOCLK; // Set ACLK = LFXTCLK; MCLK = DCO
  CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers to 1
  CSCTL4 &= ~LFXTOFF;						// Turn on LFXT

//  // Check for clock faults
//  do
//  {
//    CSCTL5 &= ~LFXTOFFG;                    // Clear XT1 fault flag
//    SFRIFG1 &= ~OFIFG;						// Clear Oscillator fault flag
//  }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag

  // Lock CS registers - Why isn't PUC created?
  CSCTL0_H = 0;


  // Enable interrupts
  __bis_SR_register(GIE);

  // Configure USCI_B0 for I2C mode - Sending
  UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
  UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, Sending, SMCLK
  UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
  UCB0CTLW1 |= UCASTP_2;                    // Automatic stop generated after UCB0TBCNT is reached
  UCB0TBCNT = 0x01;                       	// number of bytes to be sent
  UCB0I2CSA = SHT21_I2C_ADDRESS;            // Slave address
  UCB0CTL1 &= ~UCSWRST;						// Clear reset
  UCB0IE |= UCTXIE0 | UCRXIE0;

  // Start transfer
  UCB0CTL1 |= UCTXSTT;

  // Wait until stop is transmitted
  while (UCB0CTLW0 & UCTXSTP);            // Ensure stop condition got sent

  // Setup timer for 85ms measurement delay
  TB0CCTL0 = CCIE;                          // TBCCR0 interrupt enabled
  TB0CCR0 = 2800;							// (2800 ticks) * (1 second / 32768 ticks) = 85.4 ms > 85 ms required
  TB0CTL = TBSSEL__ACLK | MC__UP;           // ACLK, up mode

  __bis_SR_register(LPM3_bits);       // Enter LPM3 w/ interrupt

  // Reset receive count
  g_rxCount = 0;

  // Configure USCI_B0 for I2C mode - Receive
  UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
  UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, Sending, SMCLK
  UCB0CTLW0 &= ~UCTR;						// Clear write
  UCB0BRW = 0x0004;                         // baudrate = SMCLK / 8
  UCB0CTLW1 |= UCASTP_2;                    // Automatic stop generated after UCB0TBCNT is reached
  UCB0TBCNT = 0x02;                       	// number of bytes to be received
  UCB0I2CSA = SHT21_I2C_ADDRESS;            // Slave address
  UCB0CTL1 &= ~UCSWRST;						// Clear reset
  UCB0IE |= UCTXIE0 | UCRXIE0;

  // Start transfer
  UCB0CTL1 |= UCTXSTT;

  // Delay during transfer - How to use LPM for that?
  while (UCB0CTLW0 & UCTXSTT);            // Ensure stop condition got sent
  __bis_SR_register(LPM0_bits);       // Enter LPM0 w/ interrupts
//  __delay_cycles(10000);

  uint16_t tempRaw = ((uint16_t)g_rxArr[0] << 8) | (uint16_t)(g_rxArr[1]);
  hum = (float)(tempRaw & 0xFFFC);
  hum = -46.85f + 175.72f * (hum/65536.0f);

  __no_operation();
}




// Interrupts ----------------------------------------------------------------------------------------

// EUSCI_B I2C Interrupt
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B0_VECTOR))) USCI_B0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCB0IV, USCI_I2C_UCBIT9IFG))
  {
    case USCI_NONE:          break;         // Vector 0: No interrupts
    case USCI_I2C_UCALIFG:   break;         // Vector 2: ALIFG
    case USCI_I2C_UCNACKIFG: break;         // Vector 4: NACKIFG
    case USCI_I2C_UCSTTIFG:  break;         // Vector 6: STTIFG
    case USCI_I2C_UCSTPIFG:  break;         // Vector 8: STPIFG
    case USCI_I2C_UCRXIFG3:  break;         // Vector 10: RXIFG3
    case USCI_I2C_UCTXIFG3:  break;         // Vector 12: TXIFG3
    case USCI_I2C_UCRXIFG2:  break;         // Vector 14: RXIFG2
    case USCI_I2C_UCTXIFG2:  break;         // Vector 16: TXIFG2
    case USCI_I2C_UCRXIFG1:  break;         // Vector 18: RXIFG1
    case USCI_I2C_UCTXIFG1:  break;         // Vector 20: TXIFG1
    case USCI_I2C_UCRXIFG0:  		        // Vector 22: RXIFG0
    	if(g_rxCount < 2){
    		g_rxArr[g_rxCount] = UCB0RXBUF;
    		g_rxCount++;
    	}
    	if(g_rxCount == 2){
    		__bic_SR_register_on_exit(LPM0_bits); 	// Exit LPM0
    	}
    	break;
    case USCI_I2C_UCTXIFG0:                 // Vector 24: TXIFG0
    	UCB0TXBUF = SHT21_TEMP_NOBLOCK;
    	break;
    default: break;
  }
}


// Timer B1 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer0_B0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_B0_VECTOR))) Timer0_B0_ISR (void)
#else
#error Compiler not supported!
#endif
{
	TB0CTL &= ~MC__STOP;
	__bic_SR_register_on_exit(LPM3_bits); 	// Exit LPM0
}
