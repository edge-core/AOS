/*-----------------------------------------------------------------------------
 * MODULE NAME: AMTR_OM.H
 *-----------------------------------------------------------------------------
 * PURPOSE: To store and manage AMTR Hisam Table.
 *
 *
 * NOTES: For declarations which are only used by AMTR(such as
 *        AMTR_OM_Set..() , should move to amtr_om_private.h
 *
 * Modification History:
 *      Date                Modifier        Reason
 *      ------------------------------------
 *      03-15-2005    water_huang    create
 *
 * COPYRIGHT(C)         Accton Corporation, 2005
 *-----------------------------------------------------------------------------
 */

#ifndef _AMTR_OM_H
#define _AMTR_OM_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "amtr_type.h"
#include "amtr_mgr.h" /* for AMTR_MGR_PortInfo_T */
#include "sysrsc_mgr.h"


/* NAMING CONSTANT DECLARATIONS
 */
#define AMTR_OM_IPCMSG_TYPE_SIZE   sizeof(union AMTR_OM_IpcMsg_Type_U)

/* MACRO FUNCTION DECLARATIONS
 */
/* command used in IPC message
 */
enum
{
    AMTR_OM_IPC_GETPORTINFO
};

/* Macro function for computation of IPC msg_buf size based on field name
 * used in AMTR_OM_IpcMsg_T.data
 */
#define AMTR_OM_GET_MSG_SIZE(field_name)                      \
            (AMTR_OM_IPCMSG_TYPE_SIZE +                      \
            sizeof(((AMTR_OM_IpcMsg_T*)0)->data.field_name))


/* DATA TYPE DECLARATIONS
 */

/* IPC message structure
 */
typedef struct
{
    union AMTR_OM_IpcMsg_Type_U
    {
        UI32_T                     cmd;
        BOOL_T                     ret_bool;
    } type; /* the intended action or return value */

    union
    {
        struct
        {
            UI32_T              arg_ui32;
            AMTR_MGR_PortInfo_T arg_portinfo;
        } arg_grp_ui32_portinfo;
    } data; /* the argument(s) for the function corresponding to cmd */
}AMTR_OM_IpcMsg_T;

typedef struct
{
   UI32_T total_count;
   UI32_T static_count;
   UI32_T dynamic_count;
}AMTR_OM_CountersPerSystem_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamGetRecord
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will get next record from Hisam Table
 * INPUT:    UI8_T *key     -- search key
 * OUTPUT:   AMTR_TYPE_AddrEntry_T *hisam_entry     -- address entry
 * RETURN:   TRUE/FALSE
 * NOTES:
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_OM_HisamGetNextRecord(UI32_T kidx, UI8_T *key,  AMTR_TYPE_AddrEntry_T *hisam_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamSearch
 * -------------------------------------------------------------------------
 * PURPOSE: Search next element(record) from HISAM-structure
 * INPUT:   desc     -- HISAM descriptor to operation
 *          kidx     -- key index
 *          key      -- according the "key" to search the next element.
 *                      if element is NULL(0) then search from 1st element.
 *          call_back-- The callback function to 'view' the element.
 *                      The return value of call_back:
 *                         L_HISAM_SEARCH_BREAK: tell this function to break searching
 *                         L_HISAM_SEARCH_CONTINUE: tell this function continue searching
 *                         L_HISAM_SEARCH_SKIP: tell this function to skip counting
 *          count    -- limited element count to be searched
 * OUTPUT:  key -- return the key back so that the user can do furthur search from the next record
 * RETURN:  L_HISAM_SEARCH_BREAK: Searching is broken by (*fun)
 *          L_HISAM_SEARCH_END_OF_LIST: stop search because end of list
 *          L_HISAM_SEARCH_COMPLETED: stop search because reach the limited count
 *          L_HISAM_SEARCH_INVALID_KIDX: stop search because invalid key index
 * NOTE:    1. If caller assign count=0, this function will search all hisam table.
 *          2. If callbaack function don't need cookies, caller can set this argument to be 0.
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_OM_HisamSearch(UI32_T kidx, UI8_T *key,  UI32_T (*call_back) (void*rec, void *cookie),UI32_T count, void *cookie);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AMTR_OM_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for AMTR OM.
 *
 * INPUT   : msgbuf_p -- input request ipc message buffer
 *
 * OUTPUT  : msgbuf_p -- output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_OM_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_NABufferEnqueue
 *-------------------------------------------------------------------------
 * PURPOSE: This funciton will set  NA entries to NA queue
 * INPUT  : None
 * OUTPUT : UI8_T *addr_entry      -- NA
 * RETURN : TRUE / FALSE(queue empty)
 * NOTES  : This function is called at interrupt time, so it need to be fast.
 *          When NA queue is empty, this function return FALSE
 *-------------------------------------------------------------------------*/

BOOL_T AMTR_OM_NABufferEnqueue(UI32_T num_of_entries,AMTR_TYPE_AddrEntry_T addr_buf[]);
/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_NABufferDequeue
 *-------------------------------------------------------------------------
 * PURPOSE: This funciton will get an NA entry from NA queue
 * INPUT  : None
 * OUTPUT : UI8_T *addr_entry      -- NA
 * RETURN : TRUE / FALSE(queue empty)
 * NOTES  : This function is called at interrupt time, so it need to be fast.
 *          When NA queue is empty, this function return FALSE
 *-------------------------------------------------------------------------*/

BOOL_T AMTR_OM_NABufferDequeue(UI32_T* num_of_entries,AMTR_TYPE_AddrEntry_T addr_buf[]);
/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_NABufferReset
 *-------------------------------------------------------------------------
 * PURPOSE: This funciton will reset na buffer 
 * INPUT  : None
 * OUTPUT : None
 *-------------------------------------------------------------------------*/

void AMTR_OM_NABufferReset();

/*---------------------------------------------------------------------------------
 * FUNCTION : void AMTR_OM_InitiateSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE  : Initiate system resource
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------------*/
void AMTR_OM_InitiateSystemResources(void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_MGR_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource in the context of the calling process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void AMTR_OM_AttachSystemResources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_GetShMemInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the size for share memory
 * INPUT    : *segid_p
 *            *seglen_p
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void AMTR_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_OM_GetOperatingMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return AMTR csc operating mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : SYS_TYPE_STACKING_MASTER_MODE
 *           SYS_TYPE_STACKING_SLAVE_MODE
 *           SYS_TYPE_STACKING_TRANSITION_MODE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T AMTR_OM_GetOperatingMode(void);

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_GetPortInfo
 *------------------------------------------------------------------------------
 * Purpose  : This function get the port Infomation
 * INPUT    : ifindex
 * OUTPUT   : port_info(learning mode, life time, count, protocol)
 * RETURN   : BOOL_T        - True : successs, False : failed
 * NOTE     :
 *-----------------------------------------------------------------------------*/
BOOL_T  AMTR_OM_GetPortInfo(UI32_T ifindex, AMTR_MGR_PortInfo_T *port_info);

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_GetCountersPerSystem
 *------------------------------------------------------------------------
 * PURPOSE: This function will get per-system counters such as total dynamic
 *          mac counter and total static mac counter.
 * INPUT  : None
 * OUTPUT : counter_p
 * RETURN : None
 * NOTES  : Total counter means how many entries in the system.
 *------------------------------------------------------------------------*/
void AMTR_OM_GetCountersPerSystem(AMTR_OM_CountersPerSystem_T *counter_p);

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_GetTotalCounter
 *------------------------------------------------------------------------
 * PURPOSE: This function will get total counter.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : Value of the total counter
 * NOTES  : Total counter means how many entries in the system.
 *------------------------------------------------------------------------*/
UI32_T AMTR_OM_GetTotalCounter(void);

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_GetTotalDynamicCounter
 *------------------------------------------------------------------------
 * PURPOSE: This function will get total dynamic counter in the system
 * INPUT  : None
 * OUTPUT : None
 * RETURN : Value of the total dynamic counter
 * NOTES  : Total dynamic counter means how many dynamic entries in the
 *          system.
 *------------------------------------------------------------------------*/
UI32_T AMTR_OM_GetTotalDynamicCounter(void);

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_GetTotalStaticCounter
 *------------------------------------------------------------------------
 * PURPOSE: This function will get total static counter in the system.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : Value of the total static counter
 * NOTES  : Total static counter means how many static entries in the system.
 *------------------------------------------------------------------------*/
UI32_T AMTR_OM_GetTotalStaticCounter(void);

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_GetStaticCounterByPort
 *------------------------------------------------------------------------
 * PURPOSE: This function will get static counter of the specified port.
 * INPUT  : ifindex	- ifindex of the port to get the static counter
 * OUTPUT : None
 * RETURN : Value of the static counter of the specified port.
 * NOTES  : None
 *------------------------------------------------------------------------*/
UI32_T AMTR_OM_GetStaticCounterByPort(UI32_T ifindex);

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_GetDynCounterByPort
 *------------------------------------------------------------------------
 * PURPOSE: This function will get dynamic counter of the specified port.
 * INPUT  : ifindex	- ifindex of the port to get the dynamic counter
 * OUTPUT : None
 * RETURN : Value of the dynamic counter of the specified port
 * NOTES  : None
 *------------------------------------------------------------------------*/
UI32_T AMTR_OM_GetDynCounterByPort(UI32_T ifindex);

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_GetDynCounterByVid
 *------------------------------------------------------------------------
 * PURPOSE: This function will get the dynamic counter of the specified vid.
 * INPUT  : vid  -  vlan id
 * OUTPUT : None
 * RETURN : Value of the dynamic counter of the specified vlan id
 * NOTES  : None
 *------------------------------------------------------------------------*/
UI32_T AMTR_OM_GetDynCounterByVid(UI32_T vid);

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_GetLearntCounterByport
 *------------------------------------------------------------------------
 * PURPOSE: This function will get the learnt counter by the specified port.
 * INPUT  : ifindex	- ifindex of the port to get the learnt counter
 * OUTPUT : None
 * RETURN : Value of the learnt counter of the specified port
 * NOTES  : None
 *------------------------------------------------------------------------*/
UI32_T AMTR_OM_GetLearntCounterByport(UI32_T ifindex);

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_GetSecurityCounterByport
 *------------------------------------------------------------------------
 * PURPOSE: This function will get the security counter of the specified port.
 * INPUT  : ifindex	- ifindex of the port to get the security counter
 * OUTPUT : None
 * RETURN : Value of the security counter of the specified port
 * NOTES  : None
 *------------------------------------------------------------------------*/
UI32_T AMTR_OM_GetSecurityCounterByport(UI32_T ifindex);

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_GetConfigCounterByPort
 *------------------------------------------------------------------------
 * PURPOSE: This function will get the configuration addresses counter of the
 *          specified port.
 * INPUT  : ifindex	- ifindex of the port to get the configuration address
 *          counter
 * OUTPUT : None
 * RETURN : Value of the config counter of the specified port
 * NOTES  : None
 *------------------------------------------------------------------------*/
UI32_T AMTR_OM_GetConfigCounterByPort(UI32_T ifindex);

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_GetMacNotifyGlobalStatus
 *------------------------------------------------------------------------------
 * Purpose  : To get mac-notification-trap global status
 * INPUT    : None
 * OUTPUT   : *is_enabled_p
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_GetMacNotifyGlobalStatus(BOOL_T *is_enabled_p);

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_GetMacNotifyInterval
 *------------------------------------------------------------------------------
 * Purpose  : To get mac-notification-trap interval
 * INPUT    : None
 * OUTPUT   : interval_p - pointer to interval to get (in ticks)
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_GetMacNotifyInterval(UI32_T  *interval_p);

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_GetMacNotifyPortStatus
 *------------------------------------------------------------------------------
 * Purpose  : To get mac-notification-trap port status
 * INPUT    : ifidx        - lport ifindex
 * OUTPUT   : *is_enabled_p
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_GetMacNotifyPortStatus(UI32_T  ifidx, BOOL_T *is_enabled_p);

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_GetMacNotifyTimeStamp
 *------------------------------------------------------------------------------
 * Purpose  : To get mac-notification-trap time stamp 
 * INPUT    : None
 * OUTPUT   : time_stamp_p - pointer to time stamp to get
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_GetMacNotifyTimeStamp(UI32_T  *time_stamp_p);

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_GetVlanLearningStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get vlan learning status of specified vlan
 * INPUT    : vid
 * OUTPUT   : learning_p
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_GetVlanLearningStatus(UI32_T vid, BOOL_T *learning_p);

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_GetMlagMacNotifyPortStatus
 *------------------------------------------------------------------------------
 * Purpose  : To get MLAG mac notify port status
 * INPUT    : ifidx        - lport ifindex
 * OUTPUT   : *is_enabled_p
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_GetMlagMacNotifyPortStatus(UI32_T  ifidx, BOOL_T *is_enabled_p);

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_GetMlagMacNotifyTimeStamp
 *------------------------------------------------------------------------------
 * Purpose  : To get MLAG mac notify time stamp 
 * INPUT    : None
 * OUTPUT   : time_stamp_p - pointer to time stamp to get
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_GetMlagMacNotifyTimeStamp(UI32_T  *time_stamp_p);

#if (SYS_CPNT_OVSVTEP == TRUE)
/*------------------------------------------------------------------------------
 * Function : AMTR_OM_OvsGetMacNotifyTimeStamp
 *------------------------------------------------------------------------------
 * Purpose  : To get OVSVTEP mac notify time stamp
 * INPUT    : None
 * OUTPUT   : time_stamp_p - pointer to time stamp to get
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_OvsGetMacNotifyTimeStamp(UI32_T *time_stamp_p);
#endif

#endif /* #ifndef _AMTR_OM_H */
