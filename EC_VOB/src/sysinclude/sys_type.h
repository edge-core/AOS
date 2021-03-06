
#ifndef SYS_TYPE_H
#define SYS_TYPE_H

/* ------------------------------------------------------------------------
 * FILE NAME - SYS_TYPE.H
 * ------------------------------------------------------------------------
 * ABSTRACT :
 * Purpose: This package defines the naming constants for the basic data
 *          type.
 * Note: The naming constant defined in this package shall be reused
 *        by the all the BNBU L2/L3 switch projects .
 * ------------------------------------------------------------------------
 *  History
 *
 *   ryan     07/16/2001      new created
 *
 * ------------------------------------------------------------------------
 * Copyright(C)							  	ACCTON Technology Corp. , 2001
 * ------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */
#ifdef __KERNEL__
#include <linux/stddef.h>
#include <linux/types.h>
#else
#include <stddef.h>
#include <stdint.h>
#endif
#include "sys_type_packet.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* Define basic type
 * Note: 1. I64_T and UI64_T 64 bit defined is chip dependent
 *       2. UINT32 is defined for Winriver Tornado development kits.
 */

#define BOOL_T       unsigned char

#define I8_T         signed char          /* signed  8 bit integer  */
#define UI8_T        unsigned char        /* unsigned 8 bit integer  */

#define I16_T        short                /* signed  16 bit integer */
#define UI16_T       unsigned short       /* unsigned 16 bit integer */

#define I32_T	     int32_t              /* signed  32 bit integer */
#define UI32_T	     uint32_t             /* unsigned 32 bit integer */

#ifdef _DEBUG
    #define I64_T        __int64
    #define UI64_T       unsigned __int64
#else
    #define I64_T        int64_t          /* signed  64 bit integer */
    #define UI64_T       uint64_t         /* unsigned 64 bit integer */
#endif

#define MEM_SIZE_T     unsigned long

/* Define basic and common Naming Constants for the project.
 */
#ifdef TRUE
	#undef 	TRUE
#endif
#define		TRUE		1

#ifdef FALSE
	#undef 	FALSE
#endif
#define		FALSE		0

#ifdef NULL
	#undef 	NULL
#endif
/* To avoid compile error in Linux Kernel v3.8.13, need to add "(void*)"
 * before 0 in NULL definition. The reason of compile error is shown below:
 * The following line in $KERNDIR/kernel/events/core.c causes the compile
 * error.
 *     rcu_assign_pointer(event->rb, NULL);
 *
 * Macro definition of rcu_assign_pointer:
 *  #define rcu_assign_pointer(p, v) \
 *   __rcu_assign_pointer((p), (v), __rcu)
 *
 * Macro definition of __rcu_assign_pointer
 * #define __rcu_assign_pointer(p, v, space) \
 *     do { \
 *        smp_wmb(); \
 *        (p) = (typeof(*v) __force space *)(v); \
 *     } while (0)
 * When NULL is defined as "0", in expansion of __rcu_assign_pointer macro,
 * "typeof(*v) will be expanded as typeof(*(0)), which will lead to
 * "invalid type argument" error.
 */
#define NULL            (void*)0

/* Define the bit value according to the bit number. */
#define BIT_VALUE(bit_number)   (1<<(bit_number))

/* add #undef BIT_X to avoid compiler warning like warning: "BIT_0" redefined
 * when BIT_X has been defined in Marvell SDK header file
 */
#undef BIT_0
#define BIT_0       (1 <<  0)
#undef BIT_1
#define BIT_1       (1 <<  1)
#undef BIT_2
#define BIT_2       (1 <<  2)
#undef BIT_3
#define BIT_3       (1 <<  3)
#undef BIT_4
#define BIT_4       (1 <<  4)
#undef BIT_5
#define BIT_5       (1 <<  5)
#undef BIT_6
#define BIT_6       (1 <<  6)
#undef BIT_7
#define BIT_7       (1 <<  7)
#undef BIT_8
#define BIT_8       (1 <<  8)
#undef BIT_9
#define BIT_9       (1 <<  9)
#undef BIT_10
#define BIT_10      (1 << 10)
#undef BIT_11
#define BIT_11      (1 << 11)
#undef BIT_12
#define BIT_12      (1 << 12)
#undef BIT_13
#define BIT_13      (1 << 13)
#undef BIT_14
#define BIT_14      (1 << 14)
#undef BIT_15
#define BIT_15      (1 << 15)
#undef BIT_16
#define BIT_16      (1 << 16)
#undef BIT_17
#define BIT_17      (1 << 17)
#undef BIT_18
#define BIT_18      (1 << 18)
#undef BIT_19
#define BIT_19      (1 << 19)
#undef BIT_20
#define BIT_20      (1 << 20)
#undef BIT_21
#define BIT_21      (1 << 21)
#undef BIT_22
#define BIT_22      (1 << 22)
#undef BIT_23
#define BIT_23      (1 << 23)
#undef BIT_24
#define BIT_24      (1 << 24)
#undef BIT_25
#define BIT_25      (1 << 25)
#undef BIT_26
#define BIT_26      (1 << 26)
#undef BIT_27
#define BIT_27      (1 << 27)
#undef BIT_28
#define BIT_28      (1 << 28)
#undef BIT_29
#define BIT_29      (1 << 29)
#undef BIT_30
#define BIT_30      (1 << 30)
#undef BIT_31
#define BIT_31      (1 << 31)


/* size of address in bytes: MAC address
 */
#define SYS_TYPE_MAC_ADDR_LEN   6


/* size of address in bytes: IP address, same as in INET-ADDRESS-MIB
 */
#define SYS_TYPE_IPV4_ADDR_LEN  4   /* InetAddressIPv4 */
#define SYS_TYPE_IPV4Z_ADDR_LEN 8   /* InetAddressIPv4z */
#define SYS_TYPE_IPV6_ADDR_LEN  16  /* InetAddressIPv6 */
#define SYS_TYPE_IPV6Z_ADDR_LEN 20  /* InetAddressIPv6z */


/* common size as powers of 2 (binary), not powers of ten (decimal)
 */
#define SYS_TYPE_256_BYTES      256
#define SYS_TYPE_1K             1024
#define SYS_TYPE_1K_BYTES       1024
#define SYS_TYPE_64K_BYTES      (64 * SYS_TYPE_1K_BYTES)
#define SYS_TYPE_512K_BYTES     (512 * SYS_TYPE_1K_BYTES)
#define SYS_TYPE_1M_BYTES       (SYS_TYPE_1K_BYTES * SYS_TYPE_1K_BYTES)

/* used by function "L2MUX_MGR_SendBPDU"
 */
#define SYS_TYPE_IGNORE_VID_CHECK                   0xFFFF


/* MACRO DECLARATIONS
 */
#define SYS_TYPE_REGISTER_CALLBACKFUN(call_back_list)                               \
{                                                                                   \
    SYS_TYPE_CallBack_T  *fun_list;                                                 \
                                                                                    \
    fun_list = (SYS_TYPE_CallBack_T  *)malloc ( sizeof(SYS_TYPE_CallBack_T) );      \
                                                                                    \
    if ( fun_list == 0 )                                                            \
        return;                                                                     \
                                                                                    \
    fun_list->func = (SYS_TYPE_CALLBACK_FUNC_T) fun;                                \
    fun_list->next = call_back_list;                                                \
    call_back_list = fun_list;                                                      \
}

#define SYS_TYPE_REGISTER_CALLBACKFUN_WITH_RETVAL(call_back_list, FAIL_RETVAL)      \
{                                                                                   \
    SYS_TYPE_CallBack_T  *fun_list;                                                 \
                                                                                    \
    fun_list = (SYS_TYPE_CallBack_T  *)malloc ( sizeof(SYS_TYPE_CallBack_T) );      \
                                                                                    \
    if ( fun_list == 0 )                                                            \
        return FAIL_RETVAL;                                                         \
                                                                                    \
    fun_list->func = (SYS_TYPE_CALLBACK_FUNC_T) fun;                                \
    fun_list->next = call_back_list;                                                \
    call_back_list = fun_list;                                                      \
}



/* TYPE DECLARATIONS
 */

/* Define call-back function type.
 * Note: For any application of indirect-call or call-back function,
 *       the SYS_TYPE_CallBack_T shall be used to define the indirect-call variable.
 */
typedef void (*SYS_TYPE_CALLBACK_FUNC_T) (int, ...);

typedef struct SYS_TYPE_CallBack_S
{
    SYS_TYPE_CALLBACK_FUNC_T        func;
    struct SYS_TYPE_CallBack_S  	*next;

} SYS_TYPE_CallBack_T;



/* Define call-back function type.
 */
typedef struct
{
    UI16_T unit;
	UI16_T port;

} SYS_TYPE_Uport_T;

typedef struct SYS_TYPE_MSG_S
{
    UI32_T msg[4];

} SYS_TYPE_MSG_T;


/* Define Stacking mode type.
 */
typedef enum
{
    SYS_TYPE_STACKING_TRANSITION_MODE = 0,
    SYS_TYPE_STACKING_MASTER_MODE = 1,
    SYS_TYPE_STACKING_SLAVE_MODE = 2
} SYS_TYPE_Stacking_Mode_T;


/* Define CLI running config retrieve type
 */
typedef enum
{
    SYS_TYPE_GET_RUNNING_CFG_SUCCESS    = 1,
    SYS_TYPE_GET_RUNNING_CFG_FAIL       = 2,
    SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  = 3
} SYS_TYPE_Get_Running_Cfg_T;

/* Define Global IPC Command
 * starting from 1000, 0~999 are reserved for SYS_MODULE_ID_T
 */
typedef enum
{
    SYS_TYPE_CMD_ENTER_MASTER_MODE = 1000,
    SYS_TYPE_CMD_ENTER_SLAVE_MODE,
    SYS_TYPE_CMD_ENTER_TRANSITION_MODE,
    SYS_TYPE_CMD_PROVISION_COMPLETE,
    SYS_TYPE_CMD_MODULE_PROVISION_COMPLETE,
    SYS_TYPE_CMD_PREPROVISION_COMPLETE,
    SYS_TYPE_CMD_HANDLE_HOT_INSERTION,
    SYS_TYPE_CMD_HANDLE_HOT_REMOVAL,
    SYS_TYPE_CMD_RELOAD_SYSTEM,
    SYS_TYPE_CMD_IPCIO,
    SYS_TYPE_CMD_SYNC_SYS_TIME,
} SYS_TYPE_Global_IPC_Cmd_T;

/* Define CSCs subset index which can be used in the parameter of
 * Global IPC Command, in order to specify that the command only
 * applied to a subset of CSCs in the received group
 */
typedef enum
{
    SYS_TYPE_CMD_CSC_SUBSET_IDENTIFIER_L3IF = 1
} SYS_TYPE_CMD_CscSubset_T;

typedef struct 
{
   UI32_T csc_subset_identifier;   /* SYS_TYPE_CMD_CscSubset_T */
   UI32_T starting_port_ifindex;
   UI32_T number_of_port;
   UI8_T unit_id;
   BOOL_T use_default;
} SYS_TYPE_HandleHotSwapArg_T;



/*
 * This is a small structure to deal with 4K VLAN.
 *
 * The highest bit of 'if_count' is defined as a flag to indicate
 * whether this is bit-mapped interface array. The left 16-bit is
 * count of interfaces if interfaces are continous, or it is the
 * size in bytes of 'if_array'
 *
 * If (If_count & IF_BITMAP_IFMAPPED) is not 0, 'if_array' is an
 * array contain the interface bit map and if_start is the first
 * interface index.
 *
 */
#define IF_BITMAP_IFMAPPED                  0x8000
#define IF_BITMAP_IS_MAPPED(if_count)       (if_count &  IF_BITMAP_IFMAPPED)
#define IF_BITMAP_IFCOUNT(if_count)         (if_count & ~IF_BITMAP_IFMAPPED)
#define IF_BITMAP_ARRAY_SIZE(if_count)      IF_BITMAP_IFCOUNT(if_count)

struct IF_BITMAP_S
{
    UI16_T      if_start;           /* start ifindex                        */
    UI16_T      if_count;           /* count of interfaces or size of array */
    UI8_T       if_array[0];        /* interface bit map                    */
}; /* __attribute__((packed)); */

typedef struct IF_BITMAP_S  IF_BITMAP_T;

/* Define type for memory address
 */
typedef uintptr_t SYS_TYPE_PAddr_T;
typedef uintptr_t SYS_TYPE_VAddr_T;

#endif   /* SYS_TYPE_H */

