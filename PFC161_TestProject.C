
#include	"extern.h"
#define AP 7
#define AN 5
#define BP 3
#define BN 6

#define STOP     0 // макросы дл???правлени???оторчиком
#define FORWRD   1
#define BACKWARD 2

#define DEADTIME 1000
#define PHASE_DEL 40000

BYTE motorDirection = STOP;

//BYTE phaseCounter = 0; // считаем фазу порота
WORD stepperPhaseDel[10]; 		  // задержки между фазами
BYTE stepperPhase[10]; // лог. состояни???инов дл???аждой фазы ??газ

WORD pntStart, pntEND, phaseCounter;

WORD stepperDelValue = 0;
void stepperDel (void) {
	while(stepperDelValue > 0) {
		.DELAY 1;
	}
}


void	FPPA0 (void)
{
	.ADJUST_IC	SYSCLK=IHRC/4		//	SYSCLK=IHRC/4

	
	// заполн??м массив ??гов и задрежек
	// перебираем массив в одну сторону - моторчик крутитится в лево, перебираем в другую - в право
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

	// нельзя просто так вз??ь и написать stepperPhase[phaseCounter] - выдаёт от??бку 
	// по??ому, будем через указатели работать...
	pntStart = & stepperPhase[0];
	pntEND = & stepperPhase[9];
	phaseCounter = pntStart;


	//	$ PA.3 Out; // нижний мост
	//	$ PA.6 Out;
	//	$ PA.5 Out; // верхний мост
	//	$ PA.7 Out;
	PA = 0b00000000;
	PAC = 0b11101000;

	PB = 0b00000000;
	PBC = 0b00000000;

	while(1)
	{
		// провер??м кака???нопка нажатан
		if(PA.4 == 0)
		{
			phaseCounter += 1;
			motorDirection = FORWRD;
		}
		else 
		{
			motorDirection = STOP;
		}

		if(PB.0 == 0)
		{
			phaseCounter -= 1;
			motorDirection = BACKWARD;
		}
		else
		{
			motorDirection = STOP;
		}




		// контроль переполнения счётчика
		if(phaseCounter == pntStart) 
		{
			phaseCounter = pntEND - 1; 
		}
		else if(phaseCounter == pntEND)
		{ 
			phaseCounter = pntStart + 1; 
		}

		// ??гаем по массиву фазы
		if(motorDirection != STOP)
		{
			PA = (PA & 0b00010111) | (*phaseCounter);
		} 
		else {
			PA = (PA & 0b00010111);
		}
		.DELAY 40000;


			/*
		if(PA.4 == 0) {
			PA = (PA & 0b00010111) | stepperPhase[1];
			.DELAY DEADTIME;
			PA = (PA & 0b00010111) | stepperPhase[2];
			.DELAY 40000;
			PA = (PA & 0b00010111) | stepperPhase[3];
			.DELAY DEADTIME;
			PA = (PA & 0b00010111) | stepperPhase[4];
			.DELAY 40000;
			PA = (PA & 0b00010111) | stepperPhase[5];
			.DELAY DEADTIME;
			PA = (PA & 0b00010111) | stepperPhase[6];
			.DELAY 40000;
			PA = (PA & 0b00010111) | stepperPhase[7];
			.DELAY DEADTIME;
			PA = (PA & 0b00010111) | stepperPhase[8]; 
			.DELAY 40000;
		}
		else if(PB.0 == 0) {
			PA = (PA & 0b00010111) | stepperPhase[8];
			.DELAY 40000;
			PA = (PA & 0b00010111) | stepperPhase[7]; 
			.DELAY DEADTIME;
			PA = (PA & 0b00010111) | stepperPhase[6];
			.DELAY 40000;
			PA = (PA & 0b00010111) | stepperPhase[5];
			.DELAY DEADTIME;
			PA = (PA & 0b00010111) | stepperPhase[4];
			.DELAY 40000;
			PA = (PA & 0b00010111) | stepperPhase[3];
			.DELAY DEADTIME;
			PA = (PA & 0b00010111) | stepperPhase[2];
			.DELAY 40000;
			PA = (PA & 0b00010111) | stepperPhase[1];
			.DELAY DEADTIME;
		}
		else {
			PA = (PA & 0b00010111);
		}
		*/
	 	//PA = 0b00000000;//PA = (PA & 0b00010111);

		//PA = 0b00000000;
		/*
		if(motorDirection == 0) {
			PA = 0x00;
			motorDirection = 1;
		}
		else {
			PA = 0xFF;
			motorDirection = 0;
		}
		*/

		/*
		if(PB.0 == 0) {
			PA.7 = 1;
			PA.5 = 0;
		}
		else {
			PA.7 = 0;
			PA.5 = 1;
		}
		*/
		
//		wdreset;

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

