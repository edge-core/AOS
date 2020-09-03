/* MODULE NAME: pppoe_ia_type.h
 * PURPOSE:
 *   Declarations of variables/structure used for PPPOE IA.
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   mm/dd/yy (A.D.)
 *   03/17/09    -- Squid Ro, Create
 *   11/26/09    -- Squid Ro, Modify for Linux platform
 *
 * Copyright(C)      Accton Corporation, 2009
 */

#ifndef _PPPOE_IA_TYPE_H
#define _PPPOE_IA_TYPE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "l_mm.h"
#include "sys_module.h" /* for L_MREF debugging */
#include "leaf_es3626a.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define PPPOE_IA_TYPE_PPPOED_ETHER_TYPE         0x8863
#define PPPOE_IA_TYPE_BITMASK_VERSION           0xf0
#define PPPOE_IA_TYPE_BITMASK_TYPE              0x0f
#define PPPOE_IA_TYPE_VENDOR_ID_ADSL_IANA       0x00000DE9

#define PPPOE_IA_TYPE_EVENT_NONE                0
#define PPPOE_IA_TYPE_EVENT_TIMER               (1 << 0)
#define PPPOE_IA_TYPE_EVENT_PDU_RX              (1 << 1)
#define PPPOE_IA_TYPE_EVENT_ENTER_TRANSITION    (1 << 2)
#define PPPOE_IA_TYPE_EVENT_ALL                 0x0F

/* AGENT_CIRCUIT_ID_LEN = ACCESS_NODE_ID_LEN + " eth " + CIRCUIT_ID_LEN <= 63
 */
#define PPPOE_IA_TYPE_MAX_ACCESS_NODE_ID_LEN    MAXSIZE_pppoeiaGlobalAccessNodeId
#define PPPOE_IA_TYPE_MIN_ACCESS_NODE_ID_LEN    1 /* for cli */
#define PPPOE_IA_TYPE_MAX_CIRCUIT_ID_LEN        MAXSIZE_pppoeiaPortCircuitId
#define PPPOE_IA_TYPE_MIN_CIRCUIT_ID_LEN        1 /* for cli */
#define PPPOE_IA_TYPE_MAX_REMOTE_ID_LEN         MAXSIZE_pppoeiaPortRemoteId
#define PPPOE_IA_TYPE_MIN_REMOTE_ID_LEN         1 /* for cli */
#define PPPOE_IA_TYPE_MAX_GENERIC_ERMSG_LEN     MAXSIZE_pppoeiaGlobalGenericErrorMsg
#define PPPOE_IA_TYPE_MIN_GENERIC_ERMSG_LEN     1 /* for cli */

#define PPPOE_IA_TYPE_MAX_FRAME_SIZE            1484
#define PPPOE_IA_TYPE_MAX_AGENT_CIRCUIT_ID_LEN  63

/* TAGTYPE + TAGLEN + ADSL IANA + AGENT CIRCUIT ID + REMOTE ID
 *       2        2           4                 65          65
 */
#define PPPOE_IA_TYPE_MAX_ACC_LOOP_ID_LEN       138

#define PPPOE_IA_TYPE_DFLT_SLOT_PORT_SYNTAX     "%ld/%ld:%d"
/* ex: 2/1:4095 */

#define PPPOE_IA_TYPE_DFLT_AGNT_CCID_SYNTAX     "%s eth %s"
/* ex: 192.168.1.1 eth 2/1:4095 */

#define PPPOE_IA_TYPE_DFLT_GEN_ERMSG            \
    "PPPoE Discover packet too large to process. Try reducing the number of tags added."

/* not used in current design */
//#define PPPOE_IA_TYPE_TIMER_TICKS1SEC           100 /* every 1 sec, not used at present */

#define PPPOE_IA_TYPE_MAX_DELIM_ASCII           (MAX_pppoeiaPortRemoteIdDelimiterAscii -1) /* for cli */
#define PPPOE_IA_TYPE_MIN_DELIM_ASCII           0
#define PPPOE_IA_TYPE_DFLT_RID_DASCII           35
#define PPPOE_IA_TYPE_DFLT_RID_DELIM_EN         FALSE


/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
/* according to RFC 2516
 */
enum PPPOE_HDR_CODE_E
{
    PPPOE_CODE_PADI =0x09,
    PPPOE_CODE_PADO =0x07,
    PPPOE_CODE_PADR =0x19,
    PPPOE_CODE_PADS =0x65,
    PPPOE_CODE_PADT =0xa7,
    PPPOE_CODE_UNKN =0xFF   /* for boundary checking */
};

enum PPPOE_HDR_TAGTYPE_E
{
    PPPOE_TAGTYPE_END_OF_LST  =0x0000,
    PPPOE_TAGTYPE_SVC_NAME    =0x0101,
    PPPOE_TAGTYPE_AC_NAME     =0x0102,
    PPPOE_TAGTYPE_HOST_INIQ   =0x0103,
    PPPOE_TAGTYPE_AC_COOKIE   =0x0104,
    PPPOE_TAGTYPE_VNDR_SPEC   =0x0105,
    PPPOE_TAGTYPE_RELAY_SID   =0x0110,
    PPPOE_TAGTYPE_SVC_NAME_ER =0x0201,
    PPPOE_TAGTYPE_AC_SYS_ER   =0x0202,
    PPPOE_TAGTYPE_GENERIC_ER  =0x0203,
};

enum PPPOE_IA_TYPE_PTYPE_E
{
    PPPOE_IA_TYPE_PTYPE_ENABLED,
    PPPOE_IA_TYPE_PTYPE_TRUST,
    PPPOE_IA_TYPE_PTYPE_UNTRUST,
};

enum PPPOE_IA_TYPE_FLDID_E
{                                           /* TYPE          API for UI Get/Set */
    PPPOE_IA_TYPE_FLDID_GLOBAL_ENABLE,      /* GLOBAL BOOL,  PPPOE_IA_MGR_GetGlobalEnable/SetGlobalEnable */
    PPPOE_IA_TYPE_FLDID_GLOBAL_ACC_NODE_ID, /* GLOBAL STRING,PPPOE_IA_MGR_GetAccessNodeId/SetGlobalStrDataByField  */
    PPPOE_IA_TYPE_FLDID_GLOBAL_GEN_ERMSG,   /* GLOBAL STRING,PPPOE_IA_MGR_GetGenericErrMsg/SetGlobalStrDataByField */
    PPPOE_IA_TYPE_FLDID_PORT_ENABLE,        /* PORT   BOOL,  PPPOE_IA_MGR_GetPortBoolDataByField/SetPortBoolDataByField */
    PPPOE_IA_TYPE_FLDID_PORT_TRUST,         /* PORT   BOOL,  PPPOE_IA_MGR_GetPortBoolDataByField/SetPortBoolDataByField */
    PPPOE_IA_TYPE_FLDID_PORT_STRIP_VENDOR,  /* PORT   BOOL,  PPPOE_IA_MGR_GetPortBoolDataByField/SetPortBoolDataByField */
    PPPOE_IA_TYPE_FLDID_PORT_CIRCUIT_ID,    /* PORT   STRING,PPPOE_IA_MGR_GetPortStrDataByField/SetPortStrDataByField */
    PPPOE_IA_TYPE_FLDID_PORT_REMOTE_ID,     /* PORT   STRING,PPPOE_IA_MGR_GetPortStrDataByField/SetPortStrDataByField */
    PPPOE_IA_TYPE_FLDID_PORT_RID_DELIM,     /* PORT   BOOL,  PPPOE_IA_MGR_GetPortBoolDataByField/SetPortBoolDataByField */
    PPPOE_IA_TYPE_FLDID_PORT_RID_DASCII,    /* PORT   UI32,  PPPOE_IA_MGR_GetPortUi32DataByField/SetPortUi32DataByField */
    PPPOE_IA_TYPE_FLDID_PORT_STS_PADI,      /* for ENGINE/OM */
    PPPOE_IA_TYPE_FLDID_PORT_STS_PADO,      /* for ENGINE/OM */
    PPPOE_IA_TYPE_FLDID_PORT_STS_PADR,      /* for ENGINE/OM */
    PPPOE_IA_TYPE_FLDID_PORT_STS_PADS,      /* for ENGINE/OM */
    PPPOE_IA_TYPE_FLDID_PORT_STS_PADT,      /* for ENGINE/OM */
    PPPOE_IA_TYPE_FLDID_PORT_STS_MALFORM,   /* for ENGINE/OM */
    PPPOE_IA_TYPE_FLDID_PORT_STS_REP_UNTRUST, /* for ENGINE/OM */
    PPPOE_IA_TYPE_FLDID_PORT_STS_REQ_UNTRUST, /* for ENGINE/OM */
    PPPOE_IA_TYPE_FLDID_PORT_STS_UNKNOWN,     /* for boundary checking */
};

typedef enum PPPOE_IA_TYPE_ECODE_E
{
    PPPOE_IA_TYPE_E_OK,
    PPPOE_IA_TYPE_E_GEN_ER,
    PPPOE_IA_TYPE_E_TLV_ER,
    PPPOE_IA_TYPE_E_UKN_ER,
} PPPOE_IA_TYPE_ECODE_T;

typedef enum PPPOE_IA_TYPE_EDROP_E
{
    PPPOE_IA_TYPE_E_NO_DROP,
    PPPOE_IA_TYPE_E_REQ_UNTRUST,
    PPPOE_IA_TYPE_E_REP_UNTRUST,
    PPPOE_IA_TYPE_E_MALOFORM,
    PPPOE_IA_TYPE_E_EGRESS_NOT_ENABLE,
    PPPOE_IA_TYPE_E_BAD_CODE,
} PPPOE_IA_TYPE_EDROP_T;

/* for trace_id of user_id when allocate buffer with l_mm
 */
enum
{
    PPPOE_IA_TYPE_TRACE_ID_PPPOE_IA_ENGINE_LOCALBUILDMREF = 0,
};

typedef struct
{
    UI8_T       ver_type;   /* version: 0xf0
                             * type   : 0x0f
                             */
    UI8_T       code;
    UI16_T      session_id;
    UI16_T      payload_len;
}__attribute__((packed, aligned(1))) PPPOE_PduCommonHdr_T;

/* according to TR101, 3.9.3.2 figure 21
 */
typedef struct
{
    UI16_T      tag_type;   /* vendor spec: 0x0105 */
    UI16_T      tag_len;
    UI32_T      adsl_iana;  /* 0x00000DE9          */
    UI8_T       payload_ar[0];
}__attribute__((packed, aligned(1))) PPPOE_AccLoopIdHdr_T;

typedef struct PPPOE_IA_TYPE_PktHdr_S
{
    UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T  lport;
    UI16_T  tag_info;
} PPPOE_IA_TYPE_PktHdr_T;

/* the msg size only has max 16 bytes
 */
typedef struct  PPPOE_IA_TYPE_Msg_S
{
    PPPOE_IA_TYPE_PktHdr_T  *pkt_hdr_p;
    L_MM_Mref_Handle_T      *mem_ref_p;
    UI32_T                  reserved1;
    UI32_T                  reserved2;
} PPPOE_IA_TYPE_Msg_T;

#endif /* End of _PPPOE_IA_TYPE_H */

