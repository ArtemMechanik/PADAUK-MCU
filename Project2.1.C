#include	"extern.h"

// H-bridge pins
#define AP 7
#define AN 5
#define BP 3
#define BN 6

// motor operating modes
#define STOP     0

#define FORWRD   1
#define BACKWARD 2

#define DEADTIME 30
#define PHASE_DEL 2000

BYTE motorDirection = STOP;

WORD stepperPhaseDel[10]; 	
BYTE stepperPhase[10]; 

WORD pntStart, pntEND, phaseCounter;
WORD pntDStart, pntDEND, delCounter;

WORD delValue = 0;

void delay_tick (void) {
	while(delValue > 0) {
		delValue -= 1;
		nop;
	}
}

void	FPPA0 (void)
{
	.ADJUST_IC	SYSCLK=IHRC/8, Init_RAM;	

	// phases of motor step
	stepperPhase[1] = (0<<AP)|(0<<AN)|(1<<BN)|(0<<BP);
	stepperPhase[2] = (0<<AP)|(1<<AN)|(1<<BN)|(0<<BP);
	stepperPhase[3] = (0<<AP)|(1<<AN)|(0<<BN)|(0<<BP);
	stepperPhase[4] = (0<<AP)|(1<<AN)|(0<<BN)|(1<<BP);
	stepperPhase[5] = (0<<AP)|(0<<AN)|(0<<BN)|(1<<BP);
	stepperPhase[6] = (1<<AP)|(0<<AN)|(0<<BN)|(1<<BP);
	stepperPhase[7] = (1<<AP)|(0<<AN)|(0<<BN)|(0<<BP);
	stepperPhase[8] = (1<<AP)|(0<<AN)|(1<<BN)|(0<<BP);


	stepperPhaseDel[1] = PHASE_DEL;
	stepperPhaseDel[2] = DEADTIME;
	stepperPhaseDel[3] = PHASE_DEL;
	stepperPhaseDel[4] = DEADTIME;
	stepperPhaseDel[5] = PHASE_DEL;
	stepperPhaseDel[6] = DEADTIME;
	stepperPhaseDel[7] = PHASE_DEL;
	stepperPhaseDel[8] = DEADTIME;

	// we can't write: stepperPhase[phaseCounter], will be a compilation error
	// therefore, we use pointers
	pntStart = & stepperPhase[0];
	pntEND = & stepperPhase[9];
	phaseCounter = pntStart;

	delCounter = 0;


	PA = 0b00000000;
	PAC = 0b11101000;

	PB = 0b00000000;
	PBC = 0b00000000;

	while(1)
	{
		// buttons control
		motorDirection = STOP;
		if(PA.4 == 0)
		{
			phaseCounter += 1;
			delCounter += 1;
			motorDirection = FORWRD;
		}
		if(PB.0 == 0)
		{
			phaseCounter -= 1;
			delCounter -=1;
			motorDirection = BACKWARD;
		}

		// curent phase
		if(phaseCounter == pntStart) 
		{
			phaseCounter = pntEND - 1;
		}
		else if(phaseCounter == pntEND)
		{ 
			phaseCounter = pntStart + 1; 
		}

		if(delCounter == 0)
		{
			delCounter = 2;
		}
		else if(delCounter == 3)
		{
			delCounter = 1;
		}

		// curent step
		if(motorDirection != STOP)
		{
			PA = (PA & 0b00010111) | (*phaseCounter);

			delValue = PHASE_DEL;
			if(delCounter == 1)
			{
				delValue = DEADTIME;
			}

			delay_tick();

		//	.DELAY 40000;
		} 
		else {
			PA = (PA & 0b00010111);
		}
	}
}



void	Interrupt (void)
{
	pushaf;

	if (Intrq.T16)
	{	//	T16 Trig
		//	User can add code
		Intrq.T16	=	0;
		//...
	}

	popaf;
}
