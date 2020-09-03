/* $Id: ssh_crypto.h,v 1.5.4.1 2000/08/25 09:32:06 tls Exp $ */

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


/*
 * Crypto library specific function prototypes.
 *	These functions need to be implemented differently
 *	depending on the crypto library used.
 */

#include "options.h"

#ifdef WITH_OPENSSL
#include "ssh_crypto_openssl.h"
#endif

#ifdef WITH_CIPHER_NONE
#include "ssh_none.h"
#endif

#ifdef WITH_CIPHER_DES
#include "ssh_des.h"
#endif

#ifdef WITH_CIPHER_3DES
#include "ssh_3des.h"
#endif

#ifdef WITH_CIPHER_BLOWFISH
#include "ssh_blowfish.h"
#endif

void ssh_rand_feed(u_int8_t *, size_t);
void ssh_rand_bytes(size_t, u_int8_t *);
void ssh_rand_clean();

