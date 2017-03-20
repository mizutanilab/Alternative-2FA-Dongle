/*
Two-factor authentification dongle
Mizutani Lab (c) 2017
Modified from RFC3174
*/

/*
 *  sha1.c
 *
 *  Description:
 *      This file implements the Secure Hashing Algorithm 1 as
 *      defined in FIPS PUB 180-1 published April 17, 1995.
 *
 *      The SHA-1, produces a 160-bit message digest for a given
 *      data stream.  It should take about 2**n steps to find a
 *      message with the same digest as a given message and
 *      2**(n/2) to find any two messages with the same digest,
 *      when n is the digest size in bits.  Therefore, this
 *      algorithm can serve as a means of providing a
 *      "fingerprint" for a message.
 *
 *  Portability Issues:
 *      SHA-1 is defined in terms of 32-bit "words".  This code
 *      uses  (included via "sha1.h" to define 32 and 8
 *      bit unsigned integer types.  If your C compiler does not
 *      support 32 bit unsigned integers, this code is not
 *      appropriate.
 *
 *  Caveats:
 *      SHA-1 is designed to work with messages less than 2^64 bits
 *      long.  Although SHA-1 allows a message digest to be generated
 *      for messages of any number of bits less than 2^64, this
 *      implementation only works with messages with a length that is
 *      a multiple of the size of an 8-bit character.
 *
 */

#include "hmac-sha1.h"

/*
 *  Define the SHA1 circular left shift macro
 */
#define SHA1CircularShift(bits,word) \
                (((word) << (bits)) | ((word) >> (32-(bits))))

/* Local Function Prototyptes */
void SHA1PadMessage();
void SHA1ProcessMessageBlock();

SHA1Context sha;

/*
 *  SHA1Reset
 *
 *  Description:
 *      This function will initialize the SHA1Context in preparation
 *      for computing a new SHA1 message digest.
 *
 *  Parameters:
 *      context: [in/out]
 *          The context to reset.
 *
 *  Returns:
 *      sha Error Code.
 *
 */
char SHA1Reset()
{
    sha.Length_Low             = 0;
    sha.Length_High            = 0;
    sha.Message_Block_Index    = 0;

    sha.Intermediate_Hash[0]   = 0x67452301;
    sha.Intermediate_Hash[1]   = 0xEFCDAB89;
    sha.Intermediate_Hash[2]   = 0x98BADCFE;
    sha.Intermediate_Hash[3]   = 0x10325476;
    sha.Intermediate_Hash[4]   = 0xC3D2E1F0;

    sha.Computed   = 0;
    sha.Corrupted  = 0;

    return shaSuccess;
}

/*
 *  SHA1Result
 *
 *  Description:
 *      This function will return the 160-bit message digest into the
 *      Message_Digest array  provided by the caller.
 *      NOTE: The first octet of hash is stored in the 0th element,
 *            the last octet of hash in the 19th element.
 *
 *  Parameters:
 *      context: [in/out]
 *          The context to use to calculate the SHA-1 hash.
 *      Message_Digest: [out]
 *          Where the digest is returned.
 *
 *  Returns:
 *      sha Error Code.
 *
 */

uint8_t Message_Digest[SHA1HashSize];

char SHA1Result()
{
    int i;

    if (sha.Corrupted)
    {
        return sha.Corrupted;
    }


    if (!sha.Computed)
    {
        SHA1PadMessage();
        for(i=0; i<64; ++i)
        {
            /* message may be sensitive, clear it out */
            sha.Message_Block[i] = 0;
        }
        sha.Length_Low = 0;    /* and clear length */
        sha.Length_High = 0;
        sha.Computed = 1;
    }

    for(i = 0; i < SHA1HashSize; ++i)
    {
        Message_Digest[i] = sha.Intermediate_Hash[i>>2]
                            >> 8 * ( 3 - ( i & 0x03 ) );
    }

    return shaSuccess;
}

/*
 *  SHA1Input
 *
 *  Description:
 *      This function accepts an array of octets as the next portion
 *      of the message.
 *
 *  Parameters:
 *      context: [in/out]
 *          The SHA context to update
 *      message_array: [in]
 *          An array of characters representing the next portion of
 *          the message.
 *      length: [in]
 *          The length of the message in message_array
 *
 *  Returns:
 *      sha Error Code.
 *
 */

#define HMACSHA1_BLKSIZE 64
unsigned char ucInput[HMACSHA1_BLKSIZE];

char SHA1Input(unsigned char length)
{
	int ilength = length;
	unsigned char ucIdx;

    if (!length)
    {
        return shaSuccess;
    }

    if (sha.Computed)
    {
        sha.Corrupted = shaStateError;
        return shaStateError;
    }

    if (sha.Corrupted)
    {
         return sha.Corrupted;
    }

	ucIdx = 0;
    while(length-- && !sha.Corrupted)
    {
	    sha.Message_Block[sha.Message_Block_Index++] =
	                    (ucInput[ucIdx] & 0xFF);
	
	    sha.Length_Low += 8;
	    if (sha.Length_Low == 0)
	    {
	        sha.Length_High++;
	        if (sha.Length_High == 0)
	        {
	            /* Message is too long */
	            sha.Corrupted = 1;
	        }
	    }
	
	    if (sha.Message_Block_Index == 64)
	    {
	        SHA1ProcessMessageBlock();
	    }
	
	    ucIdx++;
    }

    return shaSuccess;
}

/*
 *  SHA1ProcessMessageBlock
 *
 *  Description:
 *      This function will process the next 512 bits of the message
 *      stored in the Message_Block array.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *      Many of the variable names in this code, especially the
 *      single character names, were used because those were the
 *      names used in the publication.
 *
 *
 */

#pragma udata section_W0
uint32_t      W0[40];             /* Word sequence               */
#pragma udata section_W1
uint32_t      W1[40];             /* Word sequence               */

#pragma udata
void SHA1ProcessMessageBlock()
{
//	uint32_t      W[80];             /* Word sequence               */
    const uint32_t K[] =    {       /* Constants defined in SHA-1   */
                            0x5A827999,
                            0x6ED9EBA1,
                            0x8F1BBCDC,
                            0xCA62C1D6
                            };
    int           t;                 /* Loop counter                */
    uint32_t      temp;              /* Temporary word value        */
    uint32_t      A, B, C, D, E;     /* Word buffers                */
	uint32_t	Wt3, Wt8, Wt14, Wt16;

    /*
     *  Initialize the first 16 words in the array W
     */

    for(t = 0; t < 16; t++)
    {
		Wt3 = sha.Message_Block[t * 4];
        W0[t] = Wt3 << 24;
		Wt3 = sha.Message_Block[t * 4 + 1];
        W0[t] |= Wt3 << 16;
		Wt3 = sha.Message_Block[t * 4 + 2];
        W0[t] |= Wt3 << 8;
        W0[t] |= sha.Message_Block[t * 4 + 3];
    }

    for(t = 16; t < 80; t++)
    {
		//W[t] = SHA1CircularShift(1, W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);
		Wt3 = (t-3 < 40) ? W0[t-3] : W1[t-3-40];
		Wt8 = (t-8 < 40) ? W0[t-8] : W1[t-8-40];
		Wt14 = (t-14 < 40) ? W0[t-14] : W1[t-14-40];
		Wt16 = (t-16 < 40) ? W0[t-16] : W1[t-16-40];
		if (t < 40) W0[t] = SHA1CircularShift(1, Wt3 ^ Wt8 ^ Wt14 ^ Wt16);
		else W1[t-40] = SHA1CircularShift(1, Wt3 ^ Wt8 ^ Wt14 ^ Wt16);
    }

    A = sha.Intermediate_Hash[0];
    B = sha.Intermediate_Hash[1];
    C = sha.Intermediate_Hash[2];
    D = sha.Intermediate_Hash[3];
    E = sha.Intermediate_Hash[4];

    for(t = 0; t < 20; t++)
    {
        temp =  SHA1CircularShift(5,A) +
                ((B & C) | ((~B) & D)) + E + W0[t] + K[0];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    for(t = 20; t < 40; t++)
    {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W0[t] + K[1];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    for(t = 40; t < 60; t++)
    {
        temp = SHA1CircularShift(5,A) +
               ((B & C) | (B & D) | (C & D)) + E + W1[t-40] + K[2];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    for(t = 60; t < 80; t++)
    {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W1[t-40] + K[3];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    sha.Intermediate_Hash[0] += A;
    sha.Intermediate_Hash[1] += B;
    sha.Intermediate_Hash[2] += C;
    sha.Intermediate_Hash[3] += D;
    sha.Intermediate_Hash[4] += E;

    sha.Message_Block_Index = 0;
}


/*
 *  SHA1PadMessage
 *
 *  Description:
 *      According to the standard, the message must be padded to an even
 *      512 bits.  The first padding bit must be a '1'.  The last 64
 *      bits represent the length of the original message.  All bits in
 *      between should be 0.  This function will pad the message
 *      according to those rules by filling the Message_Block array
 *      accordingly.  It will also call the ProcessMessageBlock function
 *      provided appropriately.  When it returns, it can be assumed that
 *      the message digest has been computed.
 *
 *  Parameters:
 *      context: [in/out]
 *          The context to pad
 *      ProcessMessageBlock: [in]
 *          The appropriate SHA*ProcessMessageBlock function
 *  Returns:
 *      Nothing.
 *
 */

void SHA1PadMessage()
{
    /*
     *  Check to see if the current message block is too small to hold
     *  the initial padding bits and length.  If so, we will pad the
     *  block, process it, and then continue padding into a second
     *  block.
     */
    if (sha.Message_Block_Index > 55)
    {
        sha.Message_Block[sha.Message_Block_Index++] = 0x80;
        while(sha.Message_Block_Index < 64)
        {
            sha.Message_Block[sha.Message_Block_Index++] = 0;
        }

        SHA1ProcessMessageBlock();

        while(sha.Message_Block_Index < 56)
        {
            sha.Message_Block[sha.Message_Block_Index++] = 0;
        }
    }
    else
    {
        sha.Message_Block[sha.Message_Block_Index++] = 0x80;
        while(sha.Message_Block_Index < 56)
        {
            sha.Message_Block[sha.Message_Block_Index++] = 0;
        }
    }

    /*
     *  Store the message length as the last 8 octets
     */
    sha.Message_Block[56] = sha.Length_High >> 24;
    sha.Message_Block[57] = sha.Length_High >> 16;
    sha.Message_Block[58] = sha.Length_High >> 8;
    sha.Message_Block[59] = sha.Length_High;
    sha.Message_Block[60] = sha.Length_Low >> 24;
    sha.Message_Block[61] = sha.Length_Low >> 16;
    sha.Message_Block[62] = sha.Length_Low >> 8;
    sha.Message_Block[63] = sha.Length_Low;

    SHA1ProcessMessageBlock();
}

char hmac_sha1(unsigned long* pulCode, unsigned char* pcKey, unsigned char ncKey, unsigned char* pcMsg, unsigned char ncMsg) {
//	SHA1Context sha;
	int i, offset;
	char cErr;

	if (ncKey > HMACSHA1_BLKSIZE) return 110;

	for (i=0; i<ncKey; i++) {
		ucInput[i] = pcKey[i] ^ 0x36;
	}
	for (i=ncKey; i<HMACSHA1_BLKSIZE; i++) {
		ucInput[i] = 0x36;
	}

	cErr = SHA1Reset();
	if (cErr) return 101;

	cErr = SHA1Input(HMACSHA1_BLKSIZE);
	if (cErr) return 102;

	for (i=0; i<ncMsg; i++) {ucInput[i] = pcMsg[i];}
	cErr = SHA1Input(ncMsg);
	if (cErr) return 102;

	cErr = SHA1Result();
	if (cErr) return 103;

	for (i=0; i<ncKey; i++) {
		ucInput[i] = (pcKey[i] ^ 0x5c);
	}
	for (i=ncKey; i<HMACSHA1_BLKSIZE; i++) {
		ucInput[i] = 0x5c;
	}

	cErr = SHA1Reset();
	if (cErr) return 101;

	cErr = SHA1Input(HMACSHA1_BLKSIZE);
	if (cErr) return 102;
	for (i=0; i<SHA1HashSize; i++) {ucInput[i] = Message_Digest[i];}
	cErr = SHA1Input(SHA1HashSize);
	if (cErr) return 102;

	cErr = SHA1Result();
	if (cErr) return 103;

	offset = Message_Digest[19] & 0x0f;
	*pulCode =
		((unsigned long)Message_Digest[offset] & 0x7f) << 24
		| ((unsigned long)Message_Digest[offset+1] & 0xff) << 16
		| ((unsigned long)Message_Digest[offset+2] & 0xff) << 8
		| ((unsigned long)Message_Digest[offset+3] & 0xff);

	//LCDposition(0x40);
	//LCDprintDec(*pulCode, 6);
	return 0;
}

