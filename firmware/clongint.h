/*	clongint.h		v0.01	11/27/2010
*	ALL RIGHTS RESERVED.   RYUTA MIZUTANI.
*/

#define LI_NCHAR 50

struct CLongInt {
	int nElem;
	unsigned char pElem[LI_NCHAR];
};

void LongIntSub(struct CLongInt* pa, struct CLongInt* pb);//*pa -= *pb
void LongIntMul(struct CLongInt* pa, struct CLongInt* pb);//*pa *= *pb
void LongIntCopy(struct CLongInt* pa, struct CLongInt* pb);//*pa <== *pb
unsigned char LongIntGreaterOrEqual(struct CLongInt* pa, struct CLongInt* pb);//return (*pa >= *pb)
void LongIntMod(struct CLongInt* pa, struct CLongInt* div);
unsigned int LongIntGetMSB(struct CLongInt* pa);
void LongIntBitShiftRight(struct CLongInt* pa);
void LongIntBitShiftLeft(struct CLongInt* pa, unsigned int ishift);
unsigned char LongIntGetAt(struct CLongInt* pa, unsigned char narg);
unsigned char LongIntAlloc(struct CLongInt* pa, int narg);
void LongIntReset(struct CLongInt* pa);
void LongIntInit(struct CLongInt* pa);
unsigned char LongIntSet(struct CLongInt* pa, unsigned char* arg, int narg);
void LongIntPowerMod(struct CLongInt* pa, struct CLongInt* pow, struct CLongInt* div);



