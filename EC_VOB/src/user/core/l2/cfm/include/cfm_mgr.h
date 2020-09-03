/*-----------------------------------------------------------------------------
 * Module Name: cfm_mgr.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Definitions for the LLDP API
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    12/5/2006 - Macauley Cheng, Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 20065
 *-----------------------------------------------------------------------------
 */

#ifndef _CFM_MGR_H
#define _CFM_MGR_H

#include "cfm_type.h"
#include "cfm_om.h"
#if (SYS_CPNT_CFM == TRUE)
/* INCLUDE FILE DECLARATIONS
 */

/* definitions of command in CFM which will be used in ipc message
 */
enum
{
    CFM_MGR_IPCCMD_ADDNEWMEP=1,
    CFM_MGR_IPCCMD_ADDREMOTEMEP,
    CFM_MGR_IPCCMD_CLEARERRORLIST,
    CFM_MGR_IPCCMD_CLEARERRORSLISTBYMDNAMEORLEVEL,
    CFM_MGR_IPCCMD_CLEARLINKTRACECACHE,
    CFM_MGR_IPCCMD_CLEARREMOTEMEPALL,
    CFM_MGR_IPCCMD_CLEARREMOTEMEPBYDOMAIN,
    CFM_MGR_IPCCMD_CLEARREMOTEMEPBYLEVEL,
    CFM_MGR_IPCCMD_DELETEDOT1AGCFMMA,
    CFM_MGR_IPCCMD_DELETEDOT1AGCFMMAMEPLISTENTRY,
    /*11-20*/
    CFM_MGR_IPCCMD_DELETEDOT1AGCFMMD,
    CFM_MGR_IPCCMD_DELETEDOT1AGCFMMEPENTRY,
    CFM_MGR_IPCCMD_DELETEMA,
    CFM_MGR_IPCCMD_DELETEMAVLAN,
    CFM_MGR_IPCCMD_DELETEMD,
    CFM_MGR_IPCCMD_DELETEMEP,
    CFM_MGR_IPCCMD_DELETEREMOTEMEP,
    CFM_MGR_IPCCMD_SETARCHIVEHOLDTIME,
    /*71-80*/
    CFM_MGR_IPCCMD_SETCCMINTERVAL,
    CFM_MGR_IPCCMD_SETCCMSTATUS,
    CFM_MGR_IPCCMD_SETCFMGLOBALSTATUS,
    CFM_MGR_IPCCMD_SETCFMPORTSTATUS,
    CFM_MGR_IPCCMD_SETCROSSCHECKSTARTDELAY,
    CFM_MGR_IPCCMD_SETCROSSCHECKSTATUS,
    CFM_MGR_IPCCMD_SETDOT1AGCFMDEFAULTMDIDPERMISSION,
    CFM_MGR_IPCCMD_SETDOT1AGCFMDEFAULTMDLEVEL,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMA,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMACCMINTERVAL,
    /*81-90*/
    CFM_MGR_IPCCMD_SETDOT1AGCFMMAFORMAT,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMAMEPLISTENTRY,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMAMHFCREATION,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMAMHFIDPERMISSION,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMANAME,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMAPRIMARYVLANVID,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMD,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMDFORMAT,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMDLEVEL,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMDMHFCREATION,
    /*91-100*/
    CFM_MGR_IPCCMD_SETDOT1AGCFMMDMHFIDPERMISSION,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMDNAME,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMEPACTIVE,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMEPCCIENABLE,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMEPCCMLTMPRIORITY,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMEPDIRECTION,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMEPENTRY,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMEPFNGALARMTIME,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMEPFNGRESETTIME,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMEPIFINDEX,
    /*101-110*/
    CFM_MGR_IPCCMD_SETDOT1AGCFMMEPLOWPRDEF,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMEPPRIMARYVID,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLBMDATATLV,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLBMDESTISMEPID,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLBMDESTMACADDRESS,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLBMDESTMEPID,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLBMMESSAGES,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLBMSTATUS,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLBMVLANDROPENABLE,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLBMVLANPRIORITY,
    /*111-120*/
    CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLTMEGRESSIDENTIFIER,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLTMFLAGS,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLTMSTATUS,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLTMTARGETISMEPID,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLTMTARGETMACADDRESS,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLTMTARGETMEPID,
    CFM_MGR_IPCCMD_SETDOT1AGCFMMEPTRANSMITLTMTTL,
    CFM_MGR_IPCCMD_SETDOT1AGCFMNUMOFVIDS,
    CFM_MGR_IPCCMD_SETFAULTNOTIFYALARMTIME,
    CFM_MGR_IPCCMD_SETFAULTNOTIFYLOWESTPRIORITY,
    /*121-130*/
    CFM_MGR_IPCCMD_SETFAULTNOTIFYRESETTIME,
    CFM_MGR_IPCCMD_SETLINKTRACECACHEHOLDTIME,
    CFM_MGR_IPCCMD_SETLINKTRACECACHESIZE,
    CFM_MGR_IPCCMD_SETLINKTRACECACHESTATUS,
    CFM_MGR_IPCCMD_SETMA,
    CFM_MGR_IPCCMD_SETMD,
    CFM_MGR_IPCCMD_SETSNMPCCSTATUS,
    CFM_MGR_IPCCMD_SETSNMPCROSSCHECKSTATUS,
    CFM_MGR_IPCCMD_TRANSMITLINKTRACE,
    CFM_MGR_IPCCMD_TRANSMITLOOPBACK,
    /*131-136*/
    CFM_MGR_IPCCMD_PROCESSPORTADMINDISABLED,
    /*AIS*/
    CFM_MGR_IPCCMD_SETAISPERIOD,
    CFM_MGR_IPCCMD_SETAISLEVEL,
    CFM_MGR_IPCCMD_SETAISSTATUS,
    CFM_MGR_IPCCMD_SETAISSUPRESSSTATUS,
    CFM_MGR_IPCCMD_CLEARAISERROR,
    CFM_MGR_IPCCMD_ABORTDELAYMEASUREBYDMM,
    CFM_MGR_IPCCMD_DODELAYMEASUREBYDMM,
    CFM_MGR_IPCCMD_ABORTTHRPTMEASUREBYLBM,
    CFM_MGR_IPCCMD_DOTHRPTMEASUREBYLBM,
    CFM_MGR_IPCCMD_SETMANAMEFORMAT,
    CFM_MGR_IPCCMD_FOLLOWISASYNCHRONISMIPC,         /****************/
    CFM_MGR_IPCCMD_CLEARLOOPBACKLIST
};

/* structure for the request ipc message in CFM_TYPE
 */
typedef struct
{
    union
    {
        UI32_T cmd;          /*cmd fnction id*/
        BOOL_T result_bool;  /*respond bool return*/
        UI32_T result_ui32;  /*respond ui32 return*/
    }type;

    union
    {

        BOOL_T  bool_v;
        UI8_T   ui8_v;
        I8_T    i8_v;
        UI32_T  ui32_v;
        UI16_T  ui16_v;
        I32_T   i32_v;
        I16_T   i16_v;
        UI8_T   ip4_v[4];

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
        }u32a1_u32a2;

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
            UI32_T u32_a3;
        }u32a1_u32a2_u32a3;

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
            UI16_T u16_a1;
        }u32a1_u32a2_u16a1;

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
            UI32_T u32_a3;
            UI32_T u32_a4;
        }u32a1_u32a2_u32a3_u32a4;

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
            UI32_T u32_a3;
            UI16_T u16_a1;
        }u32a1_u32a2_u32a3_u16a1;

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
            UI32_T u32_a3;
            BOOL_T bool_a1;
        }u32a1_u32a2_u32a3_bool;

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
            UI32_T u32_a3;
            UI8_T  mac_ar[SYS_ADPT_MAC_ADDR_LEN];
        }u32a1_u32a2_u32a3_mac;

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
            UI32_T u32_a3;
            UI8_T  identifer_ar[8];
        }u32a1_u32a2_u32a3_idary;

        struct
        {
            UI32_T                  lport;
            UI32_T                  mep_id;
            UI32_T                  md_name_len;
            UI32_T                  ma_name_len;
            UI8_T                   md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1];
            UI8_T                   ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1];
            CFM_TYPE_MP_Direction_T direction;
        }new_mep;

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
            UI32_T u32_a3;
            CFM_TYPE_MP_Direction_T  direction;
        }mep_direction_u32a3;

        struct
        {
            UI32_T md_name_len;
            UI32_T ma_name_len;
            UI32_T u32_a1;
            UI8_T  md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1];
            UI8_T  ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1];
        }md_ma_name_u32a1;

        struct
        {
            UI32_T md_name_len;
            UI32_T ma_name_len;
            UI32_T u32_a1;
            UI32_T u32_a2;
            UI8_T  md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1];
            UI8_T  ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1];
        }md_ma_name_u32a2;

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
            UI32_T u32_a3;
            UI8_T  ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1];
        }ma_name_u32a3;

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
            UI32_T u32_a3;
            CFM_OM_MepInfo_T mep;
        }mep_info_u32a3;

        struct
        {
            UI32_T name_len;
            UI8_T  md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1];
            CFM_TYPE_MdLevel_T level;
        }clear_error_by_md_level;

        struct
        {
            UI32_T name_len;
            UI8_T  md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1];
        }remote_mep_by_domain;

        struct
        {
            UI32_T md_index;
            UI32_T ma_index;
            UI32_T name_len;
            UI8_T  ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1];
            CFM_TYPE_CcmInterval_T interval;
        }ccm_interval;

        struct
        {
            UI32_T md_name_len;
            UI32_T ma_name_len;
            UI8_T  md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1];
            UI8_T  ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1];
            CFM_TYPE_CcmInterval_T interval;
        }setting_ccm_interval;

        struct
        {
            UI32_T md_index;
            UI32_T ma_index;
            CFM_TYPE_CcmInterval_T interval;
        }ma_ccm_interval;

        struct
        {
            UI32_T md_index;
            UI32_T ma_index;
            UI32_T name_len;
            UI8_T  ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1];
            CFM_TYPE_CcmStatus_T status;
        }ccm_status;

        struct
        {
            UI32_T md_name_len;
            UI32_T ma_name_len;
            UI8_T  md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1];
            UI8_T  ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1];
            CFM_TYPE_CcmStatus_T status;
        }setting_ccm_status;

        struct
        {
            UI32_T md_index;
            UI32_T ma_index;
            UI8_T  ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1];
            CFM_TYPE_CcmStatus_T status;
        }running_ccm_status;

        struct
        {
            UI32_T md_index;
            UI32_T ma_index;
            UI32_T mep_id;
            CFM_TYPE_CcmStatus_T status;
        }mep_cci_status;

        struct
        {
            UI32_T lport;
            CFM_TYPE_CfmStatus_T  status;
        }port_status;

        struct
        {
            UI32_T md_name_len;
            UI32_T ma_name_len;
            UI8_T  md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1];
            UI8_T  ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1];
            CFM_TYPE_CrossCheckStatus_T  status;
        }crosscheck_status;

        struct
        {
            UI32_T md_index;
            CFM_TYPE_MdLevel_T  level;
        }md_index_by_level;

        struct
        {
            UI32_T md_index;
            UI32_T ma_index;
            CFM_TYPE_MdLevel_T  level;
        }md_ma_index_level;

        struct
        {
            UI16_T vid;
            CFM_TYPE_MdLevel_T  level;
        }default_md_level;

        struct
        {
            UI32_T name_len;
            UI32_T md_index;
            UI8_T  md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1];
        }md_index_by_name;

        struct
        {
            UI32_T md_index;
            UI32_T ma_index;
            UI32_T mep_id;
            UI32_T content_len;
            UI8_T  content_ar[CFM_TYPE_MAX_FRAME_SIZE];
        }mep_error_ccm_fail;

        struct
        {
            UI32_T md_index;
            CFM_TYPE_FNG_LowestAlarmPri_T priority;
        }fault_notify_pri;

        struct
        {
            UI32_T md_index;
            UI32_T ma_index;
            UI32_T mep_id;
            CFM_TYPE_FNG_LowestAlarmPri_T priority;
        }mep_low_pri;

        struct
        {
            UI16_T md_primary_vid;
            CFM_TYPE_MhfIdPermission_T send_id_permission;
        }vid_permission;

        struct
        {
            UI32_T md_index;
            CFM_TYPE_MhfIdPermission_T send_id_permission;
        }md_permission;

        struct
        {
            UI32_T md_index;
            UI32_T ma_index;
            CFM_TYPE_MhfIdPermission_T send_id_permission;
        }ma_permission;

        struct
        {
            UI32_T md_index;
            UI32_T ma_index;
            CFM_TYPE_MA_Name_T format;
        }ma_format;

        struct
        {
            UI32_T md_index;
            UI32_T ma_index;
            CFM_TYPE_MhfCreation_T create_type;
        }ma_creation;

        struct
        {
            UI32_T md_index;
            CFM_TYPE_MhfCreation_T create_type;
        }md_creation;

        struct
        {
            UI32_T ma_index;
            UI32_T name_len;
            UI32_T md_index;
            UI32_T vid_num;
            UI16_T primary_vid;
            UI8_T  ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1];
            UI8_T  vid_list_ar[(SYS_DFLT_DOT1QMAXVLANID/8)+1];
            CFM_TYPE_MhfCreation_T create_way;
        }ma_create_entry;

        struct
        {
            UI32_T md_index;
            UI16_T name_len;
            UI8_T  md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1];
            CFM_TYPE_MdLevel_T      level;
            CFM_TYPE_MhfCreation_T  create_way;
        }md_create_entry;

        struct
        {
            UI32_T md_index;
            CFM_TYPE_MD_Name_T md_format;
        }md_format;

        struct
        {
            CFM_TYPE_SnmpTrapsCC_T trap;
            BOOL_T trap_enabled;
        }snmp_cc_status;

        struct
        {
            CFM_TYPE_SnmpTrapsCrossCheck_T trap;
            BOOL_T trap_enabled;
        }snmp_trap_cc_status;

        struct
        {
            UI32_T  u32_a1;
            UI32_T  u32_a2;
            UI32_T  u32_a3;
            UI32_T  u32_a4;
            UI32_T  md_name_len;
            UI32_T  ma_name_len;
            UI8_T   mac_ar[SYS_ADPT_MAC_ADDR_LEN];
            UI8_T   md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1];
            UI8_T   ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1];
        }transmit_settings;

        struct
        {
            UI32_T  src_mep_id;
            UI32_T  dst_mep_id;
            UI32_T  ma_name_len;
            UI32_T  counts;
            UI32_T  pkt_size;
            UI8_T   dst_mac_ar[SYS_ADPT_MAC_ADDR_LEN];
            UI8_T   ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1];
        }transmit_loopback;

        struct
        {
            UI32_T  src_mep_id;
            UI32_T  dst_mep_id;
            UI32_T  ma_name_len;
            UI32_T  md_name_len;
            UI32_T  counts;
            UI32_T  pkt_size;
            UI16_T  pattern;
            UI16_T  pkt_pri;
            UI8_T   dst_mac_ar[SYS_ADPT_MAC_ADDR_LEN];
            UI8_T   md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1];
            UI8_T   ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1];
        }do_thrpt_by_lbm;

        struct
        {
            UI32_T  real_send;
            UI32_T  rcvd_1sec;
            UI32_T  rcvd_total;
            UI8_T   res_bmp;
        }get_thrpt_res;

        struct
        {
            UI32_T  src_mep_id;
            UI32_T  dst_mep_id;
            UI32_T  ma_name_len;
            UI32_T  md_name_len;
            UI32_T  counts;
            UI32_T  interval;
            UI32_T  timeout;
            UI32_T  pkt_size;
            UI16_T  pkt_pri;
            UI8_T   dst_mac_ar[SYS_ADPT_MAC_ADDR_LEN];
            UI8_T   ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1];
            UI8_T   md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1];
        }transmit_dmm;

        struct
        {
            UI32_T  src_mep_id;
            UI32_T  dst_mep_id;
            UI32_T  md_name_len;
            UI32_T  ma_name_len;
            UI8_T   dst_mac_ar[SYS_ADPT_MAC_ADDR_LEN];
            UI8_T   md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1];
            UI8_T   ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1];
        }abort_xmit;

        struct
        {
            UI32_T  idx;
            UI32_T  fd_ms;
            UI8_T   res;
            BOOL_T  is_succ;
        }getnext_dmr_rec;

        struct
        {
            UI32_T  avg_fd_ms;
            UI32_T  min_fd_ms;
            UI32_T  max_fd_ms;
            UI32_T  succ_cnt;
            UI32_T  total_cnt;
        }get_dmr_res;

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
            CFM_TYPE_AIS_STATUS_T status;
        }get_ais_status;

        CFM_TYPE_MdLevel_T         md_level;
        CFM_TYPE_CfmStatus_T       cfm_status;
        CFM_TYPE_LinktraceStatus_T linktrace_status;

    } data; /* contains the supplemntal data for the corresponding cmd */
} CFM_MGR_IPCMsg_T;

#define CFM_MGR_MSGBUF_TYPE_SIZE    sizeof(((CFM_MGR_IPCMsg_T *)0)->type)
#define CFM_MGR_DATA ((CFM_MGR_IPCMsg_T *)0)->data

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessRcvdPDU
 *-------------------------------------------------------------------------
 * PURPOSE  : Process the recevied CFMDU.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : this is call from the cfm_task.c
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessRcvdPDU(L_MM_Mref_Handle_T	*mref_handle_p,
                                                   UI8_T 	        dst_mac[6],
                                                   UI8_T 	        src_mac[6],
                                                   UI16_T 	        tag_info,
                                                   UI16_T           type,
                                                   UI32_T           pkt_length,
                                                   UI32_T 	        lport);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessTimerEvent
 *-------------------------------------------------------------------------
 * PURPOSE  : Process Timer event.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This only call by CFM_Task.main() one second
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessTimerEvent();

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_GetOperationMode
 *-------------------------------------------------------------------------
 * PURPOSE  : get the bridge operation mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T CFM_MGR_GetOperationMode();

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Enter Master mode calling by cfm_task.c
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void CFM_MGR_EnterMasterMode();

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_EnterSlaveMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Enter Slave mode calling by cfm_task.c
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void CFM_MGR_EnterSlaveMode();

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_EnterTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Sending enter transition event to task calling by cfm_task.c
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void CFM_MGR_EnterTransitionMode();

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Sending enter transition event to task calling by cfm_task.c
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void CFM_MGR_SetTransitionMode();

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_Init
 * ------------------------------------------------------------------------
 * PURPOSE  : Initial the semaphore and need default value variable in thie interface
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void CFM_MGR_Init();

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetMD
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will create or modify the MD
 * INPUT    : md_index - the MD index
 *            *name_ap - the MD name array pointer
 *            name_len - the MD name length
 *            level  - the MD level
 *            create_way  - the mip create way
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetMD(UI32_T md_index, UI8_T *name_ap, UI16_T name_len, CFM_TYPE_MdLevel_T level,
                                        CFM_TYPE_MhfCreation_T create_way);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_DeleteMD
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will delete the MD
 * INPUT    : md_index - the MD index
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :If there exist MA in this domain, this MD can't be deleted
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_DeleteMD(UI32_T md_index);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetMA
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will create or modify the MA info
 * INPUT    : ma_index    - the ma index
 *            name_len    - the ma name
 *            md_index    - the md index
 *            primary_vid - the primary vid of the maS
 *            vlid_list   - the array store the vids
 *            create_way  - the mip create way
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetMA(UI32_T ma_index, UI8_T *name, UI32_T name_len, UI32_T md_index, UI16_T primary_vid,
                                        UI32_T vid_num,UI8_T vid_list[(SYS_DFLT_DOT1QMAXVLANID/8)+1], CFM_TYPE_MhfCreation_T create_way);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetMANameFormat
 *-------------------------------------------------------------------------
 * PURPOSE  : To set the name format of MA
 * INPUT    : md_index    - the md index
 *            ma_index    - the ma index
 *            name_format - the name format
 *                          (CFM_TYPE_MA_NAME_CHAR_STRING,
 *                           CFM_TYPE_MA_NAME_ICC_BASED)
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetMANameFormat(
    UI32_T                  md_index,
    UI32_T                  ma_index,
    CFM_TYPE_MA_Name_T      name_format);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_DeleteMA
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will delete the MA
 * INPUT    : md_index - the md index
 *            ma_index - the ma index
 * OUTPUT   :
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : If there still exist the Mep in this MA, this MA can't be deleted
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_DeleteMA(UI32_T md_index,UI32_T ma_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_DeleteMAVlan
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will delete the MA vlan
 * INPUT    : md_index - the md index
 *            ma_index - the ma index
 *            vid      - the vlan id
 * OUTPUT   :
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : if this vid is primary vid and ma still exit mep, this vid can't be deleted
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_DeleteMAVlan(UI32_T md_index,UI32_T ma_index,UI16_T vid);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_AddnewMEP
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will add new Mep to a MD/MA
 * INPUT    : lport       - the logical port
 *            mep_id      - the mep identifier
 *            md_name_a   - the Md name
 *            md_name_len - the Md name length
 *            ma_name_a   - the Ma name
 *            ma_name_len - the Ma name length
 *            direction   - the direction of mep
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_AddnewMEP(
    UI32_T                  lport,
    UI32_T                  mep_id,
    UI8_T                   md_name_a[CFM_TYPE_MD_MAX_NAME_LENGTH],
    UI32_T                  md_name_len,
    UI8_T                   ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH],
    UI32_T                  ma_name_len,
    CFM_TYPE_MP_Direction_T direction);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_DeleteMEP
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will delete a Mep from MA
 * INPUT    : lport  - the lport which mep reside
 *            mep_id - the mep id
 *            ma_name_a - the array pointr store the Ma name
 *            name_len - the ma_name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_DeleteMEP(
    UI8_T   md_name_ap[CFM_TYPE_MD_MAX_NAME_LENGTH],
    UI32_T  md_name_len,
    UI8_T   ma_name_ap[CFM_TYPE_MA_MAX_NAME_LENGTH],
    UI32_T  ma_name_len,
    UI32_T  lport,
    UI32_T  mep_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetCcmInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will set the CCM transmit interval level
 * INPUT    : md_name_ap  - the md name array pointer
 *            md_name_len - md name length
 *            ma_name_ap  - the ma name array pointer
 *            ma_name_len - ma name length
 *            interval    - the ccm interval level
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS - configure success
 *            CFM_TYPE_CONFIG_ERROR   - configure fail at interval parameter
 * NOTE     : for our switch use 1 sec. as time unit, so contraint interval level as 1 sec.
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetCcmInterval(
    UI8_T                   *md_name_ap,
    UI32_T                  md_name_len,
    UI8_T                   *ma_name_ap,
    UI32_T                  ma_name_len,
    CFM_TYPE_CcmInterval_T  interval);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetCcmStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will set the CCM transmit status
 * INPUT    : md_name_ap  - the MD name array pointer
 *            md_name_len - the MD name length
 *            ma_name_ap  - the MA name array pointer
 *            ma_name_len - the MA name length
 *            status      - the CFM enable status
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - fail at status parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetCcmStatus(
    UI8_T                   *md_name_ap,
    UI32_T                  md_name_len,
    UI8_T                   *ma_name_ap,
    UI32_T                  ma_name_len,
    CFM_TYPE_CcmStatus_T    status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetLinkTraceCacheStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will set the link trace cache status.
 * INPUT    : status  - the link trace cache status
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : If the link cache status set enable, then the om start to record the link trace reply
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetLinkTraceCacheStatus(CFM_TYPE_LinktraceStatus_T status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetLinkTraceCacheSize
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will set the link trace reply cache size
 * INPUT    : size - the link trace cache size
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetLinkTraceCacheSize(UI32_T size);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetLinkTraceCacheHoldTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the link trace cache entries hold time
 * INPUT    : hold_time - the link trace cache entries hold time
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : in minutes
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetLinkTraceCacheHoldTime(UI32_T hold_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ClearLinktraceCache
 *-------------------------------------------------------------------------
 * PURPOSE  : This function clear all the link trace reply in om
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_ClearLinktraceCache();

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetArchiveHoldTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the remote mep info store hold time
 * INPUT    : md_index  - the md index
 *            hold_time - the remote mep info hold time
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetArchiveHoldTime(UI32_T md_index, UI32_T hold_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_TransmitLinktrace
 *-------------------------------------------------------------------------
 * PURPOSE  : This function trigger the link trace message be sent
 * INPUT    : src_mep_id  - the source mep id
 *            dst_mep_id  - the destination mep id
 *            mac_addr    - the dest mac address of link trace message
 *            md_name_a   - the md name array
 *            md_name_len - the md name length
 *            ma_name_a   - the ma name array
 *            ma_name_len - the ma name length
 *            ttl         - the time to live
 *            pkt_pri     - the priority to transmit packet
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - fail transmit linktrace
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_TransmitLinktrace(
    UI32_T  src_mep_id,
    UI32_T  dst_mep_id,
    UI8_T   mac_addr[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T   md_name_a[CFM_TYPE_MD_MAX_NAME_LENGTH],
    UI32_T  md_name_len,
    UI8_T   ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH],
    UI32_T  ma_name_len,
    UI32_T  ttl,
    UI16_T  pkt_pri);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_TransmitLoopback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function trigger the loop back message be sent
 * INPUT    : mep_id      - the target mep_id
 *            mac_addr    - the target dest mac
 *            md_name_a   - the md name array
 *            md_name_len - the md name length
 *            ma_name_a   - the ma name array
 *            ma_name_len - the ma name length
 *            counts      - the transmit counts
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_TransmitLoopback(
    UI32_T      mep_id,
    UI8_T       mac_addr[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T       md_name_a[CFM_TYPE_MD_MAX_NAME_LENGTH],
    UI32_T      md_name_len,
    UI8_T       ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH],
    UI32_T      ma_name_len,
    UI32_T      counts);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ClearLoopBackList
 *-------------------------------------------------------------------------
 * PURPOSE  : This function trigger the loop back message be sent
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ClearLoopBackList();

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetCrossCheckStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the cross check status
 * INPUT    : status      - the cross check status
 *            md_name_ap  - the md name array pointer
 *            md_name_len - the md name length
 *            ma_name_ap  - the ma name array pointer
 *            ma_name_len - the ma name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : CFM_TYPE_CROSS_CHECK_STATUS_DISABLE,
 *            CFM_TYPE_CROSS_CHECK_STATUS_ENABLE
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetCrossCheckStatus(
    UI8_T                       *md_name_ap,
    UI32_T                      md_name_len,
    UI8_T                       *ma_name_ap,
    UI32_T                      ma_name_len,
    CFM_TYPE_CrossCheckStatus_T status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetCrossCheckStartDelay
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the cross check start delay
 * INPUT    : delay  - the cross check start delay
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetCrossCheckStartDelay(UI32_T delay);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetCFMGlobalStatus
 *-------------------------------------------------------------------------
 * PURPOSE  :  This function set the globl CFM status
 * INPUT    : staus - the CFM status
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetCFMGlobalStatus(CFM_TYPE_CfmStatus_T status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetCFMPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the each port CFM status
 * INPUT    : lport  - the logical port
 *            status - the CFM status
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetCFMPortStatus(UI32_T lport, CFM_TYPE_CfmStatus_T status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_AddRemoteMep
 *-------------------------------------------------------------------------
 * PURPOSE  : This function add a cross check remote mep
 * INPUT    : md_index - the md index
 *            mep_id   - the mep identifier
 *            ma_name_a- the ma name array
 *            name_len - the ma name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_AddRemoteMep(UI32_T md_index ,UI32_T mep_id,UI8_T ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH],UI32_T name_len);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_DeleteRemoteMep
 *-------------------------------------------------------------------------
 * PURPOSE  : This function delete a  remote mep
 * INPUT    : md_index  - the md index
 *            mep_id    - the mep identifier
 *            ma_name_a - the ma name stored array
 *            name_len  - the ma name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_DeleteRemoteMep(UI32_T md_index ,UI32_T mep_id,UI8_T ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH],UI32_T name_len);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetFaultNotifyLowestPriority
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the fault notify lowest priority
 * INPUT    : md_ndex  - the md index
 *            priority - the lowest fault notify priority
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetFaultNotifyLowestPriority(UI32_T md_index, CFM_TYPE_FNG_LowestAlarmPri_T priority);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetFaultNotifyAlarmTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the fault notify alarm time
 * INPUT    : md_index   - the md index
 *            alarm_time - the fault notify alarm time in second
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetFaultNotifyAlarmTime(UI32_T md_index, UI32_T alarm_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetFaultNotifyRestTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the fault notify reset time
 * INPUT    : md_index   - the md index
 *            reset_time - the fault notify rest time in second
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetFaultNotifyRestTime(UI32_T md_index, UI32_T reset_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ClearRemoteMepByDomain
 *-------------------------------------------------------------------------
 * PURPOSE  : This function clear remote mep info by MD domain
 * INPUT    : *md_name_ap - the md name array pointer
 *            name_len    - the md name length
 * OUTPUT   : None
  * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_ClearRemoteMepAll();

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ClearRemoteMepByDomain
 *-------------------------------------------------------------------------
 * PURPOSE  : This function clear remote mep info by MD domain
 * INPUT    : *md_name_ap - the md name array pointer
 *            name_len    - the md name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_ClearRemoteMepByDomain(UI8_T *md_name_ap, UI32_T name_len);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ClearRemoteMepByLevel
 *-------------------------------------------------------------------------
 * PURPOSE  : This function clear remote mep info by MD level
 * INPUT    : level - the md level
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_ClearRemoteMepByLevel(CFM_TYPE_MdLevel_T level);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ClearErrorsListByMdNameOrLevel
 *-------------------------------------------------------------------------
 * PURPOSE  : This function clear the error list
 * INPUT    : *md_name_ap - the md name array pointer
 *            name_len    - the md name length
 *            levle       - the md level
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : you can specify the md name or the level
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_ClearErrorList();

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ClearErrorsListByMdNameOrLevel
 *-------------------------------------------------------------------------
 * PURPOSE  : This function clear the error list
 * INPUT    : *md_name_ap - the md name array pointer
 *            name_len    - the md name length
 *            levle       - the md level
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : you can specify the md name or the level
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_ClearErrorsListByMdNameOrLevel(UI8_T *md_name_ap, UI32_T name_len,CFM_TYPE_MdLevel_T level);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetSNMPCcStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set which SNMP trap enable
 * INPUT    : trap         - the snamp CC trap type
 *            trap_enabled - the trap status
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetSNMPCcStatus(CFM_TYPE_SnmpTrapsCC_T trap, BOOL_T trap_enabled);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetSNMPCrosscheckStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the the SNMP trap status
 * INPUT    : trap        - the snmp cross check trap type
 *            trap_enable - the trap status
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetSNMPCrosscheckStatus(CFM_TYPE_SnmpTrapsCrossCheck_T trap, BOOL_T trap_enabled);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ClearAisError
 * ------------------------------------------------------------------------
 * PURPOSE  : This function clear ais error
 * INPUT    : mep_id      - the mep id
 *            md_name_p   - pointer to md name
 *            md_name_len - md name length
 *            ma_name_p   - pointer to ma name
 *            ma_name_len - ma name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - set faileds
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_ClearAisError(
    UI32_T  mep_id,
    UI8_T   *md_name_a,
    UI32_T  md_name_len,
    UI8_T   *ma_name_a,
    UI32_T  ma_name_len);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetAisPeriod
 * ------------------------------------------------------------------------
 * PURPOSE  :This function set the ais perirod
 * INPUT    :*ma_name_a   - the ma name
 *               name_len   - the ma name length
 *                period       - the ais priod
 * OUTPUT   :None
 * RETUEN    CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - get failed
 * NOTES    : CFM_TYPE_AIS_PERIOD_1S, CFM_TYPE_AIS_PERIOD_60S
 * ------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetAisPeriod(
    UI8_T   *md_name_a,
    UI32_T  md_name_len,
    UI8_T   *ma_name_a,
    UI32_T  ma_name_len,
    UI32_T  period);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetAisLevel
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the ais level
 * INPUT    : md_name_a   - the md name
 *            md_name_len - the md name length
 *            ma_name_a   - the ma name
 *            ma_name_len - the ma name length
 *            ais_level   - the ais level
 * OUTPUT   : None
 * RETUEN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - get failed
 * NOTES    : level 0 is default
 * ------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetAisLevel(
    UI8_T               *md_name_a,
    UI32_T              md_name_len,
    UI8_T               *ma_name_a,
    UI32_T              ma_name_len,
    CFM_TYPE_MdLevel_T  ais_level);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetAisStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the ais status
 * INPUT    : md_name_a   - the md name
 *            md_name_len - the md name length
 *            ma_name_a   - the ma name
 *            ma_name_len - the ma name length
 *            ais_status  - the ais status
 * OUTPUT   : None
 * RETUEN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - get failed
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetAisStatus(
    UI8_T                   *md_name_a,
    UI32_T                  md_name_len,
    UI8_T                   *ma_name_a,
    UI32_T                  ma_name_len,
    CFM_TYPE_AIS_STATUS_T   ais_status);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetAisSuppressStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the ais supress status
 * INPUT    : md_name_a          - the md name
 *            md_name_len        - the md name length
 *            ma_name_a          - the ma name
 *            ma_name_len        - the ma name length
 *            ais_supress_status - the ais suppress status
 * OUTPUT   : None
 * RETUEN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - get failed
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetAisSuppressStatus(
    UI8_T                   *md_name_a,
    UI32_T                  md_name_len,
    UI8_T                   *ma_name_a,
    UI32_T                  ma_name_len,
    CFM_TYPE_AIS_STATUS_T   ais_suppress_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMdFormat
 *-------------------------------------------------------------------------
 * PURPOSE  : This function create the md
 * INPUT    : md_index  - the md index
 *            md_format - the md format
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : 1. This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMd(UI32_T md_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_DeleteDot1agCfmMd
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will delete the MD
 * INPUT    : md_index - the MD index
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at md parameter
 * NOTE     :1. If there exist MA in this domain, this MD can't be deleted
 *           2. for mib
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_DeleteDot1agCfmMd(UI32_T md_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMdFormat
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the md name formate
 * INPUT    : md_index  - the md index
 *            md_format - the md format
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : 1. This function only for MIB
 *            2. only support char string
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMdFormat(UI32_T md_index, CFM_TYPE_MD_Name_T md_format);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMdName
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the md name
 * INPUT    : md_index    - the md index
 *            *name_ap    - the md name array pointer
 *            name_length - the md name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMdName(UI32_T md_index, UI8_T *name_ap, UI32_T name_length);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMdLevel
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the md level
 * INPUT    : md_index  - the md index
 *            level     - the md level
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMdLevel(UI32_T md_index, CFM_TYPE_MdLevel_T level);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMdMhfCreation
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the mhf cration type
 * INPUT    : md_index  - the md index
 *            create_type - the mhf create type
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter* NOTE     : This function only for MIB
 *            CFM_TYPE_MHF_CREATION_NONE
 *            CFM_TYPE_MHF_CREATION_DEFAULT
 *            CFM_TYPE_MHF_CREATION_EXPLICIT
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMdMhfCreation(UI32_T md_index, CFM_TYPE_MhfCreation_T create_type);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMdMhfIdPermission
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the mhf cration type
 * INPUT    : md_index  - the md index
 *           send_id_permission - the send id permission
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : This function only for MIB
 *            CFM_TYPE_MHF_ID_PERMISSION_NONE=1,
 *            CFM_TYPE_MHF_ID_PERMISSION_CHASSIS,
 *            CFM_TYPE_MHF_ID_PERMISSION_SENDID_MANAGE,
 *            CFM_TYPE_MHF_ID_PERMISSION_SENDID_DEFER
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMdMhfIdPermission (UI32_T md_index, CFM_TYPE_MhfIdPermission_T send_id_permission);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMaFormat
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma nam formate
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            format    - the ma name format
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :  only support char string
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMa(UI32_T md_index, UI32_T ma_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_DeleteDot1agCfmMa
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will delete the MA
 * INPUT    : md_index - the md index
 *            ma_index - the ma index
 * OUTPUT   :
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : 1. If there still exist the Mep in this MA, this MA can't be deleted
 *           2. for mib only
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_DeleteDot1agCfmMa(UI32_T md_index, UI32_T ma_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMaFormat
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma nam formate
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            format    - the ma name format
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMaFormat(UI32_T md_index, UI32_T ma_index, CFM_TYPE_MA_Name_T format);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMaName
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma name
 * INPUT    : md_index    - the md index
 *            ma_index    - the ma index
 *            *name_ap    - the ma name array pointer
 *            name_length - the ma name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     :
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMaName(UI32_T md_index, UI32_T ma_index, UI8_T *name_ap, UI32_T name_length);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMaMhfCreation
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma mhf creation type
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            create_type -the mhf cration type
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail
 * NOTE     : This function only for MIB
 * 	      CFM_TYPE_MHF_CREATION_NONE
 *            CFM_TYPE_MHF_CREATION_DEFAULT
 *            CFM_TYPE_MHF_CREATION_EXPLICIT
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMaMhfCreation(UI32_T md_index, UI32_T ma_index, CFM_TYPE_MhfCreation_T create_type);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMaMhfIdPermission
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the mhf cration type
 * INPUT    : md_index  - the md index
 *           ma_index   - the ma index
 *           send_id_permission - the send id permission
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : This function only for MIB
 *            CFM_TYPE_MHF_ID_PERMISSION_NONE=1,
 *            CFM_TYPE_MHF_ID_PERMISSION_CHASSIS,
 *            CFM_TYPE_MHF_ID_PERMISSION_SENDID_MANAGE,
 *            CFM_TYPE_MHF_ID_PERMISSION_SENDID_DEFER
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMaMhfIdPermission(UI32_T md_index, UI32_T ma_index, CFM_TYPE_MhfIdPermission_T send_id_permission);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMaCcmInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma send ccm interval
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            interval  - the sending ccm interval
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : This function only for MIB
 *            only support 4-7
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMaCcmInterval(UI32_T md_index, UI32_T ma_index, CFM_TYPE_CcmInterval_T interval);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMaMepListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma mep id list
 * INPUT    : md_index        - the md index
 *            ma_index        - the ma index
 *            mep_id          - the mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : 1. This function only for MIB
 *            2. not support this function, it will just return fail
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMaMepListEntry(UI32_T md_index, UI32_T ma_index, UI32_T mep_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_DeleteDot1agCfmMaMepListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function delete the ma mep id list
 * INPUT    : md_index        - the md index
 *            ma_index        - the ma index
 *            mep_id          - the mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : 1. This function only for MIB
 *            2. not support this function, it will just return fail
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_DeleteDot1agCfmMaMepListEntry(UI32_T md_index, UI32_T ma_index, UI32_T mep_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmNumOfVids
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma can have more than on vid
 * INPUT    : md_index   - the md index
 *            ma_index   - the ma index
 *            vid_num    - can have more than one vid
 * OUTPUT   : None
  * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmNumOfVids(UI32_T md_index, UI32_T ma_index, UI32_T vid_num);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMaPrimaryVlanVid
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma vlan id
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            primary_vid- the primary vlan id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMaPrimaryVlanVid(UI32_T md_index, UI32_T ma_index, UI16_T primary_vid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepIfIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's logical port
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            ifindex   - the logical port
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : 1. This function only for MIB
 *            2. the mep can't be configured on trunk member
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepIfIndex(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepDirection
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's dirction
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepDirection(UI32_T md_index, UI32_T ma_index, UI32_T mep_id,CFM_TYPE_MP_Direction_T directoin );

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepPrimaryVid
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's dirction
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            direction  - the mep direction
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepPrimaryVid(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI16_T primary_vid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function create the new mep
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepEntry(UI32_T md_index, UI32_T ma_index, UI32_T mep_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_DeleteDot1agCfmMepEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function delete the mep entry
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_DeleteDot1agCfmMepEntry(UI32_T md_index, UI32_T ma_index, UI32_T mep_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepActive
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's active status
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            active    - the mep active status
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepActive(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, BOOL_T active);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepCciEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's dirction
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            status    - the cci status
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : 1. This function only for MIB
 *            2. CFM_TYPE_CCI_STATUS_ENABLE
 *               CFM_TYPE_CCI_STATUS_DISABLE
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepCciEnable(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, CFM_TYPE_CcmStatus_T cci_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepCcmLtmPriority
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm priority
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            ccm_ltm_priority- the ccm and ltm default priority
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepCcmLtmPriority(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T ccm_ltm_priority);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepLowPrDef
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's lowest alarm priority
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            low_pri   - the lowest priority
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : 1. This function only for MIB
 *            2. CFM_TYPE_FNG_LOWEST_ALARM_ALL
 *               CFM_TYPE_FNG_LOWEST_ALARM_MAC_REM_ERR_XCON
 *               CFM_TYPE_FNG_LOWEST_ALARM_REM_ERR_XCON
 *               CFM_TYPE_FNG_LOWEST_ALARM_ERR_XCON
 *               CFM_TYPE_FNG_LOWEST_ALARM_XCON
 *               CFM_TYPE_FNG_LOWEST_ALARM_NO_DEF
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepLowPrDef(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, CFM_TYPE_FNG_LowestAlarmPri_T low_pri);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepFngAlarmTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's fng alarm time
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            alarm_time  - the fault alarm time by ticks
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : 1.This function only for MIB
 *            2. only supoort 300...10000 ticks
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepFngAlarmTime(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T alarm_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepFngResetTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's fault alarm reset time
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            reset_time- the fault alarm reset time by ticks
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : 1.This function only for MIB
 *            2. only supoort 300...10000 ticks
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepFngResetTime(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T reset_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLbmStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function transmit the lbm
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLbmStatus(UI32_T md_index, UI32_T ma_index, UI32_T mep_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLbmDestMacAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's destination mac address
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            dst_mac   - the lbm destination mac
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLbmDestMacAddress(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI8_T dst_mac[SYS_ADPT_MAC_ADDR_LEN]);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLbmDestMepId
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's lbm destination mep id
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            dst_mac   - the lbm destination mac
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLbmDestMepId(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T dst_mep_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLbmDestIsMepId
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's destination use mep id or mac
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            is_mep_id - set the destination address is mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLbmDestIsMepId(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, BOOL_T is_mep_id );

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLbmMessages
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's transmit the lbm counts
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            counts    - the lbm message counts
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLbmMessages(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T counts);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLbmDataTlv
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's tranmit lbm include data
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : 1. This function only for MIB
 *            2. this not suuport, it will just retrun fail
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLbmDataTlv(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI8_T *content, UI32_T content_length);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLbmVlanPriority
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's transmit blm vlan priority
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            priority  - the lbm priority
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLbmVlanPriority(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T priority);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLbmVlanDropEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's vlan drop ability
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            is_enable - the lbm vlan drop enable
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : 1. This function only for MIB
 *            2. this is not support will just return fail
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLbmVlanDropEnable(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, BOOL_T is_enable);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLtmFlags
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm transmit flags
 * INPUT    : md_index      - the md index
 *            ma_index      - the ma index
 *            mep_id        - the mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLtmStatus(UI32_T md_index, UI32_T ma_index, UI32_T mep_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLtmFlags
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm transmit flags
 * INPUT    : md_index      - the md index
 *            ma_index      - the ma index
 *            mep_id        - the mep id
 *            is_useFDBonly - set only use the fdb or not
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLtmFlags(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, BOOL_T is_useFDBonly);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLtmTargetMacAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's target mac
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            target_mac-the ltm target address
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLtmTargetMacAddress(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI8_T target_mac[SYS_ADPT_MAC_ADDR_LEN]);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLtmTargetMepId
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's target mp id
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            target_mep_id - the target mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLtmTargetMepId(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T target_mep_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepDirection
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm target is the mep id
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            is_mep_id - the ltm target is the mep id
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLtmTargetIsMepId(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, BOOL_T is_mep_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLtmTtl
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm ttl
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            ttl       - the trnamsmit ttl
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLtmTtl(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI32_T ttl);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmMepTransmitLtmEgressIdentifier
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's egress identifier
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            identifer - the egress identifier
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR - configure fail at ma parameter
 * NOTE     : 1. This function only for MIB
 *            2. this ability not suport, it will just return fail
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmMepTransmitLtmEgressIdentifier(UI32_T md_index, UI32_T ma_index, UI32_T mep_id, UI8_T identifer[8]);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmDefaultMdLevelMhfCreation
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set default md level mhf creation type
 * INPUT    : md_primary_vid  - the md primary vid
 *           creation_type    - the mhf creation type
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR   - configure fail
 * NOTE     : This function only for MIB
 *           default is none
 *            not supported yet
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmDefaultMdLevelMhfCreation(UI16_T md_primary_vid, CFM_TYPE_MhfCreation_T creation_type);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_SetDot1agCfmDefaultMdLevel
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set default md level
 * INPUT    : md_primary_vid  - the md primary vid
 *           level                    - the md level
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS  - configure success
 *            CFM_TYPE_CONFIG_ERROR   - configure fail
 * NOTE     : This function only for MIB
 *           default is none
 *            not supported yet
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_SetDot1agCfmDefaultMdLevel(UI16_T md_primary_vid, CFM_TYPE_MdLevel_T level);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessTrunkAdd1stMember_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the trunk member add call back
 * INPUT    : trunk_ifindex  - the trunk ifindex
 *            member_ifindex - the member ifindex
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : when the membe_ifindex become the trunk port, the all mep on this port
 *            should be deleted
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessTrunkAdd1stMember_Callback(UI32_T trunk_ifindex, UI32_T member_ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessTrunkAddMember_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the trunk member add call back
 * INPUT    : trunk_ifindex  - the trunk ifindex
 *            member_ifindex - the member ifindex
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : when the membe_ifindex become the trunk port, the all mep on this port
 *            should be deleted
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessTrunkAddMember_Callback(UI32_T trunk_ifindex, UI32_T member_ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessTrunkDeleteLastMember_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the trunk member remote last membercall back
 * INPUT    : trunk_ifindex  - the trunk ifindex
 *            member_ifindex - the member ifindex
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : when the trunk_ifindex has no trunk port, the all mep on this port
 *            should be deleted
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessTrunkDeleteLastMember_Callback(UI32_T trunk_ifindex, UI32_T member_ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessTrunkMemberDelete_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the trunk member remove member call back
 * INPUT    : trunk_ifindex  - the trunk ifindex
 *            member_ifindex - the member ifindex
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : when the trunk_ifindex has no trunk port, the all mep on this port
 *            should be deleted
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessTrunkMemberDelete_Callback(UI32_T trunk_ifindex, UI32_T member_ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessInterfaceStatusChange_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the interface status change call back
 * INPUT    : ifindex  - the port ifindex
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessInterfaceStatusChange_Callback(UI32_T ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessPortStatusChange_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the port status change call back
 * INPUT    : ifindex       - the port ifindex
 *            vid           - the vlan id
 *            is_forwarding - the xstp status is forarding or not
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessPortStatusChange_Callback(UI16_T vid, UI32_T ifindex, BOOL_T is_forwarding);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessPortEnterForwarding
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the port enter forwarding
 * INPUT    : xstp_id   - the spanning tree id
 *           ifindex  - the port ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     : for sys_callback_mgr, cfm_group will call this
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessPortEnterForwarding(UI32_T xstp_id, UI32_T ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessPortLeaveForwarding
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the port leave forwarding
 * INPUT    : xstp_id   - the spanning tree id
 *           ifindex  - the port ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     : for sys_callback_mgr, cfm_group will call this
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessPortLeaveForwarding(UI32_T xstp_id, UI32_T ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessPortAdminDisable_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the port admin disable
 * INPUT    : ifindex  - the port ifindex
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessPortAdminDisable_Callback(UI32_T ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessPortAdminDisable
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process the port admin disable
 * INPUT    : ifindex  - the port ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessPortAdminDisable(UI32_T ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessVlanCreate_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process vlan create
 * INPUT    : ifindex  - the port ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessVlanCreate_Callback(UI32_T vid_ifindex, UI32_T vlan_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessVlanDestory_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process vlan destory
 * INPUT    : ifindex  - the port ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessVlanDestory_Callback(UI32_T vid_ifindex,UI32_T vlan_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessVlanMemberAdd_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process vlan member add
 * INPUT    : ifindex  - the port ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessVlanMemberAdd_Callback( UI32_T vid_ifindex,UI32_T lport_ifindex,UI32_T vlan_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessVlanMemberDelete_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process vlan member delete
 * INPUT    : ifindex  - the port ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessVlanMemberDelete_Callback(UI32_T vid_ifindex,UI32_T lport_ifindex,UI32_T vlan_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_ProcessCpuMacDelete_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : This function process vlan create
 * INPUT    : ifindex  - the port ifindex
 * OUTPUT   : None
 * RETURN   :None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_MGR_ProcessCpuMacDelete_Callback(UI16_T vid);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_HandleHotInsertion
 * ------------------------------------------------------------------------
 * PURPOSE: This function will initialize the port_p OM of the module ports
 *          when the option module is inserted.
 * INPUT  : in_starting_port_ifindex -- the ifindex of the first module port_p
 *                                      inserted
 *          in_number_of_port        -- the number of ports on the inserted
 *                                      module
 *          in_use_default           -- the flag indicating the default
 *                                      configuration is used without further
 *                                      provision applied; TRUE if a new module
 *                                      different from the original one is
 *                                      inserted
 * OUTPUT : None
 * RETURN : None
 * NOTE   : Only one module is inserted at a time.
 * ------------------------------------------------------------------------
 */
void CFM_MGR_HandleHotInsertion(
    UI32_T  in_starting_port_ifindex,
    UI32_T  in_number_of_port,
    BOOL_T  in_use_default);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_HandleHotRemoval
 * ------------------------------------------------------------------------
 * PURPOSE: This function will clear the port_p OM of the module ports when
 *          the option module is removed.
 * INPUT  : in_starting_port_ifindex -- the ifindex of the first module port_p
 *                                      removed
 *          in_number_of_port        -- the number of ports on the removed
 *                                      module
 * OUTPUT : None
 * RETURN : None
 * NOTE   : Only one module is removed at a time.
 * ------------------------------------------------------------------------
 */
void CFM_MGR_HandleHotRemoval(
    UI32_T  in_starting_port_ifindex,
    UI32_T  in_number_of_port);

 /*------------------------------------------------------------------------------
  * ROUTINE NAME : SNMP_MGR_HandleIPCReqMsg
  *------------------------------------------------------------------------------
  * PURPOSE:
  *    Handle the ipc request message for cfm mgr.
  * INPUT:
  *    msg_p         --  the request ipc message buffer
  *    ipcmsgq_p     --  The handle of ipc message queue. The response message
  *                      will be sent through this handle.
  *
  * OUTPUT:
  *    None.
  *
  * RETURN:
  *    Need respond
  * NOTES:
  *    None.
  *------------------------------------------------------------------------------
  */
BOOL_T CFM_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msg_p);

/* for delay measurement
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_AbortDelayMeasureByDMM
 *-------------------------------------------------------------------------
 * PURPOSE  : To abort the delay measure in progress.
 * INPUT    : src_mep_id  - the source mep_id
 *            dst_mep_id  - the destination mep_id
 *            dst_mac_ar  - the destination mac
 *            md_name_ar  - the md name array
 *            md_name_len - the md name length
 *            ma_name_ar  - the ma name array
 *            ma_name_len - the ma name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS - success
 *            CFM_TYPE_CONFIG_ERROR   - fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_AbortDelayMeasureByDMM(
    UI32_T      src_mep_id,
    UI32_T      dst_mep_id,
    UI8_T       dst_mac_ar[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T       md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1],
    UI32_T      md_name_len,
    UI8_T       ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1],
    UI32_T      ma_name_len);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_DoDelayMeasureByDMM
 *-------------------------------------------------------------------------
 * PURPOSE  : To do delay measure by sending DMM.
 * INPUT    : src_mep_id  - the source mep_id
 *            dst_mep_id  - the destination mep_id
 *            dst_mac_ar  - the destination mac
 *            md_name_ar  - the md name array
 *            md_name_len - the md name length
 *            ma_name_ar  - the ma name array
 *            ma_name_len - the ma name length
 *            counts      - the transmit counts
 *            interval    - the transmit interval
 *            timeout     - the timeout for waiting dmr
 *            pkt_size    - the transmit packet size
 *            pkt_pri     - the priority to transmit packet
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS - success
 *            CFM_TYPE_CONFIG_ERROR   - fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_DoDelayMeasureByDMM(
    UI32_T      src_mep_id,
    UI32_T      dst_mep_id,
    UI8_T       dst_mac_ar[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T       md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1],
    UI32_T      md_name_len,
    UI8_T       ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1],
    UI32_T      ma_name_len,
    UI32_T      counts,
    UI32_T      interval,
    UI32_T      timeout,
    UI32_T      pkt_size,
    UI16_T      pkt_pri);

/* end for delay measurement
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_AbortThrptMeasureByLBM
 *-------------------------------------------------------------------------
 * PURPOSE  : To abort the throughput measure in progressing.
 * INPUT    : src_mep_id  - the source mep_id
 *            dst_mep_id  - the destination mep_id
 *            dst_mac_ar  - the destination mac
 *            md_name_ar  - the md name array
 *            md_name_len - the md name length
 *            ma_name_ar  - the ma name array
 *            ma_name_len - the ma name length
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS - success
 *            CFM_TYPE_CONFIG_ERROR   - fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_AbortThrptMeasureByLBM(
    UI32_T      src_mep_id,
    UI32_T      dst_mep_id,
    UI8_T       dst_mac_ar[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T       md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1],
    UI32_T      md_name_len,
    UI8_T       ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1],
    UI32_T      ma_name_len);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_DoThrptMeasureByLBM
 *-------------------------------------------------------------------------
 * PURPOSE  : To do throughput measure by sending LBM.
 * INPUT    : src_mep_id  - the source mep_id
 *            dst_mep_id  - the destination mep_id
 *            dst_mac_ar  - the destination mac
 *            md_name_ar  - the md name array
 *            md_name_len - the md name length
 *            ma_name_ar  - the ma name array
 *            ma_name_len - the ma name length
 *            counts      - the transmit counts
 *            pkt_size    - the transmit packet size
 *            pattern     - the pattern included in data TLV
 *            pkt_pri     - the priority to transmit packet
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_CONFIG_SUCCESS - success
 *            CFM_TYPE_CONFIG_ERROR   - fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
CFM_TYPE_Config_T CFM_MGR_DoThrptMeasureByLBM(
    UI32_T  src_mep_id,
    UI32_T  dst_mep_id,
    UI8_T   dst_mac_ar[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T   md_name_ar[CFM_TYPE_MD_MAX_NAME_LENGTH+1],
    UI32_T  md_name_len,
    UI8_T   ma_name_ar[CFM_TYPE_MA_MAX_NAME_LENGTH+1],
    UI32_T  ma_name_len,
    UI32_T  counts,
    UI32_T  pkt_size,
    UI16_T  pattern,
    UI16_T  pkt_pri);

#endif /*end of #if (SYS_CPNT_CFM == TRUE)*/

#endif /* End of CFM_MGR_H */

