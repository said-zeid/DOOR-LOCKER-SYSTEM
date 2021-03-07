 /******************************************************************************
 *
 * Module: UART
 *
 * File Name: uart.c
 *
 * Description: Source file for the UART AVR driver
 *
 * Author: SAID ZEID
 *
 *******************************************************************************/

#include "uart.h"
#include <avr/io.h>

/*******************************************************************************
 *                      Preprocessor Macros                                    *
 *******************************************************************************/
#define UART_tx_interrupt_flag 0  //////no tx interrupt
#define UART_rx_interrupt_flag 0  //////no rx interrupt

/*******************************************************************************
 *                           Global Variables                                  *
 *******************************************************************************/
volatile uint8 received_data=0;       //global variable to store the received data from UDR


/*******************************************************************************
 *                      UART ISR                                               *
 *******************************************************************************/

#if(UART_rx_interrupt_flag)
ISR(USART_RXC_vect)
   {
	received_data=UDR;              //store the received data from UDR
	}
#endif

#if(UART_tx_interrupt_flag)
ISR(USART_TXC_vect)
   {
	SET_BIT(UCSRA,TXC);             //CLEAR TXC FLAG

	}
#endif

/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/
void UART_init(const Uart *ptr)
{
////////////////////////transmitter interrupt enable//////////////////////
#if(UART_tx_interrupt_flag)
	if(UART_tx_interrupt_flag)
		{
			SET_BIT(UCSRB,TXCIE);
			SET_BIT(SREG,7);
		}
#endif
	////////////////////////receiver interrupt enable//////////////////////

#if(UART_rx_interrupt_flag)
	if(UART_rx_interrupt_flag)
			{
			SET_BIT(UCSRB,RXCIE);
			SET_BIT(SREG,7);
			}
#endif


	UCSRB|= (1<<RXEN) | (1<<TXEN);////enable tx and rx pins

	/********************************setting the UCSRC REG ************************/
	SET_BIT(UCSRC,URSEL);///SELECT TO WRITE IN UCSRC REG

	UCSRC=(UCSRC&0XBF)|((ptr->mode)<<6);/////CHOOSE THE MODE

	UCSRC=(UCSRC&0XCF)|((ptr->parity)<<4);/////CHOOSE THE PARITY OPTIONS

	UCSRC=(UCSRC&0XF7)|((ptr->stop_bits_select)<<3);/////CHOOSE THE STOP BITS OPTIONS

	UCSRC=(UCSRC&0XFE)|((ptr->clock_polarity));/////CHOOSE THE CLOCK POLARITY

/************************************	CHOOSE THE CHAR SIZE ***************************/

	if(ptr->char_size==nine_bits){
		UCSRC=(UCSRC&0XF9)|((ptr->char_size)<<1);/////CHOOSE THE CHAR SIZE 9-BITS
        SET_BIT(UCSRB,UCSZ2);
	}
	else
		UCSRC=(UCSRC&0XF9)|((ptr->char_size)<<1);/////CHOOSE THE CHAR SIZE 5-6-7-8-BITS


	/**********************CHOOSE THE DOUBLE SPEED MODE ***************************/


	UCSRA=(UCSRA&0XFD)|((ptr->double_speed)<<1);/////CHOOSE THE DOUBLE SPEED MODE


	/*********************SETTING THE BAUD RATE**************************************/
	uint16 BAUD_PRESCALE;
	CLEAR_BIT(UBRRH,URSEL);             //////SET URSEL BIT TO CHOOSE TO WRITE IN UBRRH REG


	if(ptr->mode==sync)///SYNC MODE
	{
		 BAUD_PRESCALE =(((F_CPU / ((ptr->baudrate) * 2UL))) - 1);
		 UBRRH = BAUD_PRESCALE>>8;
		 UBRRL = BAUD_PRESCALE;
	}

	else ///ASYNC MODE
	{
		if(ptr->double_speed==doubleSpeed)
		{
			BAUD_PRESCALE =(((F_CPU / ((ptr->baudrate) * 8UL))) - 1);
			 UBRRH = BAUD_PRESCALE>>8;
			 UBRRL = BAUD_PRESCALE;
		}

		else
		{
			BAUD_PRESCALE =(((F_CPU / ((ptr->baudrate) * 16UL))) - 1);
			 UBRRH = BAUD_PRESCALE>>8;
			 UBRRL = BAUD_PRESCALE;
		}
	}
}
	
void UART_sendByte(const uint8 data)
{

	/*********************************interrupt************************************/

	if(UART_tx_interrupt_flag)
		{
			UDR = data;
		}
	/* UDRE flag is set when the Tx buffer (UDR) is empty and ready for 
	 * transmitting a new byte so wait until this flag is set to one */

	/*********************************polling************************************/

	else{
	while(BIT_IS_CLEAR(UCSRA,UDRE)){}
	/* Put the required data in the UDR register and it also clear the UDRE flag as 
	 * the UDR register is not empty now */	 
	UDR = data;
	/************************* Another Method *************************
	UDR = data;
	while(BIT_IS_CLEAR(UCSRA,TXC)){} // Wait until the transimission is complete TXC = 1
	SET_BIT(UCSRA,TXC); // Clear the TXC flag
	*******************************************************************/	
	}
}

uint8 UART_recieveByte(void)
{
	/*********************************interrupt************************************/

	/*********************************polling************************************/
	/* RXC flag is set when the UART receive data so wait until this 
	 * flag is set to one */
	while(BIT_IS_CLEAR(UCSRA,RXC)){}
	/* Read the received data from the Rx buffer (UDR) and the RXC flag 
	   will be cleared after read this data */	 
    return UDR;		

}
void UART_sendString(const uint8 *Str)
{
	uint8 i = 0;
	while(Str[i] != '\0')
	{
		UART_sendByte(Str[i]);
		i++;
	}
	/************************* Another Method *************************
	while(*Str != '\0')
	{
		UART_sendByte(*Str);
		Str++;
	}		
	*******************************************************************/
}

void UART_receiveString(uint8 *Str)
{
	uint8 i = 0;
	Str[i] = UART_recieveByte();
	while(Str[i] != '#')
	{
		i++;
		Str[i] = UART_recieveByte();
	}
	Str[i] = '\0';
}
