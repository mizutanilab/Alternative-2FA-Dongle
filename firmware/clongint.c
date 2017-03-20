/*	clongint.cpp		v0.01	11/27/2010
	ALL RIGHTS RESERVED.   RYUTA MIZUTANI.
*/


#include "clongint.h"
//#include "LCDdisp.h"

void LongIntInit(struct CLongInt* pa) {
	pa->nElem = 0;
	LongIntReset(pa);
}

unsigned char LongIntAlloc(struct CLongInt* pa, int narg) {
	unsigned char i;
	if (narg > LI_NCHAR) return 255;
	for (i=0; i<narg; i++) {pa->pElem[i] = 0;}
	pa->nElem = narg;
	return 0;
}

unsigned char LongIntSet(struct CLongInt* pa, unsigned char* arg, int narg) {
	unsigned char i;
	unsigned char err = LongIntAlloc(pa, narg);
	if (err) return err;
	for (i=0; i<narg; i++) {pa->pElem[i] = arg[i];}
	return 0;
}

unsigned char LongIntGetAt(struct CLongInt* pa, unsigned char narg) {
	if ((pa->nElem) <= narg) return 0;
	return pa->pElem[narg];
}

void LongIntReset(struct CLongInt* pa) {
	unsigned char i;
	for (i=0; i<LI_NCHAR; i++) {pa->pElem[i] = 0;}
}

void LongIntCopy(struct CLongInt* pa, struct CLongInt* pb) {
	unsigned char i;
	LongIntAlloc(pa, pb->nElem);
	for (i=0; i<pb->nElem; i++) {pa->pElem[i] = pb->pElem[i];}
}

unsigned char ucResult[LI_NCHAR];
void LongIntMul(struct CLongInt* pa, struct CLongInt* pb) {
	unsigned char i, j, k, ucElem;
	unsigned int imul, iword; unsigned short long slsum;
	const unsigned char iThisMSB = (LongIntGetMSB(pa) >> 3) + 1;
	const unsigned char iArgMSB = (LongIntGetMSB(pb) >> 3) + 1;
	const unsigned char nResult = iThisMSB + iArgMSB;
	for (i=0; i<LI_NCHAR; i++) {ucResult[i] = 0;}
	for (i=0; i<iArgMSB; i++) {
		for (j=0; j<iThisMSB; j++) {
			ucElem = i + j;
			slsum = ((((unsigned int)ucResult[ucElem+1]) << 8) | ucResult[ucElem]);
			slsum += ((unsigned int)pb->pElem[i]) * pa->pElem[j];
			ucResult[ucElem] = (unsigned char)(slsum);
			ucResult[ucElem+1] = (unsigned char)(slsum >> 8);
			if ((slsum & 0x10000) == 0) continue;
			if (ucElem == (nResult - 2)) continue;
			for (k=ucElem+2; k<nResult; k++) {
				if (ucResult[k] < 0xff) {ucResult[k]++; break;}
				else ucResult[k] = 0;
			}
		}
	}
	for (i=0; i<LI_NCHAR; i++) {pa->pElem[i] = ucResult[i];}
	pa->nElem = nResult;
}

struct CLongInt p, q;
void LongIntPowerMod(struct CLongInt* pa, struct CLongInt* pow, struct CLongInt* div) {
	LongIntCopy(&p, pa);
	LongIntMod(&p, div);
	LongIntReset(pa);
	pa->pElem[0] = 1;
	pa->nElem = 1;
	LongIntCopy(&q, pow);
	while (LongIntGetAt(&q, LongIntGetMSB(&q) >> 3)) {
		if (LongIntGetAt(&q, 0) & 0x01) {
			LongIntMul(pa, &p); 
			LongIntMod(pa, div);
		}
		LongIntBitShiftRight(&q);
		LongIntMul(&p, &p);
		LongIntMod(&p, div);
	}
}

struct CLongInt reg;
void LongIntMod(struct CLongInt* pa, struct CLongInt* div) {
	int i;
	const int msbThis = LongIntGetMSB(pa);
	const int msbDiv = LongIntGetMSB(div);
	const int nShift = msbThis - msbDiv;
	LongIntInit(&reg);
	if (nShift <= 0) {
		if (LongIntGreaterOrEqual(pa, div)) {
			LongIntSub(pa, div);
		}
		return;
	}
	LongIntCopy(&reg, div);
	LongIntBitShiftLeft(&reg, nShift);
	for (i=nShift; i>=0; i--) {
		if (LongIntGreaterOrEqual(pa, &reg)) {
			LongIntSub(pa, &reg);
		}
		LongIntBitShiftRight(&reg);
	}
	return;
}

void LongIntBitShiftRight(struct CLongInt* pa) {
	unsigned char i;
	int icarry;
	for (i=0; i<pa->nElem; i++) {
		icarry = 0;
		if (i < pa->nElem -1) icarry = (((unsigned int)pa->pElem[i+1]) & 0x01) << 7;
		pa->pElem[i] = (unsigned char)(icarry | (pa->pElem[i] >> 1));
	}
}

#pragma udata section_result2
struct CLongInt result2;
#pragma udata
void LongIntBitShiftLeft(struct CLongInt* pa, unsigned int ishift) {
	const unsigned int iMSB = LongIntGetMSB(pa);
	const int iByteShift = ishift >> 3;
	const char ucBitShift = ishift & 0x07;
	char i;
	int iSourceByte;
	LongIntAlloc(&result2, (iMSB + ishift) / 8 + 1);
	for (i=result2.nElem-1; i>=0; i--) {
		iSourceByte = i - iByteShift;
		if (iSourceByte < 0) break;
		if (iSourceByte < pa->nElem)
			result2.pElem[i] |= (unsigned char)((((unsigned int)pa->pElem[iSourceByte]) << ucBitShift) & 0xff);
		if ((0 <= iSourceByte-1) && (iSourceByte-1 < pa->nElem)) 
			result2.pElem[i] |= (unsigned char)((pa->pElem[iSourceByte-1] >> (8-ucBitShift)) & 0xff);
	}
	LongIntCopy(pa, &result2);
}

void LongIntSub(struct CLongInt* pa, struct CLongInt* pb) {
	unsigned char i, nResult;
	int idigit;
	int icarry = 0;
	nResult = (pa->nElem > pb->nElem) ? (pa->nElem) : pb->nElem;
	LongIntAlloc(&result2, nResult);
	for (i=0; i<nResult; i++) {
		idigit = icarry;
		icarry = 0;
		if (i < pa->nElem) idigit += (unsigned int)(pa->pElem[i]);
		if (i < pb->nElem) idigit -= (unsigned int)(pb->pElem[i]);
		if (idigit < 0) {idigit = idigit + 0x100; icarry = -1;}
		result2.pElem[i] = (unsigned char)(idigit & 0xff);
	}
	LongIntCopy(pa, &result2);
}

unsigned int LongIntGetMSB(struct CLongInt* pa) {
	unsigned int iresult = 0;
	unsigned char i, j, idigit;
	for (i=0; i<pa->nElem; i++) {
		if (pa->pElem[i] == 0) continue;
		idigit = 0x01;
		for (j=0; j<8; j++) {
			if (pa->pElem[i] & idigit) iresult = (((unsigned int)i) << 3) + j;
			idigit = idigit << 1;
		}
	}
	return iresult;
}

unsigned char LongIntGreaterOrEqual(struct CLongInt* pa, struct CLongInt* pb) {
	unsigned char ucRet = 1;
	unsigned char iThisMSB, iArgMSB, nResult;
	char i;
	unsigned char ia, it;
	if ((pa->nElem == 0)||(pb->nElem == 0)) return 1;
	iThisMSB = (LongIntGetMSB(pa) >> 3) + 1;
	iArgMSB = (LongIntGetMSB(pb) >> 3) + 1;
	nResult = (iThisMSB > iArgMSB) ? iThisMSB : iArgMSB;
	for (i=nResult-1; i>=0; i--) {
		ia = 0; it = 0;
		if (i < pa->nElem) it = pa->pElem[i];
		if (i < pb->nElem) ia = pb->pElem[i];
		if (it > ia) break;
		else if (it < ia) {ucRet = 0; break;}
	}
	return ucRet;
}

