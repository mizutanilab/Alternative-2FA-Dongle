/*	clongint.h		v0.01	11/27/2010
*	ALL RIGHTS RESERVED.   RYUTA MIZUTANI.
*/
#include "stdafx.h"

typedef int TErr;
#if !defined( _CLONGINT_H_ )
#define _CLONGINT_H_

#define LI_NCHAR 50

struct CLongInt {
	int nElem;
	unsigned char pElem[LI_NCHAR];
};

void LongIntAdd(CLongInt* pa, CLongInt* pb);//*pa += *pb
void LongIntSub(CLongInt* pa, CLongInt* pb);//*pa -= *pb
void LongIntMul(CLongInt* pa, CLongInt* pb);//*pa *= *pb
void LongIntCopy(CLongInt* pa, CLongInt* pb);//*pa <== *pb
bool LongIntLessThan(CLongInt* pa, CLongInt* pb);//return (*pa < *pb)
bool LongIntGreaterOrEqual(CLongInt* pa, CLongInt* pb);//return (*pa >= *pb)
void LongIntMod(CLongInt* pa, CLongInt* div);
unsigned int LongIntGetMSB(CLongInt* pa);
void LongIntBitShiftRight(CLongInt* pa);
void LongIntBitShiftLeft(CLongInt* pa, unsigned int ishift);
unsigned char LongIntGetAt(CLongInt* pa, unsigned int narg);
void LongIntAddAt(CLongInt* pa, unsigned short sarg, unsigned char iElem);
TErr LongIntAlloc(CLongInt* pa, int narg = -1);
void LongIntReset(CLongInt* pa);
void LongIntInit(CLongInt* pa);
TErr LongIntSet(CLongInt* pa, unsigned char* arg, int narg);
void LongIntPowerMod(CLongInt* pa, CLongInt* pow, CLongInt* div);
CString LongIntGetHex(CLongInt* pa);


#endif // _CLONGINT_H_

