/* MODULE NAME:  ketgen_om.c
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



/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"
#include "sys_type.h"
#include "sysfun.h"
#include "keygen_type.h"
#include "keygen_om.h"

/*isiah.2004-01-09*/
#include <string.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
//static UI32_T   keygen_om_semaphore_id;
static UI8_T	keygen_om_files_buffer[SSHD_OM_FILES_LENGTH+1];

static UI8_T    keygen_om_host_rsa_key_buffer[HOST_RSA_KEY_1024B_FILE_LENGTH+1];
static UI8_T    keygen_om_host_dsa_key_buffer[HOST_DSA_KEY_1024B_FILE_LENGTH+1];
static UI8_T    keygen_om_host_rsa_public_key_buffer[USER_RSA_PUBLIC_KEY_FILE_LENGTH+1];
static UI8_T    keygen_om_host_dsa_public_key_buffer[USER_DSA_PUBLIC_KEY_FILE_LENGTH+1];
static UI8_T    keygen_om_host_rsa_public_key_sha1_fingerprint[HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH+1];
static UI8_T    keygen_om_host_dsa_public_key_sha1_fingerprint[HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH+1];
static UI8_T    keygen_om_host_rsa_public_key_md5_fingerprint[HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH+1];
static UI8_T    keygen_om_host_dsa_public_key_md5_fingerprint[HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH+1];

static  UI32_T  keygen_task_id;
static  UI32_T  keygen_om_semid;

/* EXPORTED SUBPROGRAM BODIES
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
BOOL_T KEYGEN_OM_Init(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    if(SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_KEYGEN_OM, &keygen_om_semid)!=SYSFUN_OK)
    {
        printf("%s:get om sem id fail.\n", __FUNCTION__);
        return FALSE;
    }
    return TRUE;
}



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
BOOL_T KEYGEN_OM_EnterCriticalSection(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    if (SYSFUN_GetSem(keygen_om_semaphore_id, SYSFUN_TIMEOUT_WAIT_FOREVER) != SYSFUN_OK)
    {
        // Error
        return FALSE;
    }

    return TRUE;
}



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
BOOL_T KEYGEN_OM_LeaveCriticalSection(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    if (SYSFUN_SetSem(keygen_om_semaphore_id) != SYSFUN_OK)
    {
        // Error
        return FALSE;
    }

    return TRUE;
}
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
BOOL_T KEYGEN_OM_SetServerCertificate(UI8_T *newcert)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI8_T   *offset;
    UI32_T orig_priority;

    /* BODY */
    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    offset = keygen_om_files_buffer;
    memset(offset, 0, SERVER_CERTIFICATE_FILE_LENGTH);
    strncpy((char *)offset, (char *)newcert,SERVER_CERTIFICATE_FILE_LENGTH);
    KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return TRUE;
}



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
BOOL_T KEYGEN_OM_GetServerCertificate(UI8_T *cert)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI8_T   *offset;
    UI32_T orig_priority;

    /* BODY */
    if(cert == NULL)
    {
    	return FALSE;
    }
    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    offset = keygen_om_files_buffer;
    memset(cert, 0, SERVER_CERTIFICATE_FILE_LENGTH);
    strncpy((char *)cert, (char *)offset,SERVER_CERTIFICATE_FILE_LENGTH);
    KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return TRUE;
}



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
BOOL_T KEYGEN_OM_SetServerPrivateKey(UI8_T *newkey)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI8_T   *offset;
    UI32_T orig_priority;


    /* BODY */
    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    offset = keygen_om_files_buffer;
    offset = offset + SERVER_CERTIFICATE_FILE_LENGTH;
    memset(offset, 0, SERVER_PRIVATE_KEY_FILE_LENGTH);
    strncpy((char *)offset, (char *)newkey,SERVER_PRIVATE_KEY_FILE_LENGTH);
    KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return TRUE;
}



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
BOOL_T KEYGEN_OM_GetServerPrivateKey(UI8_T *key)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI8_T   *offset;
    UI32_T orig_priority;

    /* BODY */
    if(key == NULL)
    {
    	return FALSE;
    }
    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    offset = keygen_om_files_buffer;
    offset = offset + SERVER_CERTIFICATE_FILE_LENGTH;
    memset(key, 0, SERVER_PRIVATE_KEY_FILE_LENGTH);
    strncpy((char *)key, (char *)offset, SERVER_PRIVATE_KEY_FILE_LENGTH);
    KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return TRUE;
}



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
BOOL_T KEYGEN_OM_SetServerPassPhrase(UI8_T *passwd)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI8_T   *offset;
    UI32_T orig_priority;
    /* BODY */
     orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    offset = keygen_om_files_buffer;
    offset = offset + SERVER_CERTIFICATE_FILE_LENGTH
                    + SERVER_PRIVATE_KEY_FILE_LENGTH;
    memset(offset, 0, SERVER_PASS_PHRASE_FILE_LENGTH);
    strncpy((char *)offset, (char *)passwd, SERVER_PASS_PHRASE_FILE_LENGTH);
    KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return TRUE;
}



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
BOOL_T KEYGEN_OM_GetServerPassPhrase(UI8_T *passwd)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI8_T   *offset;
    UI32_T orig_priority;

    /* BODY */
    if(passwd == NULL)
    {
    	return FALSE;
    }

    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    offset = keygen_om_files_buffer;
    offset = offset + SERVER_CERTIFICATE_FILE_LENGTH
                    + SERVER_PRIVATE_KEY_FILE_LENGTH;
    memset(passwd, 0, SERVER_PASS_PHRASE_FILE_LENGTH);
    strncpy((char *)passwd, (char *)offset, SERVER_PASS_PHRASE_FILE_LENGTH);
    KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return TRUE;
}



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
BOOL_T KEYGEN_OM_SetTempPublicKeyPair(UI8_T *pkey, UI32_T nKeyLen, UI32_T index)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI8_T   *offset;
    UI32_T  i;
    UI32_T orig_priority;

    /* BODY */
    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    offset = keygen_om_files_buffer;
    offset = offset + SERVER_CERTIFICATE_FILE_LENGTH
                    + SERVER_PRIVATE_KEY_FILE_LENGTH
                    + SERVER_PASS_PHRASE_FILE_LENGTH
#if( SYS_CPNT_SSH2 == TRUE )
                    + HOST_RSA_KEY_1024B_FILE_LENGTH
                    + HOST_DSA_KEY_1024B_FILE_LENGTH
                    + USER_RSA_PUBLIC_KEY_FILE_LENGTH
                    + USER_DSA_PUBLIC_KEY_FILE_LENGTH
                    + HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH
                    + HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH
                    + HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH
                    + HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH
                    + (USER_PUBLIC_KEY_FILE_LENGTH * SYS_ADPT_MAX_NBR_OF_LOGIN_USER)
#endif
              ;

    if ( nKeyLen == 512 )
    {
        for ( i=1 ; i<index  ; i++ )
        {
            offset = offset + TEMP_RSA_KEY_512B_FILE_LENGTH;
        }
        memset( offset, 0, TEMP_RSA_KEY_512B_FILE_LENGTH );
        strncpy( (char *)offset, (char *)pkey, TEMP_RSA_KEY_512B_FILE_LENGTH);
    }
    else if ( nKeyLen == 768 )
    {
        offset = offset + (TEMP_RSA_KEY_512B_FILE_LENGTH * NUMBER_OF_TEMP_KEY);
        for ( i=1 ; i<index  ; i++ )
        {
            offset = offset + TEMP_RSA_KEY_768B_FILE_LENGTH;
        }
        memset( offset, 0, TEMP_RSA_KEY_768B_FILE_LENGTH );
        strncpy( (char *)offset, (char *)pkey, TEMP_RSA_KEY_768B_FILE_LENGTH);
    }
    else if ( nKeyLen == 1024 )
    {
        offset = offset + (TEMP_RSA_KEY_512B_FILE_LENGTH * NUMBER_OF_TEMP_KEY) + (TEMP_RSA_KEY_768B_FILE_LENGTH * NUMBER_OF_TEMP_KEY);
        for ( i=1 ; i<index  ; i++ )
        {
            offset = offset + TEMP_RSA_KEY_1024B_FILE_LENGTH;
        }
        memset( offset, 0, TEMP_RSA_KEY_1024B_FILE_LENGTH );
        strncpy( (char *)offset, (char *)pkey, TEMP_RSA_KEY_1024B_FILE_LENGTH);
    }
    else
    {
        KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
        return FALSE;
    }
    KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return TRUE;
}



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
BOOL_T KEYGEN_OM_GetTempPublicKeyPair(UI8_T *pkey, UI32_T nKeyLen, UI32_T index)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI8_T   *offset;
    UI32_T  i;
    UI32_T orig_priority;

    /* BODY */
    if( pkey == NULL )
    {
    	return FALSE;
    }
    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    offset = keygen_om_files_buffer;
    offset = offset + SERVER_CERTIFICATE_FILE_LENGTH
                    + SERVER_PRIVATE_KEY_FILE_LENGTH
                    + SERVER_PASS_PHRASE_FILE_LENGTH
#if( SYS_CPNT_SSH2 == TRUE )
                    + HOST_RSA_KEY_1024B_FILE_LENGTH
                    + HOST_DSA_KEY_1024B_FILE_LENGTH
                    + USER_RSA_PUBLIC_KEY_FILE_LENGTH
                    + USER_DSA_PUBLIC_KEY_FILE_LENGTH
                    + HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH
                    + HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH
                    + HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH
                    + HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH
                    + (USER_PUBLIC_KEY_FILE_LENGTH * SYS_ADPT_MAX_NBR_OF_LOGIN_USER)
#endif
             ;

    if ( nKeyLen == 512 )
    {
        for ( i=1 ; i<index  ; i++ )
        {
            offset = offset + TEMP_RSA_KEY_512B_FILE_LENGTH;
        }
        memset( pkey, 0, TEMP_RSA_KEY_512B_FILE_LENGTH );
        strncpy( (char *)pkey, (char *)offset, TEMP_RSA_KEY_512B_FILE_LENGTH );
    }
    else if ( nKeyLen == 768 )
    {
        offset = offset + (TEMP_RSA_KEY_512B_FILE_LENGTH * NUMBER_OF_TEMP_KEY);
        for ( i=1 ; i<index  ; i++ )
        {
            offset = offset + TEMP_RSA_KEY_768B_FILE_LENGTH;
        }
        memset( pkey, 0, TEMP_RSA_KEY_768B_FILE_LENGTH );
        strncpy( (char *)pkey, (char *)offset, TEMP_RSA_KEY_768B_FILE_LENGTH );
    }
    else if ( nKeyLen == 1024 )
    {
        offset = offset + (TEMP_RSA_KEY_512B_FILE_LENGTH * NUMBER_OF_TEMP_KEY) + (TEMP_RSA_KEY_768B_FILE_LENGTH * NUMBER_OF_TEMP_KEY);
        for ( i=1 ; i<index  ; i++ )
        {
            offset = offset + TEMP_RSA_KEY_1024B_FILE_LENGTH;
        }
        memset( pkey, 0, TEMP_RSA_KEY_1024B_FILE_LENGTH );
        strncpy( (char *)pkey, (char *)offset, TEMP_RSA_KEY_1024B_FILE_LENGTH );
    }
    else
    {
        /* it's too expensive to generate on-the-fly, so keep 1024bit */
        offset = offset + (TEMP_RSA_KEY_512B_FILE_LENGTH * NUMBER_OF_TEMP_KEY) + (TEMP_RSA_KEY_768B_FILE_LENGTH * NUMBER_OF_TEMP_KEY);
        memset( pkey, 0, TEMP_RSA_KEY_1024B_FILE_LENGTH );
        strncpy( (char *)pkey, (char *)offset, TEMP_RSA_KEY_1024B_FILE_LENGTH );
    }
    KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return TRUE;
}



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
BOOL_T KEYGEN_OM_GetFilesBufferPointer(UI8_T **file_buffer)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI32_T orig_priority;

    /* BODY */
    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    *file_buffer = keygen_om_files_buffer;
    KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
 	return TRUE;
}



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
BOOL_T KEYGEN_OM_SetHostRsaPublicKeyPair2Memory(UI8_T *pkey, UI8_T *pub_key, UI8_T *sha1_fingerprit, UI8_T *md5_fingerprit)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI32_T orig_priority;

    /* BODY */
    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    memset( keygen_om_host_rsa_key_buffer, 0, HOST_RSA_KEY_1024B_FILE_LENGTH+1 );
    strncpy( (char *)keygen_om_host_rsa_key_buffer, (char *)pkey, HOST_RSA_KEY_1024B_FILE_LENGTH );
    memset( keygen_om_host_rsa_public_key_buffer, 0, USER_RSA_PUBLIC_KEY_FILE_LENGTH+1 );
    strncpy( (char *)keygen_om_host_rsa_public_key_buffer, (char *)pub_key, USER_RSA_PUBLIC_KEY_FILE_LENGTH );
    memset( keygen_om_host_rsa_public_key_sha1_fingerprint, 0, HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH+1 );
    strncpy( (char *)keygen_om_host_rsa_public_key_sha1_fingerprint, (char *)sha1_fingerprit,HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH );
    memset( keygen_om_host_rsa_public_key_md5_fingerprint, 0, HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH+1 );
    strncpy( (char *)keygen_om_host_rsa_public_key_md5_fingerprint, (char *)md5_fingerprit, HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH );
    KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return TRUE;
}



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
BOOL_T KEYGEN_OM_SetHostDsaPublicKeyPair2Memory(UI8_T *pkey, UI8_T *pub_key, UI8_T *sha1_fingerprit, UI8_T *md5_fingerprit)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI32_T orig_priority;

    /* BODY */
    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    memset( keygen_om_host_dsa_key_buffer, 0, HOST_DSA_KEY_1024B_FILE_LENGTH+1 );
    strncpy( (char *)keygen_om_host_dsa_key_buffer, (char *)pkey, HOST_DSA_KEY_1024B_FILE_LENGTH );
    memset( keygen_om_host_dsa_public_key_buffer, 0, USER_DSA_PUBLIC_KEY_FILE_LENGTH+1 );
    strncpy( (char *)keygen_om_host_dsa_public_key_buffer, (char *)pub_key, USER_DSA_PUBLIC_KEY_FILE_LENGTH );
    memset( keygen_om_host_dsa_public_key_sha1_fingerprint, 0, HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH+1 );
    strncpy( (char *)keygen_om_host_dsa_public_key_sha1_fingerprint, (char *)sha1_fingerprit, HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH );
    memset( keygen_om_host_dsa_public_key_md5_fingerprint, 0, HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH+1 );
    strncpy( (char *)keygen_om_host_dsa_public_key_md5_fingerprint, (char *)md5_fingerprit, HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH );
     KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return TRUE;
}



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
BOOL_T KEYGEN_OM_WriteHostKey2Flash()
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI8_T   *offset;
    UI32_T orig_priority;

    /* BODY */
    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    offset = keygen_om_files_buffer;
    offset = offset + SERVER_CERTIFICATE_FILE_LENGTH
                    + SERVER_PRIVATE_KEY_FILE_LENGTH
                    + SERVER_PASS_PHRASE_FILE_LENGTH;
    /* set new rsa host key to buffer */
    memset( offset, 0, HOST_RSA_KEY_1024B_FILE_LENGTH );
    strncpy( (char *)offset, (char *)keygen_om_host_rsa_key_buffer, HOST_RSA_KEY_1024B_FILE_LENGTH );
    /* set new dsa host key to buffer */
    offset = offset + HOST_RSA_KEY_1024B_FILE_LENGTH;
    memset( offset, 0, HOST_DSA_KEY_1024B_FILE_LENGTH );
    strncpy( (char *)offset, (char *)keygen_om_host_dsa_key_buffer,HOST_DSA_KEY_1024B_FILE_LENGTH );

    offset = offset + HOST_DSA_KEY_1024B_FILE_LENGTH;
    memset( offset, 0, USER_RSA_PUBLIC_KEY_FILE_LENGTH );
    strncpy( (char *)offset, (char *)keygen_om_host_rsa_public_key_buffer, USER_RSA_PUBLIC_KEY_FILE_LENGTH );

    offset = offset + USER_RSA_PUBLIC_KEY_FILE_LENGTH;
    memset( offset, 0, USER_DSA_PUBLIC_KEY_FILE_LENGTH );
    strncpy( (char *)offset, (char *)keygen_om_host_dsa_public_key_buffer, USER_DSA_PUBLIC_KEY_FILE_LENGTH );

    offset = offset + USER_DSA_PUBLIC_KEY_FILE_LENGTH;
    memset( offset, 0, HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH );
    strncpy( (char *)offset, (char *)keygen_om_host_rsa_public_key_sha1_fingerprint, HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH );

    offset = offset + HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH;
    memset( offset, 0, HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH );
    strncpy( (char *)offset, (char *)keygen_om_host_rsa_public_key_md5_fingerprint, HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH );

    offset = offset + HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH;
    memset( offset, 0, HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH );
    strncpy( (char *)offset, (char *)keygen_om_host_dsa_public_key_sha1_fingerprint, HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH );

    offset = offset + HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH;
    memset( offset, 0, HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH );
    strncpy( (char *)offset, (char *)keygen_om_host_dsa_public_key_md5_fingerprint, HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH );

#if 0
    if( strcmp(keygen_om_host_rsa_key_buffer, "") == 0 )
    {
        if( strcmp(keygen_om_host_dsa_key_buffer, "") == 0 )
        {
            return FALSE;
        }
        else
        {
            /* set new dsa host key to buffer */
            offset = offset + HOST_RSA_KEY_1024B_FILE_LENGTH;
            memset( offset, 0, HOST_DSA_KEY_1024B_FILE_LENGTH );
            strcpy( offset, keygen_om_host_dsa_key_buffer );
            return TRUE;
        }
    }
    /* set new rsa host key to buffer */
    memset( offset, 0, HOST_RSA_KEY_1024B_FILE_LENGTH );
    strcpy( offset, keygen_om_host_rsa_key_buffer );

    if( strcmp(keygen_om_host_dsa_key_buffer, "") != 0 )
    {
        /* set new dsa host key to buffer */
        offset = offset + HOST_RSA_KEY_1024B_FILE_LENGTH;
        memset( offset, 0, HOST_DSA_KEY_1024B_FILE_LENGTH );
        strcpy( offset, keygen_om_host_dsa_key_buffer );
    }
#endif
    KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return TRUE;
}



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
BOOL_T KEYGEN_OM_SetHostRsaPublicKeyPair(UI8_T *pkey)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI8_T   *offset;
    UI32_T orig_priority;

    /* BODY */
    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    offset = keygen_om_files_buffer;
    offset = offset + SERVER_CERTIFICATE_FILE_LENGTH
                    + SERVER_PRIVATE_KEY_FILE_LENGTH
                    + SERVER_PASS_PHRASE_FILE_LENGTH;
    memset(offset, 0, HOST_RSA_KEY_1024B_FILE_LENGTH);
    strncpy((char *)offset, (char *)pkey, HOST_RSA_KEY_1024B_FILE_LENGTH);
    memset( keygen_om_host_rsa_key_buffer, 0, HOST_RSA_KEY_1024B_FILE_LENGTH );
    strncpy( (char *)keygen_om_host_rsa_key_buffer, (char *)pkey, HOST_RSA_KEY_1024B_FILE_LENGTH );
//    keygen_om_host_rsa_key_buffer[HOST_RSA_KEY_1024B_FILE_LENGTH] = '\0';
      KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return TRUE;
}



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
BOOL_T KEYGEN_OM_GetHostRsaPublicKeyPair(UI8_T *pkey)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI32_T orig_priority;

    /* BODY */
    if(pkey == NULL)
    {
    	return FALSE;
    }
    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    memset( pkey, 0, HOST_RSA_KEY_1024B_FILE_LENGTH );
    strncpy( (char *)pkey, (char *)keygen_om_host_rsa_key_buffer, HOST_RSA_KEY_1024B_FILE_LENGTH );
//    pkey[HOST_RSA_KEY_1024B_FILE_LENGTH] = '\0';
    KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return TRUE;
}



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
BOOL_T KEYGEN_OM_SetHostDsaPublicKeyPair(UI8_T *pkey)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI8_T   *offset;
    UI32_T orig_priority;

    /* BODY */
    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    offset = keygen_om_files_buffer;
    offset = offset + SERVER_CERTIFICATE_FILE_LENGTH
                    + SERVER_PRIVATE_KEY_FILE_LENGTH
                    + SERVER_PASS_PHRASE_FILE_LENGTH
                    + HOST_RSA_KEY_1024B_FILE_LENGTH;
    memset(offset, 0, HOST_DSA_KEY_1024B_FILE_LENGTH);
    strncpy((char *)offset, (char *)pkey, HOST_DSA_KEY_1024B_FILE_LENGTH);
    memset( keygen_om_host_dsa_key_buffer, 0, HOST_DSA_KEY_1024B_FILE_LENGTH );
    strncpy( (char *)keygen_om_host_dsa_key_buffer, (char *)pkey, HOST_DSA_KEY_1024B_FILE_LENGTH );
    KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return TRUE;
}



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
BOOL_T KEYGEN_OM_GetHostDsaPublicKeyPair(UI8_T *pkey)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI32_T orig_priority;

    /* BODY */
    if(pkey == NULL)
    {
    	return FALSE;
    }
    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    memset(pkey, 0, HOST_DSA_KEY_1024B_FILE_LENGTH);
    strncpy( (char *)pkey, (char *)keygen_om_host_dsa_key_buffer, HOST_DSA_KEY_1024B_FILE_LENGTH );
//    pkey[HOST_DSA_KEY_1024B_FILE_LENGTH] = '\0';
    KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return TRUE;
}



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
BOOL_T KEYGEN_OM_SetHostRsaPublicKey(UI8_T *pub_key)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI8_T   *offset;
    UI32_T orig_priority;

    /* BODY */
    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    offset = keygen_om_files_buffer;
    offset = offset + SERVER_CERTIFICATE_FILE_LENGTH
                    + SERVER_PRIVATE_KEY_FILE_LENGTH
                    + SERVER_PASS_PHRASE_FILE_LENGTH
                    + HOST_RSA_KEY_1024B_FILE_LENGTH
                    + HOST_DSA_KEY_1024B_FILE_LENGTH;
    memset(offset, 0, USER_RSA_PUBLIC_KEY_FILE_LENGTH);
    strncpy((char *)offset, (char *)pub_key, USER_RSA_PUBLIC_KEY_FILE_LENGTH);
    memset( keygen_om_host_rsa_public_key_buffer, 0, USER_RSA_PUBLIC_KEY_FILE_LENGTH );
    strncpy( (char *)keygen_om_host_rsa_public_key_buffer, (char *)pub_key, USER_RSA_PUBLIC_KEY_FILE_LENGTH );
    KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return TRUE;
}



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
BOOL_T KEYGEN_OM_SetHostDsaPublicKey(UI8_T *pub_key)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI8_T   *offset;
    UI32_T orig_priority;

    /* BODY */
    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    offset = keygen_om_files_buffer;
    offset = offset + SERVER_CERTIFICATE_FILE_LENGTH
                    + SERVER_PRIVATE_KEY_FILE_LENGTH
                    + SERVER_PASS_PHRASE_FILE_LENGTH
                    + HOST_RSA_KEY_1024B_FILE_LENGTH
                    + HOST_DSA_KEY_1024B_FILE_LENGTH
                    + USER_RSA_PUBLIC_KEY_FILE_LENGTH;
    memset(offset, 0, USER_DSA_PUBLIC_KEY_FILE_LENGTH);
    strncpy((char *)offset, (char *)pub_key, USER_DSA_PUBLIC_KEY_FILE_LENGTH);
    memset( keygen_om_host_dsa_public_key_buffer, 0, USER_DSA_PUBLIC_KEY_FILE_LENGTH );
    strncpy( (char *)keygen_om_host_dsa_public_key_buffer, (char *)pub_key, USER_DSA_PUBLIC_KEY_FILE_LENGTH );
     KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return TRUE;
}



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
BOOL_T KEYGEN_OM_GetHostPublicKey(UI8_T *rsa_key, UI8_T *dsa_key)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI32_T orig_priority;

    /* BODY */
    if((rsa_key == NULL) || (dsa_key == NULL))
    {
    	return FALSE;
    }
    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    strncpy((char *)rsa_key, (char *)keygen_om_host_rsa_public_key_buffer, USER_RSA_PUBLIC_KEY_FILE_LENGTH);
//    rsa_key[USER_RSA_PUBLIC_KEY_FILE_LENGTH] = '\0';
    strncpy((char *)dsa_key, (char *)keygen_om_host_dsa_public_key_buffer, USER_DSA_PUBLIC_KEY_FILE_LENGTH);
//    dsa_key[USER_DSA_PUBLIC_KEY_FILE_LENGTH] = '\0';
    KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return TRUE;
}


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
BOOL_T KEYGEN_OM_CheckSshdHostkey()
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI32_T orig_priority;

    /* BODY */
    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    if( (strcmp((char *)keygen_om_host_rsa_key_buffer, "") != 0) && (strcmp((char *)keygen_om_host_dsa_key_buffer, "") != 0) )
    {
        KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
        return TRUE;
    }
    else
    {
        KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
        return FALSE;
    }
}



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
KEYGEN_SetUserPublicKeyResult_T KEYGEN_OM_SetUserPublicKey(UI8_T *public_key, UI8_T *username, UI32_T key_type)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI8_T   *offset, *offset1, *offset2;
    UI32_T  i, j;
    /* UI32_T  username_len, offset_username_len; */
    UI32_T orig_priority;

    /* BODY */
    if( (key_type != KEY_TYPE_RSA) && (key_type != KEY_TYPE_DSA) )
    {
        return KEYGEN_SET_USER_PUBLIC_KEY_INVALID_KEY_TYPE;
    }
    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    offset = keygen_om_files_buffer;
    offset = offset + SERVER_CERTIFICATE_FILE_LENGTH
                    + SERVER_PRIVATE_KEY_FILE_LENGTH
                    + SERVER_PASS_PHRASE_FILE_LENGTH
                    + HOST_RSA_KEY_1024B_FILE_LENGTH
                    + HOST_DSA_KEY_1024B_FILE_LENGTH
                    + USER_RSA_PUBLIC_KEY_FILE_LENGTH
                    + USER_DSA_PUBLIC_KEY_FILE_LENGTH
                    + HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH
                    + HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH
                    + HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH
                    + HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH;
    for ( i=0 ; i<SYS_ADPT_MAX_NBR_OF_LOGIN_USER  ; i++ )
    {
        if( strcmp((char *)username, (char *)offset) == 0 )
        {
            if( key_type == KEY_TYPE_RSA )
            {
                offset = offset + USER_NAME_FILE_LENGTH;
                strncpy( (char *)offset, (char *)public_key, USER_RSA_PUBLIC_KEY_FILE_LENGTH );
                KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
                return KEYGEN_SET_USER_PUBLIC_KEY_SUCC;
            }
            else
            {
                /* DSA
                 */
                offset = offset + USER_NAME_FILE_LENGTH
                                + USER_RSA_PUBLIC_KEY_FILE_LENGTH;
                strncpy( (char *)offset, (char *)public_key, USER_DSA_PUBLIC_KEY_FILE_LENGTH );
                KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
                return KEYGEN_SET_USER_PUBLIC_KEY_SUCC;
            }
        }
        offset = offset + USER_PUBLIC_KEY_FILE_LENGTH;
    }

    offset = offset - USER_PUBLIC_KEY_FILE_LENGTH;
    if( strcmp("", (char *)offset) != 0 )
    {
        /* no space */
        KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
        return KEYGEN_SET_USER_PUBLIC_KEY_STORAGE_FULL;
    }

    offset = keygen_om_files_buffer;
    offset = offset + SERVER_CERTIFICATE_FILE_LENGTH
                    + SERVER_PRIVATE_KEY_FILE_LENGTH
                    + SERVER_PASS_PHRASE_FILE_LENGTH
                    + HOST_RSA_KEY_1024B_FILE_LENGTH
                    + HOST_DSA_KEY_1024B_FILE_LENGTH
                    + USER_RSA_PUBLIC_KEY_FILE_LENGTH
                    + USER_DSA_PUBLIC_KEY_FILE_LENGTH
                    + HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH
                    + HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH
                    + HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH
                    + HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH;
    for ( i=0 ; i<SYS_ADPT_MAX_NBR_OF_LOGIN_USER  ; i++ )
    {
        if( strcmp("", (char *)offset) == 0 )
        {
            if( key_type == KEY_TYPE_RSA )
            {
                strncpy( (char *)offset, (char *)username, USER_NAME_FILE_LENGTH );
                offset = offset + USER_NAME_FILE_LENGTH;
                strncpy( (char *)offset, (char *)public_key,USER_RSA_PUBLIC_KEY_FILE_LENGTH );
                KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
                return KEYGEN_SET_USER_PUBLIC_KEY_SUCC;
            }
            else if( key_type == KEY_TYPE_DSA )
            {
                strncpy( (char *)offset, (char *)username, USER_NAME_FILE_LENGTH );
                offset = offset + USER_NAME_FILE_LENGTH
                                + USER_RSA_PUBLIC_KEY_FILE_LENGTH;
                strncpy( (char *)offset, (char *)public_key, USER_DSA_PUBLIC_KEY_FILE_LENGTH );
               KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
                return KEYGEN_SET_USER_PUBLIC_KEY_SUCC;
            }
            else
            {
                KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
                return KEYGEN_SET_USER_PUBLIC_KEY_INVALID_KEY;
            }
        }
        else
        {
            /*
            username_len = strlen((char *)username);
            offset_username_len = strlen((char *)offset);
            if( ((username_len == offset_username_len) && (strcmp(username,offset)<0)) || (username_len < offset_username_len) )
            ES4649-32-01297
            */
            if(strcmp((char *)username,(char *)offset) < 0)
            {
                offset1 = keygen_om_files_buffer;
                offset1 = offset1 + SERVER_CERTIFICATE_FILE_LENGTH
                                  + SERVER_PRIVATE_KEY_FILE_LENGTH
                                  + SERVER_PASS_PHRASE_FILE_LENGTH
                                  + HOST_RSA_KEY_1024B_FILE_LENGTH
                                  + HOST_DSA_KEY_1024B_FILE_LENGTH
                                  + USER_RSA_PUBLIC_KEY_FILE_LENGTH
                                  + USER_DSA_PUBLIC_KEY_FILE_LENGTH
                                  + HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH
                                  + HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH
                                  + HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH
                                  + HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH;
                offset1 = offset1 + ((SYS_ADPT_MAX_NBR_OF_LOGIN_USER-1)*USER_PUBLIC_KEY_FILE_LENGTH);
                for( j=SYS_ADPT_MAX_NBR_OF_LOGIN_USER-1 ; j>i ; j-- )
                {
                    offset2 = offset1 - USER_PUBLIC_KEY_FILE_LENGTH;
                    memcpy(offset1, offset2, USER_PUBLIC_KEY_FILE_LENGTH);
                    offset1 = offset2;
                }

                /* ES4649-ZZ-01133
                   must ZeroMemory to prevent the original user have two kinds key (rsa, dsa)
                */
                memset(offset, 0, USER_PUBLIC_KEY_FILE_LENGTH);

                if( key_type == KEY_TYPE_RSA )
                {
                    strncpy( (char *)offset, (char *)username, USER_NAME_FILE_LENGTH );
                    offset = offset + USER_NAME_FILE_LENGTH;
                    strncpy( (char *)offset, (char *)public_key,USER_RSA_PUBLIC_KEY_FILE_LENGTH );
                }
                else if( key_type == KEY_TYPE_DSA )
                {
                    strncpy( (char *)offset, (char *)username, USER_NAME_FILE_LENGTH );
                    offset = offset + USER_NAME_FILE_LENGTH
                                    + USER_RSA_PUBLIC_KEY_FILE_LENGTH;
                    strncpy( (char *)offset, (char *)public_key, USER_DSA_PUBLIC_KEY_FILE_LENGTH );
                }
                KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
                return KEYGEN_SET_USER_PUBLIC_KEY_SUCC;
            }
        }
        offset = offset + USER_PUBLIC_KEY_FILE_LENGTH;
    }
    KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return KEYGEN_SET_USER_PUBLIC_KEY_FAIL;
}



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
BOOL_T KEYGEN_OM_DeleteUserPublicKey(UI8_T *username, UI32_T key_type)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI8_T   *offset, *offset1;
    UI32_T  i, j;
    UI32_T orig_priority;

    /* BODY */
    if( (key_type != KEY_TYPE_RSA) && (key_type != KEY_TYPE_DSA) && (key_type != KEY_TYPE_BOTH_RSA_AND_DSA) )
    {
        return FALSE;
    }
    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    offset = keygen_om_files_buffer;
    offset = offset + SERVER_CERTIFICATE_FILE_LENGTH
                    + SERVER_PRIVATE_KEY_FILE_LENGTH
                    + SERVER_PASS_PHRASE_FILE_LENGTH
                    + HOST_RSA_KEY_1024B_FILE_LENGTH
                    + HOST_DSA_KEY_1024B_FILE_LENGTH
                    + USER_RSA_PUBLIC_KEY_FILE_LENGTH
                    + USER_DSA_PUBLIC_KEY_FILE_LENGTH
                    + HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH
                    + HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH
                    + HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH
                    + HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH;
    for ( i=0 ; i<SYS_ADPT_MAX_NBR_OF_LOGIN_USER  ; i++ )
    {
        offset1 = offset;
        if( strcmp((char *)username, (char *)offset) == 0 )
        {
            if( key_type == KEY_TYPE_BOTH_RSA_AND_DSA )
            {
                memset(offset, 0, USER_PUBLIC_KEY_FILE_LENGTH);
//                return TRUE;
            }
            else if( key_type == KEY_TYPE_RSA )
            {
                offset = offset + USER_NAME_FILE_LENGTH;
                memset(offset, 0, USER_RSA_PUBLIC_KEY_FILE_LENGTH);
                offset = offset + USER_RSA_PUBLIC_KEY_FILE_LENGTH;
                if( strcmp("", (char *)offset) == 0 )
                {
                    offset = offset - USER_NAME_FILE_LENGTH
                                    - USER_RSA_PUBLIC_KEY_FILE_LENGTH;
                    memset(offset, 0, USER_NAME_FILE_LENGTH);
                }
//                return TRUE;
            }
            else if( key_type == KEY_TYPE_DSA )
            {
                offset = offset + USER_NAME_FILE_LENGTH;
                if( strcmp("", (char *)offset) == 0 )
                {
                    offset = offset - USER_NAME_FILE_LENGTH;
                    memset(offset, 0, USER_PUBLIC_KEY_FILE_LENGTH);
                }
                else
                {
                    offset = offset + USER_RSA_PUBLIC_KEY_FILE_LENGTH;
                    memset(offset, 0, USER_DSA_PUBLIC_KEY_FILE_LENGTH);
                }
//                return TRUE;
            }

            if( strcmp("", (char *)offset1) == 0 )
            {
                for( j=i+1 ; j<SYS_ADPT_MAX_NBR_OF_LOGIN_USER ; j++ )
                {
                    offset = offset1 + USER_PUBLIC_KEY_FILE_LENGTH;
                    memcpy(offset1, offset, USER_PUBLIC_KEY_FILE_LENGTH);
                    offset1 = offset;
                }
                memset(offset, 0, USER_PUBLIC_KEY_FILE_LENGTH);
            }
           KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
            return TRUE;
        }
        offset = offset + USER_PUBLIC_KEY_FILE_LENGTH;
    }
    KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return FALSE;
}



/* FUNCTION NAME : KEYGEN_OM_GetUserPublicKeyType
 * PURPOSE:
 *      Get user's public key type.
 *
 * INPUT:
 *      UI8_T   *username   -- username.
 *
 * OUTPUT:
 *      UI32_T  *key_type   --  Type of host key.(KEY_TYPE_RSA, KEY_TYPE_DSA, KEY_TYPE_BOTH_RSA_AND_DSA, KEY_TYPE_NONE)
 *
 * RETURN:
 *      TRUE  - User has public key.
 *      FALSE - User has not public key.
 *
 * NOTES:
 *      This API is invoked in KEYGEN_MGR_GetUserPublicKeyType()
 */
BOOL_T KEYGEN_OM_GetUserPublicKeyType(UI8_T *username, UI32_T *key_type)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI8_T   *offset;
    UI32_T  i;
    UI32_T orig_priority;

    /* BODY */
    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    offset = keygen_om_files_buffer;
    offset = offset + SERVER_CERTIFICATE_FILE_LENGTH
                    + SERVER_PRIVATE_KEY_FILE_LENGTH
                    + SERVER_PASS_PHRASE_FILE_LENGTH
                    + HOST_RSA_KEY_1024B_FILE_LENGTH
                    + HOST_DSA_KEY_1024B_FILE_LENGTH
                    + USER_RSA_PUBLIC_KEY_FILE_LENGTH
                    + USER_DSA_PUBLIC_KEY_FILE_LENGTH
                    + HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH
                    + HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH
                    + HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH
                    + HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH;
    for ( i=0 ; i<SYS_ADPT_MAX_NBR_OF_LOGIN_USER  ; i++ )
    {
        if( strcmp((char *)username, (char *)offset) == 0 )
        {
            offset = offset + USER_NAME_FILE_LENGTH;
            if( strcmp("", (char *)offset) == 0 )
            {
                *key_type = KEY_TYPE_DSA;
                KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
                return TRUE;
            }
            else
            {
                offset = offset + USER_RSA_PUBLIC_KEY_FILE_LENGTH;
                if( strcmp("", (char *)offset) == 0 )
                {
                    *key_type = KEY_TYPE_RSA;
                    KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
                    return TRUE;
                }
                else
                {
                    *key_type = KEY_TYPE_BOTH_RSA_AND_DSA;
                    KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
                    return TRUE;
                }
            }
        }
        offset = offset + USER_PUBLIC_KEY_FILE_LENGTH;
    }

    *key_type = KEY_TYPE_NONE;
    KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return FALSE;
}



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
BOOL_T KEYGEN_OM_GetUserPublicKey(UI8_T *username, UI8_T *rsa_key, UI8_T *dsa_key)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI8_T   *offset;
    UI32_T  i;
    UI32_T orig_priority;

    /* BODY */

    if (   (NULL == username)
        || (NULL == rsa_key)
        || (NULL == dsa_key)
        )
    {
        return FALSE;
    }

    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    offset = keygen_om_files_buffer;
    offset = offset + SERVER_CERTIFICATE_FILE_LENGTH
                    + SERVER_PRIVATE_KEY_FILE_LENGTH
                    + SERVER_PASS_PHRASE_FILE_LENGTH
                    + HOST_RSA_KEY_1024B_FILE_LENGTH
                    + HOST_DSA_KEY_1024B_FILE_LENGTH
                    + USER_RSA_PUBLIC_KEY_FILE_LENGTH
                    + USER_DSA_PUBLIC_KEY_FILE_LENGTH
                    + HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH
                    + HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH
                    + HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH
                    + HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH;
    for ( i=0 ; i<SYS_ADPT_MAX_NBR_OF_LOGIN_USER  ; i++ )
    {
        if( strcmp((char *)username, (char *)offset) == 0 )
        {
            offset = offset + USER_NAME_FILE_LENGTH;
            strncpy((char *)rsa_key, (char *)offset, USER_RSA_PUBLIC_KEY_FILE_LENGTH);
            rsa_key[USER_RSA_PUBLIC_KEY_FILE_LENGTH] = '\0';

            offset = offset + USER_RSA_PUBLIC_KEY_FILE_LENGTH;
            strncpy((char *)dsa_key, (char *)offset, USER_DSA_PUBLIC_KEY_FILE_LENGTH);
            dsa_key[USER_DSA_PUBLIC_KEY_FILE_LENGTH] = '\0';

            KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
            return TRUE;
        }
        offset = offset + USER_PUBLIC_KEY_FILE_LENGTH;
    }
    KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return FALSE;
}



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
BOOL_T KEYGEN_OM_GetNextUserPublicKey(UI8_T *username, UI8_T *rsa_key, UI8_T *dsa_key)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI8_T   *offset;
    UI32_T  i;
    UI32_T  name_len/*, offset_name_len*/;
    UI32_T orig_priority;

    /* BODY */

    if (   (NULL == username)
        || (NULL == rsa_key)
        || (NULL == dsa_key)
        )
    {
        return FALSE;
    }

    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    name_len = strlen((char *)username);
    offset = keygen_om_files_buffer;
    offset = offset + SERVER_CERTIFICATE_FILE_LENGTH
                    + SERVER_PRIVATE_KEY_FILE_LENGTH
                    + SERVER_PASS_PHRASE_FILE_LENGTH
                    + HOST_RSA_KEY_1024B_FILE_LENGTH
                    + HOST_DSA_KEY_1024B_FILE_LENGTH
                    + USER_RSA_PUBLIC_KEY_FILE_LENGTH
                    + USER_DSA_PUBLIC_KEY_FILE_LENGTH
                    + HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH
                    + HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH
                    + HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH
                    + HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH;
    if( name_len == 0 )
    {
        if( strcmp((char *)offset, "") != 0 )
        {
            strncpy((char *)username, (char *)offset, USER_NAME_FILE_LENGTH);
            username[USER_NAME_FILE_LENGTH] = '\0';

            offset = offset + USER_NAME_FILE_LENGTH;
            strncpy((char *)rsa_key, (char *)offset, USER_RSA_PUBLIC_KEY_FILE_LENGTH);
            rsa_key[USER_RSA_PUBLIC_KEY_FILE_LENGTH] = '\0';

            offset = offset + USER_RSA_PUBLIC_KEY_FILE_LENGTH;
            strncpy((char *)dsa_key, (char *)offset, USER_DSA_PUBLIC_KEY_FILE_LENGTH);
            dsa_key[USER_DSA_PUBLIC_KEY_FILE_LENGTH] = '\0';

            KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
            return TRUE;
        }
        else
        {
            KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
            return FALSE;
        }
    }
    else
    {
        for ( i=0 ; i<SYS_ADPT_MAX_NBR_OF_LOGIN_USER  ; i++ )
        {
            /*offset_name_len = strlen((char *)offset);*/
            /*
            if( ((offset_name_len==name_len)&&(strcmp(offset, username)>0)) || (offset_name_len>name_len) )
            ES4649-32-01297
            */
            if(strcmp((char *)offset, (char *)username) > 0 )
            {
                strncpy((char *)username, (char *)offset, USER_NAME_FILE_LENGTH);
                username[USER_NAME_FILE_LENGTH] = '\0';

                offset = offset + USER_NAME_FILE_LENGTH;
                strncpy((char *)rsa_key, (char *)offset, USER_RSA_PUBLIC_KEY_FILE_LENGTH);
                rsa_key[USER_RSA_PUBLIC_KEY_FILE_LENGTH] = '\0';

                offset = offset + USER_RSA_PUBLIC_KEY_FILE_LENGTH;
                strncpy((char *)dsa_key, (char *)offset, USER_DSA_PUBLIC_KEY_FILE_LENGTH);
                dsa_key[USER_DSA_PUBLIC_KEY_FILE_LENGTH] = '\0';

                KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
                return TRUE;
            }
            offset = offset + USER_PUBLIC_KEY_FILE_LENGTH;
        }
        KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
        return FALSE;
    }
}



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
BOOL_T KEYGEN_OM_SetHostRsaPublicKeyFingerPrint(UI8_T *fingerprint)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI8_T   *offset;
    UI8_T   *temp_fingerprint;
    UI32_T orig_priority;

    /* BODY */
    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    offset = keygen_om_files_buffer;
    temp_fingerprint = fingerprint;
    offset = offset + SERVER_CERTIFICATE_FILE_LENGTH
                    + SERVER_PRIVATE_KEY_FILE_LENGTH
                    + SERVER_PASS_PHRASE_FILE_LENGTH
                    + HOST_RSA_KEY_1024B_FILE_LENGTH
                    + HOST_DSA_KEY_1024B_FILE_LENGTH
                    + USER_RSA_PUBLIC_KEY_FILE_LENGTH
                    + USER_DSA_PUBLIC_KEY_FILE_LENGTH;
    memset(offset, 0, HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH+HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH);
    strncpy((char *)offset, (char *)temp_fingerprint, HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH);
    memset( keygen_om_host_rsa_public_key_sha1_fingerprint, 0, HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH );
    strncpy( (char *)keygen_om_host_rsa_public_key_sha1_fingerprint, (char *)temp_fingerprint, HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH );
//    keygen_om_host_rsa_public_key_sha1_fingerprint[HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH] = '\0';

    offset = offset + HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH;
    temp_fingerprint = temp_fingerprint + HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH;
    strncpy((char *)offset, (char *)temp_fingerprint, HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH);
    memset( keygen_om_host_rsa_public_key_md5_fingerprint, 0, HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH );
    strncpy( (char *)keygen_om_host_rsa_public_key_md5_fingerprint, (char *)temp_fingerprint, HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH );
//    keygen_om_host_rsa_public_key_md5_fingerprint[HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH] = '\0';
    KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return TRUE;
}



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
BOOL_T KEYGEN_OM_SetHostDsaPublicKeyFingerPrint(UI8_T *fingerprint)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI8_T   *offset;
    UI8_T   *temp_fingerprint;
    UI32_T   orig_priority;

    /* BODY */
    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    offset = keygen_om_files_buffer;
    temp_fingerprint = fingerprint;
    offset = offset + SERVER_CERTIFICATE_FILE_LENGTH
                    + SERVER_PRIVATE_KEY_FILE_LENGTH
                    + SERVER_PASS_PHRASE_FILE_LENGTH
                    + HOST_RSA_KEY_1024B_FILE_LENGTH
                    + HOST_DSA_KEY_1024B_FILE_LENGTH
                    + USER_RSA_PUBLIC_KEY_FILE_LENGTH
                    + USER_DSA_PUBLIC_KEY_FILE_LENGTH
                    + HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH
                    + HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH;
    memset(offset, 0, HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH+HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH);
    strncpy((char *)offset, (char *)temp_fingerprint, HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH);
    memset( keygen_om_host_dsa_public_key_sha1_fingerprint, 0, HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH );
    strncpy( (char *)keygen_om_host_dsa_public_key_sha1_fingerprint, (char *)temp_fingerprint, HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH );
//    keygen_om_host_dsa_public_key_sha1_fingerprint[HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH] = '\0';

    offset = offset + HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH;
    temp_fingerprint = temp_fingerprint + HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH;
    strncpy((char *)offset, (char *)temp_fingerprint, HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH);
    memset( keygen_om_host_dsa_public_key_md5_fingerprint, 0, HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH );
    strncpy( (char *)keygen_om_host_dsa_public_key_md5_fingerprint, (char *)temp_fingerprint, HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH );
//    keygen_om_host_dsa_public_key_md5_fingerprint[HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH] = '\0';
   KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return TRUE;
}



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
BOOL_T KEYGEN_OM_GetHostPublicKeyFingerPrint(UI8_T *rsa_sha1_fingerprint, UI8_T *rsa_md5_fingerprint, UI8_T *dsa_sha1_fingerprint, UI8_T *dsa_md5_fingerprint)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI32_T orig_priority;

    /* BODY */
	if( (rsa_sha1_fingerprint == NULL) || (rsa_md5_fingerprint == NULL) || (dsa_sha1_fingerprint == NULL) || (dsa_md5_fingerprint == NULL) )
	{
	    return FALSE;
	}
#if 0
    strncpy(rsa_sha1_fingerprint, keygen_om_host_rsa_public_key_sha1_fingerprint, HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH);
//    rsa_sha1_fingerprint[HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH] = '\0';
    strncpy(rsa_md5_fingerprint, keygen_om_host_rsa_public_key_md5_fingerprint, HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH);
//    rsa_md5_fingerprint[HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH] = '\0';
    strncpy(dsa_sha1_fingerprint, keygen_om_host_dsa_public_key_sha1_fingerprint, HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH);
//    dsa_sha1_fingerprint[HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH] = '\0';
    strncpy(dsa_md5_fingerprint, keygen_om_host_dsa_public_key_md5_fingerprint, HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH);
//    dsa_md5_fingerprint[HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH] = '\0';
#endif
    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    strncpy((char *)rsa_sha1_fingerprint, (char *)keygen_om_host_rsa_public_key_sha1_fingerprint, SIZE_sshRsaHostKeySHA1FingerPrint);
    rsa_sha1_fingerprint[SIZE_sshRsaHostKeySHA1FingerPrint] = '\0';

    strncpy((char *)rsa_md5_fingerprint, (char *)keygen_om_host_rsa_public_key_md5_fingerprint, SIZE_sshRsaHostKeyMD5FingerPrint);
    rsa_md5_fingerprint[SIZE_sshRsaHostKeyMD5FingerPrint] = '\0';

    strncpy((char *)dsa_sha1_fingerprint, (char *)keygen_om_host_dsa_public_key_sha1_fingerprint, SIZE_sshDsaHostKeySHA1FingerPrint);
    dsa_sha1_fingerprint[SIZE_sshDsaHostKeySHA1FingerPrint] = '\0';

    strncpy((char *)dsa_md5_fingerprint, (char *)keygen_om_host_dsa_public_key_md5_fingerprint, SIZE_sshDsaHostKeyMD5FingerPrint);
    dsa_md5_fingerprint[SIZE_sshDsaHostKeyMD5FingerPrint] = '\0';
    KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return TRUE;
}


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
BOOL_T KEYGEN_OM_GET_TASK_ID(UI32_T *tid)
{
    UI32_T orig_priority;

    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    *tid = keygen_task_id;
    KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return TRUE;
}

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
BOOL_T KEYGEN_OM_SET_TASK_ID(UI32_T task_id)
{
    UI32_T orig_priority;

    orig_priority=KEYGEN_OM_EnterCriticalSection(keygen_om_semid);
    keygen_task_id = task_id;
    KEYGEN_OM_LeaveCriticalSection(keygen_om_semid,orig_priority);
    return TRUE;
}











/* LOCAL SUBPROGRAM BODIES
 */



