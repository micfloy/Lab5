#include <msp430.h> 
#include "game_shell/game.h"
#include "LCD/LCD.h"
#include "buttons/button.h"

unsigned char direction;
unsigned char moveDetected = 0;
char player = 0;

/*
 * main.c
 */
void init_timer();
void init_buttons();

int main(void) {
	WDTCTL = (WDTPW | WDTHOLD);

	init_timer();
	init_buttons();
	__enable_interrupt();
	initSPI();
	LCDinit();

	while (1) {

		LCDclear();
		player = 0x80;
		printPlayer(player);

		char gameOver = 0;

		while (gameOver == 0) {

			if (moveDetected != 0) {
				player = movePlayer(player, moveDetected);
				moveDetected = 0;
			}

			if(player == 0x87) {
				gameOver = 1;
			}
		}

	}

	return 0;
}

//
// YOUR TIMER A ISR GOES HERE
//

void init_timer() {
	// do timer initialization work
}

void init_buttons() {

	configureP1PinAsButton(BIT1 | BIT2 | BIT3 | BIT4);   // set buttons to input

	P1IE |= BIT1 | BIT2 | BIT3 | BIT4;                  // enable the interrupts
	P1IES |= BIT1 | BIT2 | BIT3 | BIT4; // configure interrupt to sense falling edges

	P1IFG &= ~(BIT1 | BIT2 | BIT3 | BIT4);                // clear flags
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1_ISR(void) {

	if (P1IFG & BIT1) {
		P1IFG &= ~BIT1;                         // clear flag
		moveDetected = UP;
	}

	if (P1IFG & BIT2) {
		P1IFG &= ~BIT2;                         // clear flag
		moveDetected = DOWN;
	}

	if (P1IFG & BIT3) {
		P1IFG &= ~BIT3;                         // clear flag
		moveDetected = LEFT;
	}
	if (P1IFG & BIT4) {
		P1IFG &= ~BIT4;
		moveDetected = RIGHT;
	}
}
