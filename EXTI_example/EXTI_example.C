
#include	"extern.h"


void	FPPA0 (void)
{
	.ADJUST_IC	SYSCLK=IHRC/2, IHRC=16MHz, Init_RAM	

	GPCC.7	= 0;			// comparator disable
	INTEN 	= 0b00000000;	// disable all interrupt

	PADIER	= 0b00000000;
	PA		= 0b00000000;
	PAC		= 0b01000000;	// PA4 - input, PA6 - output
	PAPH	= 0b00010000;	// PA4 pull-high

//	PBDIER	= 0b00000000;	// wake UP from turn off PB0
	PB		= 0b00000000;
	PBC		= 0b00000000;
	PBPH	= 0b00000001;	// PB0 pull-high 

	// enable external interrupt PB.0 an all logical change
	INTEN	= 0b00000010;
	INTEGS	= 0b00000000;

	ENGINT;	// global interrupt enable 

	while (1)
	{

	
		
	}
}


void	Interrupt (void)
{
	pushaf;

	if (Intrq.PB0)
	{	
		if(PA.6 == 0) {
			PA.6 = 1;
		}
		else {
			PA.6 = 0;
		}

		Intrq.PB0	=	0;
		
	}

	popaf;
}


