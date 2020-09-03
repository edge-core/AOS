/* $Id: ssh_deattack.c,v 1.1.4.2 2000/10/29 01:20:56 tls Exp $ */

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
 * Cryptographic attack detector for ssh - source code
 *
 * Copyright (c) 1998 CORE SDI S.A., Buenos Aires, Argentina.
 *
 * All rights reserved. Redistribution and use in source and binary
 * forms, with or without modification, are permitted provided that
 * this copyright notice is retained.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES ARE DISCLAIMED. IN NO EVENT SHALL CORE SDI S.A. BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY OR
 * CONSEQUENTIAL DAMAGES RESULTING FROM THE USE OR MISUSE OF THIS
 * SOFTWARE.
 *
 * Ariel Futoransky <futo@core-sdi.com>
 * <http://www.core-sdi.com>
 */

/*#include <assert.h>*/
/*#include <errno.h>*/
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>

#include "ssh_util.h"
/*#include "ssh_logging.h"*/

/* SSH Constants */
#define SSH_MAXBLOCKS (32 * 1024)
#define SSH_BLOCKSIZE (8)

/* Hashing constants */
#define HASH_MINSIZE (8 * 1024)
#define HASH_ENTRYSIZE (4)
#define HASH_FACTOR(x) ((x)*3/2)
#define HASH_UNUSEDCHAR (0xff)
#define HASH_UNUSED (0xffffffff)
#define HASH_IV     (0xfffffffe)

#define HASH_MINBLOCKS  (7*SSH_BLOCKSIZE)

/* Hash function (Input keys are cipher results) */
#define HASH(x) (*(u_int32_t *)x)	/* XXX This is not really a hash
					   XXX function at all, of course,
					   XXX but we leave it named this
					   XXX to keep this code looking
					   XXX like the CORE-SDI code for
					   XXX ease of validation. */

#define CMP(a,b) (memcmp(a, b, SSH_BLOCKSIZE))

void
crc_update(u_int32_t * a, u_int32_t b)
{
  b ^= *a;
  *a = ssh_crc32((unsigned char *) &b, sizeof(b));
}

/*
   check_crc
   detects if a block is used in a particular pattern
 */
  
int
check_crc(unsigned char *S, unsigned char *buf, u_int32_t len, unsigned char *IV
)
{
  u_int32_t          crc;
  unsigned char  *c;

  crc = 0;
  if (IV && !CMP(S, IV))
  {
    crc_update(&crc, 1);
    crc_update(&crc, 0);
  }
  for (c = buf; c < buf + len; c += SSH_BLOCKSIZE)
  {
    if (!CMP(S, c))
    {
      crc_update(&crc, 1);
      crc_update(&crc, 0);
    } else
    {
      crc_update(&crc, 0);
      crc_update(&crc, 0);
    }
  } 
  
  return (crc == 0);
}

/*
   detect_attack
   Detects a crc32 compensation attack on a packet
 */
int
detect_attack(unsigned char *buf, u_int32_t len, unsigned char *IV)
{
  static u_int32_t  *h = (u_int32_t *) NULL;
  static u_int32_t   n = HASH_MINSIZE / HASH_ENTRYSIZE;
  register u_int32_t i, j;
  u_int32_t          l;
  register unsigned char *c;
  unsigned char  *d;


  /*assert(len <= (SSH_MAXBLOCKS * SSH_BLOCKSIZE));*/
  if(!(len <= (SSH_MAXBLOCKS * SSH_BLOCKSIZE)))
      return -1;
  /*assert(len % SSH_BLOCKSIZE == 0);*/
  if(!(len % SSH_BLOCKSIZE == 0))
      return -1;

  for (l = n; l < HASH_FACTOR(len / SSH_BLOCKSIZE); l = l << 1);

  if (h == NULL)
  {
    n = l;
    h = (u_int32_t *) malloc(n * HASH_ENTRYSIZE);
    if (h == NULL) {
/*      SSH_ERROR("Unable to allocate memory for CRC compensation
 		attack detector: %s\n", strerror(errno));*/
      return -1;
    }
  } else
  {
    if (l > n)
    {
      n = l;
      h = (u_int32_t *) realloc(h, n * HASH_ENTRYSIZE);

      if (h == NULL) {
/*        SSH_ERROR("Unable to allocate memory for CRC compensation
                   attack detector: %s\n", strerror(errno));*/
        return -1;                                  
      }                     
    }
  } 


  
  if (len <= HASH_MINBLOCKS)
  {
    for (c = buf; c < buf + len; c += SSH_BLOCKSIZE)
    {
      if (IV && (!CMP(c, IV)))
      {
        if ((check_crc(c, buf, len, IV))) {
/*	  SSH_ERROR("CRC compensation attack detected!\n");*/
	  return 1;
	}
	else
	  break;
	}
      for (d = buf; d < c; d += SSH_BLOCKSIZE)
      {
        if (!CMP(c, d))
        {
          if ((check_crc(c, buf, len, IV))) {
/*	    SSH_ERROR("CRC compensation attack detected!\n");*/
            return 1;
	  }
          else
            break;
        }
      }
    } 
    return 0;
  }
  memset(h, HASH_UNUSEDCHAR, n * HASH_ENTRYSIZE);

  if (IV)
    h[HASH(IV) & (n - 1)] = HASH_IV;


  for (c = buf, j = 0; c < (buf + len); c += SSH_BLOCKSIZE, j++)
  {
    for (i = HASH(c) & (n - 1); h[i] != HASH_UNUSED;
         i = (i + 1) & (n - 1))
    {
      if (h[i] == HASH_IV)
      {
        if (!CMP(c, IV))
        {
          if (check_crc(c, buf, len, IV)) {
/*	    SSH_ERROR("CRC compensation attack detected!");*/
	    return 1;
	  }
	  else
	    break;
        }
      } else if (!CMP(c, buf + h[i] * SSH_BLOCKSIZE))
      {
        if (check_crc(c, buf, len, IV)) {
/*	  SSH_ERROR("CRC compensation attack detected!");*/
          return 1;
	}
	else
	  break;
     }
    } 
    h[i] = j;
  }
  
  return 0;
}

