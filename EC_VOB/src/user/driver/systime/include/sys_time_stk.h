#ifndef SYS_TIME_STK_H
#define SYS_TIME_STK_H

#include "sys_type.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "l_mm_type.h"
#include "isc.h"
#include "sys_time.h"

#if (SYS_CPNT_STACKING == TRUE)

typedef struct
{
    Timezone_T tz;
    Software_clock_T sc;
} SYS_TIME_RemoteSyncDateTime_T;

#define ERRMSG printf
#define SYS_TIME_ISC_TIMEOUT_VALUE 1000
#define SYS_TIME_ISC_TRY_COUNT 2
#define SYS_TIME_COMMAND_RETRY_COUNT 2
/*
 * Opcodes for different remote sevices
 */

enum SYS_TIME_RETURN_CODE_E
{
    SYS_TIME_RETURN_OK = 0,                   /* OK, Successful, Without any Error */
    SYS_TIME_RETURN_ERROR,
};

enum
{  /********************************
    * OPCODES for remote operation *
    ********************************/
    SYS_TIME_SYNC_DATE_TIME_REQUEST = 0,
    SYS_TIME_ACK,
    SYS_TIME_NAK,
    SYS_TIME_SERVICE_NOT_SUPPORT,
    SYS_TIME_TOTAL_REMOTE_SERVICES,
};

#define SYS_TIME_POOL_ID_ISC_SEND    0
#define SYS_TIME_POOL_ID_ISC_REPLY   1

typedef struct SYS_TIME_Packet_Header_S
{
    UI16_T  opcode;
    UI16_T  seq_no;
    UI16_T  data_size; /* size of the service data unit */
} __attribute__((packed, aligned(1))) SYS_TIME_Packet_Header_T;

typedef struct SYS_TIME_Request_Packet_S
{
    SYS_TIME_Packet_Header_T header;
    SYS_TIME_RemoteSyncDateTime_T rsdt;
    BOOL_T next;
} __attribute__((packed, aligned(1))) SYS_TIME_Request_Packet_T;

typedef struct SYS_TIME_Response_Packet_S
{
    SYS_TIME_Packet_Header_T header;
    SYS_TIME_RemoteSyncDateTime_T rsdt;
} __attribute__((packed, aligned(1))) SYS_TIME_Response_Packet_T;

BOOL_T SYS_TIME_ISC_Handler(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p);
UI32_T SYS_TIME_RemoteSyncDateTime();

#endif /* SYS_CPNT_STACKING */


#endif

