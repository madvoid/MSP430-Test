#include <msp430.h>


int main(void)
{
  WDTCTL = WDTPW | WDTHOLD;                 // Stop WDT

  // Configure GPIO
  P1DIR |= BIT0;                            // Output for LED
  P1OUT |= BIT0;
  PJSEL0 |= BIT4 | BIT5;					// Set J.4 & J.5 to accept crystal input for ACLK

  // Disable the GPIO power-on default high-impedance mode to activate previously configured port settings
  PM5CTL0 &= ~LOCKLPM5;

  // Clock setup
  CSCTL0_H = CSKEY >> 8;                    // Unlock CS registers
  CSCTL1 = DCOFSEL_0;                       // Set DCO to 1MHz
  CSCTL2 = SELA__LFXTCLK | SELS__DCOCLK | SELM__DCOCLK; // Set ACLK = LFXTCLK; MCLK = DCO
  CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers to 1
  CSCTL4 &= ~LFXTOFF;						// Turn on LFXT

  // Check for clock faults
  do
  {
    CSCTL5 &= ~LFXTOFFG;                    // Clear XT1 fault flag
    SFRIFG1 &= ~OFIFG;						// Clear Oscillator fault flag
  }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag

  // Lock CS registers - Why isn't PUC created?
  CSCTL0_H = 0;

  __delay_cycles(5000000);
  P1OUT ^= BIT0;

  // Setup timer
  TB0CCTL0 = CCIE;                          // TBCCR0 interrupt enabled
  TB0CCR0 = 32768;
  TB0CTL = TBSSEL__ACLK | MC__UP;           // ACLK, up mode

  __bis_SR_register(LPM3_bits | GIE);       // Enter LPM3 w/ interrupt

  	// On return, do stuff
	P1OUT ^= BIT0;

  __no_operation();                         // For debugger
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
