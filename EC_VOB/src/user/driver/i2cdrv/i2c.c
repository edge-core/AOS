/* Module Name: i2c.c
 * Purpose: 
 *         the implementation to access i2c in linux user mode
 *         
 *
 * Notes:  
 *        
 * History:                                                               
 *    09/04/2007 - Echo Chen, Created	
 *       
 * Copyright(C)      Accton Corporation, 2007   				
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysfun.h"
#include "phyaddr_access.h"
#include "i2c_export.h"
#include "sys_cpnt.h"
#include "sys_hwcfg.h"
#include <fcntl.h>
#include "i2c.h"
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "l_bitmap.h"
#include "stktplg_om.h"
#include "leaf_sys.h" /* for SYS_VAL_LOCAL_UNIT_ID */

/* NAMING CONSTANT DECLARATIONS
 */
#define MAX_EINTR_RETRY 10
#define I2C_DEV_FILENAME_TEMPLATE "/dev/i2c-%d"
#define I2C_DEV_FILENAME_MAX_LEN 10
#define I2C_DEFAULT_BUS_IDX 0

/* I2C_MAX_NBR_OF_INTERNAL_MUX_CHANNEL_INDEX
 *   This costant defines the maximum number of channel index on a I2C mux device
 *   can be used internally within this c file. Note that the channel index is 0
 *   based.
 */
#define I2C_MAX_NBR_OF_INTERNAL_MUX_CHANNEL_INDEX 31

/* I2C_INTERNAL_MUX_CHANNEL_BMP_ARRAY_SIZE
 *   This constant defines the required array size of the channel bitmap of a I2C
 *   mux device to be used internally within this c file. The value is evaulated
 *   through the definition of I2C_MAX_NBR_OF_INTERNAL_MUX_CHANNEL_INDEX.
 *   Note:
 *     1. The channel bitmap array uses UI32_T array.
 *     2. The number of required bit for the bitmap is
 *        (I2C_MAX_NBR_OF_INTERNAL_MUX_CHANNEL_INDEX+1) because index 0 occupies
 *        one bit position.
 */
#define I2C_INTERNAL_MUX_CHANNEL_BMP_ARRAY_SIZE   (((I2C_MAX_NBR_OF_INTERNAL_MUX_CHANNEL_INDEX+1)+31)/32)

#if (SYS_CPNT_I2CDRV_IOCTL_USE_I2C_SLAVE_FORCE==TRUE)
#define I2C_SLAVE_IOCTL_CMD I2C_SLAVE_FORCE
#else
#define I2C_SLAVE_IOCTL_CMD I2C_SLAVE
#endif

/* MACRO FUNCTION DECLARATIONS
 */
/* I2C_INTERNAL_MUX_CHANNEL_BMP_ARRAY
 *   This macro function is defined for the shorthand of channel bitmap
 *   array defined in I2C_InternalMuxChannelBmp_T.
 *   
 */
#define I2C_INTERNAL_MUX_CHANNEL_BMP_ARRAY(internal_mux_channel_bmp_type) ((internal_mux_channel_bmp_type).channel_bmp_ar)

/* FUNCTION NAME : I2C_DUMP_INTERNAL_MUX_CHANNEL_BMP
 * PURPOSE:
 *      This macro function is used to dump the array content of internal mux
 *      channel bitmap.
 *
 * INPUT:
 *      title_msg                - The title message that will be printed before
 *                                 the array content.
 *
 *      internal_mux_channel_bmp - The internal channel bitmap data to be dumped.
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
#define I2C_DUMP_INTERNAL_MUX_CHANNEL_BMP(title_msg, internal_mux_channel_bmp) \
do { \
    UI8_T __channel_bmp_ar_idx; \
                                \
    printf(title_msg); \
    for (__channel_bmp_ar_idx=0; __channel_bmp_ar_idx<I2C_INTERNAL_MUX_CHANNEL_BMP_ARRAY_SIZE; __channel_bmp_ar_idx++) \
        printf("0x%08lX ", I2C_INTERNAL_MUX_CHANNEL_BMP_ARRAY((internal_mux_channel_bmp))[__channel_bmp_ar_idx]); \
    printf("\r\n"); \
} while(0)

/* DATA TYPE DECLARATIONS
 */
#if 0
#define dprintf(fmtstr, arg...) printf("%s(%d)"fmtstr, __FUNCTION__, __LINE__, ##arg);
#else
#define dprintf(x...)
#endif

/* I2C_InternalMuxChannelBmp_T
 *   This is the data type for storing I2C mux channel bitmap internally
 *   within this c file.
 */
typedef struct
{
    UI32_T channel_bmp_ar[I2C_INTERNAL_MUX_CHANNEL_BMP_ARRAY_SIZE];
} I2C_InternalMuxChannelBmp_T;

/* I2C_MultiLevelMuxInfoArrayForOneBoardPtr_T
 *   This is the data type for accessing the array of multi level I2C mux device
 *   info of a specific board.
 */
typedef const SYS_HWCFG_MultiLevelMuxInfoEntry_T (*I2C_MultiLevelMuxInfoArrayForOneBoardPtr_T)[SYS_HWCFG_NUM_OF_I2C_MUX];

/* LOCAL SUBPROGRAM DECLARATIONS
 */
#if (SYS_CPNT_I2C == TRUE)
static BOOL_T I2C_InternalMuxChannelBmpIsAllZeros(I2C_InternalMuxChannelBmp_T *channel_bmp_p);
static BOOL_T I2C_MuxTranslateChannelBMPToSetVal(SYS_HWCFG_COMMON_I2C_Mux_Channel_BMP_To_Set_Val_Translation_Type_T bmp_xlat_type, I2C_InternalMuxChannelBmp_T* channel_bmp_p, UI8_T *set_val_p);
#if (SYS_CPNT_SELECT_CHANNEL_MODE==SYS_CPNT_SELECT_CHANNEL_MODE__I2C_MULTI_LEVEL_MUX)
static BOOL_T I2C_GetMultiLevelMuxInfoArrayForLocalUnit(I2C_MultiLevelMuxInfoArrayForOneBoardPtr_T* i2c_mux_info_p);
#endif
static I2C_Status I2C_Read(unsigned int  file, unsigned char data_adr,unsigned char *buf, size_t count);
static I2C_Status I2C_ReadWithoutDataAdr(unsigned int  i2c_fd, unsigned char *buf, size_t count);
static I2C_Status I2C_Write(unsigned int file, unsigned char data_adr, unsigned char *buf, size_t count);
#define I2C_BUFFER_MAXSIZE 256  
char I2C_ADAPTOR_NAME[64]="/dev/i2c-0"; 
#endif

/* FUNCTION NAME: I2C_do_transaction
 * PURPOSE  :  Perform the given I2C transaction, only MASTER_XMIT and MASTER_RCV
 *             are implemented.
 * INPUT    :   act        -- the type of transaction
 *              i2c_addr   -- the I2C address of the slave device
 *              data_addr  -- is the address of the data on the slave device
 *              len        -- the length of data to send or receive
 *              buffer     -- the address of the data buffer            
 * OUTPUT   : 
 * RETURN   : 
 * NOTES    : 
 */
I2C_Status I2C_do_transaction(I2C_TRANSACTION_MODE act, unsigned char i2c_addr, unsigned char data_addr, int len, char * buffer)
{
    return I2C_Transaction(act, i2c_addr, data_addr, len, buffer);
}

#if (SYS_CPNT_I2C == TRUE)
/* FUNCTION NAME: I2C_InternalMuxChannelBmpIsAllZeros
 * PURPOSE  :  Check whether bits in the given channel bitmap are all zeros.
 * INPUT    :  channel_bmp_p    -- the channel bitmap to be checked.
 * RETURN   :  TRUE  -  All bits in the given channel bitmap are zeros.
 *             FALSE -  At least one bit in the given channel bitmap is one.
 * NOTES    :  Caller of this function must ensure channel_bmp_p is a valid pointer.
 */
static BOOL_T I2C_InternalMuxChannelBmpIsAllZeros(I2C_InternalMuxChannelBmp_T *channel_bmp_p)
{
    UI8_T i;

    for (i=0; i<I2C_INTERNAL_MUX_CHANNEL_BMP_ARRAY_SIZE; i++)
    {
        if (I2C_INTERNAL_MUX_CHANNEL_BMP_ARRAY(*channel_bmp_p)[i]!=0)
            return FALSE;
    }
    return TRUE;
}

/* FUNCTION NAME: I2C_MuxTranslateChannelBMPToSetVal
 * PURPOSE  :  Translate the channel bitmap to the value to be set
 *             to the I2C mux according to the bmp_xlat_type.
 * INPUT    :  bmp_xlat_type    -- the type of translation
 *             channel_bmp      -- the channel bitmap value
 * OUTPUT   :  set_val_p        -- the value to be set to the I2C mux
 * RETURN   :  TRUE  -  The translation operation is successful.
 *             FALSE -  The translation operation is failed.
 * NOTES    :  See comment of SYS_HWCFG_COMMON_I2C_Mux_Channel_BMP_To_Set_Val_Translation_Type_T
 *             in sys_hwcfg_common.h for description of each type of
 *             translation.
 */
static BOOL_T I2C_MuxTranslateChannelBMPToSetVal(SYS_HWCFG_COMMON_I2C_Mux_Channel_BMP_To_Set_Val_Translation_Type_T bmp_xlat_type, I2C_InternalMuxChannelBmp_T* channel_bmp_p, UI8_T *set_val_p)
{
    if(bmp_xlat_type>SYS_HWCFG_COMMON_I2C_MUX_CHANNEL_BMP_TO_SET_VAL_TRANSLATION_TYPE_MAX)
    {
        printf("%s(%d):Invalid bmp_xlat_type=%d\r\n", __FUNCTION__, __LINE__, (int)bmp_xlat_type);
        return FALSE;
    }

    if (set_val_p==NULL)
    {
        printf("%s(%d):set_val_p is NULL\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (channel_bmp_p==NULL)
    {
        printf("%s(%d):channel_bmp_p is NULL\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    switch (bmp_xlat_type)
    {
        case SYS_HWCFG_COMMON_I2C_MUX_CHANNEL_BMP_TO_SET_VAL_TRANSLATION_TYPE_DIRECT:
        {
            if (I2C_INTERNAL_MUX_CHANNEL_BMP_ARRAY(*channel_bmp_p)[0]>0xFF)
            {
                printf("%s(%d):channel_bmp translation overflow!(0x%08lX)\r\n",
                    __FUNCTION__, __LINE__, I2C_INTERNAL_MUX_CHANNEL_BMP_ARRAY(*channel_bmp_p)[0]);
                return FALSE;
            }
            *set_val_p = (UI8_T)(I2C_INTERNAL_MUX_CHANNEL_BMP_ARRAY(*channel_bmp_p)[0]);
        }
            break;
        case SYS_HWCFG_COMMON_I2C_MUX_CHANNEL_BMP_TO_SET_VAL_TRANSLATION_TYPE_BITMAP_TO_VAL:
        {
            UI32_T num_of_bits;
            UI8_T bit_pos_list[32];
            UI8_T channel_bmp_array_idx;
            BOOL_T translation_ok=FALSE;

            if (I2C_InternalMuxChannelBmpIsAllZeros(channel_bmp_p)==TRUE)
            {
                *set_val_p = (UI8_T)(0xFF);
                translation_ok=TRUE;
            }
            else
            {
                for (channel_bmp_array_idx=0; channel_bmp_array_idx<I2C_INTERNAL_MUX_CHANNEL_BMP_ARRAY_SIZE; channel_bmp_array_idx++)
                {
                    num_of_bits=L_BITMAP_Get_BitPos_List(I2C_INTERNAL_MUX_CHANNEL_BMP_ARRAY(*channel_bmp_p)[channel_bmp_array_idx], bit_pos_list);

                    if (num_of_bits==0)
                    {
                        continue;
                    }
                    else if (num_of_bits==1)
                    {
                        *set_val_p = (32*channel_bmp_array_idx) + bit_pos_list[0];
                        translation_ok=TRUE;
                        break;
                    }
                    else
                    {
                        printf("%s(%d):Illegal channel_bmp value(idx=%hu, val=0x%08lX)\r\n",
                            __FUNCTION__, __LINE__, channel_bmp_array_idx, I2C_INTERNAL_MUX_CHANNEL_BMP_ARRAY(*channel_bmp_p)[channel_bmp_array_idx]);
                        break;
                    }
                }
            }
            return (translation_ok==TRUE) ? TRUE:FALSE;
        }
            break;
        case SYS_HWCFG_COMMON_I2C_MUX_CHANNEL_BMP_TO_SET_VAL_TRANSLATION_TYPE_BITMAP_TO_VAL_2:
        {
            UI32_T num_of_bits;
            UI8_T bit_pos_list[32];
            UI8_T channel_bmp_array_idx;
            BOOL_T translation_ok=FALSE;

            if (I2C_InternalMuxChannelBmpIsAllZeros(channel_bmp_p)==TRUE)
            {
                *set_val_p = (UI8_T)(0x0);
                translation_ok=TRUE;
            }
            else
            {
                for (channel_bmp_array_idx=0; channel_bmp_array_idx<I2C_INTERNAL_MUX_CHANNEL_BMP_ARRAY_SIZE; channel_bmp_array_idx++)
                {
                    num_of_bits=L_BITMAP_Get_BitPos_List(I2C_INTERNAL_MUX_CHANNEL_BMP_ARRAY(*channel_bmp_p)[channel_bmp_array_idx], bit_pos_list);

                    if (num_of_bits==0)
                    {
                        continue;
                    }
                    else if (num_of_bits==1)
                    {
                        if (bit_pos_list[0]>=19)
                            *set_val_p = bit_pos_list[0] + (32*channel_bmp_array_idx) + 13;
                        else if (bit_pos_list[0]>=9)
                            *set_val_p = bit_pos_list[0] + (32*channel_bmp_array_idx) + 7;
                        else
                            *set_val_p = bit_pos_list[0] + (32*channel_bmp_array_idx) + 1;

                        translation_ok=TRUE;
                        break;
                    }
                    else
                    {
                        printf("%s(%d):Illegal channel_bmp value(idx=%hu, val=0x%08lX)\r\n",
                            __FUNCTION__, __LINE__, channel_bmp_array_idx, I2C_INTERNAL_MUX_CHANNEL_BMP_ARRAY(*channel_bmp_p)[channel_bmp_array_idx]);
                        break;
                    }
                }
            }
            return (translation_ok==TRUE) ? TRUE: FALSE;
        }
            break;

        default:
            return FALSE;
            break;
    }
    return TRUE;
}

#if (SYS_CPNT_SELECT_CHANNEL_MODE==SYS_CPNT_SELECT_CHANNEL_MODE__I2C_MULTI_LEVEL_MUX)
/* FUNCTION NAME: I2C_GetMultiLevelMuxInfoArrayForLocalUnit
 * PURPOSE  :  Get the multi-level I2C mux info array for this unit
 * INPUT    :  None
 * OUTPUT   :  i2c_mux_info_p  -- The pointer to the array of the multi-level I2C mux info for this unit.
 * RETURN   :  TRUE  -  The pointer of the array is output successfully.
 *             FALSE -  Failed to output the pointer of the array.
 * NOTES    :  This function is referenced when SYS_CPNT_SELECT_CHANNEL_MODE is defined
 *             as SYS_CPNT_SELECT_CHANNEL_MODE__I2C_MULTI_LEVEL_MUX.
 */
static BOOL_T I2C_GetMultiLevelMuxInfoArrayForLocalUnit(I2C_MultiLevelMuxInfoArrayForOneBoardPtr_T* i2c_mux_info_p)
{
#if defined(SYS_HWCFG_I2C_MULTI_LEVEL_MUX_INFO_WITH_MULTIPLE_BIDS_ARRAY)
    static SYS_HWCFG_MultiLevelMuxInfoEntry_T i2c_mux_info[][SYS_HWCFG_NUM_OF_I2C_MUX] =
        SYS_HWCFG_I2C_MULTI_LEVEL_MUX_INFO_WITH_MULTIPLE_BIDS_ARRAY;
#elif defined(SYS_HWCFG_I2C_MULTI_LEVEL_MUX_INFO_ARRAY)
    static SYS_HWCFG_MultiLevelMuxInfoEntry_T i2c_mux_info[SYS_HWCFG_NUM_OF_I2C_MUX] =
        SYS_HWCFG_I2C_MULTI_LEVEL_MUX_INFO_ARRAY;
#else
    #error "Neither SYS_HWCFG_I2C_MULTI_LEVEL_MUX_INFO_WITH_MULTIPLE_BIDS_ARRAY nor SYS_HWCFG_I2C_MULTI_LEVEL_MUX_INFO_ARRAY is defined."
#endif

#if defined(SYS_HWCFG_I2C_MULTI_LEVEL_MUX_INFO_WITH_MULTIPLE_BIDS_ARRAY)
    UI32_T board_id;
    UI8_T  max_board_id_in_array;

    if (STKTPLG_OM_GetUnitBoardID(SYS_VAL_LOCAL_UNIT_ID, &board_id)==FALSE)
    {
        printf("%s(%d):Failed to get board id\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    max_board_id_in_array=sizeof(i2c_mux_info)/(sizeof(SYS_HWCFG_MultiLevelMuxInfoEntry_T)*SYS_HWCFG_NUM_OF_I2C_MUX);
    if (board_id>=max_board_id_in_array)
    {
        printf("%s(%d):Invalid board id %lu.(Max board id in array=%hu)\r\n", __FUNCTION__, __LINE__,
            board_id, max_board_id_in_array);
        return FALSE;
    }
    *i2c_mux_info_p = (I2C_MultiLevelMuxInfoArrayForOneBoardPtr_T)&(i2c_mux_info[board_id][0]);
#else
    *i2c_mux_info_p = &i2c_mux_info;
#endif /* #if defined(SYS_HWCFG_I2C_MULTI_LEVEL_MUX_INFO_WITH_MULTIPLE_BIDS_ARRAY) */

    return TRUE;
}
#endif /* #if (SYS_CPNT_SELECT_CHANNEL_MODE==SYS_CPNT_SELECT_CHANNEL_MODE__I2C_MULTI_LEVEL_MUX) */

/* FUNCTION NAME : I2C_Read
 * PURPOSE : This function read the i2c slave device data
 * INPUT   : file,data_adr and count 
 * OUTPUT  : buf 
 * RETUEN  : I2C_SUCCESS/I2C_ERROR
 * NOTES   : 
 */ 
static I2C_Status I2C_Read(unsigned int  i2c_fd, unsigned char data_adr, unsigned char *buf, size_t count)
{
    int ret = 0;
    int retry_count=0;

    /*write data address: offset address */
retry1:
    if((ret = write (i2c_fd,  &data_adr, 1)) !=1 )
    {
        dprintf("failed to write data_adr=0x%x(errno=%d)(retry_count=%d)\r\n", data_adr, errno, retry_count);
        /* On AOS5700-54X, will get EIO error when accessing CPLD2(0x61) sometimes.
         * Add retry case when errorno equals EIO.
         */
        if((errno==EINTR) || (errno==EIO))
        {
            retry_count++;
            if(retry_count<MAX_EINTR_RETRY) goto retry1;
        }
        return I2C_ERROR;
    }

    retry_count=0;
retry2:
    /* read data */
    if((ret = read (i2c_fd, buf, count)) < 0 )
    {
        dprintf("failed to read (errno=%d)(retry_count=%d)\r\n", errno, retry_count);

        /* On AOS5700-54X, will get EIO error when accessing CPLD2(0x61) sometimes.
         * Add retry case when errorno equals EIO.
         */
        if((errno==EINTR) || (errno==EIO))
        {
            retry_count++;
            if(retry_count<MAX_EINTR_RETRY) goto retry2;
        }

        return I2C_ERROR;
    }

    return I2C_SUCCESS;
}

/* FUNCTION NAME : I2C_ReadWithoutDataAdr
 * PURPOSE : This function read the i2c slave device data
 * INPUT   : i2c_fd  --  file descriptor of i2c device
 *           count   --  the length to be read from the i2c device
 * OUTPUT  : buf     --  the data fread from the i2c device
 * RETUEN  : I2C_SUCCESS/I2C_ERROR
 * NOTES   : 
 */ 
static I2C_Status I2C_ReadWithoutDataAdr(unsigned int  i2c_fd, unsigned char *buf, size_t count)
{
    int ret = 0;
    int retry_count=0;

retry1:
    /* read data */
    if((ret = read (i2c_fd, buf, count)) < 0 )
    {
        dprintf("failed to read (errno=%d)\r\n", errno);

        /* On AOS5700-54X, will get EIO error when accessing CPLD2(0x61) sometimes.
         * Add retry case when errorno equals EIO.
         */
        if((errno==EINTR) || (errno==EIO))
        {
            retry_count++;
            if(retry_count<MAX_EINTR_RETRY) goto retry1;
        }
        return I2C_ERROR;
    }
    
    return I2C_SUCCESS;
}

/* FUNCTION NAME : I2C_Write
 * PURPOSE : This function write the data to I2C devices !
 * INPUT   : i2c_fd,data_adr ,buf and count 
 * OUTPUT  : 
 * RETUEN  : ret/I2C_ERROR
 * NOTES   : 
 */
static I2C_Status I2C_Write(unsigned int i2c_fd, unsigned char data_adr, unsigned char *buf, size_t count)
{
    int ret = 0;
    int retry_count;
    static char sendbuffer[I2C_BUFFER_MAXSIZE];
    sendbuffer[0] = 0;

    /*write address: offset address */
    sendbuffer[0] = data_adr;
    memcpy(sendbuffer+1, buf, count);

    retry_count=0;
    /* write data */
retry:
    if((ret = write (i2c_fd,  sendbuffer, count+1)) < 0)
    {
        dprintf("failed to write (errno=%d)\r\n", errno);
        /* On AOS5700-54X, will get EIO error when accessing CPLD2(0x61) sometimes.
         * Add retry case when errorno equals EIO.
         */
        if((errno==EINTR) || (errno==EIO))
        {
            retry_count++;
            if(retry_count<MAX_EINTR_RETRY) goto retry;
        }
        return I2C_ERROR;
    }

    return I2C_SUCCESS;
}

/* FUNCTION NAME : I2C_setChannel
 * PURPOSE : This function set pca9548 device Channel
 * INPUT   : channel_addr ,channel_regaddr and channel_num
 * OUTPUT  : 
 * RETUEN  : I2C_SUCCESS/I2C_ERROR
 * NOTES   : 
 */
 I2C_Status I2C_SetChannel(UI32_T channel_addr, UI8_T channel_regaddr, UI8_T channel_num)
{
#if (SYS_CPNT_SELECT_CHANNEL_MODE==SYS_CPNT_SELECT_CHANNEL_MODE_NOCHANNEL)
    /* no channel device between I2C bus and I2C device
     */
    return I2C_SUCCESS;
#elif (SYS_CPNT_SELECT_CHANNEL_MODE==SYS_CPNT_SELECT_CHANNEL_MODE__PCA9548)
    int ret;
    int i2cfddev;
    int i;
    unsigned char channel_map=0;

    /* pca9548 device between I2C bus and I2C device
     */
    /*open device*/
    if ((i2cfddev = open(I2C_ADAPTOR_NAME, O_RDWR)) < 0)
    {
        dprintf("errno=%d\r\n", errno);
        return I2C_ERROR;
    }

     /* set i2c 7 bit mode */
     if((ret = ioctl( i2cfddev, I2C_TENBIT, 0)) != 0)
    {
        dprintf("errno=%d\r\n", errno);
        close(i2cfddev);
        return I2C_ERROR;
    }

    if((ret = ioctl( i2cfddev, I2C_SLAVE_IOCTL_CMD, channel_addr)) != 0)
    {
        dprintf("errno=%d\r\n", errno);
        close(i2cfddev);
        return I2C_ERROR;
    }

    /* set i2c pca channel */
    /* channel_num = 0 => channel_map = 0 */
    if(channel_num!=0)
        channel_map = 1 <<( channel_num-1);

    if((ret = I2C_Write(i2cfddev, channel_regaddr, &channel_map, 1)) != 0 )
    {
        dprintf("errno=%d\r\n", errno);
        close(i2cfddev);
        return I2C_ERROR;
    }

    /* wait some time */
    for(i=0;i<100;i++);
        close(i2cfddev);

    return I2C_SUCCESS;
#elif (SYS_CPNT_SELECT_CHANNEL_MODE==SYS_CPNT_SELECT_CHANNEL_MODE__EPLD)
    if(FALSE==PHYSICAL_ADDR_ACCESS_Write(channel_addr, 1, 1, &channel_num))
    {
        SYSFUN_Debug_Printf("\r\n%s: READ CPLD_VERSION INFORMATIN Dfail", __FUNCTION__);
        return I2C_ERROR;
    }
    return I2C_SUCCESS;
#else
    printf("%s(%d): Obsoleted API, do not use.\r\n", __FUNCTION__, __LINE__);
    return I2C_ERROR;
#endif
}

/* FUNCTION NAME : I2C_SetMux
 * PURPOSE : This function will set the channel on the multiplexer
 * INPUT   : index       - multiplexer index (0 based)
 *           channel_bmp - channel bitmap, bit 0 for channel 1, bit 1 for
 *                         channel 2 and so on.
 * OUTPUT  : None.
 * RETURN  : I2C_SUCCESS/I2C_ERROR
 * NOTES   : This function shall be called by I2CDRV only.
 */
I2C_Status I2C_SetMux(UI32_T index, UI32_T channel_bmp)
{
    return I2C_SetMuxWithFunPtr(index, channel_bmp, NULL);
}
/* FUNCTION NAME : I2C_SetMuxWithFunPtr
 * PURPOSE : This function will set the channel on the multiplexer
 * INPUT   : index       - multiplexer index (0 based)
 *           channel_bmp - channel bitmap, bit 0 for channel 1, bit 1 for
 *                         channel 2 and so on.
 *           write_fn_p -  not used here(used in i2c_marvell.c)
 * OUTPUT  : None.
 * RETURN  : I2C_SUCCESS/I2C_ERROR
 * NOTES   : This function shall be called by I2CDRV only.
 */
I2C_Status I2C_SetMuxWithFunPtr(UI32_T index, UI32_T channel_bmp, I2C_TwsiDataWrite_Func_T write_fn_p)
{
#if (SYS_CPNT_SELECT_CHANNEL_MODE==SYS_CPNT_SELECT_CHANNEL_MODE_NOCHANNEL)
    /* no channel device between I2C bus and I2C device
     */
    return I2C_SUCCESS;

#elif (SYS_CPNT_SELECT_CHANNEL_MODE==SYS_CPNT_SELECT_CHANNEL_MODE__PCA9548)
    UI8_T i2c_mux_addrs[SYS_HWCFG_NUM_OF_I2C_MUX] = SYS_HWCFG_I2C_MUX_ADDR_ARRAY;
    unsigned char channel_map;
    int ret;
    int i2cfddev;
    int i;

    /* pca9548 device between I2C bus and I2C device
     */
    /*open device*/
    if ((i2cfddev = open(I2C_ADAPTOR_NAME, O_RDWR)) < 0)
    {
        dprintf("errno=%d\r\n", errno);
        return I2C_ERROR;
    }

    /* set i2c 7 bit mode */
    if((ret = ioctl( i2cfddev, I2C_TENBIT, 0)) != 0)
    {
        dprintf("errno=%d\r\n", errno);
        close(i2cfddev);
        return I2C_ERROR;
    }

    if((ret = ioctl( i2cfddev, I2C_SLAVE_IOCTL_CMD, i2c_mux_addrs[index])) != 0)
    {
        dprintf("errno=%d\r\n", errno);
        close(i2cfddev);
        return I2C_ERROR;
    }

    channel_map=(unsigned char)channel_bmp;

    if((ret = I2C_Write(i2cfddev, i2c_mux_addrs[index], &channel_map, 1)) != 0 )
    {
        dprintf("errno=%d\r\n", errno);
        close(i2cfddev);
        return I2C_ERROR;
    }

    /* wait some time */
    for(i=0;i<100;i++);

    close(i2cfddev);
    return I2C_SUCCESS;

#elif (SYS_CPNT_SELECT_CHANNEL_MODE==SYS_CPNT_SELECT_CHANNEL_MODE__I2C_MULTI_LEVEL_MUX)
    I2C_MultiLevelMuxInfoArrayForOneBoardPtr_T i2c_mux_info_p;
    I2C_InternalMuxChannelBmp_T channel_bmp_on_mux;
    I2C_InternalMuxChannelBmp_T channel_bmp_on_leaf_mux;
    I8_T  int_mux_idx;
    UI8_T data, internal_channel_bmp_array_index;
    BOOL_T rc;

    if (index>=SYS_HWCFG_NUM_OF_I2C_MUX)
    {
        printf("%s(%d): Invalid index %lu\r\n", __FUNCTION__, __LINE__, index);
        return I2C_ERROR;
    }

    if (I2C_GetMultiLevelMuxInfoArrayForLocalUnit(&i2c_mux_info_p)==FALSE)
    {
        printf("%s(%d): Failed to get multi level mux info array.\r\n", __FUNCTION__, __LINE__);
        return I2C_ERROR;
    }

    if ((*i2c_mux_info_p)[index].common_info.num_of_intermediate_mux > SYS_HWCFG_I2C_MAX_INTERMEDIATE_MUX_NUM)
    {
        printf("%s(%d): Illegal num_of_intermediate_mux=%d. Max=%d (mux idx=%d)\r\n",
            __FUNCTION__, __LINE__, (int)((*i2c_mux_info_p)[index].common_info.num_of_intermediate_mux),
            (int)SYS_HWCFG_I2C_MAX_INTERMEDIATE_MUX_NUM,
            (int)index);

    }

    /* set channel on intermediate mux if required
     */
    if ((*i2c_mux_info_p)[index].common_info.num_of_intermediate_mux > 0)
    {
        for (int_mux_idx=0; int_mux_idx<(*i2c_mux_info_p)[index].common_info.num_of_intermediate_mux; int_mux_idx++)
        {

            internal_channel_bmp_array_index = ((*i2c_mux_info_p)[index].channel_idx_on_mux[int_mux_idx])/32;
            if (internal_channel_bmp_array_index >= I2C_INTERNAL_MUX_CHANNEL_BMP_ARRAY_SIZE)
            {
                printf("%s(%d):Invalid channel bitmap array index when doing translation.(mux_idx=%lu, int_mux_index=%hu, channel_idx_on_mux=%d, bmp_to_val_xlat_type=%d\r\n",
                    __FUNCTION__, __LINE__, index, int_mux_idx, (int)((*i2c_mux_info_p)[index].channel_idx_on_mux[int_mux_idx]),
                    (int)((*i2c_mux_info_p)[index].intermediate_mux_channel_bmp_xslt_type[int_mux_idx]) );
                return I2C_ERROR;
            }
            memset(&channel_bmp_on_mux, 0, sizeof(channel_bmp_on_mux));
            I2C_INTERNAL_MUX_CHANNEL_BMP_ARRAY(channel_bmp_on_mux)[internal_channel_bmp_array_index] |= (1UL << ((*i2c_mux_info_p)[index].channel_idx_on_mux[int_mux_idx]%32));
            
            if (I2C_MuxTranslateChannelBMPToSetVal((*i2c_mux_info_p)[index].intermediate_mux_channel_bmp_xslt_type[int_mux_idx], &channel_bmp_on_mux, &data)==FALSE)
            {
                printf("%s(%d):Failed to translate channel bmp to set value.(mux_idx=%lu, int_mux_index=%hu, bmp_to_val_xlat_type=%d\r\n",
                    __FUNCTION__, __LINE__, index, int_mux_idx, (int)((*i2c_mux_info_p)[index].intermediate_mux_channel_bmp_xslt_type[int_mux_idx]) );
                I2C_DUMP_INTERNAL_MUX_CHANNEL_BMP("Intermediate Mux Channel Bitmap:", channel_bmp_on_mux);
                return I2C_ERROR;
            }

            rc=I2C_TwsiDataWriteWithBusIdx((*i2c_mux_info_p)[index].common_info.i2c_bus_idx,
                (*i2c_mux_info_p)[index].dev_addr_of_intermediate_mux[int_mux_idx],
                I2C_7BIT_ACCESS_MODE,
                (*i2c_mux_info_p)[index].intermediate_mux_has_channel_reg[int_mux_idx],
                (*i2c_mux_info_p)[index].intermediate_mux_channel_reg_addr[int_mux_idx], FALSE, &data, 1);

            if (rc == FALSE)
            {
                printf("%s(%d): I2C write error.(mux_idx=%lu, int_mux_index=%hu, int_mux_has_channel_reg=%hu, int_mux_channel_reg_addr=0x%x, write value=0x%x)\r\n",
                    __FUNCTION__, __LINE__, index, int_mux_idx, (*i2c_mux_info_p)[index].intermediate_mux_has_channel_reg[int_mux_idx],
                    (*i2c_mux_info_p)[index].intermediate_mux_channel_reg_addr[int_mux_idx], data);
                return I2C_ERROR;
            }
        }
    }

    /* set channel on leaf mux
     */
    memset(&channel_bmp_on_leaf_mux, 0, sizeof(channel_bmp_on_leaf_mux));
    I2C_INTERNAL_MUX_CHANNEL_BMP_ARRAY(channel_bmp_on_leaf_mux)[0]=channel_bmp;
    if (I2C_MuxTranslateChannelBMPToSetVal((*i2c_mux_info_p)[index].common_info.leaf_mux_channel_bmp_xslt_type, &channel_bmp_on_leaf_mux, &data)==FALSE)
    {
        printf("%s(%d):Failed to translate channel bmp to set value.(bus_idx=%hu, mux_idx=%lu, channel_bmp=0x%08lX, bmp_to_val_xlat_type=%d\r\n",
            __FUNCTION__, __LINE__, (*i2c_mux_info_p)[index].common_info.i2c_bus_idx, index, channel_bmp, (int)((*i2c_mux_info_p)[index].common_info.leaf_mux_channel_bmp_xslt_type) );
        return I2C_ERROR;
    }

    rc=I2C_TwsiDataWriteWithBusIdx((*i2c_mux_info_p)[index].common_info.i2c_bus_idx,
        (*i2c_mux_info_p)[index].common_info.leaf_mux_dev_addr, I2C_7BIT_ACCESS_MODE,
        (*i2c_mux_info_p)[index].common_info.leaf_mux_dev_has_channel_reg,
        (*i2c_mux_info_p)[index].common_info.leaf_mux_dev_channel_reg_addr, FALSE, &data, 1);
    if (rc == FALSE)
    {
        printf("%s(%d): I2C write error.(bus_idx=%hu, mux_idx=%lu, mux_has_channel_reg=%hu, mux_channel_reg_addr=0x%x, leaf_mux_dev_addr=0x%x, write value=0x%x)\r\n",
            __FUNCTION__, __LINE__,(*i2c_mux_info_p)[index].common_info.i2c_bus_idx,
            index, (*i2c_mux_info_p)[index].common_info.leaf_mux_dev_has_channel_reg,
            (*i2c_mux_info_p)[index].common_info.leaf_mux_dev_channel_reg_addr,
            (*i2c_mux_info_p)[index].common_info.leaf_mux_dev_addr,
            data);
        return I2C_ERROR;
    }

    /* if channel_bmp is 0, it means closing all channels, 
     * channel settings on intermediate muxs should also be closed.
     */
    if ((channel_bmp == 0) && ((*i2c_mux_info_p)[index].common_info.num_of_intermediate_mux > 0))
    {
        data = 0;
        memset(&channel_bmp_on_mux, 0, sizeof(channel_bmp_on_mux));
        /* need to close all channels on intermediate mux in reverse order
         */
        for (int_mux_idx=(I8_T)((*i2c_mux_info_p)[index].common_info.num_of_intermediate_mux)-1;
             int_mux_idx>=0; int_mux_idx--)
        {
            if (I2C_MuxTranslateChannelBMPToSetVal((*i2c_mux_info_p)[index].intermediate_mux_channel_bmp_xslt_type[int_mux_idx], &channel_bmp_on_mux, &data)==FALSE)
            {
                printf("%s(%d):Failed to translate channel bmp to set value.(mux_idx=%lu, int_mux_index=%hu, channel_bmp_on_mux=0x%08lX, bmp_to_val_xlat_type=%d\r\n",
                    __FUNCTION__, __LINE__, index, int_mux_idx, channel_bmp, (int)((*i2c_mux_info_p)[index].intermediate_mux_channel_bmp_xslt_type[int_mux_idx]) );
                return I2C_ERROR;
            }

            rc=I2C_TwsiDataWriteWithBusIdx((*i2c_mux_info_p)[index].common_info.i2c_bus_idx,
                (*i2c_mux_info_p)[index].dev_addr_of_intermediate_mux[int_mux_idx],
                I2C_7BIT_ACCESS_MODE,
                (*i2c_mux_info_p)[index].intermediate_mux_has_channel_reg[int_mux_idx],
                (*i2c_mux_info_p)[index].intermediate_mux_channel_reg_addr[int_mux_idx], FALSE, &data, 1);
            if (rc == FALSE)
            {
                printf("%s(%d): Failed to close channel on intermediate mux.(mux_idx=%lu, int_mux_dev_addr=0x%x)\r\n",
                    __FUNCTION__, __LINE__, index, (*i2c_mux_info_p)[index].dev_addr_of_intermediate_mux[int_mux_idx]);
                return I2C_ERROR;
            }
        }
    }

    return I2C_SUCCESS;
#elif (SYS_CPNT_SELECT_CHANNEL_MODE==SYS_CPNT_SELECT_CHANNEL_MODE__EPLD)
    UI8_T i2c_mux_addrs[SYS_HWCFG_NUM_OF_I2C_MUX] = SYS_HWCFG_I2C_MUX_ADDR_ARRAY;
    unsigned char channel_map=(unsigned char)channel_bmp;

    if(FALSE==PHYSICAL_ADDR_ACCESS_Write(i2c_mux_addrs[index], 1, 1, &channel_map))
    {
        SYSFUN_Debug_Printf("%s: Write CPLD failed", __FUNCTION__);
        return I2C_ERROR;
	}
	return I2C_SUCCESS;
#elif (SYS_CPNT_SELECT_CHANNEL_MODE==SYS_CPNT_SELECT_CHANNEL_MODE__EPLD_VIA_I2C)
    UI8_T i2c_mux_addrs[SYS_HWCFG_NUM_OF_I2C_MUX] = SYS_HWCFG_I2C_MUX_ADDR_ARRAY;
    UI8_T data;

    if(index>=(sizeof(i2c_mux_addrs)/sizeof(i2c_mux_addrs[0])))
    {
        printf("%s(%d): Invalid index %lu\r\n", __FUNCTION__, __LINE__, index);
        return I2C_ERROR;
    }

    return (I2C_TwsiDataWrite(SYS_HWCFG_I2C_SLAVE_CPLD, I2C_7BIT_ACCESS_MODE,
        TRUE, i2c_mux_addrs[index], 0, &data, 1)==TRUE)?I2C_SUCCESS:I2C_ERROR;
#else
    printf("%s(%d): Unknown select channel mode\r\n", __FUNCTION__, __LINE__);
    return I2C_ERROR;
#endif
}

/* FUNCTION NAME: I2C_Transaction
 * PURPOSE  :  Perform the given I2C transaction, only MASTER_XMIT and MASTER_RCV
 *             are implemented.
 * INPUT    :   act        -- the type of transaction
 *                  i2c_addr   -- the I2C address of the slave device
 *                  data_addr  -- is the address of the data on the slave device
 *                  len        -- the length of data to send or receive
 *                  buffer     -- the address of the data buffer            
 * OUTPUT   : 
 * RETURN   : I2C_SUCCESS/I2C_ERROR
 * NOTES     : 
 */
I2C_Status I2C_Transaction(I2C_TRANSACTION_MODE act, 
                           unsigned char i2c_addr, 
                           unsigned char data_addr, 
                           int len,
                           char * buffer)
{
    return I2C_TransactionWithBusIdx(I2C_DEFAULT_BUS_IDX, act, i2c_addr, data_addr, len, buffer);
}

/* FUNCTION NAME: I2C_TransactionWithBusIdx
 * PURPOSE  :  Perform the given I2C transaction, only MASTER_XMIT and MASTER_RCV
 *             are implemented.
 * INPUT    :  i2c_bus_idx-- i2c bus index
 *             act        -- the type of transaction
 *             i2c_addr   -- the I2C address of the slave device
 *             data_addr  -- is the address of the data on the slave device
 *             len        -- the length of data to send or receive
 *             buffer     -- the address of the data buffer            
 * OUTPUT   : 
 * RETURN   : I2C_SUCCESS/I2C_ERROR
 * NOTES     : 
 */
I2C_Status I2C_TransactionWithBusIdx(UI8_T  i2c_bus_idx,
                                     I2C_TRANSACTION_MODE act,
                                     unsigned char i2c_addr, 
                                     unsigned char data_addr, 
                                     int len,
                                     char * buffer)
{
    char i2c_dev_filename[I2C_DEV_FILENAME_MAX_LEN+1];
    int ret = 0;
    int i2cfd=0;

    snprintf(i2c_dev_filename, I2C_DEV_FILENAME_MAX_LEN+1, I2C_DEV_FILENAME_TEMPLATE, i2c_bus_idx );
    i2c_dev_filename[I2C_DEV_FILENAME_MAX_LEN]=0;

    /* open device */
    if ((i2cfd = open(i2c_dev_filename, O_RDWR)) < 0)
    {
        dprintf("failed to open %s\r\n", I2C_ADAPTOR_NAME);
        dprintf("errno = %d(%s)\r\n", errno, strerror(errno));
        return I2C_ERROR;
    }
    

    if((ret = ioctl( i2cfd, I2C_SLAVE_IOCTL_CMD, i2c_addr)) != 0)
    {
        dprintf("failed to ioctl(I2C_SLAVE=0x%x)(errno=%d)\r\n", i2c_addr, errno);
        close(i2cfd); 
        return I2C_ERROR;
    }

       /*read or write the I2C devices !*/
    if(act == I2C_MASTER_RCV)
    {
        if((ret = I2C_Read(i2cfd, data_addr,(unsigned char*)buffer, len)) != 0 )
        {
            dprintf("failed to I2C_Read(data_addr=0x%x)(errno=%d)\r\n", data_addr, errno);
            close(i2cfd); 
            return I2C_ERROR;
        }
    }
    else
    {
        if((ret = I2C_Write(i2cfd, data_addr,(unsigned char*)buffer, len)) != 0 )
        {
            dprintf("failed to I2C_Write(i2c_addr=0x%x,data_addr=0x%x)(errno=%d)\r\n", i2c_addr, data_addr, errno);
            close(i2cfd); 
            return I2C_ERROR;
        }
    }
    close(i2cfd);

    return I2C_SUCCESS;
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME - I2C_TwsiDataRead
 * -----------------------------------------------------------------------------
 * FUNCTION: This function is used to read data from TWSI (I2C)
 * INPUT   : dev_slv_id     - slave addr
 *           type           - address type :7bit/10bit
 *           validOffset    - for EEPROM, validOffset must be set to 1
 *           offset         - if read from the beginning, set to zero.
 *           moreThen256    - some device can read more the 256 address.
 *           data_len       - length of data
 * OUTPUT  : data
 * RETURN  : True: Successfully, FALSE: Failed
 * -----------------------------------------------------------------------------
 */
BOOL_T I2C_TwsiDataRead(UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, UI8_T data_len, UI8_T* data)
{
    return I2C_TwsiDataReadWithBusIdx(I2C_DEFAULT_BUS_IDX, dev_slv_id, type, validOffset, offset, moreThen256, data_len, data);
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME - I2C_TwsiDataReadWithBusIdx
 * -----------------------------------------------------------------------------
 * FUNCTION: This function is used to read data from TWSI (I2C)
 * INPUT   : i2c_bus_idx    - i2c bus index
 *           dev_slv_id     - slave addr
 *           type           - address type :7bit/10bit
 *           validOffset    - for EEPROM, validOffset must be set to 1
 *           offset         - if read from the beginning, set to zero.
 *           moreThen256    - some device can read more the 256 address.
 *           data_len       - length of data
 * OUTPUT  : data
 * RETURN  : True: Successfully, FALSE: Failed
 * -----------------------------------------------------------------------------
 */
BOOL_T I2C_TwsiDataReadWithBusIdx(UI8_T i2c_bus_idx, UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, UI8_T data_len, UI8_T* data)
{
    char i2c_dev_filename[I2C_DEV_FILENAME_MAX_LEN+1];
    int i2cfddev,ret;

    snprintf(i2c_dev_filename, I2C_DEV_FILENAME_MAX_LEN+1, I2C_DEV_FILENAME_TEMPLATE, i2c_bus_idx);
    i2c_dev_filename[I2C_DEV_FILENAME_MAX_LEN]=0;

    /*open device*/
    if ((i2cfddev = open(i2c_dev_filename, O_RDWR)) < 0)
    {
        dprintf("errno=%d\r\n", errno);
        return FALSE;
    }

     /* set i2c 7 bit/10 bit mode */
     if((ret = ioctl( i2cfddev, I2C_TENBIT, (type==I2C_10BIT_ACCESS_MODE)?1:0)) != 0)
    {
        dprintf("errno=%d\r\n", errno);
        close(i2cfddev);
        return FALSE;
    }

    if((ret = ioctl( i2cfddev, I2C_SLAVE_IOCTL_CMD, dev_slv_id)) != 0)
    {
        dprintf("errno=%d\r\n", errno);
        close(i2cfddev);
        return FALSE;
    }

    if(validOffset==0)
    {
        if(I2C_ReadWithoutDataAdr(i2cfddev, data, data_len)!=I2C_SUCCESS)
        {
            dprintf("errno=%d\r\n", errno);
            close(i2cfddev);
            return FALSE;
        }
    }
    else
    {
        if(I2C_Read(i2cfddev, offset, data, data_len)!=I2C_SUCCESS)
        {
            dprintf("errno=%d dev_slv_id=0x%x offset=0x%x\r\n", errno, dev_slv_id, offset);
            close(i2cfddev);
            return FALSE;
        }
    }

    close(i2cfddev);
    return TRUE;
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME - I2C_TwsiDataWrite
 * -----------------------------------------------------------------------------
 * FUNCTION: This function is used to write data to TWSI (I2C)
 * INPUT   : dev_slv_id     - slave addr
 *           type           - address type :7bit/10bit
 *           validOffset    - for EEPROM, validOffset must be set to 1
 *           offset         - if read from the beginning, set to zero.
 *           moreThen256    - some device can read more the 256 address.
 *           data           - data to be written
 *           data_len       - length of data
 * RETURN  : True: Successfully, FALSE: Failed
 * -----------------------------------------------------------------------------
 */
BOOL_T I2C_TwsiDataWrite(UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, const UI8_T* data, UI8_T data_len)
{
    return I2C_TwsiDataWriteWithBusIdx(I2C_DEFAULT_BUS_IDX, dev_slv_id, type, validOffset, offset, moreThen256, data, data_len);
}

/* -----------------------------------------------------------------------------
 * ROUTINE NAME - I2C_TwsiDataWriteWithBusIdx
 * -----------------------------------------------------------------------------
 * FUNCTION: This function is used to write data to TWSI (I2C)
 * INPUT   : i2c_bus_idx    - i2c bus index
 *           dev_slv_id     - slave addr
 *           type           - address type :7bit/10bit
 *           validOffset    - for EEPROM, validOffset must be set to 1
 *           offset         - if read from the beginning, set to zero.
 *           moreThen256    - some device can read more the 256 address.
 *           data           - data to be written
 *           data_len       - length of data
 * RETURN  : True: Successfully, FALSE: Failed
 * -----------------------------------------------------------------------------
 */
BOOL_T I2C_TwsiDataWriteWithBusIdx(UI8_T i2c_bus_idx, UI8_T dev_slv_id, UI8_T type, UI8_T validOffset, UI32_T offset, UI8_T moreThen256, const UI8_T* data, UI8_T data_len)
{
    char i2c_dev_filename[I2C_DEV_FILENAME_MAX_LEN+1];
    int i2cfddev,ret;

    snprintf(i2c_dev_filename, I2C_DEV_FILENAME_MAX_LEN+1, I2C_DEV_FILENAME_TEMPLATE, i2c_bus_idx);
    i2c_dev_filename[I2C_DEV_FILENAME_MAX_LEN]=0;

    /*open device*/
    if ((i2cfddev = open(i2c_dev_filename, O_RDWR)) < 0)
    {
        dprintf("errno=%d\r\n", errno);
        return FALSE;
    }

     /* set i2c 7 bit/10 bit mode */
     if((ret = ioctl( i2cfddev, I2C_TENBIT, (type==I2C_10BIT_ACCESS_MODE)?1:0)) != 0)
    {
        dprintf("errno=%d\r\n", errno);
        close(i2cfddev);
        return FALSE;
    }

    if((ret = ioctl( i2cfddev, I2C_SLAVE_IOCTL_CMD, dev_slv_id)) != 0)
    {
        dprintf("errno=%d\r\n", errno);
        close(i2cfddev);
        return FALSE;
    }

    if((ret = I2C_Write(i2cfddev, (validOffset==1)?offset:0, (unsigned char*)data, data_len)) != I2C_SUCCESS )
    {
        dprintf("errno=%d dev_slv_id=0x%x offset=0x%x\r\n", errno, dev_slv_id, offset);
        close(i2cfddev);
        return FALSE;
    }

    close(i2cfddev);
    return TRUE;
}

#endif


