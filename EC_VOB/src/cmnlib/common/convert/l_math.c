
#ifdef _DEBUG
#include "string.h"
#endif

#include <math.h>
#include <string.h>
#include "l_math.h"
#include "l_stdlib.h"
#include "sys_hwcfg.h"


#define DO1(buf) crc = crc_table[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
#define DO2(buf)  DO1(buf); DO1(buf);
#define DO4(buf)  DO2(buf); DO2(buf);
#define DO8(buf)  DO4(buf); DO4(buf);

/* ========================================================================
 * Table of CRC-32's of all single-byte values (made by make_crc_table)
 */
static const UI32_T crc_table[256] = {
  0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
  0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
  0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
  0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
  0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
  0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
  0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
  0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
  0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
  0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
  0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
  0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
  0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
  0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
  0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
  0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
  0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
  0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
  0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
  0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
  0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
  0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
  0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
  0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
  0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
  0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
  0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
  0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
  0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
  0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
  0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
  0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
  0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
  0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
  0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
  0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
  0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
  0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
  0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
  0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
  0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
  0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
  0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
  0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
  0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
  0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
  0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
  0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
  0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
  0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
  0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
  0x2d02ef8dL
};


/* Lyn Yeh, 12/11/01, 11:25:40 */

UI32_T L_MATH_SelectBalancePort(UI32_T key, UI32_T nbr_of_active_port, UI32_T algorithm)
{
    UI8_T *temp;
    UI32_T index = 0;

    if (nbr_of_active_port == 0)
    {
         return (INVALID_ACTIVE_PORT_NUMBER);
    }
    else
    {
        temp = (UI8_T*)&key;
    }

    switch(algorithm)
    {	
        case 1:  /* should use defined constant in the future */
            index = (*temp + *(temp+1) + *(temp+2) + *(temp+3)) % nbr_of_active_port;
	        break;
		
	    case 2:
	        break;

	    default:	    			
            break;

    };

    return index;
}

UI32_T L_MATH_CheckSum(void *start_addr, UI32_T length)
{
    UI32_T  checksum;
    UI32_T  *u32p;
    UI32_T  r, len;

    len = length >> 2;

    for(u32p = (UI32_T *)start_addr, checksum = 0; len!=0 ; len--,u32p++)
    {
        checksum ^= *u32p;
    }

    /* if the length is not multiple of 4
     */
    if ( (r = length & 3) != 0 )
    {
        UI32_T  w=0;
#if 0 /* this code snippet will be wrong when running on little endian CPU */
        memcpy (&w, u32p, r);
#else
        UI32_T mask;

        switch (r)
        {
            case 1:
                mask=0xFF;
                break;
            case 2:
                mask=0xFFFF;
                break;
            case 3:
                mask=0xFFFFFF;
                break;
            default:
                /* never happen */
                break;
        }
        #if (SYS_HWCFG_LITTLE_ENDIAN_CPU!=TRUE)
        mask = mask << ((4-r)*8);
        #endif
        w = (*u32p) & mask;
#endif

        checksum ^= w;
    };

    return checksum;
} /* End of L_MATH_CheckSum */


/* ========================================================================= 
 * These CRC are copy from U-boot for calculating U-boot data partition crc
 * when we modify U-boot's data 
 */
UI32_T L_MATH_Crc32(UI32_T crc, const I8_T *buf, UI32_T length)
{
    crc = crc ^ 0xffffffffL;
    while (length >= 8)
    {
      DO8(buf);
      length -= 8;
    }
    if (length) do {
      DO1(buf);
    } while (--length);
    return crc ^ 0xffffffffL;
}

UI16_T 
L_MATH_CheckSum16 (UI16_T *ptr, UI32_T nbytes)
{
	register I32_T sum;
	UI16_T oddbyte = 0;
	register UI16_T result;
	UI8_T *p, *q;

	sum = 0;
	while (nbytes > 1)  
	{
 	    sum += *ptr++;
 	    nbytes -= 2;
	}

	if (nbytes == 1) 
	{
		p = (UI8_T *)&oddbyte;
		q = (UI8_T *)ptr;
		*p = *q;
		sum += oddbyte;
	}

	sum  = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	result = ~sum;

	return result;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - L_MATH_UdpCheckSumCalc
 * ---------------------------------------------------------------------
 * PURPOSE  : tell whether it is a digital string
 * INPUT    : UI16_T addr_len is address byte length
 *            UI16_T buff[] is an array containing all the octets in the UDP header and data.
 *            UI16_T len_udp is the length (number of octets) of the UDP header and data.
 *            UI16_T src_addr[4] and UI16_T dest_addr[4] are the IP source and destination address octets
 *  
 * OUTPUT   : none
 * RETURN   : TRUE  -- it is a digital string
 *            FALSE -- it is not a digital string
 * NOTES    : To calculate UDP checksum a "pseudo header" is added to the UDP header. This includes:
 *
 *            IP Source Address  4 bytes
 *            IP Destination Address 4 bytes
 *            Protocol  2 bytes
 *            UDP Length 2 bytes
 *              
 *            The checksum is calculated over all the octets of the pseudo header, UDP header and data. 
 *            If the data contains an odd number of octets a pad, zero octet is added to the end of data. 
 *            The pseudo header and the pad are not transmitted with the packet. 
 *             
 *            In the example code, 

 *            NOTE:
 *            if odd byte, it will use len_udp plus 1 byte to calculate, please make sure it won't overlap memory
 * ---------------------------------------------------------------------
 */
UI16_T L_MATH_UdpCheckSumCalc(UI16_T len_udp, UI16_T buff[], UI16_T addr_len, UI16_T src_addr[], UI16_T dest_addr[])
{
    const UI16_T prot_udp=17;
    register I32_T sum=0;    
    UI16_T check_word, i;

    /* Find out if the length of data is even or odd number. If odd,
       add a padding byte = 0 at the end of packet
     */
    if (len_udp%2){
        check_word = (len_udp+1)/2;
        /*buff[check_word]= buff[check_word]&0xff00; *//*reset padding byte*/
    }
    else
        check_word = len_udp/2;    

    /* make 16 bit words out of every two adjacent 8 bit words and 
       calculate the sum of all 16 vit words
     */
    for (i=0; i<check_word-1; i++)
        sum = sum + buff[i];

    if (len_udp%2)
        sum = sum + (buff[check_word-1]&0xff00);
    else
        sum = sum + buff[check_word-1];

    /* add the UDP pseudo header which contains the IP source and destinationn addresses
    */
    for (i=0;i<(addr_len/2);i++)
        sum = sum + src_addr[i];

    for (i=0;i<(addr_len/2);i++)
        sum = sum + dest_addr[i];

    /* the protocol number and the length of the UDP packet*/
    sum = sum + prot_udp + len_udp;
    
    sum  = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);

    /*Take the one's complement of sum*/
    sum = ~sum;

    return ((UI16_T) sum);
}

/* Adapted for log2 by Ulrich Drepper <drepper@cygnus.com>.  */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/* __ieee754_log2(x)
 * Return the logarithm to base 2 of x
 *
 * Method :
 *   1. Argument Reduction: find k and f such that
 *			x = 2^k * (1+f),
 *	   where  sqrt(2)/2 < 1+f < sqrt(2) .
 *
 *   2. Approximation of log(1+f).
 *	Let s = f/(2+f) ; based on log(1+f) = log(1+s) - log(1-s)
 *		 = 2s + 2/3 s**3 + 2/5 s**5 + .....,
 *	     	 = 2s + s*R
 *      We use a special Reme algorithm on [0,0.1716] to generate
 * 	a polynomial of degree 14 to approximate R The maximum error
 *	of this polynomial approximation is bounded by 2**-58.45. In
 *	other words,
 *		        2      4      6      8      10      12      14
 *	    R(z) ~ Lg1*s +Lg2*s +Lg3*s +Lg4*s +Lg5*s  +Lg6*s  +Lg7*s
 *  	(the values of Lg1 to Lg7 are listed in the program)
 *	and
 *	    |      2          14          |     -58.45
 *	    | Lg1*s +...+Lg7*s    -  R(z) | <= 2
 *	    |                             |
 *	Note that 2s = f - s*f = f - hfsq + s*hfsq, where hfsq = f*f/2.
 *	In order to guarantee error in log below 1ulp, we compute log
 *	by
 *		log(1+f) = f - s*(f - R)	(if f is not too large)
 *		log(1+f) = f - (hfsq - s*(hfsq+R)).	(better accuracy)
 *
 *	3. Finally,  log(x) = k + log(1+f).
 *			    = k+(f-(hfsq-(s*(hfsq+R))))
 *
 * Special cases:
 *	log2(x) is NaN with signal if x < 0 (including -INF) ;
 *	log2(+INF) is +INF; log(0) is -INF with signal;
 *	log2(NaN) is that NaN with no signal.
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */
#if (SYS_HWCFG_LITTLE_ENDIAN_CPU == FALSE)

typedef union
{
  double value;
  struct
  {
    UI32_T msw;
    UI32_T lsw;
  } parts;
} L_MATH_IeeeDoubleShape_T;

#else

typedef union
{
  double value;
  struct
  {
    UI32_T lsw;
    UI32_T msw;
  } parts;
} L_MATH_IeeeDoubleShape_T;

#endif

/* Get two 32 bit ints from a double.  */

#define L_MATH_EXTRACT_WORDS(ix0,ix1,d)				\
do {								\
  L_MATH_IeeeDoubleShape_T ew_u;					\
  ew_u.value = (d);						\
  (ix0) = ew_u.parts.msw;					\
  (ix1) = ew_u.parts.lsw;					\
} while (0)

/* Get the more significant 32 bit int from a double.  */

#define L_MATH_GET_HIGH_WORD(i,d)					\
do {								\
  L_MATH_IeeeDoubleShape_T gh_u;					\
  gh_u.value = (d);						\
  (i) = gh_u.parts.msw;						\
} while (0)

/* Get the less significant 32 bit int from a double.  */

#define L_MATH_GET_LOW_WORD(i,d)					\
do {								\
  L_MATH_IeeeDoubleShape_T gl_u;					\
  gl_u.value = (d);						\
  (i) = gl_u.parts.lsw;						\
} while (0)

/* Set a double from two 32 bit ints.  */

#define L_MATH_INSERT_WORDS(d,ix0,ix1)					\
do {								\
  L_MATH_IeeeDoubleShape_T iw_u;					\
  iw_u.parts.msw = (ix0);					\
  iw_u.parts.lsw = (ix1);					\
  (d) = iw_u.value;						\
} while (0)

/* Set the more significant 32 bits of a double from an int.  */

#define L_MATH_SET_HIGH_WORD(d,v)					\
do {								\
  L_MATH_IeeeDoubleShape_T sh_u;					\
  sh_u.value = (d);						\
  sh_u.parts.msw = (v);						\
  (d) = sh_u.value;						\
} while (0)

/* Set the less significant 32 bits of a double from an int.  */

#define L_MATH_SET_LOW_WORD(d,v)					\
do {								\
  L_MATH_IeeeDoubleShape_T sl_u;					\
  sl_u.value = (d);						\
  sl_u.parts.lsw = (v);						\
  (d) = sl_u.value;						\
} while (0)

/* A union which permits us to convert between a float and a 32 bit
   int.  */

typedef union
{
  float value;
  UI32_T word;
} L_MATH_IeeeFloatShape_T;

/* Get a 32 bit int from a float.  */

#define L_MATH_GET_FLOAT_WORD(i,d)					\
do {								\
  L_MATH_IeeeFloatShape_T gf_u;					\
  gf_u.value = (d);						\
  (i) = gf_u.word;						\
} while (0)

/* Set a float from a 32 bit int.  */

#define L_MATH_SET_FLOAT_WORD(d,i)					\
do {								\
  L_MATH_IeeeFloatShape_T sf_u;					\
  sf_u.word = (i);						\
  (d) = sf_u.value;						\
} while (0)

double L_MATH_log2(double x)
{
	double hfsq,f,s,z,R,w,t1,t2,dk;
	I32_T k,hx,i,j;
	UI32_T lx;

    const double
          ln2 = 0.69314718055994530942,
          two54   =  1.80143985094819840000e+16,  /* 43500000 00000000 */
          Lg1 = 6.666666666666735130e-01,  /* 3FE55555 55555593 */
          Lg2 = 3.999999999940941908e-01,  /* 3FD99999 9997FA04 */
          Lg3 = 2.857142874366239149e-01,  /* 3FD24924 94229359 */
          Lg4 = 2.222219843214978396e-01,  /* 3FCC71C5 1D8E78AF */
          Lg5 = 1.818357216161805012e-01,  /* 3FC74664 96CB03DE */
          Lg6 = 1.531383769920937332e-01,  /* 3FC39A09 D078C69F */
          Lg7 = 1.479819860511658591e-01;  /* 3FC2F112 DF3E5244 */
    const double zero   =  0.0;


	L_MATH_EXTRACT_WORDS(hx,lx,x);

	k=0;
	if (hx < 0x00100000) {			/* x < 2**-1022  */
	    if (((hx&0x7fffffff)|lx)==0)
		return -two54/(x-x);		/* log(+-0)=-inf */
	    if (hx<0) return (x-x)/(x-x);	/* log(-#) = NaN */
	    k -= 54; x *= two54; /* subnormal number, scale up x */
	    L_MATH_GET_HIGH_WORD(hx,x);
	}
	if (hx >= 0x7ff00000) return x+x;
	k += (hx>>20)-1023;
	hx &= 0x000fffff;
	i = (hx+0x95f64)&0x100000;
	L_MATH_SET_HIGH_WORD(x,hx|(i^0x3ff00000));	/* normalize x or x/2 */
	k += (i>>20);
	dk = (double) k;
	f = x-1.0;
	if((0x000fffff&(2+hx))<3) {	/* |f| < 2**-20 */
	    if(f==zero) return dk;
	    R = f*f*(0.5-0.33333333333333333*f);
	    return dk-(R-f)/ln2;
	}
	s = f/(2.0+f);
	z = s*s;
	i = hx-0x6147a;
	w = z*z;
	j = 0x6b851-hx;
	t1= w*(Lg2+w*(Lg4+w*Lg6));
	t2= z*(Lg1+w*(Lg3+w*(Lg5+w*Lg7)));
	i |= j;
	R = t2+t1;
	if(i>0) {
	    hfsq=0.5*f*f;
	    return dk-((hfsq-(s*(hfsq+R)))-f)/ln2;
	} else {
	    return dk-((s*(f-R))-f)/ln2;
	}
}


/* __ieee754_log10(x)
 * Return the base 10 logarithm of x
 *
 * Method :
 *	Let log10_2hi = leading 40 bits of log10(2) and
 *	    log10_2lo = log10(2) - log10_2hi,
 *	    ivln10   = 1/log(10) rounded.
 *	Then
 *		n = ilogb(x),
 *		if(n<0)  n = n+1;
 *		x = scalbn(x,-n);
 *		log10(x) := n*log10_2hi + (n*log10_2lo + ivln10*log(x))
 *
 * Note 1:
 *	To guarantee log10(10**n)=n, where 10**n is normal, the rounding
 *	mode must set to Round-to-Nearest.
 * Note 2:
 *	[1/log(10)] rounded to 53 bits has error  .198   ulps;
 *	log10 is monotonic at all binary break points.
 *
 * Special cases:
 *	log10(x) is NaN with signal if x < 0;
 *	log10(+INF) is +INF with no signal; log10(0) is -INF with signal;
 *	log10(NaN) is that NaN with no signal;
 *	log10(10**N) = N  for N=0,1,...,22.
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following constants.
 * The decimal values may be used, provided that the compiler will convert
 * from decimal to binary accurately enough to produce the hexadecimal values
 * shown.
 */

double L_MATH_log10(double x)
{
	double y,z;
	I32_T i,k,hx;
	UI32_T lx;

    const double
    two54      =  1.80143985094819840000e+16, /* 0x43500000, 0x00000000 */
    ivln10     =  4.34294481903251816668e-01, /* 0x3FDBCB7B, 0x1526E50E */
    log10_2hi  =  3.01029995663611771306e-01, /* 0x3FD34413, 0x509F6000 */
    log10_2lo  =  3.69423907715893078616e-13; /* 0x3D59FEF3, 0x11F12B36 */

    const double zero = 0.0;

	L_MATH_EXTRACT_WORDS(hx,lx,x);

        k=0;
        if (hx < 0x00100000) {                  /* x < 2**-1022  */
            if (((hx&0x7fffffff)|lx)==0)
                return -two54/zero;             /* log(+-0)=-inf */
            if (hx<0) return (x-x)/zero;        /* log(-#) = NaN */
            k -= 54; x *= two54; /* subnormal number, scale up x */
	    L_MATH_GET_HIGH_WORD(hx,x);
        }
	if (hx >= 0x7ff00000) return x+x;
	k += (hx>>20)-1023;
	i  = ((UI32_T)k&0x80000000)>>31;
        hx = (hx&0x000fffff)|((0x3ff-i)<<20);
        y  = (double)(k+i);
	L_MATH_SET_HIGH_WORD(x,hx);
	z  = y*log10_2lo + ivln10*L_MATH_log(x);
	return  z+y*log10_2hi;
}


/* __ieee754_log(x)
 * Return the logrithm of x
 *
 * Method :
 *   1. Argument Reduction: find k and f such that
 *			x = 2^k * (1+f),
 *	   where  sqrt(2)/2 < 1+f < sqrt(2) .
 *
 *   2. Approximation of log(1+f).
 *	Let s = f/(2+f) ; based on log(1+f) = log(1+s) - log(1-s)
 *		 = 2s + 2/3 s**3 + 2/5 s**5 + .....,
 *	     	 = 2s + s*R
 *      We use a special Reme algorithm on [0,0.1716] to generate
 * 	a polynomial of degree 14 to approximate R The maximum error
 *	of this polynomial approximation is bounded by 2**-58.45. In
 *	other words,
 *		        2      4      6      8      10      12      14
 *	    R(z) ~ Lg1*s +Lg2*s +Lg3*s +Lg4*s +Lg5*s  +Lg6*s  +Lg7*s
 *  	(the values of Lg1 to Lg7 are listed in the program)
 *	and
 *	    |      2          14          |     -58.45
 *	    | Lg1*s +...+Lg7*s    -  R(z) | <= 2
 *	    |                             |
 *	Note that 2s = f - s*f = f - hfsq + s*hfsq, where hfsq = f*f/2.
 *	In order to guarantee error in log below 1ulp, we compute log
 *	by
 *		log(1+f) = f - s*(f - R)	(if f is not too large)
 *		log(1+f) = f - (hfsq - s*(hfsq+R)).	(better accuracy)
 *
 *	3. Finally,  log(x) = k*ln2 + log(1+f).
 *			    = k*ln2_hi+(f-(hfsq-(s*(hfsq+R)+k*ln2_lo)))
 *	   Here ln2 is split into two floating point number:
 *			ln2_hi + ln2_lo,
 *	   where n*ln2_hi is always exact for |n| < 2000.
 *
 * Special cases:
 *	log(x) is NaN with signal if x < 0 (including -INF) ;
 *	log(+INF) is +INF; log(0) is -INF with signal;
 *	log(NaN) is that NaN with no signal.
 *
 * Accuracy:
 *	according to an error analysis, the error is always less than
 *	1 ulp (unit in the last place).
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */
double L_MATH_log(double x)
{
	double hfsq,f,s,z,R,w,t1,t2,dk;
	I32_T k,hx,i,j;
	UI32_T lx;

    const double
    ln2_hi  =  6.93147180369123816490e-01,	/* 3fe62e42 fee00000 */
    ln2_lo  =  1.90821492927058770002e-10,	/* 3dea39ef 35793c76 */
    two54   =  1.80143985094819840000e+16,  /* 43500000 00000000 */
    Lg1 = 6.666666666666735130e-01,  /* 3FE55555 55555593 */
    Lg2 = 3.999999999940941908e-01,  /* 3FD99999 9997FA04 */
    Lg3 = 2.857142874366239149e-01,  /* 3FD24924 94229359 */
    Lg4 = 2.222219843214978396e-01,  /* 3FCC71C5 1D8E78AF */
    Lg5 = 1.818357216161805012e-01,  /* 3FC74664 96CB03DE */
    Lg6 = 1.531383769920937332e-01,  /* 3FC39A09 D078C69F */
    Lg7 = 1.479819860511658591e-01;  /* 3FC2F112 DF3E5244 */

    const double zero   =  0.0;


	L_MATH_EXTRACT_WORDS(hx,lx,x);

	k=0;
	if (hx < 0x00100000) {			/* x < 2**-1022  */
	    if (((hx&0x7fffffff)|lx)==0)
		return -two54/zero;		/* log(+-0)=-inf */
	    if (hx<0) return (x-x)/zero;	/* log(-#) = NaN */
	    k -= 54; x *= two54; /* subnormal number, scale up x */
	    L_MATH_GET_HIGH_WORD(hx,x);
	}
	if (hx >= 0x7ff00000) return x+x;
	k += (hx>>20)-1023;
	hx &= 0x000fffff;
	i = (hx+0x95f64)&0x100000;
	L_MATH_SET_HIGH_WORD(x,hx|(i^0x3ff00000));	/* normalize x or x/2 */
	k += (i>>20);
	f = x-1.0;
	if((0x000fffff&(2+hx))<3) {	/* |f| < 2**-20 */
	    if(f==zero) {if(k==0) return zero;  else {dk=(double)k;
				 return dk*ln2_hi+dk*ln2_lo;}
	    }
	    R = f*f*(0.5-0.33333333333333333*f);
	    if(k==0) return f-R; else {dk=(double)k;
	    	     return dk*ln2_hi-((R-dk*ln2_lo)-f);}
	}
 	s = f/(2.0+f);
	dk = (double)k;
	z = s*s;
	i = hx-0x6147a;
	w = z*z;
	j = 0x6b851-hx;
	t1= w*(Lg2+w*(Lg4+w*Lg6));
	t2= z*(Lg1+w*(Lg3+w*(Lg5+w*Lg7)));
	i |= j;
	R = t2+t1;
	if(i>0) {
	    hfsq=0.5*f*f;
	    if(k==0) return f-(hfsq-s*(hfsq+R)); else
		     return dk*ln2_hi-((hfsq-(s*(hfsq+R)+dk*ln2_lo))-f);
	} else {
	    if(k==0) return f-s*(f-R); else
		     return dk*ln2_hi-((s*(f-R)-dk*ln2_lo)-f);
	}
}


/* L_MATH_sqrt(x)
 * Return correctly rounded sqrt.
 *           ------------------------------------------
 *	     |  Use the hardware sqrt if you have one |
 *           ------------------------------------------
 * Method:
 *   Bit by bit method using integer arithmetic. (Slow, but portable)
 *   1. Normalization
 *	Scale x to y in [1,4) with even powers of 2:
 *	find an integer k such that  1 <= (y=x*2^(2k)) < 4, then
 *		sqrt(x) = 2^k * sqrt(y)
 *   2. Bit by bit computation
 *	Let q  = sqrt(y) truncated to i bit after binary point (q = 1),
 *	     i							 0
 *                                     i+1         2
 *	    s  = 2*q , and	y  =  2   * ( y - q  ).		(1)
 *	     i      i            i                 i
 *
 *	To compute q    from q , one checks whether
 *		    i+1       i
 *
 *			      -(i+1) 2
 *			(q + 2      ) <= y.			(2)
 *     			  i
 *							      -(i+1)
 *	If (2) is false, then q   = q ; otherwise q   = q  + 2      .
 *		 	       i+1   i             i+1   i
 *
 *	With some algebric manipulation, it is not difficult to see
 *	that (2) is equivalent to
 *                             -(i+1)
 *			s  +  2       <= y			(3)
 *			 i                i
 *
 *	The advantage of (3) is that s  and y  can be computed by
 *				      i      i
 *	the following recurrence formula:
 *	    if (3) is false
 *
 *	    s     =  s  ,	y    = y   ;			(4)
 *	     i+1      i		 i+1    i
 *
 *	    otherwise,
 *                         -i                     -(i+1)
 *	    s	  =  s  + 2  ,  y    = y  -  s  - 2  		(5)
 *           i+1      i          i+1    i     i
 *
 *	One may easily use induction to prove (4) and (5).
 *	Note. Since the left hand side of (3) contain only i+2 bits,
 *	      it does not necessary to do a full (53-bit) comparison
 *	      in (3).
 *   3. Final rounding
 *	After generating the 53 bits result, we compute one more bit.
 *	Together with the remainder, we can decide whether the
 *	result is exact, bigger than 1/2ulp, or less than 1/2ulp
 *	(it will never equal to 1/2ulp).
 *	The rounding mode can be detected by checking whether
 *	huge + tiny is equal to huge, and whether huge - tiny is
 *	equal to huge for some floating point number "huge" and "tiny".
 *
 * Special cases:
 *	sqrt(+-0) = +-0 	... exact
 *	sqrt(inf) = inf
 *	sqrt(-ve) = NaN		... with invalid signal
 *	sqrt(NaN) = NaN		... with invalid signal for signaling NaN
 *
 * Other methods : see the appended file at the end of the program below.
 *---------------
 */


double L_MATH_sqrt(double x)
{
	double z;
	I32_T sign = (int)0x80000000;
	I32_T ix0,s0,q,m,t,i;
	UI32_T r,t1,s1,ix1,q1;
    const double one = 1.0, tiny = 1.0e-300;

	L_MATH_EXTRACT_WORDS(ix0,ix1,x);

    /* take care of Inf and NaN */
	if((ix0&0x7ff00000)==0x7ff00000) {
	    return x*x+x;		/* sqrt(NaN)=NaN, sqrt(+inf)=+inf
					   sqrt(-inf)=sNaN */
	}
    /* take care of zero */
	if(ix0<=0) {
	    if(((ix0&(~sign))|ix1)==0) return x;/* sqrt(+-0) = +-0 */
	    else if(ix0<0)
		return (x-x)/(x-x);		/* sqrt(-ve) = sNaN */
	}
    /* normalize x */
	m = (ix0>>20);
	if(m==0) {				/* subnormal x */
	    while(ix0==0) {
		m -= 21;
		ix0 |= (ix1>>11); ix1 <<= 21;
	    }
	    for(i=0;(ix0&0x00100000)==0;i++) ix0<<=1;
	    m -= i-1;
	    ix0 |= (ix1>>(32-i));
	    ix1 <<= i;
	}
	m -= 1023;	/* unbias exponent */
	ix0 = (ix0&0x000fffff)|0x00100000;
	if(m&1){	/* odd m, double x to make it even */
	    ix0 += ix0 + ((ix1&sign)>>31);
	    ix1 += ix1;
	}
	m >>= 1;	/* m = [m/2] */

    /* generate sqrt(x) bit by bit */
	ix0 += ix0 + ((ix1&sign)>>31);
	ix1 += ix1;
	q = q1 = s0 = s1 = 0;	/* [q,q1] = sqrt(x) */
	r = 0x00200000;		/* r = moving bit from right to left */

	while(r!=0) {
	    t = s0+r;
	    if(t<=ix0) {
		s0   = t+r;
		ix0 -= t;
		q   += r;
	    }
	    ix0 += ix0 + ((ix1&sign)>>31);
	    ix1 += ix1;
	    r>>=1;
	}

	r = sign;
	while(r!=0) {
	    t1 = s1+r;
	    t  = s0;
	    if((t<ix0)||((t==ix0)&&(t1<=ix1))) {
		s1  = t1+r;
		if(((t1&sign)==sign)&&(s1&sign)==0) s0 += 1;
		ix0 -= t;
		if (ix1 < t1) ix0 -= 1;
		ix1 -= t1;
		q1  += r;
	    }
	    ix0 += ix0 + ((ix1&sign)>>31);
	    ix1 += ix1;
	    r>>=1;
	}

    /* use floating add to find out rounding direction */
	if((ix0|ix1)!=0) {
	    z = one-tiny; /* trigger inexact flag */
	    if (z>=one) {
	        z = one+tiny;
	        if (q1==(UI32_T)0xffffffff) { q1=0; q += 1;}
		else if (z>one) {
		    if (q1==(UI32_T)0xfffffffe) q+=1;
		    q1+=2;
		} else
	            q1 += (q1&1);
	    }
	}
	ix0 = (q>>1)+0x3fe00000;
	ix1 =  q1>>1;
	if ((q&1)==1) ix1 |= sign;
	ix0 += (m <<20);
	L_MATH_INSERT_WORDS(z,ix0,ix1);
	return z;
}
/* __ieee754_pow(x,y) return x**y
 *
 *		      n
 * Method:  Let x =  2   * (1+f)
 *	1. Compute and return log2(x) in two pieces:
 *		log2(x) = w1 + w2,
 *	   where w1 has 53-24 = 29 bit trailing zeros.
 *	2. Perform y*log2(x) = n+y' by simulating muti-precision
 *	   arithmetic, where |y'|<=0.5.
 *	3. Return x**y = 2**n*exp(y'*log2)
 *
 * Special cases:
 *	1.  (anything) ** 0  is 1
 *	2.  (anything) ** 1  is itself
 *	3.  (anything) ** NAN is NAN
 *	4.  NAN ** (anything except 0) is NAN
 *	5.  +-(|x| > 1) **  +INF is +INF
 *	6.  +-(|x| > 1) **  -INF is +0
 *	7.  +-(|x| < 1) **  +INF is +0
 *	8.  +-(|x| < 1) **  -INF is +INF
 *	9.  +-1         ** +-INF is NAN
 *	10. +0 ** (+anything except 0, NAN)               is +0
 *	11. -0 ** (+anything except 0, NAN, odd integer)  is +0
 *	12. +0 ** (-anything except 0, NAN)               is +INF
 *	13. -0 ** (-anything except 0, NAN, odd integer)  is +INF
 *	14. -0 ** (odd integer) = -( +0 ** (odd integer) )
 *	15. +INF ** (+anything except 0,NAN) is +INF
 *	16. +INF ** (-anything except 0,NAN) is +0
 *	17. -INF ** (anything)  = -0 ** (-anything)
 *	18. (-anything) ** (integer) is (-1)**(integer)*(+anything**integer)
 *	19. (-anything except 0 and inf) ** (non-integer) is NAN
 *
 * Accuracy:
 *	pow(x,y) returns x**y nearly rounded. In particular
 *			pow(integer,integer)
 *	always returns the correct integer provided it is
 *	representable.
 *
 * Constants :
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */

double L_MATH_pow(double x, double y)
{
	double z,ax,z_h,z_l,p_h,p_l;
	double y1,t1,t2,r,s,t,u,v,w;
	I32_T i,j,k,yisint,n;
	I32_T hx,hy,ix,iy;
	UI32_T lx,ly;

    const double
    bp[] = {1.0, 1.5,},
    dp_h[] = { 0.0, 5.84962487220764160156e-01,}, /* 0x3FE2B803, 0x40000000 */
    dp_l[] = { 0.0, 1.35003920212974897128e-08,}, /* 0x3E4CFDEB, 0x43CFD006 */
    zero    =  0.0,
    one	=  1.0,
    two	=  2.0,
    two53	=  9007199254740992.0,	/* 0x43400000, 0x00000000 */
    huge	=  1.0e300,
    tiny    =  1.0e-300,
    	/* poly coefs for (3/2)*(log(x)-2s-2/3*s**3 */
    L1  =  5.99999999999994648725e-01, /* 0x3FE33333, 0x33333303 */
    L2  =  4.28571428578550184252e-01, /* 0x3FDB6DB6, 0xDB6FABFF */
    L3  =  3.33333329818377432918e-01, /* 0x3FD55555, 0x518F264D */
    L4  =  2.72728123808534006489e-01, /* 0x3FD17460, 0xA91D4101 */
    L5  =  2.30660745775561754067e-01, /* 0x3FCD864A, 0x93C9DB65 */
    L6  =  2.06975017800338417784e-01, /* 0x3FCA7E28, 0x4A454EEF */
    P1   =  1.66666666666666019037e-01, /* 0x3FC55555, 0x5555553E */
    P2   = -2.77777777770155933842e-03, /* 0xBF66C16C, 0x16BEBD93 */
    P3   =  6.61375632143793436117e-05, /* 0x3F11566A, 0xAF25DE2C */
    P4   = -1.65339022054652515390e-06, /* 0xBEBBBD41, 0xC5D26BF1 */
    P5   =  4.13813679705723846039e-08, /* 0x3E663769, 0x72BEA4D0 */
    lg2  =  6.93147180559945286227e-01, /* 0x3FE62E42, 0xFEFA39EF */
    lg2_h  =  6.93147182464599609375e-01, /* 0x3FE62E43, 0x00000000 */
    lg2_l  = -1.90465429995776804525e-09, /* 0xBE205C61, 0x0CA86C39 */
    ovt =  8.0085662595372944372e-0017, /* -(1024-log2(ovfl+.5ulp)) */
    cp    =  9.61796693925975554329e-01, /* 0x3FEEC709, 0xDC3A03FD =2/(3ln2) */
    cp_h  =  9.61796700954437255859e-01, /* 0x3FEEC709, 0xE0000000 =(float)cp */
    cp_l  = -7.02846165095275826516e-09, /* 0xBE3E2FE0, 0x145B01F5 =tail of cp_h*/
    ivln2    =  1.44269504088896338700e+00, /* 0x3FF71547, 0x652B82FE =1/ln2 */
    ivln2_h  =  1.44269502162933349609e+00, /* 0x3FF71547, 0x60000000 =24b 1/ln2*/
    ivln2_l  =  1.92596299112661746887e-08; /* 0x3E54AE0B, 0xF85DDF44 =1/ln2 tail*/

	L_MATH_EXTRACT_WORDS(hx,lx,x);
	L_MATH_EXTRACT_WORDS(hy,ly,y);
	ix = hx&0x7fffffff;  iy = hy&0x7fffffff;

    /* y==zero: x**0 = 1 */
	if((iy|ly)==0) return one;

    /* +-NaN return x+y */
	if(ix > 0x7ff00000 || ((ix==0x7ff00000)&&(lx!=0)) ||
	   iy > 0x7ff00000 || ((iy==0x7ff00000)&&(ly!=0)))
		return x+y;

    /* determine if y is an odd int when x < 0
     * yisint = 0	... y is not an integer
     * yisint = 1	... y is an odd int
     * yisint = 2	... y is an even int
     */
	yisint  = 0;
	if(hx<0) {
	    if(iy>=0x43400000) yisint = 2; /* even integer y */
	    else if(iy>=0x3ff00000) {
		k = (iy>>20)-0x3ff;	   /* exponent */
		if(k>20) {
		    j = ly>>(52-k);
		    if((j<<(52-k))==ly) yisint = 2-(j&1);
		} else if(ly==0) {
		    j = iy>>(20-k);
		    if((j<<(20-k))==iy) yisint = 2-(j&1);
		}
	    }
	}

    /* special value of y */
	if(ly==0) {
	    if (iy==0x7ff00000) {	/* y is +-inf */
	        if(((ix-0x3ff00000)|lx)==0)
		    return  y - y;	/* inf**+-1 is NaN */
	        else if (ix >= 0x3ff00000)/* (|x|>1)**+-inf = inf,0 */
		    return (hy>=0)? y: zero;
	        else			/* (|x|<1)**-,+inf = inf,0 */
		    return (hy<0)?-y: zero;
	    }
	    if(iy==0x3ff00000) {	/* y is  +-1 */
		if(hy<0) return one/x; else return x;
	    }
	    if(hy==0x40000000) return x*x; /* y is  2 */
	    if(hy==0x3fe00000) {	/* y is  0.5 */
		if(hx>=0)	/* x >= +0 */
		return L_MATH_sqrt(x);
	    }
	}

	ax   = fabs(x);
    /* special value of x */
	if(lx==0) {
	    if(ix==0x7ff00000||ix==0||ix==0x3ff00000){
		z = ax;			/*x is +-0,+-inf,+-1*/
		if(hy<0) z = one/z;	/* z = (1/|x|) */
		if(hx<0) {
		    if(((ix-0x3ff00000)|yisint)==0) {
			z = (z-z)/(z-z); /* (-1)**non-int is NaN */
		    } else if(yisint==1)
			z = -z;		/* (x<0)**odd = -(|x|**odd) */
		}
		return z;
	    }
	}

    /* (x<0)**(non-int) is NaN */
	if(((((UI32_T)hx>>31)-1)|yisint)==0) return (x-x)/(x-x);

    /* |y| is huge */
	if(iy>0x41e00000) { /* if |y| > 2**31 */
	    if(iy>0x43f00000){	/* if |y| > 2**64, must o/uflow */
		if(ix<=0x3fefffff) return (hy<0)? huge*huge:tiny*tiny;
		if(ix>=0x3ff00000) return (hy>0)? huge*huge:tiny*tiny;
	    }
	/* over/underflow if x is not close to one */
	    if(ix<0x3fefffff) return (hy<0)? huge*huge:tiny*tiny;
	    if(ix>0x3ff00000) return (hy>0)? huge*huge:tiny*tiny;
	/* now |1-x| is tiny <= 2**-20, suffice to compute
	   log(x) by x-x^2/2+x^3/3-x^4/4 */
	    t = x-1;		/* t has 20 trailing zeros */
	    w = (t*t)*(0.5-t*(0.3333333333333333333333-t*0.25));
	    u = ivln2_h*t;	/* ivln2_h has 21 sig. bits */
	    v = t*ivln2_l-w*ivln2;
	    t1 = u+v;
	    L_MATH_SET_LOW_WORD(t1,0);
	    t2 = v-(t1-u);
	} else {
	    double s2,s_h,s_l,t_h,t_l;
	    n = 0;
	/* take care subnormal number */
	    if(ix<0x00100000)
		{ax *= two53; n -= 53; L_MATH_GET_HIGH_WORD(ix,ax); }
	    n  += ((ix)>>20)-0x3ff;
	    j  = ix&0x000fffff;
	/* determine interval */
	    ix = j|0x3ff00000;		/* normalize ix */
	    if(j<=0x3988E) k=0;		/* |x|<sqrt(3/2) */
	    else if(j<0xBB67A) k=1;	/* |x|<sqrt(3)   */
	    else {k=0;n+=1;ix -= 0x00100000;}
	    L_MATH_SET_HIGH_WORD(ax,ix);

	/* compute s = s_h+s_l = (x-1)/(x+1) or (x-1.5)/(x+1.5) */
	    u = ax-bp[k];		/* bp[0]=1.0, bp[1]=1.5 */
	    v = one/(ax+bp[k]);
	    s = u*v;
	    s_h = s;
	    L_MATH_SET_LOW_WORD(s_h,0);
	/* t_h=ax+bp[k] High */
	    t_h = zero;
	    L_MATH_SET_HIGH_WORD(t_h,((ix>>1)|0x20000000)+0x00080000+(k<<18));
	    t_l = ax - (t_h-bp[k]);
	    s_l = v*((u-s_h*t_h)-s_h*t_l);
	/* compute log(ax) */
	    s2 = s*s;
	    r = s2*s2*(L1+s2*(L2+s2*(L3+s2*(L4+s2*(L5+s2*L6)))));
	    r += s_l*(s_h+s);
	    s2  = s_h*s_h;
	    t_h = 3.0+s2+r;
	    L_MATH_SET_LOW_WORD(t_h,0);
	    t_l = r-((t_h-3.0)-s2);
	/* u+v = s*(1+...) */
	    u = s_h*t_h;
	    v = s_l*t_h+t_l*s;
	/* 2/(3log2)*(s+...) */
	    p_h = u+v;
	    L_MATH_SET_LOW_WORD(p_h,0);
	    p_l = v-(p_h-u);
	    z_h = cp_h*p_h;		/* cp_h+cp_l = 2/(3*log2) */
	    z_l = cp_l*p_h+p_l*cp+dp_l[k];
	/* log2(ax) = (s+..)*2/(3*log2) = n + dp_h + z_h + z_l */
	    t = (double)n;
	    t1 = (((z_h+z_l)+dp_h[k])+t);
	    L_MATH_SET_LOW_WORD(t1,0);
	    t2 = z_l-(((t1-t)-dp_h[k])-z_h);
	}

	s = one; /* s (sign of result -ve**odd) = -1 else = 1 */
	if(((((UI32_T)hx>>31)-1)|(yisint-1))==0)
	    s = -one;/* (-ve)**(odd int) */

    /* split up y into y1+y2 and compute (y1+y2)*(t1+t2) */
	y1  = y;
	L_MATH_SET_LOW_WORD(y1,0);
	p_l = (y-y1)*t1+y*t2;
	p_h = y1*t1;
	z = p_l+p_h;
	L_MATH_EXTRACT_WORDS(j,i,z);
	if (j>=0x40900000) {				/* z >= 1024 */
	    if(((j-0x40900000)|i)!=0)			/* if z > 1024 */
		return s*huge*huge;			/* overflow */
	    else {
		if(p_l+ovt>z-p_h) return s*huge*huge;	/* overflow */
	    }
	} else if((j&0x7fffffff)>=0x4090cc00 ) {	/* z <= -1075 */
	    if(((j-0xc090cc00)|i)!=0) 		/* z < -1075 */
		return s*tiny*tiny;		/* underflow */
	    else {
		if(p_l<=z-p_h) return s*tiny*tiny;	/* underflow */
	    }
	}
    /*
     * compute 2**(p_h+p_l)
     */
	i = j&0x7fffffff;
	k = (i>>20)-0x3ff;
	n = 0;
	if(i>0x3fe00000) {		/* if |z| > 0.5, set n = [z+0.5] */
	    n = j+(0x00100000>>(k+1));
	    k = ((n&0x7fffffff)>>20)-0x3ff;	/* new k for n */
	    t = zero;
	    L_MATH_SET_HIGH_WORD(t,n&~(0x000fffff>>k));
	    n = ((n&0x000fffff)|0x00100000)>>(20-k);
	    if(j<0) n = -n;
	    p_h -= t;
	}
	t = p_l+p_h;
	L_MATH_SET_LOW_WORD(t,0);
	u = t*lg2_h;
	v = (p_l-(t-p_h))*lg2+t*lg2_l;
	z = u+v;
	w = v-(z-u);
	t  = z*z;
	t1  = z - t*(P1+t*(P2+t*(P3+t*(P4+t*P5))));
	r  = (z*t1)/(t1-two)-(w+z*w);
	z  = one-(r-z);
	L_MATH_GET_HIGH_WORD(j,z);
	j += (n<<20);
	if((j>>20)<=0) z = scalbn(z,n);	/* subnormal output */
	else L_MATH_SET_HIGH_WORD(z,j);
	return s*z;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - L_MATH_MacCalculation
 * -------------------------------------------------------------------------
 * FUNCTION: This function evaluates the given mac address and the given
 *           offset value to come out a resulting mac address.
 * INPUT   : array_index - The index to the argument mac that the offset value
 *                         will be applied on. array_index 5 means mac[5]. This
 *                         value shall not be larger than 5.
 *           freeze_array_index
 *                       - The array index that is less then this index 
 *                         in mac shall not be changed. This function
 *                         returns FALSE when the elements in mac larger than
 *                         frezze_array_index is changed.
 *                         Specify 0 if all elements in mac are allowed to be
 *                         changed.
 *           offset      - The value to be applied to the given array_index
 *                         of mac. The value can be postive or negative.
 *                         The legal range of offset is
 *                         [L_MATH_MAC_CALCULATION_OFFSET_MIN,L_MATH_MAC_CALCULATION_OFFSET_MAX]
 *           mac_p       - base mac address, must point to a UI8_T array with
 *                         6 elements.
 * OUTPUT  : mac_p       - The resulting mac address.
 * RETURN  : TRUE        - The mac calculation perfoms without error.
 *           FALSE       - An error occurs during mac calculation.
 * NOTE    : 1. In usual, the first 3 bytes in mac belongs to OUI and should
 *              not be changed. Specify freeze_array_index as 3 to ensure that
 *              OUI is not changed.
 * -------------------------------------------------------------------------*/
BOOL_T L_MATH_MacCalculation(UI8_T array_index, UI8_T freeze_array_index, I32_T offset, UI8_T *mac_p)
{
    I32_T result_one_mac_addr;

    /* sanity check for input arguments
     */
    if (offset > L_MATH_MAC_CALCULATION_OFFSET_MAX || offset < L_MATH_MAC_CALCULATION_OFFSET_MIN)
        return FALSE;

    if ((array_index > 5) || (freeze_array_index > 5) || (mac_p == NULL))
        return FALSE;

    /* Not allowed to modify the freeze part of mac
     */
    if (array_index < freeze_array_index)
        return FALSE;

    result_one_mac_addr = mac_p[array_index]+offset;

    if (result_one_mac_addr > 0xFF) /* overflow, carry one to higher mac addr index */
    {
        mac_p[array_index] = (UI8_T)(result_one_mac_addr - 0x100);
        return L_MATH_MacCalculation(--array_index, freeze_array_index, 1, mac_p);
    }
    else if (result_one_mac_addr >= 0) /* result_one_mac_addr not overflow */
    {
        mac_p[array_index] = result_one_mac_addr;
    }
    else /* underflow, carry minus one to higher mac addr index */
    {
        mac_p[array_index] = (UI8_T)(result_one_mac_addr + 0x100);
        return L_MATH_MacCalculation(--array_index, freeze_array_index, -1, mac_p);
    }

    return TRUE;
} /* end of L_MATH_MacCalculation()*/

