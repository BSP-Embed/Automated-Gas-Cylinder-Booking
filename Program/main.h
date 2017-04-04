#ifndef MAIN_H
#define MAIN_H

#include "includes.h"


#define SENSOR_DDR			DDRD
#define SENSOR_PORT			PORTD
#define SENSOR_PIN			PIND

#define LPG_SENS_PIN		PD2
#define FIRE_SENS_PIN		PD5

#define LED_APP_DDR				DDRD
#define LED_APP_PORT			PORTD

#define GLED_PIN			PD3
#define RLED_PIN			PD4

#define HX711_DDR			DDRB
#define HX711_PORT			PORTB
#define HX711_INPIN			PINB
#define HX711_CLK_PIN		PB1
#define HX711_DIN_PIN		PB0

#define RELAY_DDR			DDRC
#define RELAY_PORT			PORTC
#define RELAY_PIN			PC2


//DEFINE CONSTANT
/* ALL WEIGHTS IN KG's */

#define CYLINDER_WEIGHT		2.4
#define CYLINDER_OFFSET		0.2
#define LPG_THRESHOLD		1.0		


//DEFINE MACROS
#define StartTmr()			TCCR0  	|= _BV(CS01)
#define StopTmr()			TCCR0  	&= ~_BV(CS01)

#define FanOn()				RELAY_PORT |= _BV(RELAY_PIN);
#define FanOff()			RELAY_PORT &= ~_BV(RELAY_PIN);

#define RLEDOn()			LED_APP_PORT |= _BV(RLED_PIN)
#define RLEDOff()			LED_APP_PORT &= ~(_BV(RLED_PIN))

#define GLEDOn()			LED_APP_PORT |= _BV(GLED_PIN)
#define GLEDOff()			LED_APP_PORT &= ~(_BV(GLED_PIN))

//FUNCTION PROTOTYPES
static void		 init		(void);
static void		 HX711init	(void);
static void 	 disptitl 	(void);
static void 	tmr1init	(void);
static void 	inittest	(void);
static void 	chksens		(void);
static float 	measwt		(int8u disp);
static void 	checkLPG	(void);
static unsigned long MeasAvgWt(void);

#endif
