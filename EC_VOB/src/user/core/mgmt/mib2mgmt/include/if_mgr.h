/* -------------------------------------------------------------------------
 * FILE NAME - IF_MGR.H
 * -------------------------------------------------------------------------
 * Purpose: This package provides the services to manage/support the RFC2863 MIB
 * Notes: 1. This package shall be a reusable package for all the L2/L3 switchs.
 *        2. The Interfaces group defined by RFC1213 MIB has been obsoleted by
 *           the new RFC2863. (RFC1213 -> RFC 1573 -> RFC 2233 -> RFC 2863 )
 *        3. The following deprecated objects defined in ifTable will not be supported:
 *
 *          ifInNUcastPkts          Counter32,  -- deprecated
 *          ifOutNUcastPkts         Counter32,  -- deprecated
 *          ifOutQLen               Gauge32,    -- deprecated
 *          ifSpecific              OBJECT IDENTIFIER -- deprecated
 *
 *        4. The Interface Test Table will not be supported by this device.
 *           This group of objects is optional.  However, a media-specific
 *           MIB may make implementation of this group mandatory.
 *
 *        5. The Interface Receive Address Table will not be supported in this
 *           package.
 *
 *
 *          Written by:     Nike Chen, first create
 *          Date:           07/15/01
 *
 * Modification History:
 *   By            Date      Ver.    Modification Description
 * ------------ ----------   -----   ---------------------------------------
 *   Amytu                           Comformance to RFC2863
 *               9-25-2002           EnterTransitionMode for stacking
 * Allen Cheng  12-19-2002   V2.0    Revised for the trap mechanism changed
 * -------------------------------------------------------------------------
 * Copyright(C)                              ACCTON Technology Corp., 2002
 * -------------------------------------------------------------------------
 */

#ifndef IF_MGR_H
#define IF_MGR_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysfun.h"
#include "leaf_2863.h"
#include "leaf_es3626a.h"

/* TYPE DECLARATIONS
 */
enum
{
    IF_MGR_IPCCMD_GETIFNUMBER,
    IF_MGR_IPCCMD_GETIFTABLELASTCHANGE,
    IF_MGR_IPCCMD_GETIFENTRY,
    IF_MGR_IPCCMD_GETNEXTIFENTRY,
    IF_MGR_IPCCMD_SETIFADMINSTATUS,
    IF_MGR_IPCCMD_GETIFXENTRY,
    IF_MGR_IPCCMD_GETNEXTIFXENTRY,
    IF_MGR_IPCCMD_SETIFLINKUPDOWNTRAPENABLEGLOBAL,
    IF_MGR_IPCCMD_SETIFLINKUPDOWNTRAPENABLE,
    IF_MGR_IPCCMD_GETRUNNINGIFLINKUPDOWNTRAPENABLE,
    IF_MGR_IPCCMD_SETIFALIAS,
    IF_MGR_IPCCMD_GETIFSTACKENTRY,
    IF_MGR_IPCCMD_GETNEXTIFSTACKENTRY,
    IF_MGR_IPCCMD_GETIFSTACKLASTCHANGE,
    IF_MGR_IPCCMD_IFNAMETOIFINDEX,
    IF_MGR_IPCCMD_GETIFTYPE,
    IF_MGR_IPCCMD_FOLLOWISASYNCHRONISMIPC
};

/* Define the interface entry type for RFC2863 Interfaces Group management.
 * Note: An interface entry containing management information applicable to a
 *       particular interface.
 */
#pragma pack(1)
typedef struct
{
    /* key - A unique value, greater than zero, to indentify a unique interface.
     */
    UI32_T  if_index;

    UI8_T   if_descr[MAXSIZE_ifDescr+1];
    UI32_T  if_descr_length;
    UI32_T  if_type;
    UI32_T  if_mtu;
    UI32_T  if_speed;
    UI32_T  if_admin_status;
    UI8_T   if_phys_address[6];
    UI32_T  if_oper_status;
    UI32_T  if_last_change;
    UI64_T  if_in_octets;
    UI64_T  if_in_ucast_pkts;
    UI64_T  if_in_nucast_pkts;  /* deprecated */
    UI64_T  if_in_discards;
    UI64_T  if_in_errors;
    UI64_T  if_in_unknown_protos;
    UI64_T  if_out_octets;
    UI64_T  if_out_ucast_pkts;
    UI64_T  if_out_nucast_pkts; /* deprecated */
    UI64_T  if_out_discards;
    UI64_T  if_out_errors;
    UI64_T  if_out_qlen;        /* deprecated */
    UI64_T  if_specific;         /* deprecated */

} IF_MGR_IfEntry_T;


/* Define the interface entry type for RFC2863 Extension Interfaces Group management.
 * Note: An interface entry contains additional management information applicable to a
 *       particular interface.
 */
typedef struct
{
    /* key - A unique value, greater than zero, to indentify a unique interface.
     */
    UI32_T  if_index;
    UI8_T   if_name[MAXSIZE_ifName+1];
    UI32_T  if_name_length;
    UI64_T  if_in_multicast_pkts;
    UI64_T  if_in_broadcast_pkts;
    UI64_T  if_out_multicast_pkts;
    UI64_T  if_out_broadcast_pkts;
    UI64_T  if_hc_in_octets;
    UI64_T  if_hc_in_ucast_pkts;
    UI64_T  if_hc_in_multicast_pkts;
    UI64_T  if_hc_in_broadcast_pkts;
    UI64_T  if_hc_out_octets;
    UI64_T  if_hc_out_ucast_pkts;
    UI64_T  if_hc_out_multicast_pkts;
    UI64_T  if_hc_out_broadcast_pkts;
    UI32_T  if_link_up_down_trap_enable;
    UI32_T  if_high_speed;
    BOOL_T  if_promiscuous_mode;
    BOOL_T  if_connector_present;
    UI8_T   if_alias[MAXSIZE_ifAlias+1];
    UI32_T  if_counter_discontinuity_time;

} IF_MGR_IfXEntry_T;

#pragma pack()


/* Define interface stack entry type for RFC2863 Interface Stack Group management.
 * Notes: An interface stack entry contains information on the relationships
 *        between the multiple sub-layers of network interfaces.
 *        In particular, it contains information on which sub-layers run
 *        'on top of' which other sub-layers, where each sub-layer corresponds
 *        to a conceptual row in the ifTable
 */
typedef struct
{
    /* primary key - to indetify a unique If stack entry.
     * Note: The value of ifIndex corresponding to the higher sub-layer
     *       of the relationship, i.e., the sub-layer which runs on 'top'
     *       of the sub-layer identified by the corresponding instance of
     *       ifStackLowerLayer.
     */
    UI32_T  if_stack_higher_layer;

    /* secondary key
     * Note: The value of ifIndex corresponding to the lower sub-layer
     *       of the relationship, i.e., the sub-layer which runs 'below'
     *       the sub-layer identified by the corresponding instance of
     *       ifStackHigherLayer.
     */
    UI32_T  if_stack_lower_layer;

    UI32_T  if_stack_status;                 /* RowStatus */

} IF_MGR_IfStackEntry_T;

/*use to the definition of IPC message buffer*/
typedef struct
{
    union
    {
        UI32_T cmd;          /*cmd fnction id*/
        BOOL_T result_bool;  /*respond bool return*/
        UI32_T result_ui32;  /*respond ui32 return*/
        UI32_T result_i32;  /*respond i32 return*/
    } type;

    union
    {
        BOOL_T bool_v;
        UI8_T  ui8_v;
        I8_T   i8_v;
        UI32_T ui32_v;
        UI16_T ui16_v;
        I32_T i32_v;
        I16_T i16_v;
        UI8_T ip4_v[4];

        IF_MGR_IfEntry_T if_entry;

        struct{
            UI32_T if_index;
            UI32_T if_admin_status;
        } index_adminstatus;

        IF_MGR_IfXEntry_T if_x_entry;

        struct{
            UI32_T if_x_index;
            UI32_T trap_status;
        } index_trapstatus;

        struct{
            UI32_T if_x_index;
            UI8_T if_alias[MAXSIZE_ifAlias+1];
        } index_alias;

        IF_MGR_IfStackEntry_T if_stack_entry;

        struct{
            UI8_T ifname[MAXSIZE_ifName+1];
            UI32_T ifindex;
        } index_name;

        struct{
            UI32_T ifindex;
            UI32_T iftype;
        } index_type;

    } data; /* contains the supplemntal data for the corresponding cmd */
} IF_MGR_IPCMsg_T;

#define IF_MGR_MSGBUF_TYPE_SIZE     sizeof(((IF_MGR_IPCMsg_T *)0)->type)

/* For Exceptional Handler */
enum IF_MGR_FUN_NO_E
{
    IF_MGR_GetIfEntry_Fun_No     =   1,
    IF_MGR_SetIfAdminStatus_Fun_No,
    IF_MGR_GetIfXEntry_Fun_No,
    IF_MGR_SetIfLinkUpDownTrapEnable_Fun_No,
    IF_MGR_SetIfAlias_Fun_No,
    IF_MGR_GetIfStackEntry_Fun_No,
    IF_MGR_IfnameToIfindex_Fun_No,
    IF_MGR_IfindexToIfname_Fun_No
};

typedef enum
{
    /* these types are for valid ifIndex values
     */
    IF_MGR_NORMAL_IFINDEX = 0,
    IF_MGR_TRUNK_IFINDEX,
    IF_MGR_RS232_IFINDEX,
    IF_MGR_LOOPBACK_IFINDEX,
    IF_MGR_VLAN_IFINDEX,
    IF_MGR_ERROR_IFINDEX

} IF_MGR_IFINDEX_TYPE_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME: IF_MGR_InitiateSystemResources
 * PURPOSE: To register callback function in VLAN_MGR.
 * INPUT:  none
 * OUTPUT: none
 * RETURN: none
 */
void IF_MGR_InitiateSystemResources(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - IF_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void IF_MGR_Create_InterCSC_Relation(void);

/* FUNCTION NAME: IF_MGR_Enter_Master_Mode
 * PURPOSE: Enable the IF_MGR activities as master mode
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none.
 * NOTE:
 */
void IF_MGR_EnterMasterMode (void);


/* FUNCTION NAME: IF_MGR_Enter_Slave_Mode
 * PURPOSE: Enable the IF_MGR activities as slave mode
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none.
 * NOTE:
 */
void IF_MGR_EnterSlaveMode (void);


/* FUNCTION NAME: IF_MGR_Enter_Transition_Mode
 * PURPOSE: Enable the IF_MGR activities as transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none.
 * NOTE:
 */
void IF_MGR_EnterTransitionMode (void);


/* FUNCTION NAME - IF_MGR_SetTransitionMode
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *
 */
void  IF_MGR_SetTransitionMode(void);

/* FUNCTION NAME - IF_MGR_GetOperationMode
 * PURPOSE  : This functions returns the current operation mode of this component
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *
 */
SYS_TYPE_Stacking_Mode_T  IF_MGR_GetOperationMode(void);

/* FUNCTION NAME - IF_MGR_ProvisionComplete
 * PURPOSE  : mib2mgmt_init will call this function when provision completed
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *
 */
void IF_MGR_ProvisionComplete(void);

/* FUNCTION NAME: IF_MGR_Register_IfOperStatusChanged_CallBack
 * PURPOSE: For NETCFG to register Vlan up/down callback function
 * INPUT:  callback function
 * OUTPUT: none
 * RETURN: TRUE/FALSE
 * NOTES:
 */
void IF_MGR_Register_IfOperStatusChanged_CallBack(void (*fun)(UI32_T if_index, UI32_T status));

/* FUNCTION NAME: IF_MGR_Register_VlanDestroy_CallBack
 * PURPOSE: For NETCFG to register Vlan destroyed callback function
 * INPUT:  callback function
 * OUTPUT: none
 * RETURN: none
 * NOTES:
 */
void IF_MGR_Register_VlanDestroy_CallBack(void (*fun)(UI32_T if_index, UI32_T vlan_status));

/* FUNCTION NAME: IF_MGR_GetIfNumber
 * PURPOSE: This funtion returns the number of network interfaces
 *          (regardless of their current state) present on this system.
 * INPUT:  None.
 * OUTPUT: if_number        - the total number of network interfaces presented on the system
 * RETURN: none
 * NOTES: For those interfaces which are not installed shall not be count into this number.
 */
BOOL_T IF_MGR_GetIfNumber(UI32_T *if_number);


/* FUNCTION NAME: IF_MGR_GetIfNumber
 * PURPOSE: This funtion returns the value of sysUpTime at the time of the
 *          last creation or deletion of an entry in the ifTable. If the number of
 *          entries has been unchanged since the last re-initialization
 *          of the local network management subsystem, then this object
 *          contains a zero value.
 * INPUT:  None.
 * OUTPUT: if_table_last_change_time
 * RETURN: TRUE/FALSE
 * NOTES: None.
 */
BOOL_T IF_MGR_GetIfTableLastChange (UI32_T *if_table_last_change_time);



/*  RFC2863 - Interface Table/Group
 *
 *  The following functions provide the services to manage the ifTable
 *  The Interfaces table contains information on the entity's interfaces.  Each sub-layer
 *  below the internetwork-layer of a network interface is considered to be an interface.
 *  Interface group contains generic information about the physical interfaces of the entity,
 *  including configuration info and statistics on the events occurring at each interface.
 *  Implementation of this group is mandatory for all system.
 *
 *   INDEX  { ifIndex }
 *   ::= { ifTable 1 }
 *
 *  IfEntry ::=
 *  SEQUENCE {
 *      ifIndex                 InterfaceIndex,
 *      ifDescr                 DisplayString,
 *      ifType                  IANAifType,
 *      ifMtu                   Integer32,
 *      ifSpeed                 Gauge32,
 *      ifPhysAddress           PhysAddress,
 *      ifAdminStatus           INTEGER,
 *      ifOperStatus            INTEGER,
 *      ifLastChange            TimeTicks,
 *      ifInOctets              Counter32,
 *      ifInUcastPkts           Counter32,
 *      ifInNUcastPkts          Counter32,  -- deprecated
 *      ifInDiscards            Counter32,
 *      ifInErrors              Counter32,
 *      ifInUnknownProtos       Counter32,
 *      ifOutOctets             Counter32,
 *      ifOutUcastPkts          Counter32,
 *      ifOutNUcastPkts         Counter32,  -- deprecated
 *      ifOutDiscards           Counter32,
 *      ifOutErrors             Counter32,
 *      ifOutQLen               Gauge32,    -- deprecated
 *      ifSpecific              OBJECT IDENTIFIER -- deprecated
 *  }
 */

/* FUNCTION NAME: IF_MGR_GetIfEntry
 * PURPOSE: This funtion returns true if the specified interface entry info
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:   if_entry->if_index      - key to specify a unique interface entry
 * OUTPUT:  if_entry                - interface entry info of specified if_index
 * RETURN:  TRUE/FALSE
 * NOTES:   None.
 */
BOOL_T IF_MGR_GetIfEntry (IF_MGR_IfEntry_T  *if_entry);


/* FUNCTION NAME: IF_MGR_GetNextIfEntry
 * PURPOSE: This funtion returns true if the next available interface entry info
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:   if_entry->if_index      - key to specify a unique interface entry
 * OUTPUT:  if_entry                - interface entry info of specified if_index
 * RETURN:  TRUE/FALSE
 * NOTES:   If next available interface entry is available, the if_entry->if_index
 *          will be updated and the entry info will be retrieved from the table.
 */
BOOL_T IF_MGR_GetNextIfEntry (IF_MGR_IfEntry_T  *if_entry);


/* FUNCTION NAME: IF_MGR_SetIfAdminStatus
 * PURPOSE: This funtion returns true if desired/new admin status is successfully set to
 *          the specified interface. Otherwise, false is returned.
 * INPUT:  if_index         - key to specify a unique interface
 *         if_admin_status  - the desired/new admin status for this interface
 * OUTPUT: None.
 * RETURN: TRUE/FALSE
 * NOTES: 1. The testing(3) state indicates that no operational packets can be passed.
 *        2. When a managed system initializes, all interfaces start with ifAdminStatus
 *           in the down(2) state.
 *        3. As a result of either explicit management action or per configuration
 *           information retained by the managed system, ifAdminStatus is then changed
 *           to either the up(1) or  testing(3) states (or remains in the down(2) state).
 */
BOOL_T IF_MGR_SetIfAdminStatus (UI32_T if_index, UI32_T if_admin_status);



/* RFC2863 - Extension Interface Table/Group, the Extension to the Interface Table/Group
 *
 * The following functions provide the services to manage the ifXTable.
 * This table contains additional objects for the interface table.
 *
 *  AUGMENTS    { ifEntry }
 *  ::= { ifXTable 1 }
 *
 *  IfXEntry ::=
 *  SEQUENCE {
 *      ifName                      DisplayString,
 *      ifInMulticastPkts           Counter32,
 *      ifInBroadcastPkts           Counter32,
 *      ifOutMulticastPkts          Counter32,
 *      ifOutBroadcastPkts          Counter32,
 *      ifHCInOctets                Counter64,
 *      ifHCInUcastPkts             Counter64,
 *      ifHCInMulticastPkts         Counter64,
 *      ifHCInBroadcastPkts         Counter64,
 *      ifHCOutOctets               Counter64,
 *      ifHCOutUcastPkts            Counter64,
 *      ifHCOutMulticastPkts        Counter64,
 *      ifHCOutBroadcastPkts        Counter64,
 *      ifLinkUpDownTrapEnable      INTEGER,
 *      ifHighSpeed                 Gauge32,
 *      ifPromiscuousMode           TruthValue,
 *      ifConnectorPresent          TruthValue,
 *      ifAlias                     DisplayString,
 *      ifCounterDiscontinuityTime  TimeStamp
 *  }
 */

/* FUNCTION NAME: IF_MGR_GetIfXEntry
 * PURPOSE: This funtion returns true if the specified extension interface entry info
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:   if_x_entry->if_index    - key to specify a unique extension interface entry
 * OUTPUT:  if_entry                - extension interface entry info of specified if_index
 * RETURN:  TRUE/FALSE
 * NOTES:   None.
 */
BOOL_T IF_MGR_GetIfXEntry (IF_MGR_IfXEntry_T  *if_x_entry);


/* FUNCTION NAME: IF_MGR_GetNextIfXEntry
 * PURPOSE: This funtion returns true if the next available extension interface entry info
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:   if_x_entry->if_index    - key to specify a unique extension interface entry
 * OUTPUT:  if_x_entry              - extension interface entry info of specified if_index
 * RETURN:  TRUE/FALSE
 * NOTES:   If next available extension interface entry is available, the if_x_entry->if_index
 *          will be updated and the entry info will be retrieved from the table.
 */
BOOL_T IF_MGR_GetNextIfXEntry (IF_MGR_IfXEntry_T  *if_x_entry);

/* ------------------------------------------------------------------------
 * FUNCTION NAME: IF_MGR_SetIfLinkUpDownTrapEnableGlobal
 * ------------------------------------------------------------------------
 * PURPOSE: Set trap status to all interfaces.
 * INPUT:   trap_status -- trap status
 * OUTPUT:  None
 * RETURN:  TRUE/FALSE
 * NOTES:   None
 * ------------------------------------------------------------------------
 */
BOOL_T IF_MGR_SetIfLinkUpDownTrapEnableGlobal(
    UI32_T trap_status
);

/*-------------------------------------------------------------------------
 * FUNCTION NAME: IF_MGR_SetIfLinkUpDownTrapEnable
 * ------------------------------------------------------------------------
 * PURPOSE: This funtion returns true if the trap status of the specific interface
 *          can be set successfully.  Otherwise, return FALSE.
 * INPUT:   if_x_index  -- key to specify which index to configured.
 *          trap_status -- trap status
 *                        VAL_ifLinkUpDownTrapEnable_enabled  (1)
 *                        VAL_ifLinkUpDownTrapEnable_disabled (2)
 * OUTPUT:  none
 * RETURN:  TRUE/FALSE
 * NOTES:   none
 * ------------------------------------------------------------------------
 */
BOOL_T IF_MGR_SetIfLinkUpDownTrapEnable(UI32_T if_x_index, UI32_T trap_status);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - IF_MGR_GetRunningIfLinkUpDownTrapEnable
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the global LinkUpDownTrapEnable status.
 * INPUT    :   if_x_index              -- the specified IfIndex
 * OUTPUT   :   UI32_T *trap_status     -- pointer of the status value
 *                      VAL_ifLinkUpDownTrapEnable_enabled  (1)
 *                      VAL_ifLinkUpDownTrapEnable_disabled (2)
 *
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  IF_MGR_GetRunningIfLinkUpDownTrapEnable(UI32_T if_x_index, UI32_T *trap_status);


/* FUNCTION NAME: IF_MGR_SetIfAlias
 * PURPOSE: This funtion returns true if the if_alias of the specific interface
 *          can be set successfully.  Otherwise, return FALSE.
 * INPUT:   if_x_index - key to specify which index to configured.
 *			if_alias - the read/write name of the specific interface
 * OUTPUT:  none
 * RETURN:  TRUE/FALSE
 * NOTES:   none
 */
BOOL_T IF_MGR_SetIfAlias(UI32_T if_x_index, UI8_T *if_alias);


/* RFC2863 - Interface Stack Table/Group
 *
 * The following functions provide the services to manage the ifStack Group.
 * The table containing information on the relationships between the multiple sub-layers of
 * network interfaces.
 * Each entry contains information on a particular relationship between two sub-layers,
 * specifying that one sub-layer runs on 'top' of the other sub-layer.
 * Each sub-layer corresponds to a conceptual row in the ifTable.
 *
 *  INDEX { ifStackHigherLayer, ifStackLowerLayer }
 *    ::= { ifStackTable 1 }
 *
 *  IfStackEntry ::=
 *  SEQUENCE {
 *      ifStackHigherLayer  InterfaceIndexOrZero,
 *      ifStackLowerLayer   InterfaceIndexOrZero,
 *      ifStackStatus       RowStatus
 *   }
 */

/* FUNCTION NAME: IF_MGR_GetIfStackEntry
 * PURPOSE: This funtion returns true if specified interface stack entry info
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:  if_stack_entry->if_stack_higher_layer_if_index   - primary key to specify
 *                                                          - a unique interface stack entry
 *         if_stack_entry->if_stack_lower_layer_if_index    - secondary key
 * OUTPUT: if_stack_entry                   - the specified interface stack entry info
 * RETURN: TRUE/FALSE
 * NOTES: 1. The write-access to this object will not be supported by this device.
 *        2. Changing the value of this object from 'active' to 'notInService' or 'destroy'
 *           will likely have consequences up and down the interface stack.
 *           Thus, write access to this object is likely to be inappropriate for some types of
 *           interfaces, and many implementations will choose not to support write-access for
 *           any type of interface.
 */
BOOL_T IF_MGR_GetIfStackEntry(IF_MGR_IfStackEntry_T *if_stack_entry);


/* FUNCTION NAME: IF_MGR_GetNextIfStackEntry
 * PURPOSE: This funtion returns true if next available interface stack entry info
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:  if_stack_entry->if_stack_higher_layer_if_index   - primary key to specify
 *                                                          - a unique interface stack entry
 *         if_stack_entry->if_stack_lower_layer_if_index    - secondary key
 * OUTPUT: if_stack_entry                   - next available interface stack entry info
 * RETURN: TRUE/FALSE
 * NOTES: 1. The write-access to this object will not be supported by this device.
 *        2. Changing the value of this object from 'active' to 'notInService' or 'destroy'
 *           will likely have consequences up and down the interface stack.
 *           Thus, write access to this object is likely to be inappropriate for some types of
 *           interfaces, and many implementations will choose not to support write-access for
 *           any type of interface.
 */
BOOL_T IF_MGR_GetNextIfStackEntry(IF_MGR_IfStackEntry_T *if_stack_entry);


/* FUNCTION NAME: IF_MGR_GetIfStackLastChange
 * PURPOSE: This funtion returns true if the last update time of specified interface stack
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:   none
 * OUTPUT:  if_stack_last_change_time - The value of sysUpTime at the time of the last change of
 *                                      the (whole) interface stack.
 * RETURN: TRUE/FALSE
 * NOTES:  A change of the interface stack is defined to be any creation, deletion, or change in
 *         value of any instance of ifStackStatus.  If the interface stack has been unchanged
 *         since the last re-initialization of the local network management subsystem, then
 *         this object contains a zero value.
 */
BOOL_T IF_MGR_GetIfStackLastChange (UI32_T *if_stack_last_change_time);


/* FUNCTION NAME: IF_MGR_IfnameToIfindex
 * PURPOSE: This function returns true if the given ifname has a corresponding ifindex existed
 *          in the system.  Otherwise, returns false.
 * INPUT:   ifname - a read-only name for each interface defined during intialization
 * OUTPUT:  ifindex - corresponding interface index for the specific name.
 * RETURN:  TRUE/FALSE
 * NOTES:   None
 */
BOOL_T IF_MGR_IfnameToIfindex (UI8_T *ifname, UI32_T *ifindex);


/* FUNCTION NAME: IF_MGR_IfindexToIfname
 * PURPOSE: This function returns true if the given ifindex has a corresponding ifname existed
 *          in the system.  Otherwise, returns false.
 * INPUT:   ifindex - interface index
 * OUTPUT:  ifname - corresponding name for the specific interface index
 * RETURN:  TRUE/FALSE
 * NOTES:   None
 */
BOOL_T IF_MGR_IfindexToIfname (UI32_T ifindex, UI8_T *ifname);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - IF_MGR_GetIfType
 *------------------------------------------------------------------------------
 * PURPOSE  : This function determines the type of interface based on ifindex
 *            value.
 * INPUT    : ifindex -- interface index
 * OUTPUT   : iftype -- type of interface based on ifindex
 *                      (IF_MGR_NORMAL_IFINDEX,
 *                       IF_MGR_TRUNK_IFINDEX,
 *                       IF_MGR_RS232_IFINDEX,
 *                       IF_MGR_VLAN_IFINDEX,
 *                       IF_MGR_ERROR_IFINDEX)
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *------------------------------------------------------------------------------
 */
BOOL_T  IF_MGR_GetIfType(UI32_T ifindex, UI32_T *iftype);

/*------------------------------------------------------------------------------
 * ROUTINE NAME :  IF_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for csca mgr.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  - There is a response need to send.
 *    FALSE - There is no response to send.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T   IF_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);
#endif /* End of IF_MGR_H */
