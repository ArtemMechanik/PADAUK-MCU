
#include	"extern.h"

BYTE count = 0;

void	FPPA0 (void)
{
	.ADJUST_IC	SYSCLK=IHRC/2, IHRC=16MHz, Init_RAM

	GPCC.7	= 0;			// comparator disable
	INTEN 	= 0b00000000;	// disable all interrupt

	PA		= 0b00000000;
	PAC		= 0b01000000;	// PA4 - input, PA6 - output
	PAPH	= 0b00010000;	// PA4 pull-high

	// timer16 setup
	T16M 	= 0b00100111;	// source = system clock, devider = 1, interrupt = 15bit
	//INTEN 	= 0b00000100;	// tim16 OVF interrupt
	//INTEGS	= 0b00000000;	// rising edge to trigger interrupt

	// timer 2 setup
	TM2S 	= 0b01111111;	// 8 bit, prescaller = 64, scalar = 32
	TM2C	= 0b00010001;	// source = system clock, output = disable, period mode
	TM2CT	= 0;
	TM2B	= 255;

	INTEN	= 0b01000000;	// timer2 interrupt

	ENGINT;	// global interrupt enable

	while (1)
	{
		
		if(PA.4 == 0) {
		//	count = 10;
		}
		else {
		//	count = 254;
		}
		

	}
}


void	Interrupt (void)
{
	pushaf;

	if (Intrq.TM2)
	{
		if(PA.6 == 0) {
			PA.6 = 1;
			count = 10;
		}
		else {
			PA.6 = 0;
			count = 254;
		}

		Intrq.TM2	=	0;

		TM2B = count;

		//STT16 count;	// load start timer value
	}

	popaf;
}

