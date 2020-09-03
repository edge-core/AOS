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
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "l_mm.h"
#include "sysfun.h"

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
    FS_TYPE_TRACE_ID_FS_COPYFILECONTENT = 0,
    FS_TYPE_TRACE_ID_FS_GETFAILSAFEDATANEXTWRITEINFO,
    FS_TYPE_TRACE_ID_FS_GETFAILSAFEDATAINFO
};

enum FS_TYPE_EVENT_MASK_E
{
    FS_TYPE_EVENT_NONE                  =   0x0000L,
    FS_TYPE_EVENT_CHECKFS               =   0x0001L,
    FS_TYPE_EVENT_REMOTECALL            =   0x0002L,
    FS_TYPE_EVENT_REPLY                 =   0x0004L,
    FS_TYPE_EVENT_DUMMY_READRUNTIMEFILE =   0x0008L,
    FS_TYPE_EVENT_LEGACYFS              =   0x8000L,
    FS_TYPE_EVENT_ALL                   =   0xFFFFL | SYSFUN_SYSTEM_EVENT_SW_WATCHDOG
};

/* FS_File_Type_T is used by runtime and loader
 * It is known that some compiler option might effect the
 * data type of enum. To avoid inconsistent data type definition
 * between runtime and loader, it is better to define FS_File_Type_T as UI32 explicitly
 *
 * Note:
 *     1. Need to update the array "FS_MaxNumOfFile", "FS_FileTypeVisibility"
 *        and "fs_uid_type_to_dflt_mode_bitmap" defined in fs.c when there is a
 *        new file type being added to the FS_File_Type_E.
 *     2. Always add the new type to the existing last type for forward
 *        compatibility.
 *     3. In projects that does not support ONIE (i.e. SYS_CPNT_ONIE_SUPPORT is
 *        FALSE), the file with type FS_FILE_TYPE_RUNTIME is the file that can
 *        be booted by the loader.
 *     4. In projects that support ONIE (i.e. SYS_CPNT_ONIE_SUPPORT is TRUE),
 *        the file with type FS_FILE_TYPE_NOS_INSTALLER is the file in ONIE
 *        installer file format and the file is visiable to the end user.
 *        The file with type FS_FILE_TYPE_RUNTIME is invisible by the end user
 *        when the projects support ONIE.
 *     5. need to update UID for onie install file
 */
typedef UI32_T FS_File_Type_T;
/*typedef*/ enum FS_File_Type_E
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
    FS_FILE_TYPE_CPEFIRMWARE,
    FS_FILE_TYPE_CPECONFIG,
    FS_FILE_TYPE_FILEMAPPING, /* This file type is obsoleted. Still need to keep this value for compability among different software versions */
    FS_FILE_TYPE_LICENSE,
    FS_FILE_TYPE_NOS_INSTALLER,
    FS_FILE_TYPE_TOTAL      /* Number of total file type */
} /*FS_File_Type_T*/;

#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
#define FS_TYPE_OPCODE_FILE_TYPE FS_FILE_TYPE_NOS_INSTALLER
#define FS_TYPE_MAX_NUM_OF_FILE_OP_CODE SYS_ADPT_MAX_NUM_OF_FILE_NOS_INSTALLER
#else
#define FS_TYPE_OPCODE_FILE_TYPE FS_FILE_TYPE_RUNTIME
#define FS_TYPE_MAX_NUM_OF_FILE_OP_CODE SYS_ADPT_MAX_NUM_OF_FILE_RUNTIME
#endif

typedef enum FS_RETURN_CODE_E
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
    FS_RETURN_EXCEED_MAX_NUM_OF_FILES,  /* Total files of the given type is exceeded */
    FS_RETURN_EXCEED_VALID_FILE_SIZE   /* Write file size exceed valid size */
}FS_RETURN_CODE_T;

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


enum
{

#if defined(SYS_ADPT_MTD_ID_PART_UBOOT)
  FS_MTD_PART_UBOOT=SYS_ADPT_MTD_ID_PART_UBOOT,
#else
  FS_MTD_PART_UBOOT=0,
#endif

#if defined(SYS_ADPT_MTD_ID_PART_UB_ENV)
  FS_MTD_PART_UB_ENV=SYS_ADPT_MTD_ID_PART_UB_ENV,
#else
  FS_MTD_PART_UB_ENV,
#endif

#if defined(SYS_ADPT_MTD_ID_PART_HW_INFO)
  FS_MTD_PART_HW_INFO=SYS_ADPT_MTD_ID_PART_HW_INFO,
#else
  FS_MTD_PART_HW_INFO,
#endif

  FS_MTD_PART_RESERVE_INFO,

#if defined(SYS_ADPT_MTD_ID_PART_MFG_RUNTIME)
  FS_MTD_PART_MFG_RUNTIME=SYS_ADPT_MTD_ID_PART_MFG_RUNTIME, /*Save manufacture runtime*/
#else
  FS_MTD_PART_MFG_RUNTIME, /*Save manufacture runtime*/
#endif

  FS_MTD_PART_RECOVERY_RUNTIME, /*Save recovery runtime that is factory default runtime.*/
  FS_MTD_PART_PRIMITIVE_FS, /* Partition that employs primitive file system such as JFFS2, UBIFS, EXT3, etc. */
  FS_MTD_PART_DATA_STORAGE, /* Partition for data storage service */
  FS_MTD_PART_MAX_NUMBER, /* this must be put in the last entry */
};

#define FS_TYPE_INVALID_MTD_ID 255

/*
 *  FileMapping File Management
 */
#define FS_FILEMAPPING_CFG_NUM       1 /*only one startup cfg*/
#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
#define FS_FILEMAPPING_NOS_INSTALLER_NUM   SYS_ADPT_MAX_NUM_OF_FILE_NOS_INSTALLER
#define FS_FILEMAPPING_TOTAL_NUM           (FS_FILEMAPPING_NOS_INSTALLER_NUM+FS_FILEMAPPING_CFG_NUM) /*nos installer and one startup cfg*/
#else
#define FS_FILEMAPPING_RUNTIME_NUM         SYS_ADPT_MAX_NUM_OF_FILE_RUNTIME
#define FS_FILEMAPPING_TOTAL_NUM           (FS_FILEMAPPING_RUNTIME_NUM+FS_FILEMAPPING_CFG_NUM)       /*runtime and one startup cfg*/
#endif

typedef enum
{
    FS_FILEMAPPING_INDEX_CFG=0,
#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
    FS_FILEMAPPING_INDEX_NOS_INSTALLER_BASE=1,
#else
    FS_FILEMAPPING_INDEX_RUNTIME_BASE=1,
#endif

#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
    FS_FILEMAPPING_INDEX_NUM=FS_FILEMAPPING_INDEX_NOS_INSTALLER_BASE+FS_FILEMAPPING_NOS_INSTALLER_NUM,
#else
    FS_FILEMAPPING_INDEX_NUM=FS_FILEMAPPING_INDEX_RUNTIME_BASE+FS_FILEMAPPING_RUNTIME_NUM,
#endif
}FS_FILEMAPPING_T;

typedef struct {
    /* NOTE! The max filename string length is SYS_ADPT_FILE_SYSTEM_NAME_LEN
     * the size of field "file_name" does not reserve the NULL-terminated char '\0'
     * Need to be careful to consider the case that the string length of file_name
     * is SYS_ADPT_FILE_SYSTEM_NAME_LEN and no '\0' in the end of file_name!
     * Unexepected result might happen when calling the functions like
     * strcmp and strcpy using the string without '\0' at the end of the string.
     */
    UI8_T          file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN]; /* filename */
    UI32_T         file_size;                                /* file size */
    UI32_T         creat_time;                               /* file create time */
    FS_File_Type_T file_type;                                /* file type */
    BOOL_T         startup_flag;                             /* start-up flag  indicate status is active or free */
}FS_FILEMAPPING_FILE_DATA_T;


typedef struct {    
    FS_FILEMAPPING_FILE_DATA_T   file_data[FS_FILEMAPPING_TOTAL_NUM];
}FS_FILEMAPPING_FILE_T; /*We design file_data[0] is startup cfg, and others are runtime info*/

/* naming constants for FS_HW_Info_T.capability
 */
#define FS_HWINFO_CAPABILITY_CHANGE_LOADER_BACKDOOR_PASSWORD BIT_0

#if (SYS_CPNT_ONIE_SUPPORT==TRUE)

/********************************
 * Start of TlvInfo definitions *
 ********************************/

/*  The Definition of the TlvInfo EEPROM format can be found at onie.org or
 *  github.com/onie
 */

/* TlvInfo header: Layout of the header for the TlvInfo format
 */
struct __attribute__ ((__packed__)) tlvinfo_header_s {
    char    signature[8];       /* 0x00 - 0x07 EEPROM Tag "TlvInfo" */
    UI8_T   version;            /* 0x08        Structure version    */
    UI16_T  totallen;           /* 0x09 - 0x0A Length of all data which follows */
};
typedef struct tlvinfo_header_s tlvinfo_header_t;

/* Header Field Constants
 */
#define TLV_INFO_ID_STRING      "TlvInfo"
#define TLV_INFO_VERSION        0x01
#define TLV_INFO_MAX_LEN        2048
#define TLV_TOTAL_LEN_MAX       (TLV_INFO_MAX_LEN - sizeof(tlvinfo_header_t))

/* TlvInfo TLV: Layout of a TLV field
 */
struct __attribute__ ((__packed__)) tlvinfo_tlv_s {
    UI8_T  type;
    UI8_T  length;
    UI8_T  value[0];
};
typedef struct tlvinfo_tlv_s tlvinfo_tlv_t;

/*  The TLV Types
 */
#define TLV_CODE_PRODUCT_NAME   0x21
#define TLV_CODE_PART_NUMBER    0x22
#define TLV_CODE_SERIAL_NUMBER  0x23
#define TLV_CODE_MAC_BASE       0x24
#define TLV_CODE_MANUF_DATE     0x25
#define TLV_CODE_DEVICE_VERSION 0x26
#define TLV_CODE_LABEL_REVISION 0x27
#define TLV_CODE_PLATFORM_NAME  0x28
#define TLV_CODE_ONIE_VERSION   0x29
#define TLV_CODE_MAC_SIZE       0x2A
#define TLV_CODE_MANUF_NAME     0x2B
#define TLV_CODE_MANUF_COUNTRY  0x2C
#define TLV_CODE_VENDOR_NAME    0x2D
#define TLV_CODE_VENDOR_EXT     0xFD
#define TLV_CODE_CRC_32         0xFE

/******************************
 * End of TlvInfo definitions *
 ******************************/

typedef enum FS_TYPE_TRANSLATE_HWINFO_FLAG_E
{
    FS_TYPE_TRANSLATE_HWINFO_FLAG_MAC_ADDR        = BIT_0,
    FS_TYPE_TRANSLATE_HWINFO_FLAG_SERIAL_NO       = BIT_1,
    FS_TYPE_TRANSLATE_HWINFO_FLAG_MANUFACTURE_DATE= BIT_2,
    FS_TYPE_TRANSLATE_HWINFO_FLAG_PID             = BIT_3,
    FS_TYPE_TRANSLATE_HWINFO_FLAG_BID             = BIT_4,
    FS_TYPE_TRANSLATE_HWINFO_FLAG_HW_VER          = BIT_5,
} FS_TYPE_TRANSLATE_HWINFO_FLAG_T;

/* Maximum string length of ONIE EEPROM Product Name String
 */
#define FS_TYPE_ONIE_PRODUCT_NAME_MAX_LEN 79

typedef struct FS_TYPE_ONIEProductName_PIDBID_Map_Entry_S
{
    char    product_name[FS_TYPE_ONIE_PRODUCT_NAME_MAX_LEN+1];  /* ONIE Product Name */
    UI32_T  project_id;                                         /* HWINFO Project ID */
    UI32_T  board_id;                                           /* HWINFO Board ID   */
} FS_TYPE_ONIEProductName_PIDBID_Map_Entry_T;
#endif /* end of #if (SYS_CPNT_ONIE_SUPPORT==TRUE) */

/* enumeration for data storage id
 * FS data storage service provides non-volatile data storage service for the
 * data that would like to be hidden from the end users on open linux system(i.e. SYS_CPNT_ONIE_SUPPORT is TRUE)
 * NOTE!!! Always append the new id behind the last valid id!
 */
typedef enum
{
    FS_TYPE_DATA_STORAGE_ID_ALU=0,       /* service type = FS_TYPE_DATA_STORAGE_SERVICE_TYPE_FAIL_SAFE */
    FS_TYPE_DATA_STORAGE_ID_SYSTIME,     /* service type = FS_TYPE_DATA_STORAGE_SERVICE_TYPE_PRIMITIVE */
    FS_TYPE_DATA_STORAGE_ID_RESERVED1,
    FS_TYPE_DATA_STORAGE_ID_RESERVED2,
    FS_TYPE_DATA_STORAGE_ID_RESERVED3,
    FS_TYPE_DATA_STORAGE_ID_RESERVED4,
    FS_TYPE_DATA_STORAGE_ID_RESERVED5,
    FS_TYPE_DATA_STORAGE_ID_RESERVED6,
    FS_TYPE_DATA_STORAGE_ID_RESERVED7,
    FS_TYPE_DATA_STORAGE_ID_RESERVED8,
    FS_TYPE_DATA_STORAGE_ID_TOTAL_NUM, /* This must be kept in the last entry */
} FS_TYPE_DataStorageId_T;

/* enumeration for service type of the data storage service
 * NOTE!!! Always append the new id behind the last valid type!
 */
typedef enum
{
    FS_TYPE_DATA_STORAGE_SERVICE_TYPE_FAIL_SAFE=0,
    FS_TYPE_DATA_STORAGE_SERVICE_TYPE_PRIMITIVE,
    FS_TYPE_DATA_STORAGE_SERVICE_TYPE_TOTAL_NUM, /* This must be kept in the last entry */
} FS_TYPE_DataStorageServiceType_T;

#endif /* _FS_TYPE_H_ */
