/*-----------------------------------------------------------------------------
 * FILE NAME: NETCFG_OM_ARP.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2008/01/18     --- Lin.Li, Create
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */
#ifndef NETCFG_OM_ARP_H
#define NETCFG_OM_ARP_H

#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "netcfg_type.h"


/* NAMING CONSTANT DECLARATIONS
 */
#define NETCFG_OM_ARP_MSGBUF_TYPE_SIZE sizeof(union NETCFG_OM_ARP_IPCMsg_Type_U)

enum
{
    NETCFG_OM_ARP_IPC_GETTIMEOUT
};


/* Macro function for computation of IPC msg_buf size based on field name
 * used in NETCFG_OM_ARP_IPCMsg_T.data
 */
#define NETCFG_OM_ARP_GET_MSG_SIZE(field_name)                       \
            (NETCFG_OM_ARP_MSGBUF_TYPE_SIZE +                        \
            sizeof(((NETCFG_OM_ARP_IPCMsg_T*)0)->data.field_name))

typedef struct
{
    union NETCFG_OM_ARP_IPCMsg_Type_U
    {
        UI32_T cmd;            /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
        BOOL_T result_bool; /*respond bool return*/
        UI32_T result_ui32; /*respond ui32 return*/ 
    } type;

    union
    {
        BOOL_T        bool_v;
        UI32_T        ui32_v;    
    } data;
}NETCFG_OM_ARP_IPCMsg_T;

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_OM_ARP_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for ARP OM.
 *
 * INPUT   : ipcmsg_p -- input request ipc message buffer
 *
 * OUTPUT  : ipcmsg_p -- output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_ARP_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

/* FUNCTION NAME : NETCFG_OM_ARP_Init
 * PURPOSE:create semaphore
 *
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void NETCFG_OM_ARP_Init(void);

/* for removal warning */
/* FUNCTION NAME : NETCFG_OM_ARP_GetStaticEntry
 * PURPOSE:
 *      Get a static arp entry.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (entry.ip_net_to_media_if_index, entry.ip_net_to_media_net_address).
 */
BOOL_T NETCFG_OM_ARP_GetStaticEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry);

/* FUNCTION NAME : NETCFG_OM_ARP_GetNextStaticEntry
 * PURPOSE:
 *      Get next available static arp entry.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (entry.ip_net_to_media_if_index, entry.ip_net_to_media_net_address).
 *      If key is (0, 0), get first one.
 */
BOOL_T NETCFG_OM_ARP_GetNextStaticEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry);

/* FUNCTION NAME : NETCFG_OM_ARP_GetNextStaticEntryByAddress
 * PURPOSE:
 *      Get next available static arp entry by IP address.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (entry.ip_net_to_media_net_address).
 *      If key is 0, get first one.
 */
BOOL_T NETCFG_OM_ARP_GetNextStaticEntryByAddress(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry);

/* FUNCTION NAME : NETCFG_OM_ARP_DeleteAllStaticEntry
 * PURPOSE:
 *          Remove all static arp entries.
 *
 * INPUT:  None.
 *
 * OUTPUT: None.
 *
 * RETURN: None.
 *
 * NOTES:  None.
 */
void   NETCFG_OM_ARP_DeleteAllStaticEntry(void);

/* FUNCTION NAME : NETCFG_OM_ARP_AddStaticEntry
 * PURPOSE:
 *      Add an static ARP entry.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (entry.ip_net_to_media_if_index, entry.ip_net_to_media_net_address).
 */
BOOL_T NETCFG_OM_ARP_AddStaticEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry);

/* FUNCTION NAME : NETCFG_OM_ARP_SetTimeout
 * PURPOSE:
 *      Set arp age timeout.
 *
 * INPUT:
 *      age_time
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 */
BOOL_T NETCFG_OM_ARP_SetTimeout(UI32_T age_time);

/* FUNCTION NAME : NETCFG_OM_ARP_DeleteStaticEntry
 * PURPOSE:
 *      Remove a static arp entry.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (entry.ip_net_to_media_if_index, entry.ip_net_to_media_net_address).
 */
BOOL_T NETCFG_OM_ARP_DeleteStaticEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry);

/* FUNCTION NAME : NETCFG_OM_ARP_GetTimeout
 * PURPOSE:
 *      Get arp age timeout.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      age_time.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 */
BOOL_T NETCFG_OM_ARP_GetTimeout(UI32_T *age_time);
#endif /*NETCFG_OM_ARP_H*/

