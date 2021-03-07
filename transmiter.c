/*
 * transmiter.c
 *
 *
 *      Author: SAID ZEID
 */


#include"timer.h"
#include "lcd.h"
#include "keypad.h"
#include "uart.h"
/*******************************************************************************
 *                           Global Variables                                  *
 *******************************************************************************/
uint8 g_counter=0;                 //counter of all for loops used
uint8 g_compareMATCH=0;

/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/

void CREAT_password(void)
{
	do {
		LCD_clearScreen();
		LCD_displayString("enter new pass:");
		LCD_goToRowColumn(1,0);

			UART_sendByte('&');   //send the '&' char to tell the RECIVER  i am going to send the password
			while(UART_recieveByte()!='&'){}//wait until the receiver sends an '&' char to make sure it is ready

		GET_displayPassword();           ///get the password (first) time

		while(UART_recieveByte()!='$'){}    ///wait until the receiver is ready to get the second password
		LCD_clearScreen();


		LCD_displayString("renter password");
		LCD_goToRowColumn(1,0);

		GET_displayPassword();        ///get the password for the (second) time

		while(UART_recieveByte()!='&'){}   //wait until the receiver sends an '&' char to make sure of sync
			UART_sendByte('&'); //send the '&' char back to tell the RECIVER  i am ready to receive the compare value


			g_compareMATCH=UART_recieveByte();

			if(g_compareMATCH=='f'){         //repeat the cycle again if not matched
				LCD_clearScreen();
				LCD_displayString("not matched");
				_delay_ms(1000);
			}


		}while(g_compareMATCH!='t');     ///if the two passwords (true) matched get out of the loop

	LCD_clearScreen();
	LCD_displayString("matched");
	_delay_ms(1000);
	LCD_clearScreen();

}



void GET_displayPassword(void)
{

		for( g_counter=0;g_counter<5;g_counter++)
		{
			uint8 key=0;
			while(!KeyPad_getPressedKey())
			{}
			key = KeyPad_getPressedKey(); /* get the pressed key number */

					if((key <= 9) && (key > 0))
					{
						UART_sendByte(key);     ///send the password to the receiver byte by byte
						LCD_intgerToString(key); /* display the pressed keypad switch */
					}
					else
					{
						if(key=='0'){
							UART_sendByte(key);
							LCD_displayCharacter(key);

						}
						else
						{
						  g_counter--;// Decrement the counter in case of * or # to have proper loop operation
						}
					}
					_delay_ms(500);//Press time */
		}
		g_counter=0;
}



void CHANGE_password(void){
	uint8 passwordCount=0;
	uint8 compareResult=0;
	do{
	LCD_clearScreen();
	LCD_displayString("enter password:");
	LCD_goToRowColumn(1,0);

	UART_sendByte('c');
	while(UART_recieveByte()!='r'){}///wait until the receiver is ready to receive a new password
	GET_displayPassword();

	compareResult=UART_recieveByte();

	if(compareResult=='t'){    //the password matched the one stored in the eeprom

		CREAT_password();
		passwordCount=3;      //to exit the do while loop
	}
	else if(compareResult=='f'){   //the password  didn't match the one stored in the eeprom
		passwordCount++;           // the loop will be repeated until password count =3
		if(passwordCount==3)
		{
			UART_sendByte('z');   /// send 'z' char to activate the buzzer
			LCD_clearScreen();
			LCD_displayString("THEIF");

			while(UART_recieveByte()!='q'){} //if 'q' char is received means that 60 seconds have passed
		}
	   else
	  {
			LCD_clearScreen();
				LCD_displayString("wrong password");
				_delay_ms(1500);
		}
	}

	}while(passwordCount!=3);



}


void CHECK_password(void){
	uint8 passwordCount=0;
	uint8 compareResult=0;
	do{
	LCD_clearScreen();
	LCD_displayString("enter password:");
	LCD_goToRowColumn(1,0);

	UART_sendByte('o');               //send  'o' char stands for (open door)
	while(UART_recieveByte()!='r'){}  ////ready to receive the password
	GET_displayPassword();            //enter the password on the keypad

	compareResult=UART_recieveByte();

	if(compareResult=='t'){    //if the password (true) matched

		LCD_clearScreen();
		LCD_displayString("DOOR IS OPENING");
		while(UART_recieveByte()!='L'){}//display door is opening on the screen for 15 seconds (until i receive L)
		LCD_clearScreen();
		_delay_ms(3000);
		LCD_displayString("DOOR IS CLOSING");
		while(UART_recieveByte()!='L'){}//display door is closing on the screen for 15 seconds (until i receive L)

		passwordCount=3;                                          //to exit the loop
	}
	else if(compareResult=='f'){      //if the password (false) not matched
		passwordCount++;              // the loop will be repeated until password count =3
		if(passwordCount==3)
		{
			UART_sendByte('z');      // send 'z' char to activate the buzzer
			LCD_clearScreen();
			LCD_displayString("THEIF");
			while(UART_recieveByte()!='q'){}  //if 'q' char is received means that 60 seconds have passed

		}
	   else
	  {
			LCD_clearScreen();
				LCD_displayString("wrong password");
				_delay_ms(1500);
		}
	}

	}while(passwordCount!=3);

}



/*******************************************************************************
 *                               MAIN Function                                 *
 *******************************************************************************/
void main(void)
{

	uint8 empty=0;
	/************************uart config************************/
	Uart UartConfig ={async,9600,doubleSpeed,eight_bits,disable_parity,one_bit,tx_rising_rx_falling};
	LCD_init();
	UART_init(&UartConfig);

	empty=UART_recieveByte();                         // check the first power on or not
	if(empty=='e'){                // if the eeprom is empty then create a new password
	CREAT_password();
	}

	while(1)
	{
		LCD_clearScreen();
		LCD_displayString("# TO CHANGE PASS");
		LCD_goToRowColumn(1,0);
		LCD_displayString("* TO OPEN DOOR");

		if(KeyPad_getPressedKey()=='#'){             ///# to change the password
			CHANGE_password();
		}
		else if(KeyPad_getPressedKey()=='*'){        // * to open the door but check the password first
			CHECK_password();

	}

}
}
