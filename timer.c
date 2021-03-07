/*
 * timer.c
 *
 *  Created on: ??þ/??þ/????
 */

#include"timer.h"
/*******************************************************************************
 *                      Preprocessor Macros                                    *
 *******************************************************************************/
#define interrupt_flag 1  //////WE WILL USE interrupts

/*******************************************************************************
 *                           Global Variables                                  *
 *******************************************************************************/
/* Global variables to hold the address of the call back function in the application */
static volatile void (*g_callback_timer_ptr)(void)=NULL_PTR;


uint8 g_tick=0;                         // variable to count the no of compare top value interrupts
uint8 g_timerCountFinish=0;             // variable to check if we reached the required time value

/*******************************************************************************
 *                       Interrupt Service Routines                            *
 *******************************************************************************/
#if(interrupt_flag)                    // if we don't use the interrupt(the interrrupt_flag=0) this code will be cancelled
ISR(TIMER0_OVF_vect){
	if(g_callback_timer_ptr!=NULL_PTR)   ////CHECKING IF A FUNCTION IS USED OR NOT
		(*g_callback_timer_ptr)();             //calling the required function
}
ISR(TIMER0_COMP_vect){
	if(g_callback_timer_ptr!=NULL_PTR)
		(*g_callback_timer_ptr)();

}
ISR(TIMER1_OVF_vect){
	if(g_callback_timer_ptr!=NULL_PTR)
		(*g_callback_timer_ptr)();

}
ISR(TIMER1_COMPA_vect){

	g_tick++;
	if(g_tick==2)                         // g_tick =2 means we reached 15sec
	{
		if(g_callback_timer_ptr!=NULL_PTR)     ////CHECKING IF A FUNCTION IS USED OR NOT
			(*g_callback_timer_ptr)();          //calling the required function
		g_tick=0;                         // start count no of interrrupts from the beginning
		g_timerCountFinish++;
	}
	else if(g_tick==11){
		if(g_callback_timer_ptr!=NULL_PTR)   ////CHECKING IF A FUNCTION IS USED OR NOT
			(*g_callback_timer_ptr)();        //calling the required function
		g_tick=0;                             // start count no of interrrupts from the beginning
		g_timerCountFinish=8;                  //means we reached 60sec
	}
}
ISR(TIMER1_COMPB_vect){
	if(g_callback_timer_ptr!=NULL_PTR)
		(*g_callback_timer_ptr)();

}
ISR(TIMER2_OVF_vect){
	if(g_callback_timer_ptr!=NULL_PTR)
		(*g_callback_timer_ptr)();

}
ISR(TIMER2_COMP_vect){
	if(g_callback_timer_ptr!=NULL_PTR)
		(*g_callback_timer_ptr)();

}
#endif

/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/
void timer0_init(const TIMER0 *ptr){
	TCCR0=(TCCR0&0XCF)|((ptr->invertingtype)<<4);     ///setting the inverting type
	TCNT0=ptr->TCNT0_VALUE;                            //setting the initial value of the timer0
	OCR0=ptr->OCR0_VALUE;                              //setting the compare value of the timer0


	if(ptr->mode == Normal0)
	{
		CLEAR_BIT(TCCR0,WGM00);            ////WGM00=0, WGM01=0 choose overflow mode
		CLEAR_BIT(TCCR0,WGM01);

		SET_BIT(TCCR0,FOC0);               ///FOC0=1 because non-pwm mode

		CLEAR_BIT(TCCR0,COM00);            ///COMM00=0,COMM01=0 to choose normal port operation
		CLEAR_BIT(TCCR0,COM01);

		if(ptr->interrupt_state==enable)
		{
			SET_BIT(TIMSK,TOIE0);                 ////enable overflow mode interrupt
		}

	}
	else if(ptr->mode == PWM_Phase_Correct0){

		SET_BIT(TCCR0,WGM00);                             ////WGM00=1 ,WGM01=0 PWM_Phase_Correct0 mode
		CLEAR_BIT(TCCR0,WGM01);

		CLEAR_BIT(TCCR0,FOC0);                             ///FOC0=0 because pwm mode
	}
	else if(ptr->mode == CTC0){
		CLEAR_BIT(TCCR0,WGM00);                        ////WGM00=0 ,WGM01=1 choose compare mode
		SET_BIT(TCCR0,WGM01);

		SET_BIT(TCCR0,FOC0);                          /////FOC0=1 because non-pwm mode

		if(ptr->interrupt_state==enable)
			SET_BIT(TIMSK,OCIE0);                     ////enable compare mode interrupt

	}
	else if(ptr->mode == Fast_PWM0){
		SET_BIT(TCCR0,WGM00);                        ////WGM00=1 ,WGM01=1 FAST PWM mode
		SET_BIT(TCCR0,WGM01);
		CLEAR_BIT(TCCR0,FOC0);                        ///FOC0=0 because pwm mode

	}

	TCCR0=(TCCR0&0XF8)|(ptr->clock);                   ////STARTING THE TIMER by setting the clock prescale
}


void timer1_init(const TIMER1 *ptr){
	ICR1=ptr->ICR1_VALUE;
	TCNT1=ptr->TCNT1_VALUE;                         //setting the initial value of the timer1

	/**************************MODE SELECTION*****************************/

	uint8 mode_num =ptr->mode;
	TCCR1A=(TCCR1A&0XFC)|(mode_num&0X03);        /// set WGM10, WGM11
	TCCR1B=(TCCR1B&0XE7)|((mode_num&0X0C)<<1);  /// set WGM12, WGM13
	/********************************************************************
	 *
	 * *********************INVERTING TYPE AND CHANNEL SELECT********************************/

	switch(ptr->channel_num){                           /////choose channel A or channel B
	case 0:                                             ///means channel A

		TCCR1A=(TCCR1A&0X3F)|((ptr->invertingtype)<<6);////COMA0 and COMA1
		OCR1A=ptr->OCR1A_VALUE;                         ///set compare value on channel A
		if(mode_num==0 ||mode_num==4 ||mode_num==12)    //means normal or ctc_icr1 or ctc_ocra (non-pwm modes)
		{
			SET_BIT(TCCR1A,FOC1A);                   ////set FOC1A (non-pwm modes)

			/******************setting the compare interrupt enable**********************/
			if((ptr->interrupt_state)&&(mode_num==4))          //if we use interrupt at ctc mode
			{
				SET_BIT(TIMSK,OCIE1A);                     // enable timer compare interrupt on channel A
				SET_BIT(SREG,7);                           //enable I-bit
			}
			/******************************************************************/
		}
		else
		{
			CLEAR_BIT(TCCR1A,FOC1A);                 ////clear FOC1A if we use (pwm mode)
		}
		break;


	case 1:                                           ///means channel B
		TCCR1A=(TCCR1A&0XCF)|((ptr->invertingtype)<<4);////COMB0 and COMB1
		OCR1B=ptr->OCR1B_VALUE;                      ///set compare value on channel B

		if(mode_num==0 ||mode_num==4 ||mode_num==12)  //means normal or ctc_icr1 or ctc_ocrb (non-pwm modes)
		{
			SET_BIT(TCCR1A,FOC1B);                    ////set FOC1B (non-pwm modes)

			/******************setting the compare interrupt enable**********************/
			if((ptr->interrupt_state)&&(mode_num==4))      //if we use interrupt at ctc mode
			{
				SET_BIT(TIMSK,OCIE1B);                     //enable timer compare interrupt on channel B
				SET_BIT(SREG,7);                           //enable I-bit
			}
			/*************************************************************/
		}
		else
		{
			CLEAR_BIT(TCCR1A,FOC1B);            ////clear FOC1B if we use (pwm mode)
		}
		break;
	}
	/***********************************normal mode interrupt enable******************************/

	if((ptr->interrupt_state)&&(mode_num==0))        //if we use interrupt at normal mode
		SET_BIT(TIMSK,TOIE1);                        //enable timer normal mode interrupt

	/********************************************************************************************/
	TCCR1B=(TCCR1B&0XF8)|(ptr->clock);          /////timer prescaler (starting the timer clock)


}



void timer2_init(const TIMER2 *ptr){

	TCCR2=(TCCR2&0XCF)|((ptr->invertingtype)<<4);          ///setting the inverting type
	TCNT2=ptr->TCNT2_VALUE;                                 //setting the initial value of the timer2
	OCR2=ptr->OCR2_VALUE;                                  //setting the compare value of the timer2
	/*******************************mode select************************************/

	if(ptr->mode == Normal2){
		CLEAR_BIT(TCCR2,WGM20);                           ///WGM20=0, WGM21=0 choose overflow mode
		CLEAR_BIT(TCCR2,WGM21);

		SET_BIT(TCCR2,FOC2);                              ///FOC2=1 because non-pwm mode

		CLEAR_BIT(TCCR2,COM20);                           ///COMM20=0,COMM21=0 to choose normal port operation
		CLEAR_BIT(TCCR2,COM21);

		if(ptr->interrupt_state==enable)
		{
			SET_BIT(TIMSK,TOIE2);  						////enable overflow mode interrupt
		}

	}
	else if(ptr->mode == PWM_Phase_Correct2){
		SET_BIT(TCCR2,WGM20);                     ////WGM20=1 ,WGM21=0 PWM_Phase_Correct2 mode
		CLEAR_BIT(TCCR2,WGM21);

		CLEAR_BIT(TCCR2,FOC2);                   ///FOC2=0 because pwm mode
	}
	else if(ptr->mode == CTC2){
		CLEAR_BIT(TCCR2,WGM20);                 ////WGM20=0 ,WGM21=1 choose compare mode
		SET_BIT(TCCR2,WGM21);

		SET_BIT(TCCR2,FOC2);                   /////FOC2=1 because non-pwm mode

		if(ptr->interrupt_state==enable)
			SET_BIT(TIMSK,OCIE2);             ////enable compare mode interrupt

	}
	else if(ptr->mode == Fast_PWM2){
		SET_BIT(TCCR2,WGM20);                      ////WGM20=1 ,WGM21=1 FAST PWM mode
		SET_BIT(TCCR2,WGM21);

		CLEAR_BIT(TCCR2,FOC2); 					   ///FOC2=0 because pwm mode

	}


	/*************************************************************************************/
	TCCR2=(TCCR2&0XF8)|(ptr->clock);      ////STARTING THE TIMER CLOCK

}



void clear_timer0(void){
	TCNT0 = 0;
}


void clear_timer1(void){
	TCNT1 = 0;
}


void clear_timer2(void){
	TCNT2 = 0;
}


void timer1_deinit(void){
	TCCR1A=0;
	TCCR1B=0;
	TCNT1=0;
	TIMSK=0;
	SREG=0;
}



void callback_timer(void(*ptr)(void)){
	g_callback_timer_ptr=ptr;
}

