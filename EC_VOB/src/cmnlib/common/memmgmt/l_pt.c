// #include <bspfuncs.h>
#ifdef __KERNEL__
#include <linux/string.h>
#include <linux/kernel.h>
#else
#include <execinfo.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include  "sysfun.h"
#endif

#include  "sys_type.h"
#include  "l_cvrt.h"
#include  "l_pt.h"


/* NAMING CONSTANT DEFINITION
 */

/* If you enable TEST_BUFFER_CORRUPT, please disable RULE_CTRL_INSTANCE_DEBUG_ON *
 * which is defined in rule_type.h                                               */
#undef TEST_BUFFER_CORRUPT

#define MAGIC_PATTERN_1B 0x55
#define MAGIC_PATTERN_4B (UI32_T)(MAGIC_PATTERN_1B | \
                          MAGIC_PATTERN_1B<<8      | \
                          MAGIC_PATTERN_1B<<16     | \
                          MAGIC_PATTERN_1B<<24)

#define L_PT_IS_BIT_ON(bmp, bno)                (bmp[bno/8] & (1 << (7 - bno%8)))
#define L_PT_SET_BIT_ON(bmp, bno)               {bmp[bno/8] |= (1 << (7 - bno%8));}
#define L_PT_SET_BIT_OFF(bmp, bno)              {bmp[bno/8] &= ~(1 << (7 - bno%8));}


#ifdef __KERNEL__
#define printf    printk
#endif

/* MACRO FUNCTION DECLARATIONS
 */

/* EXPORTED FUNCTIONS BODY */
BOOL_T L_PT_Create (L_PT_Descriptor_T *desc_p)
{
	int     i, buf_nbr;
    uintptr_t  *buf_p;
    uintptr_t  next_position;

    /* BODY */
    desc_p->free = 0;

    /* adjust partition length, must be a multiple of 4 */
    desc_p->partition_len = (desc_p->partition_len+3)/4 * 4;

    buf_nbr = desc_p->buffer_len / desc_p->partition_len;

    desc_p->free = buf_p = (uintptr_t *)desc_p->buffer;

    for (i = 0; i < (buf_nbr-1); i++)
	{
        next_position = ((uintptr_t)buf_p ) + desc_p->partition_len;
        *buf_p = next_position;
        buf_p = (uintptr_t *) next_position;
	}
    *buf_p = 0;

    desc_p->free_no = buf_nbr;

    return TRUE;
}/* STA_IniL_PT_Create */

void  *L_PT_Allocate (L_PT_Descriptor_T *desc_p)
{
    uintptr_t   *dp;

	if ((dp = desc_p->free) != 0)
    {
        desc_p->free = (void *) *dp;
        desc_p->free_no--;
    }

	return dp;
} /* L_PT_Allocate */




BOOL_T  L_PT_Free (L_PT_Descriptor_T *desc_p, void *ptr)
{
    uintptr_t *dp = ptr;

    if ( ptr == 0 )
        return FALSE;

    /* check range */

    if ( ((uintptr_t)ptr < (uintptr_t) desc_p->buffer) ||
        ((uintptr_t)ptr >= (uintptr_t) (desc_p->buffer+desc_p->buffer_len) ) )
        return FALSE;

    if ( ((uintptr_t)ptr - (uintptr_t)desc_p->buffer) % desc_p->partition_len )
        return FALSE;

    *dp = (uintptr_t)desc_p->free;
    desc_p->free = dp;
    desc_p->free_no++;

    return TRUE;

}/* End of L_PT_Free() */





BOOL_T L_PT_ExtCreate (L_PT_ExtDescriptor_T *desc_p)
{
	int     i, buf_nbr;
    uintptr_t  *buf_p;
    uintptr_t  next_position;

    /* BODY */
    desc_p->free = 0;

    /* adjust partition length, must be a multiple of 4 */
    desc_p->partition_len = (desc_p->partition_len+3)/4 * 4;

    buf_nbr = desc_p->buffer_len / desc_p->partition_len;

    desc_p->free = buf_p = (uintptr_t *)desc_p->buffer;

    for (i = 0; i < (buf_nbr-1); i++)
	{
        next_position = ((uintptr_t)buf_p ) + desc_p->partition_len;
        *buf_p = next_position;
        buf_p = (uintptr_t *) next_position;
	}
    desc_p->tail = buf_p;

    *buf_p = 0;

    desc_p->free_no = buf_nbr;

    return TRUE;
}/* STA_IniL_PT_ExtCreate */

void  *L_PT_ExtAllocate (L_PT_ExtDescriptor_T *desc_p)
{
    uintptr_t   *dp;

	if ((dp = desc_p->free) != 0)
    {
        if ((desc_p->free = (void *) *dp) == 0 )
            desc_p->tail = 0;
        desc_p->free_no--;
    }

	return dp;
} /* L_PT_ExtAllocate */




BOOL_T  L_PT_ExtFree (L_PT_ExtDescriptor_T *desc_p, void *ptr, BOOL_T is_free_to_head)
{
    uintptr_t *dp = ptr;

    if ( ptr == 0 )
        return FALSE;

    /* check range */

    if ( ((uintptr_t)ptr < (uintptr_t) desc_p->buffer) ||
        ((uintptr_t)ptr >= (uintptr_t) (desc_p->buffer+desc_p->buffer_len) ) )
        return FALSE;

    if ( ((uintptr_t)ptr - (uintptr_t)desc_p->buffer) % desc_p->partition_len )
        return FALSE;


    if (is_free_to_head)
    {
        *dp = (uintptr_t)desc_p->free;
        desc_p->free = dp;
        if ( desc_p->tail == 0 )
            desc_p->tail = dp;
    }
    else
    {
        if ( desc_p->tail == 0 )
        {
            desc_p->tail = desc_p->free = dp;
            *dp=0;
        }
        else
        {
            *(uintptr_t*)desc_p->tail = (uintptr_t)dp;
            desc_p->tail = dp;
            *dp = 0;
        }
    }
    desc_p->free_no++;

    return TRUE;

}/* End of L_PT_ExtFree() */


/*-------------------------
 * L_PT for shared memory
 *-------------------------
 */

/*------------------------------------------------------------------------
 * ROUTINE NAME - L_PT_ShMem_Create
 *------------------------------------------------------------------------
 * FUNCTION: Create a partition
 * INPUT:    descriptor->buffer_offset -- the offset of the buffer
 *           descriptor->buffer_len (must > partition_len)
 *           descriptor->partition_len (must a multiple of 4)
 *
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 *------------------------------------------------------------------------*/
BOOL_T L_PT_ShMem_Create (L_PT_ShMem_Descriptor_T *desc_p)
{
    int     i, buf_nbr;
    ptrdiff_t  *buf_p;
    UI32_T     *u32_p;
    ptrdiff_t  next_position;

    /* BODY */
    if (desc_p->buffer_offset == 0)
        return FALSE;

    /* adjust partition length, must be a multiple of 4 */
    desc_p->partition_len = (desc_p->partition_len+3)/4 * 4;

    buf_nbr = desc_p->buffer_len / desc_p->partition_len;

    desc_p->free_offset = desc_p->buffer_offset;
    buf_p = L_CVRT_GET_PTR(desc_p, desc_p->free_offset);

    for (i = 0; i < (buf_nbr-1); i++)
    {
        next_position = ((ptrdiff_t)buf_p ) + desc_p->partition_len;
#ifdef TEST_BUFFER_CORRUPT
        memset(buf_p, MAGIC_PATTERN_1B, desc_p->partition_len);
#endif
        u32_p=(UI32_T*)buf_p;
        *u32_p = (UI32_T)(L_CVRT_GET_OFFSET(desc_p, next_position));
        buf_p = (ptrdiff_t *) next_position;
    }
#ifdef TEST_BUFFER_CORRUPT
    memset(buf_p, MAGIC_PATTERN_1B, desc_p->partition_len);
#endif
    u32_p=(UI32_T*)buf_p;
    *u32_p = 0;

    desc_p->free_no = buf_nbr;

#ifdef L_PT_SHMEM_VALIDATE_BUF_ALLOC_STATUS
    memset(&desc_p->alloc_state, 0, sizeof(desc_p->alloc_state));
#endif

    return TRUE;
}/* L_PT_ShMem_Create */

#ifdef L_PT_SHMEM_VALIDATE_BUF_ALLOC_STATUS
/*------------------------------------------------------------------------
 * ROUTINE NAME - L_PT_ShMem_GetAllocStatus
 *------------------------------------------------------------------------
 * FUNCTION: Get the allocation status of the specified buffer
 * INPUT:    desc_p         --  descriptor for L_PT on a shared memory
 *           ptr            --  the buffer to get the allocation status
 * OUTPUT  : alloc_status_p --  TRUE : The specified buffer is in allocated state
 *                              FALSE: The spcified buffer is in free state
 * RETURN  : TRUE:  The allocation status is outputted successfully.
 *           FALSE: Cannot get allocation status because the given desc_p or
 *                  ptr is invalid.
 * NOTE: The input descriptor must have been initiated (call L_PT_ShMem_Create).
 *------------------------------------------------------------------------*/
BOOL_T L_PT_ShMem_GetAllocStatus(L_PT_ShMem_Descriptor_T *desc_p, void *ptr, BOOL_T *alloc_status_p)
{
    int buf_idx;
    ptrdiff_t *dp = ptr;

    /* input args check
     */
    if (desc_p==NULL || ptr==NULL || alloc_status_p==NULL)
    {
        printf("%s: Invalid input argument.\r\n", __FUNCTION__);
        return FALSE;
    }

    /* input ptr range validation
     */
    if ( ( (ptrdiff_t)ptr < (ptrdiff_t)L_CVRT_GET_PTR(desc_p, desc_p->buffer_offset) ) ||
         ( (ptrdiff_t)ptr >= ((ptrdiff_t)L_CVRT_GET_PTR(desc_p, desc_p->buffer_offset) + desc_p->buffer_len) ) )
    {
        printf("%s: ptr is not in expected range.\r\n", __FUNCTION__);
        return FALSE;
    }

    /* input ptr alignment validation
     */
    if ( (L_CVRT_GET_OFFSET(desc_p, dp)-desc_p->buffer_offset)%desc_p->partition_len != 0)
    {
        printf("%s: ptr is not aligned to the buffer boundary.\r\n", __FUNCTION__);
        return FALSE;
    }

    buf_idx = (L_CVRT_GET_OFFSET(desc_p, dp)-desc_p->buffer_offset)/desc_p->partition_len;

    *alloc_status_p =  (L_PT_IS_BIT_ON(desc_p->alloc_state, buf_idx)!=0) ? TRUE : FALSE;
    return TRUE;
}
#endif /*#ifdef L_PT_SHMEM_VALIDATE_BUF_ALLOC_STATUS*/

/*------------------------------------------------------------------------
 * ROUTINE NAME - L_PT_ShMem_Allocate
 *------------------------------------------------------------------------
 * FUNCTION: Allocate a buffer
 * INPUT:    desc_p    --  descriptor for L_PT on a shared memory
 * OUTPUT  : None
 * RETURN  : Non-null -- allocated buffer on a shared memory
 *           NULL     -- fail
 *------------------------------------------------------------------------*/

void  *L_PT_ShMem_Allocate (L_PT_ShMem_Descriptor_T *desc_p)
{
    ptrdiff_t   *dp = NULL;
#ifdef TEST_BUFFER_CORRUPT
    UI32_T      i, *data_p;
#endif
    I32_T      next_free_offset;

    if (desc_p->free_offset != 0)
    {
        dp = L_CVRT_GET_PTR(desc_p, desc_p->free_offset);
        next_free_offset = (I32_T)*dp;

#ifdef L_PT_SHMEM_VALIDATE_BUF_ALLOC_STATUS
        {
            BOOL_T alloc_status;

            if (L_PT_ShMem_GetAllocStatus(desc_p, (void*)dp, &alloc_status)==TRUE)
            {
                if (alloc_status==TRUE)
                    printf("%s: Error. The allocated buffer %p is not in free state!\r\n", __FUNCTION__, (void*)dp);
            }
            else
            {
                printf("%s: L_PT_ShMem_GetAllocStatus error.\r\n", __FUNCTION__);
            }
        }
#endif

        if (next_free_offset != 0) /*next_free_offset==0 => last free*/
        {
            if ( next_free_offset >= (desc_p->buffer_offset + desc_p->buffer_len) || 
                 next_free_offset < desc_p->buffer_offset || 
               ((next_free_offset - desc_p->buffer_offset) % desc_p->partition_len) )
            {
#ifdef __KERNEL__
                printk("<0> L_PT_ShMem_Allocate Fail! %d\n", __LINE__);
#else
                printf("L_PT_ShMem_Allocate Fail! pid: %d, desc_p: 0x%p, alloc_buf: 0x%p, partition_len: %u, buffer_offset: 0x%x, buffer_len: 0x%x, next_free_offset: 0x%x, free_no: %u \r\n",
                    getpid(), desc_p, L_CVRT_GET_PTR(desc_p, desc_p->free_offset),
                    desc_p->partition_len, desc_p->buffer_offset, desc_p->buffer_len, next_free_offset, desc_p->free_no);

                DBG_DumpHex("Alloc Buf Dump:", desc_p->partition_len, (char *)dp);

#ifdef L_PT_SHMEM_VALIDATE_BUF_ALLOC_STATUS
                DBG_DumpHex("Alloc State Bitmap:", sizeof(desc_p->alloc_state), (char *)desc_p->alloc_state);
#endif /*#ifdef L_PT_SHMEM_VALIDATE_BUF_ALLOC_STATUS*/

#endif /* #ifdef __KERNEL__*/

                return NULL;
            }
        }

        desc_p->free_offset = next_free_offset;
        desc_p->free_no--;

#ifdef TEST_BUFFER_CORRUPT
        for(data_p=(UI32_T*)dp + 1, i=0; i<((desc_p->partition_len/sizeof(UI32_T))-1); i++, data_p++)
        {
            if (*data_p!=(UI32_T)MAGIC_PATTERN_4B)
            {
                printf("%s: L_PT_ShMem buffer corrupt! pid: %d, desc_p: 0x%p, dp: 0x%p, partition_len: %u, buffer_offset: %x, buffer_len: %u, free_offset: %x, free_no: %u\r\n",
                    __FUNCTION__, getpid(), desc_p, L_CVRT_GET_PTR(desc_p, desc_p->free_offset),
                    desc_p->partition_len, desc_p->buffer_offset, desc_p->buffer_len, desc_p->free_offset, desc_p->free_no);
            }
        }
#endif /*#ifdef TEST_BUFFER_CORRUPT*/

#ifdef L_PT_SHMEM_VALIDATE_BUF_ALLOC_STATUS
        {
            int buf_idx;

            buf_idx = (L_CVRT_GET_OFFSET(desc_p, dp)-desc_p->buffer_offset)/desc_p->partition_len;
            if (buf_idx < L_PT_SHMEM_VALIDATE_BUF_MAX_BUF_NUMBER)
            {
                L_PT_SET_BIT_ON(desc_p->alloc_state, buf_idx);
            }
            else
            {
                printf("%s: Warning! The index of allocated buffer(%d) is larger than or equal to L_PT_SHMEM_VALIDATE_BUF_MAX_BUF_NUMBER(%d)\r\n",
                    __FUNCTION__, buf_idx, L_PT_SHMEM_VALIDATE_BUF_MAX_BUF_NUMBER);
            }
        }
#endif /*#ifdef L_PT_SHMEM_VALIDATE_BUF_ALLOC_STATUS*/
    }

    return dp;
} /* L_PT_ShMem_Allocate */

/*------------------------------------------------------------------------
 * ROUTINE NAME - L_PT_ShMem_Free
 *------------------------------------------------------------------------
 * FUNCTION: Free an allocated partition.
 * INPUT:    desc_p  --  descriptor for L_PT on a shared memory
 *           ptr     --  allocated partition
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 *------------------------------------------------------------------------*/
BOOL_T L_PT_ShMem_Free(L_PT_ShMem_Descriptor_T *desc_p, void *ptr)
{
    void *buffer = L_CVRT_GET_PTR(desc_p, desc_p->buffer_offset);
    ptrdiff_t *dp = ptr;
    UI32_T *u32_p;

    if ( ptr == NULL )
        return FALSE;

#ifdef L_PT_SHMEM_VALIDATE_BUF_ALLOC_STATUS
    {
        BOOL_T buf_alloc_status;
        BOOL_T rc;

        rc=L_PT_ShMem_GetAllocStatus(desc_p, (void*)dp, &buf_alloc_status);

        if (rc == FALSE)
        {
            printf("%s: L_PT_ShMem_GetAllocStatus error.\r\n", __FUNCTION__);
        }

        if (rc == TRUE && buf_alloc_status==TRUE)
        {
#ifdef __KERNEL__
            printk("<0> L_PT_ShMem_Free Fail! %d\n", __LINE__);
#else
            void *array[10];
            size_t size;
            char **strframe = NULL;
            int i = 0;

            printf("%s: double free!!\r\n", __FUNCTION__);

            size = backtrace(array, 10);
            strframe = (char **)backtrace_symbols(array, size);

            printf("Stack trace: \r\n");

            for(i = 0; i < size; i++)
            {
                printf("frame %d -- %s\r\n", i, strframe[i]);
            }

            if(strframe)
            {
                free(strframe);
                strframe = NULL;
            }
#endif /*#ifdef __KERNEL__*/
            return FALSE;
        }
    }
#endif /*#ifdef L_PT_SHMEM_VALIDATE_BUF_ALLOC_STATUS*/

    /* check range */

    if ( ((ptrdiff_t)ptr < (ptrdiff_t)buffer ) ||
        ((ptrdiff_t)ptr >= (ptrdiff_t)buffer + desc_p->buffer_len) )
    {
        printf("%s: ptr is not in expected range.\r\n", __FUNCTION__);
        return FALSE;
    }

    if ( ((ptrdiff_t)ptr - (ptrdiff_t)buffer) % desc_p->partition_len )
    {
        printf("%s: ptr is not aligned to the buffer boundary.\r\n", __FUNCTION__);
        return FALSE;
    }

#ifdef TEST_BUFFER_CORRUPT
    memset(dp, MAGIC_PATTERN_1B, desc_p->partition_len);
#endif

    memset(dp, 0, desc_p->partition_len);

    /* validate desc_p->free_offset
     */
    if (desc_p->free_offset != 0) /*next_free_offset==0 => last free*/
    {
        if ( desc_p->free_offset >= (desc_p->buffer_offset + desc_p->buffer_len) || 
             desc_p->free_offset < desc_p->buffer_offset || 
           ((desc_p->free_offset - desc_p->buffer_offset) % desc_p->partition_len) )
        {
            printf("%s: Invalid free_offset(0x%x)!!(buffer_offset=0x%x, buffer_len=0x%x, partition_len=0x%x\r\n",
                __FUNCTION__, desc_p->free_offset, desc_p->buffer_offset, desc_p->buffer_len, desc_p->partition_len);
        }
    }

    u32_p=(UI32_T*)dp;
    *u32_p = desc_p->free_offset;
    desc_p->free_offset = L_CVRT_GET_OFFSET(desc_p, dp);
    desc_p->free_no++;

#ifdef L_PT_SHMEM_VALIDATE_BUF_ALLOC_STATUS
    {
        int buf_idx;

        buf_idx = (L_CVRT_GET_OFFSET(desc_p, dp)-desc_p->buffer_offset)/desc_p->partition_len;
        if (buf_idx < L_PT_SHMEM_VALIDATE_BUF_MAX_BUF_NUMBER)
        {
            L_PT_SET_BIT_OFF(desc_p->alloc_state, buf_idx);
        }
        else
        {
            printf("%s: Warning! The index of freed buffer(%d) is larger than or equal to L_PT_SHMEM_VALIDATE_BUF_MAX_BUF_NUMBER(%d)\r\n",
                __FUNCTION__, buf_idx, L_PT_SHMEM_VALIDATE_BUF_MAX_BUF_NUMBER);
        }
    }
#endif /*#ifdef L_PT_SHMEM_VALIDATE_BUF_ALLOC_STATUS*/

    return TRUE;
}


#ifdef _DEBUG
#include  <stdio.h>
//#include <conio.h>
//#include <ncurses.h>
static char buffer[1024];

void L_PT_main()
{
    int     cc;
    L_PT_Descriptor_T    pt_desc;

    char    *ptr=0;
    BOOL_T  ret;
    pt_desc.buffer = buffer;
    pt_desc.buffer_len = 1024;
    pt_desc.partition_len = 32;
    L_PT_Create (&pt_desc);

    while (1)
    {
        cc = getchar();

        if ( cc == 27 ) return;
        if ( cc =='1')
            ptr = L_PT_Allocate ( &pt_desc );
        else if ( cc == '2')
        {
            ret = L_PT_Free ( &pt_desc, ptr);
        }
    }

}





void L_PT_Extmain()
{
    int     cc;
    L_PT_ExtDescriptor_T    pt_desc;

    char    *ptr=0;
    BOOL_T  ret;
    pt_desc.buffer = buffer;
    pt_desc.buffer_len = 1024;
    pt_desc.partition_len = 32;
    L_PT_ExtCreate (&pt_desc);

    while (1)
    {
        cc = getchar();

        if ( cc == 27 ) return;
        if ( cc =='1')
        {
            ptr = L_PT_ExtAllocate ( &pt_desc );
            printf ("%X\r\n", ptr);
        }
        else if ( cc == '2')
        {
            ret = L_PT_ExtFree ( &pt_desc, ptr, FALSE);
        }
    }

}

void L_PT_ShMem_main()
{
    int     cc;
    L_PT_ShMem_Descriptor_T    pt_desc;

    char    *ptr=0;
    BOOL_T  ret;
    pt_desc.buffer_offset = L_CVRT_GET_OFFSET(&pt_desc, buffer);
    pt_desc.buffer_len = 1024;
    pt_desc.partition_len = 32;
    L_PT_ShMem_Create (&pt_desc);

    while (1)
    {
        cc = getchar();

        if ( cc == 27 ) return;
        if ( cc =='1')
            ptr = L_PT_ShMem_Allocate ( &pt_desc );
        else if ( cc == '2')
        {
            ret = L_PT_ShMem_Free ( &pt_desc, ptr);
        }
        DBG_DumpHex("pt_desc", sizeof(pt_desc), &pt_desc);
    }

}

#endif
