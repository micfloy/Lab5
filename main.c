#include <msp430.h> 
#include "game_shell/game.h"
#include "LCD/LCD.h"
#include "buttons/button.h"

unsigned char direction;
unsigned char moveDetected = 0;
char player = 0;
char flag = 0;
char buttons[] = {BIT1, BIT2, BIT3, BIT4};
char string1[] = "You";
char string2[] = "LOOOOSE!";
char string3[] = "WIN!";

/*
 * main.c
 */
void init_timer();
void init_buttons();
void resetGame();

int main(void) {
	WDTCTL = (WDTPW | WDTHOLD);

	init_timer();
	init_buttons();
	__enable_interrupt();
	initSPI();
	LCDinit();

	while (1) {

		TACTL &= ~(MC1 | MC0);
		LCDclear();
		player = 0x80;
		flag = 0;
		printPlayer(player);
		TACTL |= MC1;

		char gameOver = 0;

		while (!gameOver) {

			if (moveDetected != 0) {
				clearPlayer(player);
				player = movePlayer(player, moveDetected);
				moveDetected = 0;
			}

			if (didPlayerWin(player)) {
				gameOver = 1;

				LCDclear();
				writeString(string1);
				cursorToLineTwo();
				writeString(string3);

				resetGame();
			}
			if (flag == 7) {
				gameOver = 1;

				LCDclear();
				writeString(string1);
				cursorToLineTwo();
				writeString(string2);

				resetGame();
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
	TACTL &= ~(MC1 | MC0); // stop timer

	TACTL |= TACLR; // clear TAR

	TACTL |= TASSEL1; // configure for SMCLK -

	TACTL |= ID1 | ID0; // divide clock by 8 -

	TACTL &= ~TAIFG; // clear interrupt flag

	TACTL |= MC1; // set count mode to continuous

	TACTL |= TAIE; // enable interrupt

	__enable_interrupt();
}

void init_buttons() {

	configureP1PinAsButton(BIT1 | BIT2 | BIT3 | BIT4);   // set buttons to input

	P1IE |= BIT1 | BIT2 | BIT3 | BIT4;                  // enable the interrupts
	P1IES |= BIT1 | BIT2 | BIT3 | BIT4; // configure interrupt to sense falling edges

	P1IFG &= ~(BIT1 | BIT2 | BIT3 | BIT4);                // clear flags
}

void resetGame() {
	pollP1Buttons(buttons, 4);
	moveDetected = 0;
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR(void) {
	TACTL &= ~TAIFG; // clear interrupt flag
	flag += 1;
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
