#ifndef GSM_H
#define GSM_H

//INCLUDE HEADER FILES
#include "includes.h"

//DEFINE MACROS

//FUNCTION PROTOTYPES	
void 	GSMsndmsg		(int8u *PhNum, int8u *GSMMsg);
int8u 	GSMinit			(void);
int8u 	gsm_call_gen	(int8u *PhNum);
void	GSMRxCall		(void);		
void	GSMDisCall		(void);	

#endif
