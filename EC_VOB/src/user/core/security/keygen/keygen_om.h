/* MODULE NAME:  keygen_om.h
* PURPOSE:
*   Initialize the database resource and provide some get/set functions for accessing the
*   keygen database.
*
* NOTES:
*
* History:
*       Date          -- Modifier,  Reason
*     2002-10-16      -- Isiah , created.
*
* Copyright(C)      Accton Corporation, 2002
*/

#ifndef KEYGEN_OM_H

#define KEYGEN_OM_H



/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "keygen_type.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define KEYGEN_OM_EnterCriticalSection(sem_id)    SYSFUN_OM_ENTER_CRITICAL_SECTION(sem_id)
#define KEYGEN_OM_LeaveCriticalSection(sem_id, orig_priority)    SYSFUN_OM_LEAVE_CRITICAL_SECTION(sem_id, orig_priority)

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME:  KEYGEN_OM_Init
 * PURPOSE:
 *          Initiate the semaphore for KEYGEN objects
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in KEYGEN_MGR_Init.
 */
BOOL_T KEYGEN_OM_Init(void);



#if 0
/* FUNCTION NAME:  KEYGEN_OM_EnterCriticalSection
 * PURPOSE:
 *          Enter critical section before a task invokes the keygen objects.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_OM_EnterCriticalSection(void);



/* FUNCTION NAME:  KEYGEN_OM_LeaveCriticalSection
 * PURPOSE:
 *          Leave critical section after a task invokes the keygen objects.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_OM_LeaveCriticalSection(void);
#endif



/* FUNCTION NAME:  KEYGEN_OM_SetServerCertificate
 * PURPOSE:
 *          Store server certificate.
 *
 * INPUT:
 *          UI8_T   *newcert    -- server certificate.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_OM_SetServerCertificate(UI8_T *newcert);



/* FUNCTION NAME:  KEYGEN_OM_GetServerCertificate
 * PURPOSE:
 *          Get server certificate.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI8_T   *cert   -- server certificate.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_OM_GetServerCertificate(UI8_T *cert);



/* FUNCTION NAME:  KEYGEN_OM_SetServerPrivateKey
 * PURPOSE:
 *          Store server private key.
 *
 * INPUT:
 *          UI8_T   *newkey -- server private key.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_OM_SetServerPrivateKey(UI8_T *newkey);



/* FUNCTION NAME:  KEYGEN_OM_GetServerPrivateKey
 * PURPOSE:
 *          Get server private key.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI8_T   *key    -- server private key.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_OM_GetServerPrivateKey(UI8_T *key);



/* FUNCTION NAME:  KEYGEN_OM_SetServerPassPhrase
 * PURPOSE:
 *          Store password for read server private key.
 *
 * INPUT:
 *          UI8_T   *passwd -- password for read server private key.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_OM_SetServerPassPhrase(UI8_T *passwd);



/* FUNCTION NAME:  KEYGEN_OM_GetServerPassPhrase
 * PURPOSE:
 *          Get password for read server private key.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI8_T   *passwd -- password for read server private key.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_OM_GetServerPassPhrase(UI8_T *passwd);



/* FUNCTION NAME:  KEYGEN_OM_SetTempPublicKeyPair
 * PURPOSE:
 *          Store RSA key .
 *
 * INPUT:
 *          UI8_T   *pkey   -- RSA key.
 *
 *          UI32_T  nKeyLen -- RSA key length,valid value are 512, 768 and 1024.
 *
 *          UI32_T  index   -- index of RSA key.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_OM_SetTempPublicKeyPair(UI8_T *pkey, UI32_T nKeyLen, UI32_T index);



/* FUNCTION NAME:  KEYGEN_OM_GetTempPublicKeyPair
 * PURPOSE:
 *          Get RSA key .
 *
 * INPUT:
 *          UI32_T  nKeyLen -- RSA key length,valid value are 512, 768 and 1024.
 *
 *          UI32_T  index   -- index of RSA key.
 *
 * OUTPUT:
 *          UI8_T   *pkey   -- RSA key.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_OM_GetTempPublicKeyPair(UI8_T *pkey, UI32_T nKeyLen, UI32_T index);



/* FUNCTION NAME:  KEYGEN_OM_GetFilesBufferPointer
 * PURPOSE:
 *          This function get pointer of keygen_om_files_buffer.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI8_T   **file_buffer   -- array of pointer of keygen_om_files_buffer.
 *
 * RETURN:
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_OM_GetFilesBufferPointer(UI8_T **file_buffer);



/* FUNCTION NAME:  KEYGEN_OM_SetHostRsaPublicKeyPair2Memory
 * PURPOSE:
 *          Store host RSA key to memory.
 *
 * INPUT:
 *          UI8_T   *pkey   -- Host RSA key.
 *          UI8_T   *pub_key    --  Host RSA public key.
 *          UI8_T   *sha1_fingerprit    --  Host RSA sha1 fingerprint.
 *          UI8_T   *md5_fingerprit     --  Host RSA md5 fingerprint.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_OM_SetHostRsaPublicKeyPair2Memory(UI8_T *pkey, UI8_T *pub_key, UI8_T *sha1_fingerprit, UI8_T *md5_fingerprit);



/* FUNCTION NAME:  KEYGEN_OM_SetHostDsaPublicKeyPair2Memory
 * PURPOSE:
 *          Store host DSA key to memory.
 *
 * INPUT:
 *          UI8_T   *pkey   -- Host DSA key.
 *          UI8_T   *pub_key    --  Host DSA public key.
 *          UI8_T   *sha1_fingerprit    --  Host DSA sha1 fingerprint.
 *          UI8_T   *md5_fingerprit     --  Host DSA md5 fingerprint.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_OM_SetHostDsaPublicKeyPair2Memory(UI8_T *pkey, UI8_T *pub_key, UI8_T *sha1_fingerprit, UI8_T *md5_fingerprit);



/* FUNCTION NAME : KEYGEN_OM_WriteHostKey2Flash
 * PURPOSE:
 *      This function write host key from memory to flash.
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T KEYGEN_OM_WriteHostKey2Flash();



/* FUNCTION NAME:  KEYGEN_OM_SetHostRsaPublicKeyPair
 * PURPOSE:
 *          Store host RSA key to memory.
 *
 * INPUT:
 *          UI8_T   *pkey   -- Host RSA key.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_OM_SetHostRsaPublicKeyPair(UI8_T *pkey);



/* FUNCTION NAME:  KEYGEN_OM_GetHostRsaPublicKeyPair
 * PURPOSE:
 *          Get host RSA key to memory.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI8_T   *pkey   -- Host RSA key.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_OM_GetHostRsaPublicKeyPair(UI8_T *pkey);



/* FUNCTION NAME:  KEYGEN_OM_SetHostDsaPublicKeyPair
 * PURPOSE:
 *          Store host DSA key to memory.
 *
 * INPUT:
 *          UI8_T   *pkey   -- Host DSA key.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_OM_SetHostDsaPublicKeyPair(UI8_T *pkey);



/* FUNCTION NAME:  KEYGEN_OM_GetHostDsaPublicKeyPair
 * PURPOSE:
 *          Get host DSA key to memory.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI8_T   *pkey   -- Host DSA key.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_OM_GetHostDsaPublicKeyPair(UI8_T *pkey);



/* FUNCTION NAME : KEYGEN_OM_CheckSshdHostkey
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
BOOL_T KEYGEN_OM_CheckSshdHostkey();



/* FUNCTION NAME:  KEYGEN_OM_SetUserPublicKey
 * PURPOSE:
 *          This function set user public key.
 *
 * INPUT:
 *          UI8_T   *public_key --  user public key buffer.
 *          UI8_T   *username   --  username.
 *          UI32_T  key_type    --  Type of user public key.(KEY_TYPE_RSA, KEY_TYPE_DSA)
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          One of KEYGEN_SetUserPublicKeyResult_T to indicate that the result.
 * NOTES:
 *          .
 */
KEYGEN_SetUserPublicKeyResult_T KEYGEN_OM_SetUserPublicKey(UI8_T *public_key, UI8_T *username, UI32_T key_type);



/* FUNCTION NAME:  KEYGEN_OM_DeleteUserPublicKey
 * PURPOSE:
 *          This function delete user public key.
 *
 * INPUT:
 *			UI8_T   *username   --  name of user of public key will be delete.
 *			UI32_T  key_type    --  Type of host key.(KEY_TYPE_RSA, KEY_TYPE_DSA and KEY_TYPE_BOTH_RSA_AND_DSA)
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_OM_DeleteUserPublicKey(UI8_T *username, UI32_T key_type);



/* FUNCTION NAME : KEYGEN_OM_GetUserPublicKeyType
 * PURPOSE:
 *      Get user's public key type.
 *
 * INPUT:
 *      UI8_T   *username   -- username.
 *
 * OUTPUT:
 *      UI32_T  *key_type   --  Type of host key.(KEY_TYPE_RSA, KEY_TYPE_DSA, KEY_TYPE_BOTH_RSA_AND_DSA)
 *
 * RETURN:
 *      TRUE  - User has public key.
 *      FALSE - User has not public key.
 *
 * NOTES:
 *      This API is invoked in KEYGEN_MGR_GetUserPublicKeyType()
 */
BOOL_T KEYGEN_OM_GetUserPublicKeyType(UI8_T *username, UI32_T *key_type);



/* FUNCTION NAME : KEYGEN_OM_GetUserPublicKey
 * PURPOSE:
 *      Get user's public key type.
 *
 * INPUT:
 *      UI_8_T  *username   --  username.
 *
 * OUTPUT:
 *      UI8_T   *rsa_key    --  pointer of buffer to storage rsa public key.
 *      UI8_T   *dsa_key    --  pointer of buffer to storage dsa public key.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in KEYGEN_MGR_GetUserPublicKey()
 */
BOOL_T KEYGEN_OM_GetUserPublicKey(UI8_T *username, UI8_T *rsa_key, UI8_T *dsa_key);



/* FUNCTION NAME : KEYGEN_OM_GetNextUserPublicKey
 * PURPOSE:
 *      Get user's public key.
 *
 * INPUT:
 *      UI_8_T  *username   --  username.
 *
 * OUTPUT:
 *      UI_8_T  *username   --  Next username.
 *      UI8_T   *rsa_key    --  pointer of buffer to storage rsa public key.
 *      UI8_T   *dsa_key    --  pointer of buffer to storage dsa public key.
 *
 * RETURN:
 *      TRUE  - The output value is current active connection info.
 *      FALSE - The output value is invalid.
 *
 * NOTES:
 *      This API is invoked in KEYGEN_MGR_GetNextUserPublicKey()
 *      Initiation username is empty string.
 */
BOOL_T KEYGEN_OM_GetNextUserPublicKey(UI8_T *username, UI8_T *rsa_key, UI8_T *dsa_key);



/* FUNCTION NAME:  KEYGEN_OM_SetHostRsaPublicKey
 * PURPOSE:
 *          Store host RSA public key to memory.
 *
 * INPUT:
 *          UI8_T   *pub_key    --  Host RSA public key.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_OM_SetHostRsaPublicKey(UI8_T *pub_key);



/* FUNCTION NAME:  KEYGEN_OM_SetHostDsaPublicKey
 * PURPOSE:
 *          Store host DSA public key to memory.
 *
 * INPUT:
 *          UI8_T   *pub_key    --  Host DSA public key.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_OM_SetHostDsaPublicKey(UI8_T *pub_key);



/* FUNCTION NAME : KEYGEN_OM_GetHostPublicKey
 * PURPOSE:
 *      Get host public key.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI8_T   *rsa_key    --  pointer of buffer to storage rsa public key.
 *      UI8_T   *dsa_key    --  pointer of buffer to storage dsa public key.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in KEYGEN_MGR_GetHostPublicKey().
 */
BOOL_T KEYGEN_OM_GetHostPublicKey(UI8_T *rsa_key, UI8_T *dsa_key);



/* FUNCTION NAME:  KEYGEN_OM_SetHostRsaPublicKeyFingerPrint
 * PURPOSE:
 *          Store host RSA public key fingerprint to memory.
 *
 * INPUT:
 *          UI8_T   *fingerprint    --  Host RSA public key fingerprint.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_OM_SetHostRsaPublicKeyFingerPrint(UI8_T *fingerprint);



/* FUNCTION NAME:  KEYGEN_OM_SetHostDsaPublicKeyFingerPrint
 * PURPOSE:
 *          Store host DSA public key fingerprint to memory.
 *
 * INPUT:
 *          UI8_T   *fingerprint    --  Host DSA public key fingerprint.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_OM_SetHostDsaPublicKeyFingerPrint(UI8_T *fingerprint);



/* FUNCTION NAME : KEYGEN_OM_GetHostPublicKeyFingerPrint
 * PURPOSE:
 *      Get host public key fingerprint.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI8_T   *rsa_sha1_fingerprint   --  pointer of buffer to storage rsa public key sha1 fingerprint.
 *      UI8_T   *rsa_md5_fingerprint    --  pointer of buffer to storage rsa public key md5 fingerprint.
 *      UI8_T   *dsa_sha1_fingerprint   --  pointer of buffer to storage dsa public key sha1 fingerprint.
 *      UI8_T   *dsa_md5_fingerprint    --  pointer of buffer to storage dsa public key md5 fingerprint.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in KEYGEN_MGR_GetHostPublicKeyFingerPrint().
 */
BOOL_T KEYGEN_OM_GetHostPublicKeyFingerPrint(UI8_T *rsa_sha1_fingerprint, UI8_T *rsa_md5_fingerprint, UI8_T *dsa_sha1_fingerprint, UI8_T *dsa_md5_fingerprint);


/* FUNCTION NAME:  KEYGEN_OM_GET_TASKID
 * PURPOSE:
 *          Get Keygen Task ID
 *
 * INPUT:
 *          None
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          task id
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_OM_GET_TASK_ID(UI32_T *tid);

/* FUNCTION NAME:  KEYGEN_OM_SET_TASKID
 * PURPOSE:
 *          Set Keygen Task ID
 *
 * INPUT:
 *          None
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          task id
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_OM_SET_TASK_ID(UI32_T task_id);

#endif /* #ifndef KEYGEN_OM_H */


