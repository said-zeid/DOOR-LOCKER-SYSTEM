/*
 * reciever.c
 *
 *      Author: SAID ZEID
 */

#include"timer.h"
#include "uart.h"
#include "external_eeprom.h"

/*******************************************************************************
 *                           Global Variables                                  *
 *******************************************************************************/
extern uint8 g_timerCountFinish;        //// variable to make sure that we reached a certain time
extern uint8 g_tick;                     ///VARIABLE TO COUNT TIMER NO OF INTERRUPTS

uint8 password_arr[5];                   ///array to receive and store the password
uint8 g_compare=0;
uint8 g_counter=0;                       ///counter of all for loops used
uint16 eeprom_address=0X000A;

/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/
void SAVE_password(uint8 *passwordarr_Ptr)
{
	for( g_counter=0;g_counter<5;g_counter++){
		EEPROM_writeByte(eeprom_address, passwordarr_Ptr[g_counter]);   //write the password in address from 0x000A to 0x000A+5
		_delay_ms(10);

		eeprom_address++;

	}
	g_counter=0;                // clear the counter so if the function called again the counter starts from 0
	eeprom_address=0X000A;      //return the address into it's original value
}


uint8 COMPARE_password(uint8 *passwordarr_Ptr)
{
	uint8 mypassword=0;
	for( g_counter=0;g_counter<5;g_counter++)
	{
		EEPROM_readByte(eeprom_address, &mypassword);    //read the password saved in the eeprom
		_delay_ms(10);

		if(mypassword!=passwordarr_Ptr[g_counter])   //if the two passwords don't match
		{
			g_counter=0;                // clear the counter so if the function called again the counter starts from 0
			eeprom_address=0X000A;     //return the address into it's original value
			return 0;        ///don't match
		}
		eeprom_address++;
	}
	g_counter=0;                // clear the counter so if the function called again the counter starts from 0
	eeprom_address=0X000A;     //return the address into it's original value

	return 1;         // matched
}


void RECEIVE_password(uint8 *pass_Ptr)
{
	for( g_counter=0;g_counter<5;g_counter++){
		pass_Ptr[g_counter]=UART_recieveByte();   //receive password from MC1 keypad and store it in an array
	}
	g_counter=0;
}

void CREATE_passwordReceiver(void)
{
	uint8 flag=0;           //this flag is set to one when the two passwords matches to exit the do while loop
	do{

		/*************two way handshake between MC1 and MC2 to ensure sync **********************/
		while(UART_recieveByte()!='&'){}     //wait until the transmitter sends an '&' char to make sure it is ready
		UART_sendByte('&');     //send the '&' char back to tell the transmitter  i am ready too to receive the password

		RECEIVE_password(password_arr);                 //receive the  password for the first time


		SAVE_password(password_arr);                   //save the password in the specific location 0X000A

		UART_sendByte('$');                              ///i am ready to receive data

		RECEIVE_password(password_arr);         //receive the  password for the second time

		g_compare=COMPARE_password(password_arr);///COMPARE THE TWO PASWORDS IF==1 THEY ARE MATCHED, ELSE NOT MATCHED

		/***********WAIT UNTILE THE TRANSMITTER READY TO RECIVE**********************/

		UART_sendByte('&');                //SEND THE & CHAR TO TELL THE TRANSMITTER I WILL SEND DATA
		while(UART_recieveByte()!='&'){}   //WAIT FOR THE TRANSMITTER ACK

		/************************************************************************************/
		if(g_compare==1)
		{
			UART_sendByte('t');///////////TRUE THEY ARE MATCHED
			flag=1;  // the flag is set to one to exit the while loop
		}
		else
		{
			UART_sendByte('f');/////FALSE NOT MATCHE REENTER THE PASSWORD
		}
	}while(flag==0);
}


void STOP_buzzer(void)
{
	CLEAR_BIT(DDRD,5);
}


void CLOSE_door(void){
	SET_BIT(PORTD,PD6); // set IN1=1 & IN2=0 to rotate motor clockwise
	CLEAR_BIT(PORTD,PD7);
}


void OPEN_door(void){
	CLEAR_BIT(PORTD,PD6); // set IN1=1 & IN2=0 to rotate motor clockwise
	SET_BIT(PORTD,PD7);
}


void STOP_motor(void){
	CLEAR_BIT(PORTD,PD6);
	CLEAR_BIT(PORTD,PD7);
}


void DOOR_control(const TIMER1 *config_Ptr){

	SET_BIT(DDRD,PD6); //set the two motor input pins as output on AVR
	SET_BIT(DDRD,PD7);

	/////////////////DOOR OPENING/////////////
	OPEN_door();

	timer1_init(config_Ptr);        // start timer count

	callback_timer(STOP_motor);      ////send the stop motor function to the timer ISR

	while(g_timerCountFinish!=1){}  //wait until 15 seconds will pass
	UART_sendByte('L');        //send the 'L' char to the transmitter to clear the screen after door is opened
	timer1_deinit();           //reset the timer
	_delay_ms(3000);           ///wait 3 seconds
	////////////////DOOR CLOSE/////////////////
	CLOSE_door();
	timer1_init(config_Ptr);
	callback_timer(STOP_motor);    ////send the stop motor function to the timer ISR
	while(g_timerCountFinish!=2){} //wait until 15 seconds will pass
	UART_sendByte('L');
	timer1_deinit();

	g_timerCountFinish=0;
}


/*******************************************************************************
 *                               MAIN Function                                 *
 *******************************************************************************/

void main(void){

	uint8 action=0;   ///change or open door or ring the buzzer
	uint8 empty=0;

	/************************timer and uart config************************/
	TIMER1 timer1_config={F_CPU_1024,CTC_OCRA1,enable,normal,channelA,0,58594,1,0};//OCR1A=58594 ->> 7.5seconds
	Uart UartConfig ={async,9600,doubleSpeed,eight_bits,disable_parity,one_bit,tx_rising_rx_falling};
	/***********************************************************************/
	UART_init(&UartConfig);
	EEPROM_init();

	/********************************handling the first power on problem ***********/

	EEPROM_readByte(0x0311, &empty);            //read the value from a specific place in the memory
	_delay_ms(20);
	if(empty!=0X24)                   //means that it is the first power on and we have to create a new password
	{
		EEPROM_writeByte(0x0311,0X24);// write 0x24 in the memory in the first power on
		UART_sendByte('e');          //send 'e' char stands for (empty) memory location
		CREATE_passwordReceiver();   //create new password for the first time

	}
	else
		UART_sendByte('n');          //send 'n' char stands for (not empty) and the password is created before


	while(1){


		action=UART_recieveByte();      ///receive the action change password or open door or ring the buzzer

		if(action=='c'){                ///stands for (create password) the user entered #
			UART_sendByte('r');          // i am (ready) to receive the password
			RECEIVE_password(password_arr);
			g_compare=COMPARE_password(password_arr);

			if(g_compare==1)
			{
				UART_sendByte('t');///////////TRUE THEY ARE MATCHED
				CREATE_passwordReceiver();
			}
			else
			{
				UART_sendByte('f');/////FALSE NOT MATCHE REENTER THE PASSWORD
			}


		}



		else if(action=='o')                        ///stands for (open door) the user entered *
		{
			UART_sendByte('r');                     // i am (ready) to receive the password
			RECEIVE_password(password_arr);
			g_compare=COMPARE_password(password_arr);

			if(g_compare==1)
			{
				UART_sendByte('t');///////////TRUE THEY ARE MATCHED
				DOOR_control(&timer1_config);
			}
			else
			{
				UART_sendByte('f');/////FALSE NOT MATCHE REENTER THE PASSWORD
			}


		}




		else if(action=='z'){                  ///an action sent from the transmitter to activate the buzzer
			SET_BIT(DDRD,5);/////ENABLE BUZZER

			callback_timer(STOP_buzzer);       // send the function stop buzzer to the timer ISR
			g_tick=3;
			timer1_init(&timer1_config);
			while(g_timerCountFinish!=8){}    ///the timer counts 60 seconds when g_timerCountFinish=8
			timer1_deinit();
			g_timerCountFinish=0;
			UART_sendByte('q');   // send q char to clear the screen at the transmitter after the buzzer stop
		}
	}


}
