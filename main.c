/**
 * Author: C2C Michael Bentley
 * Date Last Modified: 11/11/13
 * Description: This is a game played on a 2x8 LCD display, using 4 buttons.  The player may move in all four directions and the
 * only goal is to move from the top-left corner of the screen to the bottom-right corner.  They player may only wait a couple seconds
 * between moves or they will lose.
 *
 */

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

	// Initialization
	init_timer();
	init_buttons();
	__enable_interrupt();
	initSPI();
	LCDinit();

	while (1) {

		// Initialization for every time the game resets
		TACTL &= ~(MC1 | MC0);
		LCDclear();
		player = 0x80;
		flag = 0;
		gameOver = 0;
		printPlayer(player);
		TACTL |= MC1;

		while (!gameOver) {

			// If the player makes it to the bottom-right corner, the game will display the winning message.
			if (didPlayerWin(player)) {
				gameOver = 1;
				LCDclear();
				writeString(string1);
				cursorToLineTwo();
				writeString(string3);
				_delay_cycles(100000);

				resetGame();

			}
			// If the player takes too long to move, they lose.
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

// Waits for a button to be pressed and then resets the player to the starting position.
void resetGame() {

	_delay_cycles(100000);
	char resetButton;
	resetButton = pollP1Buttons(buttons, 4);
	waitForP1ButtonRelease(resetButton);

}
// Helper method to testAndRespondToButtonPush.  Calls movePlayer for the specific button that was pushed.
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

// Resets the move timer and calls the movingPlayer method to move the player in the direction for the button pressed.
void testAndRespondToButtonPush(char buttonToTest) {

	// If pushed
	if (buttonToTest & P1IFG) {
		// If the edge is rising/falling (changes every time the method is called), move the player in the direction indicated by the button.
		if (buttonToTest & P1IES) {

			movingPlayer(buttonToTest);
			flag = 0;			// Clear the timer flag.
			TACTL |= MC1;

		} else {

			debounce();
		}

		P1IES ^= buttonToTest;			// Toggle the edge to trigger on.
		P1IFG &= ~buttonToTest;			// Clear the button flag.
	}
}

// Timer interrupt triggers about every third of a second.
#pragma vector = TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR(void) {
	TACTL &= ~TAIFG; // clear interrupt flag
	flag += 1;
}

// Triggers every time a change is detected in the button states.
#pragma vector=PORT1_VECTOR
__interrupt void Port_1_ISR(void) {
	if (!gameOver) {
		testAndRespondToButtonPush(BIT1);
		testAndRespondToButtonPush(BIT2);
		testAndRespondToButtonPush(BIT3);
		testAndRespondToButtonPush(BIT4);
	}
	P1IFG &= ~(BIT1 | BIT2 | BIT3 | BIT4);
}
