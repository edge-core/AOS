/* $Id: ssh_keyfile.h,v 1.8.2.1 2000/08/25 09:32:09 tls Exp $ */

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

#if 0 /*isiah.2002-10-22*/
#ifndef _SSH_KEYFILE_H
#define _SSH_KEYFILE_H

/*
 * Format of public/private key files:
 *	All fields are plain text.
 *	All fields are separated by newlines.
 *
 *   ID field	("FreSSH Private Key")
 *   Version\n	(Ascii version number. major.minor)
 *   Comment	(Ascii String)
 *   Cipher type (Ascii number identifying the cipher used for the rest)
 *   (Next 4 (3) parts are potentially encrypted.
 *	XXX This could make this file non-ascii.)
 *   Check	(4 bytes: 1 and 3, 2 and 4 should be identical)
 *   e key part	(Text hex representation of these numbers.)
 *   n key part
 *	(rest are only in private keys.)
 *   d key part
 *	(The next five are here in order to support RSAref, which
 *	unlike openssl, requires p and q instead of just d.)
 *   p key part
 *   q key part
 *
 */
#define SSH_IDSTR_VER_MAJOR	1
#define SSH_IDSTR_VER_MINOR	1
#define FRESSH_PRIVKEY_IDSTR	"FreSSH Private Key"

#define FSECURE_PRIVKEY_IDSTR	"SSH PRIVATE KEY FILE FORMAT 1.1\n"
/*
 * Compatibility keyfile format:
 *
 *	ID string	("SSH PRIVATE KEY FILE FORMAT 1.1\n")
 *	Cipher type	char
 *	0		int
 *	Num bits	int
 *	n		bignum
 *	e		bignum
 *	comment		String
 *   Encrypted portion:
 *	Random1		char
 *	Random2		char
 *	Random1 again
 *	Random2 again
 *	d		bignum
 *	iqmp		bignum
 *	q		bignum
 *	p		bignum
 *	padding to bring encrypted portion to 64-bit.
 *
 */

/*
 * Public key:  All on one line, separated by spaces:
 *	Num bits in n	int
 *	e		bignum in decimal form
 *	n		bignum in decimal form
 *	comment		string
 */

#endif /* _SSH_KEYFILE_H */

#endif /* #if 0 */
