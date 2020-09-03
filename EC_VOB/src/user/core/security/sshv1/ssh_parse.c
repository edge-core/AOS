/* $Id: ssh_parse.c,v 1.19.2.1 2000/08/25 09:32:13 tls Exp $ */

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

/*
 * Functions to parse data.
 */
#include <stdlib.h>
#include <string.h>

#include "options.h"

#include "ssh_buffer.h"
#include "ssh_cipher.h"
#include "ssh_crypto.h"
/*#include "ssh_keyfile.h"*/
/*#include "ssh_logging.h"*/
/*#include "ssh_parse.h"*/
#include "ssh_sys.h"
#include "ssh_util.h"

/* Data parsing functions: */
/*
 * Note: for a description of the layout of the keyfile
 *	look in ssh_keyfile.h.
 *
 * Decode a in-memory keyfile.
 * If interactive is non-zero this will ask for a passphrash.
 * If comment is non-NULL, a pointer to the comment field in the
 *	key is placed there.  This must be free'd by the caller.
 */
int decode_keyfile(char *buf, int buflen, char *passphrase, int pplen,
				ssh_RSA **keyp, char **comment, int *format) {

/*    SSH_DLOG(5, ("decode_keyfile\n"));*/

    /* Start of decoding: */
    if (*keyp == NULL)
	*keyp = ssh_rsa_new();
    if (*keyp == NULL) {
/*	SSH_ERROR("decode_keyfile: unable to allocate new RSA struct.\n");*/
	return(1);
    }

    if (buflen < strlen(FRESSH_PRIVKEY_IDSTR)) {
/*	SSH_ERROR("decode_keyfile: too short: %d\n", buflen);*/
	return(1);
    }
    if (strncmp(buf, FRESSH_PRIVKEY_IDSTR, strlen(FRESSH_PRIVKEY_IDSTR)) == 0) {
	if (format)
	    *format = FRESSH_PRIV_KEYFILE;
	return(decode_fressh_keyfile(buf, buflen, passphrase, pplen,
				keyp, comment));
    } else if (strncmp(buf, FSECURE_PRIVKEY_IDSTR,
			strlen(FSECURE_PRIVKEY_IDSTR)) == 0) {
	if (format)
	    *format = FSECURE_PRIV_KEYFILE;
	return(decode_fsecure_keyfile(buf, buflen, passphrase, pplen,
				keyp, comment));
    } else {
	if (format)
	    *format = 0;

/*	SSH_ERROR("decode_keyfile: Bad id string in keyfile.\n");*/
	return(1);
    }
}

int decode_fressh_keyfile(char *buf, int buflen, char *passphrase, int pplen,
			ssh_RSA **keyp, char **comment) {
  int version, minor, cipher_type;
  char *cmtbuf;

/*    SSH_DLOG(4, ("decode_fressh_keyfile\n"));*/

    cmtbuf = NULL;
	/* XXX be more careful with short buffers. */
	/* XXX (i.e. actually use buflen. ) */
	/* XXX or perhaps rewrite to use the buf_* functions */
	/* XXX (add a buf_get_line function) */

/* Macro to set the buffer to point at the start of the next line. */
#define __NEXTLINE(s) {						\
    if ((buf = strchr(buf, '\n')) == 0) {			\
/*	SSH_ERROR("decode_fressh_keyfile: missing newline after" s ".\n"); */\
	goto dfk_bad;						\
    }								\
    buf++;							\
}

/* Macro to grab and decode the next part of the key. */
#define __GETKEYPART(getpart,partstr) {				\
    if (bignum_hex2bn(&(getpart(*keyp)), buf) == 0) { \
/*	SSH_ERROR("decode_fressh_keyfile: hex2bn for %s failed.\n", partstr); */\
	goto dfk_bad;						\
    }								\
}


    /* Skip the id string: it's been checked for us already. */
    __NEXTLINE("id string");

    version = atoi(buf);
    if (version != SSH_IDSTR_VER_MAJOR) {
/*	SSH_ERROR("decode_fressh_keyfile: bad version: %d\n", version);*/
	goto dfk_bad;
    }
    /* Look for a minor. */
    if ((strchr(buf, '.') < strchr(buf, '\n')) && (strchr(buf, '.') != NULL)) {
	minor = atoi(strchr(buf, '.') + 1);
	minor = atoi(buf);
    }

    __NEXTLINE("version");

    /* Grab the comment. */
    {
      int cmtlen;
	cmtlen = strchr(buf, '\n') - buf;
	cmtbuf = malloc(cmtlen + 1);
	if (cmtbuf == NULL) {
/*	    SSH_ERROR("decode_fressh_keyfile: unable to malloc for comment.\n");*/
	    if (comment)
		*comment = NULL;
	} else {
	    strncpy(cmtbuf, buf, cmtlen);
	    cmtbuf[cmtlen] = '\0';
/*	    SSH_DLOG(4, ("decode_fressh_keyfile: comment: %s\n", cmtbuf));*/

	    /* Return the comment if it is desired. */
	    if (comment)
		*comment = cmtbuf;
	    else {
		free(cmtbuf);
		cmtbuf = NULL;
	    }
	}
    }

    __NEXTLINE("comment string");

    /* Grab the cipher type. */
    cipher_type = atoi(buf);

    /* XXX we don't handle encrypted keys yet!  This is bad! */
    if (cipher_type != 0) {
	if (passphrase)
	    /* XXX Add code to decrypt the buffer here. */
/*	    SSH_ERROR("decode_fressh_keyfile: encryption not yet supported.\n");*/
        ;
	else
	    ;/* nothing, just return */
	goto dfk_bad;
    }

    __NEXTLINE("cipher type");

    /* Make sure the check bytes are correct. */
    if ((strlen(buf) < 4) || (buf[0] != buf[2]) || (buf[1] != buf[3])) {
/*	SSH_ERROR("decode_fressh_keyfile: invalid format, check bytes fail.\n");*/
	goto dfk_bad;
    }

    __NEXTLINE("check bytes");

    __GETKEYPART(ssh_rsa_epart, "e");

    __NEXTLINE("e part");

    __GETKEYPART(ssh_rsa_npart, "n");

    {
	__NEXTLINE("n part");
	__GETKEYPART(ssh_rsa_dpart, "d");

	__NEXTLINE("d part");
	__GETKEYPART(ssh_rsa_ppart, "p");

	__NEXTLINE("p part");
	__GETKEYPART(ssh_rsa_qpart, "q");

	/* Calculate dmp1, dmq1 and iqmp */
	ssh_key_fixup(*keyp);
    }
#undef __NEXTLINE
#undef __GETKEYPART

/*    SSH_DLOG(5, ("decode_fressh_keyfile: return OK.\n"));*/
    return(0);

dfk_bad:
    if (cmtbuf)
	free(cmtbuf);
    return(1);
}

/*
 * Decode a F-secure style key file.
 */
int decode_fsecure_keyfile(char *buf, int buflen, char *passphrase, int pplen,
			ssh_RSA **keyp, char **comment) {
  struct ssh_buf inbuf;
  char *idstr;
  u_int8_t *ckbytes, *cmtbuf;
  u_int8_t ciphertype;
  u_int32_t extra, numbits, cmt_len;
  int ret_errno;

/*    SSH_DLOG(4, ("decode_fsecure_keyfile\n"));*/

    idstr = ckbytes = cmtbuf = 0;
    ciphertype = 0;

    memset(&inbuf, 0, sizeof(struct ssh_buf));
    buf_alloc(&inbuf, buflen, &ret_errno);
    buf_put_nbytes(&inbuf, buflen, buf);

#define __IFOK(x, msg)					\
    if ((x) != 0) {					\
/*	SSH_ERROR("decode_fsecure_keyfile: %s\n", msg);	*/\
	goto dfsk_bad;					\
    }

#define __GETKEYPART_F(getpart,partstr) {			\
    if (buf_get_bignum(&inbuf, &(getpart(*keyp))) != 0) {	\
/*	SSH_ERROR("decode_fsecure_keyfile: get_bn for %s failed.\n", partstr); */\
	goto dfsk_bad;						\
    }								\
 }

    __IFOK(buf_get_asciiz(&inbuf, &idstr, NULL), "get idstr");
    __IFOK(buf_get_int8(&inbuf, &ciphertype), "get ciphertype");
    __IFOK(buf_get_int32(&inbuf, &extra), "get extra");
    __IFOK(buf_get_int32(&inbuf, &numbits), "get numbits");
    __GETKEYPART_F(ssh_rsa_npart, "n");
    __GETKEYPART_F(ssh_rsa_epart, "e");
    __IFOK(buf_get_binstr(&inbuf, &cmtbuf, &cmt_len), "get comment");
    if (cmt_len > 0) {
	cmtbuf[cmt_len] = '\0';
/*	SSH_DLOG(4, ("decode_fsecure_keyfile: comment: %s\n", cmtbuf));*/
    }
    if (comment)
	*comment = cmtbuf;

    if (ciphertype != SSH_CIPHER_NONE) {
	/* XXX add encryption support */
/*	SSH_DLOG(0, ("decode_fsecure_keyfile: encrypted keyfiles not supported.\n"));*/
	goto dfsk_bad;
    } else {
	__IFOK(buf_get_nbytes(&inbuf, 4, &ckbytes), "get check bytes");
	if (ckbytes[0] != ckbytes[2] || ckbytes[1] != ckbytes[3]) {
	    /* This really isn't an SSH_ERROR */
/*	    SSH_ERROR("decode_fsecure_keyfile: bad check bytes.\n");*/
	    free(ckbytes);
	    goto dfsk_bad;
	}
	free(ckbytes);
	__GETKEYPART_F(ssh_rsa_dpart, "d");
	__GETKEYPART_F(ssh_rsa_iqmppart, "iqmp");
	__GETKEYPART_F(ssh_rsa_qpart, "q");
	__GETKEYPART_F(ssh_rsa_ppart, "p");
    }
    ssh_key_fixup(*keyp);
#undef __GETKEYPART_F
#undef __IFOK
/*    SSH_DLOG(5, ("decode_fsecure_keyfile: return OK.\n"));*/
    return(0);

dfsk_bad:
    if (idstr)
	free(idstr);
    return(1);
}

/*
 * Encodes an RSA key and a comment into a keyfile formatted buffer.
 * The buffer is allocated by this function and the size is returned.
 * The buffer must be freed by the caller.
 * Returns the size of the buffer including a terminating 0.
 */
int encode_keyfile(int format, char **buf, char *pass, int pplen,
						ssh_RSA *key, char *comment) {
    switch (format) {
      case FRESSH_PUB_KEYFILE:
	return(encode_public_keyfile(buf, key, comment));
	break;
      case FRESSH_PRIV_KEYFILE:
	return(encode_fressh_keyfile(buf, pass, pplen, key, comment));
	break;
      case FSECURE_PRIV_KEYFILE:
	return(encode_fsecure_keyfile(buf, pass, pplen, key, comment));
	break;
      default:
/*	SSH_ERROR("encode_keyfile: unknown file format: %d\n", format);*/
	return(-1);
    }
}

int encode_public_keyfile(char **buf, ssh_RSA *key, char *comment) {
  int nbits;
  char *epart, *npart;
  int len, newlen;

/*    SSH_DLOG(4, ("encode_public_keyfile\n"));*/

    if (!comment)
	comment = "";
    epart = bignum_bn2dec(ssh_rsa_epart(key));
    npart = bignum_bn2dec(ssh_rsa_npart(key));
    nbits = bignum_num_bits(ssh_rsa_npart(key));
    if (!epart || !npart) {
	if (epart) free(epart);
	if (npart) free(npart);
	return(-1);
    }
    len = strlen(epart) + 1 + strlen(npart) + 1 + 40 + strlen(comment);
    if ((*buf = malloc(len)) == NULL) {
	free(epart); free(npart);
	return(-1);
    }
/*isiah. 2002-05-28 */
/* repleaced snprintf() with sprintf() */
/*    newlen = snprintf(*buf, len, "%d %s %s %s\n", nbits, epart, npart, comment);*/
    newlen = sprintf(*buf, "%d %s %s %s\n", nbits, epart, npart, comment);
    return(newlen);
}

int decode_public_keyfile(char *buf, int len, ssh_RSA **key, char **comment) {
	/* Format: nbits e n comment */
  int nbits;
  char *e_str, *n_str;
  char *cmt;

	/* XXX make this more efficient. */
	/* XXX fix this. */
    e_str = malloc(len);
    n_str = malloc(len);
    cmt = malloc(len);
    *cmt = '\0';
    if (sscanf(buf, "%d %s %s %s", &nbits, e_str, n_str, cmt) < 3) {
	free(e_str); free(n_str) ; free(cmt);
	return(-1);
    }

    *key = ssh_rsa_new();
    if (bignum_dec2bn(&ssh_rsa_epart(*key), e_str) == 0) {
/*	SSH_ERROR("decode_public_keyfile: dec2bn for e failed.\n");*/
	free(e_str); free(n_str) ; free(cmt);
	ssh_rsa_free(*key);
	return(-1);
    }

    if (bignum_dec2bn(&ssh_rsa_npart(*key), n_str) == 0) {
/*	SSH_ERROR("decode_public_keyfile: dec2bn for n failed.\n");*/
	free(e_str); free(n_str) ; free(cmt);
	ssh_rsa_free(*key);
	return(-1);
    }

    if (comment) {
	*comment = malloc(strlen(cmt) + 1);
	strncpy(*comment, cmt, strlen(cmt) + 1);
    }

    free(e_str) ; free(n_str) ; free(cmt);

    return(0);
}
    
int encode_fressh_keyfile(char **buf, char *pass, int pplen, ssh_RSA *key,
				char *comment) {
  char *epart, *npart, *dpart;
  char *ppart, *qpart;
  int newlen, len;

/* XXX Need to add encryption of private keys. */
/*    SSH_DLOG(4, ("encode_fressh_keyfile\n"));*/

    epart = bignum_bn2hex(ssh_rsa_epart(key));
    npart = bignum_bn2hex(ssh_rsa_npart(key));
    if (!epart || !npart) {
	if (epart) free(epart);
	if (npart) free(npart);
	return(-1);
    }
    {
	dpart = bignum_bn2hex(ssh_rsa_dpart(key));
	ppart = bignum_bn2hex(ssh_rsa_ppart(key));
	qpart = bignum_bn2hex(ssh_rsa_qpart(key));
	if (!dpart || !ppart || !qpart) {
	    free(epart) ; free(npart);
	    if (dpart) free(dpart);
	    if (ppart) free(ppart);
	    if (qpart) free(qpart);
	    return(-1);
	}
    }

    len = strlen(FRESSH_PRIVKEY_IDSTR) + 1 +
	5 + 1 +	strlen(comment) + 1 + 1 + 1 +	/* Version, comment, cipher */
	4 + 1 +		/* Check bytes. */
	strlen(epart) + 1 + strlen(npart) + 1;

    len += strlen(dpart) + 1 + strlen(ppart) + 1 + strlen(qpart) + 1;
    len += 1;	/* Final zero. */

    if ((*buf = malloc(len)) == NULL) {
	free(epart); free(npart); free(dpart); free(ppart); free(qpart);
	return(-1);
    }

/*isiah. 2002-05-28 */
/* repleaced snprintf() with sprintf() */
/*    newlen = snprintf(*buf, len,
		"%s\n%2d.%2d\n%s\n%1d\n%s\n%s\n%s\n%s\n%s\n%s\n",
		FRESSH_PRIVKEY_IDSTR, SSH_IDSTR_VER_MAJOR, SSH_IDSTR_VER_MINOR,
		comment, SSH_CIPHER_NONE, "CKCK", epart, npart,
		dpart, ppart, qpart);*/
    newlen = sprintf(*buf,
		"%s\n%2d.%2d\n%s\n%1d\n%s\n%s\n%s\n%s\n%s\n%s\n",
		FRESSH_PRIVKEY_IDSTR, SSH_IDSTR_VER_MAJOR, SSH_IDSTR_VER_MINOR,
		comment, SSH_CIPHER_NONE, "CKCK", epart, npart,
		dpart, ppart, qpart);
    free(epart); free(npart); free(dpart); free(ppart); free(qpart);
    if ((newlen + 1) != len)
/*	SSH_DLOG(0, (
	"encode_fressh_keyfile: Warning: snprintf len(%d) != calculate(%d)\n",
		newlen + 1, len));*/
		    ;
    return(newlen);
}

int encode_fsecure_keyfile(char **buf, char *pass, int pplen, ssh_RSA *key,
							char *comment) {
  char tmprand[8];
  struct ssh_buf sbuf, privbuf;
  int ret_errno;

/*    SSH_DLOG(4, ("encode_fsecure_keyfile\n"));*/

    memset(&sbuf, 0, sizeof(struct ssh_buf));
    memset(&privbuf, 0, sizeof(struct ssh_buf));

    buf_alloc(&sbuf, 1024, &ret_errno);
    buf_alloc(&privbuf, 1024, &ret_errno);

#define __IFOK(x, msg)							\
    if (x != 0) {							\
/*	SSH_ERROR("encode_fsecure_keyfile: put %s failed\n", msg);	*/\
	goto efsk_bad;							\
    }					

    __IFOK(buf_put_asciiz(&sbuf, FSECURE_PRIVKEY_IDSTR), "istring");
    __IFOK(buf_put_int8(&sbuf, SSH_CIPHER_NONE), "cipher"); /* XXX */
    __IFOK(buf_put_int32(&sbuf, 0), "extra");		/* Future space */
    __IFOK(buf_put_int32(&sbuf, bignum_num_bits(ssh_rsa_npart(key))), "nbits");
    __IFOK(buf_put_bignum(&sbuf, ssh_rsa_npart(key)), "npart");
    __IFOK(buf_put_bignum(&sbuf, ssh_rsa_epart(key)), "epart");
    __IFOK(buf_put_binstr(&sbuf, comment, strlen(comment)), "comment");

    ssh_rand_bytes(2, tmprand);
    __IFOK(buf_put_nbytes(&privbuf, 2, tmprand), "chk1"); /* Check bytes. */
    __IFOK(buf_put_nbytes(&privbuf, 2, tmprand), "chk2");

    __IFOK(buf_put_bignum(&privbuf, ssh_rsa_dpart(key)), "dpart");
    __IFOK(buf_put_bignum(&privbuf, ssh_rsa_iqmppart(key)), "iqmp part");
    __IFOK(buf_put_bignum(&privbuf, ssh_rsa_qpart(key)), "qpart");
    __IFOK(buf_put_bignum(&privbuf, ssh_rsa_ppart(key)), "ppart");

    /* Add padding to make the size of the private key a multiple of 8 */
    if ((buf_alllen(&privbuf) % 8) != 0) {
      int padlen;
	padlen = 8 - (buf_alllen(&privbuf) % 8);
	ssh_rand_bytes(padlen, tmprand);
	__IFOK(buf_put_nbytes(&privbuf, padlen, tmprand), "padding");
    }

    /* XXX add encryption support */
    __IFOK(buf_append(&sbuf, &privbuf), "append");

    buf_cleanup(&privbuf);

    *buf = buf_alldata(&sbuf);
    return(buf_alllen(&sbuf));
efsk_bad:
    buf_cleanup(&privbuf);
    buf_cleanup(&sbuf);
    return(-1);
}

/*
 * Read the authorized keys file.
 * Compare each line against the given n.
 * Return a public key with the given n.
 */
int lookup_authorized_key(sshd_context_t *context, ssh_BIGNUM *n,
							ssh_RSA **key) {
  int ret;
  struct ssh_buf *ak_buf;
  char *ak_line;
  char *ak_file;
  int linelen;

    ak_buf = NULL;
    ak_line = NULL;
    ret = -1;		/* Assume we fail. */
 
    ak_file = authorized_keys_file(context);
    if (ssh_sys_readfile(ak_file, &ak_buf) < 0) {
/*	SSH_DLOG(4, ("lookup_authorized_key: unable to read file.\n"));*/
	free(ak_file);
	return(-1);
    }
    free(ak_file);

    while (buf_get_line(ak_buf, &ak_line, &linelen) == 0) {

	/* XXX grab comment and log it. */
	if (decode_public_keyfile(ak_line, linelen, key, NULL) != 0) {
	    free(ak_line);
	    ssh_rsa_free(*key);
/*	    SSH_ERROR("lookup_authorized_key: Corrupted line in file.\n");*/
	    ret = -1;
	    break;
	}

	free(ak_line);
	ak_line = NULL;

	if (bignum_compare(n, ssh_rsa_npart((*key))) == 0) {
/*	    SSH_DLOG(4, ("lookup_auth_key: found rsa key.\n"));*/
	    ret = 0;
	    break;
	}
	ssh_rsa_free(*key);
	*key = NULL;
    }
    buf_cleanup(ak_buf);
    free(ak_buf);

    return(ret);
}

#endif /* #if 0 */

