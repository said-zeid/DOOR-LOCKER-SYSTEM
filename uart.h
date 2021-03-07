 /******************************************************************************
 *
 * Module: UART
 *
 * File Name: uart.h
 *
 * Description: Header file for the UART AVR driver
 *
 * Author: Mohamed Tarek
 *
 *******************************************************************************/

#ifndef UART_H_
#define UART_H_

#include "micro_config.h"
#include "std_types.h"
#include "common_macros.h"

/*******************************************************************************
 *                      ENUMS                                                  *
 *******************************************************************************/

typedef enum{
	disable_u,enable_u
}uart_interrupt;
typedef enum{
	async,sync
}uart_mode;
typedef enum{
	normal_speed,doubleSpeed
}uart_doublespeed;
typedef enum{
	five_bits,six_bits,seven_bits,eight_bits,nine_bits
}uart_char_size;
typedef enum{
	disable_parity,reserved,even,odd
}uart_parity;
typedef enum{
	one_bit,two_bits
}uart_stop_bits;
typedef enum{
	tx_rising_rx_falling,tx_falling_rx_rising
}uart_clock_polarity;

/*******************************************************************************
 *                     UART CONFIGURATIONS STRUCTURE                                    *
 *******************************************************************************/
typedef struct{

	uart_mode mode;
	uint16 baudrate;
	uart_doublespeed double_speed;
	uart_char_size char_size;
	uart_parity parity;
	uart_stop_bits stop_bits_select;
	uart_clock_polarity clock_polarity;
}Uart;

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/
void UART_init(const Uart *ptr);

void UART_sendByte(const uint8 data);

uint8 UART_recieveByte(void);

void UART_sendString(const uint8 *Str);

void UART_receiveString(uint8 *Str); // Receive until #

#endif /* UART_H_ */
