/* Module Name: dns_list.c
 * Purpose: This package privide some common api for DNS.
 * Notes:
 * History:
 *    09/06/02        -- simon zhou, Create
 * Copyright(C)      Accton Corporation, 1999, 2000
 */

/* isiah.2004-01-06. remove all compile warring message.*/
#include "sys_type.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_DNS == TRUE)

#include <stdlib.h>

#include "dns.h"
#include "dns_list.h"

/*
 * FUNCTION NAME :DNS_DaCreate
 *
 * PURPOSE:
 *		create darray a
 *
 * INPUT:
 *		int -- number of element
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		darray a --
 *
 * NOTES:
 *		none
 *
 */
darray DNS_DaCreate(int sz)
{
	darray a;
	int tpsz = DNS_DA_ALIGNSZ(sz); /* Round up sizes for aligned access */

	a = (darray)malloc(sizeof(struct DNS_DarrayHead_S) + DNS_DA_PREALLOC*tpsz);
	if (NULL == a)
		return NULL;

	a->tpsz= tpsz;
	a->nel = 0;
	a->ael = DNS_DA_PREALLOC;
	return a;
}

/*
 * FUNCTION NAME :DNS_DaGrow
 *
 * PURPOSE:
 *		add n element to a
 *
 * INPUT:
 *		darray --
 *		int --
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		a
 *
 * NOTES:
 *		none
 *
 */
darray DNS_DaGrow(darray a, int n)
{
	return DNS_DaResize(a, a->nel+n);
}

/*
 * FUNCTION NAME :DNS_DaResize
 *
 * PURPOSE:
 *		re alloc a
 *
 * INPUT:
 *		darray --
 *		int --
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		darray a--
 *
 * NOTES:
 *		none
 *
 */
darray DNS_DaResize(darray a, int n)
{
	darray tmp;

	a->nel=n;
	if (((a->nel)>(a->ael)) || (a->nel)<((a->ael)-DNS_DA_PREALLOC*2))
	{
		/* adjust alloced space. */
		a->ael += DNS_DA_PREALLOC;
		tmp = (darray)realloc(a, sizeof(struct DNS_DarrayHead_S)+((a->tpsz)*(a->ael)));
		if (NULL == tmp)
		{
			free(a);
			a = NULL;
		}
		return tmp;
	} else
		return a;
}

/*
 * FUNCTION NAME :DNS_DaIndex
 *
 * PURPOSE:
 *		get i.th element of a
 *
 * INPUT:
 *		darray --
 *		int -- index
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		I8_T* --
 *
 * NOTES:
 *		none
 *
 */
I8_T *DNS_DaIndex(darray a, int i)
{
	return ((I8_T *)a)+sizeof(struct DNS_DarrayHead_S)+(i*(a->tpsz));
}

/*
 * FUNCTION NAME :DNS_DaNumberOfElement
 *
 * PURPOSE:
 *		get da element number
 *
 * INPUT:
 *		darray --
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		element number of a
 *
 * NOTES:
 *		none
 *
 */
int DNS_DaNumberOfElement(darray a)
{
	if (a==NULL)
		return 0;

	return a->nel;
}

#endif /* #if (SYS_CPNT_DNS == TRUE) */

