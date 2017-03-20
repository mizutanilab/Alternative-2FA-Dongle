/*	clongint.cpp		v0.01	11/27/2010
	ALL RIGHTS RESERVED.   RYUTA MIZUTANI.
*/

#include "stdafx.h"

#if !defined( _CLONGINT_CPP_ )
#define _CLONGINT_CPP_
#include "clongint.h"

void LongIntInit(CLongInt* pa) {
	pa->nElem = 0;
}

TErr LongIntAlloc(CLongInt* pa, int narg) {
	if (narg > LI_NCHAR) return 21001;
	for (int i=0; i<narg; i++) {pa->pElem[i] = 0;}
	pa->nElem = narg;
	return 0;
}

TErr LongIntSet(CLongInt* pa, unsigned char* arg, int narg) {
	TErr err = LongIntAlloc(pa, narg);
	if (err) return err;
	LongIntReset(pa);
	for (int i=0; i<narg; i++) {pa->pElem[i] = arg[i];}
	return 0;
}

unsigned char LongIntGetAt(CLongInt* pa, unsigned int narg) {
//return 1;/////////////////////////////////////
	if (pa->pElem == NULL) return 0;
	if ((unsigned int)(pa->nElem) <= narg) return 0;
	return pa->pElem[narg];
}

void LongIntReset(CLongInt* pa) {
	if (pa->pElem) {
		for (int i=0; i<pa->nElem; i++) {pa->pElem[i] = 0;}
	}
}


void LongIntAddAt(CLongInt* pa, unsigned short sarg, unsigned char iElem) {
	if (iElem == (unsigned int)(pa->nElem - 1)) {
		pa->pElem[iElem] = (unsigned char)(pa->pElem[iElem] + sarg);
	} else {
		unsigned __int16 iword = ((unsigned __int16)(pa->pElem[iElem+1] << 8)) + pa->pElem[iElem];// + sarg;
		unsigned __int16 isum = iword + sarg;
		pa->pElem[iElem] = (unsigned char)(isum & 0xff);
		pa->pElem[iElem+1] = (unsigned char)((isum >> 8) & 0xff);
		if (iword > ((unsigned __int16)0xffff - sarg)) {
			//if ((isum & 0x10000) == 0) return;
			if (iElem != (unsigned char)(pa->nElem - 2)) {
				for (unsigned char k=iElem+2; k<pa->nElem; k++) {
					if (pa->pElem[k] < 0xff) {pa->pElem[k]++; break;}
					else pa->pElem[k] = 0;
				}
			}
		}
	}
}

CString LongIntGetHex(CLongInt* pa) {
	if (pa->nElem == 0) return "0";
	CString result = "";
	CString digit;
	int nmax = 0;
	for (int i=0; i<pa->nElem; i++) {
		if (pa->pElem[i] != 0) nmax = i;
	}
	for (int i=nmax; i>=0; i--) {
		int idigit = ((int)pa->pElem[i] >> 4) & 0x0f;
		digit.Format("%X", idigit);
		result += digit.Right(1);
		idigit = pa->pElem[i] & 0x0f;
		digit.Format("%X", idigit);
		result += digit.Right(1);
	}
	if (result.IsEmpty()) return "0"; else return result;
}

void LongIntCopy(CLongInt* pa, CLongInt* pb) {
	LongIntAlloc(pa, pb->nElem);
	LongIntReset(pa);
	for (int i=0; i<pb->nElem; i++) {pa->pElem[i] = pb->pElem[i];}
}

void LongIntAdd(CLongInt* pa, CLongInt* pb) {
	CLongInt result;
	LongIntInit(&result);
	const unsigned int iThisMSB = (LongIntGetMSB(pa) / 8) + 1;
	const unsigned int iArgMSB = (LongIntGetMSB(pb) / 8) + 1;
	const unsigned int nResult = ((iThisMSB > iArgMSB) ? (iThisMSB) : iArgMSB) + 1;
	LongIntAlloc(&result, nResult);
	int icarry = 0;
	for (unsigned int i=0; i<nResult; i++) {
		int idigit = icarry;
		icarry = 0;
		if (i < iThisMSB) idigit += (unsigned int)(pa->pElem[i]);
		if (i < iArgMSB) idigit += (unsigned int)(pb->pElem[i]);
		if (idigit > 0xff) {idigit = idigit & 0xff; icarry = 1;}
		result.pElem[i] = (unsigned char)idigit;
	}
	LongIntCopy(pa,  &result);
}

unsigned char ucResult[LI_NCHAR];

void LongIntMul(CLongInt* pa, CLongInt* pb) {
	unsigned char i, j, k, iElem;
	unsigned int imul, iword; unsigned long isum;
//	struct CLongInt result;
	const unsigned char iThisMSB = (LongIntGetMSB(pa) / 8) + 1;
	const unsigned char iArgMSB = (LongIntGetMSB(pb) / 8) + 1;
	const unsigned char nResult = iThisMSB + iArgMSB;
	for (i=0; i<LI_NCHAR; i++) {ucResult[i] = 0;}
	for (i=0; i<iArgMSB; i++) {
		for (j=0; j<iThisMSB; j++) {
			imul = pb->pElem[i];
			imul *= pa->pElem[j];
//LongIntAddAt(&result, imul, i + j);
			iElem = i + j;
			if (iElem == (nResult - 1)) {
				iword = ucResult[iElem];
				pa->pElem[iElem] = (unsigned char)((iword + imul) & 0xff);
				continue;
			}
			iword = ucResult[iElem+1];
			isum = ucResult[iElem];
			isum += (iword << 8);// + sarg;
			isum += imul;
			ucResult[iElem] = (unsigned char)(isum & 0xff);
			ucResult[iElem+1] = (unsigned char)((isum >> 8) & 0xff);
//if ((i==12)&&(j==19)) break;///////////////////////////////170310//j==19OK 20NG
			//ucResult[iElem+1] = (unsigned char)((int)(isum & 0xff00) >> 8);
//if ((i==12)&&(j==19)) {
//LCDposition(0x40);
//LCDprintHex(imul, 4);
//imul = ucResult[iElem+1]; LCDprintHex(isum, 4);
//while (1) {};
//}
			//if (iword <= ((unsigned int)0xffff - imul)) continue;
			if ((isum & 0x10000) == 0) continue;
			if (iElem == (nResult - 2)) continue;
			for (k=iElem+2; k<nResult; k++) {
				if (ucResult[k] < 0xff) {ucResult[k]++; break;}
				else ucResult[k] = 0;
			}
		}
//if (i==12) break;
	}
	for (i=0; i<LI_NCHAR; i++) {pa->pElem[i] = ucResult[i];}
	pa->nElem = nResult;
}

struct CLongInt result2;
void LongIntSub(CLongInt* pa, CLongInt* pb) {
	int i, idigit, nResult;
	int icarry = 0;
	LongIntInit(&result2);
	nResult = (pa->nElem > pb->nElem) ? (pa->nElem) : pb->nElem;
	LongIntAlloc(&result2, nResult);
	for (i=0; i<nResult; i++) {
		idigit = icarry;
		icarry = 0;
		if (i < pa->nElem) idigit += (unsigned int)(pa->pElem[i]);
		if (i < pb->nElem) idigit -= (unsigned int)(pb->pElem[i]);
		if (idigit < 0) {idigit = idigit + 0x100; icarry = -1;}
		result2.pElem[i] = (unsigned char)idigit;
	}
	LongIntCopy(pa, &result2);
}

struct CLongInt p, q;
void LongIntPowerMod(CLongInt* pa, CLongInt* pow, CLongInt* div) {
unsigned char uc0 = 0;
	LongIntInit(&p);
	LongIntInit(&q);
	LongIntCopy(&p, pa);
	LongIntMod(&p, div);
	LongIntReset(pa);
	pa->pElem[0] = 1;
	LongIntCopy(&q, pow);
	pa->nElem = 1;
//AfxMessageBox("001");
	while (LongIntGetAt(&q, LongIntGetMSB(&q)/8)) {
//AfxMessageBox("002");
		if (LongIntGetAt(&q, 0) & 0x01) {
			LongIntMul(pa, &p); 
//if (uc0 > 0) break;
			LongIntMod(pa, div);
//AfxMessageBox("003");
		}
		LongIntBitShiftRight(&q);
		LongIntMul(&p, &p);
		LongIntMod(&p, div);

//if (uc0 == 3) {
//}
//uc0++; //if (uc0 > 1) break;
	}
/*
CString msg3 = "004\r\n", line3;
for (int i=0; i<pa->nElem; i++) {
	line3.Format("%2x ", pa->pElem[i]);
	msg3 += line3;
}
msg3 += "\r\n";
for (int i=0; i<p.nElem; i++) {
	line3.Format("%2x ", p.pElem[i]);
	msg3 += line3;
}
AfxMessageBox(msg3);
return;//////////////////////////
*/
}

struct CLongInt reg;
void LongIntMod(CLongInt* pa, CLongInt* div) {
	int i;
	const int msbThis = LongIntGetMSB(pa);
	const int msbDiv = LongIntGetMSB(div);
	const int nShift = msbThis - msbDiv;
	LongIntInit(&reg);
////////////////////////
//CString msg; msg.Format("%x %x %x", msbThis, msbDiv, nShift); AfxMessageBox(msg);
//LongIntGreaterOrEqual(pa, div);
//return;/////////////////////////////
	if (nShift <= 0) {
		if (LongIntGreaterOrEqual(pa, div)) {
//return;/////////////////////////////
			LongIntSub(pa, div);
		}
		return;
	}
//return;/////////////////////////////
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

void LongIntBitShiftRight(CLongInt* pa) {
//return;////////////////////////
	for (int i=0; i<pa->nElem; i++) {
		int icarry = 0;
		if (i < pa->nElem -1) icarry = (pa->pElem[i+1] & 0x01) << 7;
		pa->pElem[i] = (unsigned char)(icarry | (pa->pElem[i] >> 1));
	}
}

void LongIntBitShiftLeft(CLongInt* pa, unsigned int ishift) {
	const unsigned int iMSB = LongIntGetMSB(pa);
	CLongInt result;
	LongIntInit(&result);
	LongIntAlloc(&result, (iMSB + ishift) / 8 + 1);
	const int iByteShift = ishift >> 3;
	const int iBitShift = ishift & 0x07;
	for (int i=result.nElem-1; i>=0; i--) {
		const int iSourceByte = i - iByteShift;
		if (iSourceByte < 0) break;
		if (iSourceByte < pa->nElem)
			result.pElem[i] |= (unsigned char)((pa->pElem[iSourceByte] << iBitShift) & 0xff);
		if ((0 <= iSourceByte-1) && (iSourceByte-1 < pa->nElem)) 
			result.pElem[i] |= (unsigned char)((pa->pElem[iSourceByte-1] >> (8-iBitShift)) & 0xff);
	}
	LongIntCopy(pa, &result);
}

unsigned int LongIntGetMSB(CLongInt* pa) {
	unsigned int iresult = 0;
	for (int i=0; i<pa->nElem; i++) {
		if (pa->pElem[i] == 0) continue;
		int idigit = 0x01;
		for (int j=0; j<8; j++) {
			if (pa->pElem[i] & idigit) iresult = (unsigned int)(i * 8 + j);
			idigit = idigit << 1;
		}
	}
	return iresult;
}

bool LongIntLessThan(CLongInt* pa, CLongInt* pb) {
	unsigned char ucRet = 0;
	int iThisMSB, iArgMSB;
	int nResult, i;
	unsigned char ia, it;
	if ((pa->nElem == 0)||(pb->nElem == 0)) return 0;
	iThisMSB = (LongIntGetMSB(pa) / 8) + 1;
	iArgMSB = (LongIntGetMSB(pb) / 8) + 1;
	nResult = (iThisMSB > iArgMSB) ? iThisMSB : iArgMSB;
	for (i=nResult-1; i>=0; i--) {
		ia = 0; it = 0;
		if (i < pa->nElem) it = pa->pElem[i];
		if (i < pb->nElem) ia = pb->pElem[i];
		if (it > ia) break;
		else if (it < ia) {ucRet = 1; break;}
	}

	return ucRet;
}

bool LongIntGreaterOrEqual(CLongInt* pa, CLongInt* pb) {
//if (( pa->pElem[0] == 1)&&(pb->pElem[0] == 0xbd)) {
/*
	CString msg3 = "", line3;
	for (int i=0; i<pa->nElem; i++) {
		line3.Format("%2x ", pa->pElem[i]);
		msg3 += line3;
	}
	msg3 += "\r\n";
	for (int i=0; i<pb->nElem; i++) {
		line3.Format("%2x ", pb->pElem[i]);
		msg3 += line3;
	}
	AfxMessageBox(msg3);
*/
	if (LongIntLessThan(pa, pb)) return 0; else return 1;
}

#endif // _CLONGINT_CPP_
