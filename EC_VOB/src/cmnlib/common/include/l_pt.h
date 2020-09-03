#ifndef _L_PT_H
#define _L_PT_H

/* L_PT_SHMEM_VALIDATE_BUF_ALLOC_STATUS
 * The compile option is used for debugging buffer corruption which might
 * be caused by double free or other reasons which are related to allocation
 * or free status of a buffer. The feature is only available on L_PT_ShMem
 * series functions.
 */
#undef L_PT_SHMEM_VALIDATE_BUF_ALLOC_STATUS

#ifdef L_PT_SHMEM_VALIDATE_BUF_ALLOC_STATUS
/* L_PT_SHMEM_VALIDATE_BUF_MAX_BUF_NUMBER
 * Define the maximum number of buffer that is supported to
 * validate the buffer allocation/free status. It is better to
 * define the constant value as the multiple of 8.
 */
#define L_PT_SHMEM_VALIDATE_BUF_MAX_BUF_NUMBER 3200
#endif


/* DATY TYPE DECLARACTION */
   typedef struct
   {
      /* user provide static buffer */
      char     *buffer;

      /* buffer length, the length must > 4 */
      UI32_T   buffer_len;

      /* The buffer will be divided into partitions,
       * Note: partition_len < buffer_len
       */
      UI32_T   partition_len;

      /* unused partition number, read only */
      UI32_T   free_no;

      /* The free list pointer, this item is processed by L_rg.c,
       * the called program can't access it directly
       */
      void     *free;
   }  L_PT_Descriptor_T;



   typedef struct
   {
      /* user provide static buffer */
      char     *buffer;

      /* buffer length, the length must > 4 */
      UI32_T   buffer_len;

      /* The buffer will be divided into partitions,
       * Note: partition_len < buffer_len
       */
      UI32_T   partition_len;

      /* unused partition number, read only */
      UI32_T   free_no;

      /* The free list pointer, this item is processed by L_rg.c,
       * the called program can't access it directly
       */
      void     *free;
      /* The tail pointer, point to tail of list.
       * Internal use only, can't access it directly
       */
      void     *tail;
   }  L_PT_ExtDescriptor_T;


    typedef struct
    {
        /* user provide static buffer relative address of
         * the working buffer with desc_p as base addr.
         */
        I32_T    buffer_offset;

        /* buffer length, the length must > 4 */
        UI32_T   buffer_len;

        /* The buffer will be divided into partitions,
         * Note: partition_len < buffer_len
         */
        UI32_T   partition_len;

#ifdef L_PT_SHMEM_VALIDATE_BUF_ALLOC_STATUS
        /* Bitmap array that keeps the buffer allocation
         * status. The buffer is allocated when the corresponding
         * bit is set as 1.
         */
        UI8_T    alloc_state[(L_PT_SHMEM_VALIDATE_BUF_MAX_BUF_NUMBER+7)/8];
#endif

        /* unused partition number, read only
         */
        UI32_T   free_no;

        /* The free list relative address relative address of
         * the working buffer with desc_p as base addr.
         */
        I32_T    free_offset;

    }  L_PT_ShMem_Descriptor_T;


/* EXPORTED FUNCTIONS DECLARATION */


/*------------------------------------------------------------------------
 * ROUTINE NAME - L_PT_Create
 *------------------------------------------------------------------------
 * FUNCTION: Create a partition
 * INPUT:   descriptor->buffer
 *          descriptor->buffer_len (must > partition_len)
 *          descriptor->partition_len (must a multiple of 4)
 *
 * OUTPUT  : None
 * SIED EFFECT:   descriptor->free
 * RETURN  : TRUE/FALSE
 *------------------------------------------------------------------------*/
BOOL_T   L_PT_Create ( L_PT_Descriptor_T *descriptor);




/*------------------------------------------------------------------------
 * ROUTINE NAME - L_PT_Allocate
 *------------------------------------------------------------------------
 * FUNCTION: Allocate a partition
 * INPUT:   descriptor
 *
 * OUTPUT  : None
 * SIED EFFECT:   descriptor->free
 * RETURN  : allocated buffer pointer
 * NOTE: The input descriptor must have been initiated (call L_PT_Create).
 *------------------------------------------------------------------------*/
void     *L_PT_Allocate (L_PT_Descriptor_T *descriptor);



/*------------------------------------------------------------------------
 * ROUTINE NAME - L_PT_Free
 *------------------------------------------------------------------------
 * FUNCTION: Free an previous allocated partition.
 * INPUT:   descriptor, ptr
 *
 * OUTPUT: None
 * SIED EFFECT:   descriptor->free
 * RETURN: None
 * NOTE: The input descriptor must have been initiated (call L_PT_Create).
 *------------------------------------------------------------------------*/
BOOL_T L_PT_Free (L_PT_Descriptor_T *descriptor, void *ptr);






/*------------------------------------------------------------------------
 * ROUTINE NAME - L_PT_ExtCreate
 *------------------------------------------------------------------------
 * FUNCTION: Create a partition
 * INPUT:   descriptor->buffer
 *          descriptor->buffer_len (must > partition_len)
 *          descriptor->partition_len (must a multiple of 4)
 *
 * OUTPUT  : None
 * SIED EFFECT:   descriptor->free
 * RETURN  : TRUE/FALSE
 *------------------------------------------------------------------------*/
BOOL_T   L_PT_ExtCreate ( L_PT_ExtDescriptor_T *descriptor);




/*------------------------------------------------------------------------
 * ROUTINE NAME - L_PT_ExtAllocate
 *------------------------------------------------------------------------
 * FUNCTION: Allocate a partition
 * INPUT:   descriptor
 *
 * OUTPUT  : None
 * SIED EFFECT:   descriptor->free
 * RETURN  : allocated buffer pointer
 * NOTE: The input descriptor must have been initiated (call L_PT_ExtCreate).
 *------------------------------------------------------------------------*/
void     *L_PT_ExtAllocate (L_PT_ExtDescriptor_T *descriptor);

/*------------------------------------------------------------------------
 * ROUTINE NAME - L_PT_ExtFree
 *------------------------------------------------------------------------
 * FUNCTION: Free an previous allocated partition.
 * INPUT:   descriptor, ptr
 *          is_free_to_head
 * OUTPUT: None
 * SIED EFFECT:   descriptor->free
 * RETURN: None
 * NOTE: The input descriptor must have been initiated (call L_PT_ExtCreate).
 *------------------------------------------------------------------------*/
BOOL_T  L_PT_ExtFree (L_PT_ExtDescriptor_T *desc_p, void *ptr, BOOL_T is_free_to_head);


/*-------------------------
 * L_PT for shared memory
 *-------------------------
 */

/*------------------------------------------------------------------------
 * ROUTINE NAME - L_PT_ShMem_Create
 *------------------------------------------------------------------------
 * FUNCTION: Create a partition
 * INPUT:    descriptor->buffer_offset -- relative address of the working
 *                                        buffer with desc_p as base addr.
 *           descriptor->buffer_len (must > partition_len)
 *           descriptor->partition_len (must a multiple of 4)
 *
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : descriptor and buffer must be in shared memory
 *------------------------------------------------------------------------*/
BOOL_T L_PT_ShMem_Create(L_PT_ShMem_Descriptor_T *desc_p);

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
BOOL_T L_PT_ShMem_GetAllocStatus(L_PT_ShMem_Descriptor_T *desc_p, void *ptr, BOOL_T *alloc_status_p);
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
void *L_PT_ShMem_Allocate(L_PT_ShMem_Descriptor_T *desc_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - L_PT_ShMem_Free
 *------------------------------------------------------------------------
 * FUNCTION: Free an allocated partition.
 * INPUT:    desc_p  --  descriptor for L_PT on a shared memory
 *           ptr     --  allocated partition
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 *------------------------------------------------------------------------*/
BOOL_T L_PT_ShMem_Free(L_PT_ShMem_Descriptor_T *desc_p, void *ptr);


#endif
