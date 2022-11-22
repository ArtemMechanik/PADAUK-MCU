
#include	"extern.h"

// pins assignments
/*
#define PWMout	3	// PA3 - outputPWM
#define A1		4	// PB4 - input
#define A2		5	// PB5 - output
#define V1		6	// PB6 - analog input
#define	V2		2	// PB2 - analog input
#define V3		0	// PB0 - analog input

test pins
PB1 - UART only TX for debug
UART period send 200mS
*/

//#define debug

#define ADC_run_V1	0b11011000	// stert adc conversion on AD6 (PB6) - V1
#define ADC_run_V2	0b11001000	// stert adc conversion on AD2 (PB2) - V2
#define ADC_run_V3	0b11000000	// stert adc conversion on AD0 (PB0) - V3

// variables
BYTE A1_state = 0;
BYTE A2_state = 0;
WORD count16 = 132;
BYTE onOffAlgoritmCounter = 0;
BYTE PWM_dutyCycle = 13;
BYTE PWM_dutyCycle_dir = 0; // 1-increment, 0-decrement
BYTE PWM_dutyCycle_per = 1;
BYTE PWM_dutyCycle_counter = 0;

// ADC
BYTE V1_adc_value = 0;
BYTE V2_adc_value = 0;
BYTE V3_adc_value = 0;

// param
WORD V1_param = 0;
WORD V2_param = 0;
WORD V3_param = 0;

// flag
BYTE V1_param_flag = 0;
BYTE V2_param_flag = 0;
BYTE V3_param_flag = 0;

//counters
WORD timeCounter = 0;
WORD timeCounter_threshold = 0;


// flags
BYTE ON_flag = 0;
BYTE OFF_flag = 0;


// UART
BYTE uartStep = 0;
BYTE uartData = 0;
BYTE tempData;
BYTE uartDataBitCounter = 0;
BYTE uartPeriodCounter = 0;

void	FPPA0 (void)
{
	.ADJUST_IC	SYSCLK=IHRC/2, IHRC=16MHz, VDD=5V, Init_RAM	// clock frequency 8MHz

	GPCC.7	= 0;	// comparator disable

	// ADC setup
	adcc	= 0b10011000;	// adc enable, AD6 (PB6)
	$ ADCM /16;				// adc clock = system clock / 8

	// setup timer2 to generate PWM 15kHz on PA3
	tm2c	= 0; 				//0b00101010; clock - IHRC (16 Mhz), output PWM - PA3, PWMmode, inverse the polarity - disable
	tm2ct	= 0;
	tm2s	= 0b00100011;		// 8bit PWM, pre-scalar = 4, clock scalar = 3 
	tm2b	= PWM_dutyCycle;	// start duty cycle = 5%

	PBDIER	= 0b00010000;	// disable all wake-up event from portB
	PB		= 0b00000000;
	PBC 	= 0b10100010;	// PB7-output (test), PB5 - output, PB4 - input, PB1 - output (UART)
	PBPL	= 0b00010000;	// PB4 - pull-low 

	// setup timer16 to generate interrupt 1kHz
	T16M 	= 0b00111000;	// source = system clock, prescaller = 64 => F_timer = 125000Hz, value STT16 = 125 => T = 1mS
	STT16 count16

	// interrupt setup
	INTEN	= 0b00000100;	// tim16 ovf interrupt
	INTEGS	= 0b00000000;

	.Delay 20000;

	// watchDog setup
	clkmd	|=0b00000110;	// enable ILRC, enable wathDog
	MISC	= 0b00000001;	// 16k ILRC

	ENGINT;	// global interrupt enable 

	while (1)
	{
		// read ADC
		adcc = ADC_run_V1; 	
		while(!AD_DONE) NULL; 
		V1_adc_value = ADCRH; 

		adcc = ADC_run_V2; 	
		while(!AD_DONE) NULL; 
		V2_adc_value = ADCRH; 

		adcc = ADC_run_V3; 	
		while(!AD_DONE) NULL; 
		V3_adc_value = ADCRH;

		// set V1,V2,V3 modes, set parameters according to the modes
		V1_param = 1500;
		V1_param_flag = 0;
		if(V1_adc_value > 75)	{ V1_param = 1500; V1_param_flag = 1; }
		if(V1_adc_value > 145)	{ V1_param = 3000; V1_param_flag = 2; }
		if(V1_adc_value > 180)	{ V1_param = 4500; V1_param_flag = 3; }
		if(V1_adc_value > 200)	{ V1_param = 6000; V1_param_flag = 4; }

		V2_param = 0;
		V2_param_flag = 0;
		if(V2_adc_value > 75)	{ V2_param = 0;	   V2_param_flag = 1; }
		if(V2_adc_value > 145)	{ V2_param = 2000; V2_param_flag = 2; }
		if(V2_adc_value > 180)	{ V2_param = 4000; V2_param_flag = 3; }
		if(V2_adc_value > 200)	{ V2_param = 6000; V2_param_flag = 4; }

		V3_param = 255;							// disable
		if(V3_adc_value > 75)	V3_param = 102; // 40%
		if(V3_adc_value > 145)	V3_param = 153;	// 60%
		if(V3_adc_value > 180)	V3_param = 205;	// 80%
		if(V3_adc_value > 200)	V3_param = 254;	// 100%

		// control A2 output
		if(PWM_dutyCycle > V3_param) PB.5 = 1;
		else 						  PB.5 = 0;

		// when there is a high level on the line A1
		if(PB.4 != 0) {
			if(onOffAlgoritmCounter	== 0)	onOffAlgoritmCounter = 1;	
		}


		switch(onOffAlgoritmCounter) {
			// switch on enabling
			case 0:
				if(PB.4 != 0) onOffAlgoritmCounter = 1;
			break;

			// switch on, setup timer
			case 1:
				timeCounter = 0;
				timeCounter_threshold = V1_param;
				PWM_dutyCycle = 13;
				tm2c = 0b00101010;
				tm2b = PWM_dutyCycle;
				PWM_dutyCycle_per = V1_param>>8;	// period of increment duty sycle
				PWM_dutyCycle_counter = 0;
				PWM_dutyCycle_dir = 1;
				onOffAlgoritmCounter = 2;
			break;

			// wait rising PWM duty cycle
			case 2:
				if(timeCounter >= timeCounter_threshold) onOffAlgoritmCounter = 3;
				tm2b = PWM_dutyCycle;
			break;

			// switch off enabling
			case 3:
				if(PB.4 == 0) onOffAlgoritmCounter = 4;
			break;

			// switch delay, setup timer
			case 4:
				timeCounter = 0;
				timeCounter_threshold = V2_param;
				onOffAlgoritmCounter = 5;
			break;

			// wait while switch delay is work
			case 5:
				if(timeCounter >= timeCounter_threshold) onOffAlgoritmCounter = 6;
			break;

			// switch off, setup timer
			case 6:
				timeCounter = 0;
				timeCounter_threshold = 6000;
				PWM_dutyCycle_per = 6000>>8;
				PWM_dutyCycle_counter = 0;
				PWM_dutyCycle_dir = 0;
				onOffAlgoritmCounter = 7;
			break;

			case 7:
				if(timeCounter >= timeCounter_threshold) onOffAlgoritmCounter = 8;
				tm2b = PWM_dutyCycle;
			break;

			case 8:
				tm2c = 0;
				onOffAlgoritmCounter = 0;
			break;


		}

		#if defined (debug)
			if(onOffAlgoritmCounter == 2) {
				PB.7 = 1;
			}
			else {
				PB.7 = 0;
			}
		#endif
		
		wdreset; // clear watchdog

		
	}
}


void	Interrupt (void)
{
	pushaf;
	

	if (Intrq.T16)
	{	
		/*
		if(PB.1 != 0) PB.1 = 0;
		else PB.1 = 1;
		*/

		// control timer counters
		if(timeCounter < timeCounter_threshold) timeCounter = timeCounter + 1;

		if(PWM_dutyCycle_dir == 1) {
			if(PWM_dutyCycle < 255) {
				PWM_dutyCycle_counter += 1;
				if(PWM_dutyCycle_counter >=  PWM_dutyCycle_per) {
					PWM_dutyCycle_counter = 0;
					PWM_dutyCycle = PWM_dutyCycle + 1;
				}
			}
		}
		else {
			if(PWM_dutyCycle > 0) {
				PWM_dutyCycle_counter += 1;
				if(PWM_dutyCycle_counter >=  PWM_dutyCycle_per) {
					PWM_dutyCycle_counter = 0;
					PWM_dutyCycle = PWM_dutyCycle - 1;
				}
			}
		}

		
		// UART transmit period = 200mS
		uartPeriodCounter = uartPeriodCounter + 1;
		if(uartPeriodCounter > 200) {
			uartPeriodCounter = 0;
			if(uartStep == 0) {
				uartStep = 1;
			}
		}

		#if defined (debug)
		// UART transmit execute
		switch(uartStep) {
			// wait start transmit
			case 0:
				PB.1 = 1;
			break;

			// start bit
			case 1:
				PB.1 = 0;
				uartDataBitCounter = 0;
				tempData = V1_param_flag;
				uartStep = 2;
			break;

			// transmit word 8 bit
			case 2:
				if(tempData.0 != 0) {
					PB.1 = 1;
				}
				else {
					PB.1 = 0;
				}
				sr	tempData; // Shift right the content of memory, shift 0 to bit 7		

				uartDataBitCounter = uartDataBitCounter + 1;
				if(uartDataBitCounter == 8) uartStep = 3;
			break;

			// stop bit
			case 3:
				PB.1 = 1;
				uartStep = 0;
			break;				
		}
		#endif
		

		STT16 count16;	// load value in counter
		Intrq.T16	=	0;

	}		

	popaf;
}




