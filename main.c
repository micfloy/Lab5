#include <msp430.h> 
#include "game_shell/game.h"
#include "LCD/LCD.h"
#include "buttons/button.h"

char player = 0;
char flag = 0;
char gameOver = 0;
char buttons[] = { BIT1, BIT2, BIT3, BIT4 };
char string1[] = "You";
char string2[] = "LOOOOSE!";
char string3[] = "WIN!";

/*
 * main.c
 */
void init_timer();
void init_buttons();
void resetGame();
void testAndRespondToButtonPush(char buttonToTest);

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
		gameOver = 0;
		printPlayer(player);
		TACTL |= MC1;

		while (!gameOver) {

			if (didPlayerWin(player)) {
				gameOver = 1;
				LCDclear();
				writeString(string1);
				cursorToLineTwo();
				writeString(string3);
				_delay_cycles(100000);

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

	_delay_cycles(100000);
	char resetButton;
	resetButton = pollP1Buttons(buttons, 4);
	waitForP1ButtonRelease(resetButton);

}
void movingPlayer(char buttonToTest) {
	switch (buttonToTest) {
	case BIT1:
		player = movePlayer(player, UP);
		break;
	case BIT2:
		player = movePlayer(player, DOWN);
		break;
	case BIT3:
		player = movePlayer(player, LEFT);
		break;
	case BIT4:
		player = movePlayer(player, RIGHT);
		break;
	}
}

void testAndRespondToButtonPush(char buttonToTest) {

	if (buttonToTest & P1IFG) {

		if (buttonToTest & P1IES) {

			movingPlayer(buttonToTest);
			flag = 0;
			TACTL |= MC1;

		} else {

			debounce();
		}

		P1IES ^= buttonToTest;
		P1IFG &= ~buttonToTest;
	}
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR(void) {
	TACTL &= ~TAIFG; // clear interrupt flag
	flag += 1;
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1_ISR(void) {
	if(!gameOver) {
		testAndRespondToButtonPush(BIT1);
		testAndRespondToButtonPush(BIT2);
		testAndRespondToButtonPush(BIT3);
		testAndRespondToButtonPush(BIT4);
	}
	P1IFG &= ~(BIT1|BIT2|BIT3|BIT4);
}
