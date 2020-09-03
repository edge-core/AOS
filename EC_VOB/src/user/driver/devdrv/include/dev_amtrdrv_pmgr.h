#ifndef _DEV_AMTRDRV_PMGR_H_
#define _DEV_AMTRDRV_PMGR_H_

#include "sys_adpt.h"
#include "swdrv_type.h"
#include "dev_amtrdrv.h"
enum
{
    DEV_AMTRDRV_PMGR_RESP_RESULT_FAIL,
};

#define DEV_AMTRDRV_PMGR_REQ_CMD_SIZE       sizeof(((DEV_AMTRDRV_PMGR_IPCMSG_T *)0)->type.cmd)
#define DEV_AMTRDRV_PMGR_RESP_RESULT_SIZE   sizeof(((DEV_AMTRDRV_PMGR_IPCMSG_T *)0)->type)

typedef struct
{
    union
    {
        UI32_T cmd;          /*cmd fnction id*/
        BOOL_T result_bool;  /*respond bool return*/
        UI8_T result_ui8;
        UI32_T result_ui32;  /*respond ui32 return*/
        UI32_T result_i32;  /*respond i32 return*/
        DEV_AMTRDRV_Ret_T   result_dev_amtr_type;
    }type;

    union
    {
        union
        {
            struct
            {
                UI32_T value;
            }req;
            struct
            {
            }resp;
        }setageingtime;

        union
        {
            struct
            {
                UI32_T unit;
            }req;
            struct
            {
            }resp;
        }resettofirstaddrtblentry;

        union
        {
            struct
            {
                SWDRV_TYPE_L2_Address_Info_T addr_table_entry;
            }req;
            struct
            {
                SWDRV_TYPE_L2_Address_Info_T addr_table_entry;
            }resp;
        }getnextaddrtblentry;

        union
        {
            struct
            {
                UI32_T number_of_address;
            }req;
            struct
            {
                SWDRV_TYPE_L2_Address_Info_T address_table[SYS_ADPT_AMTR_NBR_OF_ADDR_TO_SYNC_IN_ONE_SCAN];
                UI32_T actual_number_of_address_return;
            }resp;
        }getnextnaddrtblentry;

        union
        {
            struct
            {
                SWDRV_TYPE_L2_Address_Info_T address;
            }req;
            struct
            {
            }resp;
        }setaddrtblentry;

        union
        {
            struct
            {
                UI32_T  vid;
                UI8_T   mac[6];
            }req;
            struct
            {
            }resp;
        }deleteaddr;

        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }deletealldynamicaddr;

        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }deletealladdr;

        union
        {
            struct
            {
                UI32_T  vid;
                UI8_T   mac[6];
                UI8_T   deviceGroup;
                UI32_T  mode;
            }req;
            struct
            {
            }resp;
        }setinterventionentry;

        union
        {
            struct
            {
                UI32_T  vid;
                UI8_T   mac[6];
            }req;
            struct
            {
            }resp;
        }deleteinterventionentry;

        union
        {
            struct
            {
                UI32_T  vid;
                UI8_T   vlan_mac[6];
            }req;
            struct
            {
            }resp;
        }addvlanmac;

        union
        {
            struct
            {
                UI32_T  vid;
                UI8_T   vlan_mac[6];
            }req;
            struct
            {
            }resp;
        }deletevlanmac;

        union
        {
            struct
            {
                UI32_T  vid;
                UI8_T   mac[6];
            }req;
            struct
            {
            }resp;
        }createmulticastaddrtblentry;

        union
        {
            struct
            {
                UI32_T  vid;
                UI8_T   mac[6];
            }req;
            struct
            {
            }resp;
        }destroymulticastaddrtblentry;

        union
        {
            struct
            {
                UI32_T vid;
                UI8_T  mac[6];
                UI32_T unit;
                UI32_T port;
                BOOL_T tagged;
            }req;
            struct
            {
            }resp;
        }addmulticastportmember;

        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
                UI32_T vid;
                UI8_T  mac[6];
            }req;
            struct
            {
            }resp;
        }deletemulticastportmember;

        union
        {
            struct
            {
                UI32_T vid;
                UI8_T  mac[6];
                UI8_T port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
                UI8_T untagged_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
            }req;
            struct
            {
            }resp;
        }setmulticastportlist;

        union
        {
            struct
            {
                UI32_T vid;
                UI8_T  mac[6];
            }req;
            struct
            {
                UI32_T port;
                UI32_T trunk_id;
                BOOL_T is_trunk;
            }resp;
        }arllookup;

        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }showvalidnullmac;

        union
        {
            struct
            {
                SWDRV_TYPE_L2_Address_Info_T l2_entry;
                UI32_T pri;
                BOOL_T is_drop;
            }req;
            struct
            {
            }resp;
        }setmacentrywithattribute;

        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
                UI32_T trunk_id;
            }req;
            struct
            {
            }resp;
        }deleteaddrbyport;

        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
                UI32_T trunk_id;
                UI32_T vlan_id;
            }req;
            struct
            {
            }resp;
        }deleteaddrbyportandvlan;

        union
        {
            struct
            {
                UI32_T vid;
                UI8_T  mac[6];

            }req;
            struct
            {
                UI32_T unit;
                UI32_T port;
                UI32_T trunk_id;
                BOOL_T is_trunk;
            }resp;
        }arlfirstchiplookup;

        union
        {
            struct
            {
                UI8_T mac[6];
                UI32_T vid;
                UI32_T unit;
                UI32_T port[SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK];
                UI32_T trunk_member_count_in_localunit;
            }req;
            struct
            {
                UI8_T   hitbit_value;
            }resp;
        }readandclearlocalhitbit;

        union
        {
            struct
            {
                UI8_T deviceGroup;
                SWDRV_TYPE_L2_Address_Info_T address;
            }req;
            struct
            {
            }resp;
        }setaddrtblentrybydevice;

        union
        {
            struct
            {
                UI8_T deviceGroup;
                UI32_T vid;
                UI8_T  mac[6];
            }req;
            struct
            {
            }resp;
        }deleteaddrbydevice;

        union
        {
            struct
            {
              UI32_T index;
            }req;
            struct
            {
            }resp;
        }disablehardwarelearning;

        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }setmcastentryforinternalpacket;

        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
                UI32_T mode;
            }req;
            struct
            {
            }resp;
        }setinferfaceconfig;

#if (SYS_CPNT_HASH_LOOKUP_DEPTH_CONFIGURABLE == TRUE)
        union
        {
            struct
            {
            }req;
            struct
            {
                UI32_T lookup_depth;
            }resp;
        }gethashlookupdepth;
#endif
        
    }data;
}DEV_AMTRDRV_PMGR_IPCMSG_T;

enum
{
    DEV_AMTRDRV_IPCCMD_SETAGEINGTIME,
    DEV_AMTRDRV_IPCCMD_RESETTOFIRSTADDRTBLENTRY,
    DEV_AMTRDRV_IPCCMD_GETNEXTADDRTBLENTRY,
    DEV_AMTRDRV_IPCCMD_GETNEXTNADDRTBLENTRY,
    DEV_AMTRDRV_IPCCMD_GETNEWADDR,
    DEV_AMTRDRV_IPCCMD_REGISTER_NA_CALLBACK,
    DEV_AMTRDRV_IPCCMD_SETADDRTBLENTRY,
    DEV_AMTRDRV_IPCCMD_DELETEADDR,
    DEV_AMTRDRV_IPCCMD_DELETEALLDYNAMICADDR,
    DEV_AMTRDRV_IPCCMD_DELETEALLADDR,
    DEV_AMTRDRV_IPCCMD_SETINTERVENTIONENTRY,
    DEV_AMTRDRV_IPCCMD_DELETEINTERVENTIONENTRY,
    DEV_AMTRDRV_IPCCMD_ADDVLANMAC,
    DEV_AMTRDRV_IPCCMD_DELETEVLANMAC,
    DEV_AMTRDRV_IPCCMD_CREATEMULTICASTADDRTBLENTRY,
    DEV_AMTRDRV_IPCCMD_DESTROYMULTICASTADDRTBLENTRY,
    DEV_AMTRDRV_IPCCMD_ADDMULTICASTPORTMEMBER,
    DEV_AMTRDRV_IPCCMD_DELETEMULTICASTPORTMEMBER,
    DEV_AMTRDRV_IPCCMD_SETMULTICASTPORTLIST,
    DEV_AMTRDRV_IPCCMD_ARLLOOKUP,
    DEV_AMTRDRV_IPCCMD_SHOWVALIDNULLMAC,
    DEV_AMTRDRV_IPCCMD_SETMACENTRYWITHATTRIBUTE,
    DEV_AMTRDRV_IPCCMD_DELETEADDRBYPORT,
    DEV_AMTRDRV_IPCCMD_DELETEDYNAMICADDRBYPORT,
    DEV_AMTRDRV_IPCCMD_DELETEDYNAMICADDRBYVLAN,
    DEV_AMTRDRV_IPCCMD_DELETEDYNAMICADDRBYPORTVLAN,
    DEV_AMTRDRV_IPCCMD_ARLFIRSTCHIPLOOKUP,
    DEV_AMTRDRV_IPCCMD_READANDCLEARLOCALHITBIT,
    DEV_AMTRDRV_IPCCMD_SETADDRTBLENTRYBYDEVICE,
    DEV_AMTRDRV_IPCCMD_DELETEADDRBYDEVICE,
    DEV_AMTRDRV_IPCCMD_DISABLEHARDWARELEARNING,
    DEV_AMTRDRV_IPCCMD_DISABCPU,
    DEV_AMTRDRV_IPCCMD_SETINTERFACECONFIG,
    DEV_AMTRDRV_IPCCMD_SETMCASTENTRYFORINTERNALPACKET,
#if (SYS_CPNT_HASH_LOOKUP_DEPTH_CONFIGURABLE == TRUE)
    DEV_AMTRDRV_IPCCMD_GETHASHLOOKUPDEPTH,
#endif    
};

void DEV_AMTRDRV_PMGR_InitiateProcessResource(void) ;

BOOL_T DEV_AMTRDRV_PMGR_SetAgeingTime(UI32_T value) ;

BOOL_T DEV_AMTRDRV_PMGR_ResetToFirstAddrTblEntry(UI32_T unit) ;

UI32_T DEV_AMTRDRV_PMGR_GetNextAddrTblEntry(SWDRV_TYPE_L2_Address_Info_T *addr_table_entry) ;

UI32_T  DEV_AMTRDRV_PMGR_GetNextNAddrTblEntry(SWDRV_TYPE_L2_Address_Info_T *address_table,
                                        UI32_T number_of_address,
                                        UI32_T *actual_number_of_address_return) ;
#if 0
UI32_T DEV_AMTRDRV_PMGR_GetNewAddr(void CallBackFun (UI32_T vid,
    										    UI8_T  *mac,
    										    UI32_T unit,
    										    UI32_T port,
    										    UI32_T trunk_id,
    										    BOOL_T is_trunk,
    										    BOOL_T is_aged),
   						      UI32_T limit_number,
   						      BOOL_T *hw_arl_sync_required) ;

void DEV_AMTRDRV_PMGR_Register_NA_CallBack(void CallBackFun (UI32_T vid,
                                                    UI8_T  *mac,
                                                    UI32_T unit,
                                                    UI32_T port,
                                                    UI32_T trunk_id,
                                                    BOOL_T is_trunk,
                                                    UI32_T NA_type))   /* <01242002> */ ;
#endif

BOOL_T DEV_AMTRDRV_PMGR_SetAddrTblEntry(SWDRV_TYPE_L2_Address_Info_T *address_p) ;

BOOL_T DEV_AMTRDRV_PMGR_DeleteAddr(UI32_T vid, UI8_T *mac) ;

BOOL_T  DEV_AMTRDRV_PMGR_DeleteAllDynamicAddr(void) ;

BOOL_T  DEV_AMTRDRV_PMGR_DeleteAllAddr(void) ;

DEV_AMTRDRV_Ret_T  DEV_AMTRDRV_PMGR_SetInterventionEntry(UI32_T vid, UI8_T *mac,UI8_T deviceGroup, UI32_T mode);

BOOL_T DEV_AMTRDRV_PMGR_DeleteInterventionEntry(UI32_T vid, UI8_T *mac) ;

BOOL_T DEV_AMTRDRV_PMGR_AddVlanMac(UI32_T vid, UI8_T *vlan_mac) ;

BOOL_T DEV_AMTRDRV_PMGR_DeleteVlanMac(UI32_T vid, UI8_T *vlan_mac) ;

BOOL_T DEV_AMTRDRV_PMGR_CreateMulticastAddrTblEntry(UI32_T vid, UI8_T *mac) ;

BOOL_T DEV_AMTRDRV_PMGR_DestroyMulticastAddrTblEntry(UI32_T vid, UI8_T* mac) ;

BOOL_T DEV_AMTRDRV_PMGR_AddMulticastPortMember(UI32_T vid, UI8_T *mac, UI32_T unit, UI32_T port, BOOL_T tagged) ;

BOOL_T DEV_AMTRDRV_PMGR_DeleteMulticastPortMember(UI32_T unit, UI32_T port, UI32_T vid, UI8_T *mac) ;

BOOL_T  DEV_AMTRDRV_PMGR_SetMulticastPortList(UI32_T vid, UI8_T *mac, UI8_T *port_list, UI8_T *untagged_port_list) ;

BOOL_T  DEV_AMTRDRV_PMGR_ARLLookUp(UI32_T vid,
                              UI8_T *mac,
                              UI32_T *port,
                              UI32_T *trunk_id,
                              BOOL_T *is_trunk) ;

void DEV_AMTRDRV_PMGR_ShowValidNullMac() ;

BOOL_T DEV_AMTRDRV_PMGR_SetMacEntryWithAttribute(SWDRV_TYPE_L2_Address_Info_T *l2_entry_p,  UI32_T pri, BOOL_T is_drop) ;

BOOL_T DEV_AMTRDRV_PMGR_DeleteAddrByPort(UI32_T unit,UI32_T port,UI32_T trunk_id) ;

BOOL_T DEV_AMTRDRV_PMGR_DeleteDynamicAddrByPort(UI32_T unit,UI32_T port,UI32_T trunk_id);
BOOL_T DEV_AMTRDRV_PMGR_DeleteDynamicAddrByPortAndVlan(UI32_T unit,UI32_T port,UI32_T trunk_id,UI32_T vlan_id);
BOOL_T DEV_AMTRDRV_PMGR_DeleteDynamicAddrByVlan(UI32_T unit,UI32_T vlan_id);
BOOL_T  DEV_AMTRDRV_PMGR_ARLFirstChipLookUp(UI32_T vid,
                                       UI8_T  *mac,
                                       UI32_T *unit,
                                       UI32_T *port,
                                       UI32_T *trunk_id,
                                       BOOL_T *is_trunk) ;

BOOL_T  DEV_AMTRDRV_PMGR_ReadAndClearLocalHitBit(UI8_T* hitbit_value,UI8_T*  mac, UI32_T vid, UI32_T unit, UI32_T* port, UI32_T trunk_member_count_in_localunit) ;

DEV_AMTRDRV_Ret_T DEV_AMTRDRV_PMGR_SetAddrTblEntryByDevice (UI8_T deviceGroup,SWDRV_TYPE_L2_Address_Info_T *address_p) ;

BOOL_T DEV_AMTRDRV_PMGR_DeleteAddrByDevice(UI8_T deviceGroup,UI32_T vid, UI8_T *mac) ;

BOOL_T DEV_AMTRDRV_PMGR_DisableHardwareLearning() ;
BOOL_T DEV_AMTRDRV_PMGR_Disable(UI32_T index);
BOOL_T DEV_AMTRDRV_PMGR_SetInterfaceConfig(UI32_T unit, UI32_T port, UI32_T mode);
void DEV_AMTRDRV_PMGR_SetMCastEntryForInternalPacket(void) ;

#if (SYS_CPNT_HASH_LOOKUP_DEPTH_CONFIGURABLE == TRUE)
BOOL_T DEV_AMTRDRV_PMGR_GetHashLookupDepth(UI32_T *lookup_depth_p);
#endif

#endif
