/* MODULE NAME:  keygen_mgr.c
* PURPOSE:
*   Initialize the resource and provide some functions for the keygen module.
*
* NOTES:
*
* History:
*       Date          -- Modifier,  Reason
*     2002-10-16      -- Isiah , created.
*     2007-07-04      -- Rich ,  Porting to Linux Platform
* Copyright(C)      Accton Corporation, 2002
*/



/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"
#include "sys_type.h"
#include "keygen_mgr.h"
#include "keygen_om.h"
#include "sysfun.h"

#include "fs.h"
#include "fs_type.h"
#include "stktplg_pom.h"
#include "l_mm.h"

#include "openssl/ssl.h"
#include "openssl/buffer.h"
#include "openssl/err.h"
#include "openssl/rand.h"
#include "certificate_file.h"
#include "pass_phrase_file.h"

#include "sshd_type.h"
#include "sshd_mgr.h"
#include "sshd_om.h"

#ifdef DEBUG_KEYGEN
#include "syslog_type.h"
#include "syslog_mgr.h"
#endif

#include "sys_module.h"

/*isiah.2004-01-09*/
#include <string.h>
#include "sys_time.h"
#include "xmalloc.h"  /* for xfree      */
#include "uuencode.h" /* for uuencode   */

/* NAMING CONSTANT DECLARATIONS
 */

enum
{
    KEYGEN_TRACEID_GEN_KEY,
    KEYGEN_TRACEID_DEL_KEY,
    KEYGEN_TRACEID_WRITE_KEY,
};

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T KEYGEN_LoadCertificateFiles(void);
static UI32_T KEYGEN_SeedPRNG(void);
static BOOL_T KEYGEN_ChooseNum(UI32_T l, UI32_T h, UI32_T *choose);
static BOOL_T KEYGEN_SetNewCertificateToFlash(void);
static BOOL_T KEYGEN_EVPTempPrivateKey(const char *file, char *passwd, EVP_PKEY **pkey);

static BOOL_T KEYGEN_MGR_GenerateRsaHostKeyPair();
static BOOL_T KEYGEN_MGR_GenerateDsaHostKeyPair();

static unsigned long KEYGEN_MGR_id_callback(void);


/* STATIC VARIABLE DECLARATIONS
 */
static EVP_PKEY    *https_tmp_pkey = NULL;

SYSFUN_DECLARE_CSC

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : KEYGEN_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for keygen mgr.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  - There is a response need to send.
 *    FALSE - There is no response to send.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T KEYGEN_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    UI32_T  cmd;

    if(ipcmsg_p == NULL)
        return FALSE;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        KEYGEN_MGR_MSG_RETVAL(ipcmsg_p) = FALSE;
        ipcmsg_p->msg_size = KEYGEN_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
        return TRUE;
    }

    switch((cmd = KEYGEN_MGR_MSG_CMD(ipcmsg_p)))
    {
        case KEYEGN_MGR_IPC_CMD_DELETE_HOST_KEY_PAIR:
        {
            KEYGEN_MGR_IPCMsg_HostKeyType_T *data_p = KEYGEN_MGR_MSG_DATA(ipcmsg_p);
            KEYGEN_MGR_MSG_RETVAL(ipcmsg_p) = KEYGEN_MGR_DeleteHostKeyPair(data_p->key_type);
            ipcmsg_p->msg_size = KEYGEN_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case KEYEGN_MGR_IPC_CMD_GENERATE_HOST_KEY_PAIR:
        {
            KEYGEN_MGR_IPCMsg_HostKeyType_T *data_p = KEYGEN_MGR_MSG_DATA(ipcmsg_p);
            KEYGEN_MGR_MSG_RETVAL(ipcmsg_p) = KEYGEN_MGR_GenerateHostKeyPair(data_p->key_type);
            ipcmsg_p->msg_size = KEYGEN_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }


        case KEYEGN_MGR_IPC_CMD_GET_HOST_PUBLIC_KEY:
        {
            KEYGEN_MGR_IPCMsg_GetHostPublicKey_T *data_p = KEYGEN_MGR_MSG_DATA(ipcmsg_p);
            KEYGEN_MGR_MSG_RETVAL(ipcmsg_p) = KEYGEN_MGR_GetHostPublicKey(
                data_p->rsa_key, data_p->dsa_key);
            ipcmsg_p->msg_size = KEYGEN_MGR_GET_MSGBUFSIZE(KEYGEN_MGR_IPCMsg_GetHostPublicKey_T);
            break;
        }

        case KEYGEN_MGR_IPC_CMD_GET_HOST_PBKEY_FNGR_PRNT:
        {
            KEYGEN_MGR_IPCMsg_HostPbkeyFngrPrnt_T *data_p = KEYGEN_MGR_MSG_DATA(ipcmsg_p);
            KEYGEN_MGR_MSG_RETVAL(ipcmsg_p) = KEYGEN_MGR_GetHostPublicKeyFingerPrint(
                data_p->rsa_sha1_fp, data_p->rsa_md5_fp, data_p->dsa_sha1_fp, data_p->dsa_md5_fp);
            ipcmsg_p->msg_size = KEYGEN_MGR_GET_MSGBUFSIZE(KEYGEN_MGR_IPCMsg_HostPbkeyFngrPrnt_T);
            break;
        }

        case KEYGEN_MGR_IPC_CMD_GET_NXT_USER_PBKEY:
        {
            KEYGEN_MGR_IPCMsg_UserPbkey_T *data_p = KEYGEN_MGR_MSG_DATA(ipcmsg_p);
            KEYGEN_MGR_MSG_RETVAL(ipcmsg_p) = KEYGEN_MGR_GetNextUserPublicKey(
                data_p->username, data_p->rsa_key, data_p->dsa_key);
            ipcmsg_p->msg_size = KEYGEN_MGR_GET_MSGBUFSIZE(KEYGEN_MGR_IPCMsg_UserPbkey_T);
            break;
        }

        case KEYEGN_MGR_IPC_CMD_WRITE_HOST_KEY_2_FLASH:
        {
            KEYGEN_MGR_MSG_RETVAL(ipcmsg_p) = KEYGEN_MGR_WriteHostKey2Flash();
            ipcmsg_p->msg_size = KEYGEN_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case KEYEGN_MGR_IPC_CMD_GET_SERVER_CERTIFICATE:
        {
            KEYGEN_MGR_IPCMsg_GetServerCertificate_T *data_p = KEYGEN_MGR_MSG_DATA(ipcmsg_p);
            KEYGEN_MGR_MSG_RETVAL(ipcmsg_p) = KEYGEN_MGR_GetServerCertificate(data_p->cert);
            ipcmsg_p->msg_size = KEYGEN_MGR_GET_MSGBUFSIZE(KEYGEN_MGR_IPCMsg_GetServerCertificate_T);
            break;
        }

        case KEYEGN_MGR_IPC_CMD_GET_SERVER_PRIVATE_KEY:
        {
            KEYGEN_MGR_IPCMsg_GetServerPrivateKey_T *data_p = KEYGEN_MGR_MSG_DATA(ipcmsg_p);
            KEYGEN_MGR_MSG_RETVAL(ipcmsg_p) = KEYGEN_MGR_GetServerPrivateKey(data_p->server_private);
            ipcmsg_p->msg_size = KEYGEN_MGR_GET_MSGBUFSIZE(KEYGEN_MGR_IPCMsg_GetServerPrivateKey_T);
            break;
        }

        case KEYEGN_MGR_IPC_CMD_GET_SERVER_PASS_PHRASE:
        {
            KEYGEN_MGR_IPCMsg_GetServerPassPhrase_T *data_p = KEYGEN_MGR_MSG_DATA(ipcmsg_p);
            KEYGEN_MGR_MSG_RETVAL(ipcmsg_p) = KEYGEN_MGR_GetServerPassPhrase(data_p->pass_phrase);
            ipcmsg_p->msg_size = KEYGEN_MGR_GET_MSGBUFSIZE(KEYGEN_MGR_IPCMsg_GetServerPassPhrase_T);
            break;
        }

        case KEYEGN_MGR_IPC_CMD_WRITE_SERTIFICATE:
        {
            KEYGEN_MGR_IPCMsg_ServerData_T *data_p = KEYGEN_MGR_MSG_DATA(ipcmsg_p);
            KEYGEN_MGR_MSG_RETVAL(ipcmsg_p) = KEYGEN_MGR_SetNewCertificate(
            data_p->cert,
            data_p->server_private,
            data_p->pass_phrase);
            ipcmsg_p->msg_size = KEYGEN_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        default:
        {
            KEYGEN_MGR_MSG_RETVAL(ipcmsg_p) = FALSE;
            SYSFUN_Debug_Printf("*** %s(): Invalid cmd.\n", __FUNCTION__);
            return FALSE;
        }
    } /* switch ipcmsg_p->cmd */

    if (KEYGEN_MGR_MSG_RETVAL(ipcmsg_p) == FALSE)
    {
        SYSFUN_Debug_Printf("*** %s(): [cmd: %ld] failed\n", __FUNCTION__, cmd);
    }

    return TRUE;
}


/* FUNCTION NAME:  KEYGEN_MGR_Init
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
 *          This function is invoked in KEYGEN_INIT_Initiate_System_Resources.
 */
BOOL_T KEYGEN_MGR_Init(void)
{
    if ( KEYGEN_OM_Init() == FALSE )
    {
        return FALSE;
    }

    return TRUE;
}



/* FUNCTION NAME:  KEYGEN_MGR_EnterMasterMode
 * PURPOSE:
 *          This function initiates all the system database, and also configures
 *          the switch to the initiation state based on the specified "System Boot
 *          Configruation File". After that, the SSHD subsystem will enter the
 *          Master Operation mode.
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
 *          1. If "System Boot Configruation File" does not exist, the system database and
 *              switch will be initiated to the factory default value.
 *          2. KEYGEN will handle network requests only when this subsystem
 *              is in the Master Operation mode
 *          3. This function is invoked in KEYGEN_INIT_EnterMasterMode.
 */
BOOL_T KEYGEN_MGR_EnterMasterMode(void)
{
    /*
     * Registers the available ciphers and digests.
     */
    CRYPTO_set_id_callback(KEYGEN_MGR_id_callback);
    SSL_library_init();

    /*
     * Load certificate files from file system to memory.
     */
    printf("\r\nLoad certificate files : Starting\r\n");

    if ( KEYGEN_LoadCertificateFiles() == TRUE )
    {
        printf("Load certificate files : Finished\r\n");
    }
    else
    {
        printf("Load certificate files : Failured\r\n");
        return FALSE;
    }

    /* to random the seed of ssl,
     *   must make the random number big enough b4 ssh start to work
     */
    KEYGEN_SeedPRNG();

    SYSFUN_ENTER_MASTER_MODE();

    if ( KEYGEN_MGR_GenerateHostKeyPair(KEY_TYPE_BOTH_RSA_AND_DSA) != TRUE)
    {
        printf("Generate host keypair fail\r\n");
    }
    return TRUE;
}

/* FUNCTION NAME:  KEYGEN_MGR_EnterTransitionMode
 * PURPOSE:
 *          This function forces this subsystem enter the Transition Operation mode.
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
BOOL_T KEYGEN_MGR_EnterTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */

    return TRUE;
}



/* FUNCTION NAME:  KEYGEN_MGR_EnterSlaveMode
 * PURPOSE:
 *          This function forces this subsystem enter the Slave Operation mode.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          In Slave Operation mode, any network requests
 *          will be ignored.
 */
void KEYGEN_MGR_EnterSlaveMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */

    return;

}



/* FUNCTION NAME : KEYGEN_MGR_SetTransitionMode
 * PURPOSE:
 *      Set transition mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void KEYGEN_MGR_SetTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    return;
}



/* FUNCTION NAME : KEYGEN_MGR_GetOperationMode
 * PURPOSE:
 *      Get current sshd operation mode (master / slave / transition).
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *          SYS_TYPE_Stacking_Mode_T - opmode.
 *
 * NOTES:
 *      None.
 */
SYS_TYPE_Stacking_Mode_T KEYGEN_MGR_GetOperationMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    return ( SYSFUN_GET_CSC_OPERATING_MODE() );
}



/* FUNCTION NAME:  KEYGEN_MGR_GenTempPublicKeyPair
 * PURPOSE:
 *          KEYGEN private key generate main routine.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          return TRUE to indicate success and FASLE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_MGR_GenTempPublicKeyPair()
{
    static  UI32_T      index = 0;
    UI32_T              i, times;
#ifdef DEBUG_KEYGEN
    SYSLOG_OM_Record_T  syslog_entry;
#endif
    UI32_T              key_bits = 512;

#if (SYS_CPNT_SSHD == FALSE && SYS_CPNT_SSH2 == TRUE)
    static UI32_T       original_key_bit = 768;
#endif

    BOOL_T              ret = TRUE;

    /* BODY */
    index++;

    KEYGEN_SeedPRNG();

    for ( times=0 ; times<3 ; times++ )
    {
        BUF_MEM             *bm = NULL;
        EVP_PKEY            *pkey = NULL;
        BIO                 *out = NULL;
        const EVP_CIPHER          *cipher;
        RSA                 *private_ = NULL;
        BIGNUM              *f4 = NULL;

#if (SYS_CPNT_SSHD == FALSE && SYS_CPNT_SSH2 == TRUE)
            key_bits =768;
            SSHD_OM_GetServerKeySize(&key_bits);
            if( key_bits != original_key_bit )
            {
                original_key_bit = key_bits;
                index = 0;
                break;
            }
#endif

        if ((out = BIO_new(BIO_s_mem())) == NULL)
        {
            goto err;
        }

        if ((bm = BUF_MEM_new()) == NULL)
        {
            goto err;
        }

        bm->data = (char *) OPENSSL_malloc(HOST_RSA_KEY_1024B_FILE_LENGTH+1);

        if (bm->data == NULL)
        {
            BUF_MEM_free(bm);
            bm = NULL;
            goto err;
        }

        bm->max = HOST_RSA_KEY_1024B_FILE_LENGTH+1;
        bm->length = 0;

        BIO_set_mem_buf(out, bm, 0);

        switch ( times )
        {
            case 0:
                key_bits = 512;
                break;
            case 1:
#if (SYS_CPNT_SSHD == FALSE && SYS_CPNT_SSH2 == TRUE)
                    key_bits = 768;
                    SSHD_OM_GetServerKeySize(&key_bits);
#else
                key_bits = 768;
#endif
                break;
            case 2:
                key_bits = 1024;
                break;
        }

        if ((pkey=EVP_PKEY_new()) == NULL)
        {
#ifdef DEBUG_KEYGEN
            memset((UI8_T *)&syslog_entry, 0, sizeof(SYSLOG_OM_Record_T));
            syslog_entry.owner_info.level = SYSLOG_LEVEL_DEBUG;
            syslog_entry.owner_info.module_no = SYS_MODULE_SSL;
            syslog_entry.owner_info.function_no = HTTP_TASK_Enter_Pkey_Gen_Main_Routine_FunNo;
            syslog_entry.owner_info.error_no = EVP_PKEY_new_ErrNo;
            sprintf(syslog_entry.message, "EVP_PKEY_new() error");
            SYSLOG_PMGR_AddEntry(&syslog_entry);
            printf("EVP_PKEY_new() error");
#endif
            goto err;
        }

        if ((private_ = RSA_new()) == NULL)
        {
            goto err;
        }

        if ((f4 = BN_new()) == NULL)
        {
            goto err;
        }

        if (!BN_set_word(f4, RSA_F4))
        {
            goto err;
        }

        if (!RSA_generate_key_ex(private_, key_bits, f4, NULL))
        {
            goto err;
        }

        if (!EVP_PKEY_assign_RSA(pkey, private_))
        {
#ifdef DEBUG_KEYGEN
            memset((UI8_T *)&syslog_entry, 0, sizeof(SYSLOG_OM_Record_T));
            syslog_entry.owner_info.level = SYSLOG_LEVEL_DEBUG;
            syslog_entry.owner_info.module_no = SYS_MODULE_SSL;
            syslog_entry.owner_info.function_no = HTTP_TASK_Enter_Pkey_Gen_Main_Routine_FunNo;
            syslog_entry.owner_info.error_no = EVP_PKEY_assign_RSA_ErrNo;
            sprintf(syslog_entry.message, "generate temp key error");
            SYSLOG_PMGR_AddEntry(&syslog_entry);
            printf("generate temp key error");
#endif
            goto err;
        }

        private_ = NULL;

        cipher=EVP_des_ede3_cbc();

        i=0;

        while(TRUE)
        {
            if (!PEM_write_bio_PrivateKey(out,pkey,cipher,NULL,0,NULL,PKEY_PASSWD))
            {
                if ((ERR_GET_REASON(ERR_peek_error()) == PEM_R_PROBLEMS_GETTING_PASSWORD) && (i < 3))
                {
                    ERR_clear_error();
                    i++;
                    continue;
                }

                goto err;
            }
            break;
        }

#if (SYS_CPNT_SSHD == FALSE && SYS_CPNT_SSH2 == TRUE)
        if( times == 1 )
        {
            key_bits = 768;
        }
#endif

        KEYGEN_OM_SetTempPublicKeyPair((UI8_T *)bm->data, key_bits, index);

        if (0)
        {
err:
            index--;
            ret = FALSE;
        }

        if (private_)
        {
            RSA_free(private_);
            private_ = NULL;
        }

        if (f4)
        {
            BN_free(f4);
            f4 = NULL;
        }

        /* Don't free bm, it had assign to out.
         */
        if (out)
        {
            BIO_set_close(out, 1);
            BIO_free_all(out);
            out = NULL;
        }

        if (pkey)
        {
            EVP_PKEY_free(pkey);
            pkey = NULL;
        }

        if (ret != TRUE)
        {
            break;
        }
    }

    if ( index == NUMBER_OF_TEMP_KEY )
    {
        index = 0;
    }

    return ret;
}



/* FUNCTION NAME:  KEYGEN_MGR_GetServerCertificate
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
BOOL_T KEYGEN_MGR_GetServerCertificate(UI8_T *cert)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  result;

    /* BODY */
    if(cert == NULL)
    {
        return FALSE;
    }
    result = KEYGEN_OM_GetServerCertificate(cert);

    return result;
}



/* FUNCTION NAME:  KEYGEN_MGR_GetServerPrivateKey
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
BOOL_T KEYGEN_MGR_GetServerPrivateKey(UI8_T *key)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  result;

    /* BODY */
    if(key == NULL)
    {
        return FALSE;
    }
    result = KEYGEN_OM_GetServerPrivateKey(key);

    return result;
}



/* FUNCTION NAME:  KEYGEN_MGR_GetServerPassPhrase
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
BOOL_T KEYGEN_MGR_GetServerPassPhrase(UI8_T *passwd)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  result;

    /* BODY */
    if(passwd == NULL)
    {
        return FALSE;
    }
    result = KEYGEN_OM_GetServerPassPhrase(passwd);

    return result;
}



/* FUNCTION NAME:  KEYGEN_MGR_GetHttpsTempPublicKeyPair
 * PURPOSE:
 *          Get 512 and 1024 bits temporary keys
 *
 * INPUT:
 *          int nExport -- 1 to indicate an export cipher is used and 0 to indicate not.
 *
 *          int nKeyLen -- temporary key length,valid value are 512 and 1024 .
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          RSA *rsa    -- RSA structure.
 * NOTES:
 *          This API is invoked in HTTP_Get_Temp_PublicKey_Pair
 */
void *KEYGEN_MGR_GetHttpsTempPublicKeyPair(int nExport, int nKeyLen)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    static  UI32_T  index = 0;
    RSA     *rsa = NULL;
    char    pkey[TEMP_RSA_KEY_1024B_FILE_LENGTH+1];

    /* BODY */
    memset(pkey, 0, TEMP_RSA_KEY_1024B_FILE_LENGTH+1);
    if ( https_tmp_pkey != NULL )
    {
        EVP_PKEY_free(https_tmp_pkey);
    }

    index++;

    if (nExport)
    {
        KEYGEN_OM_GetTempPublicKeyPair((UI8_T *)pkey,nKeyLen,index);

        if ( KEYGEN_EVPTempPrivateKey(pkey, PKEY_PASSWD, &https_tmp_pkey) == TRUE )
        {
            rsa = https_tmp_pkey->pkey.rsa;
        }
        else
        {
            index--;
            return NULL;
        }

        /* It's because an export cipher is used */
    }
    else
    {
        KEYGEN_OM_GetTempPublicKeyPair((UI8_T *)pkey,1024,index);

        if ( KEYGEN_EVPTempPrivateKey(pkey, PKEY_PASSWD, &https_tmp_pkey) == TRUE )
        {
            rsa = https_tmp_pkey->pkey.rsa;
        }
        else
        {
            index--;
            return NULL;
        }

        /* It's because a sign-only certificate situation exists */
    }

    if ( index == NUMBER_OF_TEMP_KEY )
    {
        index = 0;
    }

    return rsa;
}



/* FUNCTION NAME:  KEYGEN_MGR_SetNewCertificate
 * PURPOSE:
 *          Set new certificate, private key, and pass phrase to Files_Buffer and file system.
 *
 * INPUT:
 *          UI8_T   *server_certificate --  pointer of server certificate.
 *          UI8_T   *server_private_key --  pointer of server private key.
 *          UI8_T   *server_pass_phrase --  pointer of key pass phrase.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_MGR_SetNewCertificate(UI8_T *server_certificate, UI8_T *server_private_key, UI8_T *server_pass_phrase)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  result;

    /* BODY */

    KEYGEN_OM_SetServerCertificate(server_certificate);
    KEYGEN_OM_SetServerPrivateKey(server_private_key);
    KEYGEN_OM_SetServerPassPhrase(server_pass_phrase);


    result = KEYGEN_SetNewCertificateToFlash();

    return result;
}



/* FUNCTION NAME:  KEYGEN_MGR_GetSshdHostkey
 * PURPOSE:
 *          This function get hostkey of sshd.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          void * -pointer of rsa key .
 * NOTES:
 *          .
 */
void *KEYGEN_MGR_GetSshdHostkey()
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI8_T serverprivatekey[SERVER_PRIVATE_KEY_FILE_LENGTH+1];
    UI8_T passwd[SERVER_PASS_PHRASE_FILE_LENGTH+1];
    EVP_PKEY *tmp_pkey;

    /* BODY */
    memset(serverprivatekey, 0, SERVER_PRIVATE_KEY_FILE_LENGTH+1);
    memset(passwd, 0, SERVER_PASS_PHRASE_FILE_LENGTH+1);

    KEYGEN_OM_GetServerPrivateKey(serverprivatekey);
    KEYGEN_OM_GetServerPassPhrase(passwd);

    if ( KEYGEN_EVPTempPrivateKey((char *)serverprivatekey, (char *)passwd, &tmp_pkey) == TRUE )
    {
        return (void *)tmp_pkey;
    }
    else
    {
        return NULL;
    }
}

/* FUNCTION NAME:  KEYGEN_MGR_GetSshdServerkey
 * PURPOSE:
 *          This function get serverkey of sshd.
 *
 * INPUT:
 *          UI32_T - index of RSA key.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          void * -pointer of rsa key .
 * NOTES:
 *          .
 */
void *KEYGEN_MGR_GetSshdServerkey(UI32_T index)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    char pkey[TEMP_RSA_KEY_768B_FILE_LENGTH+1];
    EVP_PKEY *tmp_pkey;

    /* BODY */
    memset(pkey, 0, TEMP_RSA_KEY_768B_FILE_LENGTH+1);

    KEYGEN_OM_GetTempPublicKeyPair((UI8_T *)pkey,768,index);

    if ( KEYGEN_EVPTempPrivateKey(pkey, PKEY_PASSWD, &tmp_pkey) == TRUE )
    {
        return (void *)tmp_pkey;
    }
    else
    {
        return NULL;
    }
}




/* FUNCTION NAME:  KEYGEN_MGR_GetSshRsadHostkey
 * PURPOSE:
 *          This function get RSA hostkey of sshd.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          void * -pointer of rsa key .
 * NOTES:
 *          .
 */
void *KEYGEN_MGR_GetSshdRsaHostkey()
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    char pkey[HOST_RSA_KEY_1024B_FILE_LENGTH+1];
    EVP_PKEY *tmp_pkey;
    RSA *rsa;

    /* BODY */
    memset(pkey, 0, HOST_RSA_KEY_1024B_FILE_LENGTH+1);

    KEYGEN_OM_GetHostRsaPublicKeyPair((UI8_T *)pkey);

    if( strcmp(pkey, "") == 0 )
    {
        return NULL;
    }

    if( KEYGEN_EVPTempPrivateKey(pkey, PKEY_PASSWD, &tmp_pkey) == TRUE )
    {
/*isiah.2003-05-22*/
        rsa = tmp_pkey->pkey.rsa;
        tmp_pkey->pkey.rsa = NULL;
        EVP_PKEY_free(tmp_pkey);
        return (void *)rsa;
    }
    else
    {
        return NULL;
    }
}

/* FUNCTION NAME:  KEYGEN_MGR_GetSshDsadHostkey
 * PURPOSE:
 *          This function get DSA hostkey of sshd.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          void * -pointer of dsa key .
 * NOTES:
 *          .
 */
void *KEYGEN_MGR_GetSshdDsaHostkey()
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    char pkey[HOST_DSA_KEY_1024B_FILE_LENGTH+1];
    EVP_PKEY *tmp_pkey;
    DSA *dsa;

    /* BODY */
    memset(pkey, 0, HOST_DSA_KEY_1024B_FILE_LENGTH+1);

    KEYGEN_OM_GetHostDsaPublicKeyPair((UI8_T *)pkey);

    if( strcmp(pkey, "") == 0 )
    {
        return NULL;
    }

    if( KEYGEN_EVPTempPrivateKey(pkey, PKEY_PASSWD, &tmp_pkey) == TRUE )
    {
/*isiah.2003-05-22*/
        dsa = tmp_pkey->pkey.dsa;
        tmp_pkey->pkey.dsa = NULL;
        EVP_PKEY_free(tmp_pkey);
        return (void *)dsa;
    }
    else
    {
        return NULL;
    }
}

/* FUNCTION NAME : KEYGEN_MGR_WriteHostKey2Flash
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
 *      This API is invoked in SSHD_MGR_WriteHostKey2Flash.
 */
BOOL_T KEYGEN_MGR_WriteHostKey2Flash()
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */

    ret = KEYGEN_OM_WriteHostKey2Flash();

    if( ret == TRUE )
    {
        ret = KEYGEN_SetNewCertificateToFlash();
    }
    return ret;
}

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
BOOL_T KEYGEN_MGR_CheckSshdHostkey()
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */

    ret = KEYGEN_OM_CheckSshdHostkey();

    return ret;
}

/* FUNCTION NAME : KEYGEN_MGR_GenerateHostKeyPair
 * PURPOSE:
 *      Generate RSA/DSA public and private key pair of host key.
 *
 * INPUT:
 *      UI32_T  key_type    --  Type of host key.(KEY_TYPE_RSA, KEY_TYPE_DSA, KEY_TYPE_BOTH_RSA_AND_DSA)
 *
 * OUTPUT:
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in CLI command "ip ssh crypto host-key grnerate".
 */
BOOL_T KEYGEN_MGR_GenerateHostKeyPair(UI32_T key_type)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    if( key_type == KEY_TYPE_RSA )
    {
        return ( KEYGEN_MGR_GenerateRsaHostKeyPair() );
    }
    else if( key_type == KEY_TYPE_DSA )
    {
        return ( KEYGEN_MGR_GenerateDsaHostKeyPair() );
    }
    else if( key_type == KEY_TYPE_BOTH_RSA_AND_DSA )
    {
        if( KEYGEN_MGR_GenerateDsaHostKeyPair() == TRUE )
        {
            return ( KEYGEN_MGR_GenerateRsaHostKeyPair() );
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
}

/* FUNCTION NAME : KEYGEN_MGR_DeleteHostKeyPair
 * PURPOSE:
 *      Delete RSA/DSA public and private key pair of host key.
 *
 * INPUT:
 *      UI32_T  key_type    --  Type of host key.(KEY_TYPE_RSA, KEY_TYPE_DSA, KEY_TYPE_BOTH_RSA_AND_DSA)
 *
 * OUTPUT:
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in CLI command "ip ssh crypto zeroize".
 *      This function only clear a host key from memory.
 */
BOOL_T KEYGEN_MGR_DeleteHostKeyPair(UI32_T key_type)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    SSHD_State_T    sshd_status;
    /* BODY */

    sshd_status =  SSHD_OM_GetSshdStatus();

    if( sshd_status == SSHD_STATE_ENABLED )
    {
        return FALSE;
    }

    if( key_type == KEY_TYPE_BOTH_RSA_AND_DSA )
    {
        KEYGEN_OM_SetHostRsaPublicKeyPair2Memory((UI8_T *)"", (UI8_T *)"", (UI8_T *)"", (UI8_T *)"");
        KEYGEN_OM_SetHostDsaPublicKeyPair2Memory((UI8_T *)"", (UI8_T *)"", (UI8_T *)"", (UI8_T *)"");
        return TRUE;
    }
    else if( key_type == KEY_TYPE_RSA )
    {
        KEYGEN_OM_SetHostRsaPublicKeyPair2Memory((UI8_T *)"", (UI8_T *)"", (UI8_T *)"", (UI8_T *)"");
        return TRUE;
    }
    else if( key_type == KEY_TYPE_DSA )
    {
        KEYGEN_OM_SetHostDsaPublicKeyPair2Memory((UI8_T *)"", (UI8_T *)"", (UI8_T *)"", (UI8_T *)"");
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* FUNCTION NAME : KEYGEN_MGR_GetHostPublicKey
 * PURPOSE:
 *      Get host public key type.
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
 *      This API is invoked in CLI command "show public-key".
 */
BOOL_T KEYGEN_MGR_GetHostPublicKey(UI8_T *rsa_key, UI8_T *dsa_key)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;

    /* BODY */
    if( (rsa_key == NULL) || (dsa_key == NULL) )
    {
        return FALSE;
    }

    ret = KEYGEN_OM_GetHostPublicKey(rsa_key, dsa_key);
    return ret;
}

/* FUNCTION NAME:  KEYGEN_MGR_SetUserPublicKey
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
 *      One of KEYGEN_SetUserPublicKeyResult_T to indicate that the result.
 * NOTES:
 *          .
 */
KEYGEN_SetUserPublicKeyResult_T KEYGEN_MGR_SetUserPublicKey(UI8_T *public_key, UI8_T *username, UI32_T key_type)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    KEYGEN_SetUserPublicKeyResult_T result;

    /* BODY */
    result = KEYGEN_OM_SetUserPublicKey(public_key, username, key_type);
    if (KEYGEN_SET_USER_PUBLIC_KEY_SUCC != result)
    {
        return result;
    }

    if (FALSE == KEYGEN_SetNewCertificateToFlash())
    {
        return SSHD_SET_USER_PUBLIC_KEY_WRITE_ERROR;
    }

    return KEYGEN_SET_USER_PUBLIC_KEY_SUCC;
}

/* FUNCTION NAME:  KEYGEN_MGR_DeleteUserPublicKey
 * PURPOSE:
 *          This function delete user public key.
 *
 * INPUT:
 *          UI8_T   *username   --  name of user of public key will be delete.
 *          UI32_T  key_type    --  Type of host key.(KEY_TYPE_RSA, KEY_TYPE_DSA and KEY_TYPE_BOTH_RSA_AND_DSA)
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_MGR_DeleteUserPublicKey(UI8_T *username, UI32_T key_type)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  result;

    /* BODY */
    if(username == NULL)
    {
        return FALSE;
    }
    result = KEYGEN_OM_DeleteUserPublicKey(username, key_type);

    if( result == TRUE )
    {
        result = KEYGEN_SetNewCertificateToFlash();
    }

    return result;
}

/* FUNCTION NAME : KEYGEN_MGR_GetUserPublicKeyType
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
 *      This API is invoked in SSHD_MGR_GetUserPublicKeyType()
 */
BOOL_T KEYGEN_MGR_GetUserPublicKeyType(UI8_T *username, UI32_T *key_type)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  result;

    /* BODY */
    result = KEYGEN_OM_GetUserPublicKeyType(username, key_type);

    return result;
}

/* FUNCTION NAME : KEYGEN_MGR_GetUserPublicKey
 * PURPOSE:
 *      Get user's public key.
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
 *      This API is invoked in SSHD_MGR_GetUserPublicKey()
 */
BOOL_T KEYGEN_MGR_GetUserPublicKey(UI8_T *username, UI8_T *rsa_key, UI8_T *dsa_key)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  result;

    /* BODY */
    if((username == NULL) || (rsa_key == NULL) || (dsa_key == NULL))
    {
        return FALSE;
    }
    result = KEYGEN_OM_GetUserPublicKey(username, rsa_key, dsa_key);

    return result;
}

/* FUNCTION NAME : KEYGEN_MGR_GetNextUserPublicKey
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
 *      This API is invoked in SSHD_MGR_GetNextUserPublicKey()
 *      Initiation username is empty string.
 */
BOOL_T KEYGEN_MGR_GetNextUserPublicKey(UI8_T *username, UI8_T *rsa_key, UI8_T *dsa_key)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  result;

    /* BODY */
    if((username == NULL) || (rsa_key == NULL) || (dsa_key == NULL))
    {
        return FALSE;
    }
    result = KEYGEN_OM_GetNextUserPublicKey(username, rsa_key, dsa_key);

    return result;
}

/* FUNCTION NAME : KEYGEN_MGR_AsyncGenerateHostKeyPair
 * PURPOSE:
 *      Generate RSA/DSA public and private key pair of host key.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in SSHD_MGR_AsyncGenerateHostKeyPair().
 */
BOOL_T KEYGEN_MGR_AsyncGenerateHostKeyPair(void)
{
    UI32_T  key_type, action_type;
    BOOL_T  result;

    SSHD_OM_GetGenerateHostKeyAction(&action_type);

    switch( action_type )
    {
    case GENERATE_RSA_KEY:
        key_type = KEY_TYPE_RSA;
        break;
    case GENERATE_DSA_KEY:
        key_type = KEY_TYPE_DSA;
        break;
    case GENERATE_RSA_AND_DSA_KEY:
        key_type = KEY_TYPE_BOTH_RSA_AND_DSA;
        break;
    default:
        return FALSE;
    }

	result = KEYGEN_MGR_GenerateHostKeyPair(key_type);

    if( result == TRUE )
    {
        SSHD_MGR_SetGenerateHostKeyStatus(SUCCESS_STATE);
        SSHD_MGR_SetGenerateHostKeyAction(NON_GENERATE_KEY);
    }
    else
    {
        SSHD_MGR_SetGenerateHostKeyStatus(FAILURE_STATE);
        SSHD_MGR_SetGenerateHostKeyAction(NON_GENERATE_KEY);
    }

    return result;
}

/* FUNCTION NAME : KEYGEN_MGR_AsyncDeleteUserPublicKey
 * PURPOSE:
 *      Delete; RSA/DSA public key of user key.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in SSHD_MGR_AsyncDeleteUserPublicKey().
 */
BOOL_T KEYGEN_MGR_AsyncDeleteUserPublicKey(UI8_T *username)
{
    UI32_T  key_type, action_type;
    BOOL_T  result;

    if(username == NULL)
    {
        return FALSE;
    }

    SSHD_OM_GetDeleteUserPublicKeyAction(&action_type);

    switch( action_type )
    {
    case DELETE_RSA_KEY:
        key_type = KEY_TYPE_RSA;
        break;
    case DELETE_DSA_KEY:
        key_type = KEY_TYPE_DSA;
        break;
    case DELETE_RSA_AND_DSA_KEY:
        key_type = KEY_TYPE_BOTH_RSA_AND_DSA;
        break;
    default:
        return FALSE;
    }

    result = KEYGEN_MGR_DeleteUserPublicKey(username, key_type);
    if( result == TRUE )
    {
        SSHD_MGR_SetDeleteUserPublicKeyStatus(SUCCESS_STATE);
        SSHD_MGR_SetDeleteUserPublicKeyAction(NON_DELETE_KEY);
    }
    else
    {
        SSHD_MGR_SetDeleteUserPublicKeyStatus(FAILURE_STATE);
        SSHD_MGR_SetDeleteUserPublicKeyAction(NON_DELETE_KEY);
    }
    free(username);
    return result;
}

/* FUNCTION NAME : KEYGEN_MGR_AsyncWriteHostKey2Flash
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
 *      This API is invoked in SSHD_MGR_AsyncWriteHostKey2Flash.
 */
BOOL_T KEYGEN_MGR_AsyncWriteHostKey2Flash()
{
    BOOL_T  result;

    result = KEYGEN_MGR_WriteHostKey2Flash();
    if( result == TRUE )
    {
        SSHD_MGR_SetWriteHostKey2FlashStatus(SUCCESS_STATE);
        SSHD_MGR_SetWriteHostKey2FlashAction(NON_SAVE);
    }
    else
    {
        SSHD_MGR_SetWriteHostKey2FlashStatus(FAILURE_STATE);
        SSHD_MGR_SetWriteHostKey2FlashAction(NON_SAVE);
    }
    return result;
}

/* FUNCTION NAME : KEYGEN_MGR_GetHostPublicKeyFingerPrint
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
 *      This API is invoked in UI.
 */
BOOL_T KEYGEN_MGR_GetHostPublicKeyFingerPrint(UI8_T *rsa_sha1_fingerprint, UI8_T *rsa_md5_fingerprint, UI8_T *dsa_sha1_fingerprint, UI8_T *dsa_md5_fingerprint)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;

    /* BODY */
    if( (rsa_sha1_fingerprint == NULL) || (rsa_md5_fingerprint == NULL) || (dsa_sha1_fingerprint == NULL) || (dsa_md5_fingerprint == NULL) )
    {
        return FALSE;
    }

    ret = KEYGEN_OM_GetHostPublicKeyFingerPrint(rsa_sha1_fingerprint, rsa_md5_fingerprint, dsa_sha1_fingerprint, dsa_md5_fingerprint);
    return ret;
}

/* FUNCTION NAME : KEYGEN_MGR_SetRandomString
 * PURPOSE:
 *      This function provide user input any random string to improve random seed for
 *      generate host key. This function can accept any length srting.
 *
 * INPUT:
 *      UI8_T   *random_string  --  random string.
 *      UI32_T  string_length   --  length of random string.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in UI, if you want to improve random seeed for generate host key.
 */
BOOL_T KEYGEN_MGR_SetRandomString(UI8_T *random_string, UI32_T string_length)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    if( (random_string == NULL) || string_length == 0 )
    {
        return FALSE;
    }
    RAND_seed(random_string, string_length);
    return TRUE;
}

/* FUNCTION NAME - KEYGEN_MGR_HandleHotInsertion
 *
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is inserted at a time.
 */
void KEYGEN_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    return;
}



/* FUNCTION NAME - KEYGEN_MGR_HandleHotRemoval
 *
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is removed at a time.
 */
void KEYGEN_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    return;
}



/* FUNCTION NAME:  KEYGEN_MGR_InitCertificateFiles
 * PURPOSE:
 *          Set default files to Files_Buffer and file system.
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
 *          none.
 */
BOOL_T KEYGEN_MGR_InitCertificateFiles()
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI32_T          my_unit_id;
    UI8_T           *offset;
    UI8_T           *file_buffer;

    /* BODY */
    file_buffer = (UI8_T *)L_MM_Malloc(WRITE_TO_FLASH_FILES_LENGTH, L_MM_USER_ID2(SYS_MODULE_KEYGEN, KEYGEN_TYPE_TRACE_ID_KEYGEN_MGR_INITCERTIFICATEFILES));
    if ( file_buffer == NULL )
    {
        return FALSE;
    }

    STKTPLG_POM_GetMyUnitID(&my_unit_id);

    memset(file_buffer,0,WRITE_TO_FLASH_FILES_LENGTH);
    offset = file_buffer;


    strncpy((char *)offset,CERTIFICATE_FILE, SERVER_CERTIFICATE_FILE_LENGTH);
    KEYGEN_OM_SetServerCertificate(offset);
    offset = offset + SERVER_CERTIFICATE_FILE_LENGTH;

    strncpy((char *)offset,PRIVATE_KEY_FILE, SERVER_PRIVATE_KEY_FILE_LENGTH);
    KEYGEN_OM_SetServerPrivateKey(offset);
    offset = offset + SERVER_PRIVATE_KEY_FILE_LENGTH;

    strncpy((char *)offset,PASS_PHRASE, SERVER_PASS_PHRASE_FILE_LENGTH);
    KEYGEN_OM_SetServerPassPhrase(offset);

#if( SYS_CPNT_SSH2 == TRUE )
    offset = offset + SERVER_PASS_PHRASE_FILE_LENGTH;
    memset(offset, 0, HOST_RSA_KEY_1024B_FILE_LENGTH);
    offset = offset + HOST_RSA_KEY_1024B_FILE_LENGTH;
    memset(offset, 0, HOST_DSA_KEY_1024B_FILE_LENGTH);

    offset = offset + HOST_DSA_KEY_1024B_FILE_LENGTH;
    memset(offset, 0, USER_RSA_PUBLIC_KEY_FILE_LENGTH);
    offset = offset + USER_RSA_PUBLIC_KEY_FILE_LENGTH;
    memset(offset, 0, USER_DSA_PUBLIC_KEY_FILE_LENGTH);

    offset = offset + USER_DSA_PUBLIC_KEY_FILE_LENGTH;
    memset(offset, 0, HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH);
    offset = offset + HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH;
    memset(offset, 0, HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH);
    offset = offset + HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH;
    memset(offset, 0, HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH);
    offset = offset + HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH;
    memset(offset, 0, HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH);
#endif


    if( FS_WriteFile(my_unit_id,(UI8_T *)CERTIFICATE_FILENAME,(UI8_T *)"certificate",FS_FILE_TYPE_CERTIFICATE,file_buffer,WRITE_TO_FLASH_FILES_LENGTH,WRITE_TO_FLASH_FILES_LENGTH) != FS_RETURN_OK )
    {
        FS_DeleteFile(my_unit_id, (UI8_T *)CERTIFICATE_FILENAME);
    }

    memset(file_buffer,0,WRITE_TO_FLASH_FILES_LENGTH);

    strcpy((char *)file_buffer,(char *)PKEY_512B_1);
    KEYGEN_OM_SetTempPublicKeyPair(file_buffer,512,1);
    strcpy((char *)file_buffer,(char *)PKEY_512B_2);
    KEYGEN_OM_SetTempPublicKeyPair(file_buffer,512,2);
    strcpy((char *)file_buffer,(char *)PKEY_512B_3);
    KEYGEN_OM_SetTempPublicKeyPair(file_buffer,512,3);
    strcpy((char *)file_buffer,(char *)PKEY_512B_4);
    KEYGEN_OM_SetTempPublicKeyPair(file_buffer,512,4);
    strcpy((char *)file_buffer,(char *)PKEY_768B_1);
    KEYGEN_OM_SetTempPublicKeyPair(file_buffer,768,1);
    strcpy((char *)file_buffer,(char *)PKEY_768B_2);
    KEYGEN_OM_SetTempPublicKeyPair(file_buffer,768,2);
    strcpy((char *)file_buffer,(char *)PKEY_768B_3);
    KEYGEN_OM_SetTempPublicKeyPair(file_buffer,768,3);
    strcpy((char *)file_buffer,(char *)PKEY_768B_4);
    KEYGEN_OM_SetTempPublicKeyPair(file_buffer,768,4);
    strcpy((char *)file_buffer,(char *)PKEY_1024B_1);
    KEYGEN_OM_SetTempPublicKeyPair(file_buffer,1024,1);
    strcpy((char *)file_buffer,(char *)PKEY_1024B_2);
    KEYGEN_OM_SetTempPublicKeyPair(file_buffer,1024,2);
    strcpy((char *)file_buffer,(char *)PKEY_1024B_3);
    KEYGEN_OM_SetTempPublicKeyPair(file_buffer,1024,3);
    strcpy((char *)file_buffer,(char *)PKEY_1024B_4);
    KEYGEN_OM_SetTempPublicKeyPair(file_buffer,1024,4);


    L_MM_Free(file_buffer);

    return TRUE;
}




/* LOCAL SUBPROGRAM BODIES
 */

#if 0
/* FUNCTION NAME:  KEYGEN_InitCertificateFiles
 * PURPOSE:
 *          Set default files to Files_Buffer and file system.
 *
 * INPUT:
 *          UI32_T  my_unit_id  --  my unit id or blade number.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          Check if there is alrady allocated space existed. If not, then call this API to allocate one.
 */
static BOOL_T KEYGEN_InitCertificateFiles(UI32_T my_unit_id)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI8_T   *offset;
    UI8_T   *file_buffer;

    /* BODY */
    if ( L_MEM_MAX_SIZE >= FILES_LENGTH )
    {
        file_buffer = L_MEM_Allocate(FILES_LENGTH);
        if ( file_buffer == NULL )
        {
            return FALSE;
        }
    }
    else
    {
        file_buffer = (UI8_T *)malloc(FILES_LENGTH);
        if ( file_buffer == NULL )
        {
            return FALSE;
        }
    }

    KEYGEN_OM_EnterCriticalSection();
    memset(file_buffer,0,FILES_LENGTH);
    offset = file_buffer;

    strcpy(offset,CERTIFICATE_FILE);
    KEYGEN_OM_SetServerCertificate(offset);
    offset = offset + SERVER_CERTIFICATE_FILE_LENGTH;

    strcpy(offset,PRIVATE_KEY_FILE);
    KEYGEN_OM_SetServerPrivateKey(offset);
    offset = offset + SERVER_PRIVATE_KEY_FILE_LENGTH;

    strcpy(offset,PASS_PHRASE);
    KEYGEN_OM_SetServerPassPhrase(offset);
    offset = offset + SERVER_PASS_PHRASE_FILE_LENGTH;

    strcpy(offset,PKEY_512B_1);
    KEYGEN_OM_SetTempPublicKeyPair(offset,512,1);
    offset = offset + TEMP_RSA_KEY_512B_FILE_LENGTH;

    strcpy(offset,PKEY_512B_2);
    KEYGEN_OM_SetTempPublicKeyPair(offset,512,2);
    offset = offset + TEMP_RSA_KEY_512B_FILE_LENGTH;

    strcpy(offset,PKEY_512B_3);
    KEYGEN_OM_SetTempPublicKeyPair(offset,512,3);
    offset = offset + TEMP_RSA_KEY_512B_FILE_LENGTH;

    strcpy(offset,PKEY_512B_4);
    KEYGEN_OM_SetTempPublicKeyPair(offset,512,4);
    offset = offset + TEMP_RSA_KEY_512B_FILE_LENGTH;

    strcpy(offset,PKEY_768B_1);
    KEYGEN_OM_SetTempPublicKeyPair(offset,768,1);
    offset = offset + TEMP_RSA_KEY_768B_FILE_LENGTH;

    strcpy(offset,PKEY_768B_2);
    KEYGEN_OM_SetTempPublicKeyPair(offset,768,2);
    offset = offset + TEMP_RSA_KEY_768B_FILE_LENGTH;

    strcpy(offset,PKEY_768B_3);
    KEYGEN_OM_SetTempPublicKeyPair(offset,768,3);
    offset = offset + TEMP_RSA_KEY_768B_FILE_LENGTH;

    strcpy(offset,PKEY_768B_4);
    KEYGEN_OM_SetTempPublicKeyPair(offset,768,4);
    offset = offset + TEMP_RSA_KEY_768B_FILE_LENGTH;

    strcpy(offset,PKEY_1024B_1);
    KEYGEN_OM_SetTempPublicKeyPair(offset,1024,1);
    offset = offset + TEMP_RSA_KEY_1024B_FILE_LENGTH;

    strcpy(offset,PKEY_1024B_2);
    KEYGEN_OM_SetTempPublicKeyPair(offset,1024,2);
    offset = offset + TEMP_RSA_KEY_1024B_FILE_LENGTH;

    strcpy(offset,PKEY_1024B_3);
    KEYGEN_OM_SetTempPublicKeyPair(offset,1024,3);
    offset = offset + TEMP_RSA_KEY_1024B_FILE_LENGTH;

    strcpy(offset,PKEY_1024B_4);
    KEYGEN_OM_SetTempPublicKeyPair(offset,1024,4);

    KEYGEN_OM_LeaveCriticalSection();

    if ( FS_WriteFile(my_unit_id,CERTIFICATE_FILENAME,"",FS_FILE_TYPE_PRIVATE,file_buffer,FILES_LENGTH,FILES_LENGTH) != FS_RETURN_OK )
    {
        if ( L_MEM_MAX_SIZE >= FILES_LENGTH )
        {
            L_MEM_Free(file_buffer);
        }
        else
        {
            free(file_buffer);
        }
        return FALSE;
    }

    if ( L_MEM_MAX_SIZE >= FILES_LENGTH )
    {
        L_MEM_Free(file_buffer);
    }
    else
    {
        free(file_buffer);
    }

    return TRUE;
}



/* FUNCTION NAME:  KEYGEN_DuplicateCertificateFiles
 * PURPOSE:
 *          This function get server certificate,private key,pass phrase,
 *          and temp RSA key from file system.
 *          And set them to OM respectively
 *
 * INPUT:
 *          UI32_T  my_unit_id  --  my unit id or blade number.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
static BOOL_T KEYGEN_DuplicateCertificateFiles(UI32_T my_unit_id)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI8_T   *buffer;
    UI8_T   *offset;
    UI32_T  i,len;

    /* BODY */
    if ( L_MEM_MAX_SIZE >= FILES_LENGTH )
    {
        buffer = L_MEM_Allocate(FILES_LENGTH);
        if ( buffer == NULL )
        {
            return FALSE;
        }
    }
    else
    {
        buffer = (UI8_T *)malloc(FILES_LENGTH);
        if ( buffer == NULL )
        {
            return FALSE;
        }
    }

    if ( FS_ReadFile(my_unit_id,CERTIFICATE_FILENAME,buffer,FILES_LENGTH,&len) != FS_RETURN_OK )
    {
        if ( L_MEM_MAX_SIZE >= FILES_LENGTH )
        {
            L_MEM_Free(buffer);
        }
        else
        {
            free(buffer);
        }
        return FALSE;
    }


    KEYGEN_OM_EnterCriticalSection();

    offset = buffer;

    KEYGEN_OM_SetServerCertificate(offset);
    offset = offset + SERVER_CERTIFICATE_FILE_LENGTH;

    KEYGEN_OM_SetServerPrivateKey(offset);
    offset = offset + SERVER_PRIVATE_KEY_FILE_LENGTH;

    KEYGEN_OM_SetServerPassPhrase(offset);
    offset = offset + SERVER_PASS_PHRASE_FILE_LENGTH;

    for ( i=0 ; i<NUMBER_OF_TEMP_KEY ; i++ )
    {
        KEYGEN_OM_SetTempPublicKeyPair(offset,512,i+1);
        offset = offset + TEMP_RSA_KEY_512B_FILE_LENGTH;
    }

    for ( i=0 ; i<NUMBER_OF_TEMP_KEY ; i++ )
    {
        KEYGEN_OM_SetTempPublicKeyPair(offset,768,i+1);
        offset = offset + TEMP_RSA_KEY_768B_FILE_LENGTH;
    }

    for ( i=0 ; i<NUMBER_OF_TEMP_KEY-1 ; i++ )
    {
        KEYGEN_OM_SetTempPublicKeyPair(offset,1024,i+1);
        offset = offset + TEMP_RSA_KEY_1024B_FILE_LENGTH;
    }

    KEYGEN_OM_SetTempPublicKeyPair(offset,1024,i+1);

    KEYGEN_OM_LeaveCriticalSection();

    if ( L_MEM_MAX_SIZE >= FILES_LENGTH )
    {
        L_MEM_Free(buffer);
    }
    else
    {
        free(buffer);
    }

    return TRUE;

}
#endif



/* FUNCTION NAME:  KEYGEN_LoadCertificateFiles
 * PURPOSE:
 *          This function get server certificate,private key,pass phrase,from file system.
 *          and temp RSA key from default value.
 *          And set them to OM respectively.
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
 *          If certificate don't exist in file system, get all of them from default value.
 */
static BOOL_T KEYGEN_LoadCertificateFiles()
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    FS_File_Attr_T  file_attr;
    UI32_T          my_unit_id;
    UI8_T           *offset;
    UI8_T           *file_buffer;
    UI32_T          len;

#if( SYS_CPNT_SSH2 == TRUE )
    UI8_T           *om_file_buffer;
#endif  /* #if( SYS_CPNT_SSH2 == TRUE ) */

    /* BODY */
    file_buffer = (UI8_T *)L_MM_Malloc(WRITE_TO_FLASH_FILES_LENGTH, L_MM_USER_ID2(SYS_MODULE_KEYGEN, KEYGEN_TYPE_TRACE_ID_KEYGEN_LOADCERTIFICATEFILES));
    if ( file_buffer == NULL )
    {
        return FALSE;
    }

    STKTPLG_POM_GetMyUnitID(&my_unit_id);
    memset(&file_attr,0,sizeof(FS_File_Attr_T));
    strncpy((char *)file_attr.file_name,CERTIFICATE_FILENAME, SYS_ADPT_FILE_SYSTEM_NAME_LEN);
    file_attr.file_type_mask = FS_FILE_TYPE_MASK(FS_FILE_TYPE_CERTIFICATE);
    FS_GetFileInfo(my_unit_id,&file_attr);
    if ( (file_attr.create_time == 0) || (file_attr.file_size != WRITE_TO_FLASH_FILES_LENGTH) )
    {
        memset(file_buffer,0,WRITE_TO_FLASH_FILES_LENGTH);
        offset = file_buffer;

        strncpy((char *)offset,CERTIFICATE_FILE, SERVER_CERTIFICATE_FILE_LENGTH);
        KEYGEN_OM_SetServerCertificate(offset);
        offset = offset + SERVER_CERTIFICATE_FILE_LENGTH;

        strncpy((char *)offset,PRIVATE_KEY_FILE, SERVER_PRIVATE_KEY_FILE_LENGTH);
        KEYGEN_OM_SetServerPrivateKey(offset);
        offset = offset + SERVER_PRIVATE_KEY_FILE_LENGTH;

        strncpy((char *)offset,PASS_PHRASE, SERVER_PASS_PHRASE_FILE_LENGTH);
        KEYGEN_OM_SetServerPassPhrase(offset);

#if( SYS_CPNT_SSH2 == TRUE )
        offset = offset + SERVER_PASS_PHRASE_FILE_LENGTH;
        memset(offset, 0, HOST_RSA_KEY_1024B_FILE_LENGTH);
        offset = offset + HOST_RSA_KEY_1024B_FILE_LENGTH;
        memset(offset, 0, HOST_DSA_KEY_1024B_FILE_LENGTH);

        offset = offset + HOST_DSA_KEY_1024B_FILE_LENGTH;
        memset(offset, 0, USER_RSA_PUBLIC_KEY_FILE_LENGTH);
        offset = offset + USER_RSA_PUBLIC_KEY_FILE_LENGTH;
        memset(offset, 0, USER_DSA_PUBLIC_KEY_FILE_LENGTH);

        offset = offset + USER_DSA_PUBLIC_KEY_FILE_LENGTH;
        memset(offset, 0, HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH);
        offset = offset + HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH;
        memset(offset, 0, HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH);
        offset = offset + HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH;
        memset(offset, 0, HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH);
        offset = offset + HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH;
        memset(offset, 0, HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH);
#endif


        FS_WriteFile(my_unit_id,(UI8_T *)CERTIFICATE_FILENAME,(UI8_T *)"certificate",FS_FILE_TYPE_CERTIFICATE,file_buffer,WRITE_TO_FLASH_FILES_LENGTH,WRITE_TO_FLASH_FILES_LENGTH);
    }
    else /* certificate exist in file system */
    {
        if ( FS_ReadFile(my_unit_id,(UI8_T *)CERTIFICATE_FILENAME,file_buffer,WRITE_TO_FLASH_FILES_LENGTH,&len) != FS_RETURN_OK )
        {
            memset(file_buffer,0,WRITE_TO_FLASH_FILES_LENGTH);
            offset = file_buffer;

            strncpy((char *)offset,CERTIFICATE_FILE, SERVER_CERTIFICATE_FILE_LENGTH);
            KEYGEN_OM_SetServerCertificate(offset);
            offset = offset + SERVER_CERTIFICATE_FILE_LENGTH;

            strncpy((char *)offset,PRIVATE_KEY_FILE, SERVER_PRIVATE_KEY_FILE_LENGTH);
            KEYGEN_OM_SetServerPrivateKey(offset);
            offset = offset + SERVER_PRIVATE_KEY_FILE_LENGTH;

            strncpy((char *)offset,PASS_PHRASE, SERVER_PASS_PHRASE_FILE_LENGTH);
            KEYGEN_OM_SetServerPassPhrase(offset);

#if( SYS_CPNT_SSH2 == TRUE )
            offset = offset + SERVER_PASS_PHRASE_FILE_LENGTH;
            memset(offset, 0, HOST_RSA_KEY_1024B_FILE_LENGTH);
            offset = offset + HOST_RSA_KEY_1024B_FILE_LENGTH;
            memset(offset, 0, HOST_DSA_KEY_1024B_FILE_LENGTH);

            offset = offset + HOST_DSA_KEY_1024B_FILE_LENGTH;
            memset(offset, 0, USER_RSA_PUBLIC_KEY_FILE_LENGTH);
            offset = offset + USER_RSA_PUBLIC_KEY_FILE_LENGTH;
            memset(offset, 0, USER_DSA_PUBLIC_KEY_FILE_LENGTH);

            offset = offset + USER_DSA_PUBLIC_KEY_FILE_LENGTH;
            memset(offset, 0, HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH);
            offset = offset + HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH;
            memset(offset, 0, HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH);
            offset = offset + HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH;
            memset(offset, 0, HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH);
            offset = offset + HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH;
            memset(offset, 0, HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH);
#endif

        }
        else
        {
#if( SYS_CPNT_SSH2 == TRUE )
            KEYGEN_OM_GetFilesBufferPointer(&om_file_buffer);
            memcpy(om_file_buffer, file_buffer, WRITE_TO_FLASH_FILES_LENGTH);
            offset = file_buffer;
            offset = offset + SERVER_CERTIFICATE_FILE_LENGTH
                            + SERVER_PRIVATE_KEY_FILE_LENGTH
                            + SERVER_PASS_PHRASE_FILE_LENGTH;
            KEYGEN_OM_SetHostRsaPublicKeyPair(offset);
            offset = offset + HOST_RSA_KEY_1024B_FILE_LENGTH;
            KEYGEN_OM_SetHostDsaPublicKeyPair(offset);

            offset = offset + HOST_DSA_KEY_1024B_FILE_LENGTH;
            KEYGEN_OM_SetHostRsaPublicKey(offset);
            offset = offset + USER_RSA_PUBLIC_KEY_FILE_LENGTH;
            KEYGEN_OM_SetHostDsaPublicKey(offset);

            offset = offset + USER_DSA_PUBLIC_KEY_FILE_LENGTH;
            KEYGEN_OM_SetHostRsaPublicKeyFingerPrint(offset);
            offset = offset + HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH
                            + HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH;
            KEYGEN_OM_SetHostDsaPublicKeyFingerPrint(offset);
#else
            offset = file_buffer;

            KEYGEN_OM_SetServerCertificate(offset);
            offset = offset + SERVER_CERTIFICATE_FILE_LENGTH;

            KEYGEN_OM_SetServerPrivateKey(offset);
            offset = offset + SERVER_PRIVATE_KEY_FILE_LENGTH;

            KEYGEN_OM_SetServerPassPhrase(offset);
#endif

        }
    }

    memset(file_buffer,0,WRITE_TO_FLASH_FILES_LENGTH);
    strcpy((char *)file_buffer,PKEY_512B_1);
    KEYGEN_OM_SetTempPublicKeyPair(file_buffer,512,1);
    strcpy((char *)file_buffer,PKEY_512B_2);
    KEYGEN_OM_SetTempPublicKeyPair(file_buffer,512,2);
    strcpy((char *)file_buffer,PKEY_512B_3);
    KEYGEN_OM_SetTempPublicKeyPair(file_buffer,512,3);
    strcpy((char *)file_buffer,PKEY_512B_4);
    KEYGEN_OM_SetTempPublicKeyPair(file_buffer,512,4);
    strcpy((char *)file_buffer,PKEY_768B_1);
    KEYGEN_OM_SetTempPublicKeyPair(file_buffer,768,1);
    strcpy((char *)file_buffer,PKEY_768B_2);
    KEYGEN_OM_SetTempPublicKeyPair(file_buffer,768,2);
    strcpy((char *)file_buffer,PKEY_768B_3);
    KEYGEN_OM_SetTempPublicKeyPair(file_buffer,768,3);
    strcpy((char *)file_buffer,PKEY_768B_4);
    KEYGEN_OM_SetTempPublicKeyPair(file_buffer,768,4);
    strcpy((char *)file_buffer,PKEY_1024B_1);
    KEYGEN_OM_SetTempPublicKeyPair(file_buffer,1024,1);
    strcpy((char *)file_buffer,PKEY_1024B_2);
    KEYGEN_OM_SetTempPublicKeyPair(file_buffer,1024,2);
    strcpy((char *)file_buffer,PKEY_1024B_3);
    KEYGEN_OM_SetTempPublicKeyPair(file_buffer,1024,3);
    strcpy((char *)file_buffer,PKEY_1024B_4);
    KEYGEN_OM_SetTempPublicKeyPair(file_buffer,1024,4);

    L_MM_Free(file_buffer);

    return TRUE;
}



/* FUNCTION NAME:  KEYGEN_SeedPRNG
 * PURPOSE:
 *          Support for better seeding of SSL library's RNG
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          UI32_T - Number of bytes for seeding PRNG.
 * NOTES:
 *          This function is invoked in KEYGEN_MGR_GenTempPublicKeyPair
 */
static UI32_T KEYGEN_SeedPRNG()
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI32_T  nDone =0;
    unsigned long t;
    UI32_T  task_id, n, l;
    UI8_T   stackdata[256];

    /* BODY */
    /*
    * seed in the current time (usually just 4 bytes)
    */
    t = time(NULL);
    l = sizeof(unsigned long);
    RAND_seed((UI8_T *)&t, l);
    nDone += l;

    /*
    * seed in the current process id (usually just 4 bytes)
    */
    task_id = SYSFUN_TaskIdSelf();//getpid();
    l = sizeof(UI32_T);
    RAND_seed((UI8_T *)&task_id, l);
    nDone += l;

    /*
    * seed in some current state of the run-time stack (128 bytes)
    */
    KEYGEN_ChooseNum(0, sizeof(stackdata)-128-1, &n);
    RAND_seed(stackdata+n, 128);
    nDone += 128;
    return nDone;
}



/* FUNCTION NAME:  KEYGEN_ChooseNum
 * PURPOSE:
 *          Choose number between input parameter 1 and parameter 2.
 *
 * INPUT:
 *          UI32_T  -- Lowest number .
 *
 *          UI32_T  -- Highest number.
 *
 * OUTPUT:
 *          UI32_T * - The number between lowest and highest.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
static BOOL_T KEYGEN_ChooseNum(UI32_T l, UI32_T h, UI32_T *choose)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI32_T i,t;
    double d;

    /* BODY */
/*isiah.2004-01-08*/
    t = time(NULL);
    srand(t);
//    srand((UI32_T)time(NULL));
    d = ((double)(rand()%RAND_MAX)/RAND_MAX)*(h-l);
    i = (UI32_T)d+1;
    if (i < l)
    {
        i = l;
    }
    if (i > h)
    {
        i = h;
    }
    *choose = i;
    return TRUE;
}

/* FUNCTION NAME:  KEYGEN_SetNewCertificateToFlash
 * PURPOSE:
 *          Store new certificate file to file system.
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
 *          This function is invoked in KEYGEN_MGR_SetNewCertificate
 */
static BOOL_T KEYGEN_SetNewCertificateToFlash()
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI8_T           *file_buffer;
    UI32_T          my_unit_id;

    /* BODY */
    STKTPLG_POM_GetMyUnitID(&my_unit_id);


    KEYGEN_OM_GetFilesBufferPointer(&file_buffer);

    /* ES3550C 1-08-00349
       keygen_mgr SYSFUN_ENTER_Critical_Section too long
       should call KEYGEN_OM_LeaveCriticalSection() before FS_WriteFile()
    */


    if ( FS_WriteFile(my_unit_id,(UI8_T *)CERTIFICATE_FILENAME,(UI8_T *)"certificate",FS_FILE_TYPE_CERTIFICATE,file_buffer,WRITE_TO_FLASH_FILES_LENGTH,WRITE_TO_FLASH_FILES_LENGTH) != FS_RETURN_OK )
    {
        return FALSE;
    }

    return TRUE;
}

/* FUNCTION NAME:  KEYGEN_EVPTempPrivateKey
 * PURPOSE:
 *          Read temp private key into EVP_PKEY structure.
 *
 * INPUT:
 *          char    *file   --  temp private key
 *          char    *passwd --  password that pass phrase on an RSA temp private key.
 *
 * OUTPUT:
 *          EVP_PKEY    **pkey  --  temp private key
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
static BOOL_T KEYGEN_EVPTempPrivateKey(const char *file, char *passwd, EVP_PKEY **pkey)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BIO *in;
    EVP_PKEY *l_pkey = NULL;

    /* BODY */
    in=BIO_new_mem_buf((char *)file, -1);
    if (in == NULL)
    {
        printf("\r\nBIO_NEW error\r\n");
        return FALSE;
    }

    l_pkey=PEM_read_bio_PrivateKey(in,NULL,NULL,passwd);

    if (l_pkey == NULL)
    {
        printf("\r\nPEM_read_bio_PrivateKey error\r\n");
        BIO_free(in);
        return FALSE;
    }

    *pkey = l_pkey;

    BIO_free(in);

    return TRUE;
}

/* FUNCTION NAME : KEYGEN_MGR_GenerateRsaHostKeyPair
 * PURPOSE:
 *      Generate RSA public and private key pair of host key.
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
 *      This API is invoked in KEYGEN_MGR_GenerateHostKeyPair.
 */
static BOOL_T KEYGEN_MGR_GenerateRsaHostKeyPair()
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    EVP_PKEY            *pkey=NULL;
    BIO                 *out=NULL;
    const EVP_CIPHER          *cipher;
    UI32_T              i;
    BUF_MEM             *bm = NULL;
    UI32_T              key_bits;
    char                *buf = NULL;
    UI8_T               *rsa_key = NULL;
    UI32_T              bits;
    UI8_T               *rsa_sha1_fingerprint = NULL;
    UI8_T               *rsa_md5_fingerprint = NULL;
    RSA                 *private_ = NULL;
    BIGNUM              *f4 = NULL;
    BN_GENCB            cb;
    UI32_T              action;
    BOOL_T              ret = TRUE;

    /* BODY */
    KEYGEN_SeedPRNG();

    if ((out = BIO_new(BIO_s_mem())) == NULL)
    {
        goto err;
    }

    if ((bm = BUF_MEM_new()) == NULL)
    {
        goto err;
    }

    bm->data = (char *) OPENSSL_malloc(HOST_RSA_KEY_1024B_FILE_LENGTH+1);
    if (bm->data == NULL)
    {
        BUF_MEM_free(bm);
        bm = NULL;
        goto err;
    }

    bm->max = HOST_RSA_KEY_1024B_FILE_LENGTH+1;
    bm->length = 0;

    BIO_set_mem_buf(out, bm, 0);

    key_bits = 1024;

    if ((pkey=EVP_PKEY_new()) == NULL)
    {
        goto err;
    }

    if ((private_ = RSA_new()) == NULL)
    {
        goto err;
    }

    if ((f4 = BN_new()) == NULL)
    {
        goto err;
    }

    if (!BN_set_word(f4, RSA_F4))
    {
        goto err;
    }

    BN_GENCB_set(&cb, SSHD_MGR_GenerateKey_CallBack, NULL);

    if (!RSA_generate_key_ex(private_, key_bits, f4, &cb))
    {
        goto err;
    }

    if (!EVP_PKEY_assign_RSA(pkey, private_))
    {
        goto err;
    }

    private_ = NULL;

    cipher=EVP_des_ede3_cbc();

    i=0;

    while(TRUE)
    {
        if (!PEM_write_bio_PrivateKey(out,pkey,cipher,NULL,0,NULL,PKEY_PASSWD))
        {
            if ((ERR_GET_REASON(ERR_peek_error()) == PEM_R_PROBLEMS_GETTING_PASSWORD) && (i < 3))
            {
                ERR_clear_error();
                i++;
                continue;
            }

            goto err;
        }
        break;
    }

    rsa_key = (UI8_T *)L_MM_Malloc(USER_RSA_PUBLIC_KEY_FILE_LENGTH, L_MM_USER_ID2(SYS_MODULE_KEYGEN, KEYGEN_TYPE_TRACE_ID_KEYGEN_MGR_GENERATERSAHOSTKEYPAIR));
    if( rsa_key == NULL )
    {
        goto err;
    }
    bits = BN_num_bits(pkey->pkey.rsa->n);
    sprintf((char *)rsa_key, "%ld", (long)bits);
    buf = BN_bn2dec(pkey->pkey.rsa->e);
    strcat((char *)rsa_key, " ");
    strcat((char *)rsa_key, buf);
    OPENSSL_free(buf);
    buf = BN_bn2dec(pkey->pkey.rsa->n);
    strcat((char *)rsa_key, " ");
    strcat((char *)rsa_key, buf);
    OPENSSL_free(buf);

    rsa_sha1_fingerprint = (UI8_T *)L_MM_Malloc(HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH, L_MM_USER_ID2(SYS_MODULE_KEYGEN, KEYGEN_TYPE_TRACE_ID_KEYGEN_MGR_GENERATERSAHOSTKEYPAIR));
    if( rsa_sha1_fingerprint == NULL )
    {
        goto err;
    }
    rsa_md5_fingerprint = (UI8_T *)L_MM_Malloc(HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH, L_MM_USER_ID2(SYS_MODULE_KEYGEN, KEYGEN_TYPE_TRACE_ID_KEYGEN_MGR_GENERATERSAHOSTKEYPAIR));
    if( rsa_md5_fingerprint == NULL )
    {
        goto err;
    }
    KEYGEN_MGR_Rsa2FingerPrint(pkey->pkey.rsa, rsa_sha1_fingerprint, rsa_md5_fingerprint);

    SSHD_OM_GetGenerateHostKeyAction(&action);
    if( action == CANCEL_GENERATING_KEY )
    {
        goto err;
    }

    // TODO: modify 1st param of KEYGEN_OM_SetHostRsaPublicKeyPair2Memory to char *
    KEYGEN_OM_SetHostRsaPublicKeyPair2Memory(/*keyout*/ (UI8_T *)bm->data, rsa_key, rsa_sha1_fingerprint, rsa_md5_fingerprint);

    if (0)
    {
err:
        ret = FALSE;
    }

    if (rsa_sha1_fingerprint)
    {
        L_MM_Free(rsa_sha1_fingerprint);
        rsa_sha1_fingerprint = NULL;
    }

    if (rsa_md5_fingerprint)
    {
        L_MM_Free(rsa_md5_fingerprint);
        rsa_md5_fingerprint = NULL;
    }

    if (rsa_key)
    {
        L_MM_Free(rsa_key);
        rsa_key = NULL;
    }

    if (private_)
    {
        RSA_free(private_);
        private_ = NULL;
    }

    if (f4)
    {
        BN_free(f4);
        f4 = NULL;
    }

    /* Don't free bm, it had assign to out.
     */
    if (out)
    {
        BIO_set_close(out, 1);
        BIO_free_all(out);
        out = NULL;
    }

    if (pkey)
    {
        EVP_PKEY_free(pkey);
        pkey = NULL;
    }

    return ret;
}



/* FUNCTION NAME : KEYGEN_MGR_GenerateDsaHostKeyPair
 * PURPOSE:
 *      Generate DSA public and private key pair of host key.
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
 *      This API is invoked in KEYGEN_MGR_GenerateHostKeyPair.
 */
static BOOL_T KEYGEN_MGR_GenerateDsaHostKeyPair()
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    EVP_PKEY            *pkey=NULL;
    BIO                 *out=NULL;
    const EVP_CIPHER          *cipher;
    UI32_T              i;
    BUF_MEM             *bm = NULL;
    UI32_T              key_bits;
    DSA                 *private_ = NULL;
    UI8_T               *dsa_key = NULL;
    UI8_T               *dsa_sha1_fingerprint = NULL;
    UI8_T               *dsa_md5_fingerprint = NULL;
    UI32_T              action;
    BN_GENCB            cb;
    BOOL_T              ret = TRUE;

    /* BODY */
    KEYGEN_SeedPRNG();

    if ((out = BIO_new(BIO_s_mem())) == NULL)
    {
        goto err;
    }

    if ((bm = BUF_MEM_new()) == NULL)
    {
        goto err;
    }

    bm->data = (char *) OPENSSL_malloc(HOST_RSA_KEY_1024B_FILE_LENGTH+1);
    if (bm->data == NULL)
    {
        BUF_MEM_free(bm);
        bm = NULL;
        goto err;
    }

    bm->max = HOST_RSA_KEY_1024B_FILE_LENGTH+1;
    bm->length = 0;

    BIO_set_mem_buf(out, bm, 0);

    key_bits = 1024;

    if ((pkey=EVP_PKEY_new()) == NULL)
    {
        goto err;
    }

    if ((private_ = DSA_new()) == NULL)
    {
        goto err;
    }

    BN_GENCB_set(&cb, SSHD_MGR_GenerateKey_CallBack, NULL);

    if (!DSA_generate_parameters_ex(private_, key_bits, NULL, 0, NULL, NULL, &cb))
    {
        goto err;
    }

    if (!DSA_generate_key(private_))
    {
        goto err;
    }

    if (!EVP_PKEY_assign_DSA(pkey,private_))
    {
        goto err;
    }

    private_ = NULL;

    cipher=EVP_des_ede3_cbc();

    i=0;

    while(TRUE)
    {
        if (!PEM_write_bio_PrivateKey(out,pkey,cipher,NULL,0,NULL,PKEY_PASSWD))
        {
            if ((ERR_GET_REASON(ERR_peek_error()) == PEM_R_PROBLEMS_GETTING_PASSWORD) && (i < 3))
            {
                ERR_clear_error();
                i++;
                continue;
            }

            goto err;
        }
        break;
    }

    dsa_key = (UI8_T *)L_MM_Malloc(USER_DSA_PUBLIC_KEY_FILE_LENGTH, L_MM_USER_ID2(SYS_MODULE_KEYGEN, KEYGEN_TYPE_TRACE_ID_KEYGEN_MGR_GENERATEDSAHOSTKEYPAIR));
    if( dsa_key == NULL )
    {
        goto err;
    }

    KEYGEN_MGR_Dsa2SshDss(pkey->pkey.dsa, dsa_key);

    dsa_sha1_fingerprint = (UI8_T *)L_MM_Malloc(HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH, L_MM_USER_ID2(SYS_MODULE_KEYGEN, KEYGEN_TYPE_TRACE_ID_KEYGEN_MGR_GENERATEDSAHOSTKEYPAIR));
    if( dsa_sha1_fingerprint == NULL )
    {
        goto err;
    }
    dsa_md5_fingerprint = (UI8_T *)L_MM_Malloc(HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH, L_MM_USER_ID2(SYS_MODULE_KEYGEN, KEYGEN_TYPE_TRACE_ID_KEYGEN_MGR_GENERATEDSAHOSTKEYPAIR));
    if( dsa_md5_fingerprint == NULL )
    {
        goto err;
    }
    KEYGEN_MGR_Dsa2FingerPrint(pkey->pkey.dsa, dsa_sha1_fingerprint, dsa_md5_fingerprint);

    SSHD_OM_GetGenerateHostKeyAction(&action);
    if( action == CANCEL_GENERATING_KEY )
    {
        goto err;
    }

    // TODO: modify 1st param of KEYGEN_OM_SetHostDsaPublicKeyPair2Memory to char *
    KEYGEN_OM_SetHostDsaPublicKeyPair2Memory(/*keyout*/ (UI8_T *)bm->data, dsa_key, dsa_sha1_fingerprint, dsa_md5_fingerprint);

    if (0)
    {
err:
        ret = FALSE;
    }

    if (dsa_sha1_fingerprint)
    {
        L_MM_Free(dsa_sha1_fingerprint);
        dsa_sha1_fingerprint = NULL;
    }

    if (dsa_md5_fingerprint)
    {
        L_MM_Free(dsa_md5_fingerprint);
        dsa_md5_fingerprint = NULL;
    }

    if (dsa_key)
    {
        L_MM_Free(dsa_key);
        dsa_key = NULL;
    }

    if (private_)
    {
        DSA_free(private_);
        private_ = NULL;
    }

    /* Don't free bm, it had assign to out.
     */
    if (out)
    {
        BIO_set_close(out, 1);
        BIO_free_all(out);
        out = NULL;
    }

    if (pkey)
    {
        EVP_PKEY_free(pkey);
        pkey = NULL;
    }

    return ret;
}

/* FUNCTION NAME : KEYGEN_MGR_Dsa2SshDss
 * PURPOSE:
 *      Convert dsa key to ssh-dss format.
 *
 * INPUT:
 *      void    *dsa        --  pointer of dsa key.
 *
 * OUTPUT:
 *      UI8_T   *dsa_key    --  pointer of buffer to storage dsa public key.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Faulier.
 *
 * NOTES:
 *      This API is invoked in CLI command KEYGEN_MGR_GetHostPublicKey.
 *      This API is copy from SSHD_MGR_Dsa2SshDss() for code layer issue,
 *      and let buffer.c(.h), Bufaux.c, Uuencode.c, Base64.c be library
 */
BOOL_T KEYGEN_MGR_Dsa2SshDss(void *dsa, UI8_T *dsa_key)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    DSA *tmp_dsa;
    Buffer b;
    int len, n;
    char *uu;

    /* BODY */
    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        tmp_dsa = (DSA *) dsa;
        buffer_init(&b);
        buffer_put_cstring(&b, "ssh-dss");
        buffer_put_bignum2(&b, tmp_dsa->p);
        buffer_put_bignum2(&b, tmp_dsa->q);
        buffer_put_bignum2(&b, tmp_dsa->g);
        buffer_put_bignum2(&b, tmp_dsa->pub_key);
        len = buffer_len(&b);
        uu = (char *)malloc(2*len);
        n = uuencode(buffer_ptr(&b), len, uu, 2*len);
        if (n > 0)
        {
            strcpy((char *)dsa_key, "ssh-dss ");
            strcat((char *)dsa_key, uu);
            ret = TRUE;
        }
        else
        {
            ret = FALSE;
        }
        free(uu);
        memset(buffer_ptr(&b), 0, len);
        buffer_free(&b);
        return ret;
    }
}

/* FUNCTION NAME : KEYGEN_MGR_Dsa2FingerPrint
 * PURPOSE:
 *      Convert dsa key to sha1 and md5 fingerprint.
 *
 * INPUT:
 *      void    *dsa        --  pointer of dsa key.
 *
 * OUTPUT:
 *      UI8_T   *dsa_sha1_fingerprint   --  pointer of buffer to storage dsa sha1 fingerprint.
 *      UI8_T   *dsa_md5_fingerprint    --  pointer of buffer to storage dsa md5 fingerprint.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Faulier.
 *
 * NOTES:
 *      This API is invoked in CLI command KEYGEN_MGR_GetHostPublicKey.
 *      This API is copy from SSHD_MGR_Dsa2SshDss() for code layer issue,
 *      and let buffer.c(.h), Bufaux.c, Key.c
 */
BOOL_T KEYGEN_MGR_Dsa2FingerPrint(void *dsa, UI8_T *dsa_sha1_fingerprint, UI8_T *dsa_md5_fingerprint)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    DSA *tmp_dsa;
    Buffer b;
    int len;
    const EVP_MD *md = NULL;
    EVP_MD_CTX ctx;
    UI8_T *blob = NULL;
    UI8_T *dgst_raw = NULL;
    unsigned int dgst_raw_length = 0;
    char *retval;

    /* BODY */
    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        tmp_dsa = (DSA *) dsa;
        buffer_init(&b);
        buffer_put_cstring(&b, "ssh-dss");
        buffer_put_bignum2(&b, tmp_dsa->p);
        buffer_put_bignum2(&b, tmp_dsa->q);
        buffer_put_bignum2(&b, tmp_dsa->g);
        buffer_put_bignum2(&b, tmp_dsa->pub_key);
        len = buffer_len(&b);
        blob = malloc(len);
        if( blob == NULL )
        {
            buffer_free(&b);
            return FALSE;
        }
        memcpy(blob, buffer_ptr(&b), len);
        memset(buffer_ptr(&b), 0, len);
        buffer_free(&b);

        dgst_raw = malloc(EVP_MAX_MD_SIZE);
        if( dgst_raw == NULL )
        {
            free(blob);
            return FALSE;
        }

        md = EVP_sha1();
        EVP_DigestInit(&ctx, md);
        EVP_DigestUpdate(&ctx, blob, len);
        EVP_DigestFinal(&ctx, dgst_raw, &dgst_raw_length);
        retval = key_fingerprint_bubblebabble(dgst_raw, dgst_raw_length);
        strcpy((char *)dsa_sha1_fingerprint, retval);
        xfree(retval);

        memset(dgst_raw, 0, EVP_MAX_MD_SIZE);
        md = EVP_md5();
        EVP_DigestInit(&ctx, md);
        EVP_DigestUpdate(&ctx, blob, len);
        EVP_DigestFinal(&ctx, dgst_raw, &dgst_raw_length);
        retval = key_fingerprint_hex(dgst_raw, dgst_raw_length);
        strcpy((char *)dsa_md5_fingerprint, retval);
        xfree(retval);

        memset(blob, 0, len);
        free(blob);
        free(dgst_raw);
        return TRUE;
    }
    return TRUE;
}

/* FUNCTION NAME : KEYGEN_MGR_Rsa2FingerPrint
 * PURPOSE:
 *      Convert rsa key to sha1 and md5 fingerprint.
 *
 * INPUT:
 *      void    *rsa        --  pointer of rsa key.
 *
 * OUTPUT:
 *      UI8_T   *rsa_sha1_fingerprint   --  pointer of buffer to storage rsa sha1 fingerprint.
 *      UI8_T   *rsa_md5_fingerprint    --  pointer of buffer to storage rsa md5 fingerprint.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Faulier.
 *
 * NOTES:
 *      This API is invoked in CLI command KEYGEN_MGR_GetHostPublicKey.
 *      This API is copy from SSHD_MGR_Dsa2SshDss() for code layer issue,
 *      and let buffer.c(.h), Bufaux.c, Key.c
 */
BOOL_T KEYGEN_MGR_Rsa2FingerPrint(void *rsa, UI8_T *rsa_sha1_fingerprint, UI8_T *rsa_md5_fingerprint)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    RSA *tmp_rsa;
    int len;
    const EVP_MD *md = NULL;
    EVP_MD_CTX ctx;
    UI8_T *blob = NULL;
    UI8_T *dgst_raw = NULL;
    unsigned int dgst_raw_length = 0;
    int nlen, elen;
    UI8_T *retval;

    /* BODY */
    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        tmp_rsa = (RSA *) rsa;
        nlen = BN_num_bytes(tmp_rsa->n);
        elen = BN_num_bytes(tmp_rsa->e);
        len = nlen + elen;
        blob = malloc(len);
        if( blob == NULL )
        {
            return FALSE;
        }
        BN_bn2bin(tmp_rsa->n, blob);
        BN_bn2bin(tmp_rsa->e, blob + nlen);

        dgst_raw = malloc(EVP_MAX_MD_SIZE);
        if( dgst_raw == NULL )
        {
            free(blob);
            return FALSE;
        }

        md = EVP_sha1();
        EVP_DigestInit(&ctx, md);
        EVP_DigestUpdate(&ctx, blob, len);
        EVP_DigestFinal(&ctx, dgst_raw, &dgst_raw_length);
        retval = (UI8_T *)key_fingerprint_bubblebabble(dgst_raw, (UI32_T)dgst_raw_length);
        strcpy((char *)rsa_sha1_fingerprint, (char *)retval);
        xfree(retval);

        memset(dgst_raw, 0, EVP_MAX_MD_SIZE);
        md = EVP_md5();
        EVP_DigestInit(&ctx, md);
        EVP_DigestUpdate(&ctx, blob, len);
        EVP_DigestFinal(&ctx, dgst_raw, &dgst_raw_length);
        retval = (UI8_T *)key_fingerprint_hex(dgst_raw, (UI32_T)dgst_raw_length);
        strcpy((char *)rsa_md5_fingerprint, (char *)retval);
        xfree(retval);

        memset(blob, 0, len);
        free(blob);
        free(dgst_raw);
        return TRUE;
    }
    return TRUE;
}

/* FUNCTION NAME : KEYGEN_MGR_Async_Key_Job()
 * PURPOSE:
 *      For SSHD to nodify Keygen to key gen in Async type
 *
 * INPUT:
 *      void    *rsa        --  pointer of rsa key.
 *
 * OUTPUT:
 *      UI8_T   *rsa_sha1_fingerprint   --  pointer of buffer to storage rsa sha1 fingerprint.
 *      UI8_T   *rsa_md5_fingerprint    --  pointer of buffer to storage rsa md5 fingerprint.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Faulier.
 *
 * NOTES:
 *      This API is invoked in CLI command KEYGEN_MGR_GetHostPublicKey.
 *      This API is copy from SSHD_MGR_Dsa2SshDss() for code layer issue,
 *      and let buffer.c(.h), Bufaux.c, Key.c
 */
 #if 0
void KEYGEN_MGR_Async_Key_Job(UI32_T event_type, UI8_T *username,
                                                    void (*action_callbk)(UI32_T action)
                                                    void (*status_callbk)(UI32_T status))
{


}
#endif

static unsigned long KEYGEN_MGR_id_callback(void)
{
    return SYSFUN_TaskIdSelf();
}
