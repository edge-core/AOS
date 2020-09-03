/* Module Name: dns_list.h
 * Purpose: This package privide some common api for DNS.
 * Notes:
 * History:
 *    09/06/02        -- simon zhou, Create
 * Copyright(C)      Accton Corporation, 1999, 2000
 */

#ifndef DNS_LIST_H
#define DNS_LIST_H

//#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif	/*	__cpluscplus	*/

/* Number of elements to over-allocate by default */
#define DNS_DA_PREALLOC	5

/*
 * This will work for i386 and alpha.
 */
#define DNS_DA_ALIGNSZ(sz) (((((sz)-1)/8)+1)*8)

#define DNS_DA_CREATE(tp) (DNS_DaCreate(sizeof(tp)))
#define DNS_DA_INDEX(a,i,tp) ((tp *)(DNS_DaIndex(a,i)))
/* Used often, so make special-case macro here */
#define DNS_DA_LAST(a, tp) ((tp *)(DNS_DaIndex(a, (a)->nel-1)))

/*
 * The size of this should always be a multiple of 4 on all supported architectures.
 * Otherwise, we need further glue.
 */
struct DNS_DarrayHead_S {
	int tpsz;	/* size of the type we hold (including padding) */
	int nel;	/* number of elements in array */
	int ael;	/* number of allocated elements */
	int dummy;	/* dummy for alignment */
};

typedef struct DNS_DarrayHead_S *darray;

darray 	DNS_DaCreate(int sz);
darray 	DNS_DaGrow(darray a, int n);
darray 	DNS_DaResize(darray a, int n);
I8_T 	*DNS_DaIndex(darray a, int i);
int 	DNS_DaNumberOfElement(darray a);

#ifdef	__cpluscplus
}
#endif	/* __cpluscplus */

#endif	/* #ifndef DNS_LIST_H */
