/* Simple LED Blinker program for MSP430 */
#include <msp430g2553.h>

#define LED1 BIT0 			// The red LED on the LaunchPad
#define LED2 BIT6			// The green LED on the LaunchPad
#define LED_OUT P1OUT
#define LED_DIR P1DIR

void wait(void)          //delay function
{
  volatile int i;        //declare i as volatile int
  for(i=0;i<16000;i++);  //repeat 32000 times
}


int main(void)
{
    /* Init watchdog timer to off */
    WDTCTL = WDTPW|WDTHOLD;

    /* Init Output ports to GND */
    LED_OUT  = 0x00;

    /* I/O not module control */
    P1SEL  = 0x00;

    /*	Setup the data direction registers
    	LED1 is on P1.0 
		LED2 is on P1.6
    */
	LED_DIR = (LED1 + LED2);

    /* No Interrupts on Port Pins */
    P1IES  = 0x00;
    P1IE   = 0x00;

	/* Turn LED1 on */ 
	LED_OUT = LED1;

    /* Loop until the universe breaks down or power becomes a scarce resource*/
    while (1) {
        /* Toggle the LED ouput pins */
		LED_OUT ^= (LED1 + LED2);
		
		/* and wait for a bit */
        wait();
    }  /* while */
}
