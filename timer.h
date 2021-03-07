/*
 * timer.h
 *
 *  Created on: ??þ/??þ/????
 *      Author: said
 */

#ifndef TIMER_H_
#define TIMER_H_
#include "std_types.h"
#include "common_macros.h"
#include "micro_config.h"

/*******************************************************************************
 *                                ENUMS                                        *
 *******************************************************************************/

typedef enum{
	timer0,timer1,timer2
}timer_num;

typedef enum{
	disable,enable
}timer_interrupt;

typedef enum{
	channelA,channelB
}timer1_channel_compare;

typedef enum{
	normal,toggle_or_reserved,noninverting,inverting
}pwm_invertingtype;

typedef enum
{
NO_CLOCK,F_CPU_CLOCK,F_CPU_8,F_CPU_64,F_CPU_256,F_CPU_1024
}timer_clock;

typedef enum{
	Normal0,PWM_Phase_Correct0,CTC0,Fast_PWM0
}timer0_modes;

typedef enum{
	Normal1,PWM_PhaseCorrect_8bit,PWM_PhaseCorrect_9bit,PWM_PhaseCorrect_10bit,CTC_OCRA1
	,Fast_PWM_8bit,Fast_PWM_9bit,Fast_PWM_10bit,PWM,Phase_and_Freq_Correct_ICR1,Phase_and_Freq_Correct_OCR1A
	,PWM_Phase_Correct_ICR1,PWM_Phase_Correct_OCRA1,CTC_ICR1,Reserved,Fast_PWM_ICR1,Fast_PWM_OCRA1
}timer1_modes;

typedef enum{
	Normal2,PWM_Phase_Correct2,CTC2,Fast_PWM2
}timer2_modes;

/*******************************************************************************
 *                        STRUCTURE Types Declaration                                   *
 *******************************************************************************/
typedef struct{
	timer_clock clock;
	timer0_modes mode;
	timer_interrupt interrupt_state;
	pwm_invertingtype invertingtype;
	uint8 TCNT0_VALUE;
	uint8 OCR0_VALUE;

}TIMER0;

typedef struct{
	timer_clock clock;
	timer1_modes mode;
	timer_interrupt interrupt_state;
	pwm_invertingtype invertingtype;
	timer1_channel_compare channel_num;
	uint16 TCNT1_VALUE;
	uint16 OCR1A_VALUE;
	uint16 OCR1B_VALUE;
	uint16 ICR1_VALUE;
}TIMER1;

typedef struct{
	timer_clock clock;
	timer2_modes mode;
	timer_interrupt interrupt_state;
	pwm_invertingtype invertingtype;
	uint8 TCNT2_VALUE;
	uint8 OCR2_VALUE;

}TIMER2;

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/
void timer0_init(const TIMER0 *ptr);
void timer1_init(const TIMER1 *ptr);
void timer2_init(const TIMER2 *ptr);

void clear_timer0(void);
void clear_timer1(void);
void clear_timer2(void);

void timer1_deinit(void);
void callback_timer(void(*ptr)(void));

#endif /* TIMER_H_ */
