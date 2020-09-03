/*-------------------------------------------------------------------------
 * Module Name:fs.c
 *-------------------------------------------------------------------------
 * Purpose    : This file contained all API Function about File System
 *-------------------------------------------------------------------------
 * Notes:
 * History:
 *    09/01/2001 - Jimmy Lin, Created
 *    11/23/2001 - Allen Cheng, Revised
 *    03/28/2003 - Arden Chiu,
 *                 Flash block check sum is performed  everytime on the
 *                 necessary blocks when a read file command is issued
 *                 instead of doing a complete block checksum scan
 *                 when the system boots up to reduce boot up time.
 *    05/27/2003 - Arden Chiu,
 *                 Add FS_ShutDwon so that FS will block the system form
 *                 Entering the Transition mode or restarting if there is
 *                 an on going process is not finished.
 *    04/06/2004 - Allen Cheng,
 *                 1. Create the FS task for the background routine jobs on each
 *                    unit.
 *                 2. Request the FS task for the remote writing in the background.
 *                 3. Request the FS task to perform the checksum on all the local
 *                    files in the background.
 *    04/19/2004 - Allen Cheng,
 *                 1. The critical setction protection mechanism is revised.
 *                 2. The FS shutdown mechanism is revised.
 *    06/14/2004 - Allen Cheng,
 *                 The flash auto detection mechanism is revised.
 *
 *-------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2001
 *-------------------------------------------------------------------------
 */

/************************************
 ***   INCLUDE FILE DECLARATIONS  ***
 ************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/fs.h> /* for BLKROSET */
#include <sys/ioctl.h> /* for ioctl() */
#include <errno.h>     /* for errno */
#include <mtd/mtd-user.h> /* for erase_info_t */
#include "sysfun.h"
#include "sys_bld.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "sys_hwcfg.h"
#include "sys_hwcfg_common.h"
#include "sys_module.h"
#include "leaf_sys.h"
#include "l_mm.h"
#include "l_math.h"
#include "l_charset.h"
#include "l_stdlib.h"
#include "stktplg_pom.h"
#include "fs_type.h"
#include "sysrsc_mgr.h"
#include "fs_om.h"
#include "fs.h"
#include "buffer_mgr.h"

#if !defined(INCLUDE_DIAG) && (SYS_CPNT_FS_SUPPORT_UBIFS==TRUE && SYS_HWCFG_RUNTIME_FLASH_TYPE==SYS_HWCFG_FLASH_TYPE_NAND)
#include "syslog_om.h"
#include "syslog_pmgr.h"
#endif

#ifdef INCLUDE_DIAG
#undef SYS_CPNT_STACKING
#endif
#if (SYS_CPNT_STACKING == TRUE)
#include "isc.h"
#endif

#if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE)
#include "ams_part.h"
#endif

#include "uc_mgr.h"
#ifndef INCLUDE_DIAG
#include "backdoor_mgr.h"
#endif
//#include "fs_backdoor.h"

/* Linux
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/vfs.h> /* or <sys/statfs.h> */

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#ifndef INCLUDE_DIAG
#include "sw_watchdog_mgr.h"
#endif
#endif

#if (SYS_CPNT_ONIE_SUPPORT==FALSE)
/* The following three typedef is used in image.h
 */
typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;

#include "image.h"       /* get uboot header */
#endif

/* if flash is not ready,
 * set FS_DUMMY_WRITE to 1, to ignore all write operation.
 */
#define FS_DUMMY_WRITE  0

#if FS_DUMMY_WRITE
#define FS_DUMMY_WRITE_RETURN(...) do { \
            printf("%s:%d: driver not ready, ignore flash write opertaion.\r\n",__func__,__LINE__); \
            return __VA_ARGS__; \
        } while (0)
#else
#define FS_DUMMY_WRITE_RETURN(...)
#endif

//Wally temporary modify
#define FS_MAX_FILE_NAME_LENGTH         SYS_ADPT_FILE_SYSTEM_NAME_LEN

#ifndef FS_UTEST
#define FS_SAVE_FOLDER                  "/flash/.fs/"
#endif

#define FS_FILENAME_MARK                ""

#define FS_FILENAME_MARK_SIZE           (sizeof(FS_FILENAME_MARK)-1) /* minus 1 for null-terminated char */

#define FS_TMP_FOLDER                   "/tmp/"
#define FS_TMP_WRITE_FILE_NAME          "tmp_wfile" /* for nandwrite use */
#define FS_TMP_READ_FILE_NAME           "tmp_rfile" /* for nanddump use */
#define FS_FLASH_MTD_PREFIX             "/dev/mtd"
#define FS_NANDWRITE_CMD                "nandwrite"
#define FS_FLASH_ERASE_CMD              "flash_erase"
#define FS_NANDDUMP_CMD                 "nanddump"

#define FS_FLASH_MTD_BLOCK_PREFIX       "/dev/mtdblock"
#define FS_MTD_NUMBER_NOT_EXIST         0xFFFFFFFF

#define FS_READRUNTIMERFILE_PERIODIC_TICKS (24*60*60*100) /* 1 day */
#define DUMMY_READ_RUNTIME_BUF_SIZE (512 * 1024) /* 512 kbytes */

#define UPDATE_CACHE_RUNTIME_PARTITION_SCRIPT_FILENAME "/etc/update_cache_runtime_partition.sh"
#define CHECK_CACHE_RUNTIME_PARTITION_SCRIPT_FILENAME "/etc/is_cache_runtime_partition_valid.sh"

/* AOS_ONIE_CHANGE_STARTUP_INSTALLER_SCRIPT_FILENAME:
 *     The script file to change the startup ONIE installer file on AOS platform.
 *     Note that the execution of this script shall be treated as programming
 *     flash and must be executed under the protection of FS_LOCK().
 */
#define AOS_ONIE_CHANGE_STARTUP_INSTALLER_SCRIPT_FILENAME "/etc/aos_change_startup_installer.sh"

/* ONLP_QUERY_BASE_PATH
 *     The base path to the ONLP utility program "onlp_query".
 */
#define ONLP_QUERY_BASE_PATH "/usr/bin/onlp_query"

/* ONLP_QUERY_OUTPUT_BUF_SIZE
 *     The size of the output buffer used to keep the output result of
 *     executing the ONLP utility program "onlp_query".
 */
#define ONLP_QUERY_OUTPUT_BUF_SIZE 24

/* projects that use hard-code hardware information when ONLP library is not ready net
 */
#if defined(AOS5710_54X)
#define ONIE_EEPROM_HARD_CODE TRUE
#endif

#if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE)
typedef struct FS_PARTITION_TABLE_S
{
   UI8_T   name[FLASH_REGION_NAMELEN];
   UI32_T  base;
   UI32_T  size;
   UI32_T  type;                              /*  indicate partition type */
   UI32_T  mtd_num;
}FS_PARTITION_TABLE_T;

static FS_PARTITION_TABLE_T  fs_partition_table[MAX_PARTITION_NUMBER];
#define FS_FLASH_MTD_BLOCK_PARTITION_TABLE "dev/mtdblock1"
#endif

/* Mapping file type FS_FILE_TYPE_SUBFILE ~ FS_FILE_TYPE_TOTAL to
 * file user ID
 */
#define FS_TYPE_UID_BITMAP(_)                      \
_(FS_TYPE_UID_BITMAP_STARTUP,               BIT_0) \
_(FS_TYPE_UID_BITMAP_MODE_LEGACY,           BIT_1) \
_(FS_TYPE_UID_BITMAP_MODE_OF,               BIT_2) \
_(FS_TYPE_UID_BITMAP_MODE_RESERVED,         BIT_3) \
_(FS_TYPE_UID_BITMAP_TYPE_END,              BIT_4) \
_(FS_TYPE_UID_BITMAP_TYPE_4,                BIT_5) \
_(FS_TYPE_UID_BITMAP_TYPE_3,                BIT_5) \
_(FS_TYPE_UID_BITMAP_TYPE_2,                BIT_7) \
_(FS_TYPE_UID_BITMAP_TYPE_START,            BIT_8)

#define FS_TYPE_UID_BITMAP_ARRAY_INDEX(type, bit) type,
#define FS_TYPE_UID_BITMAP_VALUE(type, bit) bit,

typedef enum
{
  FS_TYPE_UID_BITMAP(FS_TYPE_UID_BITMAP_ARRAY_INDEX)

  FS_TYPE_UID_BITMAP_TOTAL,
}FS_TYPE_UID_BITMAP_ARRAY_INDEX_T;

#define FS_TYPE_UID_BITMAP_BIT_VALUE_ENUM(type, bit) type##_BIT = bit,

typedef enum
{
    FS_TYPE_UID_BITMAP(FS_TYPE_UID_BITMAP_BIT_VALUE_ENUM)
} FS_TYPE_UID_BITMAP_BIT_VALUE_T;

static uid_t fs_uid_bitmap[] = 
{
    FS_TYPE_UID_BITMAP(FS_TYPE_UID_BITMAP_VALUE)
};

#define FS_UID_DFLT_MODE_BITMAP_VALUE_ALL (FS_TYPE_UID_BITMAP_MODE_LEGACY_BIT|FS_TYPE_UID_BITMAP_MODE_OF_BIT)

#define FS_UID_DFLT_MODE_BITMAP_VALUE_BLD_MODE (FS_TYPE_UID_BITMAP_MODE_LEGACY_BIT)

/* If the file type can be used among all modes, set the
 * value as FS_UID_DFLT_MODE_BITMAP_VALUE_ALL.
 * If the file type can only be used on the mode that the code is built,
 * set the value as FS_UID_DFLT_MODE_BITMAP_VALUE_BLD_MODE
 */
static uid_t fs_uid_type_to_dflt_mode_bitmap[FS_FILE_TYPE_TOTAL] =
{
    FS_UID_DFLT_MODE_BITMAP_VALUE_ALL,      /*FS_FILE_TYPE_SUBFILE*/
    FS_UID_DFLT_MODE_BITMAP_VALUE_ALL,      /*FS_FILE_TYPE_KERNEL*/
    FS_UID_DFLT_MODE_BITMAP_VALUE_ALL,      /*FS_FILE_TYPE_DIAG*/
    FS_UID_DFLT_MODE_BITMAP_VALUE_ALL,      /*FS_FILE_TYPE_RUNTIME*/
    FS_UID_DFLT_MODE_BITMAP_VALUE_BLD_MODE, /*FS_FILE_TYPE_SYSLOG*/
    FS_UID_DFLT_MODE_BITMAP_VALUE_ALL,      /*FS_FILE_TYPE_CMDLOG*/
    FS_UID_DFLT_MODE_BITMAP_VALUE_BLD_MODE, /*FS_FILE_TYPE_CONFIG*/
    FS_UID_DFLT_MODE_BITMAP_VALUE_ALL,      /*FS_FILE_TYPE_POSTLOG*/
    FS_UID_DFLT_MODE_BITMAP_VALUE_ALL,      /*FS_FILE_TYPE_PRIVATE*/
    FS_UID_DFLT_MODE_BITMAP_VALUE_ALL,      /*FS_FILE_TYPE_CERTIFICATE*/
    FS_UID_DFLT_MODE_BITMAP_VALUE_ALL,      /*FS_FILE_TYPE_ARCHIVE*/
    FS_UID_DFLT_MODE_BITMAP_VALUE_BLD_MODE, /*FS_FILE_TYPE_BINARY_CONFIG*/
    FS_UID_DFLT_MODE_BITMAP_VALUE_ALL,      /*FS_FILE_TYPE_PUBLIC*/
    FS_UID_DFLT_MODE_BITMAP_VALUE_ALL,      /*FS_FILE_TYPE_CPEFIRMWARE*/
    FS_UID_DFLT_MODE_BITMAP_VALUE_ALL,      /*FS_FILE_TYPE_CPECONFIG*/
    FS_UID_DFLT_MODE_BITMAP_VALUE_ALL,      /*FS_FILE_TYPE_FILEMAPPING*/ /*obsoleted*/
    FS_UID_DFLT_MODE_BITMAP_VALUE_ALL,      /*FS_FILE_TYPE_LICENSE*/
    FS_UID_DFLT_MODE_BITMAP_VALUE_ALL,      /*FS_FILE_TYPE_NOS_INSTALLER*/
};

#define FS_UID_IS_CURRENTMODE(uid)     ((uid&fs_uid_bitmap[FS_TYPE_UID_BITMAP_MODE_LEGACY])?TRUE:FALSE)

#define FS_UID_TO_TYPE(uid)               (((uid)&((fs_uid_bitmap[FS_TYPE_UID_BITMAP_TYPE_START] << 1) - 1)) >> FS_TYPE_UID_BITMAP_TYPE_END)
#define FS_UID_TO_STARTUP(uid)            ((((uid)&fs_uid_bitmap[FS_TYPE_UID_BITMAP_STARTUP]) &&  \
                                              (FS_UID_IS_CURRENTMODE(uid))))
#define FS_TYPE_STARTUP_TO_UID(type,startup)    \
            ((type << FS_TYPE_UID_BITMAP_TYPE_END) | \
	        (startup? fs_uid_bitmap[FS_TYPE_UID_BITMAP_STARTUP]: 0) | \
	        (fs_uid_type_to_dflt_mode_bitmap[type]))

#ifdef  INCLUDE_DIAG
#define BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack(a, b)
#endif

#if (SYS_CPNT_STACKING == TRUE)
#define FS_STATE_IDLE               0
#define FS_STATE_ABORT              1
#define FS_STATE_DATA_TRANSFER      2
#define FS_STATE_DATA_COMPLETE      3
#define FS_STATE_FLASH_PROGRAMMING  4

#define FS_FLASH_LOADER             0
#define FS_FLASH_ATTRIBUTE          1
#define FS_FLASH_FILE               2

#define FS_TASK_MAX_MSGQ_LEN        64

#define FS_MAX_DATA_SIZE            (SYS_ADPT_ISC_MAX_PDU_LEN - sizeof(FS_Packet_Header_T))
#define MAX_NUM_OF_FILE             32
#define FS_TIMEOUT_VALUE            200
#define FS_ISC_TRY_COUNT            3
#define FS_SLEEP_TIME               50
#define FS_COMMAND_RETRY_COUNT      30
#define FS_COMMAND_RETRY_MORECOUNT  100
#define FS_ABORT_COUNT              1
#define FS_WORD_WRAP                16

#define FS_MCAST_DATA_SIZE          (SYS_ADPT_ISC_MAX_PDU_LEN - sizeof(FS_Packet_Header_T))
#define FS_MCAST_TRY_COUNT          3
#define FS_MCAST_TIMEOUT            200
#define FS_MCAST_NAK_RETRY_TIMES    5

/* compute how many file attr can be held in one packet, refer to FS_File_Attr_Packet_T
 */
#define FS_MAX_FILE_ATTR_IN_ONE_PACKET  ((SYS_ADPT_ISC_MAX_PDU_LEN - sizeof(FS_Packet_Header_T) - sizeof(UI16_T))/(sizeof(FS_File_Attr_T)))

#define FS_CAPABILITY_MAGIC_NUM     0x4361702E  /* 0x4361702E = the ASCII code of "Cap.".
                                                 * Used for guaranteeing it is "Capability", not "H/W Info".
                                                 * Because X3-XX-XX-XX-XX-XX is a multicast MAC, and the MAC
                                                 * that H/W Info stores will not have multicast MAC.
                                                 */
#define FS_POOL_ID_ISC_SEND    0
#define FS_POOL_ID_ISC_REPLY   1

#endif /* (SYS_CPNT_STACKING == TRUE) */

#define FS_FILENAME_CHECK_VALID_INVIS           '.'

/* size of the certificate file in bytes */
#define FS_CERTIFICATE_BLOCK_SIZE           SYS_TYPE_64K_BYTES

#define FS_INIT_FUNC_NO                         0x01
#define FS_FILENAMECHECK_FUNC_NO                0x02
#define FS_GENERICFILENAMECHECK_FUNC_NO         0x03
#define FS_GETSTORAGEFREESPACE_FUNC_NO          0x04
#define FS_GETFILESTORAGEUSAGESIZE_FUNC_NO      0x05
#define FS_GETFILEINFO_FUNC_NO                  0x06
#define FS_GETNEXTFILEINFO_FUNC_NO              0x07
#define FS_ERASEFILE_FUNC_NO                    0x08
#define FS_READFILE_FUNC_NO                     0x09
#define FS_COPYFILECONTENT_FUNC_NO              0x0A
#define FS_UPDATEFILE_FUNC_NO                   0x0B
#define FS_SETSTARTUPFILENAME_FUNC_NO           0x0C
#define FS_RESETSTARTUPFILENAME_FUNC_NO         0x0D
#define FS_GETSTARTUPFILENAME_FUNC_NO           0x0E
#define FS_WRITESPECIALBLOCK_FUNC_NO            0x0F
#define FS_READHARDWAREINFO_FUNC_NO             0x10
#define FS_STARTUPVERIFICATION_FUNC_NO          0x11
#define FS_GETCERTIFICATESTORAGEINFO_FUNC_NO    0x12
#define FS_WRITECERTIFICATEFILE_FUNC_NO         0x13
#define FS_STATIC_FUNCTION_FUNC_NO              0xFFFFFFFF

enum
{
    FS_SPECIAL_BLOCK_NONE,
    FS_SPECIAL_BLOCK_BDINFO,
    FS_SPECIAL_BLOCK_LOADER,
    FS_SPECIAL_BLOCK_STARTUP
};

enum
{
    FS_FILE_TYPE_VISIBLE_SUBFILE        = FALSE,
    FS_FILE_TYPE_VISIBLE_KERNEL         = TRUE,
    FS_FILE_TYPE_VISIBLE_DIAG           = TRUE,
#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
    FS_FILE_TYPE_VISIBLE_RUNTIME        = FALSE,
#else
    FS_FILE_TYPE_VISIBLE_RUNTIME        = TRUE,
#endif
    FS_FILE_TYPE_VISIBLE_SYSLOG         = FALSE,
    FS_FILE_TYPE_VISIBLE_CMDLOG         = FALSE,
    FS_FILE_TYPE_VISIBLE_CONFIG         = TRUE,
    FS_FILE_TYPE_VISIBLE_POSTLOG        = FALSE,
    FS_FILE_TYPE_VISIBLE_PRIVATE        = FALSE,
    FS_FILE_TYPE_VISIBLE_CERTIFICATE    = TRUE,
    FS_FILE_TYPE_VISIBLE_ARCHIVE        = TRUE,
    FS_FILE_TYPE_VISIBLE_BINARY_CONFIG  = FALSE,
    FS_FILE_TYPE_VISIBLE_PUBLIC         = TRUE,
    FS_FILE_TYPE_VISIBLE_CPEFIRMWARE    = FALSE,
    FS_FILE_TYPE_VISIBLE_CPECONFIG      = FALSE,
    FS_FILE_TYPE_VISIBLE_FILEMAPPING    = FALSE,
    FS_FILE_TYPE_VISIBLE_LICENSE        = TRUE,
#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
    FS_FILE_TYPE_VISIBLE_NOS_INSTALLER  = TRUE,
#else
    FS_FILE_TYPE_VISIBLE_NOS_INSTALLER  = FALSE,
#endif
};

#if (SYS_CPNT_STACKING == TRUE)
/*
 * Opcodes for different remote sevices
 */
enum
{  /********************************
    * OPCODES for remote operation *
    ********************************/
    FS_READ_REQUEST = 0,
    FS_WRITE_REQUEST,
    FS_SET_ATTRIBUTE_REQUEST,
    FS_DATA,
    FS_PROGRAM_FLASH,
    FS_FLASH_STATUS,
    FS_GET_SERVICE_CAPABILITY,  /* Original OPCODE name is FS_GETHARDWAREINFO */
    FS_ABORT,
    FS_ACK,
    FS_NAK,
    FS_SERVICE_NOT_SUPPORT,
    FS_MCAST_WRITE_REQUEST,     /* The opcode of FS_RemoteMCastWrite */
    FS_MCAST_DATA,              /* The opcode of FS_RemoteMCastReceiveData */
    FS_MCAST_PROGRAM_FLASH,     /* The opcode of FS_RemoteMCastFlashFile */
    FS_GET_HARDWARE_INFO,       /* Because the opcode of getting H/W info is replaced into FS_GET_SERVICE_CAPABILITY. */
    FS_COPY_FILE_CONTENT,       /* Support copy the part contents of the remote file. */
    FS_GETFILESUMMARY,          /* The opcode of FS_RemoteGetFileSystemSummary */
    FS_GETMULTIPLEFILEINFO,     /* The opcode of FS_RemoteGetMultipleFileInfo */
    FS_GETSTARTUPFILENAME,      /* The opcode of FS_RemoteGetStartupFilename */
    FS_HAVEMOREDATA,
    FS_GETSTORAGEFREESPACEFORUPDATERUNTIME, /* The opcode of FS_RemoteGetStorageFreeSpaceForUpdateRuntime */
    FS_TOTAL_REMOTE_SERVICES    /* Number of total remote services */
};
#endif /* SYS_CPNT_STACKING */

#if (SYS_CPNT_STACKING == TRUE)

/* Packet header definition.
 */
typedef struct FS_Packet_Header_S
{
    UI16_T  opcode;
    UI16_T  seq_no;
    UI16_T  data_size; /* size of the service data unit */
}__attribute__((packed, aligned(1)))FS_Packet_Header_T;

/* Packet format for Read/Write requests.
 */
typedef struct FS_Request_Packet_S
{
    FS_Packet_Header_T header;
    FS_File_Attr_T     file_attr;
    UI32_T             max_count;
    BOOL_T             next;
}__attribute__((packed, aligned(1)))FS_Request_Packet_T;

/* Packet format for NAK.
 */
typedef struct FS_NAK_Packet_S
{
    FS_Packet_Header_T  header;
    UI16_T              error_code;
}__attribute__((packed, aligned(1)))FS_NAK_Packet_T;

/* Packet format for ACK is the same as NAK.
 */
typedef FS_NAK_Packet_T FS_ACK_Packet_T;

/* Packet format for exchange file information between drives.
 */
typedef struct
{
    FS_Packet_Header_T  header;
    UI32_T              total_space_free;
    UI16_T              total_num_of_file;
    UI16_T              num_of_file[FS_FILE_TYPE_TOTAL];
}__attribute__((packed, aligned(1))) FS_File_Summary_Packet_T;

typedef struct
{
    FS_Packet_Header_T  header;
    UI16_T              file_count;
    FS_File_Attr_T      file_attr[FS_MAX_FILE_ATTR_IN_ONE_PACKET];
}__attribute__((packed, aligned(1))) FS_File_Attr_Packet_T;

/* Packet format for exchange hardware information.
 */
typedef struct FS_HW_Info_Packet_S
{
    FS_Packet_Header_T  header;
    FS_HW_Info_T        hw_info;
}__attribute__((packed, aligned(1))) FS_HW_Info_Packet_T;

/* Packet format for transfer file data.
 */
typedef struct FS_Data_Packet_S
{
    FS_Packet_Header_T  header;
    UI8_T               raw_data[FS_MAX_DATA_SIZE];
}__attribute__((packed, aligned(1)))FS_Data_Packet_T;

/* Packet format for transfer file data by ISC Multi-Cast.
 */
typedef struct FS_MCast_Data_Packet_S
{
    FS_Packet_Header_T  header;
    UI8_T               raw_data[FS_MCAST_DATA_SIZE];
}__attribute__((packed, aligned(1)))FS_MCast_Data_Packet_T;

typedef struct FS_Service_Capability_S
{
    UI32_T              magic_num;
    UI32_T              capability;
}__attribute__((packed, aligned(1)))FS_Service_Capability_T;

/* Packet format for exchange service capability. */
typedef struct FS_Capability_Packet_S
{
    FS_Packet_Header_T      header;
    FS_Service_Capability_T service_capability;
}__attribute__((packed, aligned(1)))FS_Capability_Packet_T;

/* Packet format for the request of coping the contents of a file. */
typedef struct FS_Copy_Content_Request_Packet_S
{
    FS_Packet_Header_T  header;
    FS_File_Attr_T      file_attr;
    UI32_T              offset;
    UI32_T              byte_num;
}__attribute__((packed, aligned(1)))FS_Copy_Content_Request_Packet_T;

/* Common buffer for received ISC packet */
typedef struct FS_Rx_IscBuf_S
{
    FS_Packet_Header_T  header;
    union
    {
        UI16_T                  error_code;         /* For ACK/NAK */
        FS_File_Attr_T          file_attr;          /* Read, Write, Delete ... single file operation */
        struct
        {
            UI32_T              total_space_free;
            UI16_T              total_num_of_file;
            UI16_T              num_of_file[FS_FILE_TYPE_TOTAL];
        }__attribute__((packed, aligned(1))) drive_info;                               /* To hold information of all files on a remote drive. */
        struct
        {
            UI16_T              file_count;
            FS_File_Attr_T      file_attrs[FS_MAX_FILE_ATTR_IN_ONE_PACKET];
        }__attribute__((packed, aligned(1))) files_info;
        FS_HW_Info_T            hw_info;            /* Hardware Information */
        FS_Service_Capability_T service_capability;
        UI8_T                   raw_data[FS_MAX_DATA_SIZE];
    } __attribute__((packed, aligned(1)))data;
}__attribute__((packed, aligned(1)))FS_Rx_IscBuf_T;


typedef void (*FS_Service_Func_T)();
#endif /* (SYS_CPNT_STACKING == TRUE) */

static BOOL_T FS_GetDeviceFileName(UI8_T *filename, FS_File_Type_T file_type, char *full_name);
static BOOL_T FS_AddDeviceFileNameMark(UI8_T *src_name, char *dst_name, FS_File_Type_T file_type);
static void FS_RemoveDeviceFileNameMark(char *src_name, UI8_T *dst_name, FS_File_Type_T file_type);
/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetMtdDevIdByMtdType
 * ------------------------------------------------------------------------
 * FUNCTION : This function output the mtd device id according to the given
 *            mtd type.
 * INPUT    : mtd_type     -- mtd type(FS_MTD_PART_XXX constants defined in fs_type.h)
 * OUTPUT   : mtd_dev_id_p -- The mtd device id of the given mtd_type.
 * RETURN   : None
 * NOTE     : TRUE  - The mtd device id is output successfully.
 *            FALSE - The mtd device id of the given mtd_type cannot be found.
 * ------------------------------------------------------------------------
 */
#if (SYS_CPNT_FS_GET_MTD_ID_FROM_SCRIPT == TRUE)
static inline BOOL_T FS_GetMtdDevIdByMtdType(UI32_T mtd_type, UI32_T *mtd_dev_id_p)
{
    return FS_OM_GetMtdDevIdByMtdType(mtd_type, mtd_dev_id_p);
}
#else /* #if (SYS_CPNT_FS_GET_MTD_ID_FROM_SCRIPT == TRUE) */
static inline BOOL_T FS_GetMtdDevIdByMtdType(UI32_T mtd_type, UI32_T *mtd_dev_id_p)
{
    *mtd_dev_id_p = mtd_type;
    return TRUE;
}
#endif /* end of #if (SYS_CPNT_FS_GET_MTD_ID_FROM_SCRIPT == TRUE) */

#if ((SYS_HWCFG_FLASH_TYPE & SYS_HWCFG_FLASH_TYPE_NAND)!=0)
static UI32_T FS_WriteNandFlash(UI8_T *buf, UI32_T length, UI32_T mtd, BOOL_T is_ro_mtd);
static UI32_T FS_ReadNandFlash(UI8_T *buf, UI32_T buf_len, UI32_T *read_len, UI32_T mtd);
#endif

#if ((SYS_HWCFG_FLASH_TYPE & SYS_HWCFG_FLASH_TYPE_NOR)!=0)
static UI32_T FS_WriteNorFlash(UI8_T *buf, UI32_T length, UI32_T mtd, BOOL_T is_ro_mtd);
static UI32_T FS_ReadNorFlash(UI8_T *buf, UI32_T buf_len, UI32_T *read_len, UI32_T mtd);
#endif


static BOOL_T FS_BuildFileHeaderFromFile(char file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN]);
static BOOL_T FS_BuildAllFileHeader(void);
#ifndef INCLUDE_DIAG
static void FS_BackDoor_Menu (void);
#endif


#if (SYS_CPNT_STACKING == TRUE)
static  UI8_T   FS_ByteSum(UI8_T *buf_ptr, UI32_T length);
#endif
static  UI32_T  FS_EraseFile(UI32_T unit, UI8_T  *file_name, BOOL_T privilege);
static  UI32_T  FS_UpdateFile(UI32_T unit, UI8_T  *file_name, UI8_T  *file_comment, UI32_T file_type, UI8_T *buf, UI32_T length, UI32_T resv_len, BOOL_T privilege);
static  UI32_T  FS_UpdateStartupAttribute(UI32_T file_type, UI8_T  *file_name, BOOL_T privilege);
#if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE)
static FS_PARTITION_TABLE_T *FS_GetNextFlashPartitionEntry(FS_PARTITION_TABLE_T *flash_entry_p);
static BOOL_T FS_GetFreeRuntimePartitionInfo(UI32_T *runtime_part_len_p, UI32_T *free_part_num_p);
#endif

#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
#if (ONIE_EEPROM_HARD_CODE!=TRUE) && (SYS_CPNT_FS_HWINFO_FROM_ONLP==FALSE)
static BOOL_T FS_IsValidTlvinfoHeader(tlvinfo_header_t *hdr_p);
static BOOL_T FS_IsValidONIEEeprom(UI8_T *eeprom_p);
static void   FS_StrCopyFromEepromStr(const char* eeprom_str_p, UI32_T eeprom_str_len, UI32_T output_str_buf_len, char* output_str_p);
#endif
static UI32_T FS_TranslateONIEEepromToHwInfo(UI8_T *eeprom_p, FS_HW_Info_T *hwinfo_p);
static UI32_T FS_GetHwinfoFromONIEEeprom(FS_HW_Info_T *hwinfo_p);
#endif


/*
static  UI32_T  FS_WriteSpecialBlock(UI8_T type, UI8_T *buf, UI32_T size);
static  UI8_T   FS_MemberOfSpecialBlock(UI32_T block_id);
static  UI32_T  FS_MemberOfFile(UI32_T block_id, UI8_T* file_name);
static  UI32_T  FS_CalculateBlockByteSum(UI32_T unit, UI32_T block_id, UI8_T *sum);
static  UI32_T  FS_GetStorageInfo(UI32_T unit, UI32_T block_id, FS_FileInfo_T *file_info);
static  UI32_T  FS_ReadFileContent(UI32_T unit, UI32_T block_id, UI8_T *buf, UI32_T buf_size, UI32_T *read_count);
*/

#if (SYS_CPNT_STACKING == TRUE)
/*******************************************************
 * Functions for the master to call for slave services *
 *******************************************************/
static UI32_T  FS_ReadRemoteFile(UI32_T drive, FS_File_Attr_T *file_attr, UI32_T offset, UI32_T byte_num);
static UI32_T  FS_WriteRemoteFile(UI32_T drive, FS_File_Attr_T *file_attr);
static UI32_T  FS_GetRemoteFileSystemSummary(UI32_T drive, FS_File_Summary_Packet_T *file_summary);
#if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE)
static UI32_T  FS_GetRemoteStorageFreeSpaceForUpdateRuntime(UI32_T drive, UI32_T *free_space_p);
#endif
static UI32_T  FS_GetRemoteMultipleFileInfo(UI32_T drive, BOOL_T next, FS_File_Attr_Packet_T *file_attr_packet, UI32_T max_count);
static UI32_T  FS_GetRemoteStartupFilename(UI32_T drive, FS_File_Attr_Packet_T *file_attr_packet);
static UI32_T  FS_CopyRemoteFileContent(UI32_T drive, FS_File_Attr_T *file_attr, UI32_T offset, UI32_T byte_num);

static UI32_T  FS_AbortSequence(UI32_T drive);
static UI32_T  FS_CheckRemoteStatus(UI32_T drive);
static void    FS_ReturnIdle(void);

static UI32_T  FS_GetRemoteServiceCapability(UI32_T drive, UI32_T *capability);

/*******************************************************
 * Functions for the slave to process packets received *
 * from the master unit                                *
 *******************************************************/
static void FS_Service_NotSupport(ISC_Key_T *key);
static void FS_RemoteRead(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p);
static void FS_RemoteWrite(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p);
static void FS_RemoteSetAttr(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p);

static void FS_RemoteReceiveData(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p);
static void FS_RemoteSendData(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p);

static void FS_RemoteFlashFile(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p);
static void FS_RemoteCheckStatus(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p);
static void FS_RemoteGetFileSystemSummary(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p);
static void FS_RemoteGetStorageFreeSpaceForUpdateRuntime(ISC_Key_T * key, L_MM_Mref_Handle_T *mref_handle_p);
static void FS_RemoteGetMultipleFileInfo(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p);
static void FS_RemoteGetStartupFilename(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p);
static void FS_RemoteGetHWInfo(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p);
static void FS_RemoteAbort(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p);

static void FS_SendACK(ISC_Key_T *key);
static void FS_SendNAK(ISC_Key_T *key, UI32_T error_code);

/* Service callback function for 3 phases used in FS_WriteFileToMutipleUnit */
static void FS_RemoteMCastWrite(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p, BOOL_T *ret_value);
static void FS_RemoteMCastReceiveData(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p, BOOL_T *ret_value);
static void FS_RemoteMCastFlashFile(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p, BOOL_T *ret_value);

static void FS_RemoteGetServiceCapability(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p);

static void FS_RemoteCopyFileContent(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p);

/***************************
 * Debug and misc routines *
 ***************************/
static void FS_PrintHexChar(UI8_T *string, UI16_T char_count);
static void FS_PrintPacket(UI8_T *packet, UI16_T size);
#endif /* SYS_CPNT_STACKING */

//static BOOL_T FS_Strcat(const char* normal_str_p, const char* special_str_p, UI32_T special_str_buf_size, UI32_T output_str_buf_size, char* output_str_buf_p);

#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
static BOOL_T FS_InvokeAOSChangeStartupInstallerScript(const char* installer_filename_p);
#endif

#ifndef INCLUDE_DIAG
extern void SYS_TIME_GetUtcRealTimeBySec(UI32_T *seconds);
#else
#define SYS_TIME_GetUtcRealTimeBySec(seconds) ((void)(*(seconds) = SYSFUN_GetSysTick() / SYS_BLD_TICKS_PER_SECOND))
#endif

/************************************
 ***STATIC VARIABLE DECLARATIONS  ***
 ************************************/
static  UI32_T      FS_MaxNumOfFile[FS_FILE_TYPE_TOTAL] =
                    {   FS_MAX_NUM_OF_FILE_SUBFILE,
                        FS_MAX_NUM_OF_FILE_KERNEL,
                        FS_MAX_NUM_OF_FILE_DIAG,
                        FS_MAX_NUM_OF_FILE_RUNTIME,
                        FS_MAX_NUM_OF_FILE_SYSLOG,
                        FS_MAX_NUM_OF_FILE_CMDLOG,
                        FS_MAX_NUM_OF_FILE_CONFIG,
                        FS_MAX_NUM_OF_FILE_POSTLOG,
                        FS_MAX_NUM_OF_FILE_PRIVATE,
                        FS_MAX_NUM_OF_FILE_CERTIFICATE,
                        FS_MAX_NUM_OF_FILE_ARCHIVE,
                        FS_MAX_NUM_OF_FILE_BINARY_CONFIG,
                        FS_MAX_NUM_OF_FILE_PUBLIC,
                        FS_MAX_NUM_OF_FILE_CPEFIRMWARE,
                        FS_MAX_NUM_OF_FILE_CPECONFIG,
                        FS_MAX_NUM_OF_FILE_FILEMAPPING,
                        FS_MAX_NUM_OF_FILE_LICENSE,
                        FS_MAX_NUM_OF_FILE_NOS_INSTALLER
                    };

static  BOOL_T      FS_FileTypeVisibility[FS_FILE_TYPE_TOTAL]=
                    {   FS_FILE_TYPE_VISIBLE_SUBFILE,
                        FS_FILE_TYPE_VISIBLE_KERNEL,
                        FS_FILE_TYPE_VISIBLE_DIAG,
                        FS_FILE_TYPE_VISIBLE_RUNTIME,
                        FS_FILE_TYPE_VISIBLE_SYSLOG,
                        FS_FILE_TYPE_VISIBLE_CMDLOG,
                        FS_FILE_TYPE_VISIBLE_CONFIG,
                        FS_FILE_TYPE_VISIBLE_POSTLOG,
                        FS_FILE_TYPE_VISIBLE_PRIVATE,
                        FS_FILE_TYPE_VISIBLE_CERTIFICATE,
                        FS_FILE_TYPE_VISIBLE_ARCHIVE,
                        FS_FILE_TYPE_VISIBLE_BINARY_CONFIG,
                        FS_FILE_TYPE_VISIBLE_PUBLIC,
                        FS_FILE_TYPE_VISIBLE_CPEFIRMWARE,
                        FS_FILE_TYPE_VISIBLE_CPECONFIG,
                        FS_FILE_TYPE_VISIBLE_FILEMAPPING,
                        FS_FILE_TYPE_VISIBLE_LICENSE,
                        FS_FILE_TYPE_VISIBLE_NOS_INSTALLER,
                    };


#if (SYS_CPNT_STACKING == TRUE)
static FS_File_Attr_Packet_T   fs_file_attr_packet;
static FS_Rx_IscBuf_T          fs_packet_buffer;

static FS_Service_Func_T FS_Service_Table[] =
{/*-----------------------------------+---------------------------*
  * Function name                       Corresponding opcode      *
  *-----------------------------------+---------------------------*/
    FS_RemoteRead,                      /* FS_READ_REQUEST */
    FS_RemoteWrite,                     /* FS_WRITE_REQUEST */
    FS_RemoteSetAttr,                   /* FS_SET_ATTR_REQUEST */
    FS_RemoteReceiveData,               /* FS_DATA */
    FS_RemoteFlashFile,                 /* FS_PROGRAM_FLASH */
    FS_RemoteCheckStatus,               /* FS_FLASH_STATUS */
    FS_RemoteGetServiceCapability,      /* FS_GET_SERVICE_CAPABILITY */
    FS_RemoteAbort,                     /* FS_ABORT */
    FS_RemoteSendData,                  /* FS_ACK */
    NULL,                               /* FS_NAK - no service function */
    NULL,                               /* FS_SERVICE_NOT_SUPPORT - no service function */
    FS_RemoteMCastWrite,                /* FS_MCAST_WRITE_REQUEST */
    FS_RemoteMCastReceiveData,          /* FS_MCAST_DATA */
    FS_RemoteMCastFlashFile,            /* FS_MCAST_PROGRAM_FLASH */
    FS_RemoteGetHWInfo,                 /* FS_GET_HARDWARE_INFO */
    FS_RemoteCopyFileContent,           /* FS_COPY_FILE_CONTENT */
    FS_RemoteGetFileSystemSummary,      /* FS_GETFILESUMMARY */
    FS_RemoteGetMultipleFileInfo,       /* FS_GETMULTIPLEFILEINFO */
    FS_RemoteGetStartupFilename,        /* FS_GETSTARTUPFILENAME */
    NULL,                               /* FS_HAVEMOREDATA */
    FS_RemoteGetStorageFreeSpaceForUpdateRuntime, /* FS_GETSTORAGEFREESPACEFORUPDATERUNTIME */
};
#ifdef FS_DEBUG
static char *FS_Service_Names[] =
{
    "FS_RemoteRead",                    /* FS_READ_REQUEST */
    "FS_RemoteWrite",                   /* FS_WRITE_REQUEST */
    "FS_RemoteSetAttr",                 /* FS_SET_ATTR_REQUEST */
    "FS_RemoteReceiveData",             /* FS_DATA */
    "FS_RemoteFlashFile",               /* FS_PROGRAM_FLASH */
    "FS_RemoteCheckStatus",             /* FS_FLASH_STATUS */
    "FS_RemoteGetServiceCapability",    /* FS_GET_SERVICE_CAPABILITY */
    "FS_RemoteAbort",                   /* FS_ABORT */
    "FS_RemoteSendData",                /* FS_ACK */
    "",                                 /* FS_NAK - no service name */
    "",                                 /* FS_SERVICE_NOT_SUPPORT - no service name */
    "FS_RemoteMCastWrite",              /* FS_MCAST_WRITE_REQUEST */
    "FS_RemoteMCastReceiveData",        /* FS_MCAST_DATA */
    "FS_RemoteMCastFlashFile",          /* FS_MCAST_PROGRAM_FLASH */
    "FS_RemoteGetHWInfo",               /* FS_GET_HARDWARE_INFO */
    "FS_RemoteCopyFileContent",         /* FS_COPY_FILE_CONTENT */
    "FS_RemoteGetFileSystemSummary",    /* FS_GETFILESUMMARY */
    "FS_RemoteGetMultipleFileInfo",     /* FS_GETMULTIPLEFILEINFO */
    "FS_RemoteGetStartupFilename",      /* FS_GETSTARTUPFILENAME */
    "",                                 /* FS_HAVEMOREDATA */
    "FS_RemoteGetStorageFreeSpaceForUpdateRuntime", /* FS_GETSTORAGEFREESPACEFORUPDATERUNTIME */
};
#endif

static char *FS_Opcode_Names[] =
{
    "FS_READ_REQUEST",                  /* FS_RemoteRead */
    "FS_WRITE_REQUEST",                 /* FS_RemoteWrite */
    "FS_SET_ATTRIBUTE_REQUEST",         /* FS_RemoteSetAttr */
    "FS_DATA",                          /* FS_RemoteReceiveData */
    "FS_PROGRAM_FLASH",                 /* FS_RemoteFlashFile */
    "FS_FLASH_STATUS",                  /* FS_RemoteCheckStatus */
    "FS_GET_SERVICE_CAPABILITY",        /* FS_RemoteGetServiceCapability */
    "FS_ABORT",                         /* FS_RemoteAbort */
    "FS_ACK",                           /* FS_RemoteSendData */
    "FS_NAK",                           /* no service name */
    "FS_SERVICE_NOT_SUPPORT",           /* no service name */
    "FS_MCAST_WRITE_REQUEST",           /* FS_RemoteMCastWrite */
    "FS_MCAST_DATA",                    /* FS_RemoteMCastReceiveData */
    "FS_MCAST_PROGRAM_FLASH",           /* FS_RemoteMCastFlashFile */
    "FS_GET_HARDWARE_INFO",             /* FS_RemoteGetHWInfo */
    "FS_COPY_FILE_CONTENT",             /* FS_RemoteCopyFileContent */
    "FS_GETFILESUMMARY",                /* FS_RemoteGetFileSystemSummary */
    "FS_GETMULTIPLEFILEINFO",           /* FS_RemoteGetMultipleFileInfo */
    "FS_GETSTARTUPFILENAME",            /* FS_RemoteGetStartupFilename */
    "FS_HAVEMOREDATA"                   /* no service name */
    "FS_GETSTORAGEFREESPACEFORUPDATERUNTIME", /* FS_RemoteGetStorageFreeSpaceForUpdateRuntime */
};

static char *fs_error_messages[] =
{/***************************************
  * Error messages for fs return values *
  ***************************************/
    "Operation completed successfully\r\n", /* FS_RETURN_OK */
    "File truncated\r\n",                   /* FS_RETURN_FILE_TRUNCATED */
    "Specified file is a startup file\r\n", /* FS_RETURN_PROTECTED_FILE */
    "End of file reached.\r\n",             /* FS_RETURN_END_OF_FILE */
    "Action Inhibited\r\n",                 /* FS_RETURN_ACTION_INHIBITED */
    "File system error\r\n",                /* FS_RETURN_ERROR */
    "File system not ready\r\n",            /* FS_RETURN_NOT_READY */
    "Bad parameter\r\n",                    /* FS_RETURN_BAD_PARA */
    "Index out of range\r\n",               /* FS_RETURN_INDEX_OOR */
    "Index not exist\r\n",                  /* FS_RETURN_INDEX_NEX */
    "Specified file not exist\r\n",         /* FS_RETURN_FILE_NEX */
    "Not enough buffer space\r\n",          /* FS_RETURN_NO_BUFFER */
    "No such entryn\r\n",                   /* FS_RETURN_NO_ENTRY */
    "No such block\r\n",                    /* FS_RETURN_NO_BLOCK */
    "Hardware failed\r\n",                  /* FS_RETURN_BAD_HW */
    "Not in master mode\r\n",               /* FS_RETURN_NOT_MASTER */
    "Remote operation timed out\r\n",       /* FS_RETURN_TIMEOUT */
    "Remote drive disconnected\r\n",        /* FS_RETURN_DISCONNECTED */
    "Operation aborted\r\n",                /* FS_RETURN_ABORTED */
    "Drive not exist\r\n",                  /* FS_RETURN_DRIVE_NOT_EXIST */
    "Flash programming is in progress\r\n", /* FS_RETURN_FLASHING */
    "Packet sequence error\r\n",            /* FS_RETURN_PACKET_ERROR */
    "File system is in wrong state\r\n",    /* FS_RETURN_INCORRECT_STATE */
    "Undefined error\r\n",                  /* FS_RETURN_OTHERS */
    "Have more data\r\n",                   /* FS_RETURN_HAVEMOREDATA */
    "Remote service not support\r\n",       /* FS_RETURN_SERVICE_NOT_SUPPORT */
    "The quantity of the file has already exceeded\r\n" /* FS_RETURN_EXCEED_MAX_NUM_OF_FILES */
};

#endif /* (SYS_CPNT_STACKING == TRUE) */

#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
static const char op_code_directory[]=FS_SAVE_FOLDER;
#endif

/************************************
 ***      MACRO DEFINITIONS       ***
 ************************************/
#define FS_SETBUSY()        FS_LOCK(FS_RETURN_NOT_READY)
#define FS_SETNOTBUSY()     FS_UNLOCK()

#if (SYS_CPNT_STACKING == TRUE)
#define DBGMSG(x)           if(FS_OM_GetDebugFlag()==TRUE)                      \
                                BACKDOOR_MGR_Printf("%s() Debug: %s\r\n", __FUNCTION__, (x))

#define DBGMSG_EX(...)      if(FS_OM_GetDebugFlag()==TRUE)                      \
                                BACKDOOR_MGR_Printf(__VA_ARGS__);

#define ERRMSG(x)           if(FS_OM_GetDebugFlag()==TRUE)                      \
                                BACKDOOR_MGR_Printf("%s() Error: %s\r\n", __FUNCTION__, fs_error_messages[(x)] )

#define GET_PDU_LEN(x)      (sizeof((x).header) + (x).header.data_size)

#define DUMP_HEADER(x)      if(FS_OM_GetDebugFlag()==TRUE)                                     \
                            {                                                                  \
                                BACKDOOR_MGR_Printf("\r\n%s() Send:", __FUNCTION__);           \
                                FS_PrintPacket( (UI8_T *) &(x), (sizeof(FS_Packet_Header_T) ); \
                            }

#define DUMP_TXPKT(x, y)    if(FS_OM_GetDebugFlag()==TRUE)                           \
                            {                                                        \
                                BACKDOOR_MGR_Printf("\r\n%s() Send:", __FUNCTION__); \
                                FS_PrintPacket( (UI8_T *) &(x), (y) );               \
                            }

#define DUMP_RXPKT(x, y)    if(FS_OM_GetDebugFlag()==TRUE)                               \
                            {                                                            \
                                BACKDOOR_MGR_Printf("\r\n%s() Received:", __FUNCTION__); \
                                FS_PrintPacket( (UI8_T *) &(x), (y) );                   \
                            }

#define FS_LOCK_REMOTE()    {   DBGMSG("LOCK REMOTE!"); \
                                SYSFUN_TakeSem(FS_OM_GetCommSemaphore(), SYSFUN_TIMEOUT_WAIT_FOREVER); }
#define FS_UNLOCK_REMOTE()  {   DBGMSG("UNLOCK REMOTE!");   \
                                SYSFUN_GiveSem(FS_OM_GetCommSemaphore());}
#else /* (SYS_CPNT_STACKING == TRUE) */
#define DBGMSG(x)           ;
#define DBGMSG_EX(...)      ;
#define ERRMSG(x)           ;
#define GET_PDU_LEN(x)      ;
#define DUMP_HEADER(x)      ;
#define DUMP_TXPKT(x, y)    ;
#define DUMP_RXPKT(x, y)    ;
#define FS_LOCK_REMOTE()    ;
#define FS_UNLOCK_REMOTE()  ;

#endif  /* (SYS_CPNT_STACKING == TRUE) */

#if ((SYS_HWCFG_LOADER_FLASH_TYPE & SYS_HWCFG_FLASH_TYPE_NOR)!=0)
#define FS_READ_LOADER_FROM_FLASH FS_ReadNorFlash
#define FS_WRITE_LOADER_TO_FLASH  FS_WriteNorFlash
#elif ((SYS_HWCFG_LOADER_FLASH_TYPE & SYS_HWCFG_FLASH_TYPE_NAND)!=0)
#define FS_READ_LOADER_FROM_FLASH FS_ReadNandFlash
#define FS_WRITE_LOADER_TO_FLASH  FS_WriteNandFlash
#else
#error "Incorrect SYS_HWCFG_LOADER_FLASH_TYPE"
#endif

#if ((SYS_HWCFG_HWINFO_FLASH_TYPE & SYS_HWCFG_FLASH_TYPE_NOR)!=0)
#define FS_READ_HWINFO_FROM_FLASH FS_ReadNorFlash
#define FS_WRITE_HWINFO_TO_FLASH  FS_WriteNorFlash
#elif ((SYS_HWCFG_HWINFO_FLASH_TYPE & SYS_HWCFG_FLASH_TYPE_NAND)!=0)
#define FS_READ_HWINFO_FROM_FLASH FS_ReadNandFlash
#define FS_WRITE_HWINFO_TO_FLASH  FS_WriteNandFlash
#else
#error "Incorrect SYS_HWCFG_HWINFO_FLASH_TYPE"
#endif

#if ((SYS_HWCFG_RUNTIME_FLASH_TYPE & SYS_HWCFG_FLASH_TYPE_NOR)!=0)
#define FS_READ_RUNTIME_FROM_FLASH FS_ReadNorFlash
#define FS_WRITE_RUNTIME_TO_FLASH  FS_WriteNorFlash
#elif ((SYS_HWCFG_RUNTIME_FLASH_TYPE & SYS_HWCFG_FLASH_TYPE_NAND)!=0)
#define FS_READ_RUNTIME_FROM_FLASH FS_ReadNandFlash
#define FS_WRITE_RUNTIME_TO_FLASH  FS_WriteNandFlash
#else
#error "Incorrect SYS_HWCFG_RUNTIME_FLASH_TYPE"
#endif

#if !defined(INCLUDE_DIAG) && (SYS_CPNT_FS_SUPPORT_UBIFS==TRUE && SYS_HWCFG_RUNTIME_FLASH_TYPE==SYS_HWCFG_FLASH_TYPE_NAND)
static void FS_LocalDummyReadRuntimeFile(void);
#endif

#if (SYS_CPNT_STACKING == TRUE)
static  void    FS_TASK_Main(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - FS_TASK_CreateTask
 * ------------------------------------------------------------------------
 * FUNCTION : Create the fs task.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void FS_TASK_CreateTask(void)
{
    UI32_T      task_id;
    if(SYSFUN_SpawnThread(SYS_BLD_FLSHDRV_TASK_PRIORITY,
                          SYSFUN_SCHED_DEFAULT,
                          SYS_BLD_FS_TASK,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          FS_TASK_Main,
                          NULL,
                          &task_id)!=SYSFUN_OK)

    {
        task_id                 = 0;
        /* Need to log this event in the future implementation */
        ERRORSTR("\r\n%s()::(line %d) Failed in creating task\r\n", __FUNCTION__, __LINE__);
    }
    SYSFUN_Sleep_Interruptible(5);;
    FS_OM_SetTaskId(task_id);

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#ifndef INCLUDE_DIAG
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_FLASHDRV, task_id,SYS_ADPT_FLASHDRV_SW_WATCHDOG_TIMER);
#endif
#endif

    return;
}/* End of FS_TASK_CreateTask()*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - FS_TASK_Init
 *-------------------------------------------------------------------------
 * PURPOSE  : Init the FS task
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void FS_TASK_Init(void)
{
    SYSFUN_MsgQ_T msgq_id;

    /* Create FS message queue
     * FS MsgQ: 64 messages of 16 bytes which contain the pointer to the packet buffer
     */
    SYSFUN_CreateMsgQ(FS_TASK_MAX_MSGQ_LEN, SYSFUN_MSG_FIFO, &msgq_id);
//    FS_TASK_MsgqId          = msgq_id;

    return;
} /* End of FS_TASK_Init() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - FS_TASK_Main
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize FS function
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static  void    FS_TASK_Main(void)
{
    UI32_T                          events, all_events;
    UI32_T                          timeout;
#if !defined(INCLUDE_DIAG) && (SYS_CPNT_FS_SUPPORT_UBIFS==TRUE && SYS_HWCFG_RUNTIME_FLASH_TYPE==SYS_HWCFG_FLASH_TYPE_NAND)
    void*                           timer_handle;
#endif
#if 0 /* Still under construction */
    UI32_T                          msgq_id;
    FS_TYPE_MSG_T                   msg;
    UI32_T                          current_mode;
#endif

#if !defined(INCLUDE_DIAG) && (SYS_CPNT_FS_SUPPORT_UBIFS==TRUE && SYS_HWCFG_RUNTIME_FLASH_TYPE==SYS_HWCFG_FLASH_TYPE_NAND)
    timer_handle = SYSFUN_PeriodicTimer_Create();

    if(timer_handle==NULL)
    {
        SYSLOG_OM_Record_T  syslog_entry;

        memset((UI8_T *)&syslog_entry, 0, sizeof(SYSLOG_OM_Record_T));
        syslog_entry.owner_info.level = SYSLOG_LEVEL_ERR;
        syslog_entry.owner_info.module_no = SYS_MODULE_FS;
        syslog_entry.owner_info.function_no = 0;
        syslog_entry.owner_info.error_no = 0;
        sprintf((char*)(syslog_entry.message), "FS_TASK_Main timer create failed");
        SYSLOG_PMGR_AddEntry(&syslog_entry);
        BACKDOOR_MGR_Printf("FS_TASK_Main timer create failed\r\n");
    }
    else
    {
        if(SYSFUN_PeriodicTimer_Start(timer_handle, FS_READRUNTIMERFILE_PERIODIC_TICKS, FS_TYPE_EVENT_DUMMY_READRUNTIMEFILE)==FALSE)
        {
            SYSLOG_OM_Record_T  syslog_entry;

            memset((UI8_T *)&syslog_entry, 0, sizeof(SYSLOG_OM_Record_T));
            syslog_entry.owner_info.level = SYSLOG_LEVEL_ERR;
            syslog_entry.owner_info.module_no = SYS_MODULE_FS;
            syslog_entry.owner_info.function_no = 0;
            syslog_entry.owner_info.error_no = 0;
            sprintf((char*)(syslog_entry.message), "FS_TASK_Main failed to start timer");
            SYSLOG_PMGR_AddEntry(&syslog_entry);
            BACKDOOR_MGR_Printf("FS_TASK_Main failed to timer failed\r\n");
        }
    }
#endif

    all_events =
#if (SYS_CPNT_FS_SUPPORT_UBIFS==TRUE && SYS_HWCFG_RUNTIME_FLASH_TYPE==SYS_HWCFG_FLASH_TYPE_NAND)
                 /* Force a read runtime file event in the beginning,
                  * in some case(such as DUT used by NTC or EIT) the DUT might
                  * not stay in runtime for enough time to trigger a read runtime
                  * file event.
                  */
                 FS_TYPE_EVENT_DUMMY_READRUNTIMEFILE |
#endif
                 FS_TYPE_EVENT_NONE;

    while(1)
    {
        /* Three events will be handled : 1.Timer; 2.BPDU Received; 3.Enter Transition 4.Callback */
        if (all_events)
            timeout = (UI32_T)SYSFUN_TIMEOUT_NOWAIT;
        else
            timeout = (UI32_T)SYSFUN_TIMEOUT_WAIT_FOREVER;
        SYSFUN_ReceiveEvent(FS_TYPE_EVENT_ALL,
                            SYSFUN_EVENT_WAIT_ANY,
                            timeout,
                            &events);

        all_events |= events;
        all_events &= (FS_TYPE_EVENT_LEGACYFS|FS_TYPE_EVENT_CHECKFS|FS_TYPE_EVENT_REMOTECALL|
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#ifndef INCLUDE_DIAG
                       SYSFUN_SYSTEM_EVENT_SW_WATCHDOG |
#endif
#endif
#if !defined(INCLUDE_DIAG) && (SYS_CPNT_FS_SUPPORT_UBIFS==TRUE && SYS_HWCFG_RUNTIME_FLASH_TYPE==SYS_HWCFG_FLASH_TYPE_NAND)
                       /* If bit-flip occurs on a block of NAND flash, it is
                        * better to do scrub to avoid more bit-flip occurs and
                        * lead to the data on the block cannot be corrected finally
                        * (scrub means to move data from old block contains bit-flip
                        *  data to a new block contains all correct data)
                        * It is found that if the file is not accessed, UBIFS
                        * will never be able to detect bit-flip data and do scrub.
                        * Thus, we will do read on runtime file periodically to
                        * trigger UBIFS to detect bit-flip and do scrub.
                        */
                       FS_TYPE_EVENT_DUMMY_READRUNTIMEFILE |
#endif
                       FS_TYPE_EVENT_REPLY);

        if (all_events)
        {
            /* 1. Handle legacy remote update */
            if (all_events & FS_TYPE_EVENT_LEGACYFS)
            {
                FS_RemoteFlashTaskMain();
                all_events &= ~FS_TYPE_EVENT_LEGACYFS;
            } /* End of if*/

            /* 2. Handle the event for checking the files in the file system */
            if (all_events & FS_TYPE_EVENT_CHECKFS)
            {
                FS_LocalCheckFileSystem();
                all_events &= ~FS_TYPE_EVENT_CHECKFS;
            } /* End of if*/

#if !defined(INCLUDE_DIAG) && (SYS_CPNT_FS_SUPPORT_UBIFS==TRUE && SYS_HWCFG_RUNTIME_FLASH_TYPE==SYS_HWCFG_FLASH_TYPE_NAND)
            if (all_events & FS_TYPE_EVENT_DUMMY_READRUNTIMEFILE)
            {
                DEBUGSTR("%s(%d)Start read runtime files.\r\n", __FUNCTION__, __LINE__);
                FS_LocalDummyReadRuntimeFile();
                all_events &= ~FS_TYPE_EVENT_DUMMY_READRUNTIMEFILE;
            }

#endif

            #if 0 /* Still under construction */
            /* 3. Handle the event for the remote operation request */
            if (all_events & FS_TYPE_EVENT_REMOTECALL)
            {
                msgq_id     = FS_TASK_MsgqId;

                if (SYSFUN_ReceiveMsgQ((UI32_T)msgq_id, (UI32_T*)&msg, SYSFUN_TIMEOUT_NOWAIT) != SYSFUN_RESULT_NO_MESSAGE)
                {
                }
                else
                {
                    all_events &= ~FS_TYPE_EVENT_REMOTECALL;
                }
            } /* End of if*/
            /* 4. Handle the event for replying the remote operation */
            if (all_events & FS_TYPE_EVENT_REPLY)
            {
                msgq_id     = FS_TASK_MsgqId;

                if (SYSFUN_ReceiveMsgQ((UI32_T)msgq_id, (UI32_T*)&msg, SYSFUN_TIMEOUT_NOWAIT) != SYSFUN_RESULT_NO_MESSAGE)
                {
                }
                else
                {
                    all_events &= ~FS_TYPE_EVENT_REPLY;
                }
            } /* End of if*/
            #endif

        } /* End of if (all_events) */

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#ifndef INCLUDE_DIAG
       if (all_events & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
       {
           SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_FLASHDRV);
           all_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
       }
#endif
#endif

    } /* End of while */
} /* End of FS_TASK_Main() */
#endif

#if !defined(INCLUDE_DIAG) && (SYS_CPNT_FS_SUPPORT_UBIFS==TRUE && SYS_HWCFG_RUNTIME_FLASH_TYPE==SYS_HWCFG_FLASH_TYPE_NAND)
/* FUNCTION NAME: FS_LocalDummyReadRuntimeFile
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will trigger reading all of the runtime files
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES: On UBIFS, if a bit-flip block is detected, it will do scrub(moving block
 *        with incorrect data to a new block with all correct data). However,
 *        if there is no read access to the block, the bit-flip condition on the
 *        block will never be detected and scrub will never do. So FS task will
 *        trigger a read operation to all of the runtime by calling this function
 *        periodically.
 */
static void FS_LocalDummyReadRuntimeFile(void)
{
#if 1
    FILE           *fd;
    FS_FileHeader_T file_header;
    char            runtime_path[FS_MAX_PATHNAME_BUF_SIZE]={'\0'};
    UI8_T          *buf_p;
    size_t          read_len, total_len;

    buf_p = malloc(DUMMY_READ_RUNTIME_BUF_SIZE);
    if(buf_p==NULL)
    {
        DEBUGSTR("%s(%d)malloc %d bytes failed\r\n", __FUNCTION__, __LINE__,
            DUMMY_READ_RUNTIME_BUF_SIZE);
        return;
    }

    memset(&file_header, 0, sizeof(FS_FileHeader_T));
    file_header.file_type=FS_TYPE_OPCODE_FILE_TYPE;

    if(FS_OM_GetFirstFileHeaderByType(&file_header)==FALSE)
    {
        DEBUGSTR("%s(%d)No runtime file is found in FS_OM.\r\n", __FUNCTION__, __LINE__);
        goto out_free;
    }

    FS_SEMAPHORE_LOCK();
    do
    {
        total_len=0;
        snprintf(runtime_path, FS_MAX_PATHNAME_BUF_SIZE, "%s%s", FS_SAVE_FOLDER, file_header.file_name);
        fd = fopen(runtime_path, "r");
        if(fd==NULL)
        {
            DEBUGSTR("%s(%d):Failed to read file %s\r\n", __FUNCTION__, __LINE__,
                runtime_path);
            continue;
        }

        while( (read_len = fread(buf_p, 1, DUMMY_READ_RUNTIME_BUF_SIZE, fd)) == DUMMY_READ_RUNTIME_BUF_SIZE)
            total_len+=read_len;

        total_len+=read_len;

        fclose(fd);

    }while(FS_OM_GetNextFileHeaderByName(&file_header, FS_FILE_TYPE_MASK(FS_TYPE_OPCODE_FILE_TYPE))==TRUE);
    FS_SEMAPHORE_UNLOCK();

out_free:
    free(buf_p);
    return;

#else
    /* EPRID: ES4627MB-FLF-EC-00068
     * 2011/3/9
     * Headline: driver_proc exception occurs occasionally in the middle of booting runtime
     * use "cp /flash/.fs/runtime.bix /tmp" to copy file to ramdisk when pthread is spawning new threads
     * could lead to unexpected exception in pthread lib function> It is doubed that linux kernel
     * might have bug when doing thread stack allocation and writing data to ramdisk at the same time.
     * This code snippet is kept for bug track.
     */
    FS_FileHeader_T file_header;
    char            runtime_path[FS_MAX_PATHNAME_BUF_SIZE]={'\0'};
    char           *system_str_p;
    UI32_T          system_str_len;

    /* system str will have the cases shown below:
     * "cp /flash/.fs/runtime.bix /tmp/"
     * "rm /tmp/runtime.bix"
     *
     * cp command will have longer string, so evaluate the required string len
     * based on cp command.
     */
    system_str_len=3 + /* for 'cp ' */
                   sizeof(runtime_path) + /* for '/flash/.fs/runtime.bix' */
                   1 /* for one space */ + strlen(FS_TMP_FOLDER) /* for ' /tmp/'*/;

    system_str_p = malloc(system_str_len+1); /* +1 for terminating null char */
    if(system_str_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)malloc error for %lu bytes.\r\n", __FUNCTION__, __LINE__, system_str_len+1);
        return;
    }
    memset(system_str_p, 0, system_str_len+1);
    memset(&file_header, 0, sizeof(FS_FileHeader_T));
    file_header.file_type=FS_TYPE_OPCODE_FILE_TYPE;

    if(FS_OM_GetFirstFileHeaderByType(&file_header)==FALSE)
    {
        DEBUGSTR("%s(%d)No runtime file is found in FS_OM.\r\n", __FUNCTION__, __LINE__);
        goto out_free;
    }

    do
    {
        snprintf(runtime_path, FS_MAX_PATHNAME_BUF_SIZE, "%s%s", FS_SAVE_FOLDER, file_header.file_name);

        /* do "cp /flash/.fs/runtime.bix /tmp")
         */
        snprintf(system_str_p, system_str_len+1, "cp %s %s", runtime_path, FS_TMP_FOLDER);
        system(system_str_p);
        /* do "rm /tmp/runtime.bix"
         */
        snprintf(system_str_p, system_str_len+1, "rm %s%s", FS_TMP_FOLDER, file_header.file_name);
        system(system_str_p);
        DEBUGSTR("%s(%d) read runtime file:%s\r\n", __FUNCTION__, __LINE__, file_header.file_name);
    }while(FS_OM_GetNextFileHeaderByName(&file_header, FS_FILE_TYPE_MASK(FS_TYPE_OPCODE_FILE_TYPE))==TRUE);

out_free:
    free(system_str_p);
    return;
#endif
}
#endif /* end of #if (SYS_CPNT_FS_SUPPORT_UBIFS==TRUE && SYS_HWCFG_RUNTIME_FLASH_TYPE==SYS_HWCFG_FLASH_TYPE_NAND) */

#ifndef INCLUDE_DIAG
/* FUNCTION NAME: FLASHDRV_Initiate_System_Resources
 *-----------------------------------------------------------------------------
 * PURPOSE: initializes all resources for FLASHDRV
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
void FS_InitiateSystemResources(void)
{
    FS_OM_InitiateSystemResources();
    FS_Init();
#if (SYS_CPNT_STACKING == TRUE)
    FS_TASK_Init();
#endif
    return;

}  /* End of FS_InitiateSystemResources() */

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: FS_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for FLASHDRV in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void FS_AttachSystemResources(void)
{
    FS_OM_AttachSystemResources();
    return;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: FS_INIT_GetShMemInfo
 *---------------------------------------------------------------------------------
 * PURPOSE: Provide shared memory information for FLASHDRV.
 * INPUT:   None
 * OUTPUT:  segid_p  --  shared memory segment id
 *          seglen_p --  length of the shared memroy segment
 * RETUEN:  None
 * NOTES:
 *    This function is called in SYSRSC_MGR_CreateSharedMemory().
 *---------------------------------------------------------------------------------
 */

void FS_INIT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    FS_OM_INIT_GetShMemInfo(segid_p, seglen_p);
    return;
}
#endif

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: FS_BuildFileHeaderFromFile
 *---------------------------------------------------------------------------------
 * PURPOSE: save file header to OM
 * INPUT:   file_name - the device file name
 * OUTPUT:  
 * RETUEN:  None
 * NOTES:
 *    This function is called in SYSRSC_MGR_CreateSharedMemory().
 *---------------------------------------------------------------------------------
 */
/*this function process files in file system */
static BOOL_T FS_BuildFileHeaderFromFile(char file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1])
{
    struct stat     file_info;
    FS_FileHeader_T *file_header_ptr;

    char name[FS_MAX_PATHNAME_BUF_SIZE + SYS_ADPT_FILE_SYSTEM_NAME_LEN + 1]={'\0'};

#if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE)
    FS_PARTITION_TABLE_T *flash_entry_p = NULL;
#else
    UI32_T primitive_fs_mtd_id;
#endif

    sprintf((char*)name,"%s%s",FS_SAVE_FOLDER,file_name);
    if( (stat((char*)name, &file_info)!=0) || (!S_ISREG(file_info.st_mode)) )
    {
        if( S_ISDIR(file_info.st_mode) )
        {
            DEBUGSTR("%s: Has unknown folder, delete it(%s)\r\n",__FUNCTION__,name);
        }
        else
        {
            DEBUGSTR("%s: Has unknown file %s(%x), delete it\r\n",__FUNCTION__,name, file_info.st_mode);
            unlink((char *)name);
        }
        return FALSE;
    }

    if (FS_UID_IS_CURRENTMODE(file_info.st_uid) == FALSE)
    {
        return FALSE;
    }   

    if (FS_GenericFilenameCheck((UI8_T*)file_name, FS_UID_TO_TYPE(file_info.st_uid)) != FS_RETURN_OK)
    {
        DEBUGSTR("%s:ERROR, FS_GenericFilenameCheck(\"%s\", %d)return fail,delete it\r\n",
            __FUNCTION__,file_name, FS_UID_TO_TYPE(file_info.st_uid));
        unlink((char *)name);
        return FALSE;
    }

    /* Create file header and insert into header list
     */
    if((file_header_ptr = (FS_FileHeader_T*)FS_OM_AllocateFileHeaderBuffer()) == NULL)
    {
        FS_OM_SetInitStatus(FS_RETURN_NO_BUFFER);
        return FALSE;
    }

    FS_RemoveDeviceFileNameMark(file_name, file_header_ptr->file_name, FS_UID_TO_TYPE(file_info.st_uid));
    strncpy((char*)file_header_ptr->file_comment, file_name, FS_FILE_COMMENT_LENGTH);
    file_header_ptr->file_comment[FS_FILE_COMMENT_LENGTH]='\0';
    file_header_ptr->file_type    = FS_UID_TO_TYPE(file_info.st_uid);
    file_header_ptr->file_size    = file_info.st_size;
    file_header_ptr->startup      = FS_UID_TO_STARTUP(file_info.st_uid);
#if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE)
    while (NULL != (flash_entry_p = FS_GetNextFlashPartitionEntry(flash_entry_p)))
    {
        if(flash_entry_p->type == TYPE_USERDATA)
        {
            file_header_ptr->mtd_num = flash_entry_p->mtd_num;
            break;
        }
    }
#else
    if(FS_GetMtdDevIdByMtdType(FS_MTD_PART_PRIMITIVE_FS, &primitive_fs_mtd_id)==FALSE)
    {
        primitive_fs_mtd_id=FS_TYPE_INVALID_MTD_ID;
    }

    file_header_ptr->mtd_num=primitive_fs_mtd_id; /* cfg or runtime is saved to a file, so will not use mtd num */
#endif
    file_header_ptr->create_time  = file_info.st_mtime;
    file_header_ptr->magic_number = FS_SPECIAL_MAGIC_NUMBER;

    FS_OM_AddFileHeader(file_header_ptr);

    if (file_header_ptr->startup)
    {
        FS_OM_SetStartupName(file_header_ptr->file_type, file_header_ptr->file_name);
    }

    return TRUE;
}

static BOOL_T FS_BuildAllFileHeader(void)
{
    DIR*                   dir;
    struct dirent          *file_entry;

    /*1.read .fs directory  build other  file(log file config file ....) header*/
    dir = opendir(FS_SAVE_FOLDER);
    if( dir == NULL )
    {
        DEBUGSTR("%s: First time open directory: %s not exist, create it.\r\n",__FUNCTION__,FS_SAVE_FOLDER);
        if( (mkdir(FS_SAVE_FOLDER, 0666) != 0) ||
            ((dir = opendir(FS_SAVE_FOLDER))== NULL) )
        {
            FS_OM_SetInitStatus(FS_RETURN_NOT_READY);
            ERRORSTR("%s: Can not open directory: %s\r\n",__FUNCTION__,FS_SAVE_FOLDER);
            return FALSE;
        }
    }

    while( (file_entry = readdir(dir)) != NULL )
    {
        DEBUGSTR("Build file list: %s\r\n",file_entry->d_name);

        /* Skip . and ..
         */
        if( (strcmp(file_entry->d_name, ".")==0) ||
            (strcmp(file_entry->d_name, "..")==0) )
        {
            continue; /* Skip these files */
        }

        FS_BuildFileHeaderFromFile(file_entry->d_name);
    }
    closedir(dir);


    DEBUGSTR("\r\n%s:line %d\r\n",__FUNCTION__, __LINE__);
    return TRUE;
}

#if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE)
/*read partition table partition  and save in local static data struct*/
void FS_PartitionTableInit()
{
    int fd;
    int read_len, i;
    AMS_PARTITION_TABLE_T  *partition_table;

    fd = open(FS_FLASH_MTD_BLOCK_PARTITION_TABLE, O_RDONLY, 0666);
    if( fd == -1 )
    {
        ERRORSTR("%s:ERROR!! Can not open %s!!\r\n", __FUNCTION__, FS_FLASH_MTD_BLOCK_PARTITION_TABLE);
        return ;
    }

    partition_table = (AMS_PARTITION_TABLE_T *)malloc(sizeof(AMS_PARTITION_TABLE_T));
    if(partition_table==NULL)
    {
        DEBUGSTR("Get filemapping table error\n");
        return;
    }
    read_len = read(fd, (char *)partition_table, sizeof(AMS_PARTITION_TABLE_T));
    if( read_len < sizeof(AMS_PARTITION_TABLE_T) )
    {
        DEBUGSTR("%s: read size %d < %d!!\r\n", __FUNCTION__, read_len, sizeof(AMS_PARTITION_TABLE_T));
    }
    if(partition_table->magic != PARTITION_TABLE_MAGIC)
    {
        BACKDOOR_MGR_Printf("%s(%d): Bad magic in partition table!! Halt.\r\n", __FUNCTION__, __LINE__);
        while(1);
    }
    for(i=0; i< MAX_PARTITION_NUMBER; i++)
    {
        strncpy((char *)fs_partition_table[i].name, (const char *)partition_table->flash_region[i].region_name, FLASH_REGION_NAMELEN);
        fs_partition_table[i].mtd_num = i;
        fs_partition_table[i].base = partition_table->flash_region[i].region_base;
        fs_partition_table[i].size = partition_table->flash_region[i].region_size;
        fs_partition_table[i].type = partition_table->flash_region[i].region_type;
    }
    free(partition_table);
    close(fd);
}

UI32_T FS_GetNumOfPartitionByTypeFromPartitionTable(UI32_T partition_type)
{
    UI32_T i,count;

    if(partition_type>=TYPE_MAX)
    {
        BACKDOOR_MGR_Printf("%s(%d) Invalid partition type %lu\r\n", __FUNCTION__, __LINE__, (unsigned long)partition_type);
        return 0;
    }

    for(i=0,count=0; i<MAX_PARTITION_NUMBER; i++)
    {
        if(fs_partition_table[i].name[0] == '\0')
            break;

        if(fs_partition_table[i].type==partition_type)
            count++;
    }
    return count;
}
#endif

#if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE)
static FS_PARTITION_TABLE_T *FS_GetNextFlashPartitionEntry(FS_PARTITION_TABLE_T *flash_entry_p)
{

    if (flash_entry_p == NULL) /* get first entry */
    {
        return &(fs_partition_table[0]);
    }
    else
    {
        if (((UI32_T)flash_entry_p<(UI32_T)(&(fs_partition_table[0]))) ||
            ((UI32_T)flash_entry_p>=(UI32_T)(&(fs_partition_table[MAX_PARTITION_NUMBER-1]))) ||
              (((FS_PARTITION_TABLE_T*)(((UI8_T*)flash_entry_p)+sizeof(FS_PARTITION_TABLE_T)))->name[0] == '\0'))
        {
            return NULL;
        }
        return (FS_PARTITION_TABLE_T*)(((UI8_T*)flash_entry_p)+sizeof(FS_PARTITION_TABLE_T));
    }
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetFreeRuntimePartitionInfo
 * ------------------------------------------------------------------------
 * FUNCTION : This function check the image header product id
 * INPUT    : None
 *
 * OUTPUT   : runtime_part_len_p  -- runtime partition size
 *            free_part_num_p     -- numbers of free runtime partition
 * RETURN   : TRUE       --
 *            FALSE      --
 * NOTE     : If find no free runtime partition, runtime_part_len_p will be 0.
 * ------------------------------------------------------------------------
 */
static BOOL_T FS_GetFreeRuntimePartitionInfo(UI32_T *runtime_part_len_p, UI32_T *free_part_num_p)
{
    AMS_FILEMAPPING_MGR_T *fmp_table;
    AMS_FILEMAPPING_T *filemap_entry;
    int i;
    BOOL_T ret_val=TRUE;

    *runtime_part_len_p=*free_part_num_p=0;

    fmp_table = (AMS_FILEMAPPING_MGR_T *)malloc(sizeof(AMS_FILEMAPPING_MGR_T));
    if(fmp_table==NULL)
    {
        ERRORSTR("%s:ERROR!! malloc failed.\r\n", __FUNCTION__);
        return FALSE;
    }

    if(FS_GetFileMappingTable(fmp_table) == FS_RETURN_ERROR)
    {
        ERRORSTR("%s:ERROR!! Get File Mapping table fail !!\r\n", __FUNCTION__);
        ret_val=FALSE;
        goto exit;
    }
    /* count number of free partition
     */
    for(i=0; i< MAX_PARTITION_NUMBER; i++)
    {
        filemap_entry = &fmp_table->filemapping[i];

        if(filemap_entry->file_type == TYPE_RUNTIME
            && filemap_entry->startup_flag == AMS_PARTITION_FREE)
        {
            (*free_part_num_p)++;
            /* output the runtime partition size of the minimum one
             */
            if ((*runtime_part_len_p)==0 || (*runtime_part_len_p)>fs_partition_table[i].size)
            {
                (*runtime_part_len_p)=fs_partition_table[i].size;
            }

        }
    }
exit:
    free(fmp_table);
    return ret_val;
}
#endif

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_Init
 * ------------------------------------------------------------------------
 * FUNCTION : This function will initialize the file system
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void FS_Init(void)
{
    BOOL_T result;

    FS_OM_InitateVariables();
#if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE)
    FS_PartitionTableInit();
#endif
    DEBUGSTR("%s before FS_BuildFileHeader().\r\n",__FUNCTION__);

    result = FS_BuildAllFileHeader();

    if( result == FALSE )
    {
        BACKDOOR_MGR_Printf("FS_Init::ERROR!! for FS_BuildFileHeader return FALSE %lx\r\n",(unsigned long)FS_OM_GetInitStatus());
#if FS_DUMMY_WRITE
        FS_OM_SetInitStatus(FS_RETURN_NOT_READY);
#else
        while(1);
#endif
    }

    //FS_StartupVerification();   -------need check

    DEBUGSTR("%s is done.\r\n",__FUNCTION__);
    return;
} /* End of FS_Init */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_Create_InterCSC_Relation
 * ------------------------------------------------------------------------
 * FUNCTION : This function initializes all function pointer registration operations.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */

void    FS_Create_InterCSC_Relation(void)
{
// Wally, should we implement FS backdoor function
//    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("fs", FS_BACKDOOR_Main);
#ifndef INCLUDE_DIAG
     BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("fs", SYS_BLD_DRIVER_GROUP_IPCMSGQ_KEY, FS_BackDoor_Menu);
#endif
} /* End of FS_Create_InterCSC_Relation */

UI32_T  FS_FilenameCheck(UI8_T *filename)
{
    if(L_CHARSET_IsValidFileNameString((const char *)filename))
        return FS_RETURN_OK;
    return FS_RETURN_ERROR;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GenericFilenameCheck
 * ------------------------------------------------------------------------
 * FUNCTION : This function checks the file name to be composed with the valid
 *            characters or not. As for an invisible file name it should be
 *            composed with the prefixed character.
 * INPUT    : filename      -- a string specified by users
 *            file_type     -- file type
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E;
 *            FS_RETURN_OK      -- the input file name is valid
 *            FS_RETURN_ERROR   -- the input file name is invalid
 * NOTE     : FS_RETURN_ERROR returned if filename is a NULL string
 * ------------------------------------------------------------------------
 */
UI32_T  FS_GenericFilenameCheck(UI8_T *filename, UI32_T file_type)
{
    char  ch;

    ch = FS_FILENAME_CHECK_VALID_INVIS;

    if(file_type <= FS_FILE_TYPE_SUBFILE || file_type >= FS_FILE_TYPE_TOTAL)
        return FS_RETURN_ERROR;

    if(filename == NULL || filename[0] == 0 || (strlen((char*)filename) > FS_MAX_FILE_NAME_LENGTH))
        return FS_RETURN_ERROR;



    if ( FS_FileTypeVisibility[file_type] )
    {
        if ( FS_FilenameCheck( filename ) != FS_RETURN_OK )
        return FS_RETURN_ERROR;
    }
    else
    {
        if(filename[0] != ch)
            return FS_RETURN_ERROR;

        if ( FS_FilenameCheck( (filename+1)) != FS_RETURN_OK )
            return FS_RETURN_ERROR;
    }
    return FS_RETURN_OK;
} /* End of FS_GenericFilenameCheck */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_CheckImageHeaderProductId
 * ------------------------------------------------------------------------
 * FUNCTION : This function check the image header product id
 * INPUT    : data_p     -- pointer to the image header
 *            length     -- data length
 * OUTPUT   : None
 * RETURN   : TRUE       -- product id check pass
 *            FALSE      -- product id check fail
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T  FS_CheckImageHeaderProductId(UI8_T *data_p, UI32_T length)
{
#if (SYS_CPNT_ONIE_SUPPORT==FALSE)
    image_header_t    *imghdr;
    UI32_T            project_id;
    UC_MGR_Sys_Info_T sys_info;

    if( length < sizeof(image_header_t) )
    {
        return FALSE;
    }
    if(UC_MGR_GetSysInfo(&sys_info) != TRUE)
        return FALSE;

    imghdr = (image_header_t *)data_p;

    project_id = (imghdr->ih_pid)&0xffff;/*pid (hight 8 bits is 0,the next 8 bits is the BID, and the low 16 bits is the PID)*/


    if(project_id == sys_info.project_id)
        return TRUE;

    return FALSE;
#else
    return TRUE;
#endif
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_CheckLoaderImageHeaderProductId
 * ------------------------------------------------------------------------
 * FUNCTION : This function check the loader image header product id
 * INPUT    : data_p     -- pointer to the loader image header
 * OUTPUT   : None
 * RETURN   : TRUE       -- product id check pass
 *            FALSE      -- product id check fail
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T  FS_CheckLoaderImageHeaderProductId(LDR_INFO_BLOCK_T *ldr_hdr_p)
{
    UI32_T            project_id;
    UC_MGR_Sys_Info_T sys_info;

    if(UC_MGR_GetSysInfo(&sys_info) != TRUE)
        return FALSE;

    project_id = *((UI16_T*)(ldr_hdr_p->identifier+2));

    if(project_id == sys_info.project_id)
        return TRUE;

    return FALSE;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_CheckImageHeaderCrc
 * ------------------------------------------------------------------------
 * FUNCTION : This function check the image header crc
 * INPUT    : data_p     -- pointer to the image header
 *            length     -- data length
 * OUTPUT   : None
 * RETURN   : TRUE       -- CRC check pass or no need to check (cramfs)
 *            FALSE      -- CRC check fail
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T  FS_CheckImageHeaderCrc(UI8_T *data_p, UI32_T length)
{
#if (SYS_CPNT_ONIE_SUPPORT==FALSE)
    UI32_T checksum = 0;
    image_header_t   *imghdr;

    if( length < sizeof(image_header_t) )
    {
        return FALSE;
    }
    imghdr = (image_header_t *)data_p;
    checksum = imghdr->ih_hcrc;
    imghdr->ih_hcrc = 0;
    /* clear for re-calculation */
    if(checksum != L_MATH_Crc32(0, (I8_T *)data_p, sizeof(image_header_t)))
    {
        imghdr->ih_hcrc = checksum;
        return FALSE;
    }
    imghdr->ih_hcrc = checksum;
#endif
    return TRUE;
}


/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_CheckImageDataCrc
 * ------------------------------------------------------------------------
 * FUNCTION : This function check the image data crc
 * INPUT    : data_p     -- pointer to the image header
 *            length     -- data length
 * OUTPUT   : None
 * RETURN   : TRUE       -- CRC check pass
 *            FALSE      -- CRC check fail
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T  FS_CheckImageDataCrc(UI8_T *data_p, UI32_T length)
{
#if (SYS_CPNT_ONIE_SUPPORT==FALSE)
    image_header_t   *imghdr;
    UI8_T   *data;
    UI32_T  len;

    if( length < sizeof(image_header_t) )
    {
        return FALSE;
    }
    imghdr = (image_header_t *)data_p;

    data = data_p + sizeof(image_header_t);
    len  = imghdr->ih_size;
    if (L_MATH_Crc32 (0, (I8_T *)data, len) != imghdr->ih_dcrc) {
        ERRORSTR ("%s:ERROR,  Bad Data CRC, %lx \r\n",__FUNCTION__, (unsigned long)imghdr->ih_dcrc);
        return FALSE;
    }
#endif
    return TRUE;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetStorageFreeSpace
 * ------------------------------------------------------------------------
 * FUNCTION : This function returns the available free space of the file system.
 * INPUT    : drive                     -- unit id or blade number
 * OUTPUT   : total_size_of_free_space  -- total size of the free space
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : 1. drive represents either the unit id in a stack or the blade number
 *               in a chasis.
 *            2. This function should work in transition mode.
 *               Caller of this function should set driver to DUMMY_DRIVE
 *               to access this function in transition mode.
 *            3.
 * ------------------------------------------------------------------------
 */

UI32_T  FS_GetStorageFreeSpace(UI32_T drive, UI32_T *total_size_of_free_space)
{
    struct statfs s;
    UI32_T  return_value;

#if (SYS_CPNT_STACKING == TRUE)

    FS_File_Summary_Packet_T  fs_summary_packet;

    if (    (drive == DUMMY_DRIVE)
         || (drive == SYS_VAL_LOCAL_UNIT_ID)
         || (drive == FS_OM_GetControlDriverId())
       )
#endif
    {

        if (statfs(FS_SAVE_FOLDER, &s) != 0)
        {
            ERRORSTR("\r\n%s: statfs error\r\n", __FUNCTION__);
            *total_size_of_free_space = 0;
            return_value = FS_RETURN_ERROR;
        }
        else
        {
            *total_size_of_free_space = (s.f_bavail * s.f_bsize);
            return_value = FS_RETURN_OK;
        }
        return return_value;
    }
#if (SYS_CPNT_STACKING == TRUE)
    else /* Remote access */
    {
        DEBUGSTR("\r\n%s() Drive: %ld\r\n", __FUNCTION__, (long)drive);
        FS_SETBUSY();
        fs_summary_packet.total_space_free=0;
        return_value = FS_GetRemoteFileSystemSummary(drive, &fs_summary_packet);

        if(return_value == FS_RETURN_OK)
        {
            *total_size_of_free_space = fs_summary_packet.total_space_free;
            FS_SETNOTBUSY();
        }
        else
        {
            FS_SETNOTBUSY();
            ERRMSG(return_value);
        }
    }
    return return_value;
#endif
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetStorageFreeSpaceForUpdateRuntime
 * ------------------------------------------------------------------------
 * FUNCTION : This function outputs the available free space when it is used
 *            to consider the free space for runtime file update.
 * INPUT    : drive                     -- unit id or blade number
 * OUTPUT   : free_space_p              -- total size of the free space
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : 1. drive represents either the unit id in a stack or the blade
 *                number in a chasis.
 *            2. This function should work in transition mode.
 *               Caller of this function should set driver to DUMMY_DRIVE
 *               to access this function in transition mode.
 *            3. The function FS_GetStorageFreeSpace() is used to evaulate
 *               the free space when updating file type that is not
 *               FS_TYPE_OPCODE_FILE_TYPE.
 *               This function must be called for evaluation of file type
 *               FS_TYPE_OPCODE_FILE_TYPE.
 *            4. When the caller need to evaluate the free space after a runtime
 *               file is removed, the caller must call FS_GetFileStorageUsageSize()
 *               to get the storage usage size of the runtime file and take this
 *               as the increased free size after the runtime file is removed.
 * ------------------------------------------------------------------------
 */
UI32_T  FS_GetStorageFreeSpaceForUpdateRuntime(UI32_T drive, UI32_T *free_space_p)
{
    if (free_space_p == NULL)
    {
        ERRORSTR("%s:ERROR, bad parameter\r\n", __FUNCTION__);
        return FS_RETURN_BAD_PARA;
    }

#if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE)
    /* Local operation */
    #if (SYS_CPNT_STACKING == TRUE)
    if (    (drive == DUMMY_DRIVE)
         || (drive == SYS_VAL_LOCAL_UNIT_ID)
         || (drive == FS_OM_GetControlDriverId())
       )
    #endif /* #if (SYS_CPNT_STACKING == TRUE) */
    {
        UI32_T runtime_part_len, free_part_num;

        /* check file mapping table
         * if (no free partition for a runtime image)
         *      output free size as 0.
         * else
         *      output free size as runtime partition length.
         */
        if (FS_GetFreeRuntimePartitionInfo(&runtime_part_len, &free_part_num)==TRUE)
        {
            *free_space_p = runtime_part_len;
        }
        else
        {
            ERRORSTR("%s:FS_GetFreeRuntimePartitionInfo returns error\r\n", __FUNCTION__);
            return FS_RETURN_ERROR;
        }

        return FS_RETURN_OK;
    } /* End of local access */
    #if (SYS_CPNT_STACKING == TRUE)
    else /* Remote operation */
    {
        UI32_T return_value;

        FS_SETBUSY();
        return_value = FS_GetRemoteStorageFreeSpaceForUpdateRuntime(drive, free_space_p);
        FS_SETNOTBUSY();

        if(return_value != FS_RETURN_OK)
        {
            ERRMSG(return_value);
        }

        return return_value;
    }
    #endif /* #if (SYS_CPNT_STACKING == TRUE) */
#else /* #if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE) */
    /* When runtime file is written to a filesystem, it can call
     * FS_GetStorageFreeSpace() directly to get the free size available when
     * the caller needs to update a runtime file.
     */
    return FS_GetStorageFreeSpace(drive, free_space_p);
#endif /* #if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE) */

}




/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetFileStorageUsageSize
 * ------------------------------------------------------------------------
 * FUNCTION : This function returns the usage size of the specified file.
 * INPUT    : drive                     -- unit id or blade number
 *            file_name                 -- specified file name
 * OUTPUT   : usage_size                -- usage size of the file
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T  FS_GetFileStorageUsageSize(UI32_T drive, UI8_T *file_name, UI32_T *usage_size)
{

    if ((file_name[0] == 0) || (strlen((char*)file_name)>SYS_ADPT_FILE_SYSTEM_NAME_LEN))
    {
        ERRORSTR("%s:ERROR, bad parameter\r\n", __FUNCTION__);
        return FS_RETURN_BAD_PARA;
    }

    /* Local operation */
#if (SYS_CPNT_STACKING == TRUE)
    if (    (drive == DUMMY_DRIVE)
         || (drive == SYS_VAL_LOCAL_UNIT_ID)
         || (drive == FS_OM_GetControlDriverId())
       )
#endif
    {
        FS_FileHeader_T file_header;

        strncpy((char*)file_header.file_name,(char*)file_name,SYS_ADPT_FILE_SYSTEM_NAME_LEN);
        file_header.file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN]='\0';

        if (FALSE == FS_OM_GetFileHeaderByName(&file_header,FS_FILE_TYPE_MASK_ALL))
        {
            ERRORSTR("\r\n%s: can not find the file %s\r\n", __FUNCTION__, file_name);
            *usage_size = 0;
            return FS_RETURN_FILE_NEX;
        }
        DEBUGSTR("File Name:%s (on unit %d)\r\n", file_name, (int)drive);

        *usage_size = file_header.file_size;
        return FS_RETURN_OK;
    } /* End of local access */
#if (SYS_CPNT_STACKING == TRUE)
    else /* Remote operation */
    {
        UI32_T return_value;

        FS_SETBUSY();
        strcpy((char*)fs_file_attr_packet.file_attr[0].file_name, (char*)file_name);
        fs_file_attr_packet.file_attr[0].file_type_mask = FS_FILE_TYPE_MASK_ALL;

        return_value = FS_GetRemoteMultipleFileInfo(drive, FALSE, &fs_file_attr_packet, 1);

        if(return_value != FS_RETURN_OK && return_value != FS_RETURN_HAVEMOREDATA)
        {
            ERRMSG(return_value);
            FS_SETNOTBUSY();
            return return_value;
        }

        *usage_size = fs_file_attr_packet.file_attr[0].storage_size;
        FS_SETNOTBUSY();

        return return_value;
    }
#endif
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetFileInfo
 * ------------------------------------------------------------------------
 * FUNCTION : This function will get the information of the file
 * INPUT    : drive                     -- unit id or blade number
 *            file_attr->file_name      -- the name of the file
 *            file_attr->file_type_mask -- the type mask of the file
 * OUTPUT   : file_attr                 -- the attribute of the file
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : 1.The "drive" variable represents either the unit id in a
 *              stack or the blade number in a chasis.
 *            2.The file_type_mask field of the file_attr structure specifies
 *              the type of file we want to get. It must be set prerior to
 *              calling this fuction.
 * ------------------------------------------------------------------------
 */
UI32_T FS_GetFileInfo(UI32_T drive, FS_File_Attr_T *file_attr)
{
    UI8_T startup_file[SYS_ADPT_FILE_SYSTEM_NAME_LEN + 1] = {0};
    /* File system initialized OK?
     */
    if (FS_OM_GetInitStatus() != FS_RETURN_OK)
    {
        ERRORSTR("\r\n%s:ERROR, no initial \r\n", __FUNCTION__);
        return FS_RETURN_NOT_READY;
    }

    if ( (file_attr == 0) || (file_attr->file_name == 0) )
    {
        ERRORSTR("\r\n%s:ERROR, bad parameter \r\n", __FUNCTION__);
        return FS_RETURN_BAD_PARA;
    }

    /* Local operation
     */
    if (    (drive == DUMMY_DRIVE)
         || (drive == SYS_VAL_LOCAL_UNIT_ID)
#if (SYS_CPNT_STACKING == TRUE)
         || (drive == FS_OM_GetControlDriverId())
#endif
       )
    {
        FS_FileHeader_T file_header;

        strncpy((char*)file_header.file_name,(char*)file_attr->file_name,SYS_ADPT_FILE_SYSTEM_NAME_LEN);
        file_header.file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN]='\0';

        if (FALSE == FS_OM_GetFileHeaderByName(&file_header,file_attr->file_type_mask))
        {
            ERRORSTR("\r\n%s: can not find the file %s\r\n", __FUNCTION__, file_attr->file_name);
            return FS_RETURN_FILE_NEX;
        }

        strncpy((char*)file_attr->file_name, (char*)file_header.file_name, SYS_ADPT_FILE_SYSTEM_NAME_LEN);
        file_attr->file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN]='\0';
        strncpy((char*)file_attr->file_comment, (char*)file_header.file_comment, FS_FILE_COMMENT_LENGTH);
        file_attr->file_comment[FS_FILE_COMMENT_LENGTH]='\0';
        file_attr->create_time  = file_header.create_time;
        file_attr->file_type    = file_header.file_type;
        file_attr->file_size    = file_header.file_size;
        if (file_attr->file_type==FS_TYPE_OPCODE_FILE_TYPE)
        {
#if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE)
            /* one runtime is stored in on partition. So file_attr->storage_size can used for partition size.
             */
            file_attr->storage_size = fs_partition_table[file_header.mtd_num].size;
#else
            file_attr->storage_size = file_header.file_size;
#endif
        }
        else
        {
            file_attr->storage_size    = file_header.file_size;
        }

        if(FS_OM_GetStartupName(file_header.file_type,  startup_file))
        {
            if(strcmp((const char*)file_header.file_name, (const char*)startup_file) == 0)
                file_attr->startup_file = TRUE;
            else
                file_attr->startup_file = FALSE;
        }
        else
            file_attr->startup_file = FALSE;
        file_attr->startup_file = file_header.startup;

        return FS_RETURN_OK;
    }
    else /* Remote operation */
    {
#if (SYS_CPNT_STACKING == TRUE)
        UI32_T  return_value;

        FS_SETBUSY();
        memcpy(&fs_file_attr_packet.file_attr[0], file_attr, sizeof(FS_File_Attr_T));
        return_value = FS_GetRemoteMultipleFileInfo(drive, FALSE, &fs_file_attr_packet, 1);

        if (return_value == FS_RETURN_FILE_NEX)
        {
            ERRORSTR("\r\n%s:ERROR, remote file not find \r\n", __FUNCTION__);
        }

        if((return_value != FS_RETURN_OK) && (return_value != FS_RETURN_HAVEMOREDATA))
        {
            ERRORSTR("\r\n%s:ERROR, remote \r\n", __FUNCTION__);
            FS_SETNOTBUSY();
            return return_value;
        }

        memcpy(file_attr, &fs_file_attr_packet.file_attr[0], sizeof(FS_File_Attr_T));
        FS_SETNOTBUSY();

        return FS_RETURN_OK;
#else
        return FS_RETURN_BAD_PARA;
#endif
    }
} /* End of FS_GetFileInfo */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetWebFileSize
 * ------------------------------------------------------------------------
 * FUNCTION : This function will get size of the file
 * INPUT    : file_path     -- the path (if exist) and name of file to read; the
 *                             path is relative path to /usr/webroot/
 * OUTPUT   : file_size_p   -- the size of the file to read
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : 1.the directory of web files: /usr/webroot/
 * ------------------------------------------------------------------------
 */
FS_RETURN_CODE_T FS_GetWebFileSize(const char *file_path, UI32_T *file_size_p)
{
    struct stat file_info;

    if ((file_path == 0) || (file_size_p == 0))
    {
        ERRORSTR("\r\n%s:ERROR, bad parameter \r\n", __FUNCTION__);
        return FS_RETURN_BAD_PARA;
    }

    if( (stat(file_path, &file_info)!=0) || (!S_ISREG(file_info.st_mode)) )
    {
        if( S_ISDIR(file_info.st_mode) )
        {
            DEBUGSTR("%s: The given filename is a directory(%s)\r\n", __FUNCTION__, file_path);
        }
        else
        {
            DEBUGSTR("%s: Error occurs while getting status of the given filename '%s' (st_mode=0x%x)\r\n", __FUNCTION__, file_path /*name_p*/, file_info.st_mode);
        }

        return FS_RETURN_ERROR;
    }

    *file_size_p = file_info.st_size;

    return FS_RETURN_OK;

} /* End of FS_GetWebFileSize */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetNextFileInfo
 * ------------------------------------------------------------------------
 * FUNCTION : This function will get the information of the next file
 * INPUT    : drive                     -- unit id or blade number
 *            file_attr->file_name      -- the key to get
 *            file_attr->file_type_mask -- the type mask of the file
 * OUTPUT   : file_attr->file_name      -- the name of the next file
 *            file_attr                 -- the attribute of the file
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : 1. The first file in the file list will be returned if the input
 *            file_name is a NULL pointer (file_name[0] == 0).
 *            2. Drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 *            3. The function will only get file information across units in
 *               master mode
 *            4. if (*drive == DUMMY_DRIVE) thus only the local unit is taken
 *               into consideration.
 * ------------------------------------------------------------------------
 */
UI32_T FS_GetNextFileInfo(UI32_T *drive, FS_File_Attr_T *file_attr)
{
#if (SYS_CPNT_STACKING == TRUE)
    UI8_T startup_file[SYS_ADPT_FILE_SYSTEM_NAME_LEN + 1] = {0};
    UI32_T return_value;
/*    UI32_T number_of_units;*/
    UI32_T temp_drive;

    /* File system initialized OK? */
    if (FS_OM_GetInitStatus() != FS_RETURN_OK)
    {
        ERRMSG(FS_RETURN_NOT_READY);
        return FS_RETURN_NOT_READY;
    }

    if ( (file_attr == 0) || (file_attr->file_name == 0) )
    {
        ERRMSG(FS_RETURN_BAD_PARA);
        ERRORSTR("\r\n%s:ERROR, bad parameter \r\n", __FUNCTION__);
        return FS_RETURN_BAD_PARA;
    }

    /* Arden, May, 22, 2003, enable getting file info accross units */
    if (FS_OM_GetControlStackingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        if ( (*drive) == 0)
        {
            temp_drive  = 0;
            if (STKTPLG_POM_GetNextUnit(&temp_drive) == TRUE)
            {
                /* Unit number has changed,
                 * we should get the first file from the next unit
                 */
                file_attr->file_name[0] = '\0';
            }
            else
            {
                return FS_RETURN_FILE_NEX;
            }
        }
        else
        {
            temp_drive  = *drive;
        }
    }
    else
    {
        temp_drive = DUMMY_DRIVE;
    }

    return_value = FS_RETURN_FILE_NEX;
    for(;;)
    {
        /* Local operation */
        if (    (temp_drive == DUMMY_DRIVE)
             || (temp_drive == SYS_VAL_LOCAL_UNIT_ID)
             || (temp_drive == FS_OM_GetControlDriverId())
           )
        {
            FS_FileHeader_T     file_header;

            strncpy((char*)file_header.file_name,(char*)file_attr->file_name,SYS_ADPT_FILE_SYSTEM_NAME_LEN);
            file_header.file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN]='\0';

            if (FALSE == FS_OM_GetNextFileHeaderByName(&file_header,file_attr->file_type_mask))
            {
                /* Non_master: End of file list, return false
                 */
                if (FS_OM_GetControlStackingMode() != SYS_TYPE_STACKING_MASTER_MODE)
                {
                    return FS_RETURN_FILE_NEX;
                }
                else
                {
                    if (    (temp_drive != DUMMY_DRIVE)
                        &&  (STKTPLG_POM_GetNextUnit(&temp_drive) == TRUE)
                       )
                    {
                        /* Unit number has changed,
                         * we should get the first file from the next unit
                         */
                        file_attr->file_name[0] = '\0';
                        continue;
                    }
                    else
                    {
                        /* If (*drive == DUMMY_DRIVE) thus only the local
                         * unit is taken into consideration.
                         */
                        break;
                    }
                }
            }

            /* File found, copy file information to user buffer
             */
            strncpy((char*)file_attr->file_name, (char*)file_header.file_name, SYS_ADPT_FILE_SYSTEM_NAME_LEN);
            file_attr->file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN]='\0';
            strncpy((char*)file_attr->file_comment, (char*)file_header.file_comment, FS_FILE_COMMENT_LENGTH);
            file_header.file_comment[FS_FILE_COMMENT_LENGTH]='\0';
            file_attr->file_type    = file_header.file_type;
            file_attr->file_size    = file_header.file_size;
            file_attr->create_time  = file_header.create_time;

            if(FS_OM_GetStartupName(file_header.file_type,  startup_file))
            {
                if(strcmp((const char*)file_header.file_name, (const char*)startup_file) == 0)
                    file_attr->startup_file = TRUE;
                else
                    file_attr->startup_file = FALSE;
            }
            else
            {
                DBGMSG_EX("No starup file exists file_type=%d.\r\n", file_header.file_type);
                file_attr->startup_file = FALSE;
            }

            *drive = temp_drive;
            return_value = FS_RETURN_OK;
            break;
        }
        else /* Remote operation */
        {
            if (FS_OM_GetControlStackingMode() != SYS_TYPE_STACKING_MASTER_MODE)
            {
                return_value = FS_RETURN_FILE_NEX;
                break;
            }

            FS_SETBUSY();
            memcpy(&fs_file_attr_packet.file_attr[0], file_attr, sizeof(FS_File_Attr_T));

            return_value = FS_GetRemoteMultipleFileInfo(temp_drive, TRUE, &fs_file_attr_packet, 1);

            if ((return_value == FS_RETURN_OK) || (return_value == FS_RETURN_HAVEMOREDATA))
            {
                *drive = temp_drive;
                memcpy(file_attr, &fs_file_attr_packet.file_attr[0], sizeof(FS_File_Attr_T));
                return_value = FS_RETURN_OK;
                FS_SETNOTBUSY();
                break;
            }
            else if (return_value == FS_RETURN_FILE_NEX)
            {
                /* Master: Advance to the next unit*/
                if (STKTPLG_POM_GetNextUnit(&temp_drive) == TRUE)
                {
                    /* Unit number has changed,
                     * we should get the first file from the next unit
                     */
                    file_attr->file_name[0] = '\0';
                    FS_SETNOTBUSY();
                    continue;
                }
                else /* Not more unit to get */
                {
                    FS_SETNOTBUSY();
                    break;
                }
            }
            else
            {
                ERRMSG(return_value);
                FS_SETNOTBUSY();
                break;
            }
        }
    }
    *drive = temp_drive;
    return return_value;
#else /* !SYS_CPNT_STACKING */

    FS_FileHeader_T     file_header;

    /* File system initialized OK?
     */
    if (FS_OM_GetInitStatus() != FS_RETURN_OK)
        return FS_RETURN_NOT_READY;

    if (file_attr == 0)
    {
        ERRORSTR("\r\n%s:ERROR, bad parameter \r\n", __FUNCTION__);
        return FS_RETURN_BAD_PARA;
    }

    if (file_attr->file_name == 0)
    {
        ERRORSTR("\r\n%s:ERROR, bad parameter \r\n", __FUNCTION__);
        return FS_RETURN_BAD_PARA;
    }

    strncpy((char*)file_header.file_name,(char*)file_attr->file_name,SYS_ADPT_FILE_SYSTEM_NAME_LEN);
    file_header.file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN]='\0';
    if (FALSE == FS_OM_GetNextFileHeaderByName(&file_header,file_attr->file_type_mask))
    {
        DEBUGSTR("\r\n%s:, File not found: %s \r\n", __FUNCTION__, file_attr->file_name);
        return FS_RETURN_FILE_NEX;
    }

    strncpy((char*)file_attr->file_name, (char*)file_header.file_name, SYS_ADPT_FILE_SYSTEM_NAME_LEN);
    file_attr->file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN]='\0';
    strncpy((char*)file_attr->file_comment, (char*)file_header.file_comment, FS_FILE_COMMENT_LENGTH);
    file_header.file_comment[FS_FILE_COMMENT_LENGTH]='\0';
    file_attr->file_type    = file_header.file_type;
    file_attr->create_time  = file_header.create_time;
    file_attr->file_size    = file_header.file_size;
    file_attr->startup_file = file_header.startup;

    return FS_RETURN_OK;
#endif /* SYS_CPNT_STACKING */
} /* End of FS_GetNextFileInfo() */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetDeviceFileName
 * ------------------------------------------------------------------------
 * FUNCTION : get device full name (add path)
 * INPUT    : file_name    -- the file name save in OM
 *            file_type    -- FS_FILE_TYPE_XXX
 * OUTPUT   : full_name    -- the real file name include path
 * RETURN   : TRUE/FALSE
 * NOTE     : 
 * ------------------------------------------------------------------------
 */
static BOOL_T FS_GetDeviceFileName(UI8_T *filename, FS_File_Type_T file_type, char *full_name)
{
    int rc,max_str_buf_sz=FS_MAX_PATHNAME_BUF_SIZE;
    const char* prefix_path_p=NULL;
    const char* filename_mark_p="";

    if (fs_uid_type_to_dflt_mode_bitmap[file_type]==FS_UID_DFLT_MODE_BITMAP_VALUE_BLD_MODE)
    {
        filename_mark_p=FS_FILENAME_MARK;
        max_str_buf_sz+=FS_FILENAME_MARK_SIZE;
    }

    switch(file_type) {
        case FS_FILE_TYPE_LICENSE:
            prefix_path_p=SYS_ADPT_LICENSE_FILE_PATH;
            break;
        default:
            prefix_path_p=FS_SAVE_FOLDER;
            break;
    }

    if (snprintf(full_name, max_str_buf_sz, "%s%s%s", prefix_path_p, filename, filename_mark_p) < 0)
    {
        return FALSE;
    }

    return TRUE;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_AddDeviceFileNameMark
 * ------------------------------------------------------------------------
 * FUNCTION : add file name mark for openflow config file
 * INPUT    : src_name    -- the file name save in OM
 *            file_type   -- FS_FILE_TYPE_XXX
 * OUTPUT   : dst_name    -- the real file name. (ex: XXX.of)
 * RETURN   : TRUE/FALSE
 * NOTE     : 
 * ------------------------------------------------------------------------
 */
static BOOL_T FS_AddDeviceFileNameMark(UI8_T *src_name, char *dst_name, FS_File_Type_T file_type)
{
    int rc,max_str_buf_sz=SYS_ADPT_FILE_SYSTEM_NAME_LEN+1;   /*include NULL character*/
    const char* filename_mark_p="";

    if (fs_uid_type_to_dflt_mode_bitmap[file_type]==FS_UID_DFLT_MODE_BITMAP_VALUE_BLD_MODE)
    {
        filename_mark_p=FS_FILENAME_MARK;
        max_str_buf_sz+=FS_FILENAME_MARK_SIZE;
    }
    rc=snprintf(dst_name, max_str_buf_sz, "%s%s", src_name, filename_mark_p);
    if (rc<0)
    {
        return FALSE;
    }

    return TRUE;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_RemoveDeviceFileNameMark
 * ------------------------------------------------------------------------
 * FUNCTION : remove file name mark for openflow config file
 * INPUT    : src_name    -- the real file name. (ex: XXX.of)
 *                file_type      -- FS_FILE_TYPE_XXX
 * OUTPUT   : dst_name    -- the file name save in OM
 * RETURN   : none
 * NOTE     : 
 * ------------------------------------------------------------------------
 */
static void FS_RemoveDeviceFileNameMark(char *src_name, UI8_T *dst_name, FS_File_Type_T file_type)
{
    if (fs_uid_type_to_dflt_mode_bitmap[file_type]==FS_UID_DFLT_MODE_BITMAP_VALUE_BLD_MODE)
    {
        size_t len;
        len=strlen(src_name)-FS_FILENAME_MARK_SIZE;
        strncpy((char*)dst_name, src_name, len);
        dst_name[len] = '\0';
    }
    else
    {
        strncpy((char*)dst_name, (const char*)src_name, SYS_ADPT_FILE_SYSTEM_NAME_LEN);
        dst_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN]='\0';
    }

    return;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_EraseFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function will erase a file
 * INPUT    : unit          -- the unit id
 *            file_name     -- the file to delete
 *            privilege     -- TRUE : allow to delete the protected file
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : none
 * ------------------------------------------------------------------------
 */
static  UI32_T  FS_EraseFile(UI32_T unit, UI8_T *file_name, BOOL_T privilege)
{
    FS_FileHeader_T     file_header;

    char                name[FS_MAX_PATHNAME_BUF_SIZE]={0};

    /* File system initialized OK? */
    if (FS_OM_GetInitStatus() != FS_RETURN_OK)
        return FS_RETURN_NOT_READY;

    strncpy((char*)file_header.file_name,(char*)file_name,SYS_ADPT_FILE_SYSTEM_NAME_LEN);
    file_header.file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN]=0;

    if (FALSE == FS_OM_GetFileHeaderByName(&file_header,FS_FILE_TYPE_MASK_ALL))
    {
        DEBUGSTR("\r\n%s:, File not found: %s \r\n", __FUNCTION__, file_name);
        return FS_RETURN_FILE_NEX;
    }

    /* Check the magic number to avoid the illegal access due to the file header list corruption
     */
    if(file_header.magic_number != FS_SPECIAL_MAGIC_NUMBER)
    {
        ERRORSTR("\r\n%s::Warning!! The file header list may have been destroyed!!.  \
            The operation is prohibited for safety. Please restart the system to recover.\r\n", __FUNCTION__);
        return FS_RETURN_ACTION_INHIBITED;
    }

    if(FS_TYPE_OPCODE_FILE_TYPE == file_header.file_type)
    {

        /*del file info in file mapping table*/
#if (SYS_CPNT_FS_SUPPORT_WRITE_RUNTIME_TO_FILESYSTEM==TRUE)
        snprintf(name, FS_MAX_PATHNAME_BUF_SIZE,"%s%s", FS_SAVE_FOLDER, file_name);
        if (unlink(name)<0)
        {
            ERRORSTR("%s:%d,ERROR!! can not delete file (%s)!!.\r\n", __FUNCTION__, __LINE__, name);
            return FS_RETURN_ERROR;
        }
#endif/*End #if (SYS_CPNT_FS_SUPPORT_WRITE_RUNTIME_TO_FILESYSTEM==TRUE)*/
    }
    else
    {
        if (FALSE == privilege)
        {
            if (strcmp((char*)file_name, FS_FACTORY_DEFAULT_CONFIG) == 0)
            {
                ERRORSTR("%s::Warning!! FS_FACTORY_DEFAULT_CONFIG (%s) can not be deleted !!.\r\n", __FUNCTION__, FS_FACTORY_DEFAULT_CONFIG);
                return FS_RETURN_ACTION_INHIBITED;
            }
        }

        /* delete the file in file system
         */
        if (FS_GetDeviceFileName(file_header.file_name, file_header.file_type, name) != TRUE)
        {
            ERRORSTR("%s(%d)FS_GetDeviceFileName returns FALSE.\n", __FUNCTION__, __LINE__);
            return FS_RETURN_ERROR;
	 }

        if (unlink(name) < 0) {
            ERRORSTR("%s::ERROR!! can not delete file (%s)!!.\r\n", __FUNCTION__, name);
            return FS_RETURN_ERROR;
        }
    }
#if (SYS_CPNT_FS_SUPPORT_UBIFS==TRUE) /*Need to let data to write from cache to disk immediately. Because UBIFS is write-back mech.*/ \
  ||(SYS_CPNT_FS_SUPPORT_EXT3==TRUE)
    sync();
#endif
    FS_OM_RemoveFileHeaderByName(file_name);
    return FS_RETURN_OK;
} /* End of FS_EraseFile() */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_DeleteFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function will delete a file
 * INPUT    : drive         -- unit id or blade number
 *            file_name     -- the file to delete
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : 1. Drive represents either the unit id in a stack or the blade number
 *               in a chasis.
 *            2. If the file is on some other drive, call FS_WriteRemoteFile
 *               and set the file_size parameter to zero.
 * ------------------------------------------------------------------------
 */
UI32_T FS_DeleteFile(UI32_T drive, UI8_T *file_name)
{
    UI32_T  return_value;

    FS_SETBUSY();
#if (SYS_CPNT_STACKING == TRUE)
    /* Local operation */
    if (    (drive == DUMMY_DRIVE)
         || (drive == SYS_VAL_LOCAL_UNIT_ID)
         || (drive == FS_OM_GetControlDriverId())
       )
    {
        return_value = FS_EraseFile(drive, file_name, FALSE);
    }
    else /* Remote operation */
    {
        FS_File_Attr_T  file_attr;
        memset(&file_attr, 0, sizeof(file_attr));
        file_attr.privilage = FALSE;
        memcpy(file_attr.file_name, file_name, strlen((char*)file_name) );
        return_value = FS_WriteRemoteFile(drive, &file_attr);
    }
#else
    return_value = FS_EraseFile(drive, (UI8_T*)file_name, FALSE);
#endif
    FS_SETNOTBUSY();
    return return_value;
} /* End of FS_DeleteFile() */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_DeleteProtectedFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function will delete a protected file
 * INPUT    : drive         -- unit id or blade number
 *            file_name     -- the file to delete
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T FS_DeleteProtectedFile(UI32_T drive, UI8_T *file_name)
{
    UI32_T    return_value;

#if (SYS_CPNT_STACKING == TRUE)
    /* Local operation */
    if (    (drive == DUMMY_DRIVE)
         || (drive == SYS_VAL_LOCAL_UNIT_ID)
         || (drive == FS_OM_GetControlDriverId())
       )
    {
        FS_SETBUSY();
        return_value = FS_EraseFile(drive, (UI8_T*)file_name, TRUE);
        FS_SETNOTBUSY();
    }
    else /* Remote operation */
    {
        FS_File_Attr_T  file_attr;
        memset(&file_attr, 0, sizeof(file_attr));
        file_attr.privilage = TRUE;
        memcpy(file_attr.file_name, file_name, strlen((char*)file_name) );
        FS_SETBUSY();
        return_value = FS_WriteRemoteFile(drive, &file_attr);
        FS_SETNOTBUSY();
    }
#else
    FS_SETBUSY();
    return_value = FS_EraseFile(drive, (UI8_T*)file_name, TRUE);
    FS_SETNOTBUSY();
#endif
    return return_value;
} /* End of FS_DeleteProtectedFile() */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_ReadFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function will read data from the specified file to the buffer
 * INPUT    : drive         -- unit id or blade number
 *            file_name     -- the name of file to read
 *            buf           -- the destination buffer
 *            buf_size      -- the length of the buffer
 * OUTPUT   : read_count    -- the count of bytes read
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : 1. If the buf_size is less than the file size, the file will be
 *            truncated and FS_RETURN_FILE_TRUNCATED is returned.
 *            2. drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T FS_ReadFile(UI32_T drive, UI8_T  *file_name, UI8_T  *buf, UI32_T buf_size, UI32_T *read_count)
{
    UI32_T              remainder;
    UI32_T              result;

    char                name[FS_MAX_PATHNAME_BUF_SIZE]={0};

    FILE*               fd;

    /* File system initialized OK?
     */
    if (FS_RETURN_OK != FS_OM_GetInitStatus())
        return FS_RETURN_NOT_READY;

    if ((file_name == 0) || (buf == 0) || (read_count == 0) || (strlen((char*)file_name)>SYS_ADPT_FILE_SYSTEM_NAME_LEN))
    {
        ERRORSTR("\r\n%s:ERROR, bad parameter \r\n", __FUNCTION__);
        return FS_RETURN_BAD_PARA;
    }


    /* Local operation
     */
    if (    (drive == DUMMY_DRIVE)
         || (drive == SYS_VAL_LOCAL_UNIT_ID)
#if (SYS_CPNT_STACKING == TRUE)
         || (drive == FS_OM_GetControlDriverId())
#endif
       )
    {
        FS_FileHeader_T file_header;

        strncpy((char*)file_header.file_name,(char*)file_name,SYS_ADPT_FILE_SYSTEM_NAME_LEN);
        file_header.file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN]='\0';

        if (FALSE == FS_OM_GetFileHeaderByName(&file_header,FS_FILE_TYPE_MASK_ALL))
        {
            DEBUGSTR("\r\n%s:warning, file not found (%s) \r\n", __FUNCTION__, file_name);
            return FS_RETURN_FILE_NEX;
        }

        FS_SETBUSY();
        *read_count = 0;

        if(file_header.file_type == FS_TYPE_OPCODE_FILE_TYPE )
        {
            remainder = file_header.file_size;
            if (remainder > buf_size)
                remainder = buf_size;
#if (SYS_CPNT_FS_SUPPORT_WRITE_RUNTIME_TO_FILESYSTEM==TRUE)
            snprintf(name, FS_MAX_PATHNAME_BUF_SIZE, "%s%s", FS_SAVE_FOLDER, file_header.file_name);
            fd=fopen(name, "r");
            if(fd==NULL)
            {
                ERRORSTR("FS_ReadFile: can not open file %s\r\n",name);
                FS_SETNOTBUSY();
                return FS_RETURN_FILE_NEX;
            }
            remainder=file_header.file_size;
            if (remainder > buf_size)
                remainder = buf_size;
            *read_count=fread(buf, 1, remainder, fd);
            fclose(fd);
#else
            result = FS_READ_RUNTIME_FROM_FLASH(buf, remainder, read_count, file_header.mtd_num);
            if( result != FS_RETURN_OK )
                DEBUGSTR("%s, FS_READ_RUNTIME_FROM_FLASH error\r\n", __FUNCTION__);
#endif /*End #if (SYS_CPNT_FS_SUPPORT_WRITE_RUNTIME_TO_FILESYSTEM==TRUE)*/
        }
        else
        {
            if (FS_GetDeviceFileName(file_header.file_name, file_header.file_type, name) != TRUE)
            {
                ERRORSTR("%s(%d)FS_GetDeviceFileName returns FALSE.\n", __FUNCTION__, __LINE__);
                FS_SETNOTBUSY();
                return FS_RETURN_ERROR;
            }
            fd = fopen(name, "r");
            if( fd == NULL )
            {
                ERRORSTR("FS_ReadFile: can not open file %s\r\n",name);
                FS_SETNOTBUSY();
                return FS_RETURN_FILE_NEX;
            }
            remainder   = file_header.file_size;
            if (remainder > buf_size)
                remainder = buf_size;
            *read_count = fread(buf, 1, remainder, fd);
            fclose(fd);
        }

        if ( (*read_count) < (file_header.file_size) )
        {
            /* file truncated */
            result = FS_RETURN_FILE_TRUNCATED;
        }
        else
            result = FS_RETURN_OK;

        FS_SETNOTBUSY();
    }
    else /* Remote operation */
    {
#if (SYS_CPNT_STACKING == TRUE)
        FS_SETBUSY();
        strcpy((char*)fs_file_attr_packet.file_attr[0].file_name, (char*)file_name);
        fs_file_attr_packet.file_attr[0].file_type_mask = FS_FILE_TYPE_MASK_ALL;

        result = FS_GetRemoteMultipleFileInfo(drive, FALSE, &fs_file_attr_packet, 1);

        if(result != FS_RETURN_OK && result != FS_RETURN_HAVEMOREDATA)
        {
            ERRMSG(result);
            FS_SETNOTBUSY();
            return result;
        }

//        memcpy(&(fs_control.file_attr), &(fs_file_attr_packet.file_attr[0]), sizeof(FS_File_Attr_T));
        FS_OM_SetControlAttribution(&(fs_file_attr_packet.file_attr[0]));
        FS_SETNOTBUSY();

        if(FS_OM_GetControlAttrFileSize() > buf_size)
        {
            *read_count = 0;
            result = FS_RETURN_FILE_TRUNCATED;
        }

        FS_OM_SetControlFileBuffer(buf);
        FS_OM_SetControlFilePtr(buf);

        if(!FS_OM_GetControlFileBuffer())
        {
            ERRMSG(FS_RETURN_NO_BUFFER);
            return FS_RETURN_NO_BUFFER;
        }
        else
        {
            FS_SETBUSY();
//            result = FS_ReadRemoteFile(drive, &fs_control.file_attr, 0, FS_OM_GetControlAttrFileSize());
            result = FS_ReadRemoteFile(drive, &(fs_file_attr_packet.file_attr[0]), 0, FS_OM_GetControlAttrFileSize());
            FS_SETNOTBUSY();
        }

        if(result != FS_RETURN_OK)
        {
            ERRMSG(result);
        }
        else
        {
            *read_count = FS_OM_GetControlAttrFileSize();
            result = FS_RETURN_OK;
        }
        FS_OM_SetControlFileBuffer(NULL);
        FS_OM_SetControlFilePtr(NULL);
        FS_ReturnIdle();
#else
        result = FS_RETURN_BAD_PARA;
#endif
    }
    return result;
} /* End of FS_ReadFile() */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_ReadWebFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function will read data from the specified file to the buffer
 * INPUT    : file_path     -- the path (if exist) and name of file to read; the
 *                             path is relative path to /usr/webroot/
 *            buf           -- the destination buffer
 *            buf_size      -- the length of the buffer
 * OUTPUT   : read_count_p  -- the count of bytes read
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : 1.the directory of web files: /usr/webroot/
 *            2.This API will not check the total length of the web file to be read. The maximum
 *              length that will be read is buf_size
 * ------------------------------------------------------------------------
 */
FS_RETURN_CODE_T FS_ReadWebFile(const char *file_path, UI8_T *buf, UI32_T buf_size, UI32_T *read_count_p)
{
    FILE    *fd;

    if ((file_path == 0) || (buf == 0) || (read_count_p == 0))
    {
        ERRORSTR("\r\n%s:ERROR, bad parameter \r\n", __FUNCTION__);
        return FS_RETURN_BAD_PARA;
    }

    FS_SETBUSY();
    *read_count_p = 0;

    fd = fopen(file_path, "r");
    if( fd == NULL )
    {
        ERRORSTR("FS_ReadWebFile: can not open file %s\r\n",file_path);
        FS_SETNOTBUSY();

        return FS_RETURN_FILE_NEX;
    }

    *read_count_p = fread(buf, 1, buf_size, fd);
    fclose(fd);
    FS_SETNOTBUSY();

    return FS_RETURN_OK;
} /* End of FS_ReadWebFile() */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_CopyFileContent
 * ------------------------------------------------------------------------
 * FUNCTION : This function will copy data from the specified offset of the
 *            file to the buffer
 * INPUT    : drive         -- unit id or blade number
 *            file_name     -- the name of file to be copied
 *            offset        -- the offset of the file to be copied
 *            buf           -- the destination buffer
 *            byte_num      -- the length of data to be copied
 * OUTPUT   : read_count    -- the count of bytes read
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T  FS_CopyFileContent(UI32_T drive, UI8_T *file_name, UI32_T offset, UI8_T *buf, UI32_T byte_num)
{
    FS_FileHeader_T         file_header;
    UI32_T                  copy_size;
#if (SYS_CPNT_STACKING == TRUE)
    UI32_T                  return_value;
    UI32_T                  capability;
#endif

#if (SYS_CPNT_STACKING == TRUE)

    if (    (drive != DUMMY_DRIVE)
         && (drive != SYS_VAL_LOCAL_UNIT_ID)
         && (drive != FS_OM_GetControlDriverId())
       )
    {
        if(FS_OM_GetControlStackingMode() != SYS_TYPE_STACKING_MASTER_MODE)
        {
            ERRMSG(FS_RETURN_NOT_MASTER);
            return FS_RETURN_NOT_MASTER;
        }

        FS_SETBUSY();
        strcpy((char*)fs_file_attr_packet.file_attr[0].file_name, (char*)file_name);
        fs_file_attr_packet.file_attr[0].file_type_mask = FS_FILE_TYPE_MASK_ALL;

        return_value = FS_GetRemoteMultipleFileInfo(drive, FALSE, &fs_file_attr_packet, 1);

        if(return_value != FS_RETURN_OK && return_value != FS_RETURN_HAVEMOREDATA)
        {
            ERRMSG(return_value);
            FS_SETNOTBUSY();
            return return_value;
        }

//        memcpy(&(fs_control.file_attr), &(fs_file_attr_packet.file_attr[0]), sizeof(FS_File_Attr_T));
        FS_OM_SetControlAttribution(&(fs_file_attr_packet.file_attr[0]));

        FS_SETNOTBUSY();


        DEBUGSTR("%s() File \"%s\" found, Type: %ld, Size: %ld\r\n",
                __FUNCTION__,
                file_name,
                (long)FS_OM_GetControlAttrFileType(),
                (long)FS_OM_GetControlAttrFileSize());


        /* Allocate memory space to hold file content */
        FS_OM_SetControlFileBuffer( L_MM_Malloc(byte_num, L_MM_USER_ID2(SYS_MODULE_FS, FS_TYPE_TRACE_ID_FS_COPYFILECONTENT)));
        DBGMSG("Allocate Buffer");
        FS_OM_SetControlFilePtr(FS_OM_GetControlFileBuffer());

        if(!FS_OM_GetControlFileBuffer())
        {
            ERRMSG(FS_RETURN_NO_BUFFER);
            return FS_RETURN_NO_BUFFER;
        }
        else
        {
            FS_GetRemoteServiceCapability(drive, &capability);
            if (capability > FS_COPY_FILE_CONTENT)
            {
                FS_SETBUSY();
//                return_value = FS_CopyRemoteFileContent(drive, &fs_control.file_attr, offset, byte_num);
                return_value = FS_CopyRemoteFileContent(drive, &(fs_file_attr_packet.file_attr[0]), offset, byte_num);
                FS_SETNOTBUSY();
            }
            else
            {
                FS_SETBUSY();
//                return_value = FS_ReadRemoteFile(drive, &fs_control.file_attr, offset, byte_num);
                return_value = FS_ReadRemoteFile(drive, &(fs_file_attr_packet.file_attr[0]), offset, byte_num);
                FS_SETNOTBUSY();
            }
        }

        if(return_value != FS_RETURN_OK)
        {
            ERRMSG(return_value);
        }
        else
        {
            memcpy(buf, FS_OM_GetControlFileBuffer(), byte_num);
            return_value = FS_RETURN_OK;
        }
        L_MM_Free(FS_OM_GetControlFileBuffer());
        DBGMSG("Free Buffer");
        FS_OM_SetControlFileBuffer(0);
        FS_OM_SetControlFilePtr(0);
        FS_ReturnIdle();
        return return_value;
    }
#endif

    strncpy((char*)file_header.file_name,(char*)file_name,SYS_ADPT_FILE_SYSTEM_NAME_LEN);
    file_header.file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN]='\0';

    FS_SETBUSY();

    if (FALSE == FS_OM_GetFileHeaderByName(&file_header,FS_FILE_TYPE_MASK_ALL))
    {
        /* file not found
         */
        DEBUGSTR("%s:ERROR, file not found %s\r\n", __FUNCTION__, file_name);
        FS_SETNOTBUSY();
        return FS_RETURN_FILE_NEX;
    }
    else
    {
        FILE    *fd;

        char                name[FS_MAX_PATHNAME_BUF_SIZE]={0};

        UI32_T  read_len;
        /* File exists
         */
        if(file_header.file_type == FS_TYPE_OPCODE_FILE_TYPE)
        {
#if (SYS_CPNT_FS_SUPPORT_WRITE_RUNTIME_TO_FILESYSTEM==TRUE)
            snprintf((char *)name, FS_MAX_PATHNAME_BUF_SIZE, "%s%s",FS_SAVE_FOLDER,file_header.file_name);
            fd=fopen((char *)name, "r");
            if(fd==NULL)
            {
                ERRORSTR("%s:ERROR, can not open file %s\r\n",__FUNCTION__, name);
                FS_SETNOTBUSY();
                return FS_RETURN_FILE_NEX;
            }
            fseek(fd, offset, SEEK_SET);
            if(file_header.file_size<(byte_num+offset))
            {
                DEBUGSTR(" file size %ld, < offset(%ld) + copy#(%ld)\r\n",(long)file_header.file_size, (long)offset,(long)byte_num);
            }
            read_len=fread(buf, 1, byte_num, fd);
            if(read_len<byte_num)
            {
                DEBUGSTR(" Only copy %ld which request %ld\r\n",(long)read_len,(long)byte_num);
                fclose(fd);
                FS_SETNOTBUSY();
                return FS_RETURN_ERROR;
            }
            fclose(fd);
#else
            if(FS_READ_RUNTIME_FROM_FLASH(buf, byte_num, &read_len, file_header.mtd_num) != FS_RETURN_OK )
            {
                 FS_SETNOTBUSY();
                 return FS_RETURN_ERROR;
            }
#endif /*End #if (SYS_CPNT_FS_SUPPORT_WRITE_RUNTIME_TO_FILESYSTEM==TRUE)*/

        }
        else
        {
            if (FS_GetDeviceFileName(file_header.file_name, file_header.file_type, name) != TRUE)
            {
                ERRORSTR("%s(%d)FS_GetDeviceFileName returns FALSE.\n", __FUNCTION__, __LINE__);
                FS_SETNOTBUSY();
                return FS_RETURN_ERROR;
            }
            fd = fopen((char *)name, "r");
            if( fd == NULL )
            {
                ERRORSTR("%s:ERROR, can not open file %s\r\n",__FUNCTION__, name);
                FS_SETNOTBUSY();
                return FS_RETURN_FILE_NEX;
            }
            fseek(fd, offset, SEEK_SET);
            copy_size   = file_header.file_size;
            if (copy_size < (byte_num+offset))
            {
                DEBUGSTR(" file size %ld, < offset(%ld) + copy#(%ld)\r\n",(long)copy_size, (long)offset,byte_num);
            }
            copy_size = fread(buf, 1, byte_num, fd);
            if (copy_size < byte_num)
            {
                DEBUGSTR(" Only copy %ld which request %ld\r\n",(long)copy_size,(long)byte_num);
            }
            fclose(fd);
        }
        FS_SETNOTBUSY();
        return FS_RETURN_OK;
    }
} /* End of FS_CopyFileContent */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_UpdateFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function will write data from buffer to a file
 * INPUT    : unit          -- the unit id
 *            file_name     -- the name of the file to write
 *            file_comment  -- the comment of the file
 *            file_type     -- file type
 *            buf           -- the source buffer
 *            length        -- data length to be writen (file size)
 *            resv_len      -- reserved length to guarantee the minimum size of
 *                             a file. resv_len < length will not reserve extra
 *                             space for the file.
 *            privilege     -- TRUE : allow to overwrite the protected file
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : This file only updates local files
 * ------------------------------------------------------------------------
 */
static  UI32_T  FS_UpdateFile(UI32_T unit, UI8_T *file_name, UI8_T *file_comment, UI32_T file_type, UI8_T *buf, UI32_T length, UI32_T resv_len, BOOL_T privilege)
{
    FS_FileHeader_T     file_header;

    char                name[FS_MAX_PATHNAME_BUF_SIZE]={0};

    UI32_T              usage_size, ret = FS_RETURN_ERROR;
    FILE*               fd;
    BOOL_T              exist_file_is_startup = FALSE;
    UI32_T              number_of_files;
    UI32_T              time;

    /* File system initialized OK? */
    if (FS_OM_GetInitStatus() != FS_RETURN_OK)
        return FS_RETURN_NOT_READY;

    FS_DUMMY_WRITE_RETURN(FS_RETURN_OK);


    /*
     * 1.0 check file is exist, if exist, del it .
     */
    strncpy((char*)file_header.file_name,(char*)file_name,SYS_ADPT_FILE_SYSTEM_NAME_LEN);
    file_header.file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN]='\0';

    if (TRUE == FS_OM_GetFileHeaderByName(&file_header,FS_FILE_TYPE_MASK(file_type)))
    {
        /*check privilege for default config file*/
        if(strcmp((char*)file_name, FS_FACTORY_DEFAULT_CONFIG) == 0)
        {
            if (!privilege)
            {
                ERRORSTR("\r\nWarning!! you can not overwrite default config %s.\r\n", FS_FACTORY_DEFAULT_CONFIG);
                return FS_RETURN_ACTION_INHIBITED;
            }
        }

        if (FS_RETURN_OK != FS_EraseFile(unit, file_name, privilege))
        {
            return FS_RETURN_ERROR;
        }

        exist_file_is_startup = file_header.startup;
    }

    /*
     * Check the maximum number of files for this file type
     */
    number_of_files = FS_NumOfFile(file_type);
    if(number_of_files >= FS_MaxNumOfFile[file_type])
    {
        ERRORSTR("\r\nWarning!! The number of files of the given type exceeds the maximum number. Operation Inhibited.\r\n");
        return FS_RETURN_EXCEED_MAX_NUM_OF_FILES;
    }
    /* Evaluate the free space for the new file
     * runtime  which has independent partition, so don't to check free space
     * In jffs2 file system, file is compressed, so below verify is un-nessary now.
     */
    /* EPR ID:ES3528MV2-FLF-EC-00075
     * The file written to jffs2 partition will be compressed and
     * it's hard to estimate the compression ratio. So skip the
     * check for the free size and write the file directly.
     * If the free space is not enough, the write operation fails
     * finally.
     */
    #if 0
    if( file_type  !=  FS_TYPE_OPCODE_FILE_TYPE)
    {
        FS_GetStorageFreeSpace(unit,&total_size_of_free_space);
        if ( (total_size_of_free_space < length) || (total_size_of_free_space < resv_len) )
        {
            ERRORSTR("\r\nWarning!! Not enough space for new file. Operation Inhibited.\r\n");
            return FS_RETURN_ACTION_INHIBITED;
        }
    }
    #endif

    /*
     * 2.0 Write new file
     */
    if(file_type == FS_TYPE_OPCODE_FILE_TYPE)
    {
#if (SYS_CPNT_FS_SUPPORT_WRITE_RUNTIME_TO_FILESYSTEM==TRUE)
        FILE *fd;

        snprintf((char*)name, FS_MAX_PATHNAME_BUF_SIZE, "%s%s", FS_SAVE_FOLDER, file_name);
        fd=fopen(name, "w");
        if(fd==NULL)
        {
            ERRORSTR("%s:%d,ERROR, can't fopen file %s\r\n", __FUNCTION__, __LINE__, name);
            return FS_RETURN_FILE_NEX;
        }
        usage_size=fwrite((char*)buf,1, length,fd);
        if(usage_size<length)
        {
            ERRORSTR("%s:%d,:ERROR, write size %ld < file_size %ld\r\n",__FUNCTION__, __LINE__, (long)usage_size, (long)length);
            fclose(fd);
            /* when "length > usage_size", it means there is no enough space.
             * So it doesn't need keep incomplete file in user_data.
             * It can't use current free size of user_data to compare the length of input file
             * because filesystem is compress file system.
             * Even if current free size is smaller than the length of input file,
             * it still can write to user_data partition.
             */
            unlink(name);
            return FS_RETURN_NO_BUFFER;
        }
        fclose(fd);
#if (SYS_CPNT_FS_SUPPORT_UBIFS==TRUE) /*Need to let data to write from cache to disk immediately. Because UBIFS is write-back mech.*/ \
  ||(SYS_CPNT_FS_SUPPORT_EXT3==TRUE)
        sync();
#endif
        chown(name, FS_TYPE_STARTUP_TO_UID(file_type,exist_file_is_startup), -1);
        #if (SYS_CPNT_ONIE_SUPPORT==TRUE)
        /* set the permission of the tmpfile as "rwx------"
         */
        chmod(name, 0700);
        #endif
        ret=FS_RETURN_OK;
#else  /*#if (SYS_CPNT_FS_SUPPORT_WRITE_RUNTIME_TO_FILESYSTEM==TRUE)*/
    #if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==TRUE)
    #error "Not support SYS_CPNT_FS_SUPPORT_WRITE_RUNTIME_TO_FILESYSTEM set as FALSE and SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD set as TRUE"
    #endif
        FS_PARTITION_TABLE_T   *flash_entry_p = NULL;
        FS_FileHeader_T        file_header;
        UI32_T                 startup_mtd;

        if(FS_OM_GetStartupName(file_type, file_header.file_name) != TRUE)
        {
            startup_mtd = FS_MTD_NUMBER_NOT_EXIST;
        }
        else
        {
            FS_OM_GetFileHeaderByName(&file_header, FS_FILE_TYPE_MASK(file_type));
            startup_mtd = file_header.mtd_num;
        }

        memset(&file_header, 0, sizeof(FS_FileHeader_T));

        /* We can only write the non-startup runtime rootfs.*/

        ret = FS_RETURN_ERROR;
        while (NULL != (flash_entry_p = FS_GetNextFlashPartitionEntry(flash_entry_p)))
        {
            /* write on the first found partition
             */
            if((flash_entry_p->type != TYPE_RUNTIME) || (flash_entry_p->mtd_num == startup_mtd))
                continue;

            file_header.mtd_num = flash_entry_p->mtd_num;
            if(FS_OM_GetFirstFileHeaderByMtdNum(&file_header) == TRUE)
                continue;

            ret = FS_WRITE_RUNTIME_TO_FLASH(buf, length, flash_entry_p->mtd_num, FALSE);

            break;


        }
        if(ret == FS_RETURN_ERROR)
        {
            ERRORSTR("%s:ERROR, from FS_WRITE_RUNTIME_TO_FLASH(%ld)\r\n",__FUNCTION__,(long)ret);
            return ret;
        }
#endif  /*#if (SYS_CPNT_FS_SUPPORT_WRITE_RUNTIME_TO_FILESYSTEM==TRUE)*/

        SYS_TIME_GetUtcRealTimeBySec(&time);

        ret = FS_RETURN_OK;
    }
    else
    {
       /* If input cfg name=runtime name in board, it shall return fail.
        */
        if (TRUE == FS_OM_GetFileHeaderByName(&file_header,FS_FILE_TYPE_MASK_ALL))
        {
            if(file_header.file_type==FS_TYPE_OPCODE_FILE_TYPE)
            {
               ERRORSTR("\rInput cfg file_name is the same as runtime name in board\n");
               return FS_RETURN_ERROR;
            }

        }
#if  (SYS_CPNT_FLASHDRV_LIMIT_WRITE_CONFIG_SIZE==TRUE)
        /* Because if user-data space is full, system file(syslog, binary file..,)
         * can't write. Must make sure system file can write.
         */
        if(file_type==FS_FILE_TYPE_CONFIG)
        {
            if(length>SYS_ADPT_MAX_SIZE_OF_FILE_CONFIG)
            {
                ERRORSTR("FS_UpdateFile:ERROR, write size:%ld > allow_max_size_of_cfg:%d\r\n", (long)length, SYS_ADPT_MAX_SIZE_OF_FILE_CONFIG);
                return FS_RETURN_EXCEED_VALID_FILE_SIZE;
            }
        }
#endif

        if (FS_GetDeviceFileName(file_header.file_name, file_type, name) != TRUE)
        {
            ERRORSTR("%s(%d)FS_GetDeviceFileName returns FALSE.\n", __FUNCTION__, __LINE__);
            return FS_RETURN_ERROR;
        }
        fd = fopen(name, "w");
        if( fd == NULL )
        {
            ERRORSTR("FS_UpdateFile:ERROR, can not open file %s\r\n",name);
            return FS_RETURN_FILE_NEX;
        }
        usage_size = fwrite(buf, 1, length, fd);
        if (length > usage_size)
        {
            ERRORSTR("FS_UpdateFile:ERROR, write size %ld < require %ld\r\n",(long)usage_size, (long)length);
            fclose(fd);
            /* when "length > usage_size", it means there is no enough space.
             * So it doesn't need keep incomplete file in user_data.
             * It can't use current free size of user_data to compare the length of input file
             * because user_data use Jffs2 that is compress file system.
             * Even if current free size is smaller than the length of input file,
             * it still can write to user_data partition.
             */
            unlink(name);
            return FS_RETURN_NO_BUFFER;
        }
        fclose(fd);
#if (SYS_CPNT_FS_SUPPORT_UBIFS==TRUE) /*Need to let data to write from cache to disk immediately. Because UBIFS is write-back mech.*/ \
  ||(SYS_CPNT_FS_SUPPORT_EXT3==TRUE)
        sync();
#endif

        /*record file type in file system*/
        chown(name, FS_TYPE_STARTUP_TO_UID(file_type,exist_file_is_startup), -1);

        ret = FS_RETURN_OK;
    }

    /*License just needs to save in flash.*/
    if (file_type == FS_FILE_TYPE_LICENSE)
    {
        return ret;
    }

    /*
     * 3.0 Build file header
     */
    {
        char dev_filename[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];

        if (FS_AddDeviceFileNameMark(file_name, dev_filename, file_type) != TRUE)
        {
            ERRORSTR("%s(%d)FS_AddDeviceFileNameMark returns FALSE.\n", __FUNCTION__, __LINE__);
			return FS_RETURN_ERROR;
        }
        if (FS_BuildFileHeaderFromFile(dev_filename) != TRUE)
        {
            ERRORSTR("%s(%d)FS_BuildFileHeaderFromFile returns FALSE.\n", __FUNCTION__, __LINE__);
			return FS_RETURN_ERROR;
        }
    } 

    /* when startup runtime changed, change UC signature to
     * trigger UC reinit after reload.
     */
    if (file_type == FS_TYPE_OPCODE_FILE_TYPE && exist_file_is_startup)
    {
        UC_MGR_SetDirtySignature();
    }

    return  ret;
} /* End of FS_UpdateFile() */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_WriteFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function will write data from buffer to a file
 * INPUT    : drive         -- unit id or blade number
 *            file_name     -- the name of the file to write
 *            file_comment  -- the comment of the file
 *            file_type     -- file type
 *            buf           -- the source buffer
 *            length        -- data length to be writen (file size)
 *            resv_len      -- reserved length to guarantee the minimum size of
 *                             a file. resv_len < length will not reserve extra
 *                             space for the file.
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T FS_WriteFile(UI32_T drive, UI8_T  *file_name, UI8_T  *file_comment, UI32_T file_type, UI8_T *buf, UI32_T length, UI32_T resv_len)
{
    UI32_T  return_value;
#if (SYS_CPNT_STACKING == TRUE)
    FS_File_Attr_T  file_attr;
#endif

    FS_DUMMY_WRITE_RETURN(FS_RETURN_OK);

    if(file_name == NULL || buf == NULL || length == 0)
        return FS_RETURN_BAD_PARA;

#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
    /* The file type to write shall not be FS_FILE_TYPE_RUNTIME
     * when supporting ONIE.
     */
    if (file_type==FS_FILE_TYPE_RUNTIME)
    {
        BACKDOOR_MGR_Printf("%s(%d)Write file with FS_FILE_TYPE_RUNTIME is not allowed.\r\n", __FUNCTION__, __LINE__);
        return FS_RETURN_ERROR;
    }
#endif /* end of #if (SYS_CPNT_ONIE_SUPPORT==TRUE) */

    if(FS_GenericFilenameCheck(file_name, file_type) != FS_RETURN_OK)
    {
        DEBUGSTR("Error : File name %s contains an invalid character!\r\n", file_name);
        return FS_RETURN_BAD_PARA;
    }


#if (SYS_CPNT_STACKING == TRUE)
    /* Local operation */
    if (    (drive == DUMMY_DRIVE)
         || (drive == SYS_VAL_LOCAL_UNIT_ID)
         || (drive == FS_OM_GetControlDriverId())
       )
    {
        FS_SETBUSY();
        return_value = FS_UpdateFile(drive, file_name, file_comment, file_type, buf, length, resv_len, FALSE);
        FS_SETNOTBUSY();
        return return_value;
    }
    else /* Remote operation */
    {
        FS_SETBUSY();
        /* save the file information */
        memset(&(file_attr), 0, sizeof(FS_File_Attr_T));
        strcpy((char*)file_attr.file_name, (char*)file_name);
        strcpy((char*)file_attr.file_comment, (char*)file_comment);
        file_attr.file_type  = file_type;
        file_attr.file_size  = length;
        file_attr.reserv_size = resv_len;
        file_attr.privilage  = FALSE;
        file_attr.check_sum = FS_ByteSum(buf, length);


        DEBUGSTR("%s() File %s, Size %ld, Checksum: %02x\r\n",
                __FUNCTION__, file_name, (long)length, file_attr.check_sum);

        FS_OM_SetControlAttribution(&(file_attr));

        /* points fs_control.file_buffer to buf */
        FS_OM_SetControlFileBuffer(buf);
        /* Tell slave to Write file */
        return_value = FS_WriteRemoteFile(drive, &(file_attr) );
        FS_SETNOTBUSY();

        if(return_value == FS_RETURN_OK)
        {
            DBGMSG("Operation completed successfully");
        }
        else
        {
            ERRMSG(return_value);
        }
        FS_OM_SetControlFileBuffer(0);
        FS_ReturnIdle();
        return ( return_value );
    }
#else
    FS_SETBUSY();
    return_value = FS_UpdateFile(drive, (UI8_T*)file_name, file_comment, file_type, buf, length, resv_len, FALSE);
    FS_SETNOTBUSY();
    return return_value;
#endif
} /* End of FS_WriteFile */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_WriteProtectedFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function will write data from buffer to a protected file
 * INPUT    : drive         -- unit id or blade number
 *            file_name     -- the name of the file to write
 *            file_comment  -- the comment of the file
 *            file_type     -- file type
 *            buf           -- the source buffer
 *            length        -- data length to be writen (file size)
 *            resv_len      -- reserved length to guarantee the minimum size of
 *                             a file. resv_len < length will not reserve extra
 *                             space for the file.
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T FS_WriteProtectedFile(UI32_T drive, UI8_T  *file_name, UI8_T  *file_comment, UI32_T file_type, UI8_T *buf, UI32_T length, UI32_T resv_len)
{
    UI32_T  return_value;
#if (SYS_CPNT_STACKING == TRUE)
    FS_File_Attr_T  file_attr;
#endif

    if (FS_GenericFilenameCheck(file_name, file_type) == FS_RETURN_OK)
    {
        FS_SETBUSY();
#if (SYS_CPNT_STACKING == TRUE)
        if (    (drive == DUMMY_DRIVE)
             || (drive == SYS_VAL_LOCAL_UNIT_ID)
             || (drive == FS_OM_GetControlDriverId())
           )
        {
            /* Local operation */
            return_value = FS_UpdateFile(drive, file_name, file_comment, file_type, buf, length, resv_len, FALSE);
        }
        else /* Remote operation */
        {
            /* save the file information */
            memset(&(file_attr), 0, sizeof(FS_File_Attr_T));
            strcpy((char*)file_attr.file_name, (char*)file_name);
            strcpy((char*)file_attr.file_comment, (char*)file_comment);
            file_attr.file_type  = file_type;
            file_attr.file_size  = length;
            file_attr.reserv_size = resv_len;
            file_attr.privilage  = TRUE;
            file_attr.check_sum = FS_ByteSum(buf, length);
            FS_OM_SetControlAttribution(&(file_attr));
            FS_OM_SetControlFileBuffer(buf);
            /* Tell slave to Write file */
            return_value = FS_WriteRemoteFile(drive, &(file_attr));
            FS_OM_SetControlFileBuffer(0);
        }
#else
        return_value = FS_UpdateFile(drive, (UI8_T*)file_name, file_comment, file_type, buf, length, resv_len, TRUE);
#endif
        FS_SETNOTBUSY();
        return return_value;
    }
    else
    {

        ERRORSTR("Error : File name %s contains an invalid character!\r\n", file_name);
        return  FS_RETURN_ERROR;
    }
} /* End of FS_WriteProtectedFile */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_NumOfFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function returns the number of the file of a given type
 * INPUT    : file_type     -- the given file type
 * OUTPUT   : None
 * RETURN   : Number of the file
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
UI32_T  FS_NumOfFile(UI32_T file_type)
{
    FS_File_Attr_T file_attr;
    UI32_T  count = 0;
    UI32_T  drive = DUMMY_DRIVE;

    if (file_type >= FS_FILE_TYPE_TOTAL)
    {
        return 0;
    }

    file_attr.file_name[0]      = 0;
    file_attr.file_type_mask    = FS_FILE_TYPE_MASK(file_type);

    while (FS_GetNextFileInfo(&drive, &file_attr) == FS_RETURN_OK)
    {
        count++;
    }

    return count;
} /* End of FS_NumOfFile() */
/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetNumOfFileByUnitID
 * ------------------------------------------------------------------------
 * FUNCTION : This function returns the number of the file of a given type in unit
 * INPUT    : file_type     -- the given file type
              unit       --  unit id
 * OUTPUT   : None
 * RETURN   : Number of the file
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
UI32_T FS_GetNumOfFileByUnitID(UI32_T unit, UI32_T file_type)
{
    FS_File_Attr_T file_attr;
    UI32_T  count = 0;
    UI32_T  unit_id = unit;

    if (file_type >= FS_FILE_TYPE_TOTAL)
    {
        return 0;
    }

    memset(&file_attr, 0, sizeof(FS_File_Attr_T));
    file_attr.file_type_mask    = FS_FILE_TYPE_MASK(file_type);

    while (FS_GetNextFileInfo(&unit_id, &file_attr) == FS_RETURN_OK&& (unit_id == unit))
    {
        count++;
    }
    return count;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_UpdateStartupAttribute
 * ------------------------------------------------------------------------
 * FUNCTION : This function will set the startup attribute of a given type
 * INPUT    : file_type     -- the type of file
 *            file_name     -- the name of file
 *            privilege     -- TRUE : allow to cancel a file name for a
 *                                    specified file type with an input
 *                                    of NULL file name
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static  UI32_T  FS_UpdateStartupAttribute(UI32_T file_type, UI8_T *file_name, BOOL_T privilege)
{
    FS_FileHeader_T     file_header;
    BOOL_T              file_exist;
#if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE)
    FS_PARTITION_TABLE_T   *flash_entry_p = NULL;
#endif
    char                org_startup_file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN];

    /* File system initialized OK? */
    if (FS_OM_GetInitStatus() != FS_RETURN_OK)
        return FS_RETURN_NOT_READY;

    DEBUGSTR("%s:Update startup attribute ...\r\n",__FUNCTION__);

    if( (file_type == FS_TYPE_OPCODE_FILE_TYPE) && (file_name[0] == 0) )
    {
        ERRORSTR("%s:Warning, you can not clear runtime startup attribute \r\n",__FUNCTION__);
        return FS_RETURN_ACTION_INHIBITED;
    }

    if((file_name[0] == 0 && privilege != TRUE) || (file_name[0] != 0 && privilege == TRUE))
    {
        ERRORSTR("%s:Error, file name and provilege not match\r\n",__FUNCTION__);
        return FS_RETURN_ACTION_INHIBITED;
    }
    /* Existence check
     */

    if(file_type == FS_FILE_TYPE_CONFIG && file_name[0] == 0 &&privilege == TRUE)
    {
        FS_OM_SetStartupName(file_type, (UI8_T *)SYS_DFLT_restartConfigFile);

#if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE)
        while (NULL != (flash_entry_p = FS_GetNextFlashPartitionEntry(flash_entry_p)))
        {
            if(flash_entry_p->type == TYPE_USERDATA)
            {
                break;
            }
        }
        if(flash_entry_p==NULL)
        {
            ERRORSTR("%s:%d, cannot find TYPE_USERDATA partition entry",__FUNCTION__, __LINE__);
            return FS_RETURN_ERROR;
        }
#endif

        return FS_RETURN_OK;
    }

    strncpy((char*)file_header.file_name,(char*)file_name,SYS_ADPT_FILE_SYSTEM_NAME_LEN);
    file_header.file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN]='\0';

    file_exist = FS_OM_GetFileHeaderByName(&file_header,FS_FILE_TYPE_MASK(file_type));

    if (!(TRUE == privilege && (file_name[0] == 0) ))
    {
        if ( FALSE == file_exist )
        {
            /* file not found */
            ERRORSTR("%s:ERROR, file not find(%s)\r\n",__FUNCTION__, file_name);
            return FS_RETURN_FILE_NEX;
        }
        else
        {
#if (SYS_CPNT_ONIE_SUPPORT != TRUE)
            if (file_type == FS_TYPE_OPCODE_FILE_TYPE)
            {
                if( TRUE == file_header.startup)
                {
                    DEBUGSTR("%s: Same as current runtime %s\r\n",__FUNCTION__, file_name);
                    return FS_RETURN_OK;
                }
            }
#endif

#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
            /* Need to invoke the ONIE change startup installer script to
             * change the startup installer image.
             * FS_LOCK() had been called at the place that calls
             * FS_UpdateStartupAttribute, so no need to call FS_LOCK() again
             * before calling FS_InvokeAOSChangeStartupInstallerScript().
             */
            if(file_type == FS_TYPE_OPCODE_FILE_TYPE)
            {
                /* The installation script in ONIE change startup installer script
                 * will take care the setting of uid and the update of the file
                 * mapping file. So it is not required to handle the update of
                 * uid and file mapping file when SYS_CPNT_ONIE_SUPPORT is TRUE
                 */
                if (FS_InvokeAOSChangeStartupInstallerScript((const char*)file_name)==FALSE)
                {
                    ERRORSTR("%s:%d,ERROR, FS_InvokeAOSChangeStartupInstallerScript fail\r\n",__FUNCTION__, __LINE__);
                    return FS_RETURN_ERROR;
                }
            }
#endif /* end of #if (SYS_CPNT_ONIE_SUPPORT==TRUE)*/

            {
                /* update UID */
                char     dev_name[FS_MAX_PATHNAME_BUF_SIZE]={0};

                int ret;
                BOOL_T              org_startup_exist;

                if(FS_GetDeviceFileName(file_header.file_name, file_type, dev_name) != TRUE)
                {
                    ERRORSTR("%s(%d)FS_GetDeviceFileName returns FALSE.\n", __FUNCTION__, __LINE__);
                }
                ret=chown(dev_name, FS_TYPE_STARTUP_TO_UID(file_type,TRUE),-1);
                if(ret<0)
                {
                    ERRORSTR("Set chown fail, errno=%d, path_name=%s\n",errno, dev_name);
                }
                org_startup_exist=FS_OM_GetStartupName(file_type, (UI8_T*)org_startup_file_name);
                if(org_startup_exist==TRUE && strcmp((char *)org_startup_file_name, (char *)file_name)!=0)
                {
                    if (FS_GetDeviceFileName((UI8_T *)org_startup_file_name, file_type, dev_name) != TRUE)
                    {
                        ERRORSTR("%s(%d)FS_GetDeviceFileName returns FALSE.\n", __FUNCTION__, __LINE__);
                    }
                    ret=chown((char*)dev_name, FS_TYPE_STARTUP_TO_UID(file_type,FALSE),-1);
                    if(ret<0)
                    {
                        ERRORSTR("Set chown fail, errno=%d, org_startup_file_name=%s\n",errno, org_startup_file_name);
                    }
                }
            } 

#if (SYS_CPNT_FS_SUPPORT_RUNTIME_CACHE_PARTITION==TRUE)
            if (file_type == FS_TYPE_OPCODE_FILE_TYPE)
            {
                char* shell_cmd_p=NULL;
                UI32_T shell_rc;
                int   size=sizeof(UPDATE_CACHE_RUNTIME_PARTITION_SCRIPT_FILENAME) + 1/* for space */;

                /* check whether cache partition exists
                 */
                shell_rc = SYSFUN_ExecuteSystemShell(CHECK_CACHE_RUNTIME_PARTITION_SCRIPT_FILENAME);
                if (shell_rc == SYSFUN_OK)
                {
                    size+=strlen((char*)file_name) + 1 /* for null terminated char */;
                    shell_cmd_p = malloc(size);
                    if(shell_cmd_p)
                    {
                        sprintf(shell_cmd_p, "%s %s", UPDATE_CACHE_RUNTIME_PARTITION_SCRIPT_FILENAME, file_name);
                        shell_cmd_p[size-1] = '\0';
                        shell_rc = SYSFUN_ExecuteSystemShell(shell_cmd_p);
                        free(shell_cmd_p);
                        if (shell_rc != SYSFUN_OK)
                        {
                            ERRORSTR("\r\n%s:Execute system shell error(rc=%lu).\r\n", __FUNCTION__, (unsigned long)shell_rc);
                            /* Do not return error because this will not result to boot failure
                             */
                            /* return FS_RETURN_ERROR; */
                        }
                    }
                    else
                    {
                        ERRORSTR("\r\n%s:ERROR, failed to malloc %d bytes\r\n",
                            __FUNCTION__, size);
                        /* Do not return error because this will not result to boot failure
                         */
                        /* return FS_RETURN_ERROR; */
                    }
                }
                else
                {
                    DEBUGSTR("\r\n%s(%d):Execute system shell error(rc=%lu).\r\n", __FUNCTION__, __LINE__, (unsigned long)shell_rc);
                }

            }
#endif /* end of #if (SYS_CPNT_FS_SUPPORT_RUNTIME_CACHE_PARTITION==TRUE) */

            /* when startup runtime changed, change UC signature to
             * trigger UC reinit after reload.
             */
            if (file_type == FS_TYPE_OPCODE_FILE_TYPE)
            {
                UC_MGR_SetDirtySignature();
            }

            FS_OM_SetStartupName(file_type, file_name);
            return FS_RETURN_OK;
        }
    }
    return FS_RETURN_ERROR;
} /* End of FS_UpdateStartupAttribute() */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_SetStartupFilename
 * ------------------------------------------------------------------------
 * FUNCTION : This function will set the default file of a given type
 * INPUT    : drive         -- unit id or blade number
 *            file_type     -- the type of file
 *            file_name     -- the name of file
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */

UI32_T FS_SetStartupFilename(UI32_T drive, UI32_T file_type, UI8_T  *file_name)
{
    UI32_T  return_value;

    if (FS_GenericFilenameCheck(file_name, file_type) != FS_RETURN_OK)
    {
        DEBUGSTR("Error : File name %s contains a invalid character!\r\n", file_name);
        return  FS_RETURN_ERROR;
    }

#if (SYS_CPNT_STACKING == TRUE)
    UI32_T  retry_count;
#endif

#if (SYS_CPNT_STACKING == TRUE)
    /* Remote operation */
    if (    (drive != DUMMY_DRIVE)
         && (drive != SYS_VAL_LOCAL_UNIT_ID)
         && (drive != FS_OM_GetControlDriverId())
       )
    {
        FS_Request_Packet_T *fs_rq_packet_p;
        L_MM_Mref_Handle_T* mref_handle_p;
        UI32_T              pdu_len;

        /* Check if FS is in the right state */
        if(FS_OM_GetControlStackingMode() != SYS_TYPE_STACKING_MASTER_MODE)
        {
            ERRMSG(FS_RETURN_NOT_MASTER);
            return FS_RETURN_NOT_MASTER;
        }

        /* Check if drive exists */
        /*
        if(!STKTPLG_POM_UnitExist(drive))
        {
            ERRMSG(FS_RETURN_DRIVE_NOT_EXIST);
            return FS_RETURN_DRIVE_NOT_EXIST;
        }
        */

        DEBUGSTR("\r\n%s(): Remote operation for drive %d at line %d\r\n", __FUNCTION__, (int)drive, __LINE__);

        FS_SETBUSY();
        FS_LOCK_REMOTE();

        retry_count = 0;
        for(;;)
        {
            /* Prepare the request packet */
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Request_Packet_T), /* tx_buffer_size */
                                                  L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_SEND, FS_SET_ATTRIBUTE_REQUEST) /* user_id */
                                                 );
            fs_rq_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
            if (NULL == fs_rq_packet_p)
            {
                SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
                return_value = FS_RETURN_ERROR;
                break;
            }
            memset(fs_rq_packet_p, 0, sizeof(FS_Request_Packet_T));
            fs_rq_packet_p->header.opcode     = FS_SET_ATTRIBUTE_REQUEST;
            fs_rq_packet_p->header.data_size  = sizeof(FS_File_Attr_T);
            memcpy(&(fs_rq_packet_p->file_attr.file_name), file_name, strlen((char*)file_name));
            fs_rq_packet_p->file_attr.file_type    = file_type;
            fs_rq_packet_p->file_attr.startup_file = TRUE;
            fs_rq_packet_p->file_attr.privilage    = FALSE;

            fs_rq_packet_p->header.seq_no     = FS_OM_GetControlSeqNo();
            DUMP_TXPKT(*fs_rq_packet_p, GET_PDU_LEN(*fs_rq_packet_p));
            /* Send request packet and wait reply */
            memset(&fs_packet_buffer, 0, sizeof(fs_packet_buffer) );

            fs_packet_buffer.header.opcode = FS_ACK;/*init as success for ISC new design*/
            if (ISC_RemoteCall((UI8_T)drive, ISC_FS_SID, mref_handle_p,
                                SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                sizeof(fs_packet_buffer), (UI8_T *) &fs_packet_buffer,
                                FS_ISC_TRY_COUNT, FS_TIMEOUT_VALUE) )
            {
                FS_OM_AddControlSeqNo();

                DUMP_RXPKT(fs_packet_buffer, GET_PDU_LEN(fs_packet_buffer));
                if(fs_packet_buffer.header.opcode == FS_ACK)
                {
                    return_value = FS_CheckRemoteStatus(drive);
                    break;
                }
                else
                {
                    return_value = fs_packet_buffer.data.error_code;
                    ERRMSG(return_value);
                    FS_AbortSequence(drive);
                    if(retry_count++ >= FS_COMMAND_RETRY_COUNT)
                    {
                        return_value = FS_AbortSequence(drive);
                        ERRMSG(return_value);
                        break;
                    }
                    else
                    {
                        DBGMSG("Try Again");
                        SYSFUN_Sleep(FS_SLEEP_TIME);
                        continue;
                    }
                }
            }
            else
            {
                ERRMSG(FS_RETURN_TIMEOUT);
                return_value = FS_AbortSequence(drive);
                break;
            }
        }
        FS_UNLOCK_REMOTE();
        FS_ReturnIdle();
        FS_SETNOTBUSY();
        return return_value;
    }/* End of remote operation */
    else
    {
        FS_SETBUSY();
        return_value = FS_UpdateStartupAttribute(file_type, file_name, FALSE);
        FS_SETNOTBUSY();
        return return_value;
    }
#else /* SYS_ADPT_INCLUDE_STACKING */
    /* Local operation */
    FS_SETBUSY();
    return_value = FS_UpdateStartupAttribute(file_type, file_name, FALSE);
    FS_SETNOTBUSY();
    return return_value;
#endif /* SYS_ADPT_INCLUDE_STACKING */

}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_ResetStartupFilename
 * ------------------------------------------------------------------------
 * FUNCTION : This function will set the default file of a given type
 * INPUT    : drive         -- unit id or blade number
 *            file_type     -- the type of file
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : 1. drive represents either the unit id in a stack or the blade
 *               number in a chasis.
 *            2. This function can be invoked in transition mode. Set drive to
 *               DUMMY_DRIVE to indicate a local operation.
 * ------------------------------------------------------------------------
 */
UI32_T FS_ResetStartupFilename(UI32_T drive, UI32_T file_type)
{
    UI32_T  return_value;

    if(file_type <= FS_FILE_TYPE_SUBFILE || file_type >= FS_FILE_TYPE_TOTAL)
        return FS_RETURN_ERROR;

#if(sys_cpnt_stacking == TRUE)
    /* Remote operation */
    if (    (drive != DUMMY_DRIVE)
         && (drive != SYS_VAL_LOCAL_UNIT_ID)
         && (drive != FS_OM_GetControlDriverId())
       )
    {
        FS_Request_Packet_T *fs_rq_packet_p;
        L_MM_Mref_Handle_T* mref_handle_p;
        UI32_T              pdu_len;

        /* Check if FS is in the right state */
        if(FS_OM_GetControlStackingMode() != SYS_TYPE_STACKING_MASTER_MODE)
        {
            return FS_RETURN_NOT_MASTER;
        }

        DEBUGSTR("\r\n%s(): Remote operation for drive %d at line %d\r\n", __FUNCTION__, (int)drive, __LINE__);
        FS_SETBUSY();
        FS_LOCK_REMOTE();

        for(;;)
        {
            /* Prepare request packet to send */
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Request_Packet_T), /* tx_buffer_size */
                                                  L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_SEND, FS_SET_ATTRIBUTE_REQUEST) /* user_id */
                                                 );
            fs_rq_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
            if (NULL == fs_rq_packet_p)
            {
                DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
                return_value = FS_RETURN_ERROR;
                break;
            }
            memset(fs_rq_packet_p, 0, sizeof(FS_Request_Packet_T));
            fs_rq_packet_p->header.opcode          = FS_SET_ATTRIBUTE_REQUEST;
            fs_rq_packet_p->header.data_size       = sizeof(FS_File_Attr_T);
            fs_rq_packet_p->file_attr.file_type    = file_type;
            fs_rq_packet_p->file_attr.startup_file = TRUE;
            fs_rq_packet_p->file_attr.privilage    = TRUE;

            fs_rq_packet_p->header.seq_no     = FS_OM_GetControlSeqNo();
            DUMP_TXPKT(*fs_rq_packet_p, GET_PDU_LEN(*fs_rq_packet_p));

            /* Send request and wait reply */
            memset(&fs_packet_buffer, 0, sizeof(fs_packet_buffer) );

            fs_packet_buffer.header.opcode = FS_ACK;/*init as success for ISC new design*/
            if (ISC_RemoteCall((UI8_T)drive, ISC_FS_SID, mref_handle_p,
                                SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                sizeof(fs_packet_buffer), (UI8_T *) &fs_packet_buffer,
                                FS_ISC_TRY_COUNT, FS_TIMEOUT_VALUE) )
            {
                FS_OM_AddControlSeqNo();
                DUMP_RXPKT(fs_packet_buffer, GET_PDU_LEN(fs_packet_buffer));

                if(fs_packet_buffer.header.opcode == FS_ACK)
                {/* ACK received */
                    return_value = FS_CheckRemoteStatus(drive);
                    break;
                }
                else
                {/* NAK received */
                    return_value = fs_packet_buffer.data.error_code;
                    ERRMSG(return_value);
                    FS_AbortSequence(drive);
                    if(retry_count++ >= FS_COMMAND_RETRY_COUNT)
                    {
                        ERRMSG(return_value);
                        break;
                    }
                    else
                    {
                        DBGMSG("Try Again");
                        SYSFUN_Sleep(FS_SLEEP_TIME);
                    }
                }
            }
            else
            {
                ERRMSG(FS_RETURN_TIMEOUT);
                return_value = FS_AbortSequence(drive);
                break;
            }
        }
        FS_UNLOCK_REMOTE();
        FS_ReturnIdle();
        FS_SETNOTBUSY();
        return return_value;
    }/* End of remote operation */
    else
    {
        FS_SETBUSY();
        return_value = FS_UpdateStartupAttribute(file_type, (UI8_T*)"", TRUE);
        FS_SETNOTBUSY();
        return return_value;
    }
#else
    FS_SETBUSY();
    return_value = FS_UpdateStartupAttribute(file_type, (UI8_T*)"", TRUE);
    FS_SETNOTBUSY();
    return return_value;
#endif

}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetStartupFilename
 * ------------------------------------------------------------------------
 * FUNCTION : This function will get the default file of a given type
 * INPUT    : drive         -- unit id or blade number
 *            file_type     -- the type of file
 * OUTPUT   : file_name     -- the name of file
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T FS_GetStartupFilename(UI32_T drive, UI32_T file_type, UI8_T  *file_name)
{
    /* File system initialized OK? */
    if (FS_OM_GetInitStatus() != FS_RETURN_OK)
        return FS_RETURN_NOT_READY;

    if ( (file_type == 0) || (file_type >= FS_FILE_TYPE_TOTAL) )
    {
        ERRORSTR("%s:ERROR, bad parameter.\r\n",__FUNCTION__);
        return FS_RETURN_INDEX_OOR;
    }

    if (file_name == 0)
    {
        ERRORSTR("%s:ERROR, bad parameter.\r\n",__FUNCTION__);
        return FS_RETURN_BAD_PARA;
    }

#if (SYS_CPNT_STACKING == TRUE)
    if (    (drive == DUMMY_DRIVE)
         || (drive == SYS_VAL_LOCAL_UNIT_ID)
         || (drive == FS_OM_GetControlDriverId())
       )
    {
#endif
        DEBUGSTR("\r\n%s() Drive: %ld\r\n", __FUNCTION__, (long)drive);

        if (TRUE == FS_OM_GetStartupName(file_type, file_name))
        {
            return FS_RETURN_OK;
        }
        else
        {
            ERRORSTR("%s:ERROR, no startup for file_type=%lu.\r\n",__FUNCTION__, (unsigned long)file_type);
            return FS_RETURN_ERROR;
        }

#if (SYS_CPNT_STACKING == TRUE)
    }
    else /* Remote operation */
    {
        UI32_T                  return_value;

        DEBUGSTR("\r\n%s() Drive: %ld\r\n", __FUNCTION__, (long)drive);

        file_name[0] = 0;
        FS_SETBUSY();
        fs_file_attr_packet.file_attr[0].file_type = file_type;
        return_value = FS_GetRemoteStartupFilename(drive, &fs_file_attr_packet);

        if (return_value == FS_RETURN_FILE_NEX)
        {
            ;
        }

        if(return_value != FS_RETURN_OK)
        {
            FS_SETNOTBUSY();
            ERRMSG(return_value);
            return return_value;
        }

        strcpy((char*)file_name, (char*)fs_file_attr_packet.file_attr[0].file_name);
        FS_SETNOTBUSY();

        return return_value;
    }
#endif
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_WriteLoader
 * ------------------------------------------------------------------------
 * FUNCTION : This function will write loader to flash
 * INPUT    : drive         -- unit id or blade number
 *            loader        -- loader to write
 *            size          -- size of the loader
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T FS_WriteLoader(UI32_T drive, UI8_T *loader, UI32_T size)
{
    UI32_T  result = FS_RETURN_ERROR;
#if (SYS_CPNT_STACKING == TRUE)
    FS_File_Attr_T  file_attr;
#endif

#if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE)
    FS_PARTITION_TABLE_T *flash_entry_p = NULL;
#else
    UI32_T uboot_mtd_id;
#endif
    /* Local operation */
    FS_SETBUSY();

#if (SYS_CPNT_STACKING == TRUE)
    if (    (drive == DUMMY_DRIVE)
         || (drive == SYS_VAL_LOCAL_UNIT_ID)
         || (drive == FS_OM_GetControlDriverId())
       )
    {
#endif
#if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==TRUE)
        if(FS_GetMtdDevIdByMtdType(FS_MTD_PART_UBOOT, &uboot_mtd_id)==TRUE)
        {
            result=FS_WRITE_LOADER_TO_FLASH(loader, size, uboot_mtd_id, TRUE);
        }
        else
        {
            result=FS_RETURN_BAD_HW;
        }
#else
        while (NULL != (flash_entry_p = FS_GetNextFlashPartitionEntry(flash_entry_p)))
        {
            /* write on the first found partition
             */
            if( flash_entry_p->type == TYPE_BOOTLOADER)
            {
                result=FS_WRITE_LOADER_TO_FLASH(loader, size, flash_entry_p->mtd_num, TRUE);
                break;
            }

        }
        if(flash_entry_p==NULL)
        {
            ERRORSTR("%s:ERROR, cannot find TYPE_BOOTLOADER in flash partition entry\r\n",__FUNCTION__);
            FS_SETNOTBUSY();
            return result;
        }
#endif /*End #if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==TRUE)*/

#if (SYS_CPNT_STACKING == TRUE)
    }
    else /* Remote operation */
    {
        /* save the file information */
        memset(&(file_attr), 0, sizeof(FS_File_Attr_T));
        file_attr.file_type  = FS_FILE_TYPE_TOTAL;
        file_attr.file_size  = size;
        file_attr.privilage  = TRUE;
        file_attr.check_sum = FS_ByteSum(loader, size);
        FS_OM_SetControlAttribution(&(file_attr));
        FS_OM_SetControlFileBuffer(loader);
        /* Tell slave to Write file */
        result  = ( FS_WriteRemoteFile(drive, &(file_attr)) );
    }
#endif
    FS_SETNOTBUSY();
    return result;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_ReadMtdDev
 * ------------------------------------------------------------------------
 * FUNCTION : Read data from the specified mtd device id from the given
 *            offset.
 * INPUT    : mtd_dev_id  - The mtd device id
 *            offset      - The offset to the beginning of the given mtd device
 *                          to read the data
 *            read_data_len_p
 *                        - The length of the data to be read
 * OUTPUT   : read_data_len_p
 *                        - The actual length of data read from the mtd device
 *            read_data_buf_p
 *                        - The data read from the mtd device
 * RETURN   : TRUE - Success, FALSE - Error
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T FS_ReadMtdDev(UI32_T mtd_dev_id, UI32_T offset, UI32_T* read_data_len_p, void *read_data_buf_p)
{
    int fd;
    ssize_t read_len;
    char mtd_name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+1];
    BOOL_T ret_val=TRUE;

    if ((read_data_len_p==NULL) || (read_data_buf_p==NULL))
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid input arg.\r\n",
            __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (*read_data_len_p==0)
    {
        return TRUE;
    }

    snprintf(mtd_name, SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+1, "%s%lu", FS_FLASH_MTD_PREFIX, (unsigned long)mtd_dev_id);
    fd = open(mtd_name, O_RDONLY);
    if (fd<0)
    {
        BACKDOOR_MGR_Printf("%s(%d)open %s error(errno=%d).\r\n",
            __FUNCTION__, __LINE__, mtd_name, errno);
        return FALSE;
    }
    if (lseek(fd, offset, SEEK_SET) == (off_t)-1)
    {
        BACKDOOR_MGR_Printf("%s(%d)lseek on %s error(errno=%d).\r\n",
            __FUNCTION__, __LINE__, mtd_name, errno);
        ret_val=FALSE;
        goto exit;
    }

    read_len=read(fd, read_data_buf_p, *read_data_len_p);

    if (read_len == (ssize_t)-1)
    {
        BACKDOOR_MGR_Printf("%s(%d)read on %s error(errno=%d).\r\n",
            __FUNCTION__, __LINE__, mtd_name, errno);
        ret_val=FALSE;
        goto exit;

    }

    *read_data_len_p=(UI32_T)read_len;

exit:
    close(fd);
    return ret_val;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_EraseMtdDev
 * ------------------------------------------------------------------------
 * FUNCTION : Erase the data at the specified offset with the given length.
 * INPUT    : mtd_dev_id  - The mtd device id
 *            offset      - The offset to the beginning of the given mtd device
 *                          to erase the data
 *            erase_len   - The length of the data to be erased
 * OUTPUT   : None
 * RETURN   : TRUE - Success, FALSE - Error
 * NOTE     : 1. This function provides the functionality to perform low-level
 *               erase operation to a flash through the mtd device.
 *            2. offset and erase_len must be aligned to the block size.
 * ------------------------------------------------------------------------
 */
BOOL_T FS_EraseMtdDev(UI32_T mtd_dev_id, UI32_T offset, UI32_T erase_len)
{
    erase_info_t erase_info;
    int fd, rc;
    char mtd_name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+1];
    BOOL_T ret_val=TRUE;

    snprintf(mtd_name, SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+1, "%s%lu", FS_FLASH_MTD_PREFIX, (unsigned long)mtd_dev_id);
    fd = open(mtd_name, O_WRONLY);
    if (fd<0)
    {
        BACKDOOR_MGR_Printf("%s(%d)open %s error(errno=%d).\r\n",
            __FUNCTION__, __LINE__, mtd_name, errno);
        return FALSE;
    }

    erase_info.start=offset;
    erase_info.length=erase_len;
    rc=ioctl(fd, MEMERASE, &erase_info);
    if (rc != 0)
    {
        BACKDOOR_MGR_Printf("%s(%d)erase %s failed(errno=%d, offset=0x%08lX,erase_len=0x%08lX).\r\n",
            __FUNCTION__, __LINE__, mtd_name, errno, (unsigned long)offset, (unsigned long)erase_len);
        ret_val=FALSE;
        goto exit;
    }

exit:
    close(fd);
    return ret_val;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_WriteMtdDev
 * ------------------------------------------------------------------------
 * FUNCTION : Write the data at the specified offset with the given length.
 * INPUT    : mtd_dev_id  - The mtd device id
 *            offset      - The offset to the beginning of the given mtd device
 *                          to write the data
 *            write_data_buf_p
 *                        - The data to be written to the mtd device
 *            write_data_len_p
 *                        - The length of the data to be written
 * OUTPUT   : write_data_len_p
 *                        - The actual length of the data written to the mtd device
 * RETURN   : TRUE - Success, FALSE - Error
 * NOTE     : 1. This function provides the functionality to perform low-level
 *               write operation to the specified location of a flash through
 *               the mtd device.
 *            2. The data to be written to must have been erased.
 * ------------------------------------------------------------------------
 */
BOOL_T FS_WriteMtdDev(UI32_T mtd_dev_id, UI32_T offset, void *write_data_buf_p, UI32_T* write_data_len_p)
{
    int fd;
    ssize_t write_len;
    char mtd_name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+1];
    BOOL_T ret_val=TRUE;

    if ((write_data_buf_p==NULL) || (write_data_len_p==NULL))
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid input arg.\r\n",
            __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (*write_data_len_p==0)
    {
        return TRUE;
    }

    snprintf(mtd_name, SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+1, "%s%lu", FS_FLASH_MTD_PREFIX, (unsigned long)mtd_dev_id);
    fd = open(mtd_name, O_WRONLY);
    if (fd<0)
    {
        BACKDOOR_MGR_Printf("%s(%d)open %s error(errno=%d).\r\n",
            __FUNCTION__, __LINE__, mtd_name, errno);
        return FALSE;
    }
    if (lseek(fd, offset, SEEK_SET) == (off_t)-1)
    {
        BACKDOOR_MGR_Printf("%s(%d)lseek on %s error(errno=%d).\r\n",
            __FUNCTION__, __LINE__, mtd_name, errno);
        ret_val=FALSE;
        goto exit;
    }

    write_len=write(fd, write_data_buf_p, *write_data_len_p);

    if (write_len == (ssize_t)-1)
    {
        BACKDOOR_MGR_Printf("%s(%d)write on %s error(errno=%d).\r\n",
            __FUNCTION__, __LINE__, mtd_name, errno);
        ret_val=FALSE;
        goto exit;

    }

    *write_data_len_p=(UI32_T)write_len;

exit:
    close(fd);
    return ret_val;
}

#if (SYS_CPNT_STACKING == TRUE)
/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_ByteSum
 * ------------------------------------------------------------------------
 * FUNCTION : This function will calculate the byte sum of the buffer
 * INPUT    : buf_ptr       -- pointer to the buffer
 *            length        -- length of the buffer
 * OUTPUT   : None
 * RETURN   : byte sum of the specified buffer
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static  UI8_T   FS_ByteSum(UI8_T *buf_ptr, UI32_T length)
{
    UI8_T   sum = 0;
    UI32_T  i;

    for (i=0; i < length; i++)
        sum = sum + buf_ptr[i];

    return sum;
} /* End of FS_ByteSum */
#endif

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_SetTransitionMode
 * ------------------------------------------------------------------------
 * FUNCTION : This function signal FS to prepare to enter transition mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 * ------------------------------------------------------------------------
 */
void FS_SetTransitionMode(void)
{
    return;
} /* End of FS_SetTransitionMode */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_EnterTransitionMode
 * ------------------------------------------------------------------------
 * FUNCTION : This function will put FS in transition mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. Local file operations is permitted in all three states.
 *            2. State transition can't take place while FS is executing
 *               file operation.
 * ------------------------------------------------------------------------
 */
void FS_EnterTransitionMode(void)
{
    return;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_EnterMasterMode
 * ------------------------------------------------------------------------
 * FUNCTION : This function will put FS in master mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. Local file operations is permitted in all three states.
 *            2. State transition can't take place while FS is executing
 *               file operation.
 * ------------------------------------------------------------------------
 */
void FS_EnterMasterMode(void)
{
#if (SYS_CPNT_STACKING == TRUE)
    UI32_T  dri_id;
    FS_OM_SetControlState(FS_STATE_IDLE);
    FS_OM_SetControlStackingMode(SYS_TYPE_STACKING_MASTER_MODE);
    FS_OM_SetControlFileBuffer(0);
    STKTPLG_POM_GetMyUnitID(&dri_id);
    FS_OM_SetControlDriverId(dri_id);
#endif
    FS_Restart();

    if (FS_OM_GetTaskId())
    {
        if (FS_OM_GetFileChecksum())
        {
            DBGMSG("Checksum verification for all files have already been verified.");
        }
        else
        {
            if ( SYSFUN_SendEvent(FS_OM_GetTaskId(), FS_TYPE_EVENT_CHECKFS) == SYSFUN_OK
               )
            {
                DBGMSG("Remote write");
            }
            else
            {
                DBGMSG("FS: set event failed");
            }
        }
    } /* End of if (_fs_task_existing_) */

    return;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_EnterSlaveMode
 * ------------------------------------------------------------------------
 * FUNCTION : This function will put FS in slave mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. Local file operations is permitted in all three states.
 *            2. State transition can't take place while FS is executing
 *               file operation.
 * ------------------------------------------------------------------------
 */
void FS_EnterSlaveMode(void)
{
#if (SYS_CPNT_STACKING == TRUE)
    UI32_T  dri_id;
    FS_OM_SetControlState(FS_STATE_IDLE);
    FS_OM_SetControlStackingMode(SYS_TYPE_STACKING_SLAVE_MODE);
    FS_OM_SetControlFileBuffer(0);
    STKTPLG_POM_GetMyUnitID(&dri_id);
    FS_OM_SetControlDriverId(dri_id);
#endif
    FS_Restart();
    if (FS_OM_GetTaskId())
    {
        if (FS_OM_GetFileChecksum())
        {
            DBGMSG("Checksum verification for all files have already been verified.");
        }
        else
        {
            if ( SYSFUN_SendEvent(FS_OM_GetTaskId(), FS_TYPE_EVENT_CHECKFS) == SYSFUN_OK
               )
            {
                DBGMSG("Remote write");
            }
            else
            {
                DBGMSG("FS: set event failed");
            }
        }
    } /* End of if (_fs_task_existing_) */

    return;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_ProvisionComplete
 * ------------------------------------------------------------------------
 * FUNCTION : This function will perform the operation when the provision
 *            is completed.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void FS_ProvisionComplete(void)
{
    if (FS_OM_GetTaskId())
    {
        if (FS_OM_GetFileChecksum())
        {
            DBGMSG("Checksum verification for all files have already been verified.");
        }
        else
        {
            if ( SYSFUN_SendEvent(FS_OM_GetTaskId(), FS_TYPE_EVENT_CHECKFS) == SYSFUN_OK
               )
            {
                DBGMSG("Remote write");
            }
            else
            {
                DBGMSG("FS: set event failed");
            }
        }
    } /* End of if (_fs_task_existing_) */

    return;
} /* End of FS_ProvisionComplete() */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_Shutdown
 * ------------------------------------------------------------------------
 * FUNCTION : This function will set the shutdown flag to terminate all the
 *            access to the FS. All the FS operations will be rejected
 *            after this function is invoked and this function will not
 *            return until no operation is performing (or trying to perform)
 *            accessing the FS.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : -- Rapid shutdown procedure:
 *               1. set the flag;
 *               2. lock;
 *               3. unlock;
 *            -- Perfect shutdown procedure:
 *               1. lock;
 *               2. set the flag;
 *               3. unlock;
 * ------------------------------------------------------------------------
 */
void FS_Shutdown(void)
{
    FS_OM_SetShutdownFlag(TRUE);
    FS_SEMAPHORE_LOCK();
    FS_SEMAPHORE_UNLOCK();

    /* When SYS_CPNT_ONIE_SUPPORT is TRUE, all of the mounted file systems
     * will be unmounted through rc scripts
     */
    #if (SYS_CPNT_ONIE_SUPPORT!=TRUE)
    /* On ES4627MB-FLF-EC, it uses Nand flash + UBIFS,
     * it is found that reload without unmount flash will see the messages
     * in kernel log shown below:
     * --------
     * UBIFS: recovery needed
     * UBIFS: recovery completed
     * --------
     * If do unmount flash before reload, the messages are gone,
     * it is believed that doing unmount flash before reload is a necessary
     * operation before reboot.
     */
    system("umount flash");
    #endif

    return;
} /* End of FS_Shutdown */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_Restart
 * ------------------------------------------------------------------------
 * FUNCTION : This function will clear the shutdown flag to allow the
 *            access to the FS.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void FS_Restart(void)
{
    FS_SEMAPHORE_LOCK();
    FS_OM_SetShutdownFlag(FALSE);
    FS_SEMAPHORE_UNLOCK();
    return;
} /* End of FS_Restart */

#if (SYS_CPNT_ONIE_SUPPORT==TRUE)

/****************************************
 * project dependant declarations START *
 ****************************************/
#if defined(AOS5600_52X) || defined(AOS5600_52X_OF)
FS_TYPE_ONIEProductName_PIDBID_Map_Entry_T product_name_to_pidbid_map[] =
{
    {"5600-52X", 463, 0},
    {"5610-52X", 463, 2},
};
#elif (defined(AOS5700_54X) || defined(VESTA5700_54X))
FS_TYPE_ONIEProductName_PIDBID_Map_Entry_T product_name_to_pidbid_map[] =
{
    {"5710-54X", SYS_ADPT_PROJECT_ID, 0},
    {"6700-32X", SYS_ADPT_PROJECT_ID, 1},
};
#elif defined(AOS4600_54T)
FS_TYPE_ONIEProductName_PIDBID_Map_Entry_T product_name_to_pidbid_map[] =
{
    {"4600-54T", 466, 0},
};
#elif defined(AOS7710_32X) || defined(VESTA7710_32X)
FS_TYPE_ONIEProductName_PIDBID_Map_Entry_T product_name_to_pidbid_map[] =
{
    {"7712-32X", SYS_ADPT_PROJECT_ID, 0},
    {"7312-54X", SYS_ADPT_PROJECT_ID, 1},
    {"7712-16X", SYS_ADPT_PROJECT_ID, 2},
};
#elif defined(AOS5510_54X) || defined(VESTA5510_54X)
FS_TYPE_ONIEProductName_PIDBID_Map_Entry_T product_name_to_pidbid_map[] =
{
    {"5512-54X", SYS_ADPT_PROJECT_ID, 0},
    {"CS6550-48S6Q-SI", SYS_ADPT_PROJECT_ID, 0},
    {"C1020", SYS_ADPT_PROJECT_ID, 0},
};
#elif defined(AOS5710_54X)
#define ONIE_EEPROM_HARD_CODE_PRODUCT_NAME "5710-54X"
#define ONIE_EEPROM_HARD_CODE_SERIAL_NUMBER "1234567890"
#define ONIE_EEPROM_HARD_CODE_MAC_BASE 0x0,0x0,0x0,0x0,0x0,0x1
#define ONIE_EEPROM_HARD_CODE_MANUF_DATE "2015-12-10"
#define ONIE_EEPROM_HARD_CODE_LABEL_REVISION "RXX"
FS_TYPE_ONIEProductName_PIDBID_Map_Entry_T product_name_to_pidbid_map[] =
{
    {"5710-54X", 472, 0},
};
#elif defined(AOS5810_54X)
FS_TYPE_ONIEProductName_PIDBID_Map_Entry_T product_name_to_pidbid_map[] =
{
    {"5812-54X", 479, 0},
    {"5812-54T", 479, 1},
    {"6812-32X", 479, 2},
};
#elif defined(VESTA5810_54X)
FS_TYPE_ONIEProductName_PIDBID_Map_Entry_T product_name_to_pidbid_map[] =
{
    {"5812-54X", 500, 0},
    {"5812-54T", 500, 1},
    {"6812-32X", 500, 2},
};
#else
#error "Project dependant array product_name_to_pidbid_map is not defined!"
#endif
/**************************************
 * project dependant declarations END *
 **************************************/

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetPIDAndBIDByProductName
 * ------------------------------------------------------------------------
 * FUNCTION : Get the PID and BID in FS_HW_Info_T according to the given
 *            product_name in ONIE EEPROM data.
 * INPUT    : product_name    - the product name in ONIE EEPROM data.
 *            product_name_sz - number of char in product_name not including
 *                              terminated NULL char.
 * OUTPUT   : hwinfo_p     - the PID and BID will be filled into the given
 *                           FS_HW_Info_T.
 * RETURN   : TRUE : The PID and BID are filled into hwinfo_p.
 *            FALSE: The PID and BID are not filled into hwinfo_p due to
 *                   error. (Not recognized prduct name)
 * NOTE     :
 *            1.For each project that defines SYS_CPNT_ONIE_SUPPORT as TRUE,
 *              it must define its own product_name_to_pidbid_map.
 *            2.As long as the string in product_name_to_pidbid_map[n].product_name
 *              can be found in product_name, this function will set the
 *              corresponding project_id and board_id of the entry into hwinfo_p.
 * ------------------------------------------------------------------------
 */
static BOOL_T FS_GetPIDAndBIDByProductName(char* product_name, UI8_T product_name_sz, FS_HW_Info_T *hwinfo_p)
{
    UI32_T array_size = sizeof(product_name_to_pidbid_map)/sizeof(product_name_to_pidbid_map[0]);
    UI32_T i;
    BOOL_T found=FALSE;
    UI8_T product_name_local[product_name_sz+1]; /* reserve one extra byte for null terminated char */

    if (product_name==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)product_name is NULL.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (product_name_sz==0)
    {
        BACKDOOR_MGR_Printf("%s(%d)product_name_sz is 0.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (hwinfo_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)hwinfo_p is NULL.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    memcpy(product_name_local, product_name, product_name_sz);
    product_name_local[product_name_sz]='\0';
    
    DEBUGSTR("%s(%d)Device Product Name:'%s'\r\n", __FUNCTION__, __LINE__,
        product_name_local);

    for (i=0; i<array_size; i++)
    {
        DEBUGSTR("%s(%d)Compare entry[%lu] Product name '%s'\r\n",
            __FUNCTION__, __LINE__, (unsigned long)i, product_name_to_pidbid_map[i].product_name);

        if (strstr(product_name, product_name_to_pidbid_map[i].product_name))
        {
            hwinfo_p->project_id=product_name_to_pidbid_map[i].project_id;
            hwinfo_p->board_id=product_name_to_pidbid_map[i].board_id;
            found=TRUE;
            DEBUGSTR("%s(%d)[%lu]Product name '%s' matched. PID=%lu, BID=%lu\r\n",
                __FUNCTION__, __LINE__, (unsigned long)i, product_name_to_pidbid_map[i].product_name,
                (unsigned long)product_name_to_pidbid_map[i].project_id, (unsigned long)product_name_to_pidbid_map[i].board_id);
            break;
        }
    }

    return found;
}

#if (ONIE_EEPROM_HARD_CODE!=TRUE) && (SYS_CPNT_FS_HWINFO_FROM_ONLP==FALSE)
/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_IsValidTlvinfoHeader
 * ------------------------------------------------------------------------
 * FUNCTION : Validate the given tlvinfo_header_t.
 * INPUT    : hdr - the pointer to tlvinfo_header_t.
 * OUTPUT   : None
 * RETURN   : TRUE : Valid header.
 *            FALSE: Invalid header.
 * NOTE     : This function performs sanity checks on the first 11 bytes of
 *            the TlvInfo EEPROM data pointed to by the parameter:
 *            1. First 8 bytes contain null-terminated ASCII string "TlvInfo"
 *            2. Version byte is 1
 *            3. Total length bytes contain value which is less than or equal
 *               to the allowed maximum (2048-11)
 * ------------------------------------------------------------------------
 */
static BOOL_T FS_IsValidTlvinfoHeader(tlvinfo_header_t *hdr_p)
{
    return( (strcmp(hdr_p->signature, TLV_INFO_ID_STRING) == 0) &&
        (hdr_p->version == TLV_INFO_VERSION) &&
        (L_STDLIB_Ntoh16(hdr_p->totallen) <= TLV_TOTAL_LEN_MAX) );
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_IsValidONIEEeprom
 * ------------------------------------------------------------------------
 * FUNCTION : Validate the given eeprom_p.
 * INPUT    : eeprom_p - the pointer to the beginning of ONIE EEPROM.
 *                       (i.e. tlvinfo_header_t)
 * OUTPUT   : None
 * RETURN   : TRUE : Valid ONIE EEPROM.
 *            FALSE: Invalid ONIE EEPROM.
 * NOTE     : This function performs:
 *            1. Sanity checks on the first 11 bytes of
 *               the TlvInfo EEPROM data header pointed by eeprom_p first.
 *               (i.e. tlvinfo_header_t)
 *            2. Make sure the last TLV is a CRC-32 TLV.
 *            3. Validate the EEPROM data part through the comparision of
 *               calculated CRC32 and the value stored in the EEPROM CRC-32
 *               TLV.
 * ------------------------------------------------------------------------
 */
static BOOL_T FS_IsValidONIEEeprom(UI8_T *eeprom_p)
{
    UI32_T calc_crc, stored_crc;
    tlvinfo_header_t *hdr_p = (tlvinfo_header_t*)eeprom_p;
    tlvinfo_tlv_t    *eeprom_crc_p;

    if (FS_IsValidTlvinfoHeader(hdr_p)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid EEPROM header.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    /* Is the last TLV a CRC?
     */
    eeprom_crc_p = (tlvinfo_tlv_t *) (eeprom_p + sizeof(tlvinfo_header_t) +
        L_STDLIB_Ntoh16(hdr_p->totallen) - (sizeof(tlvinfo_tlv_t) + 4) );
    if ((eeprom_crc_p->type != TLV_CODE_CRC_32) || (eeprom_crc_p->length != 4))
    {
        BACKDOOR_MGR_Printf("%s(%d)Error. Last EEPROM TLV is not type TLV_CODE_CRC_32.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    /* Calculate the checksum
     */
    calc_crc = L_MATH_Crc32(0, (I8_T *)eeprom_p,
             sizeof(tlvinfo_header_t) + L_STDLIB_Ntoh16(hdr_p->totallen) - 4);

    stored_crc = *((UI32_T*)(eeprom_crc_p->value));
    stored_crc = L_STDLIB_Ntoh32(stored_crc);

    if (calc_crc != stored_crc)
    {
        BACKDOOR_MGR_Printf("%s(%d)Checksum error.(Calc=0x%08lX,Stored=0x%08lX).\r\n",
            __FUNCTION__, __LINE__, calc_crc, stored_crc);
        return FALSE;
    }
    DBGMSG("EEPROM TLV checksum valid.");

    return TRUE;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_StrCopyFromEepromStr
 * ------------------------------------------------------------------------
 * FUNCTION : This function copy the string in EEPROM data(which is not
 *            terminated by NULL char) to the specified string buffer(which
 *            is terminated by NULL char).
 * INPUT    : eeprom_str_p       -- pointer to ONIE EEPROM TLV data(non-null-terminated string)
 *            eeprom_str_len     -- length of string in eeprom_str_p
 *            output_str_buf_len -- length of buffer of the output string(including the size for null-terminated char)
 * OUTPUT   : output_str_p       -- output string(with null-terminated string)
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void FS_StrCopyFromEepromStr(const char* eeprom_str_p, UI32_T eeprom_str_len, UI32_T output_str_buf_len, char* output_str_p)
{
    UI32_T copy_str_len;

    /* sanity check */
    if ( (eeprom_str_p==NULL) || (output_str_p==NULL))
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid input argument.\r\n", __FUNCTION__, __LINE__);
        return;
    }

    /* the length in tlv does not count the terminated NULL char */
    if (eeprom_str_len > (output_str_buf_len-1))
    {
        /* the length of string in eeprom_str_p is larger than
         * output_str_buf_len-1, need to truncate
         * the string
         */
        copy_str_len=output_str_buf_len-1;
    }
    else
    {
        copy_str_len=eeprom_str_len;
    }

    memcpy(output_str_p, eeprom_str_p, copy_str_len);
    output_str_p[copy_str_len]='\0'; /* set null terminated char */
}
#endif /* end of #if (ONIE_EEPROM_HARD_CODE!=TRUE) && (SYS_CPNT_FS_HWINFO_FROM_ONLP==FALSE) */

#if (SYS_CPNT_FS_HWINFO_FROM_ONLP==TRUE)
/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetONIEEepromInfoFromONLPQuery
 * ------------------------------------------------------------------------
 * FUNCTION : This function will invoke the ONLP utility program onlp_query
 *            with the given option string in onlp_query_opt_str and write
 *            the output of onlp_query to query_output_buf.
 * INPUT    : onlp_query_opt_str -- option string to be passed to onlp_query
 * OUTPUT   : query_output_buf   -- output string after executing onlp_query
 * RETURN   : TRUE - Successfully, FALSE - Failed.
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static BOOL_T FS_GetONIEEepromInfoFromONLPQuery(const char* onlp_query_opt_str, char query_output_buf[ONLP_QUERY_OUTPUT_BUF_SIZE])
{
    char shell_cmd_base[28];
    char *real_shell_cmd_p=NULL;
    UI32_T output_buf_size;
    BOOL_T rc_b;
    int    rc_i;

    if (onlp_query_opt_str==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)onlp_query_opt_str is NULL.\r\n",
            __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (query_output_buf==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)query_output_buf is NULL.\r\n",
            __FUNCTION__, __LINE__);
        return FALSE;
    }

    rc_i=snprintf(shell_cmd_base, sizeof(shell_cmd_base), "%s %s",
        ONLP_QUERY_BASE_PATH, onlp_query_opt_str);
    if (rc_i >= sizeof(shell_cmd_base))
    {
        BACKDOOR_MGR_Printf("%s(%d)Illegal onlp_query_opt_str(%s)\r\n",
            __FUNCTION__, __LINE__, onlp_query_opt_str); \
        return FALSE;
    }

    real_shell_cmd_p=SYSFUN_GetRealAOSShellCmd(shell_cmd_base);
    if (real_shell_cmd_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)SYSFUN_GetRealAOSShellCmd returns error.\r\n",
            __FUNCTION__, __LINE__);
        return FALSE;
    }

    DEBUGSTR("Execute shell command:%s\r\n", real_shell_cmd_p);
    output_buf_size=ONLP_QUERY_OUTPUT_BUF_SIZE;
    rc_b=SYSFUN_GetExecuteCmdOutput(real_shell_cmd_p, &output_buf_size, query_output_buf);
    if ((rc_b==FALSE) || (output_buf_size==0))
    {
        BACKDOOR_MGR_Printf("%s(%d)Shell command '%s' returns error.\r\n",
            __FUNCTION__, __LINE__, real_shell_cmd_p);
        free(real_shell_cmd_p);
        return FALSE;
    }
    free(real_shell_cmd_p);
    real_shell_cmd_p=NULL;

    return TRUE;
}
#endif /* end of #if (SYS_CPNT_FS_HWINFO_FROM_ONLP==TRUE) */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_TranslateONIEEepromToHwInfo
 * ------------------------------------------------------------------------
 * FUNCTION : This function translates the ONIE EEPROM TLV data to
 *            FS_HW_Info_T format.
 * INPUT    : eeprom_p      -- ONIE EEPROM TLV data
 * OUTPUT   : hwinfo_p      -- hardware information to write
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : The data in eeprom_p must be validated before calling this
 *            function.
 * ------------------------------------------------------------------------
 */
static UI32_T FS_TranslateONIEEepromToHwInfo(UI8_T *eeprom_p, FS_HW_Info_T *hwinfo_p)
{
#if (SYS_CPNT_FS_HWINFO_FROM_ONLP==TRUE)
    char query_output_buf[ONLP_QUERY_OUTPUT_BUF_SIZE];
    FS_TYPE_TRANSLATE_HWINFO_FLAG_T verify_flag_bmp, translate_flag_bmp=0;
    verify_flag_bmp = FS_TYPE_TRANSLATE_HWINFO_FLAG_MAC_ADDR         |
                      FS_TYPE_TRANSLATE_HWINFO_FLAG_SERIAL_NO        |
                      FS_TYPE_TRANSLATE_HWINFO_FLAG_MANUFACTURE_DATE |
                      FS_TYPE_TRANSLATE_HWINFO_FLAG_PID              |
                      FS_TYPE_TRANSLATE_HWINFO_FLAG_BID              |
                      FS_TYPE_TRANSLATE_HWINFO_FLAG_HW_VER;

    memset(hwinfo_p, 0, sizeof(*hwinfo_p));

    if (FS_GetONIEEepromInfoFromONLPQuery("-en", query_output_buf)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Get product name error.\r\n",
            __FUNCTION__, __LINE__);
    }
    else
    {
        L_CHARSET_TrimTrailingNonPrintableChar(query_output_buf);
        if (FS_GetPIDAndBIDByProductName(query_output_buf,
            strlen(query_output_buf), hwinfo_p)==TRUE)
        {
            translate_flag_bmp |= FS_TYPE_TRANSLATE_HWINFO_FLAG_PID |
                                  FS_TYPE_TRANSLATE_HWINFO_FLAG_BID;
        }
        else
        {
            BACKDOOR_MGR_Printf("%s(%d)translate product name error.\r\n",
                __FUNCTION__, __LINE__);
        }
    }

    if (FS_GetONIEEepromInfoFromONLPQuery("-es", query_output_buf)==TRUE)
    {
        L_CHARSET_TrimTrailingNonPrintableChar(query_output_buf);
        strncpy((char*)hwinfo_p->serial_no, query_output_buf,
            sizeof(hwinfo_p->serial_no));
        translate_flag_bmp |= FS_TYPE_TRANSLATE_HWINFO_FLAG_SERIAL_NO;
    }
    else
    {
        BACKDOOR_MGR_Printf("%s(%d)Get serial number error.\r\n",
            __FUNCTION__, __LINE__);
    }

    if (FS_GetONIEEepromInfoFromONLPQuery("-em", query_output_buf)==TRUE)
    {
        memcpy(hwinfo_p->mac_addr, query_output_buf, SYS_ADPT_MAC_ADDR_LEN);
        translate_flag_bmp |= FS_TYPE_TRANSLATE_HWINFO_FLAG_MAC_ADDR;
    }
    else
    {
        BACKDOOR_MGR_Printf("%s(%d)Get mac base error.\r\n",
            __FUNCTION__, __LINE__);
    }

    if (FS_GetONIEEepromInfoFromONLPQuery("-ea", query_output_buf)==TRUE)
    {
        L_CHARSET_TrimTrailingNonPrintableChar(query_output_buf);
        strncpy((char*)hwinfo_p->manufacture_date, query_output_buf, sizeof(hwinfo_p->manufacture_date));
        translate_flag_bmp |= FS_TYPE_TRANSLATE_HWINFO_FLAG_MANUFACTURE_DATE;
    }
    else
    {
        BACKDOOR_MGR_Printf("%s(%d)Get manufacture date error.\r\n",
            __FUNCTION__, __LINE__);
    }

    if (FS_GetONIEEepromInfoFromONLPQuery("-el", query_output_buf)==TRUE)
    {
        L_CHARSET_TrimTrailingNonPrintableChar(query_output_buf);
        strncpy((char*)(hwinfo_p->agent_hw_ver), query_output_buf,
            sizeof(hwinfo_p->agent_hw_ver));
        translate_flag_bmp |= FS_TYPE_TRANSLATE_HWINFO_FLAG_HW_VER;
    }
    else
    {
        BACKDOOR_MGR_Printf("%s(%d)Get label revision error.\r\n",
            __FUNCTION__, __LINE__);
    }

    if (verify_flag_bmp!=translate_flag_bmp)
    {
        BACKDOOR_MGR_Printf("%s(%d)translate failed flag bmp=0x%08lX\r\n", __FUNCTION__,
            __LINE__, (unsigned long)(verify_flag_bmp ^ translate_flag_bmp));
        return FS_RETURN_ERROR;
    }

    return FS_RETURN_OK;

#elif (ONIE_EEPROM_HARD_CODE==TRUE)
    /* temp solution
     */
	memset(hwinfo_p, 0, sizeof(FS_HW_Info_T));

    if (FS_GetPIDAndBIDByProductName(ONIE_EEPROM_HARD_CODE_PRODUCT_NAME,
        sizeof(ONIE_EEPROM_HARD_CODE_PRODUCT_NAME), hwinfo_p)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)translate product name error.\r\n",
            __FUNCTION__, __LINE__);
        return FS_RETURN_ERROR;
    }

    strncpy((char*)hwinfo_p->serial_no, ONIE_EEPROM_HARD_CODE_SERIAL_NUMBER,
        sizeof(hwinfo_p->serial_no));

    {
        char tmp_mac_addr[]={ONIE_EEPROM_HARD_CODE_MAC_BASE};
        memcpy(hwinfo_p->mac_addr, tmp_mac_addr, SYS_ADPT_MAC_ADDR_LEN);
    }

    strncpy((char*)hwinfo_p->manufacture_date, ONIE_EEPROM_HARD_CODE_MANUF_DATE, sizeof(hwinfo_p->manufacture_date));
    
    strncpy((char*)hwinfo_p->agent_hw_ver, ONIE_EEPROM_HARD_CODE_LABEL_REVISION, sizeof(hwinfo_p->agent_hw_ver));
    return FS_RETURN_OK;
#else /* #if (ONIE_EEPROM_HARD_CODE==TRUE) */
    tlvinfo_header_t *hdr_p = (tlvinfo_header_t*)eeprom_p;
    tlvinfo_tlv_t    *curr_tlv_p = NULL;
    char* str_p=NULL;
    FS_TYPE_TRANSLATE_HWINFO_FLAG_T verify_flag_bmp, translate_flag_bmp=0;
    UI16_T curr_tlv_offset, tlv_end_offset;

	memset(hwinfo_p, 0, sizeof(FS_HW_Info_T));
    verify_flag_bmp = FS_TYPE_TRANSLATE_HWINFO_FLAG_MAC_ADDR         |
                      FS_TYPE_TRANSLATE_HWINFO_FLAG_SERIAL_NO        |
                      FS_TYPE_TRANSLATE_HWINFO_FLAG_MANUFACTURE_DATE |
                      FS_TYPE_TRANSLATE_HWINFO_FLAG_PID              |
                      FS_TYPE_TRANSLATE_HWINFO_FLAG_BID              |
                      FS_TYPE_TRANSLATE_HWINFO_FLAG_HW_VER;

    curr_tlv_offset = sizeof(tlvinfo_header_t);
    tlv_end_offset = sizeof(tlvinfo_header_t) + L_STDLIB_Ntoh16(hdr_p->totallen);

    while (curr_tlv_offset < tlv_end_offset)
    {
        curr_tlv_p = (tlvinfo_tlv_t *) (eeprom_p + curr_tlv_offset);
        str_p = (char*)(curr_tlv_p->value);
        switch (curr_tlv_p->type)
        {
            case TLV_CODE_PRODUCT_NAME:
                if (FS_GetPIDAndBIDByProductName(str_p, curr_tlv_p->length, hwinfo_p)==TRUE)
                {
                    translate_flag_bmp |= FS_TYPE_TRANSLATE_HWINFO_FLAG_PID |
                                          FS_TYPE_TRANSLATE_HWINFO_FLAG_BID;
                }
                break;
            case TLV_CODE_SERIAL_NUMBER:
                FS_StrCopyFromEepromStr(str_p, curr_tlv_p->length,
                    sizeof(hwinfo_p->serial_no), (char*)(hwinfo_p->serial_no));
                translate_flag_bmp |= FS_TYPE_TRANSLATE_HWINFO_FLAG_SERIAL_NO;
                break;
            case TLV_CODE_MAC_BASE:
            {
                memcpy(hwinfo_p->mac_addr, str_p, SYS_ADPT_MAC_ADDR_LEN);
#if 0
                /* mac addr in EEPROM is mac base + 1
                 * so need to minus by 1 here
                 * the first 3 bytes of mac addr is OUI and should not be
                 * changed after minus by 1.
                 */
                if (L_MATH_MacCalculation(5, 3, -1, hwinfo_p->mac_addr)==FALSE)
                {
                    BACKDOOR_MGR_Printf("ERROR: OUI of mac addr is changed after calcuation.\r\n");
                    BACKDOOR_MGR_Printf("EEPROM TLV Mac Addr='%s'\r\n", str_p);
                    break;
                }
#endif
                DEBUGSTR("%s(%d)HWINFO MAC_ADDR-%02hX:%02hX:%02hX:%02hX:%02hX:%02hX\r\n",
                    __FUNCTION__, __LINE__, hwinfo_p->mac_addr[0], hwinfo_p->mac_addr[1],
                    hwinfo_p->mac_addr[2], hwinfo_p->mac_addr[3], hwinfo_p->mac_addr[4],
                    hwinfo_p->mac_addr[5]);

                translate_flag_bmp |= FS_TYPE_TRANSLATE_HWINFO_FLAG_MAC_ADDR;
            }
                break;
            case TLV_CODE_MANUF_DATE:
            {
                char date_str[SYS_ADPT_MANUFACTURE_DATE_LEN+1]; /* add extra one byte for '\0' at end of the string */

                FS_StrCopyFromEepromStr(str_p, curr_tlv_p->length,
                    sizeof(date_str), date_str);

                memcpy(hwinfo_p->manufacture_date, date_str, sizeof(hwinfo_p->manufacture_date));

                translate_flag_bmp |= FS_TYPE_TRANSLATE_HWINFO_FLAG_MANUFACTURE_DATE;
            }
                break;
            
            case TLV_CODE_LABEL_REVISION:
                FS_StrCopyFromEepromStr(str_p, curr_tlv_p->length,
                    sizeof(hwinfo_p->agent_hw_ver), (char*)(hwinfo_p->agent_hw_ver));

                translate_flag_bmp |= FS_TYPE_TRANSLATE_HWINFO_FLAG_HW_VER;
                break;
            default:
                break;
        } /* end of switch (curr_tlv_p->type) */

        curr_tlv_offset += sizeof(tlvinfo_tlv_t) + curr_tlv_p->length;
    } /* end of while (curr_tlv_offset < tlv_end_offset) */

    if (verify_flag_bmp!=translate_flag_bmp)
    {
        BACKDOOR_MGR_Printf("%s(%d)translate failed flag bmp=0x%08lX\r\n", __FUNCTION__,
            __LINE__, (UI32_T)(verify_flag_bmp ^ translate_flag_bmp));
        return FS_RETURN_ERROR;
    }

    return FS_RETURN_OK;
#endif /* #if (ONIE_EEPROM_HARD_CODE==TRUE) */
} /* end of static UI32_T FS_TranslateONIEEepromToHwInfo(UI8_T *eeprom_p, FS_HW_Info_T *hwinfo_p) */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetHwinfoFromONIEEeprom
 * ------------------------------------------------------------------------
 * FUNCTION : This function reads the ONIE EEPROM data from the mtd block device
 *            "mtdblock"FS_MTD_PART_HW_INFO and translates the data into
 *            FS_HW_Info_T format.
 * INPUT    : None
 * OUTPUT   : hwinfo        -- hardware information to write
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static UI32_T FS_GetHwinfoFromONIEEeprom(FS_HW_Info_T *hwinfo_p)
{
    UI32_T rc, read_len;
    UI32_T ret_val=FS_RETURN_OK;
    UI8_T *eeprom_buf_p=NULL;
    UI32_T hw_info_mtd_id;

    if (hwinfo_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)hwinfo_p is NULL.\r\n", __FUNCTION__, __LINE__);
        return FS_RETURN_BAD_PARA;
    }

    /* malloc for eeprom_buf_p and init eeprom_hdr_p
     */
    eeprom_buf_p = malloc(TLV_INFO_MAX_LEN);
    if (eeprom_buf_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)malloc failed for %d bytes.\r\n", __FUNCTION__, __LINE__,
            TLV_INFO_MAX_LEN);
        return FS_RETURN_NO_BUFFER;
    }
    else
    {
        memset(eeprom_buf_p, 0, TLV_INFO_MAX_LEN);
    }

#if (ONIE_EEPROM_HARD_CODE!=TRUE) && (SYS_CPNT_FS_HWINFO_FROM_ONLP==FALSE)
    /* read ONIE EEPROM data
     */
    if(FS_GetMtdDevIdByMtdType(FS_MTD_PART_HW_INFO, &hw_info_mtd_id)==TRUE)
    {
        rc=FS_READ_HWINFO_FROM_FLASH(eeprom_buf_p, TLV_INFO_MAX_LEN, &read_len, hw_info_mtd_id);
    }
    else
    {
        rc=FS_RETURN_BAD_HW;
    }

    if (rc!=FS_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to read EEPROM data from the flash.\r\n",
            __FUNCTION__, __LINE__);
        ret_val=rc;
        goto exit;
    }

    /* validate ONIE EEPROM data
     */
    if (FS_IsValidONIEEeprom(eeprom_buf_p)==FALSE)
    {
        DBGMSG("Invalid EEPROM data.");
        ret_val=FS_RETURN_ERROR;
        goto exit;
    }
#endif /* end of #if (ONIE_EEPROM_HARD_CODE!=TRUE) && (SYS_CPNT_FS_HWINFO_FROM_ONLP==FALSE) */

    /* translate the ONIE EEPROM data into FS_HW_Info_T
     */
    rc = FS_TranslateONIEEepromToHwInfo(eeprom_buf_p, hwinfo_p);
    if (rc!=FS_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("%s(%d)Translate ONIE EEPROM to Hardware Information failed.\r\n",
            __FUNCTION__, __LINE__);
        ret_val=rc;
        goto exit;
    }

exit:
    if (eeprom_buf_p)
        free(eeprom_buf_p);

    return ret_val;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_WriteHardwareInfo
 * ------------------------------------------------------------------------
 * FUNCTION : This function will write hardware information to flash
 * INPUT    : hwinfo        -- hardware information to write
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
UI32_T  FS_WriteHardwareInfo(FS_HW_Info_T *hwinfo)
{
    BACKDOOR_MGR_Printf("%s(%d)Not support to write hardware info yet.\r\n", __FUNCTION__, __LINE__);
    return FS_RETURN_ERROR;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_ReadHardwareInfo
 * ------------------------------------------------------------------------
 * FUNCTION : This function will read hardware information from flash
 * INPUT    : drive         -- unit number or blade number
 *            hwinfo        -- hardware information read from the file system
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T  FS_ReadHardwareInfo(UI32_T drive, FS_HW_Info_T *hwinfo)
{
    if (hwinfo == NULL)
    {
        ERRORSTR("%s:ERROR, bad parameter hwinfo is NULL.\r\n",__FUNCTION__);
        return FS_RETURN_BAD_PARA;
    }

    /* Local operation */
#if (SYS_CPNT_STACKING == TRUE)
    if (    (drive == DUMMY_DRIVE)
         || (drive == SYS_VAL_LOCAL_UNIT_ID)
         || (drive == FS_OM_GetControlDriverId())
       )
#endif
    {
        I32_T       ret = FS_RETURN_ERROR;

#ifndef INCLUDE_DIAG
        FS_SETBUSY();
#endif

        ret = FS_GetHwinfoFromONIEEeprom(hwinfo);

        DEBUGSTR(" %s read flash\r\n", __FUNCTION__);

#ifndef INCLUDE_DIAG
        FS_SETNOTBUSY();
#endif

        DEBUGSTR("%s return\r\n", __FUNCTION__);
        return ret;
    }
#if (SYS_CPNT_STACKING == TRUE)
    else /* Remote operation */
    {
        FS_Packet_Header_T  *fs_rq_packet_p;
        FS_HW_Info_Packet_T *hw_info_packet;
        FS_NAK_Packet_T     *nak_packet;
        UI32_T              retry_count = 0;
        UI32_T              return_value;
        L_MM_Mref_Handle_T* mref_handle_p;
        UI32_T              pdu_len, capability;
        UI16_T              opcode;

        /* Make sure FS is in master mode */
        if(FS_OM_GetControlStackingMode() != SYS_TYPE_STACKING_MASTER_MODE)
        {
            return_value = FS_RETURN_NOT_MASTER;

            ERRMSG(return_value);
            return return_value;
        }

        FS_GetRemoteServiceCapability(drive, &capability);
        if (capability > FS_GET_HARDWARE_INFO)
        {
            /* Can use the new service opcode to get the H/W info. */
            opcode = FS_GET_HARDWARE_INFO;
        }
        else
        {
            /* Must use the old service opcode to get the H/W info. */
            opcode = FS_GET_SERVICE_CAPABILITY;
        }

        /* Send request packet */
        FS_LOCK_REMOTE();
        for(;;)
        {
            /* fill the content of the read request packet */
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Packet_Header_T), /* tx_buffer_size */
                                                  L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_SEND, opcode) /* user_id */
                                                 );
            fs_rq_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
            if (NULL == fs_rq_packet_p)
            {
                DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
                return_value = FS_RETURN_ERROR;
                break;
            }

            fs_rq_packet_p->opcode    = opcode;
            fs_rq_packet_p->seq_no    = FS_OM_GetControlSeqNo();
            fs_rq_packet_p->data_size = 0;

            /* re-assign seq no here when retry ?? kh_shi */

            memset(&fs_packet_buffer, 0, sizeof(fs_packet_buffer) );
            fs_packet_buffer.header.opcode = FS_ACK;/*init as success for ISC new design*/
            if (ISC_RemoteCall((UI8_T)drive, ISC_FS_SID, mref_handle_p,
                                SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                sizeof(fs_packet_buffer),(UI8_T *) &fs_packet_buffer,
                                FS_ISC_TRY_COUNT, FS_TIMEOUT_VALUE) )
            {   /* ISC ok */
                FS_OM_AddControlSeqNo();
                /* ACK received */
                if(fs_packet_buffer.header.opcode == FS_ACK)
                {
                    hw_info_packet = (FS_HW_Info_Packet_T *) &fs_packet_buffer;
                    memcpy(hwinfo, &hw_info_packet->hw_info, sizeof(FS_HW_Info_T) );
                    return_value = FS_RETURN_OK;
                    break;
                }
                else
                {
                    nak_packet = (FS_NAK_Packet_T *) &fs_packet_buffer;
                    ERRMSG(nak_packet->error_code);
                    FS_AbortSequence(drive);
                    if(retry_count++ >= FS_COMMAND_RETRY_COUNT)
                    {
                        return_value = FS_AbortSequence(drive);
                        ERRMSG(drive);
                        break;
                    }
                    else
                    {
                        DBGMSG("Try Again");
                        SYSFUN_Sleep(FS_SLEEP_TIME);
                    }
                }

            }
            else
            {
                return_value = FS_AbortSequence(drive);
                ERRMSG(return_value);
                break;
            }
        }
        FS_UNLOCK_REMOTE();
        FS_ReturnIdle();
        return return_value;
    }
#endif /* end of #if (SYS_CPNT_STACKING == TRUE) */

} /* end of UI32_T  FS_ReadHardwareInfo(UI32_T drive, FS_HW_Info_T *hwinfo) */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetOpCodeDirectory
 * ------------------------------------------------------------------------
 * FUNCTION : This function returns the directory where the op code files
 *            are put in.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
const char* FS_GetOpCodeDirectory(void)
{
    return op_code_directory;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetRequiredTmpFilenameBufSize
 * ------------------------------------------------------------------------
 * FUNCTION : This function calcuate the required buffer size when
 *            calling the function FS_CreateTmpFile().
 * INPUT    : prefix_tmp_filename_p --
 *                The name to be prefixed to the temporarily file name. Suggests
 *                to use CSC name or "CSC name"+"operation name".
 * OUTPUT   : required_buf_sz_p --
 *                The required buffer size when calling the function
 *                FS_CreateTmpFile().
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : The output value of required_buf_sz_p incudes the terminating
 *            null char.
 * ------------------------------------------------------------------------
 */
UI32_T FS_GetRequiredTmpFilenameBufSize(const char* prefix_tmp_filename_p, UI32_T *required_buf_sz_p)
{
    if ((prefix_tmp_filename_p==NULL) || (required_buf_sz_p==NULL))
    {
        return FS_RETURN_BAD_PARA;
    }

    *required_buf_sz_p=strlen(prefix_tmp_filename_p) +
        sizeof(FS_TMP_FOLDER) +
        6 /* template string for mkstemp() */ +
        1 /* for terminating null char */;
    return FS_RETURN_OK;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_CreateTmpFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function creates a temporarily file and write the given
 *            data to the file. The full path to the temporarily file will be
 *            output.
 * INPUT    : prefix_tmp_filename_p --
 *                The name to be prefixed to the temporarily file name. Suggests
 *                to use CSC name or "CSC name"+"operation name".
 *            file_data_p --
 *                The pointer to the data to be written to the new created
 *                temporarily file.
 *            file_data_size --
 *                The size of the data to be written to the new created
 *                temporarily file.
 *            tmpfilename_size --
 *                The size of the output buffer for full path to temporarily
 *                file. The size includes the terminating null char.
 * OUTPUT   : tmpfilename_p    --
 *                The full path to the new created temporarily file.
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : 1. The permission of the new created temporarily file is set
 *               as "rwx------".
 *            2. The required buffer size of tmpfilename_p can be calcuated
 *               by FS_GetRequiredTmpFilenameBufSize().
 *            3. The temporarily file is only created when return value is
 *               FS_RETURN_OK
 *            4. The caller shall remove the temporaily file when it will not
 *               be used any more by calling FS_RemoveTmpFile().
 * ------------------------------------------------------------------------
 */
UI32_T FS_CreateTmpFile(const char* prefix_tmp_filename_p, const UI8_T *file_data_p,
    UI32_T file_data_size, UI32_T tmpfilename_size, char* tmpfilename_p)
{
    int fd,rc;

    rc=snprintf(tmpfilename_p, tmpfilename_size, "%s%sXXXXXX",FS_TMP_FOLDER,
        prefix_tmp_filename_p);

    if (rc>=(tmpfilename_size))
    {
        DBGMSG_EX("%s:%d,tmpfilename_size(=%lu) too small.(required=%d)\r\n",
            __FUNCTION__, __LINE__, (unsigned long)tmpfilename_size, rc+1);
        return FS_RETURN_NO_BUFFER;
    }

    fd=mkstemp(tmpfilename_p);
    if (fd==-1)
    {
        DBGMSG_EX("%s:%d,mkstemp error(errno=%d)\r\n", __FUNCTION__, __LINE__, errno);
        return FS_RETURN_OTHERS;
    }

    DBGMSG_EX("%s:%d, temp file:'%s' is created.\r\n", __FUNCTION__, __LINE__, tmpfilename_p);

    rc=write(fd, file_data_p, file_data_size);
    if(rc==-1)
    {
        DBGMSG_EX("%s:%d, write error(errno=%d)\r\n",  __FUNCTION__, __LINE__, errno);
        close(fd);
        unlink(tmpfilename_p);
        return FS_RETURN_ERROR;
    }

    close(fd);
    /* set the permission of the tmpfile as "rwx------"
     */
    chmod(tmpfilename_p, 0700);
    return FS_RETURN_OK;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_RemoveTmpFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function removes the temporarily file created by the function
 *            FS_CreateTmpFile().
 * INPUT    : tmpfilename_p    --
 *                The full path to the temporarily file.
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : This function can only used to remove the file created by
 *            the function FS_CreateTmpFile
 * ------------------------------------------------------------------------
 */
UI32_T FS_RemoveTmpFile(char* tmpfilename_p)
{
    int rc;

    if (tmpfilename_p==NULL)
    {
        DBGMSG_EX("%s,%d:tmpfilename_p is NULL.\r\n", __FUNCTION__,__LINE__);
        return FS_RETURN_BAD_PARA;
    }

    if (strncmp(tmpfilename_p, FS_TMP_FOLDER, sizeof(FS_TMP_FOLDER)-1))
    {
        DBGMSG_EX("%s,%d:tmpfilename_p(%s) is illegal.\r\n", __FUNCTION__,__LINE__, tmpfilename_p);
        return FS_RETURN_OTHERS;
    }

    rc=unlink(tmpfilename_p);
    if (rc==-1)
    {
        DBGMSG_EX("%s,%d:unlink error(errno=%d)\r\n", __FUNCTION__,__LINE__, errno);
        return FS_RETURN_ERROR;
    }

    return FS_RETURN_OK;
}

#else /* #if (SYS_CPNT_ONIE_SUPPORT==TRUE) */
/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_WriteHardwareInfo
 * ------------------------------------------------------------------------
 * FUNCTION : This function will write hardware information to flash
 * INPUT    : hwinfo        -- hardware information to write
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
UI32_T  FS_WriteHardwareInfo(FS_HW_Info_T *hwinfo)
{
    UI32_T  result = FS_RETURN_ERROR;
#if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE)
    FS_PARTITION_TABLE_T *flash_entry_p = NULL;
#else
    UI32_T hw_info_mtd_id;
#endif

#ifndef INCLUDE_DIAG
    FS_SETBUSY();
#endif
#if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE)
    while (NULL != (flash_entry_p = FS_GetNextFlashPartitionEntry(flash_entry_p)))
    {
       /* write on the first found partition
        */
       if( flash_entry_p->type == TYPE_HWINFO)
       {
           result=FS_WRITE_HWINFO_TO_FLASH((UI8_T*)hwinfo, sizeof(FS_HW_Info_T), flash_entry_p->mtd_num, TRUE);
           break;
       }

    }
    if (flash_entry_p==NULL)
    {
        ERRORSTR("%s:ERROR, cannot find TYPE_HWINFO in flash partition entry\r\n",__FUNCTION__);
        FS_SETNOTBUSY();
        return result;
    }
#else
    if(FS_GetMtdDevIdByMtdType(FS_MTD_PART_HW_INFO, &hw_info_mtd_id)==TRUE)
    {
        result=FS_WRITE_HWINFO_TO_FLASH((UI8_T*)hwinfo, sizeof(FS_HW_Info_T), hw_info_mtd_id, TRUE);
    }
    else
    {
        result=FS_RETURN_BAD_HW;
    }
#endif /*End #if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==TRUE)*/

#ifndef INCLUDE_DIAG
    FS_SETNOTBUSY();
#endif
    return result;
} /* End of FS_WriteHardwareInfo() */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_ReadHardwareInfo
 * ------------------------------------------------------------------------
 * FUNCTION : This function will read hardware information from flash
 * INPUT    : drive         -- unit number or blade number
 *            hwinfo        -- hardware information read from the file system
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T  FS_ReadHardwareInfo(UI32_T drive, FS_HW_Info_T *hwinfo)
{
#if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE)
    FS_PARTITION_TABLE_T *flash_entry_p = NULL;
#else
    UI32_T hw_info_mtd_id;
#endif

    if (hwinfo == NULL)
    {
        ERRORSTR("%s:ERROR, bad parameter hwinfo is NULL.\r\n",__FUNCTION__);
        return FS_RETURN_BAD_PARA;
    }

    /* Local operation */
#if (SYS_CPNT_STACKING == TRUE)
    if (    (drive == DUMMY_DRIVE)
         || (drive == SYS_VAL_LOCAL_UNIT_ID)
         || (drive == FS_OM_GetControlDriverId())
       )
#endif
    {
        UI32_T      count;
        I32_T       ret = FS_RETURN_ERROR;

#ifndef INCLUDE_DIAG
        FS_SETBUSY();
#endif

#if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE)
        while (NULL != (flash_entry_p = FS_GetNextFlashPartitionEntry(flash_entry_p)))
        {
           /* write on the first found partition
            */
           if( flash_entry_p->type == TYPE_HWINFO)
           {
               ret=FS_READ_HWINFO_FROM_FLASH((UI8_T*)hwinfo, sizeof(FS_HW_Info_T), &count, flash_entry_p->mtd_num);
               break;
           }

        }
        if(flash_entry_p==NULL)
        {
            ERRORSTR("%s:ERROR, cannot find TYPE_HWINFO in flash partition entry\r\n",__FUNCTION__);
            FS_SETNOTBUSY();
            return ret;
        }
#else
        if(FS_GetMtdDevIdByMtdType(FS_MTD_PART_HW_INFO, &hw_info_mtd_id)==TRUE)
        {
            ret=FS_READ_HWINFO_FROM_FLASH((UI8_T*)hwinfo, sizeof(FS_HW_Info_T), &count, hw_info_mtd_id);
        }
        else
        {
            ret=FS_RETURN_BAD_HW;
        }
#endif  /*End #if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE)*/

        DEBUGSTR("%s read flash\r\n", __FUNCTION__);

        if( count < sizeof(FS_HW_Info_T) )
        {
            DEBUGSTR("%s::warning, read size %ld < %d!!\r\n", __FUNCTION__, (long)count, sizeof(FS_HW_Info_T));
            if( ret == FS_RETURN_OK)
                ret = FS_RETURN_FILE_TRUNCATED;
        }

#ifndef INCLUDE_DIAG
        FS_SETNOTBUSY();
#endif

        DEBUGSTR("%s return\r\n", __FUNCTION__);
        return ret;
    }
#if (SYS_CPNT_STACKING == TRUE)
    else /* Remote operation */
    {
        FS_Packet_Header_T  *fs_rq_packet_p;
        FS_HW_Info_Packet_T *hw_info_packet;
        FS_NAK_Packet_T     *nak_packet;
        UI32_T              retry_count = 0;
        UI32_T              return_value;
        L_MM_Mref_Handle_T* mref_handle_p;
        UI32_T              pdu_len, capability;
        UI16_T              opcode;

        /* Make sure FS is in master mode */
        if(FS_OM_GetControlStackingMode() != SYS_TYPE_STACKING_MASTER_MODE)
        {
            return_value = FS_RETURN_NOT_MASTER;

            ERRMSG(return_value);
            return return_value;
        }

        FS_GetRemoteServiceCapability(drive, &capability);
        if (capability > FS_GET_HARDWARE_INFO)
        {
            /* Can use the new service opcode to get the H/W info. */
            opcode = FS_GET_HARDWARE_INFO;
        }
        else
        {
            /* Must use the old service opcode to get the H/W info. */
            opcode = FS_GET_SERVICE_CAPABILITY;
        }

        /* Send request packet */
        FS_LOCK_REMOTE();
        for(;;)
        {
            /* fill the content of the read request packet */
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Packet_Header_T), /* tx_buffer_size */
                                                  L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_SEND, opcode) /* user_id */
                                                 );
            fs_rq_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
            if (NULL == fs_rq_packet_p)
            {
                DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
                return_value = FS_RETURN_ERROR;
                break;
            }

            fs_rq_packet_p->opcode    = opcode;
            fs_rq_packet_p->seq_no    = FS_OM_GetControlSeqNo();
            fs_rq_packet_p->data_size = 0;

            /* re-assign seq no here when retry ?? kh_shi */

            memset(&fs_packet_buffer, 0, sizeof(fs_packet_buffer) );
            fs_packet_buffer.header.opcode = FS_ACK;/*init as success for ISC new design*/
            if (ISC_RemoteCall((UI8_T)drive, ISC_FS_SID, mref_handle_p,
                                SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                sizeof(fs_packet_buffer),(UI8_T *) &fs_packet_buffer,
                                FS_ISC_TRY_COUNT, FS_TIMEOUT_VALUE) )
            {   /* ISC ok */
                FS_OM_AddControlSeqNo();
                /* ACK received */
                if(fs_packet_buffer.header.opcode == FS_ACK)
                {
                    hw_info_packet = (FS_HW_Info_Packet_T *) &fs_packet_buffer;
                    memcpy(hwinfo, &hw_info_packet->hw_info, sizeof(FS_HW_Info_T) );
                    return_value = FS_RETURN_OK;
                    break;
                }
                else
                {
                    nak_packet = (FS_NAK_Packet_T *) &fs_packet_buffer;
                    ERRMSG(nak_packet->error_code);
                    FS_AbortSequence(drive);
                    if(retry_count++ >= FS_COMMAND_RETRY_COUNT)
                    {
                        return_value = FS_AbortSequence(drive);
                        ERRMSG(drive);
                        break;
                    }
                    else
                    {
                        DBGMSG("Try Again");
                        SYSFUN_Sleep(FS_SLEEP_TIME);
                    }
                }

            }
            else
            {
                return_value = FS_AbortSequence(drive);
                ERRMSG(return_value);
                break;
            }
        }
        FS_UNLOCK_REMOTE();
        FS_ReturnIdle();
        return return_value;
    }
#endif /* end of #if (SYS_CPNT_STACKING == TRUE) */
}
#endif /* end of #if (SYS_CPNT_ONIE_SUPPORT==TRUE) */

#if ((SYS_HWCFG_FLASH_TYPE & SYS_HWCFG_FLASH_TYPE_NAND)!=0)
#if 0 /* these definition should be included from standard include path of the file <mtd/mtd-user.h> */
struct mtd_info_user {
    UI8_T  type;
    UI32_T flags;
    UI32_T size;    /* Total size of the MTD */
    UI32_T erasesize;
    UI32_T writesize;
    UI32_T oobsize; /* Amount of OOB data per block (e.g. 16) */
    UI64_T padding; /* Old obsolete field; do not use */
};
#define MEMGETINFO      _IOR('M', 1, struct mtd_info_user)
#endif /* end of #if 0 */
struct mtd_info_user mtd_info;

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_WriteNandFlash
 * ------------------------------------------------------------------------
 * FUNCTION : This function handles write data to nand flash
 * INPUT    : buf       -- store data need to write
 *            length    -- the length need to write
 *            mtd       -- mtd device number
 *            is_ro_mtd -- mtd dev is readonly or not

 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : This func use "nandwrite" to write data to nand flash.
 *            The advantage of nandwrite is that can deal with bad block
 *            and mark bad block when writing.
 * ------------------------------------------------------------------------
 */
static UI32_T FS_WriteNandFlash(UI8_T *buf, UI32_T length, UI32_T mtd, BOOL_T is_ro_mtd)
{
    UI32_T  ret_val;
    UI32_T  ret_length, block_count;
    I32_T   fd_io;
    int     policy;
    char    path[FS_MAX_PATHNAME_BUF_SIZE];
    char    mtd_name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+1];
    char    exec_cmd[FS_MAX_PATHNAME_BUF_SIZE];
    FILE    *fd;

    ret_val=FS_RETURN_OK;
    snprintf(mtd_name, SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+1, "%s%lu", FS_FLASH_MTD_PREFIX, (unsigned long)mtd);

    snprintf(path, FS_MAX_PATHNAME_BUF_SIZE, "%s%s", FS_TMP_FOLDER, FS_TMP_WRITE_FILE_NAME);
    fd=fopen(path, "w");
    if (fd==NULL)
    {
        ERRORSTR("%s:%d,ERROR, can't fopen file %s\r\n", __FUNCTION__, __LINE__, path);
        return FS_RETURN_FILE_NEX;
    }
    ret_length=fwrite((char*)buf,1, length,fd);
    if (ret_length<length)
    {
        ERRORSTR("%s:%d,:ERROR, write size %ld < file_size %ld\r\n",__FUNCTION__, __LINE__, (long)ret_length, (long)length);
        fclose(fd);
        unlink(path);
        return FS_RETURN_NO_BUFFER;
    }
    fclose(fd);

    /* need to make device writable if it is readonly device
     */
    fd_io=open(mtd_name, O_RDONLY, 0666);
    if (fd_io<0)
    {
        ERRORSTR("%s:ERROR!! Can not open %s!!\r\n", __FUNCTION__, mtd_name);
        perror("open");
        unlink(path);
        return FS_RETURN_ERROR;
    }
    if (ioctl(fd_io, MEMGETINFO, &mtd_info))
    {
        ERRORSTR("%s:ERROR!! Can not MEMGETINFO %s!!\r\n", __FUNCTION__, mtd_name);
        close(fd_io);
        unlink(path);
        return FS_RETURN_ERROR;
    }
    block_count=mtd_info.size/mtd_info.erasesize;
    DEBUGSTR("mtd_info.size=%ld, mtd_info.erasesize=%ld, block_count=%ld\n", (long)block_count, (long)mtd_info.size, (long)mtd_info.erasesize);
    close(fd_io);

    if (is_ro_mtd==TRUE)
    {
        snprintf(mtd_name, SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+1, "%s%lu", FS_FLASH_MTD_PREFIX, (unsigned long)mtd);
        fd_io=open(mtd_name, O_RDONLY, 0666);
        if (fd_io<0)
        {
            ERRORSTR("%s:ERROR!! Can not open %s!!\r\n", __FUNCTION__, mtd_name);
            perror("open");
            fflush(stdout);
            unlink(path);
            return FS_RETURN_ERROR;
        }
        policy=0;
        if (ioctl(fd_io, BLKROSET, &policy))
        {
            ERRORSTR("%s:ERROR(%d)!! Failed to change readonly attribute on %s.\r\n", __FUNCTION__, errno, mtd_name);
            perror("ioctl");
            close(fd_io);
            unlink(path);
            return FS_RETURN_ERROR;
        }
        close(fd_io);
    }
    snprintf(mtd_name, SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+1, "%s%lu", FS_FLASH_MTD_PREFIX, (unsigned long)mtd);
    snprintf(exec_cmd, FS_MAX_PATHNAME_BUF_SIZE, "%s %s%s%ld%s", FS_FLASH_ERASE_CMD, mtd_name, " 0 ", (long)block_count," >/dev/null 2>&1");
    DEBUGSTR("exec_cmd=%s\n", exec_cmd);
    system(exec_cmd);
    snprintf(exec_cmd, FS_MAX_PATHNAME_BUF_SIZE,"%s %s%s%s%s", FS_NANDWRITE_CMD, mtd_name, " -pm ", path, " >/dev/null 2>&1");
    DEBUGSTR("exec_cmd=%s\n", exec_cmd);
    system(exec_cmd);
    if (unlink(path)<0)
    {
        ERRORSTR("%s:%d,ERROR!! can not delete file (%s)!!.\r\n", __FUNCTION__, __LINE__, path);
    }
    sync();

    /* need to set device back to readonly if it is readonly device
     */
    if (is_ro_mtd==TRUE)
    {
        /* Need to change the readonly attribute after the loader programming
         * done. From the current test, it can be done after close(fd) is
         * called, no delay is required.
         */
        snprintf(mtd_name, SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+1, "%s%lu", FS_FLASH_MTD_PREFIX, (unsigned long)mtd);
        fd_io = open(mtd_name, 0, 0666);
        if (fd_io==-1)
        {
            ERRORSTR("%s:ERROR(%d,%d)!! Failed to open %s.\r\n", __FUNCTION__, 0, errno, mtd_name);
            return FS_RETURN_ERROR;

        }
        policy=1;
        if (ioctl(fd_io, BLKROSET, &policy))
        {
            ERRORSTR("%s:ERROR(%d)!! Failed to restore readonly attribute on %s.\r\n", __FUNCTION__, errno, mtd_name);
            close(fd_io);
            return FS_RETURN_ERROR;

        }
        close(fd_io);
    }

    return FS_RETURN_OK;
}
#endif /* End #if ((SYS_HWCFG_FLASH_TYPE & SYS_HWCFG_FLASH_TYPE_NAND)!=0) */

#if ((SYS_HWCFG_FLASH_TYPE & SYS_HWCFG_FLASH_TYPE_NOR)!=0)
static UI32_T FS_WriteNorFlash(UI8_T *buf, UI32_T length, UI32_T mtd, BOOL_T is_ro_mtd)
{
    I32_T   fd;
    char    name[FS_MAX_PATHNAME_BUF_SIZE];
    UI32_T  count, ret_val;
    int     rc,policy;

    ret_val=FS_RETURN_OK;
    snprintf(name, FS_MAX_PATHNAME_BUF_SIZE, "%s%lu", FS_FLASH_MTD_BLOCK_PREFIX, (unsigned long)mtd);
    fd = open(name, O_WRONLY, 0666);
    if( fd == -1 )
    {
        ERRORSTR("%s:ERROR!! Can not open %s!!\r\n", __FUNCTION__, name);
        return FS_RETURN_ERROR;
    }

    /* need to make device writable if it is readonly device
     */
    if(is_ro_mtd==TRUE)
    {
        policy=0;
        rc=ioctl(fd, BLKROSET, &policy);
        if(rc!=0)
        {
            ERRORSTR("%s:ERROR(%d,%d)!! Failed to change readonly attribute on %s.\r\n", __FUNCTION__, rc, errno, name);
            ret_val=FS_RETURN_ERROR;
            goto exit;
        }
    }

    count = write(fd, buf, length);
    if( count < length )
    {
        ERRORSTR("%s:ERROR!! read size %ld < %ld!!\r\n", __FUNCTION__, (long)count, (long)length);
        ret_val=FS_RETURN_ERROR;
        goto exit;
    }

    /* need to set device back to readonly if it is readonly device
     */
    if(is_ro_mtd==TRUE)
    {
        /* close the fd first to let flash programming begin
         * mtd_blktrans_thread() will start programming once the fd is closed
         */
        close(fd);

        /* Need to change the readonly attribute after the loader programming
         * done. From the current test, it can be done after close(fd) is
         * called, no delay is required.
         */
        fd = open(name, 0, 0666);
        if(fd==-1)
        {
            ERRORSTR("%s:ERROR(%d,%d)!! Failed to open %s.\r\n", __FUNCTION__, 0, errno, name);
            ret_val=FS_RETURN_ERROR;
            goto exit;
        }

        policy=1;
        rc=ioctl(fd, BLKROSET, &policy);
        if(rc!=0)
        {
            ERRORSTR("%s:ERROR(%d,%d)!! Failed to restore readonly attribute on %s.\r\n", __FUNCTION__, rc, errno, name);
            ret_val=FS_RETURN_ERROR;
            goto exit;
        }
    }
exit:
    close(fd);
    return ret_val;
}
#endif /* #if ((SYS_HWCFG_FLASH_TYPE & SYS_HWCFG_FLASH_TYPE_NOR)!=0) */

#if ((SYS_HWCFG_FLASH_TYPE & SYS_HWCFG_FLASH_TYPE_NAND)!=0)
/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_ReadNandFlash
 * ------------------------------------------------------------------------
 * FUNCTION : This function handles that read raw data from nand flash.
 * INPUT    : buf_len   -- the length that want to read.
 *            mtd       -- mtd number
 *
 * OUTPUT   : buf       -- buf for store reading data
 *            read_len  -- the length of buf from nand flash.
 * RETURN   : None
 * NOTE     : This func use "nanddump" to get data from nand flash.
 *            The advantage of nanddump is that can deal with bad block.
 *
 * ------------------------------------------------------------------------
 */
static UI32_T FS_ReadNandFlash(UI8_T *buf, UI32_T buf_len, UI32_T *read_len, UI32_T mtd)
{
    FILE *  fd;
    char    mtd_name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+1];
    char    path[FS_MAX_PATHNAME_BUF_SIZE];
    char    exec_cmd[FS_MAX_PATHNAME_BUF_SIZE];

    *read_len = 0;
    snprintf(mtd_name, SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+1, "%s%lu", FS_FLASH_MTD_PREFIX, (unsigned long)mtd);

    snprintf(path, FS_MAX_PATHNAME_BUF_SIZE, "%s%s", FS_TMP_FOLDER, FS_TMP_READ_FILE_NAME);
    memset(exec_cmd, 0, FS_MAX_PATHNAME_BUF_SIZE);
    snprintf(exec_cmd, FS_MAX_PATHNAME_BUF_SIZE, "%s %s%s%ld%s%s%s", FS_NANDDUMP_CMD, mtd_name, " -s 0 -l ", (long)buf_len," -f ", path , " >/dev/null 2>&1");
    DEBUGSTR("exec_cmd=%s\n", exec_cmd);
    system(exec_cmd);

    fd=fopen(path, "r");
    if(fd==NULL)
    {
        ERRORSTR("FS_ReadNandFlash: can not open file %s\r\n",path);
        return FS_RETURN_FILE_NEX;
    }
    *read_len = fread(buf, 1, buf_len, fd);
    if(*read_len<buf_len)
    {
        ERRORSTR("FS_ReadNandFlash: can not read file %s\r\n",path);
        fclose(fd);
        unlink(path);
        return FS_RETURN_FILE_TRUNCATED;
    }
    if(unlink(path)<0)
    {
        ERRORSTR("%s:%d,ERROR!! can not delete file (%s)!!.\r\n", __FUNCTION__, __LINE__, path);
        fclose(fd);
        return FS_RETURN_ERROR;
    }
    fclose(fd);
    return FS_RETURN_OK;
}
#endif /* #if ((SYS_HWCFG_FLASH_TYPE & SYS_HWCFG_FLASH_TYPE_NAND)!=0) */

#if ((SYS_HWCFG_FLASH_TYPE & SYS_HWCFG_FLASH_TYPE_NOR)!=0)
static UI32_T FS_ReadNorFlash(UI8_T *buf, UI32_T buf_len, UI32_T *read_len, UI32_T mtd)
{
    I32_T   fd;
    char    name[FS_MAX_PATHNAME_BUF_SIZE];

    *read_len = 0;

    snprintf(name, FS_MAX_PATHNAME_BUF_SIZE, "%s%lu", FS_FLASH_MTD_BLOCK_PREFIX, (unsigned long)mtd);
    fd = open(name, O_RDONLY, 0666);
    if( fd == -1 )
    {
        ERRORSTR("%s:ERROR!! Can not open %s!!\r\n", __FUNCTION__, name);
        return FS_RETURN_ERROR;
    }
    *read_len = read(fd, buf, buf_len);
    if( *read_len < buf_len )
    {
        DEBUGSTR("%s: read size %ld < %ld!!\r\n", __FUNCTION__, (long)*read_len, (long)buf_len);
        close(fd);
        return FS_RETURN_ERROR;
    }
    close(fd);
    DEBUGSTR(" %s read flash OK\r\n", __FUNCTION__);
    return FS_RETURN_OK;


}
#endif /* #if ((SYS_HWCFG_FLASH_TYPE & SYS_HWCFG_FLASH_TYPE_NOR)!=0) */


#if (SYS_CPNT_STACKING == TRUE)

void FS_Abort(void)
{
    /* Abort command can not be issued in abort state */
    if(FS_OM_GetControlState() == FS_STATE_ABORT || FS_OM_GetControlState() == FS_STATE_IDLE)
    {
        return;
    }

    FS_OM_SetControlState(FS_STATE_ABORT);
    FS_LOCK_REMOTE();
    FS_UNLOCK_REMOTE();
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_ISC_Handler
 * ------------------------------------------------------------------------
 * FUNCTION : This function is the serive demultiplexer
 * INPUT    : key       - key for ISC service
 *            mem_ref   - memory reference for received packet
 * OUTPUT   : None
 * RETURN   : TRUE: function completed successfully, FALSE: function failed
 * NOTE     : 1. This function will invoke functions corresponding to the opcode
 *               of the received packet.
 *            2. This function will check the sequence_number of the received
 *               packet.
 * ------------------------------------------------------------------------
 */
BOOL_T FS_ISC_Handler(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p)
{
    FS_Packet_Header_T  *rx_header_p;
    UI32_T              pdu_len;

    if( NULL == (rx_header_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len)))
    {
        return FALSE;
    }

    DUMP_RXPKT(*rx_header_p, sizeof(*rx_header_p));

    /* Check to abort operation if callback service id(opcode) is more then
     * number of callback service on this drive.
     */
    if (rx_header_p->opcode >= FS_TOTAL_REMOTE_SERVICES)
    {
        FS_Service_NotSupport(key);
        L_MM_Mref_Release(&mref_handle_p);
        return TRUE;
    }


    DEBUGSTR("%s() Calling %s()\r\n", __FUNCTION__, FS_Service_Names[rx_header_p->opcode]);

    /* mapping service callback function for 3 phases in FS_WriteFileToMutipleUnit */
    if (   rx_header_p->opcode == FS_MCAST_WRITE_REQUEST
        || rx_header_p->opcode == FS_MCAST_DATA
        || rx_header_p->opcode == FS_MCAST_PROGRAM_FLASH)
    {
        BOOL_T ret_value = FALSE;
        FS_Service_Table[rx_header_p->opcode](key, mref_handle_p, &ret_value);
        L_MM_Mref_Release(&mref_handle_p);
        return ret_value;
    }

    FS_Service_Table[rx_header_p->opcode](key, mref_handle_p);
    L_MM_Mref_Release(&mref_handle_p);

    return TRUE;
}


/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_ReadRemoteFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function will read a file from remote drive.
 * INPUT    : drive     - drive id
 *            file_attr - attributes of the file that are going to be written
 *                        or deleted
 *            offset    - the offset of the file to be copied
 *            byte_num  - the length of data to be copied
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E:
 * NOTE     : This function can read either a complete file or a partial file
 *            on remote drive
 * ------------------------------------------------------------------------
 */
static UI32_T  FS_ReadRemoteFile(UI32_T drive, FS_File_Attr_T *file_attr, UI32_T offset, UI32_T byte_num)
{
    UI16_T              retry_count;
    UI32_T              return_value;
    UI32_T              total_received_data_size = 0;
    UI32_T              total_useful_data_size = 0;
    UI32_T              copy_byte_num = 0;
    FS_Request_Packet_T *fs_rq_packet_p;
    FS_NAK_Packet_T     *fs_nak_packet_p;
    L_MM_Mref_Handle_T* mref_handle_p;
    UI32_T              pdu_len;

    /* 1.0 Send Read Request to remote drive */

    /* 1.1 stacking mode check */
    /* Check if FS is in the correct state */
    if(FS_OM_GetControlStackingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        ERRMSG(FS_RETURN_NOT_MASTER);
        return FS_RETURN_NOT_MASTER;
    }

    /* Check if drive exists */
    /*
    if(!STKTPLG_POM_UnitExist(drive))
    {
        ERRMSG(FS_RETURN_DRIVE_NOT_EXIST);
        return FS_RETURN_DRIVE_NOT_EXIST;
    }
    */

    DEBUGSTR("\r\n%s(): Remote operation for drive %d at line %d\r\n", __FUNCTION__, (int)drive, __LINE__);

    if(FS_OM_GetControlFileBuffer() == (void *) 0)
    {
        DEBUGSTR("%s  %s\r\n", __FUNCTION__, fs_error_messages[FS_RETURN_NO_BUFFER]);
        return FS_RETURN_NO_BUFFER;
    }

    /* Start remote operation. Block all other remote accesses */
    FS_LOCK_REMOTE();

    DEBUGSTR("%s() Read File: %s, size: %ld\r\n",
            __FUNCTION__,
            file_attr->file_name,
            (long)file_attr->file_size);
    /* Send the read request */
    retry_count = 0;
    FS_OM_SetControlFilePtr(FS_OM_GetControlFileBuffer());
    DBGMSG("Send request to remote drive");
    for (;;)
    {
        /* fill the content of the read request packet */
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Request_Packet_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_SEND, FS_READ_REQUEST) /* user_id */
                                             );
        fs_rq_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if (NULL == fs_rq_packet_p)
        {
            DEBUGSTR("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
            FS_UNLOCK_REMOTE();
            return FS_RETURN_ERROR;
        }
        fs_rq_packet_p->header.opcode    = FS_READ_REQUEST;
        fs_rq_packet_p->header.data_size = sizeof(FS_File_Attr_T);
        memcpy(&(fs_rq_packet_p->file_attr), file_attr, sizeof(FS_File_Attr_T));

        fs_rq_packet_p->header.seq_no = FS_OM_GetControlSeqNo();
        DUMP_TXPKT(*fs_rq_packet_p, GET_PDU_LEN(*fs_rq_packet_p));
        memset(&fs_packet_buffer, 0, sizeof(fs_packet_buffer) );

        /*dummy for ISC new design*/
        ;

        if (ISC_RemoteCall((UI8_T)drive, ISC_FS_SID, mref_handle_p,
                            SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                            sizeof(fs_packet_buffer),(UI8_T *) &fs_packet_buffer,
                            FS_ISC_TRY_COUNT, FS_TIMEOUT_VALUE) )
        {
            FS_OM_AddControlSeqNo();

            DUMP_RXPKT(fs_packet_buffer, GET_PDU_LEN(fs_packet_buffer));

            /* NAK received */
            if( fs_packet_buffer.header.opcode == FS_NAK )
            {
                return_value = fs_packet_buffer.data.error_code;
                ERRMSG(return_value);

                if(retry_count++ >= FS_COMMAND_RETRY_COUNT)
                {
                    return_value = FS_AbortSequence(drive);
                    ERRMSG(return_value);
                    FS_UNLOCK_REMOTE();
                    return return_value;
                }
                else
                {
                    FS_AbortSequence(drive);
                    SYSFUN_Sleep(FS_SLEEP_TIME);
                    continue;
                }
            }
            else /* ACK Received */
            {
                DBGMSG("Enter data receiving loop");
                DUMP_TXPKT(fs_packet_buffer, GET_PDU_LEN(fs_packet_buffer));
                total_received_data_size += fs_packet_buffer.header.data_size;
                if (fs_packet_buffer.header.data_size >= offset)
                {
                    if (fs_packet_buffer.header.data_size - offset > byte_num)
                    {
                        copy_byte_num = byte_num;
                    }
                    else
                    {
                        copy_byte_num = fs_packet_buffer.header.data_size - offset;
                    }
                    memcpy(FS_OM_GetControlFilePtr(), fs_packet_buffer.data.raw_data + offset, copy_byte_num);
                    FS_OM_AddControlFilePtr(copy_byte_num);
                    total_useful_data_size += copy_byte_num;
                    copy_byte_num = 0;
                }

                /* The file transmission is complete */
                if (fs_packet_buffer.header.data_size < FS_MAX_DATA_SIZE ||
                    total_useful_data_size == byte_num)
                {
                    FS_AbortSequence(drive);
                    return_value = FS_RETURN_OK;
                    DBGMSG("Data transfer completed");
                    FS_UNLOCK_REMOTE();
                    return return_value;
                }
                else /* more packets to come */
                    break;
            }
        }
        else /* ISC Error */
        {
            ERRMSG(FS_RETURN_TIMEOUT);
            return_value = FS_AbortSequence(drive);
            FS_UNLOCK_REMOTE();
            FS_ReturnIdle();
            return return_value;
        }
    }/* End for loop (request to remote) */

    FS_OM_SetControlState(FS_STATE_DATA_TRANSFER);

    /* 2.0 Remote request granted and first data packet received */
    /* Enter data receiving loop */
    for(;;)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_NAK_Packet_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_SEND, FS_ACK) /* user_id */
                                             );
        fs_nak_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if (NULL == fs_nak_packet_p)
        {
            DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
            return_value = FS_RETURN_ERROR;
            break;
        }

        fs_nak_packet_p->header.opcode    = FS_ACK;
        fs_nak_packet_p->header.seq_no    = FS_OM_GetControlSeqNo();
        fs_nak_packet_p->header.data_size = sizeof(fs_nak_packet_p->error_code);
        fs_nak_packet_p->error_code       = FS_RETURN_OK;

        memset(&fs_packet_buffer, 0, sizeof(fs_packet_buffer) );

        /*none for ISC new design*/
        ;
        if (ISC_RemoteCall((UI8_T)drive, ISC_FS_SID, mref_handle_p,
                            SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                            sizeof(fs_packet_buffer) , (UI8_T *) &fs_packet_buffer,
                            FS_ISC_TRY_COUNT, FS_TIMEOUT_VALUE) )
        {
            FS_OM_AddControlSeqNo();
            DUMP_RXPKT(fs_packet_buffer, GET_PDU_LEN(fs_packet_buffer));
            total_received_data_size += fs_packet_buffer.header.data_size;
            if (total_useful_data_size > 0)
            {
                if (total_useful_data_size + fs_packet_buffer.header.data_size > byte_num)
                {
                    copy_byte_num = byte_num - total_useful_data_size;
                }
                else
                {
                    copy_byte_num = fs_packet_buffer.header.data_size;
                }
                memcpy(FS_OM_GetControlFilePtr(), fs_packet_buffer.data.raw_data, copy_byte_num);
                FS_OM_AddControlFilePtr(copy_byte_num);
                total_useful_data_size += copy_byte_num;
            }
            else if (total_received_data_size >= offset)
            {
                if (fs_packet_buffer.header.data_size - offset > byte_num)
                {
                    copy_byte_num = byte_num;
                }
                else
                {
                    copy_byte_num = fs_packet_buffer.header.data_size - offset;
                }
                memcpy(FS_OM_GetControlFilePtr(), fs_packet_buffer.data.raw_data + offset, copy_byte_num);
                FS_OM_AddControlFilePtr(copy_byte_num);
                total_useful_data_size += copy_byte_num;
            }
            copy_byte_num = 0;

            if (    (total_useful_data_size < byte_num)
                &&  (total_received_data_size >= file_attr->file_size)
               )
            {
                DBGMSG("Data transfer completed, but cannot read enough number of byte\n");
                return_value = FS_RETURN_ERROR;
                break;
            }
            else if (fs_packet_buffer.header.data_size < FS_MAX_DATA_SIZE ||
                total_useful_data_size == byte_num)
            {
                DBGMSG("Data transfer completed");
                return_value = FS_RETURN_OK;
                break;
            }
        }
        else /* ISC Error */
        {
            ERRMSG(FS_RETURN_TIMEOUT);
            return_value = FS_AbortSequence(drive);
            break;
        }
    }
    FS_UNLOCK_REMOTE();

    if(return_value == FS_RETURN_OK)
    {
        DBGMSG("Read operation completed successfully");
    }
    else
    {
        ERRMSG(return_value);
    }
    return return_value;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_CopyRemoteFileContent
 * ------------------------------------------------------------------------
 * FUNCTION : This function will copy data from the specified offset of the
 *            file on remote to the buffer.
 * INPUT    : drive     - drive id
 *            file_attr - attributes of the file that are going to be written
 *                        or deleted
 *            offset    - the offset of the file to be copied
 *            byte_num  - the length of data to be copied
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E.
 * NOTE     : This function can only read partial file on remote drive.
 * ------------------------------------------------------------------------
 */
static UI32_T FS_CopyRemoteFileContent(UI32_T drive, FS_File_Attr_T *file_attr, UI32_T offset, UI32_T byte_num)
{
    UI16_T                              retry_count;
    UI32_T                              return_value;
    UI32_T                              total_received_data_size = 0;
    UI32_T                              total_useful_data_size = 0;
    UI32_T                              copy_byte_num = 0;
    FS_Copy_Content_Request_Packet_T    *copy_rq_packet_p;
    FS_NAK_Packet_T                     *fs_nak_packet_p;
    L_MM_Mref_Handle_T                  *mref_handle_p;
    UI32_T                              pdu_len;



    /* 1 - Start remote operation: block all other remote accesses. */
    FS_LOCK_REMOTE();

    DEBUGSTR("\r\n%s() Read File: %s, size: %ld\r\n",
            __FUNCTION__,
            file_attr->file_name,
            (long)file_attr->file_size);


    /* 1.4 - Send the request packet */
    retry_count = 0;
    DBGMSG("Send request to remote drive\r\n");
    for (;;)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Copy_Content_Request_Packet_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_SEND, FS_COPY_FILE_CONTENT) /* user_id */
                                             );
        copy_rq_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if (NULL == copy_rq_packet_p)
        {
            DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
            FS_UNLOCK_REMOTE();
            return FS_RETURN_ERROR;
        }

        /* 1.3 - Fill the content of the request packet */
        copy_rq_packet_p->header.opcode    = FS_COPY_FILE_CONTENT;
        copy_rq_packet_p->header.data_size = sizeof(FS_File_Attr_T) + sizeof(UI32_T) + sizeof(UI32_T);
        memcpy(&(copy_rq_packet_p->file_attr), file_attr, sizeof(FS_File_Attr_T));
        copy_rq_packet_p->offset   = offset;
        copy_rq_packet_p->byte_num = byte_num;

        copy_rq_packet_p->header.seq_no = FS_OM_GetControlSeqNo();
        DUMP_TXPKT(*copy_rq_packet_p, GET_PDU_LEN(*copy_rq_packet_p));
        memset(&fs_packet_buffer, 0, sizeof(fs_packet_buffer));

        if (ISC_RemoteCall((UI8_T)drive, ISC_FS_SID, mref_handle_p,
                            SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                            sizeof(fs_packet_buffer),(UI8_T *) &fs_packet_buffer,
                            FS_ISC_TRY_COUNT, FS_TIMEOUT_VALUE) )
        {
            FS_OM_AddControlSeqNo();

            DUMP_RXPKT(fs_packet_buffer, GET_PDU_LEN(fs_packet_buffer));

            if (fs_packet_buffer.header.opcode == FS_NAK)
            {
                /* NAK received */

                return_value = fs_packet_buffer.data.error_code;
                ERRMSG(return_value);

                if (retry_count++ >= FS_COMMAND_RETRY_COUNT)
                {
                    return_value = FS_AbortSequence(drive);
                    ERRMSG(return_value);
                    FS_UNLOCK_REMOTE();
                    return return_value;
                }
                else
                {
                    FS_AbortSequence(drive);
                    SYSFUN_Sleep(FS_SLEEP_TIME);
                    continue;
                }
            }
            else
            {
                /* ACK Received */

                DBGMSG("Enter data receiving loop.");
                DUMP_TXPKT(fs_packet_buffer, GET_PDU_LEN(fs_packet_buffer));
                total_received_data_size += fs_packet_buffer.header.data_size;

                if (fs_packet_buffer.header.data_size > byte_num)
                {
                    /* The received data size of once has already exceeded
                     * the data quantity of the request.
                     */
                    copy_byte_num = byte_num;
                }
                else
                {
                    /* Need to continue receiving the data. */
                    copy_byte_num = fs_packet_buffer.header.data_size;
                }
                memcpy(FS_OM_GetControlFilePtr(), fs_packet_buffer.data.raw_data, copy_byte_num);
                FS_OM_AddControlFilePtr(copy_byte_num);
                total_useful_data_size += copy_byte_num;
                copy_byte_num = 0;

                /* Check whether have already received the data required. */
                if (total_useful_data_size == byte_num)
                {
                    /* Have already received the amount of data needed. */
                    FS_AbortSequence(drive);
                    return_value = FS_RETURN_OK;
                    DBGMSG("Data transfer completed.\n");
                    FS_UNLOCK_REMOTE();
                    return return_value;
                }
                else
                {
                    /* More packets to come, enter second data receiving loop. */
                    break;
                }
            }
        }
        else /* ISC Error */
        {
            ERRMSG(FS_RETURN_TIMEOUT);
            return_value = FS_AbortSequence(drive);
            FS_UNLOCK_REMOTE();
            FS_ReturnIdle();
            return return_value;
        }
    } /* End for loop (request to remote) */

    /* 2 - Remote request granted and first data packet received */
    /* Enter data receiving loop */
    FS_OM_SetControlState(FS_STATE_DATA_TRANSFER);
    for (;;)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_NAK_Packet_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_SEND, FS_ACK) /* user_id */
                                             );
        fs_nak_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if (NULL == fs_nak_packet_p)
        {
            DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
            return_value = FS_RETURN_ERROR;
            break;
        }

        fs_nak_packet_p->header.opcode    = FS_ACK;
        fs_nak_packet_p->header.seq_no    = FS_OM_GetControlSeqNo();
        fs_nak_packet_p->header.data_size = sizeof(fs_nak_packet_p->error_code);
        fs_nak_packet_p->error_code       = FS_RETURN_OK;

        memset(&fs_packet_buffer, 0, sizeof(fs_packet_buffer) );

        if (ISC_RemoteCall((UI8_T)drive, ISC_FS_SID, mref_handle_p,
                            SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                            sizeof(fs_packet_buffer) , (UI8_T *) &fs_packet_buffer,
                            FS_ISC_TRY_COUNT, FS_TIMEOUT_VALUE) )
        {
            FS_OM_AddControlSeqNo();
            DUMP_RXPKT(fs_packet_buffer, GET_PDU_LEN(fs_packet_buffer));
            total_received_data_size += fs_packet_buffer.header.data_size;

            if (total_useful_data_size + fs_packet_buffer.header.data_size > byte_num)
            {
                /* The received data size has already exceeded the data quantity
                 * of the request.
                 */
                copy_byte_num = byte_num - total_useful_data_size;
            }
            else
            {
                /* Need to continue receiving the data. */
                copy_byte_num = fs_packet_buffer.header.data_size;
            }
            memcpy(FS_OM_GetControlFilePtr(), fs_packet_buffer.data.raw_data, copy_byte_num);
            FS_OM_AddControlFilePtr(copy_byte_num);
            total_useful_data_size += copy_byte_num;

            copy_byte_num = 0;

            /* Check whether have already received the data required. */
            if (   (total_useful_data_size < byte_num)
                && (total_received_data_size >= file_attr->file_size - offset))
            {
                /* The number of bytes required exceeds the size of the file. */
                DBGMSG("Data transfer completed, but cannot read enough number of byte.\n");
                return_value = FS_RETURN_ERROR;
                break;
            }
            else if (total_useful_data_size == byte_num)
            {
                /* Have already received the amount of data needed. */
                DBGMSG("Data transfer completed.\n");
                return_value = FS_RETURN_OK;
                break;
            }
        }
        else /* ISC Error */
        {
            ERRMSG(FS_RETURN_TIMEOUT);
            return_value = FS_AbortSequence(drive);
            break;
        }
    }
    FS_UNLOCK_REMOTE();

    if(return_value == FS_RETURN_OK)
    {
        DBGMSG("Read operation completed successfully");
    }
    else
        ERRMSG(return_value);

    return return_value;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_WriteRemoteFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function will perform write operations on remote drive.
 * INPUT    : drive     - drive id
 *            file_attr - attributes of the file that are going to be written
 *                        or deleted
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E:
 * NOTE     : This function can perform write and delete files on remote
 *            drive.
 *            1. If file_attr->file_size and file_attr->storage_size are both
 *               zero, it is a delete command
 *            2. If file_attr->file_name is a null string and
 *               file_attr->startup_file is TRUE, it means that we are
 *               writing a boot loader.
 * ------------------------------------------------------------------------
 */
static UI32_T FS_WriteRemoteFile(UI32_T drive, FS_File_Attr_T *file_attr)
{
    UI32_T              return_value;
    UI32_T              remaining_size;
    UI32_T              retry_count;
    BOOL_T              is_erase;
    BOOL_T              done;
    FS_Request_Packet_T *fs_rq_packet_p;
    FS_Data_Packet_T    *fs_data_packet_p;
    FS_Packet_Header_T  *fs_flash_packet_p;
    L_MM_Mref_Handle_T  *mref_handle_p;
    UI32_T              pdu_len;

    /* 1.0 Send Write Request to remote drive */
    /* 1.1 FS stacking mode check */
    if(FS_OM_GetControlStackingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        ERRMSG(FS_RETURN_NOT_MASTER);
        return FS_RETURN_NOT_MASTER;
    }
    /* Check if drive exists */
    /*
    if(!STKTPLG_POM_UnitExist(drive))
    {
        ERRMSG(FS_RETURN_DRIVE_NOT_EXIST);
        return FS_RETURN_DRIVE_NOT_EXIST;
    }
*/

    DEBUGSTR("\r\n%s(): Remote operation for drive %d at line %d\r\n", __FUNCTION__, (int)drive, __LINE__);

    /* 1.2 Prepare request packet and send */
    FS_LOCK_REMOTE();

    if(file_attr->file_name[0] != 0 && file_attr->file_size == 0)
    {
        is_erase = TRUE;
        DBGMSG("Erase operation");
    }
    else
    {
        is_erase = FALSE;
        DBGMSG("Copy operation");
    }

    retry_count = 0;
    for(;;)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Request_Packet_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_SEND, FS_WRITE_REQUEST) /* user_id */
                                             );
        fs_rq_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if (NULL == fs_rq_packet_p)
        {
            DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
            FS_UNLOCK_REMOTE();
            FS_ReturnIdle();
            return FS_RETURN_ERROR;
        }

        memset(fs_rq_packet_p, 0, sizeof(*fs_rq_packet_p));

        /* fill the content of tx packet */
        fs_rq_packet_p->header.opcode       = FS_WRITE_REQUEST;
        fs_rq_packet_p->header.data_size    = sizeof(FS_File_Attr_T);
        memcpy(&(fs_rq_packet_p->file_attr), file_attr, sizeof(FS_File_Attr_T));

        fs_rq_packet_p->header.seq_no       = FS_OM_GetControlSeqNo();
        DBGMSG("Send File Information to Slave");
        DUMP_TXPKT(*fs_rq_packet_p, GET_PDU_LEN(*fs_rq_packet_p));
        memset(&fs_packet_buffer, 0, sizeof(fs_packet_buffer) );

        fs_packet_buffer.header.opcode = FS_ACK;/*init as success for ISC new design*/
        if (ISC_RemoteCall((UI8_T)drive, ISC_FS_SID, mref_handle_p,
                            SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                            sizeof(fs_packet_buffer), (UI8_T *) &fs_packet_buffer,
                            FS_ISC_TRY_COUNT, FS_TIMEOUT_VALUE) )
        {
            /* 1.3.1 packet received successfully, increase seq_no */
            FS_OM_AddControlSeqNo();

            DUMP_RXPKT(fs_packet_buffer, GET_PDU_LEN(fs_packet_buffer));

            /* check packet content */
            if((fs_packet_buffer.header.opcode == FS_ACK))
            {
                /* ACK received, continue */
                break;
            }
            /* 1.3.1 a. NAK received */
            else
            {
                if(retry_count++ >= FS_COMMAND_RETRY_COUNT)
                {
                    return_value = FS_AbortSequence(drive);
                    FS_UNLOCK_REMOTE();
                    FS_ReturnIdle();
                    return return_value;
                }
                else /* Try Again */
                {
                    return_value = fs_packet_buffer.data.error_code;
                    ERRMSG(return_value);
                    FS_AbortSequence(drive);
                    SYSFUN_Sleep(FS_SLEEP_TIME);
                    continue;
                }
            }
        }
        else /* 1.3.1 b. ISC error */
        {
            ERRMSG(FS_RETURN_TIMEOUT);
            return_value = FS_AbortSequence(drive);
            FS_UNLOCK_REMOTE();
            FS_ReturnIdle();
            return return_value;
        }
    }

    /* 1.3.1.1 Erase operation */
    /* Check flash programming status and return */
    /* Caller can't abort an erase operation */
    if( is_erase )
    {
        DBGMSG("Check Erase Progress");
        return_value = FS_CheckRemoteStatus(drive);
        FS_UNLOCK_REMOTE();
        FS_ReturnIdle();
        return return_value;
    }

    /* 1.3.1 c. Check for caller abort */
    if(FS_OM_GetControlState() == FS_STATE_ABORT)
    {
        DBGMSG("Caller Abort");
        return_value = FS_AbortSequence(drive);
        FS_UNLOCK_REMOTE();
        FS_ReturnIdle();
        return return_value;
    }

    FS_OM_SetControlFilePtr(FS_OM_GetControlFileBuffer());
    remaining_size      = FS_OM_GetControlAttrFileSize(); // fs_control.file_attr.file_size;

    DBGMSG("Data Transfering Loop");
    done = FALSE;

    for(; !done; )
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Data_Packet_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_SEND, FS_DATA) /* user_id */
                                             );
        fs_data_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if (NULL == fs_data_packet_p)
        {
            DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
            FS_UNLOCK_REMOTE();
            FS_ReturnIdle();
            return FS_RETURN_ERROR;
        }

        /* 2.0 Write request granted by remote drive, start data transfer */
        fs_data_packet_p->header.opcode  = FS_DATA;

        fs_data_packet_p->header.seq_no  = FS_OM_GetControlSeqNo();

        /* 2.1.1 calculate remaining file size */
        /* here, if file size if multiple of FS_MAX_DATA_SIZE, there will be a blank packet
         * sent at last to let remote FS_RemoteReceiveData know it is the last packet
         */
        if(remaining_size >= FS_MAX_DATA_SIZE)
        {
            fs_data_packet_p->header.data_size = FS_MAX_DATA_SIZE;
            remaining_size -= FS_MAX_DATA_SIZE;
        }
        else
        {
            fs_data_packet_p->header.data_size = remaining_size;
            done=TRUE;
        }
        /* Copy data from data buffer to packet */
        memcpy(fs_data_packet_p->raw_data, FS_OM_GetControlFilePtr(), fs_data_packet_p->header.data_size);
        FS_OM_AddControlFilePtr(fs_data_packet_p->header.data_size);

        /* 2.1.3 Send data packet */
        DUMP_TXPKT(*fs_data_packet_p, GET_PDU_LEN(*fs_data_packet_p));
        memset(&fs_packet_buffer, 0, sizeof(fs_packet_buffer) );

        fs_packet_buffer.header.opcode = FS_ACK;/*init as success for ISC new design*/
        if (ISC_RemoteCall((UI8_T)drive, ISC_FS_SID, mref_handle_p,
                            SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                            sizeof(fs_packet_buffer), (UI8_T *) &fs_packet_buffer,
                            FS_ISC_TRY_COUNT, FS_TIMEOUT_VALUE) )
        {
            /* packet received successfully, increase seq_no */
            FS_OM_AddControlSeqNo();

            /* check packet content */
            DUMP_RXPKT(fs_packet_buffer, GET_PDU_LEN(fs_packet_buffer));
            if( fs_packet_buffer.header.opcode == FS_ACK )
            {
                /* ACK received, do nothing */
                SYSFUN_Sleep(1);
            }
            /* 2.1.3 a. a NAK or a unexpected packet received */
            else
            {

                return_value = fs_packet_buffer.data.error_code;
                ERRMSG(return_value);
                /* Enter abort state */
                FS_AbortSequence(drive);
                FS_UNLOCK_REMOTE();
                FS_ReturnIdle();
                return return_value;
            }
        }
        else /* 2.1.3 b. ISC error */
        {
            ERRMSG(FS_RETURN_TIMEOUT);
            return_value = FS_AbortSequence(drive);
            FS_UNLOCK_REMOTE();
            FS_ReturnIdle();
            return return_value;
        }

        /* 2.1.3 c. Check for caller abort */
        if(FS_OM_GetControlState() == FS_STATE_ABORT)
        {
            return_value = FS_AbortSequence(drive);
            ERRMSG(return_value);
            FS_UNLOCK_REMOTE();
            FS_ReturnIdle();
            return return_value;
        }
    }

    DBGMSG("Data Transfer is Done");
    SYSFUN_Sleep(FS_SLEEP_TIME);

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Packet_Header_T), /* tx_buffer_size */
                                          L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_SEND, FS_PROGRAM_FLASH) /* user_id */
                                         );
    fs_flash_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    if (NULL == fs_flash_packet_p)
    {
        DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
        FS_UNLOCK_REMOTE();
        FS_ReturnIdle();
        return FS_RETURN_ERROR;
    }
    /* 3.0 data transfer completed, send flash command */
    fs_flash_packet_p->opcode       = FS_PROGRAM_FLASH;
    fs_flash_packet_p->seq_no       = FS_OM_GetControlSeqNo();
    fs_flash_packet_p->data_size    = 0;

    /* 3.1 Send Program flash command */
    DBGMSG("Send Program Flash Command");
    DUMP_TXPKT(*fs_flash_packet_p, sizeof(*fs_flash_packet_p));
    memset(&fs_packet_buffer, 0, sizeof(fs_packet_buffer));

    fs_packet_buffer.header.opcode = FS_ACK;/*init as success for ISC new design*/
    if (ISC_RemoteCall((UI8_T)drive, ISC_FS_SID, mref_handle_p,
                        SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                        sizeof(fs_packet_buffer), (UI8_T *) &fs_packet_buffer,
                        FS_ISC_TRY_COUNT, FS_TIMEOUT_VALUE) )
    {
        /* packet received successfully, increase seq_no */
        FS_OM_AddControlSeqNo();

        /* check packet content */
        DUMP_RXPKT(fs_packet_buffer, GET_PDU_LEN(fs_packet_buffer));

        /* ACK received */
        if(fs_packet_buffer.header.opcode == FS_ACK)
        {
            /* ACK received, do nothing */
            DBGMSG("Slave is writing the file");
        }
        else /* 3.1.1 a. NAK received */
        {
            return_value = fs_packet_buffer.data.error_code;
            ERRMSG(return_value);

            /* Enter abort state */
            FS_AbortSequence(drive);
            FS_UNLOCK_REMOTE();
            FS_ReturnIdle();
            return return_value;

        }
    }
    else /* 3.1.2 b. ISC error */
    {
        ERRMSG(FS_RETURN_TIMEOUT);
        return_value = FS_AbortSequence(drive);
        FS_UNLOCK_REMOTE();
        FS_ReturnIdle();
        return return_value;
    }

    SYSFUN_Sleep(FS_SLEEP_TIME);
    /* 4.0 Check flash status */
    return_value = FS_CheckRemoteStatus(drive);

    if(return_value == FS_RETURN_OK)
    {
        DBGMSG("Operation Completed Successfully");
    }
    else
    {
        ERRMSG(return_value);
        FS_AbortSequence(drive);
    }

    FS_UNLOCK_REMOTE();
    FS_ReturnIdle();
    return return_value;
}


/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetRemoteFileSystemSummary
 * ------------------------------------------------------------------------
 * FUNCTION : Get file system summary on remote drive.
 * INPUT    : drive         - drive id
 * OUTPUT   : file_summary  - hold return file system summary of remote drive
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : 1. The remote handler function is FS_RemoteGetFileSystemSummary
 * ------------------------------------------------------------------------
 */
static UI32_T FS_GetRemoteFileSystemSummary(UI32_T drive, FS_File_Summary_Packet_T *file_summary)
{
    FS_Packet_Header_T   *fs_rq_packet_p;
    L_MM_Mref_Handle_T   *mref_handle_p;
    UI32_T               pdu_len;

    UI32_T  return_value;
    UI32_T  retry_count;

    /* Check if FS is in the right stacking mode */
    if(FS_OM_GetControlStackingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        ERRMSG(FS_RETURN_NOT_MASTER);
        return FS_RETURN_NOT_MASTER;
    }

    DEBUGSTR("\r\n%s(): Remote operation for drive %d at line %d\r\n", __FUNCTION__, (int)drive, __LINE__);


    FS_LOCK_REMOTE();

    /* send packet to remote drive */

    retry_count = 0;
    for(;;)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Request_Packet_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_SEND, FS_GETFILESUMMARY) /* user_id */
                                             );
        fs_rq_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if (NULL == fs_rq_packet_p)
        {
            DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
            return_value = FS_RETURN_ERROR;
            break;
        }

        /* fill header with appropriate data */
        fs_rq_packet_p->opcode    = FS_GETFILESUMMARY;
        fs_rq_packet_p->data_size = 0;

        fs_rq_packet_p->seq_no = FS_OM_GetControlSeqNo();
        memset(&fs_packet_buffer, 0, sizeof(fs_packet_buffer) );

        /* Dump the packet content if debug flag is set */
        DUMP_TXPKT(*fs_rq_packet_p, sizeof(*fs_rq_packet_p));

        if (ISC_RemoteCall((UI8_T)drive, ISC_FS_SID, mref_handle_p,
                            SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                            sizeof(fs_packet_buffer), (UI8_T *) &fs_packet_buffer,
                            FS_ISC_TRY_COUNT, FS_TIMEOUT_VALUE) )
        {
            /* ISC returned successful, increase seq_no by 1 */
            FS_OM_AddControlSeqNo();

            DUMP_RXPKT(fs_packet_buffer, GET_PDU_LEN(fs_packet_buffer));

            /* ACK received */
            if( fs_packet_buffer.header.opcode == FS_ACK)
            {
                memcpy(file_summary, &fs_packet_buffer, sizeof(*file_summary));
                return_value = FS_RETURN_OK;
                break;
            }
            else /* NAK Received*/
            {
                return_value = fs_packet_buffer.data.error_code;
                ERRMSG(return_value);
                FS_AbortSequence(drive);

                if(retry_count++ >= FS_COMMAND_RETRY_COUNT)
                {
                    return_value = FS_AbortSequence(drive);
                    ERRMSG(return_value);
                    break;
                }
                else
                {
                    DBGMSG("Try Again");
                    SYSFUN_Sleep(FS_SLEEP_TIME);
                    continue;
                }
            }
        }
        else /* ISC error */
        {
            ERRMSG(FS_RETURN_TIMEOUT);
            return_value = FS_AbortSequence(drive);
            break;
        }
    }/* End for loop */

    FS_UNLOCK_REMOTE();
    return return_value;
}
#if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE)
/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetRemoteStorageFreeSpaceForUpdateRuntime
 * ------------------------------------------------------------------------
 * FUNCTION : Get the available free space when it is used
 *            to consider the free space for runtime file update.
 * INPUT    : drive         - drive id
 * OUTPUT   : free_space_p              -- total size of the free space
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : 1. The remote handler function is FS_RemoteGetStorageFreeSpaceForUpdateRuntime
 * ------------------------------------------------------------------------
 */
static UI32_T FS_GetRemoteStorageFreeSpaceForUpdateRuntime(UI32_T drive, UI32_T *free_space_p)
{
    FS_Packet_Header_T   *fs_rq_packet_p;
    L_MM_Mref_Handle_T   *mref_handle_p;
    UI32_T               pdu_len;

    UI32_T  return_value;
    UI32_T  retry_count;

    /* Check if FS is in the right stacking mode */
    if(FS_OM_GetControlStackingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        ERRMSG(FS_RETURN_NOT_MASTER);
        return FS_RETURN_NOT_MASTER;
    }

    DEBUGSTR("%s(): Remote operation for drive %d at line %d\r\n", __FUNCTION__, (int)drive, __LINE__);


    FS_LOCK_REMOTE();

    /* send packet to remote drive */

    retry_count = 0;
    for(;;)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Request_Packet_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_SEND, FS_GETSTORAGEFREESPACEFORUPDATERUNTIME) /* user_id */
                                             );
        fs_rq_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if (NULL == fs_rq_packet_p)
        {
            DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\r\n",__FUNCTION__);
            return_value = FS_RETURN_ERROR;
            break;
        }

        /* fill header with appropriate data */
        fs_rq_packet_p->opcode    = FS_GETSTORAGEFREESPACEFORUPDATERUNTIME;
        fs_rq_packet_p->data_size = 0;

        fs_rq_packet_p->seq_no = FS_OM_GetControlSeqNo();
        memset(&fs_packet_buffer, 0, sizeof(fs_packet_buffer) );

        /* Dump the packet content if debug flag is set */
        DUMP_TXPKT(*fs_rq_packet_p, sizeof(*fs_rq_packet_p));

        if (ISC_RemoteCall((UI8_T)drive, ISC_FS_SID, mref_handle_p,
                            SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                            sizeof(fs_packet_buffer), (UI8_T *) &fs_packet_buffer,
                            FS_ISC_TRY_COUNT, FS_TIMEOUT_VALUE) )
        {
            /* ISC returned successful, increase seq_no by 1 */
            FS_OM_AddControlSeqNo();

            DUMP_RXPKT(fs_packet_buffer, GET_PDU_LEN(fs_packet_buffer));

            /* ACK received */
            if( fs_packet_buffer.header.opcode == FS_ACK)
            {
                *free_space_p = fs_packet_buffer.data.drive_info.total_space_free;
                return_value = FS_RETURN_OK;
                break;
            }
            else /* NAK Received*/
            {
                return_value = fs_packet_buffer.data.error_code;
                ERRMSG(return_value);
                FS_AbortSequence(drive);

                if(retry_count++ >= FS_COMMAND_RETRY_COUNT)
                {
                    return_value = FS_AbortSequence(drive);
                    ERRMSG(return_value);
                    break;
                }
                else
                {
                    DBGMSG("Try Again");
                    SYSFUN_Sleep(FS_SLEEP_TIME);
                    continue;
                }
            }
        }
        else /* ISC error */
        {
            ERRMSG(FS_RETURN_TIMEOUT);
            return_value = FS_AbortSequence(drive);
            break;
        }
    }/* End for loop */

    FS_UNLOCK_REMOTE();
    return return_value;
}
#endif /* End of #if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE)*/
/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetRemoteMultipleFileInfo
 * ------------------------------------------------------------------------
 * FUNCTION : Get multiple file attrs on remote drive.
 * INPUT    : drive             - drive id
 *            next              - get current file or next file
 *            file_attr_packet  - file_attr_packet->file_attr[0].file_name:
 *                                     target file name of file, NULL string for first file
 *                                file_attr_packet->file_attr[0].file_type_mask:
 *                                     type mask of file, only files that match type mask are searched
 *            max_count         - maximal file attr number that return
 * OUTPUT   : file_attr_packet  - hold all the found file attrs
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : 1. The remote handler function is FS_RemoteGetMultipleFileInfo
 * ------------------------------------------------------------------------
 */
static UI32_T  FS_GetRemoteMultipleFileInfo(UI32_T drive, BOOL_T next, FS_File_Attr_Packet_T * file_attr_packet, UI32_T max_count)
{
    FS_Request_Packet_T  *fs_rq_packet_p;
    L_MM_Mref_Handle_T   *mref_handle_p;
    UI32_T               pdu_len;

    UI32_T  return_value;
    UI32_T  retry_count;

    /* Check if FS is in the right stacking mode */
    if(FS_OM_GetControlStackingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        ERRMSG(FS_RETURN_NOT_MASTER);
        return FS_RETURN_NOT_MASTER;
    }



    DEBUGSTR("\r\n%s(): Remote operation for drive %d at line %d\r\n", __FUNCTION__, (int)drive, __LINE__);


    if (0 == max_count)
    {
        return FS_RETURN_BAD_PARA;
    }

    FS_LOCK_REMOTE();

    /* send packet to remote drive */

    retry_count = 0;
    for(;;)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Request_Packet_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_SEND,FS_GETMULTIPLEFILEINFO) /* user_id */
                                             );
        fs_rq_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if (NULL == fs_rq_packet_p)
        {
            DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
            return_value = FS_RETURN_ERROR;
            break;
        }

        /* fill header with appropriate data */
        fs_rq_packet_p->header.opcode    = FS_GETMULTIPLEFILEINFO;
        fs_rq_packet_p->header.data_size = 0;
        fs_rq_packet_p->max_count        = max_count;
        fs_rq_packet_p->next             = next;
        memcpy(&(fs_rq_packet_p->file_attr),&(file_attr_packet->file_attr[0]),sizeof(FS_File_Attr_T));

        fs_rq_packet_p->header.seq_no = FS_OM_GetControlSeqNo();
        memset(&fs_packet_buffer, 0, sizeof(fs_packet_buffer) );

        /* Dump the packet content if debug flag is set */
        DUMP_TXPKT(*fs_rq_packet_p, GET_PDU_LEN(*fs_rq_packet_p));

        if (ISC_RemoteCall((UI8_T)drive, ISC_FS_SID, mref_handle_p,
                            SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                            sizeof(fs_packet_buffer), (UI8_T *) &fs_packet_buffer,
                            FS_ISC_TRY_COUNT, FS_TIMEOUT_VALUE) )
        {
            /* ISC returned successful, increase seq_no by 1 */
            FS_OM_AddControlSeqNo();

            DUMP_RXPKT(fs_packet_buffer, GET_PDU_LEN(fs_packet_buffer));

            /* ACK received */
            if( fs_packet_buffer.header.opcode == FS_ACK ||
                fs_packet_buffer.header.opcode == FS_HAVEMOREDATA)
            {
                if (fs_packet_buffer.data.files_info.file_count > 0)
                {
                    memcpy(file_attr_packet, &fs_packet_buffer, sizeof(FS_Packet_Header_T) + sizeof(UI16_T)
                           + sizeof(FS_File_Attr_T)*fs_packet_buffer.data.files_info.file_count);
                }
                if (fs_packet_buffer.header.opcode == FS_ACK)
                {
                    if (fs_packet_buffer.data.files_info.file_count>0)
                    {
                        return_value = FS_RETURN_OK;
                    }
                    else
                    {
                        return_value = FS_RETURN_FILE_NEX;
                    }
                }
                else
                {
                    return_value = FS_RETURN_HAVEMOREDATA;
                }

                break;
            }
            else /* NAK Received*/
            {
                return_value = fs_packet_buffer.data.error_code;
                ERRMSG(return_value);
                FS_AbortSequence(drive);

                if(retry_count++ >= FS_COMMAND_RETRY_COUNT)
                {
                    return_value = FS_AbortSequence(drive);
                    ERRMSG(return_value);
                    break;
                }
                else
                {
                    DBGMSG("Try Again");
                    SYSFUN_Sleep(FS_SLEEP_TIME);
                    continue;
                }
            }
        }
        else /* ISC error */
        {
            ERRMSG(FS_RETURN_TIMEOUT);
            return_value = FS_AbortSequence(drive);
            break;
        }
    }/* End for loop */

    FS_UNLOCK_REMOTE();
    return return_value;
}


/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetRemoteStartupFilename
 * ------------------------------------------------------------------------
 * FUNCTION : Get startup file name of target file type on remote drive.
 * INPUT    : drive             - drive id
 *            file_attr_packet  - file_attr_packet->file_attr[0].file_type:
 *                                   target file type
 * OUTPUT   : file_attr_packet  - file_attr_packet->file_attr[0].file_name:
 *                                   hold return file name
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : 1. The remote handler function is FS_RemoteGetStartupFilename
 * ------------------------------------------------------------------------
 */
static UI32_T  FS_GetRemoteStartupFilename(UI32_T drive, FS_File_Attr_Packet_T *file_attr_packet)
{
    FS_Request_Packet_T  *fs_rq_packet_p;
    L_MM_Mref_Handle_T   *mref_handle_p;
    UI32_T               pdu_len;

    UI32_T  return_value;
    UI32_T  retry_count;

    /* Check if FS is in the correct stacking mode */
    if(FS_OM_GetControlStackingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        ERRMSG(FS_RETURN_NOT_MASTER);
        return FS_RETURN_NOT_MASTER;
    }


    DEBUGSTR("\r\n%s(): Remote operation for drive %d at line %d\r\n", __FUNCTION__, (int)drive, __LINE__);


    if (file_attr_packet->file_attr[0].file_type >= FS_FILE_TYPE_TOTAL)
    {
        return FS_RETURN_BAD_PARA;
    }

    FS_LOCK_REMOTE();

    /* send packet to remote drive */

    retry_count = 0;
    for(;;)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Request_Packet_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_SEND, FS_GETSTARTUPFILENAME) /* user_id */
                                             );
        fs_rq_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if (NULL == fs_rq_packet_p)
        {
            DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
            return_value = FS_RETURN_ERROR;
            break;
        }

        /* fill header with appropriate data */
        fs_rq_packet_p->header.opcode    = FS_GETSTARTUPFILENAME;
        fs_rq_packet_p->header.data_size = 0;
        fs_rq_packet_p->file_attr.file_type = file_attr_packet->file_attr[0].file_type;

        fs_rq_packet_p->header.seq_no = FS_OM_GetControlSeqNo();
        memset(&fs_packet_buffer, 0, sizeof(fs_packet_buffer) );

        /* Dump the packet content if debug flag is set */
        DUMP_TXPKT(*fs_rq_packet_p, GET_PDU_LEN(*fs_rq_packet_p));

        if (ISC_RemoteCall((UI8_T)drive, ISC_FS_SID, mref_handle_p,
                            SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                            sizeof(fs_packet_buffer), (UI8_T *) &fs_packet_buffer,
                            FS_ISC_TRY_COUNT, FS_TIMEOUT_VALUE) )
        {
            /* ISC returned successful, increase seq_no by 1 */
            FS_OM_AddControlSeqNo();

            DUMP_RXPKT(fs_packet_buffer, GET_PDU_LEN(fs_packet_buffer));

            /* ACK received */
            if( fs_packet_buffer.header.opcode == FS_ACK)
            {
                if (fs_packet_buffer.data.files_info.file_count == 1)
                {
                    memcpy(file_attr_packet, &fs_packet_buffer, sizeof(FS_Packet_Header_T) + sizeof(UI16_T)
                            + sizeof(FS_File_Attr_T));
                    return_value = FS_RETURN_OK;
                }
                else
                {
                    return_value = FS_RETURN_FILE_NEX;
                }

                break;
            }
            else /* NAK Received*/
            {
                return_value = fs_packet_buffer.data.error_code;
                ERRMSG(return_value);
                FS_AbortSequence(drive);

                if(retry_count++ >= FS_COMMAND_RETRY_COUNT)
                {
                    return_value = FS_AbortSequence(drive);
                    ERRMSG(return_value);
                    break;
                }
                else
                {
                    DBGMSG("Try Again");
                    SYSFUN_Sleep(FS_SLEEP_TIME);
                    continue;
                }
            }
        }
        else /* ISC error */
        {
            ERRMSG(FS_RETURN_TIMEOUT);
            return_value = FS_AbortSequence(drive);
            break;
        }
    }/* End for loop */

    FS_UNLOCK_REMOTE();
    return return_value;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_AbortSequence
 * ------------------------------------------------------------------------
 * FUNCTION : Send abort packets to remote drive
 * INPUT    : drive - drive id
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E:
 *            FS_RETURN_DISCONNECTED - The connection is broken
 *            FS_RETURN_ABORTED      - Aborted successfully
 * NOTE     : Send Abort packet to remote
 *            1. Keep sending Abort if NAK is received.
 *            2. Report disconnected if the timed out.
 * ------------------------------------------------------------------------
 */
static UI32_T FS_AbortSequence(UI32_T drive)
{
    UI32_T return_value;
    UI32_T retry_count;

    FS_Packet_Header_T   *fs_abort_packet_p;
    L_MM_Mref_Handle_T   *mref_handle_p;
    UI32_T               pdu_len;

    retry_count = 0;
    for(;;)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Packet_Header_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_SEND, FS_ABORT) /* user_id */
                                             );
        fs_abort_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if (NULL == fs_abort_packet_p)
        {
            DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
            return_value = FS_RETURN_ERROR;
            break;
        }

        /* Fill packet header with appropriate value */
        fs_abort_packet_p->opcode    = FS_ABORT;
        fs_abort_packet_p->data_size = 0;

        fs_abort_packet_p->seq_no = FS_OM_GetControlSeqNo();

        DBGMSG("Send abort command");
        DUMP_TXPKT(*fs_abort_packet_p, sizeof(*fs_abort_packet_p));

        memset(&fs_packet_buffer, 0, sizeof(fs_packet_buffer) );

        fs_packet_buffer.header.opcode = FS_ACK;/*init as success for ISC new design*/
        if (ISC_RemoteCall((UI8_T)drive, ISC_FS_SID, mref_handle_p,
                            SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                            sizeof(fs_packet_buffer), (UI8_T *) &fs_packet_buffer,
                            FS_ISC_TRY_COUNT, FS_TIMEOUT_VALUE) )
        {
            DUMP_RXPKT(fs_packet_buffer, GET_PDU_LEN(fs_packet_buffer));

            /* Increment seq_no by 1 */
            FS_OM_AddControlSeqNo();

            /* ACK received, slave returned to idle state successfully */
            if( fs_packet_buffer.header.opcode == FS_ACK )
            {
                return_value = FS_RETURN_ABORTED;
                DBGMSG("Slave returned to Idle");
                break;
            }
            else
            {
                if(retry_count++ >= FS_ABORT_COUNT)
                {
                    return_value = FS_RETURN_DISCONNECTED;
                    ERRMSG(return_value);
                    break;
                }
                else
                {
                    /* NAK returned, send abort again*/
                    SYSFUN_Sleep(FS_SLEEP_TIME);
                    DBGMSG("Send abort again");
                    continue;
                }
            }
        }
        else /* ISC timed out or ISC error, report disconnect */
        {
            if(retry_count++ >= FS_ABORT_COUNT)
            {
                return_value = FS_RETURN_DISCONNECTED;
                ERRMSG(return_value);
                break;
            }
            else
            {
                ERRMSG(FS_RETURN_TIMEOUT);
                continue;
            }
        }
    }/* End for loop */
    return return_value;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_CheckRemoteStatus
 * ------------------------------------------------------------------------
 * FUNCTION : This function will check the flash programming status of the
 *            remote drive
 * INPUT    : drive     - drive id
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E:
 * NOTE     : 1. This function can perform write and delete files on remote
 *            drive.
 *            2. The function will try to reset the remote drive to idle
 *               state once an error occur.
 * ------------------------------------------------------------------------
 */
static  UI32_T  FS_CheckRemoteStatus(UI32_T drive)
{
    /*  static global "packet_header" is used to send check flash
        status to remote drive */
    UI32_T                  return_value;
    UI32_T                  retry_count = 0;

    FS_Packet_Header_T      *fs_rq_packet_p;
    L_MM_Mref_Handle_T      *mref_handle_p;
    UI32_T                  pdu_len;

    DBGMSG("FS_CheckRemoteStatus: Check flash programming status");
    for(;;)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Request_Packet_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_SEND, FS_FLASH_STATUS) /* user_id */
                                             );
        fs_rq_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if (NULL == fs_rq_packet_p)
        {
            DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
            return_value = FS_RETURN_ERROR;
            break;
        }

        fs_rq_packet_p->opcode     = FS_FLASH_STATUS;
        fs_rq_packet_p->data_size  = 0;

        fs_rq_packet_p->seq_no = FS_OM_GetControlSeqNo();
        DUMP_TXPKT(*fs_rq_packet_p, sizeof(*fs_rq_packet_p));

        memset(&fs_packet_buffer, 0, sizeof(fs_packet_buffer) );

        fs_packet_buffer.header.opcode = FS_ACK;/*init as success for ISC new design*/
        if (ISC_RemoteCall((UI8_T)drive, ISC_FS_SID, mref_handle_p,
                            SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                            sizeof(fs_packet_buffer), (UI8_T *) &fs_packet_buffer,
                            FS_ISC_TRY_COUNT, FS_TIMEOUT_VALUE) )
        {
            /* 1.1 packet received successfully, increase seq_no */
            FS_OM_AddControlSeqNo();

            DUMP_RXPKT(fs_packet_buffer, GET_PDU_LEN(fs_packet_buffer));

            /* 1.1 (b) NAK received */
            if( fs_packet_buffer.header.opcode == FS_NAK )
            {
                if(fs_packet_buffer.data.error_code == FS_RETURN_FLASHING)
                {
                    if(retry_count++ >= FS_COMMAND_RETRY_MORECOUNT)
                    {
                        return_value = FS_RETURN_TIMEOUT;
                        ERRMSG(FS_RETURN_TIMEOUT);
                        break;
                    }
                    else
                    {

                        DEBUGSTR("FS_CheckRemoteStatus: Wait the next 3 seconds for the unit %d to try Again...\r\n", (int)drive);
                        SYSFUN_Sleep(300);
                    }
                }
                else
                {   /* Flash failed */
                    return_value = fs_packet_buffer.data.error_code;
                    ERRMSG(return_value);
                    if (return_value == FS_RETURN_EXCEED_MAX_NUM_OF_FILES)
                    {
                        BACKDOOR_MGR_Printf("\r\nWarning!! The number of files of the given type exceeds the maximum number. Operation Inhibited.\r\n");
                    }
                    break;
                }
            }
            else /* ACK received */
            {
                return_value = FS_RETURN_OK;
                DBGMSG("FS_CheckRemoteStatus: Programming flash completed.\r\n");
                break;
            }
            SYSFUN_Sleep(FS_SLEEP_TIME);
        }
        else /* 1.1 b. ISC error */
        {
            ERRMSG(FS_RETURN_TIMEOUT);
            return_value = FS_AbortSequence(drive);
            break;
        }
    }

    DBGMSG("FS_CheckRemoteStatus: Status check finished.\r\n");
    /* 2.0 report status */
    return return_value;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetRemoteServiceCapability
 * ------------------------------------------------------------------------
 * FUNCTION : This function will get the service capability from remote
 *            unit.
 * INPUT    : drive         -- unit number or blade number
 * OUTPUT   : capability    -- number of supported service function on
 *                             remote unit.
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     :
 * ------------------------------------------------------------------------
 */
static UI32_T FS_GetRemoteServiceCapability(UI32_T drive, UI32_T *capability)
{
    FS_Capability_Packet_T  cap_packet;

    UI32_T  return_value;
    UI32_T  retry_count;

    FS_Packet_Header_T   *fs_rq_packet_p;
    L_MM_Mref_Handle_T   *mref_handle_p;
    UI32_T               pdu_len;

    /* Check if FS is in the right stacking mode. */
    if(FS_OM_GetControlStackingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        ERRMSG(FS_RETURN_NOT_MASTER);
        return FS_RETURN_NOT_MASTER;
    }


    DEBUGSTR("\r\n%s(): Remote operation for drive %d at line %d\r\n", __FUNCTION__, (int)drive, __LINE__);


    FS_LOCK_REMOTE();

    /* Send packet to remote drive. */
    retry_count = 0;
    for (;;)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Packet_Header_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_SEND, FS_GET_SERVICE_CAPABILITY) /* user_id */
                                             );
        fs_rq_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if (NULL == fs_rq_packet_p)
        {
            DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
            return_value = FS_RETURN_ERROR;
            break;
        }

        /* Fill header with appropriate data. */
        fs_rq_packet_p->opcode    = FS_GET_SERVICE_CAPABILITY;
        fs_rq_packet_p->data_size = 0;

        fs_rq_packet_p->seq_no = FS_OM_GetControlSeqNo();
        memset(&fs_packet_buffer, 0, sizeof(fs_packet_buffer));

        /* Dump the packet content if debug flag is set. */
        DUMP_TXPKT(*fs_rq_packet_p, sizeof(*fs_rq_packet_p));

        if (ISC_RemoteCall((UI8_T)drive, ISC_FS_SID, mref_handle_p,
                            SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                            sizeof(fs_packet_buffer), (UI8_T *) &fs_packet_buffer,
                            FS_ISC_TRY_COUNT, FS_TIMEOUT_VALUE) )
        {
            /* ISC returned successful, increase seq_no by 1 */
            FS_OM_AddControlSeqNo();

            DUMP_RXPKT(fs_packet_buffer, GET_PDU_LEN(fs_packet_buffer));

            /* ACK received */
            if (fs_packet_buffer.header.opcode == FS_ACK)
            {
                memcpy(&cap_packet, &fs_packet_buffer, sizeof(cap_packet));
                if (cap_packet.service_capability.magic_num == FS_CAPABILITY_MAGIC_NUM)
                {
                    *capability = cap_packet.service_capability.capability;
                }
                else
                {
                    *capability = 0;
                }
                return_value = FS_RETURN_OK;
                break;
            }
            else /* NAK Received*/
            {
                return_value = fs_packet_buffer.data.error_code;
                ERRMSG(return_value);
                FS_AbortSequence(drive);

                if(retry_count++ >= FS_COMMAND_RETRY_COUNT)
                {
                    return_value = FS_AbortSequence(drive);
                    ERRMSG(return_value);
                    break;
                }
                else
                {
                    DBGMSG("Try Again");
                    SYSFUN_Sleep(FS_SLEEP_TIME);
                    continue;
                }
            }
        }
        else /* ISC error */
        {
            ERRMSG(FS_RETURN_TIMEOUT);
            return_value = FS_AbortSequence(drive);
            break;
        }
    }/* End for loop */

    FS_UNLOCK_REMOTE();
    return return_value;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_Service_NotSupport
 * ------------------------------------------------------------------------
 * FUNCTION : This function sends a SERVICE_NOT_SUPPORT packet
              back to master.
 * INPUT    : key - key for ISC service
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void FS_Service_NotSupport(ISC_Key_T *key)
{
    L_MM_Mref_Handle_T  *rep_mref_handle_p;
    FS_Packet_Header_T  *fs_rep_packet_p;
    UI32_T              pdu_len;

    rep_mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Packet_Header_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_REPLY, FS_SERVICE_NOT_SUPPORT) /* user_id */
                                             );
    fs_rep_packet_p = L_MM_Mref_GetPdu(rep_mref_handle_p, &pdu_len);
    if (NULL == fs_rep_packet_p)
    {
        DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
        return;
    }

    memset(fs_rep_packet_p, 0, sizeof(*fs_rep_packet_p));

    fs_rep_packet_p->opcode    = FS_SERVICE_NOT_SUPPORT;
    fs_rep_packet_p->seq_no    = FS_OM_GetControlSeqNo();
    fs_rep_packet_p->data_size = 0;

    if (ISC_RemoteReply(rep_mref_handle_p, key))
    {
        FS_OM_AddControlSeqNo();
    }
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_RemoteRead
 * ------------------------------------------------------------------------
 * FUNCTION : This function handles Read Request packet received from master
 * INPUT    : key       - key for ISC service
 *            mref_handle_p - memory reference points to received packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Upon receiving a read request from master,
 *           1. slave searched for the request file,
 *           2. check the attributes,
 *           3. send first data packet, and
 *           4. enter data transfer state.
 * ------------------------------------------------------------------------
 */
static void FS_RemoteRead(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p)
{
    FS_Request_Packet_T *rx_packet;
    UI32_T              return_value;
    UI32_T              read_count;

    L_MM_Mref_Handle_T  *rep_mref_handle_p;
    FS_Data_Packet_T    *fs_data_packet_p;
    UI32_T              pdu_len;
    UI8_T               name[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];

    /* Make sure FS is in correct state */
    if(FS_OM_GetControlState() != FS_STATE_IDLE)
    {
        ERRMSG(FS_RETURN_INCORRECT_STATE);
        FS_SendNAK(key, FS_RETURN_INCORRECT_STATE);
        return;
    }

    /* FS idle, save the file information */
    rx_packet   = (FS_Request_Packet_T*)L_MM_Mref_GetPdu(mref_handle_p, &return_value); /* return_value is used as dummy here */

//    memcpy(&(fs_control.file_attr), &(rx_packet->file_attr), sizeof(FS_File_Attr_T) );
    FS_OM_SetControlAttribution(&(rx_packet->file_attr));
    FS_OM_GetControlAttrFileName(name);

    DEBUGSTR("%s() Read: %s, size: %8ld\r\n", __FUNCTION__, name, FS_OM_GetControlAttrFileSize());

    /* Allocate space to hold data */
#if (SYS_CPNT_BUFFERMGMT == TRUE)
    FS_OM_SetControlFileBuffer(BUFFER_MGR_Allocate());
#else
    FS_OM_SetControlFileBuffer(malloc(FS_OM_GetControlAttrFileSize()));
#endif
    DBGMSG("Allocate Buffer");

    if(!FS_OM_GetControlFileBuffer())
    {
        ERRMSG(FS_RETURN_NO_BUFFER);
        FS_SendNAK(key, FS_RETURN_NO_BUFFER);
        return;
    }

    /* Put the whole file in buffer */
    return_value = FS_ReadFile( DUMMY_DRIVE, name,
                                FS_OM_GetControlFileBuffer(), FS_OM_GetControlAttrFileSize(),
                                &read_count);
    if(return_value != FS_RETURN_OK)
    {
        ERRMSG(return_value);
        FS_SendNAK(key, return_value);
        return;
    }

    /* Read local file completed, Send the first data packet */
    FS_OM_SetControlFilePtr(FS_OM_GetControlFileBuffer());

    rep_mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Data_Packet_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_REPLY, FS_DATA) /* user_id */
                                             );
    fs_data_packet_p = L_MM_Mref_GetPdu(rep_mref_handle_p, &pdu_len);
    if (NULL == fs_data_packet_p)
    {
        DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
        return;
    }

    fs_data_packet_p->header.opcode = FS_DATA;
    fs_data_packet_p->header.seq_no = FS_OM_GetControlSeqNo();
    if(FS_OM_GetControlAttrFileSize() > FS_MAX_DATA_SIZE)
    {
        fs_data_packet_p->header.data_size = FS_MAX_DATA_SIZE;
        memcpy(fs_data_packet_p->raw_data, FS_OM_GetControlFileBuffer(), FS_MAX_DATA_SIZE);
        FS_OM_SetControlFilePtr(FS_OM_GetControlFileBuffer() + FS_MAX_DATA_SIZE);
        FS_OM_MinusControlAttrFileSize(FS_MAX_DATA_SIZE);
        ISC_RemoteReply(rep_mref_handle_p, key);
        FS_OM_AddControlSeqNo();
        FS_OM_SetControlState(FS_STATE_DATA_TRANSFER);
    }
    else
    {
        fs_data_packet_p->header.data_size = FS_OM_GetControlAttrFileSize();
        memcpy(fs_data_packet_p->raw_data, FS_OM_GetControlFileBuffer(), FS_OM_GetControlAttrFileSize());
        ISC_RemoteReply(rep_mref_handle_p, key);
        FS_OM_AddControlSeqNo();
#if (SYS_CPNT_BUFFERMGMT == TRUE)
        BUFFER_MGR_Free(FS_OM_GetControlFileBuffer());
#else
        free(FS_OM_GetControlFileBuffer());
#endif
        DBGMSG("Free Buffer");
        FS_OM_SetControlFileBuffer(NULL);
        FS_OM_SetControlFilePtr(NULL);
        FS_ReturnIdle();
    }
    DUMP_TXPKT(*fs_data_packet_p, GET_PDU_LEN(*fs_data_packet_p));
    return;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_RemoteWrite
 * ------------------------------------------------------------------------
 * FUNCTION : This function handles Write Request packet received from master
 * INPUT    : key       - key for ISC service
 *            mem_ref   - memory reference points to received packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Upon receiving a write request from master, slave must save
 *            the file information for later use. Slave also needs to allocate
 *            buffer to hold data.
 * ------------------------------------------------------------------------
 */
static void FS_RemoteWrite(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p)
{
    UI8_T               *rx_packet;
    FS_File_Attr_T      *rx_file_attr;
    UI32_T               pdu_len;


    rx_packet = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    /* if slave is busy, return nak to master */
    if(FS_OM_GetControlState() != FS_STATE_IDLE)
    {
        FS_SendNAK(key, FS_RETURN_INCORRECT_STATE);
        return;
    }

    /* 1.0 save the current file information */
    DBGMSG("Save File Information");
    rx_file_attr = (FS_File_Attr_T *)(rx_packet+sizeof(FS_Packet_Header_T));
//    memcpy(&(fs_control.file_attr), rx_file_attr, sizeof(FS_File_Attr_T));
    FS_OM_SetControlAttribution(rx_file_attr);

    /* 2.0 Determine what kind of operation we should perform */

    /* 2.0 a: Erase File*/
    if( FS_OM_GetControlAttrFileSize() == 0)
    {
        DBGMSG("Erase file...");
        /* Send ACK and Perform Erase operation */
        rx_packet   = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

        FS_OM_SetControlState(FS_STATE_FLASH_PROGRAMMING);
        FS_OM_SetControlFlashOperation(FS_FLASH_FILE);
        DBGMSG("Start to Program Flash, Create Task");

        if (TRUE == FS_OM_GetShutdownFlag())
        {
            FS_SendNAK(key, FS_RETURN_NOT_READY);
            return;
        }

        if (FS_OM_GetTaskId())
        {
            if ( SYSFUN_SendEvent(FS_OM_GetTaskId(), FS_TYPE_EVENT_LEGACYFS) != SYSFUN_OK
               )
            {
                DBGMSG("Remote write");
                FS_SendNAK(key, FS_RETURN_ERROR);
            }
            else
            {
                FS_SendACK(key);
            }
        } /* End of if (_fs_task_existing_) */
        return;
    }

    /* 2.0 b: Write File*/
    if( FS_OM_GetControlAttrFileSize() != 0)
    {
        DBGMSG("Write File...");
        /* allocate space for the file to write */
#if (SYS_CPNT_BUFFERMGMT == TRUE)
        FS_OM_SetControlFileBuffer(BUFFER_MGR_Allocate());
#else
        FS_OM_SetControlFileBuffer(malloc(FS_OM_GetControlAttrFileSize()));
#endif
        DBGMSG("Allocate Buffer");
        FS_OM_SetControlFilePtr(FS_OM_GetControlFileBuffer());

        if(FS_OM_GetControlAttrFileType() == FS_FILE_TYPE_TOTAL)
            FS_OM_SetControlFlashOperation(FS_FLASH_LOADER);
        else
            FS_OM_SetControlFlashOperation(FS_FLASH_FILE);

        if(!FS_OM_GetControlFileBuffer())
        {
            ERRMSG(FS_RETURN_NO_BUFFER);
            FS_SendNAK(key, FS_RETURN_NO_BUFFER);
            return;
        }
        else
        {
            FS_SendACK(key);
        }
        FS_OM_SetControlState(FS_STATE_DATA_TRANSFER);
        DBGMSG("Entering Data Loop... \\");
    }
}
/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_RemoteSetAttr
 * ------------------------------------------------------------------------
 * FUNCTION : This function handles set attribute packet received from master
 * INPUT    : key       - key for ISC service
 *            mref_handle_p - memory reference points to received packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 * ------------------------------------------------------------------------
 */
static void FS_RemoteSetAttr(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p)
{
    FS_Request_Packet_T *rx_packet;
    UI32_T               pdu_len;

    rx_packet = (FS_Request_Packet_T *) L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    if(FS_OM_GetControlState() != FS_STATE_IDLE)
    {
        FS_SendNAK(key, FS_RETURN_INCORRECT_STATE);
        return;
    }
    FS_OM_SetControlState(FS_STATE_FLASH_PROGRAMMING);
    FS_OM_SetControlFlashOperation(FS_FLASH_ATTRIBUTE);
//    memcpy(&fs_control.file_attr, &(rx_packet->file_attr), sizeof(fs_control.file_attr));
    FS_OM_SetControlAttribution(&(rx_packet->file_attr));

    DBGMSG("Start to Program Flash, Create Task");

    if (TRUE == FS_OM_GetShutdownFlag())
    {
        FS_SendNAK(key, FS_RETURN_NOT_READY);
        return;
    }

    if (FS_OM_GetTaskId())
    {
        if ( SYSFUN_SendEvent(FS_OM_GetTaskId(), FS_TYPE_EVENT_LEGACYFS) != SYSFUN_OK
           )
        {
            DBGMSG("Remote write");
            FS_SendNAK(key, FS_RETURN_ERROR);
        }
        else
        {
            FS_SendACK(key);
        }
    }
    return;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_RemoteReceiveData
 * ------------------------------------------------------------------------
 * FUNCTION : This function handles Data packets received from master
 * INPUT    : key       - key for ISC service
 *            mem_ref   - memory reference points to received packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function simply puts whatever in the data packet into
 *            file buffer.
 * ------------------------------------------------------------------------
 */
static void FS_RemoteReceiveData(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p)
{
    UI8_T               *rx_packet;
    FS_Packet_Header_T  *rx_header;
    UI8_T               *rx_data;
    UI8_T               sum;
    UI32_T              pdu_len;

    rx_packet   = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    rx_header   = (FS_Packet_Header_T *) rx_packet;
    rx_data     = (UI8_T *) (rx_packet + sizeof(FS_Packet_Header_T));

    //DUMP_RXPKT(*rx_packet,sizeof(FS_Packet_Header_T)+rx_header->data_size);

    /* 1.0 make sure FS is in correct state */
    if(FS_OM_GetControlState() != FS_STATE_DATA_TRANSFER)
    {
        ERRMSG(FS_RETURN_INCORRECT_STATE);
        FS_SendNAK(key, FS_RETURN_INCORRECT_STATE);
        return;
    }

    /* 2.0 put data in data buffer */
    memcpy(FS_OM_GetControlFilePtr(), rx_data, rx_header->data_size);
    FS_OM_AddControlFilePtr(rx_header->data_size);

    /* if EOF reached */
    if(rx_header->data_size < FS_MAX_DATA_SIZE)
    {
        DBGMSG("File transimission completed");
        FS_OM_SetControlState(FS_STATE_DATA_COMPLETE);
        /* check file error */
        sum = FS_ByteSum(FS_OM_GetControlFileBuffer(), FS_OM_GetControlAttrFileSize());

        if(sum != FS_OM_GetControlAttrCheckSum())
        {
            ERRMSG(FS_RETURN_ACTION_INHIBITED);
            FS_SendNAK(key, FS_RETURN_ACTION_INHIBITED);
            return;
        }
    }
    /* Send ACK if there is no mistake */
    FS_SendACK(key);
}
/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_RemoteSendData
 * ------------------------------------------------------------------------
 * FUNCTION : This function handles ACK packets received from master
 * INPUT    : key       - key for ISC service
 *            mem_ref   - memory reference points to received packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : The onlys situation that the slave will receive an ACK from
 *            a master is that the master wants to read a file from slave.
 * ------------------------------------------------------------------------
 */
static void FS_RemoteSendData(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p)
{
    UI32_T               pdu_len;

    L_MM_Mref_Handle_T  *rep_mref_handle_p;
    FS_Data_Packet_T    *fs_data_packet_p;

    if(FS_OM_GetControlState() != FS_STATE_DATA_TRANSFER)
    {
        ERRMSG(FS_RETURN_INCORRECT_STATE);
        FS_SendNAK(key, FS_RETURN_INCORRECT_STATE);
    }
    else
    {
        rep_mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Data_Packet_T), /* tx_buffer_size */
                                                  L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_REPLY, FS_DATA) /* user_id */
                                                 );
        fs_data_packet_p = L_MM_Mref_GetPdu(rep_mref_handle_p, &pdu_len);
        if (NULL == fs_data_packet_p)
        {
            DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
            return;
        }
        fs_data_packet_p->header.opcode = FS_DATA;
        fs_data_packet_p->header.seq_no = FS_OM_GetControlSeqNo();
        /* Calculate remaining data size */
        if(FS_OM_GetControlAttrFileSize() > FS_MAX_DATA_SIZE)
        {
            fs_data_packet_p->header.data_size = FS_MAX_DATA_SIZE;
            memcpy(fs_data_packet_p->raw_data, FS_OM_GetControlFilePtr(), FS_MAX_DATA_SIZE);
            FS_OM_AddControlFilePtr(FS_MAX_DATA_SIZE);
//            fs_control.file_attr.file_size -= FS_MAX_DATA_SIZE;
            FS_OM_MinusControlAttrFileSize(FS_MAX_DATA_SIZE);
            ISC_RemoteReply(rep_mref_handle_p, key);
            FS_OM_AddControlSeqNo();
        }
        else
        {
            fs_data_packet_p->header.data_size = FS_OM_GetControlAttrFileSize();
            memcpy(fs_data_packet_p->raw_data, FS_OM_GetControlFilePtr(), FS_OM_GetControlAttrFileSize());
            ISC_RemoteReply(rep_mref_handle_p, key);
            FS_OM_AddControlSeqNo();

            /* Free the space to hold the file data */
            if(FS_OM_GetControlFileBuffer() != (void *) 0)
            {
#if (SYS_CPNT_BUFFERMGMT == TRUE)
                BUFFER_MGR_Free(FS_OM_GetControlFileBuffer());
#else
                free(FS_OM_GetControlFileBuffer());
#endif
                DBGMSG("Free Buffer");
                FS_OM_SetControlFilePtr(NULL);
                FS_OM_SetControlFileBuffer(NULL);
            }

            FS_ReturnIdle();

        }
        DUMP_TXPKT(*fs_data_packet_p, GET_PDU_LEN(*fs_data_packet_p));
    }
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_RemoteFlashFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function handles flash programming command sent from
 *            master.
 * INPUT    : key       - key for ISC service
 *            mem_ref   - memory reference points to received packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function will create a task that writes the file
 *            in the fs control block onto the flash device.
 * ------------------------------------------------------------------------
 */
static void FS_RemoteFlashFile(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p)
{
    UI32_T               pdu_len;


    if(FS_OM_GetControlState() != FS_STATE_DATA_COMPLETE)
    {
        FS_SendNAK(key, FS_RETURN_INCORRECT_STATE);
        return;
    }
    FS_OM_SetControlState(FS_STATE_FLASH_PROGRAMMING);
    DBGMSG("Start to Program Flash, Create Task");

    if (TRUE == FS_OM_GetShutdownFlag())
    {
        FS_SendNAK(key, FS_RETURN_NOT_READY);
        return;
    }

    if (FS_OM_GetTaskId())
    {
        if ( SYSFUN_SendEvent(FS_OM_GetTaskId(), FS_TYPE_EVENT_LEGACYFS) != SYSFUN_OK
           )
        {
            DBGMSG("Remote write");
            FS_SendNAK(key, FS_RETURN_ERROR);
        }
        else
        {
            FS_SendACK(key);
        }
    }
    return;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_RemoteCheckStatus
 * ------------------------------------------------------------------------
 * FUNCTION : This function checks the flash programming status of the
 *            slave drive
 * INPUT    : key       - key for ISC service
 *            mem_ref   - memory reference points to received packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void FS_RemoteCheckStatus(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p)
{
    FS_LOCK_REMOTE();
    if( FS_OM_GetControlState() == FS_STATE_FLASH_PROGRAMMING)
    {   /* Flash programming is still in progress */
        FS_SendNAK(key, FS_RETURN_FLASHING);
    }
    else if(FS_OM_GetControlState() == FS_STATE_IDLE)
    {   /* flash programming finished */
        if(FS_OM_GetControlFlashResult() == FS_RETURN_OK)
        {
            FS_SendACK(key); /* Success */
        }
        else
        {/* flash programming failed */
            FS_SendNAK(key, FS_OM_GetControlFlashResult());
        }
        FS_OM_SetControlFlashResult(FS_RETURN_OK);
    }
    else
    {
        FS_SendNAK(key, FS_RETURN_INCORRECT_STATE);
    }
    FS_UNLOCK_REMOTE();
    return;
}


/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_RemoteGetFileSystemSummary
 * ------------------------------------------------------------------------
 * FUNCTION : This function get file system summary of local drive
 * INPUT    : key       - key for ISC service
 *            mem_ref   - memory reference points to received packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void FS_RemoteGetFileSystemSummary(ISC_Key_T * key, L_MM_Mref_Handle_T *mref_handle_p)
{
    FS_File_Type_T           file_type;
    L_MM_Mref_Handle_T       *rep_mref_handle_p;
    FS_File_Summary_Packet_T *fs_sum_packet_p;
    UI32_T                   pdu_len;

    /* fs is not in correct state */
    if(FS_OM_GetControlState() != FS_STATE_IDLE)
    {
        FS_SendNAK(key,FS_RETURN_INCORRECT_STATE);
        return;
    }

    rep_mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_File_Summary_Packet_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_REPLY, FS_ACK) /* user_id */
                                             );
    fs_sum_packet_p = L_MM_Mref_GetPdu(rep_mref_handle_p, &pdu_len);
    if (NULL == fs_sum_packet_p)
    {
        DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
        return;
    }

    memset(fs_sum_packet_p, 0, sizeof(*fs_sum_packet_p));

    fs_sum_packet_p->header.opcode = FS_ACK;
    fs_sum_packet_p->header.seq_no = FS_OM_GetControlSeqNo();
    fs_sum_packet_p->header.data_size = sizeof(FS_File_Summary_Packet_T) - sizeof(FS_Packet_Header_T);

    for(file_type=0;file_type<FS_FILE_TYPE_TOTAL;file_type++)
    {
        fs_sum_packet_p->num_of_file[file_type] = FS_NumOfFile(file_type);
        fs_sum_packet_p->total_num_of_file += fs_sum_packet_p->num_of_file[file_type];
    }

    FS_GetStorageFreeSpace(DUMMY_DRIVE, &(fs_sum_packet_p->total_space_free));

    DUMP_TXPKT(*fs_sum_packet_p, GET_PDU_LEN(*fs_sum_packet_p));

    if( ISC_RemoteReply(rep_mref_handle_p, key))
    {
        FS_OM_AddControlSeqNo();
    }

    FS_ReturnIdle();
    return;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_RemoteGetStorageFreeSpaceForUpdateRuntime
 * ------------------------------------------------------------------------
 * FUNCTION : Get the available free space of the local drive when it is used
 *            to consider the free space for runtime file update.
 * INPUT    : key           - key for ISC service
 *            mref_handle_p - memory reference points to received packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void FS_RemoteGetStorageFreeSpaceForUpdateRuntime(ISC_Key_T * key, L_MM_Mref_Handle_T *mref_handle_p)
{
//    FS_File_Type_T           file_type;
    L_MM_Mref_Handle_T       *rep_mref_handle_p;
    FS_File_Summary_Packet_T *fs_sum_packet_p;
    UI32_T                   pdu_len;

    /* fs is not in correct state */
    if(FS_OM_GetControlState() != FS_STATE_IDLE)
    {
        FS_SendNAK(key,FS_RETURN_INCORRECT_STATE);
        return;
    }

    rep_mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_File_Summary_Packet_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_REPLY, FS_ACK) /* user_id */
                                             );
    fs_sum_packet_p = L_MM_Mref_GetPdu(rep_mref_handle_p, &pdu_len);
    if (NULL == fs_sum_packet_p)
    {
        DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
        return;
    }

    memset(fs_sum_packet_p, 0, sizeof(*fs_sum_packet_p));

    fs_sum_packet_p->header.opcode = FS_ACK;
    fs_sum_packet_p->header.seq_no = FS_OM_GetControlSeqNo();
    fs_sum_packet_p->header.data_size = sizeof(FS_File_Summary_Packet_T) - sizeof(FS_Packet_Header_T);

    FS_GetStorageFreeSpaceForUpdateRuntime(DUMMY_DRIVE, &(fs_sum_packet_p->total_space_free));

    DUMP_TXPKT(*fs_sum_packet_p, GET_PDU_LEN(*fs_sum_packet_p));

    if( ISC_RemoteReply(rep_mref_handle_p, key))
    {
        FS_OM_AddControlSeqNo();
    }

    FS_ReturnIdle();
    return;

}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_RemoteGetMultipleFileInfo
 * ------------------------------------------------------------------------
 * FUNCTION : This function get multiple file attrs of local drive
 * INPUT    : key       - key for ISC service
 *            mem_ref   - memory reference points to received packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void FS_RemoteGetMultipleFileInfo(ISC_Key_T * key, L_MM_Mref_Handle_T * mref_handle_p)
{
    FS_File_Attr_T          file_attr;
    UI32_T                  drive;
    FS_Request_Packet_T     *fs_req_packet;
    UI32_T                  expected_data_size, data_size;
    UI32_T                  return_value;

    L_MM_Mref_Handle_T      *rep_mref_handle_p;
    FS_File_Attr_Packet_T   *fs_attr_packet_p;
    UI32_T                  pdu_len;

    /* fs is not in correct state */
    if(FS_OM_GetControlState() != FS_STATE_IDLE)
    {
        FS_SendNAK(key,FS_RETURN_INCORRECT_STATE);
        return;
    }

    fs_req_packet = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    memcpy(&file_attr, &(fs_req_packet->file_attr), sizeof(FS_File_Attr_T));

    drive = FS_OM_GetControlDriverId();
    if (fs_req_packet->file_attr.file_name[0] == '\0') /* means get the first file */
    {
        fs_req_packet->next = TRUE;
    }

    /* calculate maximal possible ISC payload size of this FS request
     */
    expected_data_size = sizeof(FS_Packet_Header_T) + sizeof(UI16_T) + sizeof(FS_File_Attr_T)*fs_req_packet->max_count;
    rep_mref_handle_p = L_MM_AllocateTxBuffer(expected_data_size, /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_REPLY, FS_ACK) /* user_id */
                                             );
    fs_attr_packet_p = L_MM_Mref_GetPdu(rep_mref_handle_p, &pdu_len);
    if (NULL == fs_attr_packet_p)
    {
        DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
        return;
    }

    fs_attr_packet_p->header.opcode = FS_ACK;
    fs_attr_packet_p->header.seq_no = FS_OM_GetControlSeqNo();
    fs_attr_packet_p->file_count = 0;

    if (FALSE == fs_req_packet->next)
    {
        if ((return_value = FS_GetFileInfo(drive, &file_attr)) == FS_RETURN_OK)
        {
            memcpy(&(fs_attr_packet_p->file_attr[fs_attr_packet_p->file_count]),
                   &file_attr, sizeof(file_attr));
            fs_attr_packet_p->file_count++;
        }
        else
        {
            FS_SendNAK(key, return_value);
            FS_ReturnIdle();
            return;
        }
    }

    while (fs_attr_packet_p->file_count < (fs_req_packet->max_count))
    {
        if(FS_GetNextFileInfo(&drive, &file_attr) == FS_RETURN_OK)
        {
            /* Get storage size */
            FS_GetFileStorageUsageSize(drive, file_attr.file_name, &(file_attr.storage_size) );
            memcpy(&(fs_attr_packet_p->file_attr[fs_attr_packet_p->file_count]),
                   &file_attr, sizeof(file_attr));
            fs_attr_packet_p->file_count++;
        }
        else
        {
            break;
        }
    }

    if (fs_attr_packet_p->file_count == fs_req_packet->max_count)
    {
        if(FS_GetNextFileInfo(&drive, &file_attr) == FS_RETURN_OK)
        {
            fs_attr_packet_p->header.opcode = FS_HAVEMOREDATA;
        }
    }

    DUMP_TXPKT(*fs_attr_packet_p, GET_PDU_LEN(*fs_attr_packet_p));

    data_size = sizeof(UI16_T) + sizeof(FS_File_Attr_T)*fs_attr_packet_p->file_count;
    fs_attr_packet_p->header.data_size = data_size;

    L_MM_Mref_SetPduLen(rep_mref_handle_p, sizeof(FS_Packet_Header_T) + data_size);

    if( ISC_RemoteReply(rep_mref_handle_p, key) )
    {
        FS_OM_AddControlSeqNo();
    }

    FS_ReturnIdle();
    return;
}


/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_RemoteGetStartupFilename
 * ------------------------------------------------------------------------
 * FUNCTION : This function get the startup file name of target file type in
 *            local drive
 * INPUT    : key       - key for ISC service
 *            mem_ref   - memory reference points to received packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void FS_RemoteGetStartupFilename(ISC_Key_T * key, L_MM_Mref_Handle_T * mref_handle_p)
{
    FS_Request_Packet_T     *fs_req_packet;
    UI32_T                  data_size;
    UI32_T                  return_value;

    L_MM_Mref_Handle_T      *rep_mref_handle_p;
    FS_File_Attr_Packet_T   *fs_attr_packet_p;
    UI32_T                  pdu_len;


    /* fs is not in correct state */
    if(FS_OM_GetControlState() != FS_STATE_IDLE)
    {
        FS_SendNAK(key,FS_RETURN_INCORRECT_STATE);
        return;
    }

    fs_req_packet = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    data_size = sizeof(UI16_T) + sizeof(FS_File_Attr_T);
    rep_mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Packet_Header_T) + data_size, /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_REPLY, FS_ACK) /* user_id */
                                             );
    fs_attr_packet_p = L_MM_Mref_GetPdu(rep_mref_handle_p, &pdu_len);
    if (NULL == fs_attr_packet_p)
    {
        DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
        return;
    }

    fs_attr_packet_p->header.opcode = FS_ACK;
    fs_attr_packet_p->header.seq_no = FS_OM_GetControlSeqNo();
    fs_attr_packet_p->file_count = 0;

    return_value = FS_GetStartupFilename(FS_OM_GetControlDriverId(), fs_req_packet->file_attr.file_type, fs_attr_packet_p->file_attr[0].file_name);

    if (FS_RETURN_OK != return_value && FS_RETURN_FILE_NEX != return_value)
    {
        L_MM_Mref_Release(&rep_mref_handle_p);
        FS_SendNAK(key, return_value);
        return;
    }

    if (FS_RETURN_OK == return_value)
    {
        fs_attr_packet_p->file_count = 1;
    }

    DUMP_TXPKT(*fs_attr_packet_p, GET_PDU_LEN(*fs_attr_packet_p));

    fs_attr_packet_p->header.data_size = data_size;

    if( ISC_RemoteReply(rep_mref_handle_p, key))
    {
        FS_OM_AddControlSeqNo();
    }

    FS_ReturnIdle();
    return;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_RemoteGetHWInfo
 * ------------------------------------------------------------------------
 * FUNCTION : This function checks the flash programming status of the
 *            slave drive
 * INPUT    : key       - key for ISC service
 *            mem_ref   - memory reference points to received packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void FS_RemoteGetHWInfo(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p)
{
    L_MM_Mref_Handle_T      *rep_mref_handle_p;
    FS_HW_Info_Packet_T     *fs_info_packet_p;
    UI32_T                  pdu_len;

    if(FS_OM_GetControlState() != FS_STATE_IDLE)
    {
        FS_SendNAK(key, FS_RETURN_INCORRECT_STATE);
        return;
    }

    rep_mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_HW_Info_Packet_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_REPLY, FS_ACK) /* user_id */
                                             );
    fs_info_packet_p = L_MM_Mref_GetPdu(rep_mref_handle_p, &pdu_len);
    if (NULL == fs_info_packet_p)
    {
        DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
        return;
    }

    fs_info_packet_p->header.opcode = FS_ACK;
    fs_info_packet_p->header.seq_no = FS_OM_GetControlSeqNo();
    fs_info_packet_p->header.data_size = sizeof(fs_info_packet_p->hw_info);
    FS_ReadHardwareInfo(DUMMY_DRIVE, &(fs_info_packet_p->hw_info));
    if( ISC_RemoteReply(rep_mref_handle_p, key))
    {
        FS_OM_AddControlSeqNo();
    }
    FS_ReturnIdle();
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_RemoteAbort
 * ------------------------------------------------------------------------
 * FUNCTION : This function checks the flash programming status of the
 *            slave drive
 * INPUT    : key       - key for ISC service
 *            mem_ref   - memory reference points to received packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void FS_RemoteAbort(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p)
{
    if(FS_OM_GetControlState() != FS_STATE_IDLE)
    {
        if(FS_OM_GetControlState() == FS_STATE_FLASH_PROGRAMMING)
        {
            FS_SendNAK(key, FS_RETURN_FLASHING);
            return;
        }
        else
        {
            if(FS_OM_GetControlFileBuffer())
            {
#if (SYS_CPNT_BUFFERMGMT == TRUE)
                BUFFER_MGR_Free(FS_OM_GetControlFileBuffer());
#else
                free(FS_OM_GetControlFileBuffer());
#endif
                DBGMSG("Free Buffer");
                FS_OM_SetControlFilePtr(NULL);
                FS_OM_SetControlFileBuffer(NULL);
            }
            FS_ReturnIdle();
        }
    }
    FS_SendACK(key);
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_RemoteFlashTaskMain
 * ------------------------------------------------------------------------
 * FUNCTION : The main function of the flash programming task. It sets the
 *            FS back to idle state after flash programming is complete.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function actually performs three actions.
 *          1. Write file
 *          2. Write loader : Filename[0] = 0;
 *          3. Erase file: file size = 0;
 * ------------------------------------------------------------------------
 */
void FS_RemoteFlashTaskMain(void)
{
    UI32_T  result = FS_RETURN_ERROR;
    UI8_T   name[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];
#if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE)
    FS_PARTITION_TABLE_T *flash_entry_p = NULL;
#else
    UI32_T hw_info_mtd_id;
#endif

    switch(FS_OM_GetControlFlashOperation())
    {
    case FS_FLASH_LOADER:
        DBGMSG("Writing Loader...");
        FS_LOCK( );
#if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE)
        while (NULL != (flash_entry_p = FS_GetNextFlashPartitionEntry(flash_entry_p)))
        {
            /* write on the first found partition
             */
           if( flash_entry_p->type == TYPE_HWINFO)
           {
               break;
           }
        }
#endif
/*
        fs_control.flash_result = FS_WriteSpecialBlock(
                                    FS_SPECIAL_BLOCK_LOADER,
                                    fs_control.file_buffer,
                                    fs_control.file_attr.file_size);
*/

#if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE)
        if(flash_entry_p!=NULL)
        {
            result = FS_WRITE_HWINFO_TO_FLASH(
                                FS_OM_GetControlFileBuffer(),
                                FS_OM_GetControlAttrFileSize(),
                                flash_entry_p->mtd_num, TRUE);
        }
#else
        if(FS_GetMtdDevIdByMtdType(FS_MTD_PART_HW_INFO, &hw_info_mtd_id)==TRUE)
        {
            result=FS_WRITE_HWINFO_TO_FLASH(
                                FS_OM_GetControlFileBuffer(),
                                FS_OM_GetControlAttrFileSize(),
                                hw_info_mtd_id, TRUE);
        }
        else
        {
            result=FS_RETURN_BAD_HW;
        }

#endif /*End #if (SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD==FALSE)*/

        FS_UNLOCK();
        if(result == FS_RETURN_OK)
        {
            DBGMSG("Operation Completed Successfully");
        }
        else
        {
            ERRMSG(result);
        }
#if (SYS_CPNT_BUFFERMGMT == TRUE)
        BUFFER_MGR_Free(FS_OM_GetControlFileBuffer());
#else
        free(FS_OM_GetControlFileBuffer());
#endif
        DBGMSG("Free Buffer");
        FS_OM_SetControlFilePtr(0);
        FS_OM_SetControlFileBuffer(0);
        break;
    case FS_FLASH_ATTRIBUTE:
        FS_OM_GetControlAttrFileName(name);
        if(name[0] != 0)
        {   /* Update Startup Attributes */
            DBGMSG("Updating Startup Attributes");
            FS_LOCK( );
            result = FS_UpdateStartupAttribute(
                                        FS_OM_GetControlAttrFileType(),
                                        name,
                                        FS_OM_GetControlAttrPrivilage());
            FS_UNLOCK();
            if(result == FS_RETURN_OK)
            {
                DBGMSG("Operation Completed Successfully");
            }
            else
            {
                ERRMSG(result);
            }
        }
        else
        {
            DBGMSG("Resetting Startup Attributes");
            FS_LOCK( );
            result = FS_UpdateStartupAttribute(
                                        FS_OM_GetControlAttrFileType(),
                                        (UI8_T*)"",
                                        FS_OM_GetControlAttrPrivilage());
            FS_UNLOCK();
            if(result == FS_RETURN_OK)
            {
                DBGMSG("Operation Completed Successfully");
            }
            else
            {
                ERRMSG(result);
            }
        }
        break;
    default:
        if(FS_OM_GetControlAttrFileSize() != 0)
        {   /* Write File */
            FS_File_Attr_T file_attr;
            DBGMSG("Writing File...");

            FS_OM_GetControlAttribution(&file_attr);
            FS_LOCK( );
            result      = FS_UpdateFile( DUMMY_DRIVE,
                                        file_attr.file_name,
                                        file_attr.file_comment,
                                        file_attr.file_type,
                                        FS_OM_GetControlFileBuffer(),
                                        file_attr.file_size,
                                        file_attr.reserv_size,
                                        file_attr.privilage );
            FS_UNLOCK();
            if(result == FS_RETURN_OK)
            {
                DBGMSG("Operation Completed Successfully");
            }
            else
            {
                ERRMSG(result);
            }
#if (SYS_CPNT_BUFFERMGMT == TRUE)
            BUFFER_MGR_Free(FS_OM_GetControlFileBuffer());
#else
            free(FS_OM_GetControlFileBuffer());
#endif
            DBGMSG("Free Buffer");
            FS_OM_SetControlFilePtr(0);
            FS_OM_SetControlFileBuffer(0);
        }
        else
        {   /* Erase File */
            DBGMSG("Erasing File...");
            FS_LOCK( );
            FS_OM_GetControlAttrFileName(name);
            result      = FS_EraseFile( DUMMY_DRIVE,
                                        name,
                                        FS_OM_GetControlAttrPrivilage());
            FS_UNLOCK();
            if(result == FS_RETURN_OK)
            {
                DBGMSG("Operation Completed Successfully");
            }
            else
            {
                ERRMSG(result);
            }
        }
        break;
    }
    FS_OM_SetControlFlashResult(result);
    FS_OM_SetControlFileBuffer(0);
    FS_ReturnIdle();

    return;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_RemoteGetServiceCapability
 * ------------------------------------------------------------------------
 * FUNCTION : This function handles the request packet of getting service
 *            capability received from master.
 * INPUT    : key           - key for ISC service
 *            mref_handle_p - memory reference points to received packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void FS_RemoteGetServiceCapability(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p)
{
    L_MM_Mref_Handle_T      *rep_mref_handle_p;
    FS_Capability_Packet_T  *fs_cap_packet_p;
    UI32_T                  pdu_len;

    if(FS_OM_GetControlState() != FS_STATE_IDLE)
    {
        FS_SendNAK(key, FS_RETURN_INCORRECT_STATE);
        return;
    }

    rep_mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Capability_Packet_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_REPLY, FS_ACK) /* user_id */
                                             );
    fs_cap_packet_p = L_MM_Mref_GetPdu(rep_mref_handle_p, &pdu_len);
    if (NULL == fs_cap_packet_p)
    {
        DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
        return;
    }

    fs_cap_packet_p->header.opcode      = FS_ACK;
    fs_cap_packet_p->header.seq_no      = FS_OM_GetControlSeqNo();
    fs_cap_packet_p->header.data_size   = sizeof(fs_cap_packet_p->service_capability);

    fs_cap_packet_p->service_capability.magic_num  = FS_CAPABILITY_MAGIC_NUM;
    fs_cap_packet_p->service_capability.capability = FS_TOTAL_REMOTE_SERVICES;

    if( ISC_RemoteReply(rep_mref_handle_p, key))
    {
        FS_OM_AddControlSeqNo();
    }
    FS_ReturnIdle();
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_RemoteCopyFileContent
 * ------------------------------------------------------------------------
 * FUNCTION : This function handles the request packet of coping the
 *            contents of a file received from master.
 * INPUT    : key           - key for ISC service
 *            mref_handle_p - memory reference points to received packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Upon receiving this request packet from master,
 *            1. slave searched for the request file,
 *            2. check the attributes,
 *            3. send first data packet, and
 *            4. enter data transfer state.
 * ------------------------------------------------------------------------
 */
static void FS_RemoteCopyFileContent(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p)
{
    FS_Copy_Content_Request_Packet_T *rx_packet;
    UI32_T                           return_value;
    UI32_T                           offset;
    UI32_T                           byte_num;

    L_MM_Mref_Handle_T      *rep_mref_handle_p;
    FS_Data_Packet_T        *fs_data_packet_p;
    UI32_T                  pdu_len;
    UI8_T                   name[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];

    /* Make sure FS is in correct state */
    if (FS_OM_GetControlState() != FS_STATE_IDLE)
    {
        ERRMSG(FS_RETURN_INCORRECT_STATE);
        FS_SendNAK(key, FS_RETURN_INCORRECT_STATE);
        return;
    }

    /* FS idle, save the file information */
    rx_packet = (FS_Copy_Content_Request_Packet_T *) L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    FS_OM_SetControlAttribution(&(rx_packet->file_attr));
    FS_OM_GetControlAttrFileName(name);

    offset   = rx_packet->offset;
    byte_num = rx_packet->byte_num;


    DEBUGSTR("%s() Read: %s, size: %8ld\r\n", __FUNCTION__, name, FS_OM_GetControlAttrFileSize());


    /* Allocate space to hold data */
#if (SYS_CPNT_BUFFERMGMT == TRUE)
    FS_OM_SetControlFileBuffer(BUFFER_MGR_Allocate());
#else
    FS_OM_SetControlFileBuffer(malloc(byte_num));
#endif
    DBGMSG("Allocate Buffer");

    if (!FS_OM_GetControlFileBuffer())
    {
        ERRMSG(FS_RETURN_NO_BUFFER);
        FS_SendNAK(key, FS_RETURN_NO_BUFFER);
        return;
    }

    /* Put the whole file in buffer */
    return_value = FS_CopyFileContent(DUMMY_DRIVE, name,
                                      offset, FS_OM_GetControlFileBuffer(), byte_num);
    if (return_value != FS_RETURN_OK)
    {
        ERRMSG(return_value);
        FS_SendNAK(key, return_value);
        return;
    }

    /* Read local file completed, Send the first data packet */
    FS_OM_SetControlFilePtr(FS_OM_GetControlFileBuffer());

    rep_mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Data_Packet_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_REPLY, FS_DATA) /* user_id */
                                             );
    fs_data_packet_p = L_MM_Mref_GetPdu(rep_mref_handle_p, &pdu_len);
    if (NULL == fs_data_packet_p)
    {
        DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
        return;
    }

    fs_data_packet_p->header.opcode = FS_DATA;
    fs_data_packet_p->header.seq_no = FS_OM_GetControlSeqNo();
    if (byte_num > FS_MAX_DATA_SIZE)
    {
        fs_data_packet_p->header.data_size = FS_MAX_DATA_SIZE;
        memcpy(fs_data_packet_p->raw_data, FS_OM_GetControlFileBuffer(), FS_MAX_DATA_SIZE);
        FS_OM_SetControlFilePtr(FS_OM_GetControlFileBuffer() + FS_MAX_DATA_SIZE);
        byte_num -= FS_MAX_DATA_SIZE;
        ISC_RemoteReply(rep_mref_handle_p, key);
        FS_OM_AddControlSeqNo();
        FS_OM_SetControlState(FS_STATE_DATA_TRANSFER);
    }
    else
    {
        fs_data_packet_p->header.data_size = byte_num;
        memcpy(fs_data_packet_p->raw_data, FS_OM_GetControlFileBuffer(), byte_num);
        ISC_RemoteReply(rep_mref_handle_p, key);
        FS_OM_AddControlSeqNo();
#if (SYS_CPNT_BUFFERMGMT == TRUE)
        BUFFER_MGR_Free(FS_OM_GetControlFileBuffer());
#else
        free(FS_OM_GetControlFileBuffer());
#endif
        DBGMSG("Free Buffer");
        FS_OM_SetControlFileBuffer(NULL);
        FS_OM_SetControlFilePtr(NULL);
        FS_ReturnIdle();
    }
    DUMP_TXPKT(*fs_data_packet_p, GET_PDU_LEN(*fs_data_packet_p));
    return;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_LocalCheckFileSystem
 * ------------------------------------------------------------------------
 * FUNCTION : This function will check the file system to remove the
 *            destroyed files.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void FS_LocalCheckFileSystem(void)
{
#if 0
    FS_FileHeader_T *this_header_ptr;
    BOOL_T          file_error, stop;
    UI8_T           sum = 0;
    UI32_T          block_id;

    this_header_ptr = FS_OM_GetFileHeaderList();
    stop            = FALSE;
    FS_LOCK( );
    while ( (!stop) && (this_header_ptr != NULL) )
    {
        FS_FileBlockInfo_T  *this_file_block_ptr;
        UI8_T               *file_name;

        file_name   = this_header_ptr->file_name;
        if (FS_Debug(FS_DEBUG_FLAG_DBGMSG))
            BACKDOOR_MGR_Printf("%s: Check File %s\r\n", __FUNCTION__, file_name);

        this_file_block_ptr = this_header_ptr->file_block_list;
        file_error  = FALSE;
        while (     (!file_error)
                &&  (this_file_block_ptr != NULL)
              )
        {
            block_id    = this_file_block_ptr->block_id;
            if (    (block_id == 0)
                ||  (block_id > FS_Info.total_num_of_block)
               )
            {
                /* Error in the file header list */
                sum = 1;
            }
            else
            {
                /* Perform the file checksum procedure */
                FS_CalculateBlockByteSum(0, block_id, &sum);
            }
            if (sum)
            {
                /* Check sum error!! */
                file_error  = TRUE;
            }
            else
            {
                this_file_block_ptr = this_file_block_ptr->next;
            }
        }

        this_header_ptr = this_header_ptr->next;
        if (file_error)
        {
            /* Remove this file from the file header list */
            FS_EraseFile(DUMMY_DRIVE, (I8_T*)file_name, TRUE);
            if (FS_Debug(FS_DEBUG_FLAG_DBGMSG))
                BACKDOOR_MGR_Printf("%s: File %s check sum error!! Removed.\r\n", __FUNCTION__, file_name);
        }

    } /* End of while (all_files_in_FileHeaderList) */
    FS_UNLOCK();
#endif

    FS_OM_SetFileChecksum(TRUE);

    return;
} /* End of FS_LocalCheckFileSystem */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_ReturnIdle
 * ------------------------------------------------------------------------
 * FUNCTION : This function returns master FS back to idle state.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void FS_ReturnIdle(void)
{
    FS_LOCK_REMOTE();
    DBGMSG("Return Idle");

    /* Clear the fs_control structure */
    FS_OM_ClearControlAttribution();
    FS_OM_SetControlState(FS_STATE_IDLE);

    FS_UNLOCK_REMOTE();
    return;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_SendACK
 * ------------------------------------------------------------------------
 * FUNCTION : This function sends a pure ACK back to master
 * INPUT    : key       - key for ISC service
 *            mem_ref   - memory reference points to received packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function is not suitable for either FS_GETFILEINFO or
 *            FS_GET_HARDWARE_INFO command. ACK's for boths command should
 *            contain further information
 * ------------------------------------------------------------------------
 */
static void FS_SendACK(ISC_Key_T *key)
{
    L_MM_Mref_Handle_T   *rep_mref_handle_p;
    FS_NAK_Packet_T      *fs_nak_packet_p;
    UI32_T               pdu_len;

    rep_mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_NAK_Packet_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_REPLY, FS_ACK) /* user_id */
                                             );
    fs_nak_packet_p = L_MM_Mref_GetPdu(rep_mref_handle_p, &pdu_len);
    if (NULL == fs_nak_packet_p)
    {
        DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
        return;
    }

    fs_nak_packet_p->header.opcode       = FS_ACK;
    fs_nak_packet_p->header.seq_no       = FS_OM_GetControlSeqNo();
    fs_nak_packet_p->header.data_size    = sizeof(fs_nak_packet_p->error_code);
    fs_nak_packet_p->error_code          = FS_RETURN_OK;

    if( ISC_RemoteReply(rep_mref_handle_p, key))
    {
        FS_OM_AddControlSeqNo();
    }
    DUMP_TXPKT(*fs_nak_packet_p, GET_PDU_LEN(*fs_nak_packet_p));
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_SendNAK
 * ------------------------------------------------------------------------
 * FUNCTION : This function sends a pure ACK back to master
 * INPUT    : key       - key for ISC service
 *            mem_ref   - memory reference points to received packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void FS_SendNAK(ISC_Key_T *key, UI32_T error_code)
{
    L_MM_Mref_Handle_T   *rep_mref_handle_p;
    FS_NAK_Packet_T      *fs_nak_packet_p;
    UI32_T               pdu_len;

    rep_mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_NAK_Packet_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_REPLY, FS_ACK) /* user_id */
                                             );
    fs_nak_packet_p = L_MM_Mref_GetPdu(rep_mref_handle_p, &pdu_len);
    if (NULL == fs_nak_packet_p)
    {
        DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
        return;
    }

    fs_nak_packet_p->header.opcode    = FS_NAK;
    fs_nak_packet_p->header.seq_no    = FS_OM_GetControlSeqNo();
    fs_nak_packet_p->header.data_size = sizeof(fs_nak_packet_p->error_code);
    fs_nak_packet_p->error_code       = error_code;

    if (ISC_RemoteReply(rep_mref_handle_p, key))
    {
        FS_OM_AddControlSeqNo();
    }
    DUMP_TXPKT(*fs_nak_packet_p, GET_PDU_LEN(*fs_nak_packet_p));
}

static void FS_PrintHexChar(UI8_T *string, UI16_T char_count)
{
    UI8_T index;
    BACKDOOR_MGR_Printf(" ");
    for(index = 0; index < char_count; index++)
        BACKDOOR_MGR_Printf("%02X ", string[index]);

    for(; index < FS_WORD_WRAP; index++)
            BACKDOOR_MGR_Printf("%s", "   ");

    BACKDOOR_MGR_Printf("| ");
    for(index = 0; index < char_count; index++)
    {
        if(string[index] < ' ' || string[index] > '~')
            BACKDOOR_MGR_Printf(".");
        else
            BACKDOOR_MGR_Printf("%c", string[index]);
    }
    BACKDOOR_MGR_Printf("\r\n");
}

static void FS_PrintPacket(UI8_T *packet, UI16_T size)
{
    char title_bar[] = {
" 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F | 0123456789ABCDEF\r\n\
-------------------------------------------------+------------------"
};

    FS_Packet_Header_T  *header;

    header = (FS_Packet_Header_T *) packet;
    BACKDOOR_MGR_Printf("\r\nOpcode:     %s", FS_Opcode_Names[header->opcode]);
    BACKDOOR_MGR_Printf("\r\nSequence:   %d", header->seq_no);
    BACKDOOR_MGR_Printf("\r\nSize:       %d", header->data_size);
    BACKDOOR_MGR_Printf("\r\nPacket Content:\r\n%s\r\n", title_bar);
    for(;size > FS_WORD_WRAP;)
    {
        FS_PrintHexChar(packet, FS_WORD_WRAP);
        packet += FS_WORD_WRAP;
        size -= FS_WORD_WRAP;
    }
    FS_PrintHexChar(packet, size);
    return;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_WriteFileToMultipleUnit
 * ------------------------------------------------------------------------
 * FUNCTION : This function will concurrently write a file from buffer to
 *            the multiple unit.
 * INPUT    : dst_drive_bmp -- bit map of destination unit id
 *            file_name     -- the name of the file to write
 *            file_comment  -- the comment of the file
 *            file_type     -- file type
 *            buf           -- the source buffer
 *            length        -- data length to be writen (file size)
 *            resv_len      -- reserved length to guarantee the minimum
 *                             size of a file. resv_len < length will not
 *                             reserve extra space for the file.
 * OUTPUT   : None
 * RETURN   : TRUE  -- write file to all unit successfully
 *            FALSE -- fail in writing file to any unit
 * NOTE     : In ACP_V3, this function uses ISC reliable multi-cast to
 *            implement concurrent communication with each unit.
 * ------------------------------------------------------------------------
 */
BOOL_T FS_WriteFileToMultipleUnit (UI16_T  *dst_drive_bmp,  UI8_T   *file_name,
                                   UI8_T   *file_comment,   UI32_T  file_type,
                                   UI8_T   *buf,            UI32_T  length,
                                   UI32_T  resv_len)
{
    UI32_T                  my_drive_id = 0;
    UI16_T                  fail_drive_bmp = 0;
    UI16_T                  phase1_drive_bmp = 0, phase2_drive_bmp = 0, phase3_drive_bmp = 0, temp_drive_bmp = 0;
    UI32_T                  remaining_size;
    UI32_T                  return_value;
    UI32_T                  retry_count;
    BOOL_T                  done;
    UI32_T                  test_drive_id = 0;
    BOOL_T                  write_file_to_master = FALSE;

    FS_Request_Packet_T     *fs_rq_packet_p;
    FS_MCast_Data_Packet_T  *fs_mcast_data_packet_p;
    FS_Packet_Header_T      *fs_flash_packet_p;
    L_MM_Mref_Handle_T*     mref_handle_p;
    UI32_T                  pdu_len;

    /* check file name */
    if (FS_GenericFilenameCheck(file_name, file_type) != FS_RETURN_OK)
    {
        DEBUGSTR("Error : File name %s contains an invalid character!\r\n", file_name);
        return FALSE;
    }

    /* check file length */
    if (length <= 0)
    {
        DEBUGSTR("Error : File length is an invalid value!\r\n");
        return FALSE;
    }

    /* 0.0 if master unit in destination drive bitmap, turn ON the write_file_to_master flag */
    my_drive_id = FS_OM_GetControlDriverId();
    if ((*dst_drive_bmp & ((0x01) << (my_drive_id - 1))))
    {
        write_file_to_master = TRUE;
        *dst_drive_bmp      ^= (0x01) << (my_drive_id - 1);
    }

    if (   *dst_drive_bmp == 0
        && write_file_to_master == FALSE)
    {
        return TRUE;
    }
    /* have another target unit in destination drive bitmap */
    else
    {
        FS_File_Attr_T file_attr;
        FS_SETBUSY();
        FS_LOCK_REMOTE();

        /* save the file information to master's fs_control */
        memset(&(file_attr), 0, sizeof(FS_File_Attr_T));
        strncpy((char*)file_attr.file_name, (char*)file_name, SYS_ADPT_FILE_SYSTEM_NAME_LEN);
        strncpy((char*)file_attr.file_comment, (char*)file_comment, FS_FILE_COMMENT_LENGTH);
        file_attr.file_type   = file_type;
        file_attr.file_size   = length;
        file_attr.reserv_size = resv_len;
        file_attr.privilage   = FALSE;
        file_attr.check_sum   = FS_ByteSum(buf, length);
        FS_OM_SetControlAttribution(&(file_attr));

        /* 1.2 send request packet*/
        phase1_drive_bmp = *dst_drive_bmp;
        retry_count      = 0;
        while (1)
        {
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Request_Packet_T), /* tx_buffer_size */
                                                  L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_SEND, FS_MCAST_WRITE_REQUEST) /* user_id */
                                                 );
            fs_rq_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
            if (NULL == fs_rq_packet_p)
            {
                DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
                FS_UNLOCK_REMOTE();
                FS_SETNOTBUSY();
                return FALSE;
            }
            /* 1.0 send write request to remote drive */
            memset(fs_rq_packet_p, 0, sizeof(*fs_rq_packet_p));

            /* 1.1 prepare request packet */
            fs_rq_packet_p->header.opcode       = FS_MCAST_WRITE_REQUEST;
            fs_rq_packet_p->header.data_size    = sizeof(FS_File_Attr_T);
            FS_OM_GetControlAttribution(&(fs_rq_packet_p->file_attr));

            fs_rq_packet_p->header.seq_no   = FS_OM_GetControlSeqNo();

            if ((phase1_drive_bmp = ISC_SendMcastReliable(phase1_drive_bmp, ISC_FS_SID,
                                      mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                      FS_MCAST_TRY_COUNT,  FS_MCAST_TIMEOUT, FALSE)) != 0)
            {
                /* 1.2.1 NAK received or ISC error */
                if (retry_count ++ >= FS_MCAST_NAK_RETRY_TIMES)
                {
                    break;
                }
                else /* try again */
                {
                    SYSFUN_Sleep(FS_SLEEP_TIME);
                    continue;
                }
            }
            else
            {
                /* packet received successfully, increase seq_no */
                FS_OM_AddControlSeqNo();
                break;
            }
        }
        fail_drive_bmp |= phase1_drive_bmp;

        /* 2.0 write request granted by remote drive, start data transfer */
        temp_drive_bmp   = *dst_drive_bmp ^ phase1_drive_bmp;
        phase2_drive_bmp = 0;

        FS_OM_SetControlFilePtr(buf);

        /* 2.2 data transfering loop */
        remaining_size  = FS_OM_GetControlAttrFileSize();
        done            = FALSE;

        /*if file type is runtime image, need increase thread priority for packet send fast.*/

        while (!done)
        {
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_MCast_Data_Packet_T), /* tx_buffer_size */
                                                  L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_SEND, FS_MCAST_DATA) /* user_id */
                                                 );
            fs_mcast_data_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
            if (NULL == fs_mcast_data_packet_p)
            {
                DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
                FS_UNLOCK_REMOTE();
                FS_SETNOTBUSY();
                return FALSE;
            }

            /* 2.1 prepare data packet header */
            fs_mcast_data_packet_p->header.opcode  = FS_MCAST_DATA;

            fs_mcast_data_packet_p->header.seq_no  = FS_OM_GetControlSeqNo();
            phase2_drive_bmp                ^= temp_drive_bmp;

            /* 2.2.1 calculate remaining file size */
            if(remaining_size >= FS_MCAST_DATA_SIZE)
            {
                fs_mcast_data_packet_p->header.data_size = FS_MCAST_DATA_SIZE;
                remaining_size -= FS_MCAST_DATA_SIZE;
            }
            else
            {
                fs_mcast_data_packet_p->header.data_size = remaining_size;
                done = TRUE;
            }

            /* 2.2.2 copy data from data buffer to packet */
            memcpy(fs_mcast_data_packet_p->raw_data, FS_OM_GetControlFilePtr(), fs_mcast_data_packet_p->header.data_size);
            FS_OM_AddControlFilePtr(fs_mcast_data_packet_p->header.data_size);

            /* 2.2.3 send data packet */
            if ((phase2_drive_bmp = ISC_SendMcastReliable(phase2_drive_bmp, ISC_FS_SID,
                                        mref_handle_p,  SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                        FS_MCAST_TRY_COUNT, FS_MCAST_TIMEOUT, FALSE)) != 0)
            {
                fail_drive_bmp |= phase2_drive_bmp;
            }
            else
            {
                /* packet received successfully, increase seq_no */
                FS_OM_AddControlSeqNo();
            }

        }

        SYSFUN_Sleep(FS_SLEEP_TIME);

        /* 3.0 data transfer completed, send flash command */
        phase3_drive_bmp = *dst_drive_bmp ^ phase2_drive_bmp;

//        memset(&fs_packet_buffer, 0, sizeof(fs_packet_buffer));
        retry_count = 0;
        while (1)
        {
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Packet_Header_T), /* tx_buffer_size */
                                                  L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_SEND, FS_MCAST_PROGRAM_FLASH) /* user_id */
                                                 );
            fs_flash_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
            if (NULL == fs_flash_packet_p)
            {
                DBGMSG_EX("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
                FS_UNLOCK_REMOTE();
                FS_SETNOTBUSY();
                return FALSE;
            }

            /* 3.1 prepare program flash command packet */
            fs_flash_packet_p->opcode       = FS_MCAST_PROGRAM_FLASH;
            fs_flash_packet_p->seq_no       = FS_OM_GetControlSeqNo();
            fs_flash_packet_p->data_size    = 0;

            /* 3.2 send program flash command packet */
            DBGMSG("Send Program Flash Command");
            DUMP_TXPKT(*fs_flash_packet_p, sizeof(*fs_flash_packet_p));

//            fs_flash_packet_p->seq_no = FS_OM_GetControlSeqNo();

            if ((phase3_drive_bmp = ISC_SendMcastReliable(phase3_drive_bmp, ISC_FS_SID,
                                        mref_handle_p,  SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                        FS_MCAST_TRY_COUNT, FS_MCAST_TIMEOUT, FALSE)) != 0)
            {
                /* 3.2.1 NAK received or ISC error */
                if (retry_count ++ >= FS_MCAST_NAK_RETRY_TIMES)
                {
                    break;
                }
                else /* try again */
                {
                    SYSFUN_Sleep(FS_SLEEP_TIME);
                    continue;
                }
            }
            else
            {
                /* packet received successfully, increase seq_no */
                FS_OM_AddControlSeqNo();
                break;
            }
        }
        fail_drive_bmp |= phase3_drive_bmp;

        /* 3.3 write file to master, if master unit is in original destination bit map */
        if (TRUE == write_file_to_master)
        {
            return_value = FS_UpdateFile(my_drive_id, file_name, file_comment, file_type, buf, length, resv_len, FALSE);

            if (return_value != FS_RETURN_OK)
            {
                fail_drive_bmp |= (0x01) << (my_drive_id - 1);
            }
        }

        /* 3.4 check slave flash programming status */
        for (test_drive_id = (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * 2); test_drive_id >= 1; test_drive_id --)
        {
            if (   (*dst_drive_bmp != 0)
                && (*dst_drive_bmp & ((0x01) << (test_drive_id - 1))))
            {
                FS_CheckRemoteStatus(test_drive_id);
            }
        }

        FS_UNLOCK_REMOTE();
        FS_SETNOTBUSY();
    }

    *dst_drive_bmp = fail_drive_bmp;
    if (*dst_drive_bmp == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_RemoteMCastWrite
 * ------------------------------------------------------------------------
 * FUNCTION : This function handles multi-cast write request packet
 *            received from master.
 * INPUT    : key       -- key for ISC service
 *            mem_ref   -- memory reference points to received packet
 * OUTPUT   : ret_value -- TRUE  = request is accepted
 *                         FALSE = request is rejected by slave in busy
 *                                 state, fail in allocating buffer or FS
 *                                 shutdown flag is ON.
 * RETURN   : None
 * NOTE     : Upon receiving a write request from master, slave must save
 *            the file information for later use. Slave also needs to
 *            allocate buffer to hold data.
 * ------------------------------------------------------------------------
 */
static void FS_RemoteMCastWrite(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p, BOOL_T *ret_value)
{
    UI8_T               *rx_packet;
    FS_File_Attr_T      *rx_file_attr;
    UI32_T               pdu_len;

    rx_packet = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    /* if slave is busy, return nak to master */
    if (FS_OM_GetControlState() != FS_STATE_IDLE)
    {
        *ret_value = FALSE;
        return;
    }

    /* 1.0 save the current file information */
    DBGMSG("Save File Information");
    rx_file_attr = (FS_File_Attr_T *)(rx_packet + sizeof(FS_Packet_Header_T));
    FS_OM_SetControlAttribution(rx_file_attr);

    /* 2.0 allocate lending buffer */
    if (FS_OM_GetControlAttrFileSize() != 0)
    {
        DBGMSG("Write File...");

        /* allocate space for the file to write */
#if (SYS_CPNT_BUFFERMGMT == TRUE)
        FS_OM_SetControlFileBuffer(BUFFER_MGR_Allocate());
#else
        FS_OM_SetControlFileBuffer(malloc(FS_OM_GetControlAttrFileSize()));
#endif
        DBGMSG("Allocate Buffer");

        FS_OM_SetControlFilePtr(FS_OM_GetControlFileBuffer());

        if (FS_OM_GetControlAttrFileType() == FS_FILE_TYPE_TOTAL)
            FS_OM_SetControlFlashOperation(FS_FLASH_LOADER);
        else
            FS_OM_SetControlFlashOperation(FS_FLASH_FILE);

        if (!FS_OM_GetControlFileBuffer())
        {
            ERRMSG(FS_RETURN_NO_BUFFER);
            *ret_value = FALSE;
            return;
        }

        FS_OM_SetControlState(FS_STATE_DATA_TRANSFER);

        DBGMSG("Entering Data Loop... \\");
    }
    *ret_value = TRUE;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_RemoteMCastReceiveData
 * ------------------------------------------------------------------------
 * FUNCTION : This function handles multi-cast data packets received from
 *            master.
 * INPUT    : key       -- key for ISC service
 *            mem_ref   -- memory reference points to received packet
 * OUTPUT   : ret_value -- TRUE  = all of the data packets are received
 *                                 successfully.
 *                         FALSE = received data check sum error.
 * RETURN   : None
 * NOTE     : This function simply puts whatever in the data packet into
 *            file buffer.
 * ------------------------------------------------------------------------
 */
static void FS_RemoteMCastReceiveData(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p, BOOL_T *ret_value)
{
    UI8_T                   *rx_packet;
    FS_MCast_Data_Packet_T  *mcast_data_packet;
    UI8_T                   sum;
    UI32_T                  pdu_len;


    rx_packet           = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    mcast_data_packet   = (FS_MCast_Data_Packet_T *) rx_packet;


    /* 1.0 make sure FS is in correct state */
    if (FS_OM_GetControlState() != FS_STATE_DATA_TRANSFER)
    {
        ERRMSG(FS_RETURN_INCORRECT_STATE);
        *ret_value = FALSE;
        return;
    }

    /* 2.0 put data in data buffer */
    memcpy(FS_OM_GetControlFilePtr(), mcast_data_packet->raw_data, mcast_data_packet->header.data_size);
    FS_OM_AddControlFilePtr(mcast_data_packet->header.data_size);

    /* if EOF reached */
    if (mcast_data_packet->header.data_size < FS_MCAST_DATA_SIZE)
    {
        DBGMSG("File transimission completed");
        FS_OM_SetControlState(FS_STATE_DATA_COMPLETE);
        /* check file error */
        sum = FS_ByteSum(FS_OM_GetControlFileBuffer(), FS_OM_GetControlAttrFileSize());

        if (sum != FS_OM_GetControlAttrCheckSum())
        {
            ERRMSG(FS_RETURN_ACTION_INHIBITED);
            ret_value = FALSE;
            return;
        }
    }

    /* Return TRUE if there is no mistake */
    *ret_value = TRUE;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_RemoteMCastFlashFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function handles flash programming command sent from
 *            master.
 * INPUT    : key       -- key for ISC service
 *            mem_ref   -- memory reference points to received packet
 * OUTPUT   : ret_value -- TRUE  = successfully sending event to FS task
 *                         FALSE = sending event to FS task fail
 * RETURN   : None
 * NOTE     : This function will send event to FS task that writes the file
 *            in the fs control block onto the flash device.
 * ------------------------------------------------------------------------
 */
static void FS_RemoteMCastFlashFile(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p, BOOL_T *ret_value)
{

    if (FS_OM_GetControlState() != FS_STATE_DATA_COMPLETE)
    {
        *ret_value = FALSE;
        return;
    }
    FS_OM_SetControlState(FS_STATE_FLASH_PROGRAMMING);
    DBGMSG("Start to Program Flash, Create Task");

    if (TRUE == FS_OM_GetShutdownFlag())
    {
       *ret_value = FALSE;
        return;
    }

    if (FS_OM_GetTaskId())
    {
        if (SYSFUN_SendEvent(FS_OM_GetTaskId(), FS_TYPE_EVENT_LEGACYFS) != SYSFUN_OK)
        {
            DBGMSG("Remote write");
            *ret_value = FALSE;
        }
        else
        {
            *ret_value = TRUE;
        }
    } /* End of if (_fs_task_existing_) */
    return;
}

#endif /* End of #if (SYS_CPNT_STACKING == TRUE) */

#if 0
/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_Strcat
 * ------------------------------------------------------------------------
 * FUNCTION : This function concatenates the normal_str_p(string that always
 *            terminated with '\0') and the special_str_p(string that might
 *            not terminated with '\0').
 * INPUT    : normal_str_p -
 *                The string to be concatenated with the other string. This
 *                string must be terminated with '\0'.
 *            special_str_p -
 *                 The string that might not terminated with '\0'.
 *            special_str_buf_size -
 *                 The buffer size of the argument special_str_p.
 *            output_str_buf_size -
 *                 The buffer size of the argument output_str_buf_p.
 *
 * OUTPUT   : output_str_buf_p -
 *                 The buffer for the result of concatenating normal_str_p and
 *                 special_str_p. The string in output_str_buf_p will always be
 *                 terminated with '\0' when the function returns TRUE.
 * RETURN   : TRUE   - Successfully
 *            FALSE  - Failed
 * NOTE     :
 *            This function concatenates the normal string(always terminated
 *            with '\0') and the special string(string that might not terminated
 *            with '\0' and has provided the max string length). The
 *            concatenated string will be output to output_str_buf_p if the
 *            size of output_str_buf_p is large enough to keep the string.
 * ------------------------------------------------------------------------
 */
static BOOL_T FS_Strcat(const char* normal_str_p, const char* special_str_p, UI32_T special_str_buf_size, UI32_T output_str_buf_size, char* output_str_buf_p)
{
    int  i, output_str_buf_free_len=output_str_buf_size;
    int  output_len=0,output_len_tmp;
    BOOL_T special_str_is_null_terminated;

    /* sanity check
     */
    if ( (normal_str_p==NULL) ||
         (special_str_p==NULL) ||
         (special_str_buf_size==0) ||
         (output_str_buf_size==0) ||
         (output_str_buf_p==NULL)
       )
    {
        BACKDOOR_MGR_Printf("%s(%d):Invalid input argument.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    output_len_tmp=snprintf(output_str_buf_p+output_len, output_str_buf_free_len,
        "%s", normal_str_p);
    output_len+=output_len_tmp;
    output_str_buf_free_len-=output_len_tmp;

    if (output_str_buf_free_len<=0)
    {
        DBGMSG("size of output_str_buf_p is too small\r\n");
        return FALSE;
    }

    /* check whether special_str_p is terminated by '\0'
     */
    for (special_str_is_null_terminated=FALSE,i=0; i<special_str_buf_size; i++)
    {
        if (special_str_p[i]=='\0')
        {
            special_str_is_null_terminated=TRUE;
            break;
        }
    }

    if (special_str_is_null_terminated==TRUE)
    {
        output_len_tmp=snprintf(output_str_buf_p+output_len, output_str_buf_free_len,
            "%s", special_str_p);
        output_len+=output_len_tmp;
        output_str_buf_free_len-=output_len_tmp;
        if (output_str_buf_free_len<0) /* it's ok when output_str_buf_free_len==0. That means all space in the specified buffer are used. */
        {
            DBGMSG("size of output_str_buf_p is not enough when appending special_str_p\r\n");
            return FALSE;
        }    
    }
    else if ((output_str_buf_free_len-1)<special_str_buf_size)/* In this case, output_str_buf_free_len contains the terminating null char, special_str_buf_size does not contain the terminating null char. */
    {
        DBGMSG("size of output_str_buf_p is not enough when appending special_str_p\r\n");
        return FALSE;
    }
    else
    {
        /* adding special_str_p with length 'special_str_buf_size' into the
         * end of the output_str_buf_p
         */
        memcpy(output_str_buf_p+output_len, special_str_p, special_str_buf_size);
        output_len+=special_str_buf_size;
        output_str_buf_free_len-=special_str_buf_size;
        /* add the terminating null char into the end of output_str_buf_p */
        if (output_str_buf_free_len>=1)
        {
            output_str_buf_p[output_len]='\0';
        }
        else
        {
            /* this case should not happen!
             */
            BACKDOOR_MGR_Printf("%s(%d)Bug in this function!\n", __FUNCTION__, __LINE__);
            return FALSE;
            
        }
    }

    return TRUE;
}
#endif

#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_InvokeAOSChangeStartupInstallerScript
 * ------------------------------------------------------------------------
 * FUNCTION : This function will invoke the AOS change starup installer script
 *            with the given startup installer filename.
 * INPUT    : installer_filename_p -
 *                filename of the installer file to be set as startup
 * OUTPUT   : None.
 * RETURN   : TRUE   - Successfully
 *            FALSE  - Failed
 * NOTE     : The caller shall call FS_SEMAPHORE_LOCK() before calling this
 *            function to protect the transaction of ONIE installation.
 * ------------------------------------------------------------------------
 */
static BOOL_T FS_InvokeAOSChangeStartupInstallerScript(const char* installer_filename_p)
{
    char *shell_cmd_buf_p=NULL;
    char *script_filename_buf_p=NULL;
    int rc, shell_exit_status;
    BOOL_T ret_val=TRUE;

    shell_cmd_buf_p=malloc(FS_MAX_PATHNAME_BUF_SIZE);
    if (shell_cmd_buf_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s:%d, failed to malloc %d bytes.\r\n", __FUNCTION__, __LINE__,
            FS_MAX_PATHNAME_BUF_SIZE);
        return FALSE;
    }

    if (FS_OM_GetONIEInstallerDebugFlag()==FALSE)
    {
        rc=snprintf(shell_cmd_buf_p, FS_MAX_PATHNAME_BUF_SIZE,
            "%s %s 1>/dev/null 2>&1", AOS_ONIE_CHANGE_STARTUP_INSTALLER_SCRIPT_FILENAME, installer_filename_p);
    }
    else
    {
        rc=snprintf(shell_cmd_buf_p, FS_MAX_PATHNAME_BUF_SIZE,
            "%s %s", AOS_ONIE_CHANGE_STARTUP_INSTALLER_SCRIPT_FILENAME, installer_filename_p);
    }

    if (rc <0)
    {
        BACKDOOR_MGR_Printf("%s:%d, unknown snprintf error\r\n", __FUNCTION__, __LINE__);
        ret_val=FALSE;
        goto error_exit;
    }
    else if(rc>(FS_MAX_PATHNAME_BUF_SIZE-1))
    {
        BACKDOOR_MGR_Printf("%s:%d, shell_cmd_buf_p is too small(required buf len=%d)\r\n",
            __FUNCTION__, __LINE__, rc+1);
        ret_val=FALSE;
        goto error_exit;

    }

    DBGMSG_EX("%s:%d, shell_cmd_buf_p='%s'\n", __FUNCTION__, __LINE__, shell_cmd_buf_p);

    if (SYSFUN_ExecuteSystemShellEx(shell_cmd_buf_p, &shell_exit_status)!=SYSFUN_OK)
    {
        DBGMSG_EX("%s:%d, shell script %s returns error(exit_status=%d).\r\n", __FUNCTION__, __LINE__,
            AOS_ONIE_CHANGE_STARTUP_INSTALLER_SCRIPT_FILENAME, shell_exit_status);
        ret_val=FALSE;
        goto error_exit;
    }

error_exit:
    if (shell_cmd_buf_p)
    {
        free(shell_cmd_buf_p);
    }
    return ret_val;
} /* end of FS_InvokeAOSChangeStartupInstallerScript() */
#endif

#ifndef INCLUDE_DIAG
BOOL_T FS_BACKDOOR_TestNandwrite(void)
{
   return TRUE;
}
BOOL_T FS_BACKDOOR_Test_FS_ReadNandFlash(void)
{
    UI32_T count, ret;
    FS_HW_Info_T *hwinfo;
    UI32_T hw_info_mtd_id;

    hwinfo=(FS_HW_Info_T*)malloc(sizeof(FS_HW_Info_T));

    if(FS_GetMtdDevIdByMtdType(FS_MTD_PART_HW_INFO, &hw_info_mtd_id)==TRUE)
    {
        ret=FS_READ_HWINFO_FROM_FLASH((UI8_T*)hwinfo, sizeof(FS_HW_Info_T), &count, hw_info_mtd_id);
    }
    else
    {
        ret=FS_RETURN_BAD_HW;
    }

    if(ret!=FS_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("Test FS_READ_HWINFO_FROM_FLASH() fail\n");
        free(hwinfo);
        return FALSE;
    }
    BACKDOOR_MGR_Printf("hwinfo->mac_addr=%s\n", hwinfo->mac_addr);
    BACKDOOR_MGR_Printf("hwinfo->serial_no=%s\n", hwinfo->serial_no);
    BACKDOOR_MGR_Printf("hwinfo->agent_hw_ver=%s\n", hwinfo->agent_hw_ver);
    free(hwinfo);
    return TRUE;
}
BOOL_T FS_BACKDOOR_Test_FS_GetStorageFreeSpaceForUpdateRuntime(void)
{
    UI32_T size;

    if(FS_GetStorageFreeSpaceForUpdateRuntime(0, &size)!=FS_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("Fail in %s:%d\r\n", __FUNCTION__, __LINE__);
        return 0;
    }
    BACKDOOR_MGR_Printf("get size=%ld\r\n", (long)size);
    return 1;
}
#define MAX_FS_BACKDOOR_CMDS (sizeof(FS_BACKDOOR_Cmd)/sizeof(FS_BACKDOOR_Cmd[0]))
typedef struct
{
    char   index;
    int   (*fpFunction)(void);
    char   *help;
    char   *usage;
} REGISTER_CLI_CMD_T;

REGISTER_CLI_CMD_T FS_BACKDOOR_Cmd[]={
{'1', (void*)FS_BACKDOOR_TestNandwrite, "Test nandwrite", NULL},
{'2', (void*)FS_BACKDOOR_Test_FS_ReadNandFlash, "Test FS_ReadNandFlash", NULL},
{'3', (void*)FS_BACKDOOR_Test_FS_GetStorageFreeSpaceForUpdateRuntime, "Test FS_GetStorageFreeSpaceForUpdateRuntime", NULL}
};
static void FS_BackDoor_SubMenu (void)
{
    int   ch;
    BOOL_T  eof=FALSE;
    int i;

    while (!eof)
    {
        BACKDOOR_MGR_Printf("\n==========FS BackDoor Sub Menu========\n");
        BACKDOOR_MGR_Printf(" 0. Exit\n");
        for(i=0;i<MAX_FS_BACKDOOR_CMDS;i++)
           BACKDOOR_MGR_Printf(" %c. %s \n",FS_BACKDOOR_Cmd[i].index, FS_BACKDOOR_Cmd[i].help);
        BACKDOOR_MGR_Printf("   select =");
        ch = BACKDOOR_MGR_GetChar();
        if(ch==EOF)
        {
            continue;
        }

        BACKDOOR_MGR_Printf ("%c\n",ch);
        if(ch=='0')
        {
          break;
        }
        else
        {
            for(i=0;i<MAX_FS_BACKDOOR_CMDS; i++)
            {
                if((char)ch==FS_BACKDOOR_Cmd[i].index)
                {
                    FS_BACKDOOR_Cmd[i].fpFunction();
                }
            }
        }
    }

}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_BackDoor_Menu
 * ------------------------------------------------------------------------
 * FUNCTION : This is FS backdoor main function
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 *
 * Note     : None
 * ------------------------------------------------------------------------
 */
static void FS_BackDoor_Menu (void)
{
    int     ch;
    BOOL_T  eof=FALSE;

    while (! eof)
    {
        BACKDOOR_MGR_Printf("\r\n==========FS BackDoor Menu=====================\r\n");
        BACKDOOR_MGR_Printf("0. Exit\r\n");
        BACKDOOR_MGR_Printf("1. Toggle FS debug flag(%s)\r\n", (FS_OM_GetDebugFlag()==TRUE)?"On":"Off");
        BACKDOOR_MGR_Printf("2. Toggle ONIE installer debug flag(%s)\r\n", (FS_OM_GetONIEInstallerDebugFlag()==TRUE)?"On":"Off");
        BACKDOOR_MGR_Printf("=================================================\r\n");
        BACKDOOR_MGR_Printf("  select =");
        ch = BACKDOOR_MGR_GetChar();
        if(ch==EOF)
        {
            continue;
        }

        BACKDOOR_MGR_Printf ("%c\r\n",ch);
        switch(ch)
        {
            case '0':
                eof=TRUE;
                break;
            case '1':
                FS_OM_SetDebugFlag(!FS_OM_GetDebugFlag());
                break;
            case '2':
                FS_OM_SetONIEInstallerDebugFlag(!FS_OM_GetONIEInstallerDebugFlag());
                break;
            case 'J':
                FS_BackDoor_SubMenu();
                break;
            default:
                ch=0;
                break;
      }/*End switch(ch)*/

    }/*End while (! eof)*/

}
#endif /*End of #if INCLUDE_DIAG*/
