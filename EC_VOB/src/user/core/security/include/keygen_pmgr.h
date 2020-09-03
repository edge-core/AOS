/*-----------------------------------------------------------------------------
 * Module   : keygen_pmgr.h
 *-----------------------------------------------------------------------------
 * PURPOSE  : Provide APIs to access keygen control functions.
 *-----------------------------------------------------------------------------
 * NOTES    :
 *
 *-----------------------------------------------------------------------------
 * HISTORY  : 09/27/2007 - Squid Ro, Create
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef	KEYGEN_PMGR_H
#define	KEYGEN_PMGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "keygen_mgr.h"

/* NAME CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - KEYGEN_PMGR_InitiateProcessResources
 *-------------------------------------------------------------------------
 * PURPOSE : Initiate resource used in the calling process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T KEYGEN_PMGR_InitiateProcessResources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - KEYGEN_PMGR_DeleteHostKeyPair
 *-------------------------------------------------------------------------
 * PURPOSE : Delete RSA/DSA public and private key pair of host key.
 * INPUT   : key_type - Type of host key.
 *              (KEY_TYPE_RSA, KEY_TYPE_DSA, KEY_TYPE_BOTH_RSA_AND_DSA)
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T KEYGEN_PMGR_DeleteHostKeyPair(UI32_T key_type);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - KEYGEN_PMGR_GenerateHostKeyPair
 * ------------------------------------------------------------------------|
 * FUNCTION : Generate RSA/DSA public and private key pair of host key.
 * INPUT    : key_type - Type of host key.
 *              (KEY_TYPE_RSA, KEY_TYPE_DSA, KEY_TYPE_BOTH_RSA_AND_DSA)
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: This API is invoked in CLI command "ip ssh crypto host-key grnerate".
 * ------------------------------------------------------------------------*/
BOOL_T KEYGEN_PMGR_GenerateHostKeyPair(UI32_T key_type);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - KEYGEN_PMGR_WriteHostKey2Flash
 * ------------------------------------------------------------------------|
 * FUNCTION : This function write host key from memory to flash.
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: This API is invoked in CLI command "ip ssh save host-key"
 * ------------------------------------------------------------------------*/
BOOL_T KEYGEN_PMGR_WriteHostKey2Flash(void);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - KEYGEN_PMGR_GetServerCertificate
 * ------------------------------------------------------------------------|
 * FUNCTION : This function get server certificate.
 * INPUT    : None
 * OUTPUT	: cert
 * RETURN   : TRUE/FALSE
 * NOTE		: This API is invoked in HTTP module
 * ------------------------------------------------------------------------*/
BOOL_T KEYGEN_PMGR_GetServerCertificate(UI8_T *cert);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - KEYGEN_PMGR_GetServerPrivateKey
 * ------------------------------------------------------------------------|
 * FUNCTION : This function get server private key which in certificate.
 * INPUT    : None
 * OUTPUT	: key
 * RETURN   : TRUE/FALSE
 * NOTE		: This API is invoked in HTTP module
 * ------------------------------------------------------------------------*/
BOOL_T KEYGEN_PMGR_GetServerPrivateKey(UI8_T *key);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - KEYGEN_PMGR_SetNewCertificate
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set server data include(certificate, private_key, pass_phase
 * INPUT    : server_certificate, server_private_key, server_pass_phase
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: This API is invoked in HTTP module
 * ------------------------------------------------------------------------*/
BOOL_T KEYGEN_PMGR_SetNewCertificate(UI8_T *server_certificate, UI8_T *server_private_key, UI8_T *server_pass_phrase);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - KEYGEN_PMGR_GetServerPassPhrase
 * ------------------------------------------------------------------------|
 * FUNCTION : This function get server pass phrase
 * INPUT    : None
 * OUTPUT	: passwd
 * RETURN   : TRUE/FALSE
 * NOTE		: This API is invoked in HTTP module
 * ------------------------------------------------------------------------*/
BOOL_T KEYGEN_PMGR_GetServerPassPhrase(UI8_T *passwd);

void *KEYGEN_PMGR_GetHttpsTempPublicKeyPair(int nExport, int nKeyLen);

/* FUNCTION NAME : KEYGEN_MGR_CheckSshdHostkey
 * PURPOSE:
 *      This function to check host key is exist or not.
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN:
 *      TRUE  - Host key is exist.
 *      FALSE - Host key not exist.
 *
 * NOTES:
 *      This API is invoked in SSHD_MGR_SetSshdStatus.
 */
BOOL_T KEYGEN_PMGR_CheckSshdHostkey();

#if(SYS_CPNT_SSH2 == TRUE)
/*-------------------------------------------------------------------------|
 * ROUTINE NAME - KEYGEN_PMGR_GetHostPublicKey
 * ------------------------------------------------------------------------|
 * FUNCTION : Get host public key type.
 * INPUT    : None
 * OUTPUT	: rsa_key - pointer of buffer to storage rsa public key.
 *            dsa_key - pointer of buffer to storage dsa public key.
 * RETURN   : TRUE/FALSE
 * NOTE		: This API is invoked in CLI command "show public-key".
 * ------------------------------------------------------------------------*/
BOOL_T KEYGEN_PMGR_GetHostPublicKey(UI8_T *rsa_key, UI8_T *dsa_key);

/* for SNMP
 */
/*-------------------------------------------------------------------------|
 * ROUTINE NAME - KEYGEN_PMGR_GetHostPublicKeyFingerPrint
 * ------------------------------------------------------------------------|
 * FUNCTION : Get host public key fingerprint.
 * INPUT    : None
 * OUTPUT	: rsa_sha1_fingerprint --  pointer of buffer to storage rsa public key sha1 fingerprint.
 *            rsa_md5_fingerprint  --  pointer of buffer to storage rsa public key md5 fingerprint.
 *            dsa_sha1_fingerprint --  pointer of buffer to storage dsa public key sha1 fingerprint.
 *            dsa_md5_fingerprint  --  pointer of buffer to storage dsa public key md5 fingerprint.
 * RETURN   : TRUE/FALSE
 * NOTE		: This API is invoked in UI.
 * ------------------------------------------------------------------------*/
BOOL_T KEYGEN_PMGR_GetHostPublicKeyFingerPrint(
    UI8_T   *rsa_sha1_fingerprint,  UI8_T   *rsa_md5_fingerprint,
    UI8_T   *dsa_sha1_fingerprint,  UI8_T   *dsa_md5_fingerprint);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - KEYGEN_PMGR_GetNextUserPublicKey
 * ------------------------------------------------------------------------|
 * FUNCTION : Get user's public key.
 * INPUT    : username --  username.
 * OUTPUT	: username --  Next username.
 *            rsa_key  --  pointer of buffer to storage rsa public key.
 *            dsa_key  --  pointer of buffer to storage dsa public key.key sha1 fingerprint.
 * RETURN   : TRUE/FALSE
 * NOTES    : This API is invoked in SSHD_MGR_GetNextUserPublicKey()
 *            Initiation username is empty string.
 * ------------------------------------------------------------------------*/
BOOL_T KEYGEN_PMGR_GetNextUserPublicKey(
    UI8_T   *username,  UI8_T   *rsa_key,   UI8_T   *dsa_key);

#endif /* #if(SYS_CPNT_SSH2 == TRUE) */

#endif /* KEYGEN_PMGR_H */

