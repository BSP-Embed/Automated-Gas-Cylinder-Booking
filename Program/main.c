#include "main.h"

//#define TESTING

#ifdef TESTING
	const int8u FPhNum[] = "9902899863";
	const int8u GPhNum[] = "9902899863";
#else
	const int8u FPhNum[] = "8970989334";
	const int8u GPhNum[] = "9886764311";
#endif

volatile int8u monitflag = 0;
volatile int8u lowLPG = 0;
extern lcdptr;

const char *stat[5] = { 
	"Low Gas Level. Please Refill immediately.",
	"LPG",
	"LPG Gas leakage. Please take action.",
	"Fire Occurred. Please take action." ,
	" Automated SMS By: LPG Kit."
};

int main(void)
{
	init();
	while (TRUE) {
		if (monitflag == 1) {
			monitflag = 0;
			chksens();
			checkLPG();
		}
	}
	return 0;
}
static void init(void)
{
	buzinit();
	ledinit();

	SENSOR_DDR 		&= ~( _BV(LPG_SENS_PIN)  | _BV(FIRE_SENS_PIN));
	SENSOR_PORT		|=  ( _BV(LPG_SENS_PIN)  | _BV(FIRE_SENS_PIN));

	LED_APP_DDR			|= (_BV(GLED_PIN) | _BV(RLED_PIN));
	LED_APP_PORT 		&= ~(_BV(GLED_PIN) | _BV(RLED_PIN));

	RELAY_DDR 		|= _BV(RELAY_PIN);
	RELAY_PORT		&= ~_BV(RELAY_PIN);
	
	beep(2,100);
	lcdinit();
	uartinit();
	HX711init();
	GSMinit();
	inittest();
	tmr1init();
	disptitl();
	sei();
	StartTmr();
}
static void disptitl(void)
{
	lcdclrr(0);
	lcdws("LPG in Cyl:   Kg");
	lcdclrr(1);
	lcdws("Auto LPG Booking");
}
		
static void HX711init(void)
{
	HX711_DDR |= _BV(HX711_CLK_PIN);
	HX711_DDR &= ~_BV(HX711_DIN_PIN);

	HX711_PORT |= _BV(HX711_DIN_PIN);

	HX711_PORT |= _BV(HX711_CLK_PIN);
}
static void chksens(void)
{
	int8u i;

	static int8u smsfireflag = 0;
	static int8u smslpgflag = 0;

	volatile char GSMmsg[50];

	for (i = 0; i < 50; i++) 
		GSMmsg[i] = '\0';

	if (SENSOR_PIN & _BV(FIRE_SENS_PIN)) {
		if (smsfireflag == 0) {
			smsfireflag = 1;
			FanOn();
			buzon();
			lcdclrr(1);
			lcdws("Fire Occurred.");
			dlyms(2000);
			strcat(GSMmsg, stat[3]);
			strcat(GSMmsg, stat[4]);
			GSMsndmsg(FPhNum, GSMmsg);
			disptitl();
			buzoff();
		}
	} else {
		smsfireflag = 0;
		FanOff();
	} 

	if (SENSOR_PIN & _BV(LPG_SENS_PIN)) {
		if (smslpgflag == 0) {
			smslpgflag = 1;
			buzon();
			lcdclrr(1);
			lcdws("LPG Leakage Det");
			dlyms(2000);
			strcat(GSMmsg, stat[2]);
			strcat(GSMmsg, stat[4]);
			GSMsndmsg(FPhNum, GSMmsg);
			disptitl();
			buzoff();
		}
	} else {
		smslpgflag = 0;
	} 
			
}
static void checkLPG(void)
{
	static int8u smsflag = 0;
	int8u i;
	char GSMmsg[80];
	
	for (i = 0; i < 80; i++) 
		GSMmsg[i] = '\0';

	if ((measwt(1)-CYLINDER_WEIGHT) < LPG_THRESHOLD) {
		if (smsflag == 0) {
			smsflag = 1; 
			lowLPG = 1;
			RLEDOn();
			GLEDOff();
			beep(1,150);
			strcat(GSMmsg, stat[0]);
			strcat(GSMmsg, stat[4]);
			GSMsndmsg(FPhNum, GSMmsg);
			dlyms(3000);
			GSMsndmsg(GPhNum, stat[1]);
			disptitl();
		} 
	} else {
		smsflag  = 0;
		lowLPG = 0;
		RLEDOff();
		GLEDOn();
	}
}

static void tmr1init(void)
{
	TCNT1H   = 0xD3;
	TCNT1L   = 0x00;
	TIMSK   |= _BV(TOIE1);			//ENABLE OVERFLOW INTERRUPT
	TCCR1A   = 0x00;					
	TCCR1B  |= _BV(CS10) | _BV(CS11); /* PRESCALAR BY 16 */
}

/* overflows at every 100msec */
ISR(TIMER1_OVF_vect) 
{ 
	static int8u i,j,k;

	TCNT1H = 0xD3;
	TCNT1L = 0x00;
	
	if (++i >= 50) i = 0;
	switch(i) {
		case 0: case 2: ledon(); if(lowLPG) buzon(); break;
		case 1: case 3: ledoff(); if(lowLPG) buzoff(); break;
	} 
	if (++k >= 20) {
		k = 0;
		monitflag = 1;
	}
}
static void inittest(void)
{
	lcdclr();
	lcdws("SEN Warming:");
	if (SENSOR_PIN & _BV(LPG_SENS_PIN)) {
		lcdws("NOT OK");
		while (SENSOR_PIN & _BV(LPG_SENS_PIN)) {
			beep(1,200);
			dlyms(2000);
		}
	} else {
		lcdws("OK");
		dlyms(1000);
	}
	lcdr2();
	lcdws("LPG Chk:");

	
	if (measwt(0) < (CYLINDER_WEIGHT-CYLINDER_OFFSET)) {
		lcdws("No Cyl");
		while (measwt(0) < (CYLINDER_WEIGHT-CYLINDER_OFFSET)) {
			beep(1,100);
			dlyms(2000);
		}

	} else {
		lcdws("OK");
		dlyms(1000);
	}

	lcdclr();
}
static float measwt(int8u disp)
{
	int8u i;
	char s[10];

	volatile unsigned long weight;
	volatile float wtgms, wtkg;

	weight = 0;

	for (i = 0; i < 8; i++)	
		weight += MeasAvgWt();
		 
	weight >>= 3;

	weight = (weight - 8608400L); 
	wtgms = weight * 0.00459;

	wtkg = wtgms / 1000; 
	
	 CYLINDER_WEIGHT;
	 
	if (disp) {
		lcdptr = 0x8b;
		lcdws("   ");
		ftoa((wtkg-CYLINDER_WEIGHT),s);
		lcdptr = 0x8b;
		lcdws(s);
	}
	
	return wtkg;	

}

static unsigned long MeasAvgWt(void) 
{
		
	int8u i;
	volatile unsigned long Count = 0;


	HX711_PORT |= _BV(HX711_DIN_PIN);
	HX711_PORT &= ~_BV(HX711_CLK_PIN);

	while (HX711_INPIN & _BV(HX711_DIN_PIN)) ;

	for (i = 0; i < 24; i++)	{
			HX711_PORT |= _BV(HX711_CLK_PIN);
			dlyus(2);
			Count = Count<<1;

			HX711_PORT &= ~_BV(HX711_CLK_PIN);
			dlyus(1);

			if (HX711_INPIN & _BV(HX711_DIN_PIN))
				Count++; 

			dlyus(1);
	}
	
	
	HX711_PORT |= _BV(HX711_CLK_PIN);

	Count = Count ^ 0x800000;
	dlyus(2);
	 
	HX711_PORT &= ~_BV(HX711_CLK_PIN);
	
	return Count;
}
