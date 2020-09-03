/* $Id: options.h,v 1.10.2.2 2000/08/25 10:43:18 erh Exp $ */

/*
 * Copyright 1999 RedBack Networks, Incorporated.
 * All rights reserved.
 *
 * This software is not in the public domain.  It is distributed 
 * under the terms of the license in the file LICENSE in the
 * same directory as this file.  If you have received a copy of this
 * software without the LICENSE file (which means that whoever gave
 * you this software violated its license) you may obtain a copy from
 * http://www.panix.com/~tls/LICENSE.txt
 */


#ifndef _OPTIONS_H
#define _OPTIONS_H

#define WITH_OPENSSL

#define WITH_AUTH_PASSWORD
/*#define WITH_AUTH_RSA*/

//Isiah.
//#define  WITH_COMPRESSION
#define WITH_CIPHER_NONE
#define WITH_CIPHER_DES
#define WITH_CIPHER_3DES
//Isiah.
//#define WITH_CIPHER_BLOWFISH

#include "ssh_sys_bsd44+.h"		/* XXX There must be a better way. */

#endif /* _OPTIONS_H */
