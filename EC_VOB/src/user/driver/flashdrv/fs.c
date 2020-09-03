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
#include "sysfun.h"
#include "sys_bld.h"
#include "sys_cpnt.h"
#include "sys_bld.h"
#include "sys_adpt.h"
#include "sys_hwcfg.h"
#include "sys_dflt.h"
#include "sys_module.h"
#include "leaf_sys.h"
#include "l_mm.h"
#include "l_math.h"
#include "l_stdlib.h"
#include "stktplg_pom.h"
#include "fs_type.h"
#include "sysrsc_mgr.h"
#include "fs_om.h"
#include "fs.h"
#include "buffer_mgr.h"

#ifdef LOADER
#include "console.h"
#endif
#ifdef INCLUDE_DIAG
#undef SYS_CPNT_STACKING
#endif
#if (SYS_CPNT_STACKING == TRUE)
#include "isc.h"
#endif

//#include "backdoor_mgr.h"
//#include "fs_backdoor.h"

/* Linux 
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/vfs.h> /* or <sys/statfs.h> */ 

/* The following three typedef is used in image.h
 */
typedef	unsigned char	uint8_t;
typedef	unsigned short	uint16_t;
typedef	unsigned int	uint32_t;

#include "image.h"       /* get uboot header */
#include "cramfs_fs.h"   /* get cramfs header */

//Wally temporary modify
#define FS_MAX_FILE_NAME_LENGTH         SYS_ADPT_FILE_SYSTEM_NAME_LEN
#define FS_SAVE_FOLDER                  "/flash/.fs/"
#define FS_UBOOT_BOOT_ARGS              "bootargs=root=/dev/mtdblock"
#define FS_UBOOT_BOOT_CMD               "bootcmd=bootm "
#define FS_FLASH_MTD_BLOCK_PREFIX       "/dev/mtdblock"
#define FS_RANDOM_KERNEL_NAME_PREFIX    "kernel"
#define FS_RANDOM_ROOTFS_NAME_PREFIX    "rootfs"
#define FS_MTD_NUMBER_NOT_EXIST         0xFFFFFFFF
#ifdef ECN430_FB2
#define FS_CFCARD_PREFIX    "/dev/cfcard"
#endif

/* Mapping file type FS_FILE_TYPE_SUBFILE(0) ~ FS_FILE_TYPE_TOTAL(13) to 
 * file user ID (1000 ~ 1027) 
 */
#define FS_TYPE_UID_BASE                        1000
#define FS_UID_TO_TYPE(uid)                     (((uid)-FS_TYPE_UID_BASE)>>1)
#define FS_UID_TO_STARTUP(uid)                  ((((uid)&0x1)==1)?TRUE:FALSE)
#define FS_TYPE_STARTUP_TO_UID(type,startup)    ((startup)?(((type)<<1)+FS_TYPE_UID_BASE+1):(((type)<<1)+FS_TYPE_UID_BASE))

#define EH_MGR_Handle_Exception(a,b,c,d)        ERRORSTR("\r\n%s: ERROR !!!", __FUNCTION__)
#define EH_MGR_Handle_Exception1(a,b,c,d,e)     ERRORSTR("\r\n%s: ERROR !!! %s.",__FUNCTION__, e)
#if defined (ES4626H) /*ES4626H*/
enum
{
    FS_HRCW = 0,
    FS_KERNEL_A,
    FS_KERNEL_B,
    FS_ROOTFS_A,
    FS_ROOTFS_B,
    FS_USERDATA,
    FS_LOADER,
    FS_LOADERDATA,
    FS_HWINFO
};

//Wally temporary modify
#elif defined (ECN430_FB2)
enum
{
    FS_HRCW = 0,
    FS_SYSINFO,
    FS_SYSCONFIG,
    FS_KERNEL,
    FS_USER_DATA,
    FS_LOADERDATA,
    FS_HWINFO,
    FS_LOADER
};
/* jerry.du add 20080602 */
#elif defined (ES3628BT_FLF_ZZ) || defined(ASF4526B_FLF_P5)
enum
{
    FS_LOADER = 0,
    FS_PARTITIONTABLE,
    FS_LOADERDATA,
    FS_FILEMAPPING,
    FS_HWINFO
};
#endif

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
#define FS_TIMEOUT_VALUE            30000
#define FS_ISC_TRY_COUNT            5
#define FS_SLEEP_TIME               50
#define FS_COMMAND_RETRY_COUNT      30
#define FS_COMMAND_RETRY_MORECOUNT  100
#define FS_ABORT_COUNT              1
#define FS_WORD_WRAP                16

#define FS_MCAST_DATA_SIZE          (SYS_ADPT_ISC_MAX_PDU_LEN - sizeof(FS_Packet_Header_T))
#define FS_MCAST_TRY_COUNT          5
#define FS_MCAST_TIMEOUT            30000
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

#define FS_FILENAME_CHECK_VALID_NUM_OF_RULE     5
#define FS_FILENAME_CHECK_VALID_RANGE_1         "AZ"
#define FS_FILENAME_CHECK_VALID_RANGE_2         "az"
#define FS_FILENAME_CHECK_VALID_RANGE_3         "09"
#define FS_FILENAME_CHECK_VALID_RANGE_4         ""
#define FS_FILENAME_CHECK_VALID_ENUM            "-_.()"
#define FS_FILENAME_CHECK_VALID_INVIS           "." // Wally "$"

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
    FS_FILE_TYPE_VISIBLE_RUNTIME        = TRUE,
    FS_FILE_TYPE_VISIBLE_SYSLOG         = FALSE,
    FS_FILE_TYPE_VISIBLE_CMDLOG         = FALSE,
    FS_FILE_TYPE_VISIBLE_CONFIG         = TRUE,
    FS_FILE_TYPE_VISIBLE_POSTLOG        = FALSE,
    FS_FILE_TYPE_VISIBLE_PRIVATE        = FALSE,
    FS_FILE_TYPE_VISIBLE_CERTIFICATE    = TRUE,
    FS_FILE_TYPE_VISIBLE_ARCHIVE        = TRUE,
    FS_FILE_TYPE_VISIBLE_BINARY_CONFIG  = FALSE,
    FS_FILE_TYPE_VISIBLE_PUBLIC         = TRUE
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
    FS_TOTAL_REMOTE_SERVICES    /* Number of total remote services */
};
#endif /* SYS_CPNT_STACKING */

#if 0
/* Following is located at the head of a block */
typedef struct
{
    UI32_T  magic_number;
    UI8_T   file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1]; /* the name of the file */
    UI8_T   file_comment[FS_FILE_COMMENT_LENGTH+1];     /* the comment of the file */
    UI8_T   file_type;                                  /* the type of the file */
    UI8_T   check_sum_complement;                       /* value which makes the byte sum of this block as zero */
    UI32_T  create_time;                                /* the time when the file is created */
    UI32_T  file_size;                                  /* the size of the file */
    UI32_T  next_block_id;
}__attribute__((packed, aligned(1))) FS_FileInfo_T;

typedef struct
{
    UI32_T  magic_number;
    UI16_T  pad;
    UI8_T   num_of_block;
    UI8_T   check_sum_complement;
}__attribute__((packed, aligned(1))) FS_SpecialBlockTail_T;

typedef struct
{
    UI32_T  bdinfo_block_id;                                /* the block id of board information */
    UI32_T  loader_block_id;                                /* the block id of loader */
    UI32_T  startup_block_id;                               /* the block id saving the startup file name of file system */
    UI32_T  backup_startup_block_id;
    UI32_T  reserve_block_id[FS_MAX_NUM_OF_RESERVE_BLOCK];  /* the block id of reserve block */
                                                            /* FS_NULL_BLOCK_ID (0xFFFFFFFF): reserve block is not existed */
                                                            /* 0 ~ total_num_of_block       : range of existed block id    */
    UI32_T  total_num_of_block;
    UI32_T  max_file_name_length;
} FS_Info_T;

typedef struct FS_FileBlockInfo_S
{
    UI32_T  block_id;
    UI32_T  block_size;
    struct  FS_FileBlockInfo_S  *next;
} FS_FileBlockInfo_T;
#endif /* 0 */


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

static UI32_T FS_WriteFlash(UI8_T *buf, UI32_T length, UI32_T mtd);
static UI32_T FS_ReadFlash(UI8_T *buf, UI32_T buf_len, UI32_T *read_len, UI32_T mtd);
static BOOL_T FS_GetMtdNumByStartAddress(UI32_T start_addr, UI32_T *mtd_num_p);
static UI32_T FS_CheckKernelHeader(UI8_T *buf_p, UI32_T length);
static UI32_T FS_CheckRootfsHeader(UI8_T *buf_p, UI32_T length);
static BOOL_T FS_BuildStartupKernelFile(char file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN]);
static BOOL_T FS_BuildStartupRootfsFile(char file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN]);
static BOOL_T FS_BuildFileHeaderFromFile(char file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN]);
static BOOL_T FS_BuildAllFileHeader(void);

static  UI8_T   FS_ByteSum(UI8_T *buf_ptr, UI32_T length);
static  void    FS_StartupVerification(void);
static  UI32_T  FS_EraseFile(UI32_T unit, UI8_T  *file_name, BOOL_T privilege);
static  UI32_T  FS_UpdateFile(UI32_T unit, UI8_T  *file_name, UI8_T  *file_comment, UI32_T file_type, UI8_T *buf, UI32_T length, UI32_T resv_len, BOOL_T privilege);
static  UI32_T  FS_UpdateStartupAttribute(UI32_T file_type, UI8_T  *file_name, BOOL_T privilege);

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
                        FS_MAX_NUM_OF_FILE_PUBLIC
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
                        FS_FILE_TYPE_VISIBLE_PUBLIC
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
    NULL                                /* FS_HAVEMOREDATA */
};

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
    ""
};

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

/************************************
 ***      MACRO DEFINITIONS       ***
 ************************************/
#define FS_SEMAPHORE_LOCK()     SYSFUN_TakeSem(FS_OM_GetSemaphore(), SYSFUN_TIMEOUT_WAIT_FOREVER);
#define FS_SEMAPHORE_UNLOCK()   SYSFUN_GiveSem(FS_OM_GetSemaphore());
#define FS_LOCK(_ret_)      {   FS_SEMAPHORE_LOCK();                \
                                if (FS_OM_GetShutdownFlag())        \
                                {                                   \
                                    FS_SEMAPHORE_UNLOCK();          \
                                    return _ret_;                   \
                                }                                   \
                                if (FS_Debug(FS_DEBUG_FLAG_DBGMSG)) \
                                    printf("\r\n%s(): Set busy at line %d\r\n", __FUNCTION__, __LINE__);    \
                            }

#define FS_UNLOCK()         {   FS_SEMAPHORE_UNLOCK();              \
                                if (FS_Debug(FS_DEBUG_FLAG_DBGMSG)) \
                                    printf("\r\n%s(): Set not_busy at line %d\r\n", __FUNCTION__, __LINE__);    \
                            }

#define FS_SETBUSY()        FS_LOCK(FS_RETURN_NOT_READY)
#define FS_SETNOTBUSY()     FS_UNLOCK()
                            
#if (SYS_CPNT_STACKING == TRUE)
#define DBGMSG(x)           if(FS_Debug(FS_DEBUG_FLAG_DBGMSG))                      \
                                printf("\r\n%s() Debug: %s", __FUNCTION__, (x))

#define ERRMSG(x)           if(FS_Debug(FS_DEBUG_FLAG_ERRMSG))                      \
                                printf("\r\n%s() Error: %s", __FUNCTION__, fs_error_messages[(x)] )

#define GET_PDU_LEN(x)      (sizeof((x).header) + (x).header.data_size)

#define DUMP_HEADER(x)      if( FS_Debug(FS_DEBUG_FLAG_DUMP_TXPKT) )                \
                            {                                                       \
                                printf("\r\n%s() Send:", __FUNCTION__);             \
                                FS_PrintPacket( (UI8_T *) &(x), (sizeof(FS_Packet_Header_T) );      \
                            }
#define DUMP_TXPKT(x, y)    if( FS_Debug(FS_DEBUG_FLAG_DUMP_TXPKT) )                \
                            {                                                       \
                                printf("\r\n%s() Send:", __FUNCTION__);             \
                                FS_PrintPacket( (UI8_T *) &(x), (y) );      \
                            }

#define DUMP_RXPKT(x, y)    if( FS_Debug(FS_DEBUG_FLAG_DUMP_RXPKT) )                \
                            {                                                       \
                                printf("\r\n%s() Received:", __FUNCTION__);         \
                                FS_PrintPacket( (UI8_T *) &(x), (y) );      \
                            }

#define FS_LOCK_REMOTE()    {   DBGMSG("LOCK REMOTE!"); \
                                SYSFUN_TakeSem(FS_OM_GetCommSemaphore(), SYSFUN_TIMEOUT_WAIT_FOREVER); }
#define FS_UNLOCK_REMOTE()  {   DBGMSG("UNLOCK REMOTE!");   \
                                SYSFUN_GiveSem(FS_OM_GetCommSemaphore());}
#else /* (SYS_CPNT_STACKING == TRUE) */
#define DBGMSG(x)           ;
#define ERRMSG(x)           ;
#define GET_PDU_LEN(x)      ;
#define DUMP_HEADER(x)      ;
#define DUMP_TXPKT(x, y)    ;
#define DUMP_RXPKT(x, y)    ;
#define FS_LOCK_REMOTE()    ;
#define FS_UNLOCK_REMOTE()  ;

#endif  /* (SYS_CPNT_STACKING == TRUE) */

/*
 * Flash structure:
 * +-------------------+ -> flash_start_addr
 * | Board Info        |        Used by Manufacture
 * +-------------------+
 * | Startup block     |        Used by File System. Store the default file names.
 * +-------------------+
 * |                   |        Used by File System. After this block, files are stored.
 * +-------------------+
 * |  .                |
 * |  .                |
 * |  .                |
 * |  .                |
 * |  .                |
 * |  .                |
 * +-------------------+
 * | Loader            |        Used by Loader
 * +-------------------+
 * |  .                |
 * |  .                |
 * +-------------------+
 */

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
#if 0
    if (    SYSFUN_SpawnTask(   SYS_BLD_FS_TASK,
                                SYS_BLD_FLSHDRV_TASK_PRIORITY,
                                SYS_BLD_TASK_COMMON_STACK_SIZE,
                                0,              /* task_option */
                                FS_TASK_Main,
                                0,              /* arg */
                                &task_id) != SYSFUN_OK)
#endif
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
        printf("\r\n%s()::(line %d) Failed in creating task\r\n", __FUNCTION__, __LINE__);
    }
    FS_OM_SetTaskId(task_id);

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
    UI32_T  msgq_id;

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
#if 0 /* Still under construction */
    UI32_T                          msgq_id;
    FS_TYPE_MSG_T                   msg;
    UI32_T                          current_mode;
#endif

    all_events = FS_TYPE_EVENT_NONE;

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
        all_events &= (FS_TYPE_EVENT_LEGACYFS|FS_TYPE_EVENT_CHECKFS|FS_TYPE_EVENT_REMOTECALL|FS_TYPE_EVENT_REPLY);

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

    } /* End of while */
} /* End of FS_TASK_Main() */
#endif

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

UI32_T  FS_WriteUbootBootargs(UI32_T mtd_num)
{
    UI32_T      length;
    UI32_T      count;
    I32_T       fno;
    UI32_T      crc;
    UI8_T       *buf;
    UI8_T       *str;
    I32_T       ret = FS_RETURN_OK;
    BOOL_T      valid_mtd_num;
    BOOL_T      found_loader_data_partition;
    UI8_T       loader_data_partition_name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN];
    UI32_T      loader_data_partition_size;
    UI8_T       write_bootargs_str[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN];
    SYS_HWCFG_FlashEntry_T *flash_entry_p;

    flash_entry_p = NULL;
    valid_mtd_num = FALSE;
    found_loader_data_partition = FALSE;
    loader_data_partition_size  = 0;
    while (NULL != (flash_entry_p = SYS_HWCFG_GetNextFlashPartitionEntry(flash_entry_p)))
    {
        if (mtd_num == flash_entry_p->mtdnum)
        {
            valid_mtd_num = TRUE;
            sprintf((char*)write_bootargs_str,"%s%lu",FS_UBOOT_BOOT_ARGS,mtd_num);
        }
        if (SYS_HWCFG_FLASH_TYPE_LOADERDATA == flash_entry_p->type)
        {
            found_loader_data_partition = TRUE;
            sprintf((char*)loader_data_partition_name,"%s%lu",FS_FLASH_MTD_BLOCK_PREFIX,flash_entry_p->mtdnum);
            loader_data_partition_size = flash_entry_p->size;
        }
        if (TRUE == valid_mtd_num && TRUE == found_loader_data_partition)
        {
            break;
        }
    }

    if (FALSE == found_loader_data_partition)
    {
        ERRORSTR("\r\n%s::ERROR!! loaderdata partition not found\n", __FUNCTION__);
        return FS_RETURN_ERROR;
    }
    
    if (FALSE == valid_mtd_num)
    {
        ERRORSTR("\r\n%s::ERROR!! invalid mtd_num = %lu\n", __FUNCTION__, mtd_num);
        return FS_RETURN_BAD_PARA;
    }

    fno = open((char*)loader_data_partition_name, O_RDWR, 0666);
    if( fno == -1 )
    {
        ERRORSTR("\r\n%s::ERROR!! Can not open %s!!", __FUNCTION__, loader_data_partition_name);
        return FS_RETURN_ERROR;
    }
    buf = malloc(loader_data_partition_size);
    if( buf == NULL )
    {
        ERRORSTR("\r\n%s::ERROR!! no buffer !!!", __FUNCTION__);
        close(fno);
        return FS_RETURN_NO_BUFFER;
    }
    
    count = read(fno, buf, loader_data_partition_size);
    if( count < loader_data_partition_size )
    {
        ERRORSTR("\r\n%s::ERROR!! read size %lu < %lu!!", __FUNCTION__, count, loader_data_partition_size);
        ret = FS_RETURN_ERROR;
    }
    else
    {
        crc = *((UI32_T*)buf);
        str = buf+4;

        /* verify crc32 (or data has problem */        
        if( crc != (count = L_MATH_Crc32(0, (I8_T*)(buf+4), loader_data_partition_size-4)) )
        {
            ERRORSTR("\r\n%s:ERROR, check CRC error, %lx != %lx", __FUNCTION__, crc, count);
            ret = FS_RETURN_ERROR;
        }
        else
        {
            // DEBUGSTR("\r\n%s, check CRC OK, %lx ", __FUNCTION__, crc);
            while( str[0] != 0 )
            {
                length = strlen(str);
                if( length ) DEBUGSTR("\r\n\"%s\", length=%ld", str, length);

                if( strncmp(str, FS_UBOOT_BOOT_ARGS, strlen(FS_UBOOT_BOOT_ARGS)) == 0)
                {
                    if( strncmp(str, write_bootargs_str, strlen(write_bootargs_str)) == 0)
                    {
                        DEBUGSTR("\r\n%s, setup to same rootfs /dev/mtdblock%lu, do nothing", __FUNCTION__, mtd_num);
                        break;
                    }
                    else
                    {
                        DEBUGSTR("\r\n%s, setup bootargs root=/dev/mtdblock%lu...", __FUNCTION__, mtd_num);
                        memcpy(str,write_bootargs_str,strlen(write_bootargs_str));
                        /* re-calculate the CRC */
                        crc = L_MATH_Crc32(0, (I8_T*)(buf+4), loader_data_partition_size-4);
                        *((UI32_T*)buf) = crc;

                        /* write data back to flash */
                        /*lseek(fno, 0, SEEK_SET);
                        count = write(fno,buf,loader_data_partition_size);
                        if( count < loader_data_partition_size )
                        {
                            ERRORSTR("\r\n%s:ERROR, write u-boot data %ld < %d",__FUNCTION__, count, loader_data_partition_size);
                            ret = FS_RETURN_ERROR;
                        }*/
                        break;
                    }
                }
                /* pointer to next string 
                 */
                str += (strlen(str)+1);
            }
        }
    }
    
    close(fno);

    fno = open((char*)loader_data_partition_name, O_WRONLY, 0666);
    if( fno == -1 )
    {
        ERRORSTR("\r\n%s::ERROR!! Can not open %s to write!!", __FUNCTION__, loader_data_partition_name);
        free(buf);
        return FS_RETURN_ERROR;
    }

    count = write(fno, buf, loader_data_partition_size);
    if( count < loader_data_partition_size )
    {
        ERRORSTR("\r\n%s:ERROR!! write size %ld < %ld!!", __FUNCTION__, count, loader_data_partition_size);
        free(buf);
        close(fno);
        return FS_RETURN_ERROR;
    }

    free(buf);
    close(fno);   
    return FS_RETURN_OK;
}

static UI32_T  FS_GetKernelBootPartition(UI32_T *mtd_num_p)
{
    UI32_T      count;
    I32_T       fno;
    UI32_T      crc;
    char        *str;
    char        *buf;
    I32_T       ret;
    UI32_T      boot_addr;
    BOOL_T      found_loader_data_partition;
    UI8_T       loader_data_partition_name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];
    UI32_T      loader_data_partition_size;
    SYS_HWCFG_FlashEntry_T *flash_entry_p;
    
    if( mtd_num_p == NULL )
    {
        ERRORSTR("\r\n%s::ERROR!! bad parameter!!", __FUNCTION__);
        return FS_RETURN_BAD_PARA;
    }

    flash_entry_p = NULL;
    found_loader_data_partition = FALSE;
    while (NULL != (flash_entry_p = SYS_HWCFG_GetNextFlashPartitionEntry(flash_entry_p)))
    {
#if 0
llll=2;
        if (flash_entry_p->type == SYS_HWCFG_FLASH_TYPE_LOADERDATA)
        {
            sprintf(loader_data_partition_name,"%s%lu",FS_FLASH_MTD_BLOCK_PREFIX,flash_entry_p->mtdnum);
            loader_data_partition_size = flash_entry_p->size;
            found_loader_data_partition = TRUE;
            break;
        }
#else
#ifdef SYS_HWCFG_FLASH_TYPE_KERNEL
	if (flash_entry_p->type == SYS_HWCFG_FLASH_TYPE_KERNEL)
        {
	
		*mtd_num_p = flash_entry_p->mtdnum;
                ret = FS_RETURN_OK;
        }
#endif
#endif
    }
#if 0
dgeg=5;
    if (FALSE == found_loader_data_partition)
    {
        ERRORSTR("\r\n%s::ERROR!! no loader data partition found!!", __FUNCTION__);
        return FS_RETURN_ERROR;
    }

    fno = open(loader_data_partition_name, O_RDONLY, 0666);
    if( fno == -1 )
    {
        ERRORSTR("\r\n%s::ERROR!! Can not open %s!!", __FUNCTION__, loader_data_partition_name);
        return FS_RETURN_ERROR;
    }
    
    buf = malloc(loader_data_partition_size);
    if( buf == NULL )
    {
        ERRORSTR("\r\n%s::ERROR!! no buffer !!!", __FUNCTION__);
        close(fno);
        return FS_RETURN_NO_BUFFER;
    }
    
    count = read(fno, buf, loader_data_partition_size);
    if( count < loader_data_partition_size )
    {
        ERRORSTR("\r\n%s::ERROR!! read size %ld < %d!!", __FUNCTION__, count, loader_data_partition_size);
        ret = FS_RETURN_ERROR;
    }
    else
    {
        crc = *((UI32_T*)buf);
        str = buf+4;

        /* verify crc32 (or data has problem */        
        if( crc != (count = L_MATH_Crc32(0, (I8_T*)(buf+4), loader_data_partition_size-4)) )
        {
            ERRORSTR("\r\n%s:ERROR, check CRC error, %lx != %lx", __FUNCTION__, crc, count);
            ret = FS_RETURN_ERROR;
        }
        else
        {
            // DEBUGSTR("\r\n%s, check CRC OK, %lx ", __FUNCTION__, crc);
            ret = FS_RETURN_ERROR;

            while( str[0] != 0 )
            {
                // DEBUGSTR("\r\n\"%s\", length=%d", str, strlen(str));
                if( strncmp(str, FS_UBOOT_BOOT_CMD, strlen(FS_UBOOT_BOOT_CMD)) == 0)
                {
                    boot_addr = strtoul(str+strlen(FS_UBOOT_BOOT_CMD), NULL, 16);
                    if (TRUE == FS_GetMtdNumByStartAddress(boot_addr, mtd_num_p))
                    {
                        ret = FS_RETURN_OK;
                    }
                    else
                    {
                        ERRORSTR("\r\n%s:ERROR, not expect u-boot command (%s)",__FUNCTION__, str);
                    }
                    break;
                }
                /* pointer to next string 
                 */
                str += (strlen(str)+1);
            }
        }
    }

    free(buf);
    close(fno);
#endif   
    return ret;
}


UI32_T  FS_GetRootfsBootPartition(UI32_T *mtd_num)
{
    UI32_T      count;
    I32_T       fno;
    UI32_T      crc;
    char        *str;
    char        *buf;
    I32_T       ret = FS_RETURN_OK;
    BOOL_T      found_loader_data_partition;
    UI8_T       loader_data_partition_name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];
    UI32_T      loader_data_partition_size;
    SYS_HWCFG_FlashEntry_T *flash_entry_p;

    if( mtd_num == NULL )
    {
        ERRORSTR("\r\n%s::ERROR!! bad parameter!!", __FUNCTION__);
        return FS_RETURN_BAD_PARA;
    }
    
    *mtd_num = FS_MTD_NUMBER_NOT_EXIST;
    
    flash_entry_p = NULL;
    found_loader_data_partition = FALSE;
    while (NULL != (flash_entry_p = SYS_HWCFG_GetNextFlashPartitionEntry(flash_entry_p)))
    {
        if (flash_entry_p->type == SYS_HWCFG_FLASH_TYPE_LOADERDATA)
        {
            sprintf(loader_data_partition_name,"%s%lu",FS_FLASH_MTD_BLOCK_PREFIX,flash_entry_p->mtdnum);
            loader_data_partition_size = flash_entry_p->size;
            found_loader_data_partition = TRUE;
            break;
        }
    }

    if (FALSE == found_loader_data_partition)
    {
        ERRORSTR("\r\n%s::ERROR!! no loader data partition found!!", __FUNCTION__);
        return FS_RETURN_ERROR;
    }

    fno = open(loader_data_partition_name, O_RDONLY, 0666);
    if( fno == -1 )
    {
        ERRORSTR("\r\n%s::ERROR!! Can not open %s!!", __FUNCTION__, loader_data_partition_name);
        return FS_RETURN_ERROR;
    }
    
    buf = malloc(loader_data_partition_size);
    if( buf == NULL )
    {
        ERRORSTR("\r\n%s::ERROR!! no buffer !!!", __FUNCTION__);
        close(fno);
        return FS_RETURN_NO_BUFFER;
    }
    
    count = read(fno, buf, loader_data_partition_size);
    if( count < loader_data_partition_size )
    {
        ERRORSTR("\r\n%s::ERROR!! read size %ld < %d!!", __FUNCTION__, count, loader_data_partition_size);
    }
    else
    {
        crc = *((UI32_T*)buf);
        str = buf+4;

        /* verify crc32 (or data has problem */        
        if( crc != (count = L_MATH_Crc32(0, (I8_T*)(buf+4), loader_data_partition_size-4)) )
        {
            ERRORSTR("\r\n%s:ERROR, check CRC error, %lx != %lx", __FUNCTION__, crc, count);
        }
        else
        {
            while( str[0] != 0 )
            {
                if( strncmp(str, FS_UBOOT_BOOT_ARGS, strlen(FS_UBOOT_BOOT_ARGS)) == 0)
                {
                    *mtd_num = atoi(str+strlen(FS_UBOOT_BOOT_ARGS));
                    break;
                }
                /* pointer to next string 
                 */
                str += (strlen(str)+1);
            }
        }
    }
    
    free(buf);
    close(fno);
    if( *mtd_num == FS_MTD_NUMBER_NOT_EXIST ) 
    {
        ret = FS_RETURN_ERROR;
    }
   
    return ret;
}

static BOOL_T FS_ReadKernelHeader(UI32_T mtd_num, image_header_t *header_p)
{
    I32_T  fd;
    char   mtd_name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN];
    UI32_T read_len;
    
    sprintf(mtd_name,"%s%lu",FS_FLASH_MTD_BLOCK_PREFIX,mtd_num);
    fd = open(mtd_name, O_RDONLY, 0666);
    if( fd == -1 )
        return FALSE;
    read_len = read(fd, header_p, sizeof(image_header_t));
    if( read_len < sizeof(image_header_t) )
    {
        close(fd);
        return FALSE;
    }
    if (FS_RETURN_OK != FS_CheckKernelHeader((UI8_T*)header_p, sizeof(image_header_t)))
    {
        close(fd);
        return FALSE;
    }
    close(fd);
    return TRUE;
}


static BOOL_T FS_ReadRootfsHeader(UI32_T mtd_num, struct cramfs_super *header_p)
{
    I32_T  fd;
    char   mtd_name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN];
    UI32_T read_len;
#ifdef ECN430_FB2
    sprintf(mtd_name,"%s",FS_CFCARD_PREFIX);
#else
    sprintf(mtd_name,"%s%lu",FS_FLASH_MTD_BLOCK_PREFIX,mtd_num);
#endif
    fd = open(mtd_name, O_RDONLY, 0666);
    if( fd == -1 )
        return FALSE;
    read_len = read(fd, header_p, sizeof(struct cramfs_super));
    if( read_len < sizeof(struct cramfs_super) )
    {
        close(fd);
        return FALSE;
    }
    if (FS_RETURN_OK != FS_CheckRootfsHeader((UI8_T*)header_p, sizeof(struct cramfs_super)))
    {
        close(fd);
        return FALSE;
    }
    close(fd);
    return TRUE;
}

static BOOL_T FS_BuildKernelFile(char file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN], UI32_T mtd_num, BOOL_T startup)
{
    FILE    *fd;
    char    file_path[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];
    char    write_buf[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN];
    UI32_T  length;

    sprintf(file_path,"%s%s",FS_SAVE_FOLDER,file_name);
    
    fd = fopen(file_path, "w");
    if( fd == NULL )
    {
        return FALSE;
    }
    sprintf(write_buf,"%lu",mtd_num);
    length = strlen(write_buf);
    if (fwrite(write_buf, 1, length, fd)<length)
    {
        fclose(fd);
        return FALSE;
    }
    fclose(fd);
        
    chown(file_path, FS_TYPE_STARTUP_TO_UID(FS_FILE_TYPE_KERNEL,startup), -1);
    return TRUE;
}

static BOOL_T FS_BuildStartupKernelFile(char file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN])
{
    UI32_T  mtd_num;
    FILE    *fd;
    char    file_path[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];
    char    write_buf[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN];
    UI32_T  length;
    image_header_t  kernel_header;
    UI32_T  random_num;

   // printf("FS_BuildStartupKernelFile file_name=%s\n",file_name);
    mtd_num = FS_MTD_NUMBER_NOT_EXIST;
    if (FS_RETURN_OK != FS_GetKernelBootPartition(&mtd_num))
    {
        return FALSE;
    }
    
    if (FALSE == FS_ReadKernelHeader(mtd_num, &kernel_header))
    {
        printf("%s %lu FS_ReadKernelHeader return false\n",__FUNCTION__,__LINE__);
        return FALSE;
    }

    random_num = random();
    sprintf(file_path,"%s%s%lu",FS_SAVE_FOLDER,FS_RANDOM_KERNEL_NAME_PREFIX,random_num);
    while (fd = fopen(file_path,"r") != NULL)
    {
        fclose(fd);
        random_num = random();
        sprintf(file_path,"%s%s%lu",FS_SAVE_FOLDER,FS_RANDOM_KERNEL_NAME_PREFIX,random_num);
    }
    sprintf(file_name,"%s%lu",FS_RANDOM_KERNEL_NAME_PREFIX,random_num);
    
    fd = fopen(file_path, "w");
    if( fd == NULL )
    {
        return FALSE;
    }
    sprintf(write_buf,"%lu",mtd_num);
    length = strlen(write_buf);
    if (fwrite(write_buf, 1, length, fd)<length)
    {
        fclose(fd);
        return FALSE;
    }
    fclose(fd);
    chown(file_path, FS_TYPE_STARTUP_TO_UID(FS_FILE_TYPE_KERNEL,TRUE), -1);
    return TRUE;
}

static BOOL_T FS_BuildRootfsFile(char file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN], UI32_T mtd_num, BOOL_T startup)
{
    FILE    *fd;
    char    file_path[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];
    char    write_buf[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN];
    UI32_T  length;

    sprintf(file_path,"%s%s",FS_SAVE_FOLDER,file_name);
    
    fd = fopen(file_path, "w");
    if( fd == NULL )
    {
        return FALSE;
    }
#ifndef ECN430_FB2
    sprintf(write_buf,"%lu",mtd_num);
    length = strlen(write_buf);
    if (fwrite(write_buf, 1, length, fd)<length)
    {
        fclose(fd);
        return FALSE;
    }
#endif
    fclose(fd);
        
    chown(file_path, FS_TYPE_STARTUP_TO_UID(FS_FILE_TYPE_RUNTIME,startup), -1);
    return TRUE;
}

static BOOL_T FS_BuildStartupRootfsFile(char file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN])
{
    UI32_T mtd_num;
    struct cramfs_super rootfs_header;
    FILE   *fd;
    char   file_path[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];
    char   write_buf[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN];
    UI32_T length;
    UI32_T random_num;
#if !defined (ECN430_FB2) || !defined (ES3628BT_FLF_ZZ) || !defined(ASF4526B_FLF_P5)
    if (FS_RETURN_OK != FS_GetRootfsBootPartition(&mtd_num))
    {
        return FALSE;
    }
#endif
    
    if (FALSE == FS_ReadRootfsHeader(mtd_num, &rootfs_header))
    {
        return FALSE;
    }

    random_num = random();
    sprintf(file_path,"%s%s%lu",FS_SAVE_FOLDER,FS_RANDOM_ROOTFS_NAME_PREFIX,random_num);
    while (fd = fopen(file_path,"r") != NULL)
    {
        fclose(fd);
        random_num = random();
        sprintf(file_path,"%s%s%lu",FS_SAVE_FOLDER,FS_RANDOM_ROOTFS_NAME_PREFIX,random_num);
    }
    sprintf(file_name,"%s%lu",FS_RANDOM_ROOTFS_NAME_PREFIX,random_num);
    
    fd = fopen(file_path, "w");
    if( fd == NULL )
    {
        return FALSE;
    }
#if !defined (ECN430_FB2) || !defined (ES3628BT_FLF_ZZ) || !defined(ASF4526B_FLF_P5)
    sprintf(write_buf,"%lu",mtd_num);
    length = strlen(write_buf);
    if (fwrite(write_buf, 1, length, fd)<length)
    {
        fclose(fd);
        return FALSE;
    }
#endif
    fclose(fd);
    chown(file_path, FS_TYPE_STARTUP_TO_UID(FS_FILE_TYPE_RUNTIME,TRUE), -1);
    return TRUE;
}


static BOOL_T FS_BuildFileHeaderFromFile(char file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1])
{
    UI8_T name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];
    UI8_T tmp_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];
    struct stat     file_info;
    FS_FileHeader_T *file_header_ptr;
    
    //printf("enter FS_BuildHeaderFromFile file_name=%s!\n",file_name);
    sprintf((char*)name,"%s%s",FS_SAVE_FOLDER,file_name);
    if( (stat((char*)name, &file_info)!=0) || (!S_ISREG(file_info.st_mode)) )
    {
        if( S_ISDIR(file_info.st_mode) )
        {
            //sprintf(name,"rm -rf %s%s",FS_SAVE_FOLDER,file_entry->d_name);
            DEBUGSTR("\r\n%s: Has unknown folder, delete it(%s)",__FUNCTION__,name);
            //system(name);
        }
        else 
        {
            DEBUGSTR("\r\n%s: Has unknown file %s(%x), delete it",__FUNCTION__,name, file_info.st_mode);
            unlink(name);
        }    
        return FALSE;
    }

    if (FS_GenericFilenameCheck((UI8_T*)file_name, FS_UID_TO_TYPE(file_info.st_uid)) != FS_RETURN_OK)
    {
        DEBUGSTR("\r\n%s:ERROR, FS_GenericFilenameCheck(\"%s\", %d)return fail,delete it",
            __FUNCTION__,file_name, FS_UID_TO_TYPE(file_info.st_uid));
        unlink(name);
        return FALSE;
    }

    /* Create file header and insert into header list
     */
    if((file_header_ptr = (FS_FileHeader_T*)FS_OM_AllocateFileHeaderBuffer()) == NULL)
    {
        ERRORSTR("\r\n%s:ERROR!! for FS_BUF_Allocate return NULL",__FUNCTION__);
        FS_OM_SetInitStatus(FS_RETURN_NO_BUFFER);
        
        return FALSE;
    }
        
    strncpy((char*)file_header_ptr->file_name, file_name, SYS_ADPT_FILE_SYSTEM_NAME_LEN);
    strncpy((char*)file_header_ptr->file_comment, file_name, FS_FILE_COMMENT_LENGTH);
    file_header_ptr->file_type    = FS_UID_TO_TYPE(file_info.st_uid);
    file_header_ptr->startup      = FS_UID_TO_STARTUP(file_info.st_uid);
    file_header_ptr->create_time  = file_info.st_mtime;
    file_header_ptr->magic_number = FS_SPECIAL_MAGIC_NUMBER;
    //printf("file_header_ptr->file_type =%d\n",file_header_ptr->file_type);
    if (file_header_ptr->file_type == FS_FILE_TYPE_KERNEL ||
        file_header_ptr->file_type == FS_FILE_TYPE_RUNTIME)
    {
        FILE           *fd;
        UI32_T         read_count;
        UI8_T          read_buf[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN];
        UI32_T         mtd_num;

        fd = fopen((char*)name, "r");
        if( fd == NULL )
        {
            DEBUGSTR("\r\n%s, Can not open file %s.\n",__FUNCTION__,name);
            return FALSE;
        }
        read_count = fread(read_buf, 1, SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN, fd);
        if ( read_count == 0 )
        {
            DEBUGSTR("\r\n%s::Read file failed %s.\n",__FUNCTION__,name);
            return FALSE;
        }
        read_buf[read_count] = 0;
	if (file_header_ptr->file_type == FS_FILE_TYPE_KERNEL)
	{
        	mtd_num = atoi((char*)read_buf);
        	if (mtd_num == 0)
        	{
            		DEBUGSTR("\r\n%s::mtd_num = 0.\n",__FUNCTION__);
            		return FALSE;
        	}
	}
        if (file_header_ptr->file_type == FS_FILE_TYPE_KERNEL)
        {
            image_header_t header;
            //printf("enter FS_ReadKernelHeader !\n");
            if (TRUE == FS_ReadKernelHeader(mtd_num, &header))
            {
                file_header_ptr->file_size = header.ih_size;
                file_header_ptr->mtd_num   = mtd_num;
            }
            else
            {
                DEBUGSTR("\r\n%s::FS_ReadKernelHeader fail. mtd_num = %lu\n",__FUNCTION__,mtd_num);
                return FALSE;            
            }
        }
        else if (file_header_ptr->file_type == FS_FILE_TYPE_RUNTIME)
        {
            struct cramfs_super header;
            mtd_num = FS_MTD_NUMBER_NOT_EXIST;
            if (TRUE == FS_ReadRootfsHeader(mtd_num, &header))
            {
                file_header_ptr->file_size = header.size;
                //file_header_ptr->mtd_num   = mtd_num;
            }
            else
            {
                DEBUGSTR("\r\n%s::FS_ReadRootfsHeader fail. mtd_num = %lu\n",__FUNCTION__,mtd_num);
                return FALSE;            
            }            
        }
    }
    else
    {
        file_header_ptr->file_size = file_info.st_size;
        file_header_ptr->mtd_num = FS_MTD_NUMBER_NOT_EXIST;        
    }

    FS_OM_AddFileHeader(file_header_ptr);
    //printf("file_header_ptr->startup =%d\n",file_header_ptr->startup);
    if (TRUE == file_header_ptr->startup)
    {
        if (TRUE == FS_OM_GetStartupName(file_header_ptr->file_type, tmp_name))
        {
            /* Already have startup file, clear this one
             */
	    //printf("already have startup file !\n");
            file_header_ptr->startup = FALSE;
            chown((char*)name, FS_TYPE_STARTUP_TO_UID(file_header_ptr->file_type,FALSE),-1);
        }
        else
        {
	    //printf("before FS_OM_SetStartupName !\n");
            FS_OM_SetStartupName(file_header_ptr->file_type, file_header_ptr->file_name);
        }
    }
    return TRUE;
}

static BOOL_T FS_BuildAllFileHeader(void)
{
    DIR*             dir;
    struct dirent*   file_entry;
	
    //printf("enter FS_BuildFileHeader !\n");
    dir = opendir(FS_SAVE_FOLDER);
    if( dir == NULL ) 
    {
        DEBUGSTR("\r\n%s: First time open directory: %s not exist, create it.",__FUNCTION__,FS_SAVE_FOLDER);
        if( (mkdir(FS_SAVE_FOLDER, 0666) != 0) ||
            ((dir = opendir(FS_SAVE_FOLDER))== NULL) )
        {
            FS_OM_SetInitStatus(FS_RETURN_NOT_READY);
            ERRORSTR("\r\n%s: Can not open directory: %s",__FUNCTION__,FS_SAVE_FOLDER);
            return FALSE;
        }
    }
    //printf("before while file_entery readdir !\n");
    while( (file_entry = readdir(dir)) != NULL )
    {
        DEBUGSTR("\r\nBuild file list: %s",file_entry->d_name);
        
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

#define FS_Debug(a)         (FALSE)
//#define FS_Debug(a)         (TRUE)
#define FS_SetDebugFlag(a)

#if 0
BOOL_T  FS_Debug(UI32_T flag)
{
    return (FS_OM_GetDebugFlag( & flag)? TRUE: FALSE;
}

void    FS_SetDebugFlag(UI32_T flag)
{
    FS_DebugFlag = flag;
    return;
}

void    FS_GetDebugFlag(UI32_T *flag)
{
    *flag   = FS_DebugFlag;
    return;
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
    BOOL_T              result;

#if 0    
    FS_TaskId   = 0;
    fs_all_file_checksum_verified   = FALSE;
#if (SYS_CPNT_STACKING == TRUE)
    /*
     * Init FS control block,
     * SYS_TYPE_STACKING_TRANSITION_MODE = 0
     */
    memset(&fs_control, 0, sizeof(fs_control));
    
    /*
     * Create a semaphore for remote access
     * Remote operation must be carried out in sequence
     */
    if(SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO, &FS_Communication_Sem) != SYSFUN_OK)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_FS, FS_INIT_FUNC_NO, EH_TYPE_MSG_SEMPH_CREATE, SYSLOG_LEVEL_EMERG);
        while(1);
    }
#endif

    unit = DUMMY_DRIVE;
    FS_InitStatus       = FS_RETURN_OK;
    FS_FileHeaderList   = NULL;
    FS_SetDebugFlag( (UI32_T)FS_DEBUG_FLAG_NONE );

    fs_shutdown_flag = FALSE;

    /* create semaphore. Wait forever if fail */
    if(SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO, &FS_Semaphore) != SYSFUN_OK)
    {
        ERRORSTR("\r\n%s:ERROR, SYSFUN_CreateSem() fail",__FUNCTION__);
        while(1);
    }    

    FS_FileTypeVisibility[FS_FILE_TYPE_DIAG]            = FS_FILE_TYPE_VISIBLE_DIAG;
    FS_FileTypeVisibility[FS_FILE_TYPE_RUNTIME]         = FS_FILE_TYPE_VISIBLE_RUNTIME;
    FS_FileTypeVisibility[FS_FILE_TYPE_SYSLOG]          = FS_FILE_TYPE_VISIBLE_SYSLOG;
    FS_FileTypeVisibility[FS_FILE_TYPE_CMDLOG]          = FS_FILE_TYPE_VISIBLE_CMDLOG;
    FS_FileTypeVisibility[FS_FILE_TYPE_CONFIG]          = FS_FILE_TYPE_VISIBLE_CONFIG ;
    FS_FileTypeVisibility[FS_FILE_TYPE_POSTLOG]         = FS_FILE_TYPE_VISIBLE_POSTLOG;
    FS_FileTypeVisibility[FS_FILE_TYPE_PRIVATE]         = FS_FILE_TYPE_VISIBLE_PRIVATE;
    FS_FileTypeVisibility[FS_FILE_TYPE_CERTIFICATE]     = FS_FILE_TYPE_VISIBLE_CERTIFICATE;
    FS_FileTypeVisibility[FS_FILE_TYPE_ARCHIVE]         = FS_FILE_TYPE_VISIBLE_ARCHIVE;
    FS_FileTypeVisibility[FS_FILE_TYPE_BINARY_CONFIG]   = FS_FILE_TYPE_VISIBLE_BINARY_CONFIG;
    FS_FileTypeVisibility[FS_FILE_TYPE_PUBLIC]          = FS_FILE_TYPE_VISIBLE_PUBLIC;
#endif
    
    FS_OM_InitateVariables();
    
    DEBUGSTR("\r\n%s before FS_BuildFileHeader().\n",__FUNCTION__);
    //printf("\r\n%s before FS_BuildFileHeader().\n",__FUNCTION__);

    result = FS_BuildAllFileHeader();
    
    if( result == FALSE )
    {
        ERRORSTR("\r\nFS_Init::ERROR!! for FS_BuildFileHeader return FALSE %lx",FS_OM_GetInitStatus());
        while(1);
        return;
    }
    //printf("Before FS_StartupVerification !\n");
    FS_StartupVerification();
    DEBUGSTR("\r\n%s is done.\n",__FUNCTION__);
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
} /* End of FS_Create_InterCSC_Relation */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_SetTaskId
 * ------------------------------------------------------------------------
 * FUNCTION : This function will record the FS task ID.
 * INPUT    : task_id   -- the FS task ID
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
#if 0

void    FS_SetTaskId(UI32_T task_id)
{
    FS_OM_SetTaskId(task_id);
    return;
} /* End of FS_SetTaskId */
#endif

static BOOL_T FS_GetMtdNumByStartAddress(UI32_T start_addr, UI32_T *mtd_num_p)
{
    SYS_HWCFG_FlashEntry_T *flash_entry_p = NULL;

    while (NULL != (flash_entry_p = SYS_HWCFG_GetNextFlashPartitionEntry(flash_entry_p)))
    {
        if (start_addr == flash_entry_p->address)
        {
            *mtd_num_p = flash_entry_p->mtdnum;
            return TRUE;
        }
    }
    return FALSE;
}


/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_StartupVerification
 * ------------------------------------------------------------------------
 * FUNCTION : This function will verify the existence for the startup file.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void FS_StartupVerification(void)
{
    FS_FileHeader_T file_header;
    char            *filetype_str;
    UI32_T          filetype;
    char            name[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];
    UI32_T          mtd_num;

    for (filetype = FS_FILE_TYPE_SUBFILE+1; filetype < FS_FILE_TYPE_TOTAL; filetype++)
    {
        switch (filetype)
        {
            /* Following need to have a startup file. 
             */
            case FS_FILE_TYPE_KERNEL:
                filetype_str = "Kernel Code";
                break;
            case FS_FILE_TYPE_RUNTIME:
                filetype_str = "Runtime Code";
                break;
            case FS_FILE_TYPE_CONFIG:
                filetype_str = "Configuration File";
                break;
            default:
                filetype_str = "";
                break;
        }
	//printf("this filetype_str =%s\n",filetype_str);
        if (FALSE == FS_OM_GetStartupName(filetype, (UI8_T*)name))
        {
            /* If not kernel or rootfs startup file is found, create one
             */
	    //printf("FS_OM_GetStartupName is error !\n");
            if (FS_FILE_TYPE_KERNEL == filetype)
            {
		//printf("before FS_BuildStartupKernelFile name=%s\n",name);
                if (TRUE == FS_BuildStartupKernelFile(name))
                {
                    FS_BuildFileHeaderFromFile(name);
                }
            }
            else if (FS_FILE_TYPE_RUNTIME == filetype)
            {
		//printf("before FS_BuildStartupRootfsFile name=%s\n",name);
                if (TRUE == FS_BuildStartupRootfsFile(name))
                {
                    FS_BuildFileHeaderFromFile(name);
                }
            }
            else
            {
                if (filetype_str[0] != 0) /* need starup file for this file tpye */
                {
		    //printf("filetype_str =%s\n",filetype_str);
                    file_header.file_type = filetype;
                    if (TRUE == FS_OM_GetFirstFileHeaderByType(&file_header))
                    {
                        FS_OM_SetStartupName(filetype, file_header.file_name);
                    }
                    else
                    {
                        /* This file type should have a startup file 
                         */
                        DEBUGSTR("\r\n%s, need Startup file for %s",__FUNCTION__, filetype_str);
                    }
                }
            }
        }
        else
        {
	    //printf("there is have startup file !\n");
            mtd_num = FS_MTD_NUMBER_NOT_EXIST;
            if (FS_FILE_TYPE_KERNEL == filetype)
            {  
                if (FS_RETURN_OK != FS_GetKernelBootPartition(&mtd_num))
                {
                    ERRORSTR("\r\n%s:ERROR, FS_GetKernelBootPartition fail.",__FUNCTION__);
                    return;
                }
                file_header.mtd_num = mtd_num;
                if (FALSE == FS_OM_GetFirstFileHeaderByMtdNum(&file_header))
                {
                    if (TRUE == FS_BuildStartupKernelFile(name))
                    {
                        FS_BuildFileHeaderFromFile(name);
                    }
                }
                else
                {
                    if (FALSE == file_header.startup)
                    {
                        FS_OM_SetStartupName(filetype, file_header.file_name);
                    }
                }
            }
            else if (FS_FILE_TYPE_RUNTIME == filetype)
            {
#if !defined (ECN430_FB2) || !defined (ES3628BT_FLF_ZZ) || !defined(ASF4526B_FLF_P5)
                if (FS_RETURN_OK != FS_GetRootfsBootPartition(&mtd_num))
                {
                    ERRORSTR("\r\n%s:ERROR, FS_GetRootfsBootPartition fail.",__FUNCTION__);
                    return;
                }
                file_header.mtd_num = mtd_num;
#endif
                if (FALSE == FS_OM_GetFirstFileHeaderByMtdNum(&file_header))
                {
                    if (TRUE == FS_BuildStartupRootfsFile(name))
                    {
                        FS_BuildFileHeaderFromFile(name);
                    }
                }
                else
                {
                    if (FALSE == file_header.startup)
                    {
                        FS_OM_SetStartupName(filetype, file_header.file_name);
                    }
                }
            }        
        }
    } /* End of for */
    return;
} /* End of FS_StartupVerification */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_FilenameCheck
 * ------------------------------------------------------------------------
 * FUNCTION : This function checks the file name to be composed with the valid
 *            characters or not.
 * INPUT    : filename      -- a string specified by users
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E; FS_RETURN_OK returned if valid otherwise
 *            FS_RETURN_ERROR if filename contains the invalid character.
 * NOTE     : FS_RETURN_ERROR returned if filename is a NULL string
 * ------------------------------------------------------------------------
 */
UI32_T  FS_FilenameCheck(UI8_T *filename)
{
    BOOL_T  valid, pass;
    UI8_T   ch, enum_ch, range, ch_index, i;
    char    *check[FS_FILENAME_CHECK_VALID_NUM_OF_RULE] =
            {   FS_FILENAME_CHECK_VALID_ENUM,
                FS_FILENAME_CHECK_VALID_RANGE_1,
                FS_FILENAME_CHECK_VALID_RANGE_2,
                FS_FILENAME_CHECK_VALID_RANGE_3,
                FS_FILENAME_CHECK_VALID_RANGE_4,
            };

    if (filename[0] == 0)
        valid = FALSE;
    else
        valid = TRUE;

    ch_index = 0;
    while (valid && ( (ch = filename[ch_index]) != 0) )
    {

        /* Range check */
        range   = 1;
        pass    = FALSE;
        while ( (!pass) && (range < FS_FILENAME_CHECK_VALID_NUM_OF_RULE) )
        {
            if ( check[range][0] != 0 )
            {
                if ( (ch >= check[range][0]) && (ch <= check[range][1]) )
                    pass = TRUE;
            }
            range++;
        }
        if (!pass)
            valid = FALSE;

        /* Enumeration check */
        range = 0;
        i = 0;
        while ( (!valid) && ( (enum_ch = check[range][i]) != 0) )
        {
            if (ch == enum_ch)
                valid = TRUE;
            i++;
        }
        ch_index++;
    }        

    if (valid)
        return FS_RETURN_OK;
    else
    {
        ERRORSTR("\r\n%s:ERROR, Filename(%s) check fail.",__FUNCTION__,filename);
        return FS_RETURN_ERROR;
    }
} /* End of FS_FilenameCheck */

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
    BOOL_T  valid;
    char    *check, ch;
    UI8_T   i;

    valid = FALSE;
    check = FS_FILENAME_CHECK_VALID_INVIS;

    if (    (filename[0] != 0)
         && (strlen((char*)filename) <= FS_MAX_FILE_NAME_LENGTH) )
    {
        if ( (file_type > FS_FILE_TYPE_SUBFILE) && (file_type < FS_FILE_TYPE_TOTAL) )
        {
            if ( FS_FileTypeVisibility[file_type] )
            {
                /* Visible file */
                if ( FS_FilenameCheck( filename ) == FS_RETURN_OK )
                {
                    valid = TRUE;
                }
            }
            else
            {
                /* Invisible file */
                i = 0;
                while ( (!valid) && (ch = check[i]) )
                {
                    if ( filename[0] == ch  )
                    {
                        valid = TRUE;
                    }
                    else
                        i++;
                }
                if (valid)
                {
                    if ( FS_FilenameCheck( (filename+1)) != FS_RETURN_OK )
                    {
                        valid = FALSE;
                    }
                }
            }
        }
    }

    if (valid)
        return FS_RETURN_OK;
    else
    {
        ERRORSTR("\r\n%s:ERROR, Filename(%s), type(%ld) check fail.",__FUNCTION__,filename, file_type);
        return FS_RETURN_ERROR;
    }
} /* End of FS_GenericFilenameCheck */


/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetImageType
 * ------------------------------------------------------------------------
 * FUNCTION : This function check the image type of the input data
 * INPUT    : data_p     -- pointer to the image header
 *            length     -- data length
 * OUTPUT   : None
 * RETURN   : the image type of the data, only two kind of image can
 *            be validated now, kernel and rootfs
 *            FS_FILE_TYPE_KERNEL   --- kernel image
 *            FS_FILE_TYPE_RUNTIME  --- root file system
 *            FS_FILE_TYPE_TOTAL    --- cant't validate
 * NOTE     : return FS_FILE_TYPE_TOTAL, just mean that not kernel or rootfs
 *            it may be config file or other valid files
 * ------------------------------------------------------------------------
 */
FS_File_Type_T  FS_GetImageType(UI8_T *data_p, UI32_T length)
{
    if (length >= sizeof(struct cramfs_super))
    {
        struct cramfs_super *hdr = (struct cramfs_super *)data_p;
        
	    if ((hdr->magic == CRAMFS_MAGIC) && (memcmp((char*)hdr->signature,CRAMFS_SIGNATURE,strlen(CRAMFS_SIGNATURE))==0))
        {   
            return FS_FILE_TYPE_RUNTIME;
        }
    }

    if (length >= sizeof(image_header_t))
    {
        image_header_t *hdr = (image_header_t*)data_p;
        
	    if ((hdr->ih_magic == IH_MAGIC) && (hdr->ih_type == IH_TYPE_KERNEL))
        {   
            return FS_FILE_TYPE_KERNEL;
        }
    }
    
    return FS_FILE_TYPE_TOTAL;
}


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
    FS_File_Type_T file_type;

    file_type = FS_GetImageType(data_p, length);
    if (file_type == FS_FILE_TYPE_RUNTIME)
    {
        /* kh_shi: add checking here
         */
        return TRUE;
    }
    else if (file_type == FS_FILE_TYPE_RUNTIME)
    {
        /* kh_shi: add checking here
         */
        return TRUE;
    }
    return TRUE;
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
    FS_File_Type_T file_type;

    file_type = FS_GetImageType(data_p, length);
    if (file_type == FS_FILE_TYPE_RUNTIME)
    {
        return TRUE; /* not crc field in cramfs header */
    }
    else if (file_type == FS_FILE_TYPE_RUNTIME)
    {
        if (FS_RETURN_OK == FS_CheckKernelHeader(data_p, length))
            return TRUE;
        else return FALSE;
    }
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
    FS_File_Type_T      image_type;
    image_header_t      *kernel_hdr_p;
    struct cramfs_super *rootfs_hdr_p;
    UI32_T              crc,cramfs_crc;
    UI32_T              data_len;

    image_type = FS_GetImageType(data_p, length);
    if (FS_FILE_TYPE_KERNEL == image_type)
    {
        kernel_hdr_p = (image_header_t*)data_p;
        data_len = kernel_hdr_p->ih_size;
        if (data_len > (length - sizeof(image_header_t)))
        {
            return FALSE;
        }
        crc = L_MATH_Crc32(0, data_p+sizeof(image_header_t), data_len);
        if (crc != kernel_hdr_p->ih_dcrc)
        {
            return FALSE;
        }
        return TRUE;
    }
    else if (FS_FILE_TYPE_RUNTIME == image_type)
    {
        rootfs_hdr_p = (struct cramfs_super *)data_p;
        data_len = rootfs_hdr_p->size;
        if (data_len > length)
        {
            return FALSE;
        }
        cramfs_crc = rootfs_hdr_p->fsid.crc;
        rootfs_hdr_p->fsid.crc = 0;
        crc = L_MATH_Crc32(0, data_p, data_len);
        rootfs_hdr_p->fsid.crc = cramfs_crc;
        
        /* cramfs crc field may be little endian
         */
        if (crc != cramfs_crc && crc != L_STDLIB_Swap32(cramfs_crc))
        {
            printf("%s: crc error %lu != %lu\n",__FUNCTION__,crc,cramfs_crc);
            return FALSE;
        }
        return TRUE;        
    }
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetStorageFreeSpace
 * ------------------------------------------------------------------------
 * FUNCTION : This function returns the available free space of the file system.
 * INPUT    : drive 			        -- unit id or blade number
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

	/* Local access */
#if (SYS_CPNT_STACKING == TRUE)

    FS_File_Summary_Packet_T  fs_summary_packet;
    
    if (    (drive == DUMMY_DRIVE)
         || (drive == SYS_VAL_LOCAL_UNIT_ID)
         || (drive == FS_OM_GetControlDriverId())
       )
#endif
    {
        if( FS_Debug(FS_DEBUG_FLAG_DBGMSG) )
            printf("\r\n%s() Drive: %ld\r\n", __FUNCTION__, drive);

//        FS_SETBUSY();

        if (statfs(FS_SAVE_FOLDER, &s) != 0) 
        {
            ERRORSTR("\r\n%s: statfs error\r\n", __FUNCTION__);
            *total_size_of_free_space = 0;
            return_value = FS_RETURN_ERROR;
        }
        else
        {
            /*DEBUGSTR("\r\nFS_GetStorageFreeSpace: type=%x,f_bsize=%d,f_blocks=%ld,f_bfree=%ld,f_bavail=%ld",
                s.f_type,s.f_bsize,s.f_blocks,s.f_bfree,s.f_bavail); 
            DEBUGSTR("\r\n Freespace is %ld",s.f_bavail * s.f_bsize);*/
            *total_size_of_free_space = (s.f_bavail * s.f_bsize);
            return_value = FS_RETURN_OK;
        }

//        FS_SETNOTBUSY();
        return return_value;
    }
#if (SYS_CPNT_STACKING == TRUE)
    else /* Remote access */
    {
        
        if( FS_Debug(FS_DEBUG_FLAG_DBGMSG) )
            printf("\r\n%s() Drive: %ld", __FUNCTION__, drive);

        FS_SETBUSY();
        return_value = FS_GetRemoteFileSystemSummary(drive, &fs_summary_packet);

        if(return_value == FS_RETURN_OK)
        {
            *total_size_of_free_space = fs_summary_packet.total_space_free;
            FS_SETNOTBUSY();
        }
        else
        {
            FS_SETNOTBUSY();
            printf("\r\n%s(): %s\r\n", __FUNCTION__, fs_error_messages[return_value]);
        }
    }
    return return_value;
#endif
}



/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_GetFileStorageUsageSize
 * ------------------------------------------------------------------------
 * FUNCTION : This function returns the usage size of the specified file.
 * INPUT    : drive 			        -- unit id or blade number
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
        ERRORSTR("\r\n%s:ERROR, bad parameter", __FUNCTION__);
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

        strncpy((char*)file_header.file_name,(char*)file_name,SYS_ADPT_FILE_SYSTEM_NAME_LEN+1);

        if (FALSE == FS_OM_GetFileHeaderByName(&file_header,FS_FILE_TYPE_MASK_ALL))
        {
            ERRORSTR("\r\n%s: can not find the file %s\r\n", __FUNCTION__, file_name);
            *usage_size = 0;
            return FS_RETURN_FILE_NEX;
        }

        if (FS_Debug(FS_DEBUG_FLAG_DBGMSG))
        {
            printf("\r\nFile Name:%s (on unit %d)", file_name, (int)drive);
        }
        
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
        
        if (return_value == FS_RETURN_FILE_NEX)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_FS, FS_GETFILESTORAGEUSAGESIZE_FUNC_NO, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "File");
        }

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
 * INPUT    : drive 			        -- unit id or blade number
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
    
        strncpy((char*)file_header.file_name,(char*)file_attr->file_name,SYS_ADPT_FILE_SYSTEM_NAME_LEN+1);

        if (FALSE == FS_OM_GetFileHeaderByName(&file_header,file_attr->file_type_mask))
        {
            ERRORSTR("\r\n%s: can not find the file %s\r\n", __FUNCTION__, file_attr->file_name);
            return FS_RETURN_FILE_NEX;
        }

        strncpy((char*)file_attr->file_name, (char*)file_header.file_name, SYS_ADPT_FILE_SYSTEM_NAME_LEN + 1);
        strncpy((char*)file_attr->file_comment, (char*)file_header.file_comment, FS_FILE_COMMENT_LENGTH + 1);
        file_attr->create_time  = file_header.create_time;
        file_attr->file_type    = file_header.file_type;
        file_attr->file_size    = file_header.file_size;
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
 * ROUTINE NAME - FS_GetNextFileInfo
 * ------------------------------------------------------------------------
 * FUNCTION : This function will get the information of the next file
 * INPUT    : drive 			        -- unit id or blade number
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

            strncpy((char*)file_header.file_name,(char*)file_attr->file_name,SYS_ADPT_FILE_SYSTEM_NAME_LEN+1);

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
            strncpy((char*)file_attr->file_name, (char*)file_header.file_name, SYS_ADPT_FILE_SYSTEM_NAME_LEN+1);
            strncpy((char*)file_attr->file_comment, (char*)file_header.file_comment, FS_FILE_COMMENT_LENGTH+1);
            file_attr->file_type    = file_header.file_type;
            file_attr->file_size    = file_header.file_size;
            file_attr->create_time  = file_header.create_time;
            file_attr->startup_file = file_header.startup;            

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
                    EH_MGR_Handle_Exception1(SYS_MODULE_FS, FS_GETNEXTFILEINFO_FUNC_NO, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "File");
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

    strncpy((char*)file_header.file_name,(char*)file_attr->file_name,SYS_ADPT_FILE_SYSTEM_NAME_LEN+1);
    if (FALSE == FS_OM_GetNextFileHeaderByName(&file_header,file_attr->file_type_mask))
    {
        DEBUGSTR("\r\n%s:, File not found: %s \r\n", __FUNCTION__, file_attr->file_name);
        return FS_RETURN_FILE_NEX;
    }

    strncpy((char*)file_attr->file_name, (char*)file_header.file_name, SYS_ADPT_FILE_SYSTEM_NAME_LEN+1);
    strncpy((char*)file_attr->file_comment, (char*)file_header.file_comment, FS_FILE_COMMENT_LENGTH+1);
    file_attr->file_type    = file_header.file_type;
    file_attr->create_time  = file_header.create_time;
    file_attr->file_size    = file_header.file_size;
    file_attr->startup_file = file_header.startup;

    return FS_RETURN_OK;
#endif /* SYS_CPNT_STACKING */
} /* End of FS_GetNextFileInfo() */


/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_EraseFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function will erase a file
 * INPUT    : unit          -- the unit id
 *            file_name     -- the file to delete
 *            privilege     -- TRUE : allow to delete the protected file
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static  UI32_T  FS_EraseFile(UI32_T unit, UI8_T *file_name, BOOL_T privilege)
{
    FS_FileHeader_T     file_header;
    char                name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];

    /* File system initialized OK? */
    if (FS_OM_GetInitStatus() != FS_RETURN_OK)
        return FS_RETURN_NOT_READY;

    strncpy((char*)file_header.file_name,(char*)file_name,SYS_ADPT_FILE_SYSTEM_NAME_LEN+1);

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

    if( TRUE == file_header.startup &&
        (FS_FILE_TYPE_KERNEL  == file_header.file_type ||
         FS_FILE_TYPE_RUNTIME == file_header.file_type))
    {
        ERRORSTR("\r\n%s::Warning!! startup file %s can not be deleted !!.", 
                  __FUNCTION__, file_name); 
        return FS_RETURN_ACTION_INHIBITED;
    }
    else
    {
        if (FALSE == privilege)
        {
            if (strcmp((char*)file_name, FS_FACTORY_DEFAULT_CONFIG) == 0)
            {
                ERRORSTR("\r\n%s::Warning!! FS_FACTORY_DEFAULT_CONFIG (%s) can not be deleted !!.", __FUNCTION__, FS_FACTORY_DEFAULT_CONFIG); 
                return FS_RETURN_ACTION_INHIBITED;
            }
        }

        /* delete the file 
         */
        sprintf(name,"%s%s",FS_SAVE_FOLDER,file_name);
        if (unlink(name) < 0) {
            ERRORSTR("\r\n%s::ERROR!! can not delete file (%s)!!.", __FUNCTION__, name); 
            return FS_RETURN_ERROR;
        }
    }

    FS_OM_RemoveFileHeaderByName(file_name);
    return FS_RETURN_OK;
} /* End of FS_EraseFile() */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_DeleteFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function will delete a file
 * INPUT    : drive 		-- unit id or blade number
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
    return_value = FS_EraseFile(drive, (I8_T*)file_name, FALSE);
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
    return_value = FS_EraseFile(drive, (I8_T*)file_name, TRUE);
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
    char                name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];
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
    
        strncpy((char*)file_header.file_name,(char*)file_name,SYS_ADPT_FILE_SYSTEM_NAME_LEN+1);

        if (FALSE == FS_OM_GetFileHeaderByName(&file_header,FS_FILE_TYPE_MASK_ALL))
        {
            DEBUGSTR("\r\n%s:warning, file not found (%s) \r\n", __FUNCTION__, file_name);
            return FS_RETURN_FILE_NEX;
        }

        FS_SETBUSY();
        *read_count = 0;

        if( file_header.file_type == FS_FILE_TYPE_KERNEL ||
            file_header.file_type == FS_FILE_TYPE_RUNTIME )
        {            
            remainder = file_header.file_size;
            if (remainder > buf_size)
                remainder = buf_size;
            result = FS_ReadFlash(buf, remainder, read_count, file_header.mtd_num);
            if( result != FS_RETURN_OK ) 
                DEBUGSTR("\r\n%s, FS_ReadFlash() error", __FUNCTION__);
        }
        else
        {
            sprintf(name,"%s%s",FS_SAVE_FOLDER,file_header.file_name);
            fd = fopen(name, "r");
            if( fd == NULL )
            {
                ERRORSTR("\r\nFS_ReadFile: can not open file %s",name);
                FS_SETNOTBUSY();
                return FS_RETURN_FILE_NEX;
            }
            remainder   = file_header.file_size;
            if (remainder > buf_size) 
                remainder = buf_size;
            *read_count = fread(buf, 1, remainder, fd);
            fclose(fd);
        }

        /*if (block_id != FS_NULL_BLOCK_ID)*/
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
            EH_MGR_Handle_Exception1(SYS_MODULE_FS, FS_READFILE_FUNC_NO, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "File");
            FS_SETNOTBUSY();
            return result;
        }
        
//        memcpy(&(fs_control.file_attr), &(fs_file_attr_packet.file_attr[0]), sizeof(FS_File_Attr_T));
        FS_OM_SetControlAttribution(&(fs_file_attr_packet.file_attr[0]));
        FS_SETNOTBUSY();

        if(FS_Debug(FS_DEBUG_FLAG_DBGMSG))
        {
            printf("\r\n%s() File \"%s\" found, Type: %ld, Size: %ld",
                    __FUNCTION__,
                   file_name,
                   FS_OM_GetControlAttrFileType(),
                   FS_OM_GetControlAttrFileSize());
        }
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
            EH_MGR_Handle_Exception(SYS_MODULE_FS, FS_READFILE_FUNC_NO, EH_TYPE_MSG_MEM_ALLOCATE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT);
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
            EH_MGR_Handle_Exception(SYS_MODULE_FS, FS_COPYFILECONTENT_FUNC_NO, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
            return FS_RETURN_NOT_MASTER;
        }

        FS_SETBUSY();
        strcpy((char*)fs_file_attr_packet.file_attr[0].file_name, (char*)file_name);
        fs_file_attr_packet.file_attr[0].file_type_mask = FS_FILE_TYPE_MASK_ALL;

        return_value = FS_GetRemoteMultipleFileInfo(drive, FALSE, &fs_file_attr_packet, 1);
        
        if(return_value != FS_RETURN_OK && return_value != FS_RETURN_HAVEMOREDATA)
        {
            ERRMSG(return_value);
            EH_MGR_Handle_Exception1(SYS_MODULE_FS, FS_READFILE_FUNC_NO, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "File");
            FS_SETNOTBUSY();
            return return_value;
        }
        
//        memcpy(&(fs_control.file_attr), &(fs_file_attr_packet.file_attr[0]), sizeof(FS_File_Attr_T));
        FS_OM_SetControlAttribution(&(fs_file_attr_packet.file_attr[0]));
        
        FS_SETNOTBUSY();

        if(FS_Debug(FS_DEBUG_FLAG_DBGMSG))
        {
            printf("\r\n%s() File \"%s\" found, Type: %ld, Size: %ld",
                __FUNCTION__,
                file_name,
                FS_OM_GetControlAttrFileType(),
                FS_OM_GetControlAttrFileSize());
        }

        /* Allocate memory space to hold file content */
        FS_OM_SetControlFileBuffer( L_MM_Malloc(byte_num, L_MM_USER_ID2(SYS_MODULE_FS, FS_TYPE_TRACE_ID_FS_COPYFILECONTENT)));
        DBGMSG("Allocate Buffer");
        FS_OM_SetControlFilePtr(FS_OM_GetControlFileBuffer());

        if(!FS_OM_GetControlFileBuffer())
        {
            ERRMSG(FS_RETURN_NO_BUFFER);
            EH_MGR_Handle_Exception(SYS_MODULE_FS, FS_COPYFILECONTENT_FUNC_NO, EH_TYPE_MSG_MEM_ALLOCATE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT);
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

    strncpy((char*)file_header.file_name,(char*)file_name,SYS_ADPT_FILE_SYSTEM_NAME_LEN+1);

    if (FALSE == FS_OM_GetFileHeaderByName(&file_header,FS_FILE_TYPE_MASK_ALL))        
    {
        /* file not found 
         */
        DEBUGSTR("\r\n%s:ERROR, file not found %s", __FUNCTION__, file_name);
        return FS_RETURN_FILE_NEX;
    }
    else
    {
        FILE    *fd;
        char    name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];

        /* File exists 
         */
        if( file_header.file_type == FS_FILE_TYPE_KERNEL ||
            file_header.file_type == FS_FILE_TYPE_RUNTIME)
        {
            DEBUGSTR("\r\n%s:warning, kernel/runtime file not support in this function.", __FUNCTION__);
            FS_SETNOTBUSY();
            return FS_RETURN_ACTION_INHIBITED;
        }
        else
        {
            sprintf(name,"%s%s",FS_SAVE_FOLDER,file_header.file_name);
            fd = fopen(name, "r");
            if( fd == NULL )
            {
                ERRORSTR("\r\n%s:ERROR, can not open file %s",__FUNCTION__, name);
                FS_SETNOTBUSY();
                return FS_RETURN_FILE_NEX;
            }
            fseek(fd, offset, SEEK_SET);
            copy_size   = file_header.file_size;
            if (copy_size < (byte_num+offset)) DEBUGSTR("\r\n file size %ld, < offset(%ld) + copy#(%ld)",copy_size, offset,byte_num);
            copy_size = fread(buf, 1, byte_num, fd);
            if (copy_size < byte_num) DEBUGSTR("\r\n Only copy %ld which request %ld",copy_size,byte_num);
            fclose(fd);
            FS_SETNOTBUSY();
            return FS_RETURN_OK;
        }
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
    UI32_T              total_size_of_free_space;
    char                name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];
    UI32_T              usage_size, ret;
    FILE*               fd;
    BOOL_T              file_exist = FALSE;
    BOOL_T              exist_file_is_startup = FALSE;
    UI32_T              number_of_files;

    /* File system initialized OK? */
    if (FS_OM_GetInitStatus() != FS_RETURN_OK)
        return FS_RETURN_NOT_READY;
    
    if ((file_name == NULL) || (buf == NULL) || (length == 0))
    {
        ERRORSTR("\r\n%s:ERROR, bad parameter",__FUNCTION__);
        return FS_RETURN_BAD_PARA;
    }

    if (strlen((char*)file_name) > FS_MAX_FILE_NAME_LENGTH)
    {
        ERRORSTR("\r\n%s:ERROR, bad parameter file name %s",__FUNCTION__, file_name);
        return FS_RETURN_BAD_PARA;
    }

    if (    (file_type  <=  FS_FILE_TYPE_SUBFILE    )
        &&  (file_type  >=  FS_FILE_TYPE_TOTAL      )
       )
    {
        ERRORSTR("\r\n%s:ERROR, bad parameter file type %ld",__FUNCTION__, file_type);
        return FS_RETURN_BAD_PARA;
    }

    /* 1.0 Delete the old file if it exists
     * 1.1 Find this file 
     */
    strncpy((char*)file_header.file_name,(char*)file_name,SYS_ADPT_FILE_SYSTEM_NAME_LEN+1);

    if (TRUE == FS_OM_GetFileHeaderByName(&file_header,FS_FILE_TYPE_MASK_ALL))        
    {
        file_exist = TRUE;
        exist_file_is_startup = file_header.startup;
        /* Check the file type if trying to overwrite an existent file 
         */    
        if( file_header.file_type != (UI8_T)file_type )
        {
            /* The file name is the same as an existent file in the file system
             * with a different file type 
             */
            ERRORSTR("\r\nWarning!! Trying to overwrite an existent file with different file type. Operation Inhibited.\r\n");
            return FS_RETURN_ACTION_INHIBITED;
        }

        if(((file_type  ==  FS_FILE_TYPE_KERNEL) ||
            (file_type  ==  FS_FILE_TYPE_RUNTIME)) &&
             (TRUE      ==  file_header.startup))
        {
            /* We can not update current runtime file 
             */
            ERRORSTR("\r\n%s:ERROR, you can not update current kernel/runtime file %s!!!\r\n",__FUNCTION__, file_name);
            return FS_RETURN_ACTION_INHIBITED;
        }
    }

    /* Check the maximum number of files for this file type 
     */
    number_of_files = FS_NumOfFile(file_type);
    if (TRUE == file_exist)
    {
        if (number_of_files == 0)
        {
            ERRORSTR("\r\n%s(%lu): Warning!! File system error.\r\n",__FUNCTION__,__LINE__);
            return FS_RETURN_ACTION_INHIBITED;
        }
        number_of_files = number_of_files - 1;
    }
    if (number_of_files >= FS_MaxNumOfFile[file_type])
    {
        /* The number of files of the given type exceeds the maximum number if a new
         * file is added in the storage.
         */
        ERRORSTR("\r\nWarning!! The number of files of the given type exceeds the maximum number. Operation Inhibited.\r\n");
        return FS_RETURN_EXCEED_MAX_NUM_OF_FILES;
    }

    /* Evaluate the free space for the new file
     * runtime is rootfs which has independent partition, so don't to check free space
     * In jffs2 file system, file is compressed, so below verify is un-nessary now. 
     */
    #if 0
    if( file_type  !=  FS_FILE_TYPE_KERNEL &&
        file_type  !=  FS_FILE_TYPE_RUNTIME)
    {
        FS_GetStorageFreeSpace(unit,&total_size_of_free_space);

        if (TRUE == file_exist)
        {
            /* If the file exists thus the new file overwrites the old file so that
             * the size of the old file should be added in the total size of the free space
             */
            FS_GetFileStorageUsageSize(unit, (UI8_T*)file_name, &usage_size);
            total_size_of_free_space += usage_size;
        }
        if ( (total_size_of_free_space < length) || (total_size_of_free_space < resv_len) )
        {
            ERRORSTR("\r\nWarning!! Not enough space for new file. Operation Inhibited.\r\n");
            return FS_RETURN_ACTION_INHIBITED;
        }
    }
    #endif

    if (!privilege)
    {
        /* If the FS_FACTORY_DEFAULT_CONFIG exists, the action is inhibited to overwrite. */
        if ( (strcmp((char*)file_name, FS_FACTORY_DEFAULT_CONFIG) == 0) && (TRUE == file_exist) )
        {
            ERRORSTR("\r\nWarning!! you can not overwrite default config %s.\r\n", FS_FACTORY_DEFAULT_CONFIG);
            return FS_RETURN_ACTION_INHIBITED;
        }
    }

    /* 1.2 File exists: erase the old file first */
    if ( TRUE == file_exist )
    {
        /* Check the magic number to avoid the illegal access due to the file header list corruption */
        if(file_header.magic_number != FS_SPECIAL_MAGIC_NUMBER)
        {
            ERRORSTR("\r\nFS_UpdateFile::Warning!! The file header list may have been destroyed!!. \
                    The operation is prohibited for safety. Please restart the system to recover.\r\n");
            return FS_RETURN_ACTION_INHIBITED;
        }

        /* delete file 
         */
        if (FS_RETURN_OK == FS_EraseFile(unit, file_name, privilege))
        {
            /* 1.2.3 Remove file header 
             */
            //FS_OM_RemoveFileHeaderByName(file_name);
        }
    }

    /* 2.0 Write new file 
     */
    if( file_type  ==  FS_FILE_TYPE_KERNEL ||
        file_type  ==  FS_FILE_TYPE_RUNTIME)
    {
        SYS_HWCFG_FlashEntry_T *flash_entry_p = NULL;
        FS_FileHeader_T        file_header;
        
        FS_OM_GetStartupName(file_type, file_header.file_name);
        FS_OM_GetFileHeaderByName(&file_header, FS_FILE_TYPE_MASK_ALL);

        /* We can only write the non-startup runtime rootfs
         */
        ret = FS_RETURN_OK;
        while (NULL != (flash_entry_p = SYS_HWCFG_GetNextFlashPartitionEntry(flash_entry_p)))
        {
#ifdef SYS_HWCFG_FLASH_TYPE_KERNEL
            if (file_type == FS_FILE_TYPE_KERNEL &&
                flash_entry_p->type != SYS_HWCFG_FLASH_TYPE_KERNEL)
                continue;
#endif
#ifdef SYS_HWCFG_FLASH_TYPE_ROOTFS
            if (file_type == FS_FILE_TYPE_RUNTIME &&
                flash_entry_p->type != SYS_HWCFG_FLASH_TYPE_ROOTFS)
                continue;
#endif
            if (flash_entry_p->mtdnum == file_header.mtd_num)
                continue;

            /* write on the first found partition
             */
            ret = FS_WriteFlash(buf, length, flash_entry_p->mtdnum); 
            break;
        }

        if( ret != FS_RETURN_OK ) 
        {
            ERRORSTR("\r\n%s:ERROR, from FS_WriteFlash(%ld)",__FUNCTION__,ret);
            return ret;
        }
        if (file_type  ==  FS_FILE_TYPE_KERNEL)
            FS_BuildKernelFile(file_name, flash_entry_p->mtdnum, FALSE);
        else if (file_type  ==  FS_FILE_TYPE_RUNTIME)
            FS_BuildRootfsFile(file_name, flash_entry_p->mtdnum, FALSE);        
    }
    else
    {
        sprintf(name,"%s%s",FS_SAVE_FOLDER,file_name);
        fd = fopen(name, "w");
        if( fd == NULL )
        {
            ERRORSTR("\r\nFS_UpdateFile:ERROR, can not open file %s",name);
            return FS_RETURN_FILE_NEX;
        }
        usage_size = fwrite(buf, 1, length, fd);
        if (length > usage_size) 
        {
            ERRORSTR("\r\nFS_UpdateFile:ERROR, write size %ld < require %ld",usage_size, length);
            fclose(fd);
            return FS_RETURN_NO_BUFFER;
        }
        fclose(fd);
        if (TRUE == file_exist)
        {
            chown(name, FS_TYPE_STARTUP_TO_UID(file_type,exist_file_is_startup), -1);
        }
        else
        {
            chown(name, FS_TYPE_STARTUP_TO_UID(file_type,FALSE), -1);
        }
    }

    /* 2.3 Create the file header and insert it into the sorted FS_FileHeaderList 
     */
    FS_BuildFileHeaderFromFile(file_name);
    return  FS_RETURN_OK;
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
    FS_File_Attr_T  file_attr;

#if (SYS_CPNT_STACKING == TRUE)
    /* Check file name */
    if (FS_GenericFilenameCheck(file_name, file_type) != FS_RETURN_OK)
    {
        if(FS_Debug(FS_DEBUG_FLAG_DBGMSG))
        {
            printf("\r\nError : File name %s contains an invalid character!", file_name);
        }
        return  FS_RETURN_ERROR;
    }
    
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

        if(FS_Debug(FS_DEBUG_FLAG_DBGMSG))
        {
            printf("\r\n%s() File %s, Size %ld, Checksum: %02x",
                __FUNCTION__, file_name, length, file_attr.check_sum);
        }
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
    if (FS_GenericFilenameCheck(file_name, file_type) == FS_RETURN_OK)
    {
        FS_SETBUSY();
        return_value = FS_UpdateFile(drive, (I8_T*)file_name, file_comment, file_type, buf, length, resv_len, FALSE);
        FS_SETNOTBUSY();
        return return_value;
    }
    else
    {
        if(FS_Debug(FS_DEBUG_FLAG_DBGMSG))
        {
            printf("\r\nError : File name %s contains an invalid character!", file_name);
        }
        ERRORSTR("\r\n%s:ERROR, filename is not correct %s", __FUNCTION__, file_name);
        return  FS_RETURN_ERROR;
    }
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
    FS_File_Attr_T  file_attr;

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
        return_value = FS_UpdateFile(drive, (I8_T*)file_name, file_comment, file_type, buf, length, resv_len, TRUE);
#endif
        FS_SETNOTBUSY();
        return return_value;
    }
    else
    {
        if(FS_Debug(FS_DEBUG_FLAG_DBGMSG))
        {
            printf("\r\nError : File name %s contains an invalid character!", file_name);
        }
        ERRORSTR("\r\n%s:ERROR, filename is not correct %s", __FUNCTION__, file_name);
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
    UI32_T              ret;
    BOOL_T              file_exist;
    UI8_T               name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];
    UI8_T               old_file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN];
    
    /* File system initialized OK? */
    if (FS_OM_GetInitStatus() != FS_RETURN_OK)
        return FS_RETURN_NOT_READY;

    DEBUGSTR("\r\n%s:Update startup attribute ...",__FUNCTION__);

    if ( (file_type == 0) || (file_type >= FS_FILE_TYPE_TOTAL) )
    {
        ERRORSTR("\r\n%s:ERROR, file_type(%ld) is wrong",__FUNCTION__, file_type);
        return FS_RETURN_INDEX_OOR;
    }

    if (strlen((char*)file_name) > FS_MAX_FILE_NAME_LENGTH)
    {
        ERRORSTR("\r\n%s:ERROR, file name(%s) is wrong",__FUNCTION__, file_name);
        return FS_RETURN_BAD_PARA;
    }

    if( (file_type == FS_FILE_TYPE_KERNEL) && (file_name[0] == 0) )
    {
        ERRORSTR("\r\n%s:Warning, you can not clear kernel startup attribute ",__FUNCTION__);
        return FS_RETURN_ACTION_INHIBITED;
    }
    
    if( (file_type == FS_FILE_TYPE_RUNTIME) && (file_name[0] == 0) )
    {
        ERRORSTR("\r\n%s:Warning, you can not clear runtime startup attribute ",__FUNCTION__);
        return FS_RETURN_ACTION_INHIBITED;
    }  

    /* Existence check 
     */
    strncpy((char*)file_header.file_name,(char*)file_name,SYS_ADPT_FILE_SYSTEM_NAME_LEN+1);

    file_exist = FS_OM_GetFileHeaderByName(&file_header,FS_FILE_TYPE_MASK(file_type));

    if (!(TRUE == privilege && (file_name[0] == 0) ))
    {
        if ( FALSE == file_exist )
        {
            /* file not found */
            ERRORSTR("\r\n%s:ERROR, file not find(%s)",__FUNCTION__, file_name);
            return FS_RETURN_FILE_NEX;
        }
        else
        {
            /* file found and check version of runtime image 
             */
            if (file_type == FS_FILE_TYPE_KERNEL ||
                file_type == FS_FILE_TYPE_RUNTIME)
            {
                if( TRUE == file_header.startup)
                {
                    /* Same as current rootfs 
                     */
                    DEBUGSTR("\r\n%s: Same as current runtime %s",__FUNCTION__, file_name);
                }
                else
                {
                    /* Set U-boot bootargs to new rootfs 
                     */
                    ret= FS_WriteUbootBootargs(file_header.mtd_num);
                    if( ret != FS_RETURN_OK ) 
                    {
                        ERRORSTR("\r\n%s:ERROR, write U-boot bootargs error (%ld)",__FUNCTION__, ret);
                        return ret;
                    }
                }

            }
        }
    }

//    strcpy(FS_Startup[0], FS_STARTUP_BLOCK_MAGIC_STRING);
//    FS_Startup[0][SYS_ADPT_FILE_SYSTEM_NAME_LEN-1] = 0;
    if (TRUE == FS_OM_GetStartupName(file_type, old_file_name))
    {
        sprintf((char*)name,"%s%s",FS_SAVE_FOLDER,old_file_name);
        chown((char*)name, FS_TYPE_STARTUP_TO_UID(file_type,FALSE),-1);
    }
    //printf("Before FS_OM_SetStartupName !\n");
    FS_OM_SetStartupName(file_type, (UI8_T*)file_name);
    sprintf((char*)name,"%s%s",FS_SAVE_FOLDER,file_name);
    chown((char*)name, FS_TYPE_STARTUP_TO_UID(file_type,TRUE),-1);
    
    /* for bytesum */
/*
    sum = FS_ByteSum((UI8_T*)FS_Startup[0], sizeof(FS_Startup));
    FS_Startup[0][SYS_ADPT_FILE_SYSTEM_NAME_LEN-1] = ~sum +1;
*/
#if 0 //kh_shi
    {
        UI8_T   fs_startup[FS_FILE_TYPE_TOTAL][SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];
        FS_OM_GetStartupAll(fs_startup[0]);
        fd = fopen(FS_STARTUP_FILENAME, "w");
        if( !fd )
        {
            /* fail */
            ERRORSTR("\r\n%s::ERROR!! can not open %s",__FUNCTION__,FS_STARTUP_FILENAME);
            return FS_RETURN_BAD_HW;
        }
        count = fwrite(fs_startup[0], 1, sizeof(fs_startup), fd);
        if ( count < sizeof(fs_startup) )
        {
            /* fail */
            ERRORSTR("\r\n%s::ERROR!! for write %ld < %d",__FUNCTION__,count,sizeof(fs_startup));
            fclose(fd);
            return FS_RETURN_BAD_HW;
        }
        fclose(fd);
        DEBUGSTR("\r\n%s:Update startup file successfully",__FUNCTION__);
    }
#endif
    return FS_RETURN_OK;
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
#if (SYS_CPNT_STACKING == TRUE)
    UI32_T  retry_count;
#endif
#if (SYS_CPNT_STACKING == TRUE)


    if (FS_GenericFilenameCheck(file_name, file_type) != FS_RETURN_OK)
    {
        printf("\r\nError : File name %s contains a invalid character!", file_name);
        return  FS_RETURN_ERROR;
    }

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
            EH_MGR_Handle_Exception(SYS_MODULE_FS, FS_SETSTARTUPFILENAME_FUNC_NO, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
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
        if (FS_Debug(FS_DEBUG_FLAG_DBGMSG))
        {
            printf("\r\n%s(): Remote operation for drive %d at line %d\r\n", __FUNCTION__, (int)drive, __LINE__);
        }
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
    //printf("Enter FS_UpdateStartupAttribute !\n");
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
            EH_MGR_Handle_Exception(SYS_MODULE_FS, FS_RESETSTARTUPFILENAME_FUNC_NO, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
            return FS_RETURN_NOT_MASTER;
        }

        /* Check if drive exists */
                /*
        if(!STKTPLG_POM_UnitExist(drive))
        {
            return FS_RETURN_DRIVE_NOT_EXIST;
        }
*/
        if (FS_Debug(FS_DEBUG_FLAG_DBGMSG))
        {
            printf("\r\n%s(): Remote operation for drive %d at line %d\r\n", __FUNCTION__, (int)drive, __LINE__);
        }
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
                SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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
        ERRORSTR("\r\n%s:ERROR, bad parameter.",__FUNCTION__);
        return FS_RETURN_INDEX_OOR;
    }

    if (file_name == 0)
    {
        ERRORSTR("\r\n%s:ERROR, bad parameter.",__FUNCTION__);
        return FS_RETURN_BAD_PARA;
    }

#if (SYS_CPNT_STACKING == TRUE)
    if (    (drive == DUMMY_DRIVE)
         || (drive == SYS_VAL_LOCAL_UNIT_ID)
         || (drive == FS_OM_GetControlDriverId())
       )
    {
#endif
        if( FS_Debug(FS_DEBUG_FLAG_DBGMSG) )
            printf("\r\n%s() Drive: %ld\r\n", __FUNCTION__, drive);

        if (TRUE == FS_OM_GetStartupName(file_type, file_name))
        {
            return FS_RETURN_OK;        
        }
        else
        {
            ERRORSTR("\r\n%s:ERROR, no startup for file_type=%lu.",__FUNCTION__, file_type);
            return FS_RETURN_ERROR;        
        }

#if (SYS_CPNT_STACKING == TRUE)
    }
    else /* Remote operation */
    {
        UI32_T                  return_value;

        if( FS_Debug(FS_DEBUG_FLAG_DBGMSG) )
            printf("\r\n%s() Drive: %ld\r\n", __FUNCTION__, drive);

        file_name[0] = 0;
        FS_SETBUSY();
        fs_file_attr_packet.file_attr[0].file_type = file_type;
        return_value = FS_GetRemoteStartupFilename(drive, &fs_file_attr_packet);

        if (return_value == FS_RETURN_FILE_NEX)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_FS, FS_GETSTARTUPFILENAME_FUNC_NO, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "File");
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
    UI32_T  result;
    FS_File_Attr_T  file_attr;

    /* Local operation */
    FS_SETBUSY();
#if (SYS_CPNT_STACKING == TRUE)
    if (    (drive == DUMMY_DRIVE)
         || (drive == SYS_VAL_LOCAL_UNIT_ID)
         || (drive == FS_OM_GetControlDriverId())
       )
#endif
    {
        result  = (FS_WriteFlash(loader, size, FS_LOADER) );
    }
#if (SYS_CPNT_STACKING == TRUE)
    else /* Remote operation */
    {
        /* save the file information */
        memset(&(file_attr), 0, sizeof(FS_File_Attr_T));
        file_attr.file_type  = FS_FILE_TYPE_TOTAL;
        file_attr.file_size  = size;
        file_attr.privilage  = TRUE;
        file_attr.check_sum = FS_ByteSum(loader, size);
        FS_OM_SetControlAttribution(&(file_attr));
        /* Tell slave to Write file */
        result  = ( FS_WriteRemoteFile(drive, &(file_attr)) );
    }
#endif
    FS_SETNOTBUSY();
    return result;
}

static UI32_T FS_CheckKernelHeader(UI8_T *buf_p, UI32_T length)
{
	I8_T	*data;
	UI32_T	len, checksum;
	image_header_t *hdr = (image_header_t *)buf_p;

	if( length < sizeof(image_header_t) ) 
    {
		return FS_RETURN_ERROR;
	}

	if ((hdr->ih_magic != IH_MAGIC) || (hdr->ih_type != IH_TYPE_KERNEL))
    {
		return FS_RETURN_ERROR;
	}

	data = (I8_T*)buf_p;
	len  = sizeof(image_header_t);

	/* blank CRC field for re-calculation 
     */
	checksum = hdr->ih_hcrc;
	hdr->ih_hcrc = 0;

	if( L_MATH_Crc32(0, data, len) != checksum) 
    {
        hdr->ih_hcrc = checksum;
		return FS_RETURN_ERROR;
	}
    hdr->ih_hcrc = checksum;
    
	return FS_RETURN_OK;
}

static UI32_T FS_CheckRootfsHeader(UI8_T *buf_p, UI32_T length)
{
	struct cramfs_super *hdr = (struct cramfs_super *)buf_p;

	if( length < sizeof(struct cramfs_super) ) 
    {
		return FS_RETURN_ERROR;
	}

	if ((hdr->magic != CRAMFS_MAGIC) || (memcmp((char*)hdr->signature,CRAMFS_SIGNATURE,strlen(CRAMFS_SIGNATURE))!=0))
    {
		return FS_RETURN_ERROR;
	}
    
	return FS_RETURN_OK;    
}

static UI32_T FS_CheckKernelImage(UI8_T *addr, UI32_T length)
{
	I8_T	*data;
	UI32_T	len, checksum;
	image_header_t *hdr = (image_header_t*)addr;

	DEBUGSTR("\r\n%s: ## Checking Image at %08lx ...\n",__FUNCTION__, (UI32_T)addr);
	if( length < (sizeof(image_header_t)+hdr->ih_size) ) {
		ERRORSTR ("\r\n%s:ERROR,  Image length incorrect %lx < %lx\n",
		    __FUNCTION__, length, (UI32_T)sizeof(image_header_t)+hdr->ih_size);
		return FS_RETURN_ERROR;
	}

	if (hdr->ih_magic != IH_MAGIC) {
		ERRORSTR ("\r\n%s:ERROR,  Bad Magic Number %lx \n",__FUNCTION__, (UI32_T)hdr->ih_magic);
		return FS_RETURN_ERROR;
	}

	data = (I8_T*)addr;
	len  = sizeof(image_header_t);

	/* blank CRC field for re-calculation */
	checksum = hdr->ih_hcrc;
	hdr->ih_hcrc = 0;

	if( L_MATH_Crc32(0, data, len) != checksum) {
		ERRORSTR ("\r\n%s:ERROR,  Bad Header Checksum, %lx \n",__FUNCTION__, checksum);
        hdr->ih_hcrc = checksum;
		return FS_RETURN_ERROR;
	}
    hdr->ih_hcrc = checksum;

	/* for multi-file images we need the data part, too */
    DEBUGSTR("\n============================\n");
    DEBUGSTR("   Image Name:   %.*s\n", IH_NMLEN, hdr->ih_name);
    DEBUGSTR("   Image CRC:    %lx\n", (UI32_T)hdr->ih_hcrc); 
    DEBUGSTR("   Data CRC:     %lx\n", (UI32_T)hdr->ih_dcrc); 
    DEBUGSTR("   Create Time:  %lx\n", (UI32_T)hdr->ih_time);
    DEBUGSTR("   System:       %x\n", hdr->ih_os); 
    DEBUGSTR("   Image Type:   %x\n", hdr->ih_type); 
    DEBUGSTR("   Data Size:    %lx\n", (UI32_T)hdr->ih_size);
    DEBUGSTR("   CPU Arch:     %x\n", hdr->ih_arch); 
    DEBUGSTR("   Compressed:   %x\n", hdr->ih_comp); 
    DEBUGSTR("   Load Address: %08lx\n", (UI32_T)hdr->ih_load);
    DEBUGSTR("   Entry Point:  %08lx\n", (UI32_T)hdr->ih_ep);
    DEBUGSTR("\n============================\n");

	data = (I8_T*)addr + sizeof(image_header_t);
	len  = hdr->ih_size;

	DEBUGSTR ("\r\n%s: Verifying Checksum ...\n", __FUNCTION__);

	if (L_MATH_Crc32 (0, data, len) != hdr->ih_dcrc) {
		ERRORSTR ("\r\n%s:ERROR,  Bad Data CRC, %lx \n",__FUNCTION__, (UI32_T)hdr->ih_dcrc);
		return FS_RETURN_ERROR;
	}
	DEBUGSTR ("\r\n%s: END\n", __FUNCTION__);
	return FS_RETURN_OK;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - FS_WriteKernel
 * ------------------------------------------------------------------------
 * FUNCTION : This function will write kernel image to flash
 * INPUT    : drive         -- unit id or blade number
 *            kernel        -- loader to write
 *            size          -- size of the loader
 * OUTPUT   : None
 * RETURN   : one of FS_RETURN_CODE_E
 * NOTE     : drive represents either the unit id in a stack or the blade number
 *            in a chasis.
 * ------------------------------------------------------------------------
 */
UI32_T FS_WriteKernel(UI32_T drive, UI8_T *kernel, UI32_T size)
{
    UI32_T  result=0;

    /* Local operation */
    FS_SETBUSY();
#if (SYS_CPNT_STACKING == TRUE)
    if (    (drive == DUMMY_DRIVE)
         || (drive == SYS_VAL_LOCAL_UNIT_ID)
         || (drive == FS_OM_GetControlDriverId())
       )
#endif
    {
        result  = FS_CheckKernelImage(kernel, size);
        if( result == FS_RETURN_OK )
        {
            UI32_T  ret;
#ifdef ECN430_FB2 /*ECN430_FB2*/
#if 0
#ifdef FS_KERNEL
            ret = FS_WriteFlash(kernel, size, FS_KERNEL);
            if( ret != FS_RETURN_OK )
            {
                ERRORSTR ("\r\n%s:ERROR,  write kernel A fail\n",__FUNCTION__);
                result = ret;
            }
#endif
#endif
#endif 
#ifdef  ES4626H /*ES4626H*/
            ret  = FS_WriteFlash(kernel, size, FS_KERNEL_B);
            if( ret != FS_RETURN_OK )
            {
                ERRORSTR ("\r\n%s:ERROR,  write kernel B fail\n",__FUNCTION__);
                result = ret;
            }
#endif
        }
        else
        {
            ERRORSTR ("\r\n%s:ERROR, kernel image is wrong\n",__FUNCTION__);
        }
    }
#if (SYS_CPNT_STACKING == TRUE)
#if 0 // not support now
    else /* Remote operation */
    {
        /* save the file information */
        memset(&(fs_control.file_attr), 0, sizeof(FS_File_Attr_T));
        fs_control.file_attr.file_type  = FS_FILE_TYPE_TOTAL;
        fs_control.file_attr.file_size  = size;
        fs_control.file_attr.privilage  = TRUE;
        fs_control.file_attr.check_sum = FS_ByteSum(kernel, size);
        /* Tell slave to Write file */
        result  = ( FS_WriteRemoteFile(drive, &(fs_control.file_attr)) );
    }
#endif
#endif
    FS_SETNOTBUSY();
    return result;
}



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
    FS_SetDebugFlag( (UI32_T)FS_DEBUG_FLAG_NONE );

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
    FS_SetDebugFlag( (UI32_T)FS_DEBUG_FLAG_NONE );

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
    UI32_T  result;

#ifndef INCLUDE_DIAG
    FS_SETBUSY();
#endif
    result=FS_WriteFlash((UI8_T*)hwinfo, sizeof(FS_HW_Info_T), FS_HWINFO);

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
 
    if (hwinfo == NULL)
    {
        ERRORSTR("\r\n%s:ERROR, bad parameter hwinfo is NULL.",__FUNCTION__);
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
        I32_T       ret = FS_RETURN_OK;
        
#ifndef INCLUDE_DIAG
        FS_SETBUSY();
#endif
        
        ret = FS_ReadFlash((UI8_T*)hwinfo, sizeof(FS_HW_Info_T), &count, FS_HWINFO);
        
        if( count < sizeof(FS_HW_Info_T) )
        {
            DEBUGSTR("\r\n%s::warning, read size %ld < %d!!", __FUNCTION__, count, sizeof(FS_HW_Info_T));
            if( ret == FS_RETURN_OK) ret = FS_RETURN_FILE_TRUNCATED;
        }

#ifndef INCLUDE_DIAG
        FS_SETNOTBUSY();
#endif
        
        DEBUGSTR("\r\n%s return", __FUNCTION__);
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
            EH_MGR_Handle_Exception(SYS_MODULE_FS, FS_READHARDWAREINFO_FUNC_NO, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
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
                SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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
#endif
}



static UI32_T FS_WriteFlash(UI8_T *buf, UI32_T length, UI32_T mtd)
{
    I32_T   fd;
    char    name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];
    UI32_T  count;

    SYS_HWCFG_FlashEntry_T *flash_entry_p = NULL;

    while (NULL != (flash_entry_p = SYS_HWCFG_GetNextFlashPartitionEntry(flash_entry_p)))
    {
        if (flash_entry_p->mtdnum == mtd)
        {
            switch (flash_entry_p->type)
            {
#ifdef SYS_HWCFG_FLASH_TYPE_HRCW
                case SYS_HWCFG_FLASH_TYPE_HRCW:
#endif
                case SYS_HWCFG_FLASH_TYPE_USERDATA:
                case SYS_HWCFG_FLASH_TYPE_LOADERDATA:
                    ERRORSTR("\r\nFS_WriteFlash: UNSUPPORT mtd=%ld",mtd);
                    return FS_RETURN_ERROR;
                    break;
                default:
                   if (length > flash_entry_p->size)
                   {
                       ERRORSTR("\r\n%s:ERROR, Rootfs_a size %lu > %lu",__FUNCTION__, length, flash_entry_p->size);
                       length = flash_entry_p->size;
                   }
                   break;
            }
            sprintf(name, "%s%lu", FS_FLASH_MTD_BLOCK_PREFIX, mtd);
            fd = open(name, O_WRONLY, 0666);
            if( fd == -1 )
            {
                ERRORSTR("\r\n%s:ERROR!! Can not open %s!!", __FUNCTION__, name);
                return FS_RETURN_ERROR;
            }
            count = write(fd, buf, length);
            if( count < length )
            {
                ERRORSTR("\r\n%s:ERROR!! read size %ld < %ld!!", __FUNCTION__, count, length);
                close(fd);
                return FS_RETURN_ERROR;
            }
            close(fd);
            return FS_RETURN_OK;            
        }
    }
    
    ERRORSTR("\r\nFS_WriteFlash: UNKNOWN mtd=%ld",mtd);
    return FS_RETURN_ERROR;
}

static UI32_T FS_ReadFlash(UI8_T *buf, UI32_T buf_len, UI32_T *read_len, UI32_T mtd)
{
    I32_T   fd;
    char    name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];
    UI32_T  count = buf_len;

    SYS_HWCFG_FlashEntry_T *flash_entry_p = NULL;

    while (NULL != (flash_entry_p = SYS_HWCFG_GetNextFlashPartitionEntry(flash_entry_p)))
    {
        if (flash_entry_p->mtdnum == mtd)
        {
            switch (flash_entry_p->type)
            {
#ifdef SYS_HWCFG_FLASH_TYPE_HRCW
                case SYS_HWCFG_FLASH_TYPE_HRCW:
#endif
                case SYS_HWCFG_FLASH_TYPE_USERDATA:
                case SYS_HWCFG_FLASH_TYPE_LOADERDATA:
                    ERRORSTR("\r\n%s: UNSUPPORT mtd=%ld",__FUNCTION__, mtd);
                    return FS_RETURN_ERROR;
                    break;
                default:
                   DEBUGSTR("\r\n%s: mtd=%lu size %ld, read %ld", __FUNCTION__,mtd,flash_entry_p->size, count);
                   if (count > flash_entry_p->size)
                   {
                       count = flash_entry_p->size;
                   }
                   break;
            }

            *read_len = 0;
            
            sprintf(name, "%s%lu", FS_FLASH_MTD_BLOCK_PREFIX, mtd);
            fd = open(name, O_RDONLY, 0666);
            if( fd == -1 )
            {
                ERRORSTR("\r\n%s:ERROR!! Can not open %s!!", __FUNCTION__, name);
                return FS_RETURN_ERROR;
            }
            *read_len = read(fd, buf, count);
            if( *read_len < buf_len )
            {
                DEBUGSTR("\r\n%s: read size %ld < %ld!!", __FUNCTION__, *read_len, buf_len);
            }
            close(fd);
        
            return FS_RETURN_OK;
        }
    }
    
    ERRORSTR("\r\n%s: UNKNOWN mtd=%ld",__FUNCTION__, mtd);
    return FS_RETURN_ERROR;
}


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

    if(FS_Debug(FS_DEBUG_FLAG_DBGMSG))
        printf("\r\n%s() Calling %s()", __FUNCTION__, FS_Service_Names[rx_header_p->opcode]);
        
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
        EH_MGR_Handle_Exception(SYS_MODULE_FS, FS_STATIC_FUNCTION_FUNC_NO, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
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
    if (FS_Debug(FS_DEBUG_FLAG_DBGMSG))
    {
        printf("\r\n%s(): Remote operation for drive %d at line %d\r\n", __FUNCTION__, (int)drive, __LINE__);
    }
    if(FS_OM_GetControlFileBuffer() == (void *) 0)
    {
        ERRMSG(FS_RETURN_NO_BUFFER);
        EH_MGR_Handle_Exception(SYS_MODULE_FS, FS_STATIC_FUNCTION_FUNC_NO, EH_TYPE_MSG_MEM_ALLOCATE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT);
        return FS_RETURN_NO_BUFFER;
    }

    /* Start remote operation. Block all other remote accesses */
    FS_LOCK_REMOTE();

    if(FS_Debug(FS_DEBUG_FLAG_DBGMSG))
    {
        printf("\r\n%s() Read File: %s, size: %ld",
            __FUNCTION__,
            file_attr->file_name,
            file_attr->file_size);
    }

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
            SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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
            SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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

    /* 1 - Send "Copy File Content" request to remote drive. */

    /* 1.1 - Stacking mode check: check if FS is in the correct state. */
    if(FS_OM_GetControlStackingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        ERRMSG(FS_RETURN_NOT_MASTER);
        EH_MGR_Handle_Exception(SYS_MODULE_FS, FS_STATIC_FUNCTION_FUNC_NO, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
        return FS_RETURN_NOT_MASTER;
    }

    if (FS_Debug(FS_DEBUG_FLAG_DBGMSG))
    {
        printf("\r\n%s(): Remote operation for drive %d at line %d\r\n", __FUNCTION__, (int)drive, __LINE__);
    }
    if(FS_OM_GetControlFileBuffer() == (void *) 0)
    {
        ERRMSG(FS_RETURN_NO_BUFFER);
        EH_MGR_Handle_Exception(SYS_MODULE_FS, FS_STATIC_FUNCTION_FUNC_NO, EH_TYPE_MSG_MEM_ALLOCATE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT);
        return FS_RETURN_NO_BUFFER;
    }

    /* 1.2 - Start remote operation: block all other remote accesses. */
    FS_LOCK_REMOTE();

    if(FS_Debug(FS_DEBUG_FLAG_DBGMSG))
    {
        printf("\r\n%s() Read File: %s, size: %ld",
            __FUNCTION__,
            file_attr->file_name,
            file_attr->file_size);
    }

    /* 1.4 - Send the request packet */
    retry_count = 0;
    FS_OM_SetControlFilePtr(FS_OM_GetControlFileBuffer());
    DBGMSG("Send request to remote drive");
    for (;;)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Copy_Content_Request_Packet_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_SEND, FS_COPY_FILE_CONTENT) /* user_id */
                                             );
        copy_rq_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if (NULL == copy_rq_packet_p)
        {
            SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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
            SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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
        EH_MGR_Handle_Exception(SYS_MODULE_FS, FS_STATIC_FUNCTION_FUNC_NO, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
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
    if (FS_Debug(FS_DEBUG_FLAG_DBGMSG))
    {
        printf("\r\n%s(): Remote operation for drive %d at line %d\r\n", __FUNCTION__, (int)drive, __LINE__);
    }
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
            SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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
            SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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
        SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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
        EH_MGR_Handle_Exception(SYS_MODULE_FS, FS_STATIC_FUNCTION_FUNC_NO, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
        return FS_RETURN_NOT_MASTER;
    }

    if (FS_Debug(FS_DEBUG_FLAG_DBGMSG))
    {
        printf("\r\n%s(): Remote operation for drive %d at line %d\r\n", __FUNCTION__, (int)drive, __LINE__);
    }

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
            SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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
        EH_MGR_Handle_Exception(SYS_MODULE_FS, FS_STATIC_FUNCTION_FUNC_NO, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
        return FS_RETURN_NOT_MASTER;
    }

    if (FS_Debug(FS_DEBUG_FLAG_DBGMSG))
    {
        printf("\r\n%s(): Remote operation for drive %d at line %d\r\n", __FUNCTION__, (int)drive, __LINE__);
    }

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
            SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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
        EH_MGR_Handle_Exception(SYS_MODULE_FS, FS_STATIC_FUNCTION_FUNC_NO, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
        return FS_RETURN_NOT_MASTER;
    }

    if (FS_Debug(FS_DEBUG_FLAG_DBGMSG))
    {
        printf("\r\n%s(): Remote operation for drive %d at line %d\r\n", __FUNCTION__, (int)drive, __LINE__);
    }

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
            SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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
    BOOL_T done;
    UI32_T return_value;
    UI32_T retry_count;

    FS_Packet_Header_T   *fs_abort_packet_p;
    L_MM_Mref_Handle_T   *mref_handle_p;
    UI32_T               pdu_len; 

    done = FALSE;
    retry_count = 0;    
    for(;;)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Packet_Header_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_SEND, FS_ABORT) /* user_id */
                                             );
        fs_abort_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if (NULL == fs_abort_packet_p)
        {
            SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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
    UI32_T                  timeout_count;
    UI32_T                  retry_count = 0;
    
    FS_Packet_Header_T      *fs_rq_packet_p;
    L_MM_Mref_Handle_T      *mref_handle_p;
    UI32_T                  pdu_len;  

    timeout_count = 0;
    DBGMSG("FS_CheckRemoteStatus: Check flash programming status");
    for(;;)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_Request_Packet_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_SEND, FS_FLASH_STATUS) /* user_id */
                                             );
        fs_rq_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if (NULL == fs_rq_packet_p)
        {
            SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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
                        if(FS_Debug(FS_DEBUG_FLAG_DBGMSG))
                            printf("\r\nFS_CheckRemoteStatus: Wait the next 3 seconds for the unit %d to try Again...", (int)drive);
                        SYSFUN_Sleep(300);
                    }
                }
                else
                {   /* Flash failed */
                    return_value = fs_packet_buffer.data.error_code;
                    ERRMSG(return_value);
                    if (return_value == FS_RETURN_EXCEED_MAX_NUM_OF_FILES)
                    {
                        printf("\r\nWarning!! The number of files of the given type exceeds the maximum number. Operation Inhibited.\r\n");
                    }
                    break;
                }
            }
            else /* ACK received */
            {
                return_value = FS_RETURN_OK;
                DBGMSG("FS_CheckRemoteStatus: Programming flash completed.");
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

    DBGMSG("FS_CheckRemoteStatus: Status check finished.");
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
        EH_MGR_Handle_Exception(SYS_MODULE_FS, FS_STATIC_FUNCTION_FUNC_NO, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
        return FS_RETURN_NOT_MASTER;
    }

    if (FS_Debug(FS_DEBUG_FLAG_DBGMSG))
    {
        printf("\r\n%s(): Remote operation for drive %d at line %d\r\n", __FUNCTION__, (int)drive, __LINE__);
    }
    
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
            SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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
        SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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
    
    if( FS_Debug(FS_DEBUG_FLAG_DBGMSG) )
        printf("\r\n%s() Read: %s, size: %8ld", __FUNCTION__, name, FS_OM_GetControlAttrFileSize());

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
        EH_MGR_Handle_Exception(SYS_MODULE_FS, FS_STATIC_FUNCTION_FUNC_NO, EH_TYPE_MSG_MEM_ALLOCATE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT);
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
        SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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
    FS_Packet_Header_T  *rx_header;
    FS_File_Attr_T      *rx_file_attr;
    UI32_T               pdu_len;
    

    rx_packet = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    rx_header = (FS_Packet_Header_T *) rx_packet;
    
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
        rx_header   = (FS_Packet_Header_T *) rx_packet;

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
            EH_MGR_Handle_Exception(SYS_MODULE_FS, FS_STATIC_FUNCTION_FUNC_NO, EH_TYPE_MSG_MEM_ALLOCATE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT);
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
    UI8_T               ch = '\\';
    UI32_T              pdu_len;
    
    rx_packet   = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    rx_header   = (FS_Packet_Header_T *) rx_packet;
    rx_data     = (UI8_T *) (rx_packet + sizeof(FS_Packet_Header_T));

    if(FS_Debug(FS_DEBUG_FLAG_DBGMSG))
    {
        ch = (ch == '\\')? '-': '\\';
        printf("\b%c", ch);
    }

    if(FS_Debug(FS_DEBUG_FLAG_DUMP_RXPKT))
    {
        printf("\r\n%s() Received:", __FUNCTION__);
        FS_PrintPacket(rx_packet, sizeof(FS_Packet_Header_T)+rx_header->data_size);
    }

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
    FS_ACK_Packet_T     *ack_packet;    /* ack received */
    UI32_T               pdu_len;

    L_MM_Mref_Handle_T  *rep_mref_handle_p;
    FS_Data_Packet_T    *fs_data_packet_p;    

    ack_packet = (FS_ACK_Packet_T *) L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
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
            SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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
    UI8_T               *rx_packet;
    FS_Packet_Header_T  *rx_header;
    UI32_T               pdu_len;
    

    rx_packet   = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    rx_header   = (FS_Packet_Header_T *) rx_packet;

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
        SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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
        SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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
        SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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
        SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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
    UI32_T  result;
    UI8_T   name[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];
    
    switch(FS_OM_GetControlFlashOperation())
    {
    case FS_FLASH_LOADER:
        DBGMSG("Writing Loader...");
        FS_LOCK( );
/*
        fs_control.flash_result = FS_WriteSpecialBlock(
                                    FS_SPECIAL_BLOCK_LOADER,
                                    fs_control.file_buffer,
                                    fs_control.file_attr.file_size);
*/
        result  =  FS_WriteFlash(
                                    FS_OM_GetControlFileBuffer(),
                                    FS_OM_GetControlAttrFileSize(), 
                                    FS_LOADER);
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
        SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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

    if (FS_Debug(FS_DEBUG_FLAG_DBGMSG))
    {
        printf("\r\n%s() Read: %s, size: %8ld", __FUNCTION__, name, FS_OM_GetControlAttrFileSize());
    }

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
        EH_MGR_Handle_Exception(SYS_MODULE_FS, FS_STATIC_FUNCTION_FUNC_NO, EH_TYPE_MSG_MEM_ALLOCATE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT);
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
        SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
        return;
    }

    fs_data_packet_p->header.opcode = FS_DATA;
    fs_data_packet_p->header.seq_no = FS_OM_GetControlSeqNo();
    if (FS_OM_GetControlAttrFileSize() > FS_MAX_DATA_SIZE)
    {
        fs_data_packet_p->header.data_size = FS_MAX_DATA_SIZE;
        memcpy(fs_data_packet_p->raw_data, FS_OM_GetControlFileBuffer(), FS_MAX_DATA_SIZE);
        FS_OM_SetControlFilePtr(FS_OM_GetControlFileBuffer() + FS_MAX_DATA_SIZE);
//        fs_control.file_attr.file_size -= FS_MAX_DATA_SIZE;
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
            printf("\r\n%s: Check File %s", __FUNCTION__, file_name);

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
                printf("\r\n%s: File %s check sum error!! Removed.", __FUNCTION__, file_name);
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
        SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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
        SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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
    printf(" ");
    for(index = 0; index < char_count; index++)
        printf("%02X ", string[index]);

    for(; index < FS_WORD_WRAP; index++)
            printf("%s", "   ");
    
    printf("| ");
    for(index = 0; index < char_count; index++)
    {
        if(string[index] < ' ' || string[index] > '~')
            printf(".");
        else
            printf("%c", string[index]);
    }
    printf("\r\n");
}

static void FS_PrintPacket(UI8_T *packet, UI16_T size)
{
    char title_bar[] = {
" 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F | 0123456789ABCDEF\r\n\
-------------------------------------------------+------------------"
};

    FS_Packet_Header_T  *header;
    
    header = (FS_Packet_Header_T *) packet;
    printf("\r\nOpcode:     %s", FS_Opcode_Names[header->opcode]);
    printf("\r\nSequence:   %d", header->seq_no);
    printf("\r\nSize:       %d", header->data_size);
    printf("\r\nPacket Content:\r\n%s\r\n", title_bar);
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
        if(FS_Debug(FS_DEBUG_FLAG_DBGMSG))
        {
            printf("\r\nError : File name %s contains an invalid character!", file_name);
        }
        return FALSE;
    }

    /* check file length */
    if (length <= 0)
    {
        if(FS_Debug(FS_DEBUG_FLAG_DBGMSG))
        {
            printf("\r\nError : File length is an invalid value!");
        }
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
                SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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
        while (!done)
        {
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(FS_MCast_Data_Packet_T), /* tx_buffer_size */
                                                  L_MM_USER_ID(SYS_MODULE_FS, FS_POOL_ID_ISC_SEND, FS_MCAST_DATA) /* user_id */
                                                 );
            fs_mcast_data_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
            if (NULL == fs_mcast_data_packet_p)
            {
                SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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
                SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
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
    FS_Packet_Header_T  *rx_header;
    FS_File_Attr_T      *rx_file_attr;
    UI32_T               pdu_len;


    rx_packet = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    rx_header = (FS_Packet_Header_T *) rx_packet;

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
            EH_MGR_Handle_Exception(SYS_MODULE_FS, FS_STATIC_FUNCTION_FUNC_NO, EH_TYPE_MSG_MEM_ALLOCATE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT);
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
    UI8_T                   ch = '\\';
    UI32_T                  pdu_len;


    rx_packet           = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    mcast_data_packet   = (FS_MCast_Data_Packet_T *) rx_packet;

    if (FS_Debug(FS_DEBUG_FLAG_DBGMSG))
    {
        ch = (ch == '\\')? '-': '\\';
        printf("\b%c", ch);
    }

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
    UI8_T               *rx_packet;
    FS_Packet_Header_T  *rx_header;
    UI32_T               pdu_len;

    rx_packet   = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    rx_header   = (FS_Packet_Header_T *) rx_packet;

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

