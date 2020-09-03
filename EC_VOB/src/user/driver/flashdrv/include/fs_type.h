/*-------------------------------------------------------------------------
 * Module Name: fs_type.h
 *-------------------------------------------------------------------------
 * Purpose    : This file defines the constants and data types for file
 *              System.
 *-------------------------------------------------------------------------
 * NOTES:
 *
 *-------------------------------------------------------------------------
 * HISTORY:
 *    11/23/2001 - Allen Cheng, Created
 *
 *-------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2001
 *-------------------------------------------------------------------------
 */


#ifndef _FS_TYPE_H_
#define _FS_TYPE_H_


#include "sys_type.h"
#include "l_mm.h"

#define FS_NULL_BLOCK_ID                    0xFFFFFFFF
#define FS_FILE_MAGIC_NUMBER                0x5AA5AA55
#define FS_SPECIAL_MAGIC_NUMBER             0x5A5A55AA
#define FS_STARTUP_BLOCK_MAGIC_STRING       "FILE SYSTEM STARTUP BLOCK"
#define FS_BAK_STARTUP_BLOCK_MAGIC_STRING   "FILE SYSTEM BAK STARTUP BLOCK"
                                            /* less than (SYS_ADPT_FILE_SYSTEM_NAME_LEN-1) characters */

#define FS_MAX_NUM_OF_RESERVE_BLOCK         8

/* for trace_id of user_id when allocate buffer with l_mm
 */
enum
{
    FS_TYPE_TRACE_ID_FS_COPYFILECONTENT = 0
};

enum FS_TYPE_EVENT_MASK_E
{
    FS_TYPE_EVENT_NONE                  =   0x0000L,
    FS_TYPE_EVENT_CHECKFS               =   0x0001L,
    FS_TYPE_EVENT_REMOTECALL            =   0x0002L,
    FS_TYPE_EVENT_REPLY                 =   0x0004L,
    FS_TYPE_EVENT_LEGACYFS              =   0x8000L,
    FS_TYPE_EVENT_ALL                   =   0xFFFFL
};

typedef enum FS_File_Type_E
{
    FS_FILE_TYPE_SUBFILE    = 0,
    FS_FILE_TYPE_KERNEL,
    FS_FILE_TYPE_DIAG,
    FS_FILE_TYPE_RUNTIME,
    FS_FILE_TYPE_SYSLOG,
    FS_FILE_TYPE_CMDLOG,
    FS_FILE_TYPE_CONFIG,
    FS_FILE_TYPE_POSTLOG,
    FS_FILE_TYPE_PRIVATE,
    FS_FILE_TYPE_CERTIFICATE,
    FS_FILE_TYPE_ARCHIVE,
    FS_FILE_TYPE_BINARY_CONFIG,
    FS_FILE_TYPE_PUBLIC,
#if (SYS_CPNT_EFM_OAM == TRUE)  
#if (SYS_CPNT_EFM_OAM_ORG_SPEC == TRUE)
#ifdef SYS_CPNT_EFM_OAM_ORG_SPEC_CPE_STYLE
#if (SYS_CPNT_EFM_OAM_ORG_SPEC_CPE_STYLE == SYS_CPNT_EFM_OAM_ORG_SPEC_ACCTON_CPE)
    FS_FILE_TYPE_CPEFIRMWARE,
    FS_FILE_TYPE_CPECONFIG,
#endif
#endif
#endif
#endif    
    FS_FILE_TYPE_TOTAL      /* Number of total file type */
} FS_File_Type_T;

enum FS_RETURN_CODE_E
{
    FS_RETURN_OK = 0,                   /* OK, Successful, Without any Error */
    FS_RETURN_FILE_TRUNCATED,           /* File truncated */
    FS_RETURN_PROTECTED_FILE,           /* Startup file */
    FS_RETURN_END_OF_FILE,              /* End of the File */
    FS_RETURN_ACTION_INHIBITED,         /* Action inhibited */
    FS_RETURN_ERROR,                    /* Error */
    FS_RETURN_NOT_READY,                /* Not Ready */
    FS_RETURN_BAD_PARA,                 /* Bad Parameter */
    FS_RETURN_INDEX_OOR,                /* Index Out Of Range */
    FS_RETURN_INDEX_NEX,                /* Index Not Existed */
    FS_RETURN_FILE_NEX,                 /* File Not Existed */
    FS_RETURN_NO_BUFFER,                /* No buffer */
    FS_RETURN_NO_ENTRY,                 /* No Entry */
    FS_RETURN_NO_BLOCK,                 /* No Block */
    FS_RETURN_BAD_HW,                   /* H/W Error */
    FS_RETURN_NOT_MASTER,               /* System is not a master */
    FS_RETURN_TIMEOUT,                  /* Requested service has timed out*/
    FS_RETURN_DISCONNECTED,             /* The connection is broken */
    FS_RETURN_ABORTED,                  /* Operation is aborted */
    FS_RETURN_DRIVE_NOT_EXIST,          /* Drive does not exist */
    FS_RETURN_FLASHING,                 /* Flashing programming in progress */
    FS_RETURN_PACKET_ERROR,             /* Packet error */
    FS_RETURN_INCORRECT_STATE,          /* FS is not in the right state */
    FS_RETURN_OTHERS,
    FS_RETURN_HAVEMOREDATA,             /* Transfer not coomplete, have more data */
    FS_RETURN_SERVICE_NOT_SUPPORT,      /* Remote service not support */
    FS_RETURN_EXCEED_MAX_NUM_OF_FILES   /* Total files of the given type is exceeded */
};

enum FS_DEBUG_FLAG_E
{
    FS_DEBUG_FLAG_NONE      = 0x00000000L,
    FS_DEBUG_FLAG_ERRMSG    = 0x00000001L,
    FS_DEBUG_FLAG_DBGMSG    = 0x00000002L,
    FS_DEBUG_FLAG_DUMP_TXPKT= 0x00000004L,
    FS_DEBUG_FLAG_DUMP_RXPKT= 0x00000008L,
    FS_DEBUG_FLAG_ALL       = 0xFFFFFFFFL
};

/* The message length must be 16 bytes */
typedef struct
{
    UI16_T              msg_type;       /* 2 bytes */
    UI16_T              unit;           /* 2 bytes */
    L_MM_Mref_Handle_T  *mref_handle_p; /* 4 bytes */
    UI32_T              pad1;           /* 4 bytes */
    UI32_T              pad2;           /* 4 bytes */
} FS_TYPE_MSG_T;                /* 16 bytes in total */

#endif /* _FS_TYPE_H_ */
