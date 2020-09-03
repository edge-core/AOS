/*
 * Ascii internet address interpretation routine.
 * The value returned is in network order.
 */
#include "sys_type.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_DNS == TRUE)

//#include "radiusclient.h"
#include "l_stdlib.h"
/* #include "socket.h" */
#include <stdlib.h>
#include <string.h>
#include "dns_type.h"

/* isiah.2004-01-06. remove all compile warring message.*/
#include <ctype.h>

static int dns_inet_aton(const char *cp,struct in_addr *addr);

typedef unsigned int u_int32_t ;

/*
static int isascii (int  c)
 {
 return (((c) & ~0x7f)==0);
 }
 */

//#define isascii(x)  ((unsigned)(x)<=0177) /*maggie liu remove warning*/

I32_T
dns_inet_addr(cp)
	register const char *cp;
{
	struct in_addr val;

	if (dns_inet_aton(cp, &val))
		return (val.s_addr);
	return (-1);
}

/*
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 */
static int
dns_inet_aton(cp, addr)
	const char *cp;
	struct in_addr *addr;
{
	static const u_int32_t max[4] = { 0xffffffff, 0xffffff, 0xffff, 0xff };
	register u_int32_t val;	/* changed from u_long --david */
#ifndef _LIBC
	register int base;
#endif
	register int n;
	register char c;
	u_int32_t parts[4];
	register u_int32_t *pp = parts;


	memset (parts, '\0', sizeof (parts));

	c = *cp;
	for (;;) {
		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 * 0x=hex, 0=octal, isdigit=decimal.
		 */
		if (!isdigit(c))
			goto ret_0;
#ifdef _LIBC
		{
			unsigned long ul = strtoul (cp, (char **) &cp, 0);
			if (ul == ULONG_MAX && errno == ERANGE)
				goto ret_0;
			if (ul > 0xfffffffful)
				goto ret_0;
			val = ul;
		}
		c = *cp;
#else
		base = 10;
		if (c == '0') {
			c = *++cp;
			if (c == 'x' || c == 'X')
				base = 16, c = *++cp;
			else
				base = 8;
		}
		val = 0;
		for (;;) {
			if (isascii(c) && isdigit(c)) {
				val = (val * base) + (c - '0');
				c = *++cp;
			} else if (base == 16 && isascii(c) && isxdigit(c)) {
				val = (val << 4) |
					(c + 10 - (islower(c) ? 'a' : 'A'));
				c = *++cp;
			} else
				break;
		}
#endif
		if (c == '.') {
			/*
			 * Internet format:
			 *	a.b.c.d
			 *	a.b.c	(with c treated as 16 bits)
			 *	a.b	(with b treated as 24 bits)
			 */
			if (pp >= parts + 3)
				goto ret_0;
			*pp++ = val;
			c = *++cp;
		} else
			break;
	}
	/*
	 * Check for trailing characters.
	 */
	if (c != '\0' && (!isascii(c) || !isspace(c)))
		goto ret_0;
	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */
	n = pp - parts + 1;

	if (n == 0	/* initial nondigit */
	    || parts[0] > 0xff || parts[1] > 0xff || parts[2] > 0xff
	    || val > max[n - 1])
	  goto ret_0;

	val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);

	if (addr)
        {
             addr->s_addr = L_STDLIB_Hton32(val);
        }

	return (1);

ret_0:
	return (0);
}

#endif /* #if (SYS_CPNT_DNS == TRUE) */

