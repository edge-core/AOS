/* $Id: ssh_tty.h,v 1.2.2.1 2000/08/25 09:32:18 tls Exp $ */

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

#ifndef _SSH_TTY_H
#define _SSH_TTY_H

#define SET_IFLAG(t,x,f)	if (x) (t)->c_iflag |= (f); \
				else (t)->c_iflag &= ~(f)

#define SET_OFLAG(t,x,f)	if (x) (t)->c_oflag |= (f); \
				else (t)->c_oflag &= ~(f)

#define SET_LFLAG(t,x,f)	if (x) (t)->c_lflag |= (f); \
				else (t)->c_lflag &= ~(f)

#define SET_CFLAG(t,x,f)	if (x) (t)->c_cflag |= (f); \
				else (t)->c_cflag &= ~(f)

/* From SSH RFC:
 *	The tty mode description is a stream of bytes.  The stream consists
 *	of opcode-argument pairs.  It is terminated by opcode TTY_OP_END(0).
 *	Opcodes 1-127 have one-byte arguments.  Opcodes 128-159 have 32-bit
 *	integer arguments (stored msb first).  Opcodes 160-255 are not yet
 *	defined, and cause parsing to stop (they should only be used after
 *	any other data).
 */

#define SSH_TTY_OP_END		0
#define SSH_TTY_VINTR		1
#define SSH_TTY_VQUIT		2
#define SSH_TTY_VERASE		3
#define SSH_TTY_VKILL		4
#define SSH_TTY_VEOF		5
#define SSH_TTY_VEOL		6
#define SSH_TTY_VEOL2		7
#define SSH_TTY_VSTART		8
#define SSH_TTY_VSTOP		9
#define SSH_TTY_VSUSP		10
#define SSH_TTY_VDSUSP		11
#define SSH_TTY_VREPRINT	12
#define SSH_TTY_VWERASE		13
#define SSH_TTY_VLNEXT		14
#define SSH_TTY_VFLUSH		15
#define SSH_TTY_VSWITCH		16
#define SSH_TTY_VSTATUS		17
#define SSH_TTY_VDISCARD	18
/* 19 .. 29 */
#define SSH_TTY_IGNPAR		30
#define SSH_TTY_PARMRK		31
#define SSH_TTY_INPCK		32
#define SSH_TTY_ISTRIP		33
#define SSH_TTY_INLCR		34
#define SSH_TTY_IGNCR		35
#define SSH_TTY_ICRNL		36
#define SSH_TTY_IUCLC		37
#define SSH_TTY_IXON		38
#define SSH_TTY_IXANY		39
#define SSH_TTY_IXOFF		40
#define SSH_TTY_IMAXBEL		41
/* 42 .. 49 */
#define SSH_TTY_ISIG		50
#define SSH_TTY_ICANON		51
#define SSH_TTY_XCASE		52
#define SSH_TTY_ECHO		53
#define SSH_TTY_ECHOE		54
#define SSH_TTY_ECHOK		55
#define SSH_TTY_ECHONL		56
#define SSH_TTY_NOFLSH		57
#define SSH_TTY_TOSTOP		58
#define SSH_TTY_IEXTEN		59
#define SSH_TTY_ECHOCTL		60
#define SSH_TTY_ECHOKE		61
#define SSH_TTY_PENDIN		62
/* 63 .. 69 */
#define SSH_TTY_OPOST		70
#define SSH_TTY_OLCUC		71
#define SSH_TTY_ONLCR		72
#define SSH_TTY_OCRNL		73
#define SSH_TTY_ONOCR		74
#define SSH_TTY_ONLRET		75
/* 76 .. 89 */
#define SSH_TTY_CS7		90
#define SSH_TTY_CS8		91
#define SSH_TTY_PARENB		92
#define SSH_TTY_PARODD		93
/* 94 .. 127 */

/* 32-bit arguments: */
/* 128 .. 191 */
#define SSH_TTY_OP_ISPEED	192
#define SSH_TTY_OP_OSPEED	193
/* 194 .. 255 */

#endif /* _SSH_TTY_H */

#endif /* #if 0 */
