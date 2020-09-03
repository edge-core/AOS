/* Module Name: _DHCP_MGR.H
 * Purpose:
 *      DHCP_MGR provides all DHCP accessing functions, include Provision Start/Complete,
 *      Restart. All task access-protections are kept in this module.
 *
 * Notes:
 *      1. CLI command includes :
 *          Enter Master/Slave/Transition mode.
 *          ProvisionComplete
 *          Bind DHCP to interface.
 *          Set Client ip, gateway_ip, server_ip
 *          Set role of interface, Client, Server, or Relay Agent.
 *      2. Execution function includes :
 *          Restart dhcp processing.
 *          Processing received packet
 *          Processing timeout request.
 *
 *
 * History:
 *       Date       -- Modifier,  Reason
 *  0.1 2001.12.25  --  William, Created
 *  0.3 2003.09.18  --  Jamescyl, Enhanced DHCP Relay Server APIs
 *
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001.
 */



#ifndef     _DHCP_MGR_H
#define     _DHCP_MGR_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "leaf_es3626a.h"
#include "l_mm.h"
#include "sysfun.h"
#include "l_inet.h"
#include "dhcp_type.h"

/* NAME CONSTANT DECLARATIONS
 */

#define DHCP_MGR_IPCMSG_TYPE_SIZE   sizeof(union DHCP_MGR_IpcMsg_Type_U)

#define DHCLIENT_CRAFT_PID_FILE "/var/run/dhclient_craft.pid" /* the pid file while dhclient is triggered by AOS for craft port */

/* command used in IPC message
 */
enum
{
	DHCP_MGR_IPC_DISABLEDHCP,
	DHCP_MGR_IPC_CHECKRESTARTOBJ,
	DHCP_MGR_IPC_RESTART3,
	DHCP_MGR_IPC_REMOVESYSTEMROLE,
	DHCP_MGR_IPC_RESTART,
	DHCP_MGR_IPC_SETCLIENTID,
	DHCP_MGR_IPC_GETCLIENTID,
	DHCP_MGR_IPC_GETRUNNINGCLIENTID,
	DHCP_MGR_IPC_GETNEXTCLIENTID,
	DHCP_MGR_IPC_DELETECLIENTID,
	DHCP_MGR_IPC_SETIFROLE,
	DHCP_MGR_IPC_C_SETVENDORCLASSID,
	DHCP_MGR_IPC_C_DELETEVENDORCLASSID,
	DHCP_MGR_IPC_C_GETVENDORCLASSID,
	DHCP_MGR_IPC_C_GETRUNNINGVENDORCLASSID,
	DHCP_MGR_IPC_C_GETNEXTVENDORCLASSID,
	DHCP_MGR_IPC_C_RELEASE_CLIENT_LEASE,
	DHCP_MGR_IPC_S_CLEARALLIPBINDING,
	DHCP_MGR_IPC_S_CLEARIPBINDING,
	DHCP_MGR_IPC_S_SETEXCLUDEDIP,
	DHCP_MGR_IPC_S_DELEXCLUDEDIP,
	DHCP_MGR_IPC_S_ENTERDHCPPOOL,
	DHCP_MGR_IPC_S_DELDHCPPOOL,
	DHCP_MGR_IPC_S_SETHOSTTOPOOLCONFIG,
	DHCP_MGR_IPC_S_DELHOSTFROMPOOLCONFIG,
	DHCP_MGR_IPC_S_SETMACTOPOOLCONFIG,
	DHCP_MGR_IPC_S_SETMACTYPETOPOOLCONFIG,
	DHCP_MGR_IPC_S_DELMACFROMPOOLCONFIG,
	DHCP_MGR_IPC_S_SETNETWORKTOPOOLCONFIG,
	DHCP_MGR_IPC_S_DELNETWORKFROMPOOLCONFIG,
	DHCP_MGR_IPC_S_SETDOMAINNAMETOPOOLCONFIG,
	DHCP_MGR_IPC_S_DELDOMAINNAMEFROMPOOLCONFIG,
	DHCP_MGR_IPC_S_SETDNSSERVERTOPOOLCONFIG,
	DHCP_MGR_IPC_S_DELDNSSERVERFROMPOOLCONFIG,
	DHCP_MGR_IPC_S_SETLEASETIMETOPOOLCONFIG,
	DHCP_MGR_IPC_S_DELLEASETIMEFROMPOOLCONFIG,
	DHCP_MGR_IPC_S_SETDEFAULTROUTETOPOOLCONFIG,
	DHCP_MGR_IPC_S_DELDEFAULTROUTEFROMPOOLCONFIG,
	DHCP_MGR_IPC_S_SETBOOTFILETOPOOLCONFIG,
	DHCP_MGR_IPC_S_DELBOOTFILEFROMPOOLCONFIG,
	DHCP_MGR_IPC_S_SETNETBIOSNAMESERVERTOPOOLCONFIG,
	DHCP_MGR_IPC_S_DELNETBIOSNAMESERVERFROMPOOLCONFIG,
	DHCP_MGR_IPC_S_SETNETBIOSNODETYPETOPOOLCONFIG,
	DHCP_MGR_IPC_S_DELNETBIOSNODETYPEFROMPOOLCONFIG,
	DHCP_MGR_IPC_S_SETNEXTSERVERTOPOOLCONFIG,
	DHCP_MGR_IPC_S_DELNEXTSERVERFROMPOOLCONFIG,
	DHCP_MGR_IPC_S_SETCLIENTIDENTIFIERTOPOOLCONFIG,
	DHCP_MGR_IPC_S_DELCLIENTIDENTIFIERFROMPOOLCONFIG,
	DHCP_MGR_IPC_S_GETDHCPPOOLTABLE,
	DHCP_MGR_IPC_S_GETNEXTDHCPPOOLTABLE,
	DHCP_MGR_IPC_S_GETDHCPPOOLOPTIONTABLE,
	DHCP_MGR_IPC_S_GETNEXTDHCPPOOLOPTIONTABLE,
	DHCP_MGR_IPC_S_GETDHCPPOOLDNSSERVERTABLE,
	DHCP_MGR_IPC_S_GETNEXTDHCPPOOLDNSSERVERTABLE,
	DHCP_MGR_IPC_S_GETDHCPPOOLDEFAULTROUTERTABLE,
	DHCP_MGR_IPC_S_GETNEXTDHCPPOOLDEFAULTROUTERTABLE,
	DHCP_MGR_IPC_S_GETDHCPPOOLNETBIOSSERVERTABLE,
	DHCP_MGR_IPC_S_GETNEXTDHCPPOOLNETBIOSSERVERTABLE,
	DHCP_MGR_IPC_S_GETDHCPSERVEREXCLUDEDIPTABLE,
	DHCP_MGR_IPC_S_GETNEXTDHCPSERVEREXCLUDEDIPTABLE,
	DHCP_MGR_IPC_S_GETIPBINDING,
	DHCP_MGR_IPC_S_GETACTIVEIPBINDING,
	DHCP_MGR_IPC_S_GETNEXTIPBINDING,
	DHCP_MGR_IPC_S_GETNEXTACTIVEIPBINDING,
	DHCP_MGR_IPC_S_SETDHCPPOOLSTATUS,
	DHCP_MGR_IPC_S_SETDHCPPOOLOPTDNSSERIP,
	DHCP_MGR_IPC_S_SETDHCPPOOLOPTDEFAULTROUTERIP,
	DHCP_MGR_IPC_S_SETDHCPPOOLOPTNETBIOSSERVERIP,
	DHCP_MGR_IPC_S_SETDHCPPOOLOPTCIDBUF,
	DHCP_MGR_IPC_S_SETDHCPPOOLOPTCIDMODE,
	DHCP_MGR_IPC_S_SETDHCPPOOLPOOLTYPE,
	DHCP_MGR_IPC_S_SETDHCPPOOLADDR,
	DHCP_MGR_IPC_S_SETDHCPPOOLSUBNETMASK,
	DHCP_MGR_IPC_GETDHCPSERVERSERVICESTATUS,
	DHCP_MGR_IPC_R_ADDRELAYSERVERADDRESS,
	DHCP_MGR_IPC_R_DELRELAYSERVERADDRESS,
	DHCP_MGR_IPC_R_DELALLRELAYSERVERADDRESS,
	DHCP_MGR_IPC_R_GETIFRELAYSERVERADDRESS,
	DHCP_MGR_IPC_R_GETRELAYSERVERADDRESS,
	DHCP_MGR_IPC_R_GETNEXTRELAYSERVERADDRESS,
	DHCP_MGR_IPC_R_SETRELAYSERVERADDRESS,
	DHCP_MGR_IPC_SETRESTARTOBJECT,
	DHCP_MGR_IPC_DELETEIFROLE,
#if(SYS_CPNT_DHCP_SERVER == TRUE)
    DHCP_MGR_IPC_S_GETNEXTPOOLCONFIG,
    DHCP_MGR_IPC_S_GETNEXTACTIVEPOOL,
#endif
	
#if(SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
    DHCP_MGR_IPC_RELAY_OPTION82_SET_OPTION82_STATUS,
    DHCP_MGR_IPC_RELAY_OPTION82_SET_OPTION82_POLICY,
    DHCP_MGR_IPC_RELAY_OPTION82_SET_OPTION82_RID_MODE,
    DHCP_MGR_IPC_RELAY_OPTION82_SET_OPTION82_RID_VALUE,
    DHCP_MGR_IPC_RELAY_OPTION82_SET_OPTION82_SUBTYPE_FORMAT,
    DHCP_MGR_IPC_RELAY_OPTION82_SET_RELAY_SERVER,
    DHCP_MGR_IPC_RELAY_OPTION82_SET_RELAY_SERVER_FROM_SNMP,
    DHCP_MGR_IPC_RELAY_OPTION82_GET_RELAY_SERVER_FROM_SNMP,
    DHCP_MGR_IPC_RELAY_OPTION82_DELETE_RELAY_SERVER,
    DHCP_MGR_IPC_RELAY_OPTION82_GET_RUNNING_STATUS,
    DHCP_MGR_IPC_RELAY_OPTION82_GET_RUNNING_POLICY,
    DHCP_MGR_IPC_RELAY_OPTION82_GET_RUNNING_RID_MODE,
    DHCP_MGR_IPC_RELAY_OPTION82_GET_RUNNING_FORMAT,
    DHCP_MGR_IPC_RELAY_OPTION82_GET_RUNNING_RELAY_SERVER,
#endif
	
	
};

/*  Function returned value */
#define DHCP_MGR_OK                     				0
#define DHCP_MGR_NO_SUCH_INTERFACE      				0x80000001
#define DHCP_MGR_INVALID_IP             				0x80000002
#define DHCP_MGR_INVALID_UDP_PORT       				0x80000003
#define DHCP_MGR_NO_SUCH_POOL_NAME						0x80000004
#define DHCP_MGR_SERVER_ON								0x80000005
#define DHCP_MGR_NO_IP									0x80000006
#define DHCP_MGR_DYNAMIC_IP								0x80000007
#define DHCP_MGR_FAIL									0x80000008
#define DHCP_MGR_EXCEED_MAX_POOL_NAME   				0x80000009
#define DHCP_MGR_IS_NETWORK_IP_OR_NETWORK_BROADCAST		0x8000000a
#define DHCP_MGR_EXCLUDE_ALL_IP_IN_SUBNET				0x8000000b
#define DHCP_MGR_HIGH_IP_SMALLER_THAN_LOW_IP			0x8000000c
#define DHCP_MGR_EXCLUDED_IP_IN_OTHER_SUBNET			0x8000000d
#define DHCP_MGR_POOL_HAS_CONFIG_TO_OTHERS				0x8000000e
#define DHCP_MGR_MAC_OVERSIZE							0x8000000f
#define DHCP_MGR_EXCEED_MAX_SIZE						0x80000010
#define DHCP_MGR_INVALID_ARGUMENT						0x80000011
#define DHCP_MGR_RELAY_ON								0x80000012
#define DHCP_MGR_CID_EXCEED_MAX_SIZE					0x80000013
#define DHCP_MGR_NO_CID									0x80000014
#define DHCP_MGR_SHUT_DOWN_RELAY						0x80000015
#define DHCP_MGR_EXCEED_CAPACITY                        0x80000016
#define DHCP_MGR_NETWORK_EXIST                          0x80000017
#define DHCP_MGR_INVALID_MASK                           0x80000018
#if (SYS_CPNT_DHCP_HOSTNAME == TRUE)
#define DHCP_MGR_HOSTNAME_EXCEED_MAX_SIZE				0x80000019      /* 2006-07, Joseph */
#endif
#define DHCP_MGR_CLASSID_EXCEED_MAX_SIZE				0x8000001a      /* 2007-12, Joseph */
#define DHCP_MGR_INVALID_MAC_ADDRESS                    0x8000001b

/* EXTERN VARIABLE DECLARATIONS */
extern BOOL_T dhcp_server_debug_flag;

/* MACRO FUNCTION DECLARATIONS
 */
#define DHCP_Serverprintf(format,args...) (!dhcp_server_debug_flag)?:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)

/* Macro function for computation of IPC msg_buf size based on field name
 * used in DHCP_MGR_IpcMsg_T.data
 */
#define DHCP_MGR_GET_MSG_SIZE(field_name)                       \
            (DHCP_MGR_IPCMSG_TYPE_SIZE +                        \
            sizeof(((DHCP_MGR_IpcMsg_T*)0)->data.field_name))


/* DATA TYPE DECLARATIONS
 */
#define DHCP_MGR_HTYPE_NONE				0
#define DHCP_MGR_HTYPE_ETHER			1
#define DHCP_MGR_HTYPE_IEEE802			6
#define DHCP_MGR_HTYPE_FDDI				8

#define DHCP_MGR_RESTART_NONE		BIT_0
#define DHCP_MGR_RESTART_CLIENT		BIT_1
#define DHCP_MGR_RESTART_SERVER		BIT_2
#define DHCP_MGR_RESTART_RELAY		BIT_3

#define	DHCP_MGR_BIND_CLIENT		BIT_1
#define	DHCP_MGR_BIND_SERVER		BIT_2
#define	DHCP_MGR_BIND_RELAY			BIT_3

#define	DHCP_MGR_CLIENT_UP		    BIT_0
#define	DHCP_MGR_CLIENT_DOWN		BIT_1


#define DHCP_MGR_NETBIOS_NODE_TYPE_NONE        	0
#define DHCP_MGR_NETBIOS_NODE_TYPE_B_NODE      	1
#define DHCP_MGR_NETBIOS_NODE_TYPE_P_NODE      	2
#define DHCP_MGR_NETBIOS_NODE_TYPE_M_NODE      	4
#define DHCP_MGR_NETBIOS_NODE_TYPE_H_NODE      	8

#define DHCP_MGR_INFINITE_LEASE_TIME           	MAX_TIME 

/* client identifier node */
#define DHCP_MGR_CID_HEX			VAL_dhcpPoolOptionCidMode_hex
#define DHCP_MGR_CID_TEXT			VAL_dhcpPoolOptionCidMode_text
#define DHCP_MGR_CID_BUF_MAX_SIZE	MAXSIZE_dhcpcIfClientId
#define DHCP_MGR_MAX_POOL_NAME_LEN			SYS_ADPT_DHCP_MAX_POOL_NAME_LEN
/* begin 2007-12, Joseph */
#define DHCP_MGR_CLASSID_BUF_MAX_SIZE	MAXSIZE_dhcpcIfVendorClassId        
#define DHCP_MGR_CLASSID_TEXT			VAL_dhcpcIfVendorClassIdMode_text       
#define DHCP_MGR_CLASSID_HEX			VAL_dhcpcIfVendorClassIdMode_hex       
/* end 2007-12 */

/* DHCP Server Pool Type */
#define DHCP_MGR_POOL_NONE		VAL_dhcpPoolPoolType_notSpecify
#define DHCP_MGR_POOL_NETWORK	VAL_dhcpPoolPoolType_netWork
#define DHCP_MGR_POOL_HOST		VAL_dhcpPoolPoolType_host


/* 2002-7-16: Penny 1st created for cid issue (JBOS) */
typedef struct DHCP_MGR_ClientId_S
{
	UI32_T id_mode;							 	/* DHCP_MGR_CID_HEX | DHCP_MGR_CID_TEXT */
	UI32_T id_len;								/* len of buffer */
	char id_buf[DHCP_MGR_CID_BUF_MAX_SIZE+1];	/* content of CID */
}DHCP_MGR_ClientId_T;

/* begin 2006-07, Joseph */
#if (SYS_CPNT_DHCP_HOSTNAME == TRUE)
typedef struct DHCP_MGR_HostName_S
{
	UI32_T host_len;								/* len of buffer */
	char  host_buf[SYS_ADPT_DHCP_MAX_CLIENT_HOSTNAME_LEN+1];   	/* content of HostName */
}DHCP_MGR_HostName_T;
#endif
/* end 2006-07 */

/* begin 2007-04, Joseph */  /* begin 2007-12, Joseph */
typedef struct DHCP_MGR_Vendor_S 
{
    UI32_T vendor_mode;                                     /* DHCP_MGR_CLASSID_HEX | DHCP_MGR_CLASSID_TEXT */
	UI32_T vendor_len;								        /* len of buffer */
	char  vendor_buf[DHCP_MGR_CLASSID_BUF_MAX_SIZE+1];   	/* content of Vendor */
}DHCP_MGR_Vendor_T;
/* end 2007-04, 2007-12 */ 

typedef struct DHCP_MGR_Server_Lease_Config_S
{
	struct DHCP_MGR_Server_Lease_Config_S *next;
	UI32_T   lease_ip;
	UI8_T    hardware_address[SYS_ADPT_MAC_ADDR_LEN+1];
	//UI8_T    client_hostname[SYS_ADPT_DHCP_MAX_CLIENT_HOSTNAME_LEN];
	UI32_T 	 lease_time; /* in sec */
	UI32_T	 start_time; /* in sec: show current system real time */
}DHCP_MGR_Server_Lease_Config_T;



#if (SYS_CPNT_DHCP_SERVER == TRUE)
typedef struct DHCP_MGR_ActivePoolRange_S
{
	UI32_T	low_address;
    UI32_T	high_address;
}DHCP_MGR_ActivePoolRange_T;
#endif

/* IPC message structure
 */
typedef struct
{
	union DHCP_MGR_IpcMsg_Type_U
	{
		UI32_T cmd;
		UI32_T ret_ui32;
		BOOL_T ret_bool;
        SYS_TYPE_Get_Running_Cfg_T result_running_cfg;  /* respond get running config return */
	} type; /* the intended action or return value */

	union
	{
        UI32_T arg_ui32;
        struct
        {
            UI32_T arg_ui32_1;
            UI32_T arg_ui32_2;
            UI32_T arg_ui32_3;
            char  arg_cid[MAXSIZE_dhcpcIfClientId+1];
        } arg_grp_ui32_ui32_ui32_cid;

        struct
        {
            UI32_T              arg_ui32;
            DHCP_MGR_ClientId_T arg_client_id;
        } arg_grp_ui32_clientid;

        struct
        {
            UI32_T      vid;
            DHCP_MGR_Vendor_T   vendordata;
        }arg_grp_vendor_class_id;
        
        struct
        {
            UI32_T arg_ui32_1;
            UI32_T arg_ui32_2;
        } arg_grp_ui32_ui32;
		
		struct
		{
			char pool_name[SYS_ADPT_DHCP_MAX_POOL_NAME_LEN+1];
		}arg_grp_pool_name;
		
		struct
		{
			char  domain_name[SYS_ADPT_DHCP_MAX_DOMAIN_NAME_LEN+1];
			char pool_name[SYS_ADPT_DHCP_MAX_POOL_NAME_LEN+1];
		}arg_grp_pool_domain_name;
		
		struct
		{	
			UI32_T arg_ui32_1;
            UI32_T arg_ui32_2;
            UI32_T arg_ui32_3;
			char pool_name[SYS_ADPT_DHCP_MAX_POOL_NAME_LEN+1];
		}arg_grp_pool_config;
		
		struct
		{
			char	bootfile[SYS_ADPT_DHCP_MAX_BOOTFILE_NAME_LEN+1];						
			char pool_name[SYS_ADPT_DHCP_MAX_POOL_NAME_LEN+1];	
		}arg_grp_pool_boot_file;
		
        struct
		{	
			UI32_T size;
			union
			{
				UI32_T   default_router[SYS_ADPT_MAX_NBR_OF_DHCP_DEFAULT_ROUTER];
				UI32_T   dns_server[SYS_ADPT_MAX_NBR_OF_DHCP_DNS_SERVER];
				UI32_T   netbios_name_server[SYS_ADPT_MAX_NBR_OF_DHCP_NETBIOS_NAME_SERVER];
			}ip_array;
			char pool_name[SYS_ADPT_DHCP_MAX_POOL_NAME_LEN+1];
		}arg_grp_pool_config_iparray;
		
		struct
		{
            UI32_T type;
            UI8_T  phy_addr[SYS_ADPT_MAC_ADDR_LEN];
			char pool_name[SYS_ADPT_DHCP_MAX_POOL_NAME_LEN+1];	
		}arg_grp_pool_config_phy_addr;

		struct
        {
            UI32_T arg_ui32_1;
            UI32_T arg_ui32_2;
            char  arg_cid[MAXSIZE_dhcpcIfClientId+1];
			char   pool_name[SYS_ADPT_DHCP_MAX_POOL_NAME_LEN+1];	
        } arg_grp_ui32_ui32_cid_pool;

		
		struct
		{
			DHCP_TYPE_PoolConfigEntry_T pool_config;
			char   pool_name[SYS_ADPT_DHCP_MAX_POOL_NAME_LEN+1];	
		} arg_grp_pool_config_entry;
		
        struct
		{
			DHCP_TYPE_ServerOptions_T pool_option;
			char   pool_name[SYS_ADPT_DHCP_MAX_POOL_NAME_LEN+1];	
		} arg_grp_pool_option;

		struct
		{
			UI32_T ipaddr;
			DHCP_MGR_Server_Lease_Config_T lease_entry;
		}arg_grp_lease_entry;

		struct 
		{
			UI32_T ifindex;
			UI32_T num;
			UI32_T ip_array[SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER];
		}arg_grp_relay_server;

		struct
		{
			UI32_T arg_ui32_1;
            UI32_T arg_ui32_2;
            UI32_T arg_ui32_3;
		}arg_grp_ui32_ui32_ui32;
#if (SYS_CPNT_DHCP_SERVER == TRUE)
        struct 
        {
            DHCP_MGR_ActivePoolRange_T     pool_range;
            char   pool_name[SYS_ADPT_DHCP_MAX_POOL_NAME_LEN+1];
        }arg_active_pool_range;
#endif       

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
        struct 
        {
            UI32_T arg_relay_server[SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER];
        }arg_grp_option82_relay_server;

        struct
        {
            UI8_T arg_rid_value[SYS_ADPT_MAX_LENGTH_OF_CID + 1];
        }arg_grp_option82_rid_value;

        struct
        {   
            BOOL_T subtype_format;
        }arg_grp_option82_subtype_format;

        struct
        {
            UI32_T index;
            UI32_T server_ip;
        }arg_grp_option82_relay_server_snmp;
#endif
	} data; /* the argument(s) for the function corresponding to cmd */
} DHCP_MGR_IpcMsg_T;



/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME : DHCP_MGR_Init
 * PURPOSE:
 *      Initialize DHCP_MGR used system resource, eg. protection semaphore.
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
void    DHCP_MGR_Init (void);

/* FUNCTION	NAME : DHCP_MGR_GetOperationMode
 * PURPOSE:
 *		Get current dhcp operation mode (master / slave / transition).
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		operation_mode -- DHCP_TYPE_STACKING_TRANSITION_MODE | DHCP_TYPE_STACKING_MASTER_MODE |
 *						  DHCP_TYPE_SYSTEM_STATE_SLAVE | DHCP_TYPE_SYSTEM_STATE_PROVISION_COMPLETE.
 *
 * NOTES:
 *		None.
 */
UI32_T DHCP_MGR_GetOperationMode(void);

/* FUNCTION	NAME : DHCP_MGR_SetTransitionMode
 * PURPOSE:
 *		Set transition mode.
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		None.
 */
void DHCP_MGR_SetTransitionMode(void);

/* FUNCTION NAME : DHCP_MGR_EnterMasterMode
 * PURPOSE:
 *      Enter master mode.
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
void    DHCP_MGR_EnterMasterMode (void);


/* FUNCTION NAME : DHCP_MGR_EnterTransitionMode
 * PURPOSE:
 *      Enter transition mode.
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
void    DHCP_MGR_EnterTransitionMode (void);


/* FUNCTION NAME : DHCP_MGR_EnterSlaveMode
 * PURPOSE:
 *      Enter slave mode.
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
void    DHCP_MGR_EnterSlaveMode (void);

/* FUNCTION NAME : DHCP_MGR_Create_InterCSC_Relation
 * PURPOSE:
 *      This function initializes all function pointer registration operations.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 */
void DHCP_MGR_Create_InterCSC_Relation(void);

/* FUNCTION	NAME : DHCP_MGR_CheckSystemState
 * PURPOSE:
 *		To check current system state [master | slave | transition | provision c]
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		(DHCP_TYPE_SYSTEM_STATE_T) dhcp_mgr_lcb.system_state
 *
 * NOTES:
 *		None.
 */
int DHCP_MGR_CheckSystemState(void);


/* FUNCTION NAME : DHCP_MGR_ProvisionComplete3
 * PURPOSE:
 *      Be signaled by STKCTRL, CLI had provision all configuration commands, all
 *      CSC should begin do his job.
 *
 * INPUT:
 *      restart_object.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. ProvisionComplete, task should begins his job. The job is not implememted in
 *         MGR, TASK takes the responsibility.
 *      2. This function is called by STKCTRL.
 *		3. Possible Value for restart_object:
 *			DHCP_MGR_RESTART_NONE
 *			DHCP_MGR_RESTART_CLIENT
 *			DHCP_MGR_RESTART_SERVER
 *			DHCP_MGR_RESTART_RELAY
 */
void    DHCP_MGR_ProvisionComplete3(UI32_T restart_object);



/* FUNCTION NAME : DHCP_MGR_Register_ProvisionComplete_Callback
 * PURPOSE:
 *      Register provision complete function of DHCP_TASK. Each time, STK_CTRL signaling
 *      system, MGR should signal TASK.
 *
 * INPUT:
 *      function    -- handling routine of callback function.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. Before provision, each CSC will enter transition mode, then enter master mode,
 *         begin provision.
 *      2. This function is for DHCP_TASK_SystemStateChanged_Callback.
 */
void    DHCP_MGR_Register_ProvisionComplete_Callback (void *function);



/* FUNCTION NAME : DHCP_MGR_Register_ChangeMode_Callback
 * PURPOSE:
 *      Register ChangeMode function of DHCP_TASK. Each time, CLI change the mode,
 *		DHCP_MGR will signal DHCP_TASK.
 *
 * INPUT:
 *      function    -- handling routine of callback function.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. DHCP_MGR will keep this information is WA, OM, and TASK at the same time.
 void    DHCP_MGR_Register_ChangeMode_Callback (void *function);
 */


/* FUNCTION NAME : DHCP_MGR_ReactiveProcessing
 * PURPOSE:
 *      Restart function to starting stage, discovery all interfaces and build
 *      working data structures.
 *
 * INPUT:
 *      restart_object - the specific object need to restart
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. This function performs 3 functions :
 *         a. Clear DHCP_OM (not include DHCP_WA) and DHCP_ALGO.
 *         b. Read configuration and lease from DHCP_WA to DHCP_OM.
 *         b. Discover Interface
 *         c. Construct DHCP_OM/protocols link list.
 *		2. Possible Value for restart_object:
 *			DHCP_TYPE_RESTART_NONE
 *			DHCP_TYPE_RESTART_CLIENT
 *			DHCP_TYPE_RESTART_SERVER
 *			DHCP_TYPE_RESTART_RELAY
 */
void    DHCP_MGR_ReactiveProcessing(UI32_T restart_object);

/* FUNCTION NAME : DHCP_MGR_Restart3
 * PURPOSE:
 *      The funtion provided to CLI or web to only change the state
 *		(change to provision complete) and inform DHCP task to do
 *		 the restart.
 *
 *
 * INPUT:
 *      restart_object.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES: 1. it will not immediately restart when called this funtion
 *			instead, it will activate restart in DHCP_task when get
 *			provision complete state
 *		  2. Possible Value for restart_object:
 *			DHCP_MGR_RESTART_NONE
 *			DHCP_MGR_RESTART_CLIENT
 *			DHCP_MGR_RESTART_SERVER
 *			DHCP_MGR_RESTART_RELAY
 *
 */
void    DHCP_MGR_Restart3(UI32_T restart_object);

/* FUNCTION NAME : DHCP_MGR_Restart
 * PURPOSE:
 *      The funtion provided to CLI or web to only change the state
 *		(change to provision complete) and inform DHCP task to do
 *		 the restart.
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES: 1. it will not immediately restart when called this funtion
 *			instead, it will activate restart in DHCP_task when get
 *			provision complete state
 *		  2. Possible Value for restart_object:
 *			DHCP_TYPE_RESTART_NONE
 *			DHCP_TYPE_RESTART_CLIENT
 *			DHCP_TYPE_RESTART_SERVER
 *			DHCP_TYPE_RESTART_RELAY
 *
 */
void    DHCP_MGR_Restart();

/* FUNCTION	NAME : DHCP_MGR_Dispatch
 * PURPOSE:
 *		To receive packet thru socket under semaphore protection
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		None.
 */
void DHCP_MGR_Dispatch();

/* FUNCTION NAME : DHCP_MGR_do_packet
 * PURPOSE:
 *      convert received DHCP packet to do_packet() acceptable parameters and
 *      call do_packet() to take some proper processing.
 *
 * INPUT:
 *      mem_ref         --  holder of received packet buffer.
 *      packet_length   --  received packet length.
 *      rxRifNum            --  the RIF packet coming,
 *                          -1 if not a configured interface.
 *      dst_mac         --  the destination hardware mac address in frame.
 *      src_mac         --  the source hardware mac address in frame.
 *      vid             --  the vlan the packet coming from.
 *      src_lport_ifIndex-- the ingress physical port ifIndex.
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
void DHCP_MGR_do_packet(
                L_MM_Mref_Handle_T *mref_handle_p,
                UI32_T packet_length,
                UI32_T rxRifNum,
                UI8_T *dst_mac,
                UI8_T *src_mac,
                UI32_T vid,
                UI32_T src_lport_ifIndex);


/* FUNCTION NAME : DHCP_MGR_CheckTimeout
 * PURPOSE:
 *      Call DHCP_TIME_IsTimeout to check whether a timeout request occurs,
 *      if yes, call the handle-function. Process all timeout request.
 *
 * INPUT:
 *      time_ticks -- system ticks from system started.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. This api is for protection purpose, prevent DHCP_TASK call to lower function.
 */
void DHCP_MGR_CheckTimeout (UI32_T time_ticks);


/*-----------------------------------
 *  CLI provision command function.
 *-----------------------------------
 *  1. all configuration commands are saved in DHCP_WA,
 *     not set to DHCP_OM.
 *  2. Interface Bind-checking is done in provision-command.
 *     eg. CLIENT, SERVER can not be coexisted on same interface.
 */

/* FUNCTION	NAME : DHCP_MGR_C_SetClientId
 * PURPOSE:
 *		Define associated client identifier per interface.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		id_mode 	-- the client-port of dhcp.
 *		id_len 		-- the len of buffer.
 *		id_buf		-- the content of the cid.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	-- No more space in allocating memory
 *		DHCP_MGR_CID_EXCEED_MAX_SIZE -- cid exceeds the max size
 *
 * NOTES:
 *		1. Possible values of id_mode are {DHCP_TYPE_CID_HEX | DHCP_TYPE_CID_TEXT}.
 *
 */
UI32_T DHCP_MGR_C_SetClientId(UI32_T vid_ifIndex, UI32_T id_mode, UI32_T id_len, char *id_buf);

/* FUNCTION	NAME : DHCP_MGR_C_GetClientId
 * PURPOSE:
 *		Retrieve the associated client identifier for specified interface from OM.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		cid_p		-- the pointer of cid structure
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_MGR_OK	  -- successfully.
 *		DHCP_MGR_FAIL -- failure.
 *		DHCP_MGR_NO_SUCH_INTERFACE -- the specified vlan not existed in DHCP_OM table
 *		DHCP_MGR_NO_CID	-- this interface has no CID configuration.
 *
 * NOTES:
 *		None.
 *
 */
UI32_T DHCP_MGR_C_GetClientId(UI32_T vid_ifIndex, DHCP_MGR_ClientId_T *cid_p);

/* FUNCTION	NAME : DHCP_MGR_C_GetRunningClientId
 * PURPOSE:
 *		Get running config for CID in WA.
 *
 * INPUT:
 *		vid_ifIndex -- the key to identify ClientId
 *
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *
 * NOTES:
 *
 *
 */
UI32_T DHCP_MGR_C_GetRunningClientId(UI32_T vid_ifIndex, DHCP_MGR_ClientId_T *cid_p);

/* FUNCTION	NAME : DHCP_MGR_C_GetNextClientId
 * PURPOSE:
 *		Get the next client identifier for specified interface from OM.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_MGR_OK	  -- successfully.
 *		DHCP_MGR_FAIL -- failure.
 *		DHCP_MGR_NO_SUCH_INTERFACE -- the specified vlan not existed in DHCP_OM table
 *		DHCP_MGR_NO_CID	-- this interface has no CID configuration.
 *
 * NOTES:
 *		1. (vid_ifIndex = 0) to get 1st interface, for layer 2, get the mgmt vlan id
 *
 */
UI32_T DHCP_MGR_C_GetNextClientId(UI32_T *vid_ifIndex, DHCP_MGR_ClientId_T *cid_p);

/* FUNCTION	NAME : DHCP_MGR_C_DeleteClientId
 * PURPOSE:
 *		Delete the client identifier for specified interface. (Restart DHCP needed)
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_MGR_OK	  -- successfully.
 *		DHCP_MGR_FAIL -- the specified interface has no cid config, so can't delete cid
 *
 *
 * NOTES:
 *		1. This function will set Client_id structure to empty in WA. Until restart DHCP
 *		client to update WA info into OM
 *
 */
UI32_T DHCP_MGR_C_DeleteClientId(UI32_T vid_ifIndex);

/*add by simon shih*/
BOOL_T DHCP_MGR_SetClientDefaultGateway(UI32_T vid_ifindex, UI8_T default_gateway[SYS_ADPT_IPV4_ADDR_LEN]);
BOOL_T DHCP_MGR_DeleteClientDefaultGateway(UI32_T vid_ifindex);


/* FUNCTION NAME : DHCP_MGR_ProvisionComplete
 * PURPOSE:
 *      All provision commands are settle down.
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
 *      1. This function is called by STKCTRL.
 */
void    DHCP_MGR_ProvisionComplete();


/*  related API in NETCFG
 *  NETCFG_SetDefaultManagedVlan (vid_ifIndex);
 *  NETCFG_BindDhcpToIf (vid_ifIndex)
 *  NETCFG_UnbindDhcpFromIf (vid_ifIndex)
 */

/**** L3: DHCP Server and Relay API ****/
/* API for Relay */


/* FUNCTION	NAME : DHCP_MGR_SetIfRole
 * PURPOSE:
 *		Define interface role in DHCP; this info. is kept in DHCP_WA.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		role		-- one of { client | server | relay }
 *						DHCP_TYPE_INTERFACE_BIND_CLIENT
 *						DHCP_TYPE_INTERFACE_BIND_RELAY
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_MGR_OK	-- successfully.
 *		DHCP_MGR_NO_IP -- interface does not have ip bind to it
 *		DHCP_MGR_DYNAMIC_IP -- the interface is acting as client (bootp/dhcp)
 *		DHCP_MGR_SERVER_ON -- the system is running as a dhcp server
 *		DHCP_MGR_FAIL	--  fail to set interface role.
 *
 * NOTES:
 *		1. This function is not used in layer2 switch.
 */
UI32_T DHCP_MGR_SetIfRole(UI32_T vid_ifIndex,UI32_T role);
UI32_T DHCP_MGR_SetIfStatus(UI32_T vid_ifIndex,UI32_T status);
/* FUNCTION NAME : DHCP_MGR_DeleteIfRole
 * PURPOSE:
 *      Delete interface role to WA
 *
 * INPUT:
 *      vid_ifIndex -- the interface to be configured.
 *		role	-- client / relay
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK	-- successfully.
 *		DHCP_MGR_FAIL	--  fail to set the interface role.
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_DeleteIfRole(UI32_T vid_ifIndex, UI32_T role);

/* FUNCTION NAME : DHCP_MGR_RemoveSystemRole
 * PURPOSE:
 *      Remove system role (server / server + client / relay + client / relay / client )
 *		to OM.
 *
 * INPUT:
 *      role	-- the role to be removed from system
 *
 * OUTPUT:
 *
 * RETURN:
 *      DHCP_MGR_OK 	-- Successfully set the system role
 *		DHCP_MGR_FALSE   -- Can't set the system role
 *
 * NOTES: None.
 *
 */
UI32_T DHCP_MGR_RemoveSystemRole(UI32_T role);

#if (SYS_CPNT_DHCP_RELAY == TRUE)
/* FUNCTION NAME : DHCP_MGR_SetIfRelayServerAddress
 * PURPOSE:
 *      Set dhcp server ip address for relay agent to WA
 *
 * INPUT:
 *      vid_ifIndex -- the interface to be configured.
 *		ip1, ip2, ip3, ip4, ip5	-- the ip list of servers
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	--  fail to set the relay server address.
 *
 * NOTES:
 *	1. The function will delete the previous setting for relay server address
 * 		and add the newly configured relay server address
 *	2. The maximun numbers of server ip are 5. If user specifies (ip1, ip2,0,0,0) which means
 *		the user only specifies 2 DHCP Relay Server.
 *	3. If user specifies (ip1, 0,ip3, ip4, ip5), (ip1,0,0,0,0)will be set to DHCP
 *		Relay Server Address List
 *
 */
UI32_T DHCP_MGR_SetIfRelayServerAddress(UI32_T vid_ifIndex, UI32_T ip1, UI32_T ip2, UI32_T ip3, UI32_T ip4, UI32_T ip5);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_MGR_GetIfRelayServerAddress
 *------------------------------------------------------------------------------
 * PURPOSE : Get relay server address for specified vlan interface
 * INPUT   : vid_ifIndex -- the interface vlan to be searched for the interface vlan's
 *						relay server address.
 *           ip_array    -- the array for Relay server IP Address.
 * OUTPUT  : None
 * RETUEN  : DHCP_MGR_OK/DHCP_MGR_FAIL      
 * NOTE    : None
 *------------------------------------------------------------------------------
 */
UI32_T DHCP_MGR_GetIfRelayServerAddress(
    UI32_T vid_ifIndex, 
    UI32_T ip_array[SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER]
);



/* FUNCTION NAME : DHCP_MGR_DeleteAllRelayServerAddress
 * PURPOSE:
 *      Delete all Server addresses for DHCP Relay Agent in WA
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK	-- successfully.
 *		DHCP_MGR_FAIL	--  FAIL to delete all relay server addresses.
 *
 * NOTES:1. For add relay server address, we can add all server IPs at
 *			one time. If we modify the server address twice, the last update
 *			would replace the previous setting.
 *
 *		 2. For delete relay server IP, we only allow delete all at a time
 *
 */
UI32_T DHCP_MGR_DeleteAllRelayServerAddress(UI32_T vid_ifIndex);

/* ==========================
** The APIs for SNMP Use
** ==========================
*/

/* FUNCTION NAME : DHCP_MGR_SetDhcpRelayServerAddrServerIp
 * PURPOSE:
 *      Set relay server address item by array index.
 *
 * INPUT:
 *      vid_ifIndex 	-- the interface to be configured.
 *	  	index			-- the index of the relay server array
 *		ip_address		-- the relay server address of the index to be set.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	--  fail to set the relay server address.
 *
 * NOTES:
 *		1. The 1st relay ip's index is 1.
 *		2. If the ip_address = 0, it will clear out the rest of the relay server address
 *			the case of current index's address is 0 and the rest of the relay server IPs
 *			are not 0, we need to clear out the rest of the relay server address.
 *
 */
UI32_T DHCP_MGR_SetDhcpRelayServerAddrServerIp(UI32_T vid_ifIndex, int index, UI32_T ip_address);

/* FUNCTION NAME : DHCP_MGR_AddInterfaceRelayServerAddress
 * PURPOSE:
 *      Add dhcp relay server ip addresses to the interface.
 *
 * INPUT:
 *      vid_ifIndex             -- the interface id
 *      number_of_relay_server  -- the number of dhcp relay server addresses
 *      relay_server_address[]  -- the list of dhcp relay server addresses
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCP_MGR_NO_SUCH_INTERFACE  -- fail to find the interface
 *      DHCP_MGR_FAIL               -- fail to add the dhcp relay server address
 *      DHCP_MGR_OK                 -- successfully
 *      DHCP_MGR_EXCEED_CAPACITY    -- exceed the maximum limit of dhcp relay server address
 *
 *
 * NOTES:
 *  1. The function returns DHCP_MGR_NO_SUCH_INTERFACE if
 *     - vid_ifIndex was invalid
 *
 *  2. The function returns DHCP_MGR_FAIL if
 *     - the interface is in slave mode
 *     - it fails to add addresses
 *
 *  3. The function returns DHCP_MGR_EXCEED_CAPACITY if
 *     - the total amount is bigger than SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER
 *     example1: (A,B,C) + (C,D,E) = (A,B,C,D,E) does not exceed the capacity.
 *     example2: (A,B) + (C,D,E,F) = (A,B,C,D,E,F) exceeds the capacity.
 *
 *  4. The function appends new relay server ip addresses right after the current
 *     relay server addresses.
 *
 */
UI32_T DHCP_MGR_AddInterfaceRelayServerAddress(UI32_T vid_ifIndex,
        UI32_T number_of_relay_server,
        UI32_T relay_server_address[]);

/* FUNCTION NAME : DHCP_MGR_GetDhcpRelayServerAddrTable
 * PURPOSE:
 *      Get dhcp relay server ip address for relay agent to WA.
 *		(Get whole record function for SNMP)
 *
 * INPUT:
 *      vid_ifIndex 	-- the interface to be configured.
 *		index			-- the index number to specify which
 *							relay server address provided to SNMP
 *		ip_address_p	-- the relay server retrieved by above keys.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	--  fail to get the relay server address.
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_GetDhcpRelayServerAddrTable(UI32_T vid_ifIndex, UI32_T index, UI32_T *ip_address_p);

/* FUNCTION NAME : DHCP_MGR_GetNextDhcpRelayServerAddrTable
 * PURPOSE:
 *      Get next dhcp relay server ip address for relay agent to WA.
 *
 * INPUT:
 *      vid_ifIndex_p	-- the pointer to the interface to be configured.
 *		index_p			-- the index number to specify which
 *							relay server address provided to SNMP
 *		ip_address_p	-- the relay server next to above keys.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	--  fail to get the relay server address.
 *
 * NOTES:
 *	1.  			---------> Get next
 * vid_ifIndex |        relay server address index
 *         		[1]    	|    [2]     |    [3]     |    [4]	   |    [5]     |
 *     		------------+------------+------------+------------+------------+
 * 1001   	 10.1.1.1	| 10.1.1.2	 | 10.1.1.3	  | 10.1.1.4   | 10.1.1.5	|
 *     		------------+------------+------------+------------+------------+
 * 1002   	 10.1.2.1	| 10.1.2.2	 | 10.1.2.3	  | 10.1.2.4   | 10.1.2.5	|
 *     		------------+------------+------------+------------+------------+
 * 1003   	 10.1.3.1	| 10.1.3.2	 | 10.1.3.3	  | 10.1.3.4   | 10.1.3.5	|
 *     		------------+------------+------------+------------+------------+
 *	2. GetNext for (0,0, &ip_address) will get the 1st address of the 1st interface which in above
 *		example is 10.1.1.1;
 *	3. GetNext for (1001,5, &ip_address), the result will be ip_address = 10.1.2.1
 *
 *
 */
UI32_T DHCP_MGR_GetNextDhcpRelayServerAddrTable(UI32_T *vid_ifIndex_p, UI32_T *index_p, UI32_T *ip_address_p);

/* FUNCTION NAME : DHCP_MGR_DeleteInterfaceRelayServerAddress
 * PURPOSE:
 *      Delete dhcp relay server ip addresses from the interface
 *
 * INPUT:
 *      vid_ifIndex             -- the interface id
 *      number_of_relay_server  -- the number of dhcp relay server addresses
 *      relay_server_address[]  -- the list of dhcp relay server addresses
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *      DHCP_MGR_NO_SUCH_INTERFACE  -- fail to find the interface
 *      DHCP_MGR_FAIL               -- fail to add the dhcp relay server address
 *      DHCP_MGR_OK                 -- successfully
 *
 * NOTES:
 *  1. The function returns DHCP_MGR_NO_SUCH_INTERFACE if
 *     - vid_ifIndex was invalid
 *
 *  2. The function returns DHCP_MGR_FAIL if
 *     - the interface is in slave mode.
 *     - it fails to set addresses.
 *
 *  3. The function deletes every dhcp relay server address before it
 *     returns DHCP_MGR_OK.
 *
 *  4. The function does not do any thing with an address which actually does not exist.
 *     If all the addresses do not exist, the function returns DHCP_MGR_OK.
 *
 */
UI32_T DHCP_MGR_DeleteInterfaceRelayServerAddress(UI32_T vid_ifIndex,
        UI32_T number_of_relay_server,
        UI32_T relay_server_address[]);
#endif


/* FUNCTION NAME : DHCP_MGR_SetRestartObject
 * PURPOSE:
 *      Setup the restart object in WA
 *
 * INPUT:
 *      restart_object -- specify the object needed to restart to system
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK	-- successfully.
 *		DHCP_MGR_FAIL	--  FAIL to delete all relay server addresses.
 *
 * NOTES:
 *		None.
 *
 */
UI32_T DHCP_MGR_SetRestartObject(UI32_T restart_object);


/* ==========================
** The APIs for DHCP Server
** ==========================
*/
#if (SYS_CPNT_DHCP_SERVER == TRUE)
/* FUNCTION	NAME : DHCP_MGR_EnterPool
 * PURPOSE: This function checks the specified pool's existence. If not existed,
 *			created the pool with the specified pool_name; if existed, do nothing
 *			just return.
 *
 * INPUT:
 *		pool_name -- specify the pool name to be entered.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_MGR_EXCEED_MAX_POOL_NAME -- exceed max size of pool name
 *		DHCP_MGR_FAIL -- Fail in allocating memory for pool creation
 *		DHCP_MGR_OK
 *
 * NOTES:
 *		1. Check pool name exceeds the max pool name length.
 *		2. Check pool existence. If not, create one with pool name.
 *
 */
UI32_T DHCP_MGR_EnterPool(char *pool_name);

/* FUNCTION NAME : DHCP_MGR_DeletePool
 * PURPOSE:
 *      To delete pool config entry
 *
 * INPUT:
 *      pool_name -- the name of the pool which will be deleted.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	-- fail to delete pool.
 *
 * NOTES:
 *		1. key is pool_name -- must be specified in order to retrieve
 */
UI32_T DHCP_MGR_DeletePool(char *pool_name);

/* FUNCTION NAME : DHCP_MGR_SetExcludedIp
 * PURPOSE:
 *      To specify the excluded ip address (range).
 *
 * INPUT:
 *      low_address  -- the lowest ip in the excluded ip address range
 * 		high_address -- the highest ip in the excluded ip address range
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	-- fail to set excluded ip address.
 *
 * NOTES:
 *		1. To specify exluded ip address range, the high-address is required.
 *		Otherwise, only specify low-address is treated as excluding an address
 *		from available address range.
 *		2. (low_address != 0, high_address == 0) will treat low_address the only
 *		address to be excluded.
 */
UI32_T DHCP_MGR_SetExcludedIp(UI32_T low_address, UI32_T high_address);

/* FUNCTION NAME : DHCP_MGR_DelExcludedIp
 * PURPOSE:
 *      To put back the excluded ip address (range) to available address pool.
 *
 * INPUT:
 *      low_address  -- the lowest ip in the excluded ip address range
 * 		high_address -- the highest ip in the excluded ip address range
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	-- fail to remove excluded ip address.
 *
 * NOTES:
 *		1. To specify exluded ip address range, the high-address is required.
 *		Otherwise, only specify low-address is treated as an excluded address
 *		back to available address range.
 *		2. (low_address != 0, high_address == 0) will treat low_address the only
 *		address to be added back.
 */
UI32_T DHCP_MGR_DelExcludedIp(UI32_T low_address, UI32_T high_address);

/* FUNCTION	NAME : DHCP_MGR_GetNextExcludedIp
 * PURPOSE:
 *		This function gets the next excluded IP address(es).
 *
 * INPUT:
 *		low_address  -- the lowest ip address of the excluded ip range
 *		high_address -- the highest ip address of the excluded ip range
 *
 * OUTPUT:
 *		low_address  -- the lowest ip address of the excluded ip range
 *		high_address -- the highest ip address of the excluded ip range
 *
 * RETURN:
 *		DHCP_MGR_OK -- successfully get the specified excluded ip.
 *		DHCP_MGR_FAIL -- FAIL to get the specified excluded ip.
 *
 * NOTES:
 *		1. low_address = 0 to get the 1st excluded IP entry.
 *		2. if return high_address = 0 which means there is no high address; this
 *			exclusion is only exclude an IP (low_address).
 */
UI32_T DHCP_MGR_GetNextExcludedIp(UI32_T *low_address, UI32_T *high_address);

/* FUNCTION NAME : DHCP_MGR_SetNetworkToPoolConfigEntry
 * PURPOSE:
 *      Set network address to pool config entry (set by field) and link it to
 *		network sorted link list.
 *
 * INPUT:
 *      pool_name 			-- pool name
 *		network_address	  	-- network ip address
 *		sub_netmask	  		-- subnet mask
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_POOL_HAS_CONFIG_TO_OTHERS 	-- pool has configured to host pool.
 *		DHCP_MGR_FAIL						-- fail to set network to pool config
 *		DHCP_MGR_NO_SUCH_POOL_NAME 			-- pool not existed.
 *		DHCP_MGR_INVALID_IP 				-- the specified network address is invalid.
 *
 * NOTES:
 *		1. Network address can't be specified to '0x0'
 *		2. If sub_netmask is 0x0 or user not specified, and also pool->sub_netmask
 *			is 0x0, we take natural subnet mask (class A, B or C).
 *		3. If sub_netmask is 0x0, but pool->sub_netmask is not 0x0, we still use
 *			 pool->sub_netmask as subnet mask
 */
UI32_T DHCP_MGR_SetNetworkToPoolConfigEntry(char *pool_name, UI32_T network_address, UI32_T sub_netmask);

/* FUNCTION NAME : DHCP_MGR_DelNetworkFromPoolConfigEntry
 * PURPOSE:
 *      Delete subnet number and mask from pool.
 *
 * INPUT:
 *      pool_name 			-- pool name
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	-- fail to set network to pool config
 *		DHCP_MGR_NO_SUCH_POOL_NAME -- pool name not existed.
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_DelNetworkFromPoolConfigEntry(char *pool_name);


/* FUNCTION NAME : DHCP_MGR_SetMacToPoolConfigEntry
 * PURPOSE:
 *      Set hardware address to pool config entry (set by field)
 *
 * INPUT:
 *      pool_name -- pool name
 *		mac		  -- hardware address
 *		type	  -- hardware address type  (DHCP_TYPE_HTYPE_ETHER / DHCP_TYPE_HTYPE_IEEE802)
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_FAIL				-- fail to set pool config entry to WA.
 *		DHCP_MGR_NO_SUCH_POOL_NAME 	-- Can't find any pool by specified pool_name
 *		DHCP_MGR_MAC_OVERSIZE		-- MAC address len larger than requirment.
 *
 * NOTES:
 *		1. For type, if user types 'ethernet', type = DHCP_MGR_HTYPE_ETHER;
 *			if user types 'ieee802', type = DHCP_MGR_HTYPE_IEEE802. If user
 *			doesn't type anything, by default, type = DHCP_MGR_HTYPE_ETHER
 *
 */
UI32_T DHCP_MGR_SetMacToPoolConfigEntry(char *pool_name, UI8_T *mac, UI32_T type);

/* FUNCTION NAME : DHCP_MGR_DelMacFromPoolConfigEntry
 * PURPOSE:
 *      Delete hardware address from pool config entry.
 *
 * INPUT:
 *      pool_name -- pool name.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_FAIL				-- fail to delete pool config entry to WA.
 *		DHCP_MGR_NO_SUCH_POOL_NAME 	-- Can't find any pool by specified pool_name
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_DelMacFromPoolConfigEntry(char *pool_name);

/* FUNCTION NAME : DHCP_MGR_SetCidToPoolConfigEntry
 * PURPOSE:
 *      Set client identifier to pool config entry (set by field)
 *
 * INPUT:
 *      pool_name -- pool name
 *		cid_mode  -- cid mode (text format or hex format)
 *		cid_len	  -- length of cid
 *		buf		  -- buffer that contains cid
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully
 *		DHCP_MGR_FAIL				-- fail to set Cid to pool config
 *		DHCP_MGR_EXCEED_MAX_SIZE	-- cid_len exceeds max size defined in leaf_es3626a.h
 *		DHCP_MGR_NO_SUCH_POOL_NAME  -- pool not existed
 *
 * NOTES:
 *		1. possible values of mode {DHCP_MGR_CID_HEX | DHCP_MGR_CID_TEXT}
 */
UI32_T DHCP_MGR_SetCidToPoolConfigEntry(char *pool_name, UI32_T cid_mode, UI32_T cid_len, char *buf);

/* FUNCTION NAME : DHCP_MGR_DelCidToPoolConfigEntry
 * PURPOSE:
 *      Delete client identifier from pool config entry
 *
 * INPUT:
 *      pool_name -- pool name
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	-- fail to set Cid to pool config
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_DelCidToPoolConfigEntry(char *pool_name);

/* FUNCTION NAME : DHCP_MGR_SetDomainNameToPoolConfigEntry
 * PURPOSE:
 *      Set domain name to pool config entry (set by field)
 *
 * INPUT:
 *      pool_name -- pool name
 *		domain_name -- domain name
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_FAIL				-- fail to set domain name
 *		DHCP_MGR_EXCEED_MAX_SIZE 	-- the length of domain name exceeds MAX size.
 *		DHCP_MGR_NO_SUCH_POOL_NAME  -- pool not exist.
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_SetDomainNameToPoolConfigEntry(char *pool_name, char *domain_name);


/* FUNCTION NAME : DHCP_MGR_DelDomainNameFromPoolConfigEntry
 * PURPOSE:
 *      Delete domain name from pool config entry (set by field)
 *
 * INPUT:
 *      pool_name -- pool name
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	-- fail to set domain name
 *		DHCP_MGR_NO_SUCH_POOL_NAME -- pool not exist
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_DelDomainNameFromPoolConfigEntry(char *pool_name);

/* FUNCTION NAME : DHCP_MGR_SetDnsServerToPoolConfigEntry
 * PURPOSE:
 *      Set domain name server ip to pool config entry (set by field)
 *
 * INPUT:
 *      pool_name -- pool name
 *		ip_array  -- array that contains DNS server ip address(es)
 *		size	  -- number of array elements
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_FAIL				-- fail to set domain name
 *		DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
 *		DHCP_MGR_EXCEED_MAX_SIZE	-- exceed max number of DNS server
 *
 * NOTES:
 *		1. The max numbers of dns server ip are 2.
 */
UI32_T DHCP_MGR_SetDnsServerToPoolConfigEntry(char *pool_name, UI32_T ip_array[], int size);

/* FUNCTION NAME : DHCP_MGR_DelDnsServerFromPoolConfigEntry
 * PURPOSE:
 *      Delete domain name server ip from pool config entry
 *
 * INPUT:
 *      pool_name -- pool name
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
  *		DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
 *		DHCP_MGR_FAIL	-- fail to remove dns server address(es)
 *
 * NOTES:
 *		1. The max numbers of dns server ip are 8.
 */
UI32_T DHCP_MGR_DelDnsServerFromPoolConfigEntry(char *pool_name);


/* FUNCTION NAME : DHCP_MGR_SetLeaseTimeToPoolConfigEntry
 * PURPOSE:
 *      Set lease time to pool config entry (set by field)
 *
 * INPUT:
 *      pool_name -- pool name
 *		day 	  -- day (0-365)
 *		hour 	  -- hour (0-23)
 *		minute 	  -- minute (0-59)
 *		is_infinite  -- infinite lease time (TRUE / FALSE)
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	-- fail to set domain name
 *		DHCP_MGR_INVALID_ARGUMENT 	-- argument is over boundary
 *		DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_SetLeaseTimeToPoolConfigEntry(char *pool_name, UI32_T lease_time
											,UI32_T  infinite);

/* FUNCTION NAME : DHCP_MGR_DelLeaseTimeFromPoolConfigEntry
 * PURPOSE:
 *      Delete lease time from pool config entry and restore default lease time.
 *
 * INPUT:
 *      pool_name -- pool name
 *
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_FAIL				-- fail to delete
 *		DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_DelLeaseTimeFromPoolConfigEntry(char *pool_name);

/* FUNCTION NAME : DHCP_MGR_SetDfltRouterToPoolConfigEntry
 * PURPOSE:
 *      Set default router address to pool config entry (set by field)
 *
 * INPUT:
 *      pool_name -- pool name
 *		ip_array  -- array that contains DNS server ip address(es)
 *		size	  -- number of array elements
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_FAIL				-- fail to set domain name
 *		DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
 *		DHCP_MGR_EXCEED_MAX_SIZE	-- exceed max number of dflt router
 *
 * NOTES:
 *		1. The max numbers of dflt router are 2.
 */
UI32_T DHCP_MGR_SetDfltRouterToPoolConfigEntry(char *pool_name, UI32_T ip_array[], int size);

/* FUNCTION NAME : DHCP_MGR_DelDfltRouterFromPoolConfigEntry
 * PURPOSE:
 *      Delete default router from pool config entry.
 *
 * INPUT:
 *      pool_name -- pool name
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
 *		DHCP_MGR_FAIL	-- fail to remove dflt router address(es)
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_DelDfltRouterFromPoolConfigEntry(char *pool_name);

/* FUNCTION NAME : DHCP_MGR_SetBootfileToPoolConfigEntry
 * PURPOSE:
 *      Set bootfile name to pool config entry (set by field)
 *
 * INPUT:
 *      pool_name -- pool name
 *		bootfile  -- bootfile name
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_FAIL				-- fail to set bootfile name
 *		DHCP_MGR_EXCEED_MAX_SIZE 	-- the length of bootfile name exceeds MAX size.
 *		DHCP_MGR_NO_SUCH_POOL_NAME  -- pool not exist.
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_SetBootfileToPoolConfigEntry(char *pool_name, char *bootfile);

/* FUNCTION NAME : DHCP_MGR_DelBootfileFromPoolConfigEntry
 * PURPOSE:
 *      Delete bootfile name from pool config entry (set by field)
 *
 * INPUT:
 *      pool_name -- pool name
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	-- fail to remove bootfile name
 *		DHCP_MGR_NO_SUCH_POOL_NAME -- pool not exist
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_DelBootfileFromPoolConfigEntry(char *pool_name);

/* FUNCTION NAME : DHCP_MGR_SetNetbiosNameServerToPoolConfigEntry
 * PURPOSE:
 *      Set netbios name server ip to pool config entry (set by field)
 *
 * INPUT:
 *      pool_name -- pool name
 *		ip_array  -- array that contains Netbios Name Server ip address(es)
 *		size	  -- number of array elements
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_FAIL				-- fail to set netbios name server ip
 *		DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
 *		DHCP_MGR_EXCEED_MAX_SIZE	-- exceed max number of netbios name server
 *
 * NOTES:
 *		1. The max numbers of dns server ip are 5.
 */
UI32_T DHCP_MGR_SetNetbiosNameServerToPoolConfigEntry(char *pool_name, UI32_T ip_array[], int size);

/* FUNCTION NAME : DHCP_MGR_DelNetbiosNameServerFromPoolConfigEntry
 * PURPOSE:
 *      Delete netbios name server ip from pool config entry
 *
 * INPUT:
 *      pool_name -- pool name
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
 *		DHCP_MGR_FAIL	-- fail to remove netbios name server ip address(es)
 *
 * NOTES:
 *		1. The max numbers of dns server ip are 8.
 */
UI32_T DHCP_MGR_DelNetbiosNameServerFromPoolConfigEntry(char *pool_name);

/* FUNCTION NAME : DHCP_MGR_SetNetbiosNodeTypeToPoolConfigEntry
 * PURPOSE:
 *      Set netbios node type to pool config entry (set by field).
 *
 * INPUT:
 *      pool_name -- pool name
 *		node_type -- netbios node type
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	-- fail to set netbios node type
 *		DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
 *
 * NOTES:
 *		1. possible value of node_type:
 *		DHCP_MGR_NETBIOS_NODE_TYPE_B_NODE -- broadcast
 *		DHCP_MGR_NETBIOS_NODE_TYPE_P_NODE -- peer to peer
 *		DHCP_MGR_NETBIOS_NODE_TYPE_M_NODE -- mixed
 *		DHCP_MGR_NETBIOS_NODE_TYPE_H_NODE -- hybrid (recommended)
 */
UI32_T DHCP_MGR_SetNetbiosNodeTypeToPoolConfigEntry(char *pool_name, UI32_T node_type);

/* FUNCTION NAME : DHCP_MGR_DelNetbiosNodeTypeFromPoolConfigEntry
 * PURPOSE:
 *      Delete netbios node type from pool config entry and restore default lease time.
 *
 * INPUT:
 *      pool_name -- pool name
 *
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_FAIL				-- fail to delete
 *		DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_DelNetbiosNodeTypeFromPoolConfigEntry(char *pool_name);

/* FUNCTION NAME : DHCP_MGR_SetNextServerToPoolConfigEntry
 * PURPOSE:
 *      Set next server ip to pool config entry (set by field)
 *
 * INPUT:
 *      pool_name -- pool name
 *		ip_array  -- array that contains next server ip address(es)
 *		size	  -- number of array elements
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_FAIL				-- fail to set next server
 *		DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
 *		DHCP_MGR_EXCEED_MAX_SIZE	-- exceed max number of next server
 *
 * NOTES:
 *		1. The max numbers of dns server ip are 5.
 */
UI32_T DHCP_MGR_SetNextServerToPoolConfigEntry(char *pool_name, UI32_T ipaddr);

/* FUNCTION NAME : DHCP_MGR_DelNextServerFromPoolConfigEntry
 * PURPOSE:
 *      Delete next server ip from pool config entry
 *
 * INPUT:
 *      pool_name -- pool name
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_NO_SUCH_POOL_NAME  -- the pool not existed
 *		DHCP_MGR_FAIL	-- fail to remove next server address(es)
 *
 * NOTES:
 *		1. The max numbers of next server ip are 8.
 */
UI32_T DHCP_MGR_DelNextServerFromPoolConfigEntry(char *pool_name);

/* FUNCTION NAME : DHCP_MGR_SetHostToPoolConfigEntry
 * PURPOSE:
 *      Set host address to pool config entry (set by field).
 *
 * INPUT:
 *      pool_name 			-- pool name
 *		network_address	  	-- host ip address
 *		sub_netmask	  		-- subnet mask
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK							-- successfully.
 *		DHCP_MGR_POOL_HAS_CONFIG_TO_OTHERS 	-- pool has configured to network pool.
 *		DHCP_MGR_FAIL						-- fail to set host to pool config
 *		DHCP_MGR_NO_SUCH_POOL_NAME 			-- pool not existed.
 *		DHCP_MGR_INVALID_IP 				-- the specified host address is invalid.
 *
 * NOTES:
 *		1. host address can't be specified to '0x0'
 *		2. If sub_netmask is 0x0 or user not specified, and also pool->sub_netmask
 *			is 0x0, we take natural subnet mask (class A, B or C).
 *		3. If sub_netmask is 0x0, but pool->sub_netmask is not 0x0, we still use
 *			 pool->sub_netmask as subnet mask
 */
UI32_T DHCP_MGR_SetHostToPoolConfigEntry(char *pool_name, UI32_T host_address, UI32_T sub_netmask);

/* FUNCTION NAME : DHCP_MGR_DelHostFromPoolConfigEntry
 * PURPOSE:
 *      Delete host address and mask from pool.
 *
 * INPUT:
 *      pool_name 			-- pool name
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	-- fail to remove host to pool config
 *		DHCP_MGR_NO_SUCH_POOL_NAME -- pool not existed
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_DelHostFromPoolConfigEntry(char *pool_name);

/* FUNCTION NAME : DHCP_MGR_GetIpBinding
 * PURPOSE:
 *      Get the specified ip binding in server_om (memory.c)
 *
 * INPUT:
 *      ip_address -- the ip for corresponding binding information
 *
 * OUTPUT:
 *      ip_binding -- NULL: Can't get ip_binding lease based on the input IP.
 *					  others: the pointer to the ip_binding lease in memory.c
 *
 *
 * RETURN:
 *		DHCP_MGR_OK		--  successfully.
 *		DHCP_MGR_FAIL	--  FAIL to get ip binding.
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_GetIpBinding(UI32_T ip_address, DHCP_MGR_Server_Lease_Config_T *ip_binding);

/* FUNCTION NAME : DHCP_MGR_GetActiveIpBinding
 * PURPOSE:
 *      Get the active specified ip binding in server_om (memory.c)
 *
 * INPUT:
 *      ip_address -- the ip for corresponding binding information
 *
 * OUTPUT:
 *      ip_binding -- NULL: Can't get ip_binding lease based on the input IP.
 *                    others: the pointer to the ip_binding lease in memory.c
 *
 *
 * RETURN:
 *      DHCP_MGR_OK     --  successfully.
 *      DHCP_MGR_FAIL   --  FAIL to get ip binding.
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_GetActiveIpBinding(UI32_T ip_address, DHCP_MGR_Server_Lease_Config_T *ip_binding);

/* FUNCTION NAME : DHCP_MGR_GetNextIpBinding
 * PURPOSE:
 *      Get the next specified ip binding in server_om (memory.c)
 *
 * INPUT:
 *      ip_address -- the ip for corresponding binding information
 *
 * OUTPUT:
 *     	ip_binding -- NULL: Can't get ip_binding lease based on the input IP.
 *					  others: the pointer to the next ip_binding lease
 *						in memory.c
 *
 * RETURN:
 *
 *		DHCP_MGR_OK		--  successfully.
 *		DHCP_MGR_FAIL	--  FAIL to get next ip binding.
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_GetNextIpBinding(UI32_T ip_address, DHCP_MGR_Server_Lease_Config_T *ip_binding);

/* FUNCTION NAME : DHCP_MGR_GetNextActiveIpBinding
 * PURPOSE:
 *      Get the next active specified ip binding in server_om (memory.c)
 *
 * INPUT:
 *      ip_address -- the ip for corresponding binding information
 *
 * OUTPUT:
 *      ip_binding -- NULL: Can't get ip_binding lease based on the input IP.
 *                    others: the pointer to the next ip_binding lease
 *                      in memory.c
 *
 * RETURN:
 *
 *      DHCP_MGR_OK     --  successfully.
 *      DHCP_MGR_FAIL   --  FAIL to get next ip binding.
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_GetNextActiveIpBinding(UI32_T ip_address, DHCP_MGR_Server_Lease_Config_T *ip_binding);


/* FUNCTION NAME : DHCP_MGR_ClearIpBinding
 * PURPOSE:
 *      Clear the specified ip binding in server_om (memory.c)
 *
 * INPUT:
 *      ip_address -- the ip for marking available again in pool
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK	-- successfully.
 *		DHCP_MGR_FAIL	--  FAIL to clear ip binding.
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_ClearIpBinding(UI32_T ip_address);

/* FUNCTION NAME : DHCP_MGR_ClearAllIpBinding
 * PURPOSE:
 *      Clear all ip bindings in server_om (memory.c)
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_MGR_OK	-- successfully.
 *		DHCP_MGR_FAIL	--  FAIL to clear ip binding.
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_ClearAllIpBinding(void);

/* FUNCTION	NAME : DHCP_MGR_GetPoolByPoolName
 * PURPOSE:
 *		Get pool config by pool name in WA.
 *
 * INPUT:
 *		pool_name -- the name of the pool to be searched.
 *
 * OUTPUT:
 *		pool_config -- the pointer to the pool config structure
 *
 * RETURN:
 *		DHCP_MGR_OK	-- successfully.
 *		DHCP_MGR_FAIL	--  FAIL to get.
 *
 * NOTES:
 *
 *
 */
UI32_T DHCP_MGR_GetPoolByPoolName(char *pool_name, DHCP_TYPE_PoolConfigEntry_T *pool_config_p);

/* FUNCTION	NAME : DHCP_MGR_GetNextPoolConfig
 * PURPOSE:
 *		Get next config for pool in WA.
 *
 * INPUT:
 *		pool_config -- the pointer to the pool config structure
 *					-- NULL to get the 1st pool config
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_MGR_OK	-- successfully.
 *		DHCP_MGR_FAIL	--  FAIL to get.
 *
 * NOTES:
 *
 *
 */
UI32_T DHCP_MGR_GetNextPoolConfig(DHCP_TYPE_PoolConfigEntry_T *pool_config_p);

/* FUNCTION	NAME : DHCP_MGR_GetActivePool
 * PURPOSE:
 *		Get   active pool for WA. Search key is pool_name + pool_range.low_address
 *
 * INPUT:
 *		pool_name -- the pointer to the pool name
 *            pool_range_p.low_address --  pool_range to get next range. key is pool_range.low_address.
 * OUTPUT:
 *		pool_range_p.high_address
 *
 * RETURN:
 *		DHCP_MGR_OK	-- successfully.
 *		DHCP_MGR_FAIL	--  FAIL to get.
 *              DHCP_MGR_INVALID_ARGUMENT
 *              DHCP_MGR_NO_SUCH_POOL_NAME  -- can not find pool with giving pool name
 *
 * NOTES:
 *
 *
 */
UI32_T DHCP_MGR_GetActivePool(char* pool_name, DHCP_MGR_ActivePoolRange_T *pool_range_p);

/* FUNCTION	NAME : DHCP_MGR_GetNextActivePool
 * PURPOSE:
 *		Get next active pool for WA. Search key is pool_name + pool_range.low_address
 *
 * INPUT:
 *		pool_name -- the pointer to the pool name
 *                -- get 1st pool if pool_name is null string
 *              pool_range_p --  pool_range to get next range. key is pool_range.low_address.
 *                               --  get 1st range if pool_range.low_address is 0
 * OUTPUT:
 *		pool_range_p.
 *      pool_name
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	--  FAIL to get.
 *           DHCP_MGR_INVALID_ARGUMENT
 *
 * NOTES:
 *
 *
 */
UI32_T DHCP_MGR_GetNextActivePool(char* pool_name, DHCP_MGR_ActivePoolRange_T *pool_range_p);

/* FUNCTION	NAME : DHCP_MGR_GetDhcpServerServiceStatus
 * PURPOSE:
 *		Get DHCP Server Service Status. (Enable / Disable)
 *
 * INPUT:
 *		None.
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		TRUE  -- Indicate DHCP server is enabled.
 *		FALSE -- Indicate DHCP server is disabled.
 *
 * NOTES:
 *		This API is for WEB to get Currently system running status for DHCP Server.
 *
 */
BOOL_T DHCP_MGR_GetDhcpServerServiceStatus(void);

/* FUNCTION NAME : DHCP_MGR_GetDhcpPoolTable
 * PURPOSE:
 *      Get dhcp pool to WA. (Get by record function for SNMP)
 *
 * INPUT:
 *		pool_name 		-- specify the pool name of the table.
  *		pool_table_p -- the pointer to dhcp pool table.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	--  fail to get dhcp pool table
 *
 * NOTES:
 */
UI32_T DHCP_MGR_GetDhcpPoolTable(char *pool_name, DHCP_TYPE_PoolConfigEntry_T *pool_table_p);

/* FUNCTION NAME : DHCP_MGR_GetNextDhcpPoolTable
 * PURPOSE:
 *      Get next dhcp pool to WA. (Get by record function for SNMP)
 *
 * INPUT:
 *		pool_name 		-- specify the pool name of the table.
 *		pool_table_p -- the pointer to dhcp pool table.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	--  fail to get next dhcp pool table.
 *
 * NOTES:
 *	1. *pool_name = 0 to get the 1st pool and returning pool_name in *pool_name.
 */
UI32_T DHCP_MGR_GetNextDhcpPoolTable(char *pool_name, DHCP_TYPE_PoolConfigEntry_T *pool_table_p);

/* FUNCTION NAME : DHCP_MGR_SetDhcpPoolPoolType
 * PURPOSE:
 *      Set pool type to the pool. (set by field)
 *
 * INPUT:
 *		pool_name 		-- specify the pool name of the table.
 *		pool_type		-- the type of the pool.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_NO_SUCH_POOL_NAME 	-- pool not existed.
 *		DHCP_MGR_FAIL				-- fail to set the pool type.
 *
 * NOTES:
 *		1. If pool has been configured to other pool_type, user must delete the original
 *			pool before setting new pool type.
 */
UI32_T DHCP_MGR_SetDhcpPoolPoolType(char *pool_name, UI32_T pool_type);

/* FUNCTION NAME : DHCP_MGR_SetDhcpPoolPoolAddress
 * PURPOSE:
 *      Set pool address to the pool. (set by field)
 *
 * INPUT:
 *		pool_name 		-- specify the pool name of the table.
 *		pool_address	-- the address of the pool.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_NO_SUCH_POOL_NAME 			-- pool not existed.
 *		DHCP_MGR_POOL_HAS_CONFIG_TO_OTHERS	-- fail to set pool address due to user not yet
 *												specified pool type.
 *		DHCP_MGR_INVALID_IP 				-- the specified network address is invalid.
 *		DHCP_MGR_FAIL						--  fail to set the pool address.
 *
 * NOTES:
 *		1. User needs to specified pool type first before specified pool address.
 */
UI32_T DHCP_MGR_SetDhcpPoolPoolAddress(char *pool_name, UI32_T pool_address);


/* FUNCTION NAME : DHCP_MGR_SetDhcpPoolMacAddress
 * PURPOSE:
 *      Set hardware mac to the pool. (set by field)
 *
 * INPUT:
 *		pool_name 		-- specify the pool name of the table.
 *		pool_hardware_mac	-- the hardware mac of the pool.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_NO_SUCH_POOL_NAME 	-- pool not existed.
 *		DHCP_MGR_FAIL				-- fail to set the pool hardware mac.
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_SetDhcpPoolMacAddress(char *pool_name, UI8_T *pool_hardware_mac);

/* FUNCTION NAME : DHCP_MGR_SetDhcpPoolStatus
 * PURPOSE:
 *      Set pool status to the pool. (set by field)
 *
 * INPUT:
 *		pool_name 		-- specify the pool name of the table.
 *		pool_status	-- the status of the pool.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_NO_SUCH_POOL_NAME 	-- pool not existed.
 *		DHCP_MGR_FAIL				-- fail to set the pool status.
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_SetDhcpPoolStatus(char *pool_name, UI32_T pool_status);

/* FUNCTION NAME : DHCP_MGR_GetDhcpPoolOptionTable
 * PURPOSE:
 *      Get dhcp option table from WA. (Get by record function for SNMP)
 *
 * INPUT:
 *		pool_name 		-- specify the pool name of the table.
  *		pool_option_p 	-- the pointer to dhcp option table.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_NO_SUCH_POOL_NAME 	-- pool not existed.
 *		DHCP_MGR_FAIL				-- fail to get dhcp option table
 *
 * NOTES:
 */
UI32_T DHCP_MGR_GetDhcpPoolOptionTable(char *pool_name, DHCP_TYPE_ServerOptions_T *pool_option_p);

/* FUNCTION NAME : DHCP_MGR_GetNextDhcpPoolOptionTable
 * PURPOSE:
 *      Get next dhcp option table from WA. (Get by record function for SNMP)
 *
 * INPUT:
 *		pool_name 		-- specify the pool name of the table.
 *		pool_option_p 	-- the pointer to dhcp option table.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_FAIL	--  fail to get dhcp option table
 *
 * NOTES:
 *	1. *pool_name = NULL to get the 1st pool option table and return 1st pool name in
 *		*pool_name.
 */
UI32_T DHCP_MGR_GetNextDhcpPoolOptionTable(char *pool_name, DHCP_TYPE_ServerOptions_T *pool_option_p);


/* FUNCTION NAME : DHCP_MGR_SetDhcpPoolOptionDnsSerIpAddress
 * PURPOSE:
 *      Set the dns server ip address.
 *
 * INPUT:
 *		pool_name 	-- specify the pool name of the table
 *		index		-- dns server index
 *		ip_addr	 	-- dns server ip
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK							-- successfully.
 *		DHCP_MGR_FAIL	--  fail to set the value
 *
 * NOTES:
 *	1. 'index' is not array's index; instead, it is the index using in SNMP to
 *		specify either the 1st dns server address or the second one.
 *	2. If index == 1, and ip_addr[2] != 0 and ip_addr[1] == 0, this configuration will
 *		clear out all ip setting.
 */
UI32_T DHCP_MGR_SetDhcpPoolOptionDnsSerIpAddress(char *pool_name,
        UI32_T index,
        UI32_T ip_addr);

/* FUNCTION NAME : DHCP_MGR_GetDhcpPoolOptionDnsSerTable
 * PURPOSE:
 *      Get dns server of dhcp pool from WA.
 *
 * INPUT:
 *		pool_name 		-- specify the pool name of the table
 *		index			-- the index of the address
 *		dns_server_p	-- the dns server addresses
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK							-- successfully.
 *		DHCP_MGR_FAIL	-- fail to get the value.
 *
 * NOTES:
 */
UI32_T DHCP_MGR_GetDhcpPoolOptionDnsSerTable(char *pool_name,
        UI32_T index,
        UI32_T *dns_server_p);

/* FUNCTION NAME : DHCP_MGR_GetNextDhcpPoolOptionDnsSerTable
 * PURPOSE:
 *      Get next dns server table of dhcp pool from WA.
 *
 * INPUT:
 *		pool_name 		-- specify the pool name of the table
 *		index_p			-- the index of the address
 *		dns_server_p 	-- the array to store dns server addresses
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_FAIL	-- fail to get the value.
 *
 * NOTES:
 *
 *	1.  			---------> Get next
 * pool_name|        address index
 *         		[1]    	|    [2]     |
 *     		------------+------------+
 * pool1   	 10.1.1.1	| 10.1.1.2	 |
 *     		------------+------------+
 * pool2   	 10.1.2.1	| 10.1.2.2	 |
 *     		------------+------------+
 *	2. getNext(NULL, 0, &ip_address) will get the 1st pool and 1st index address, in above case
 *		ip_address = 10.1.1.1.
 *	3. getNext(pool1, 2, &ip_address), the result ip_address = 10.1.2.1
 */
UI32_T DHCP_MGR_GetNextDhcpPoolOptionDnsSerTable(char *pool_name,
        UI32_T *index_p,
        UI32_T *dns_server_p);


/* FUNCTION NAME : DHCP_MGR_SetDhcpPoolOptDefaultRouterIpAddress
 * PURPOSE:
 *      Set the dflt router ip address.
 *
 * INPUT:
 *		pool_name 	-- specify the pool name of the table
 *		index		-- dflt router address index
 *		ip_addr	 	-- dflt router ip
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_FAIL	--  fail to set the value
 *
 * NOTES:
 *	1. 'index' is not array's index; instead, it is the index using in SNMP to
 *		specify either the 1st dflt router address or the second one.
 */
UI32_T DHCP_MGR_SetDhcpPoolOptDefaultRouterIpAddress(char *pool_name,
        UI32_T index,
        UI32_T ip_addr);

/* FUNCTION NAME : DHCP_MGR_GetDhcpPoolOptDefaultRouterTable
 * PURPOSE:
 *      Get dflt router of dhcp pool from WA.
 *
 * INPUT:
 *		pool_name 		-- specify the pool name of the table
 *		index			-- the address index
 *		dflt_router_p	-- the dflt router addresses
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_FAIL	-- fail to get the value.
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_GetDhcpPoolOptDefaultRouterTable(char *pool_name,
        UI32_T index,
        UI32_T *dflt_router_p);

/* FUNCTION NAME : DHCP_MGR_GetNextDhcpPoolOptDefaultRouterTable
 * PURPOSE:
 *      Get next dflt router table of dhcp pool from WA. (Get by record function for SNMP)
 *
 * INPUT:
 *		pool_name 		-- specify the pool name of the table
 *		index_p			-- the address index
 *		dflt_router_p	-- the dflt router addresses
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_FAIL	-- fail to get the value.
 *
 * NOTES:
 *	1.  			---------> Get next
 * pool_name|        address index
 *         		[1]    	|    [2]     |
 *     		------------+------------+
 * pool1   	 10.1.1.1	| 10.1.1.2	 |
 *     		------------+------------+
 * pool2   	 10.1.2.1	| 10.1.2.2	 |
 *     		------------+------------+
 *	2. getNext(NULL, 0, &ip_address) will get the 1st pool and 1st index address, in above case
 *		ip_address = 10.1.1.1.
 *	3. getNext(pool1, 2, &ip_address), the result ip_address = 10.1.2.1
 */
UI32_T DHCP_MGR_GetNextDhcpPoolOptDefaultRouterTable(char *pool_name,
        UI32_T *index_p,
        UI32_T *dflt_router_p);


/* FUNCTION NAME : DHCP_MGR_SetDhcpPoolOptNetbiosServerIpAddress
 * PURPOSE:
 *      Set the netbios server ip address.
 *
 * INPUT:
 *		pool_name 	-- specify the pool name of the table
 *		index		-- netbios server index
 *		ip_addr	 	-- netbios server ip
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					-- successfully.
 *		DHCP_MGR_FAIL	--  fail to set the value
 *
 * NOTES:
 *	1. 'index' is not array's index; instead, it is the index using in SNMP to
 *		specify either the 1st netbios server address or the second one.
 */
UI32_T DHCP_MGR_SetDhcpPoolOptNetbiosServerIpAddress(char *pool_name,
        UI32_T index,
        UI32_T ip_addr);

/* FUNCTION NAME : DHCP_MGR_GetDhcpPoolOptNetbiosServerTable
 * PURPOSE:
 *      Get netbios server of dhcp pool from WA.
 *
 * INPUT:
 *		pool_name 			-- specify the pool name of the table
 *		index				-- the address index
 *		netbios_server_p 	-- the netbios server addresses
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	-- fail to get the value.
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_GetDhcpPoolOptNetbiosServerTable(char *pool_name,
        UI32_T index,
        UI32_T *netbios_server_p);

/* FUNCTION NAME : DHCP_MGR_GetNextDhcpPoolOptNetbiosServerTable
 * PURPOSE:
 *      Get next netbios server of dhcp pool from WA. (Get by record function for SNMP)
 *
 * INPUT:
 *		pool_name 		-- specify the pool name of the table
 *		index_p				-- the address index
 *		netbios_server_p 	-- the netbios server addresses
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	-- fail to get the value.
 *
 * NOTES:
 *	1.  			---------> Get next
 * pool_name|        address index
 *         		[1]    	|    [2]     |
 *     		------------+------------+
 * pool1   	 10.1.1.1	| 10.1.1.2	 |
 *     		------------+------------+
 * pool2   	 10.1.2.1	| 10.1.2.2	 |
 *     		------------+------------+
 *	2. getNext(NULL, 0, &ip_address) will get the 1st pool and 1st index address, in above case
 *		ip_address = 10.1.1.1.
 *	3. getNext(pool1, 2, &ip_address), the result ip_address = 10.1.2.1
 */
UI32_T DHCP_MGR_GetNextDhcpPoolOptNetbiosServerTable(char *pool_name,
        UI32_T *index_p,
        UI32_T *netbios_server_p);

/* FUNCTION NAME : DHCP_MGR_SetDhcpPoolOptionCidMode
 * PURPOSE:
 *      Set client identifier mode to the option table from WA.
 *
 * INPUT:
 *		pool_name 	-- specify the pool name of the table.
 *		cid_mode 	-- cid mode for this pool.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK					--  successfully.
 *		DHCP_MGR_NO_SUCH_POOL_NAME  --  pool not exist.
 *		DHCP_MGR_FAIL				--  fail to set the value
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_SetDhcpPoolOptionCidMode(char *pool_name, UI32_T cid_mode);

/* FUNCTION NAME : DHCP_MGR_SetDhcpPoolOptionCidBuffer
 * PURPOSE:
 *      Set client identifier in the buffer to the option table from WA.
 *
 * INPUT:
 *		pool_name 	-- specify the pool name of the table.
 *		cid_buf 	-- cid buffer for this pool.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		--  successfully.
 *		DHCP_MGR_FAIL	--  fail to set the value
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_SetDhcpPoolOptionCidBuffer(char *pool_name, char *cid_buf);

/* FUNCTION NAME : DHCP_MGR_SetDhcpPoolSubnetMask
 * PURPOSE:
 *      Set subnet mask to the pool. (set by field)
 *
 * INPUT:
 *		pool_name 	-- specify the pool name of the table.
 *		pool_subnet_mask	-- the subnet mask of the pool.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		--  successfully.
 *		DHCP_MGR_NO_SUCH_POOL_NAME 			-- pool not existed.
 *		DHCP_MGR_POOL_HAS_CONFIG_TO_OTHERS	-- fail to set pool address due to user not yet
 *												specified pool type.
 *		DHCP_MGR_INVALID_IP 				-- the specified network address is invalid.
 *		DHCP_MGR_FAIL						-- fail to set the pool subnet mask.
 *
 * NOTES:
 *	1. If pool_subnet_mask is the same as what we previous configured, do nothing for it.
 */
UI32_T DHCP_MGR_SetDhcpPoolSubnetMask(char *pool_name, UI32_T pool_subnet_mask);

/* FUNCTION NAME : DHCP_MGR_SetDhcpPoolHardwareType
 * PURPOSE:
 *      Set hardware type to the pool. (set by field)
 *
 * INPUT:
 *		pool_name 			-- specify the pool name of the table.
 *		pool_hardware_type	-- the hardware type of the pool.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_NO_SUCH_POOL_NAME 	-- pool not existed.
 *		DHCP_MGR_FAIL				-- fail to set the pool hardware type.
 *
 * NOTES:
 *	1. SNMP needs to change the hardware type from leaf constant value
 *		to the corresponding value presenting in DHCP_MGR.h
 */
UI32_T DHCP_MGR_SetDhcpPoolHardwareType(char *pool_name, UI32_T pool_hardware_type);

/* FUNCTION NAME : DHCP_MGR_GetDhcpServerExcludedIpAddrTable
 * PURPOSE:
 *      Get exclude IP addr table of dhcp pool from WA. (Get by record function for SNMP)
 *
 * INPUT:
 *		low_address  -- the lowest ip address of the excluded ip range
 *		high_address -- the highest ip address of the excluded ip range
 *
 * OUTPUT:
 *		low_address  -- the lowest ip address of the excluded ip range
 *		high_address -- the highest ip address of the excluded ip range
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	-- fail to get the value.
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_GetDhcpServerExcludedIpAddrTable(UI32_T *low_address,
        UI32_T *high_address);

#endif





/* FUNCTION NAME : DHCP_MGR_SetPoolNameToPoolConfigEntry
 * PURPOSE:
 *      To set pool name to pool config entry
 *
 * INPUT:
 *      pool_name
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	-- fail to set pool name
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_SetPoolNameToPoolConfigEntry(char *pool_name);

/* end of server config */



/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DHCP_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for DHCP MGR.
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
BOOL_T DHCP_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

/* begin 2007-12, Joseph */

/* FUNCTION	NAME : DHCP_MGR_C_SetVendorClassId
 * PURPOSE:
 *		Define associated class identifier per interface.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		id_mode 	-- the class id mode.
 *		id_len 		-- the len of buffer.
 *		id_buf		-- the content of the class id.	
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	-- No more space in allocating memory
 *		DHCP_MGR_CLASSID_EXCEED_MAX_SIZE -- class id exceeds the max size
 *
 * NOTES:
 *		1. Possible values of id_mode are {DHCP_TYPE_CLASSID_HEX | DHCP_TYPE_CLASSID_TEXT}.
 *
 */
UI32_T DHCP_MGR_C_SetVendorClassId(UI32_T vid_ifIndex, UI32_T id_mode, UI32_T id_len, char *id_buf);

/* FUNCTION	NAME : DHCP_MGR_C_GetVendorClassId
 * PURPOSE:
 *		Retrieve the associated class identifier for specified interface from OM.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *
 * OUTPUT:
 *		class_id_p	-- the pointer of class id structure
 *
 * RETURN:
 *		DHCP_MGR_OK	  -- successfully.
 *		DHCP_MGR_FAIL -- failure. 
 *		DHCP_MGR_NO_SUCH_INTERFACE -- the specified vlan not existed in DHCP_OM table
 *		DHCP_MGR_NO_CLASSID	-- this interface has no CLASSID configuration.
 *
 * NOTES:
 *		None.	
 *
 */
UI32_T DHCP_MGR_C_GetVendorClassId(UI32_T vid_ifIndex, DHCP_MGR_Vendor_T *class_id_p);

/* FUNCTION	NAME : DHCP_MGR_C_GetRunningVendorClassId
 * PURPOSE:
 *		Get running config for CLASS ID in WA (Working Area).
 *
 * INPUT:
 *		vid_ifIndex -- the key to identify Vendor Class Id
 *		
 *
 * OUTPUT:
 *		class_id_p	-- the pointer of class id structure
 *
 * RETURN:
 *		SYS_TYPE_GET_RUNNING_CFG_SUCCESS 
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL              
 *
 * NOTES:
 *
 */
UI32_T DHCP_MGR_C_GetRunningVendorClassId(UI32_T vid_ifIndex, DHCP_MGR_Vendor_T *class_id_p);

/* FUNCTION	NAME : DHCP_MGR_C_GetNextVendorClassId
 * PURPOSE:
 *		Get the next class id for specified interface.
 *
 * INPUT:
 *		vid_ifIndex -- the current interface.
 *		
 *
 * OUTPUT:
 *		vid_ifIndex --  the next active vlan interface.
 *      class_id_p	-- the pointer of class id structure
 *
 * RETURN:
 *		DHCP_MGR_OK	  
 *		DHCP_MGR_FAIL  
 *
 * NOTES:
 *		
 */
UI32_T DHCP_MGR_C_GetNextVendorClassId(UI32_T *vid_ifIndex, DHCP_MGR_Vendor_T *class_id_p);

/* FUNCTION	NAME : DHCP_MGR_C_DeleteVendorClassId
 * PURPOSE:
 *		Delete the client identifier for specified interface, so that the 
 *      vendor class option60 shall not be added into packet.
 *
 * INPUT:
 *		vid_ifIndex -- the interface to be defined.
 *		
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		DHCP_MGR_OK	  -- successfully.
 *		DHCP_MGR_FAIL -- the specified interface has no cid config, so can't delete class id 
 *
 *
 * NOTES:
 *		1. This function will set class_id structure to empty in WA (Working Area). 
 *
 */
UI32_T DHCP_MGR_C_DeleteVendorClassId(UI32_T vid_ifIndex);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCP_MGR_Destroy_If
 *------------------------------------------------------------------------------
 * PURPOSE: Destory DHCP interface via L3 interface destroy callback
 * INPUT :  vid_ifindex --  vlan id ifindex
 * OUTPUT:  None
 * RETURN:  DHCP_MGR_FAIL/DHCP_MGR_OK
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCP_MGR_Destroy_If(UI32_T vid_ifindex);
/* end 2007-12 */



#if (SYS_CPNT_DNS_FROM_DHCP == TRUE || SYS_CPNT_DNS == TRUE)
BOOL_T DHCP_MGR_SetClientNameServer(UI32_T vid_ifindex, UI8_T name_server[SYS_ADPT_IPV4_ADDR_LEN]);
BOOL_T DHCP_MGR_DeleteClientNameServer(UI32_T vid_ifindex);
#endif /*SYS_CPNT_DNS_FROM_DHCP*/

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_MGR_SetOption82Status
 *------------------------------------------------------------------------------
 * PURPOSE  : Set Dhcp option82 status : enable/disable
 * INPUT    : status you want to set. 1 :DHCP_OPTION82_DISABLE, 2 :DHCP_OPTION82_ENABLE
 * OUTPUT   : none
 * RETURN   : DHCP_MGR_OK : If success
 *			  DHCP_MGR_FAIL:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
 UI32_T DHCP_MGR_SetOption82Status(UI32_T status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_MGR_SetOption82Policy
 *------------------------------------------------------------------------------
 * PURPOSE  : Set Dhcp option82 policy : drop/replace/keep
 * INPUT    : policy you want to set. 1 :DHCP_OPTION82_POLICY_DROP, 2 :DHCP_OPTION82_POLICY_REPLACE
                   3:DHCP_OPTION82_POLICY_KEEP
 * OUTPUT   : none
 * RETURN   : DHCP_MGR_OK : If success
 *			  DHCP_MGR_FAIL:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
 UI32_T DHCP_MGR_SetOption82Policy(UI32_T policy);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_MGR_SetOption82RidMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Set Dhcp option82 remote id mode
 * INPUT    : mode you want to set: DHCP_OPTION82_RID_MAC/DHCP_OPTION82_RID_IP 
 * OUTPUT   : none
 * RETURN   : DHCP_MGR_OK : If success
 *			  DHCP_MGR_FAIL:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
 UI32_T DHCP_MGR_SetOption82RidMode(UI32_T mode);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_MGR_SetOption82RidValue
 *------------------------------------------------------------------------------
 * PURPOSE  : Set Dhcp option82 remote id value
 * INPUT    : string   --  remote id string 
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *			  FALSE:
 * NOTES    : max number of characters is 32.
 *------------------------------------------------------------------------------*/
 UI32_T DHCP_MGR_SetOption82RidValue(UI8_T *string);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_MGR_SetOption82Format
 *------------------------------------------------------------------------------
 * PURPOSE  : Set Dhcp option82 encode format
 * INPUT    : subtype_format     -- Setting value
 * OUTPUT   : none
 * RETURN   : DHCP_MGR_OK : If success
 *			  DHCP_MGR_FAIL:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
 UI32_T  DHCP_MGR_SetOption82Format(BOOL_T subtype_format);

/* FUNCTION NAME : DHCP_MGR_SetRelayServerAddress
 * PURPOSE:
 *      Set global dhcp server ip address for L2 relay agent to WA
 *
 * INPUT:
 *		ip1, ip2, ip3, ip4, ip5	-- the ip list of servers 
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	--  fail to set the relay server address.	
 *
 * NOTES: 
 *	1. The function will delete the previous setting for relay server address
 * 		and add the newly configured relay server address
 *	2. The maximun numbers of server ip are 5. If user specifies (ip1, ip2,0,0,0) which means
 *		the user only specifies 2 DHCP Relay Server.
 *	3. If user specifies (ip1, 0,ip3, ip4, ip5), (ip1,0,0,0,0)will be set to DHCP
 *		Relay Server Address List
 *
 */
UI32_T DHCP_MGR_SetRelayServerAddress(UI32_T ip1, UI32_T ip2, UI32_T ip3, UI32_T ip4, UI32_T ip5);

/* FUNCTION NAME : DHCP_MGR_SetRelayServerAddressFromSnmp
 * PURPOSE:
 *      Set global dhcp server ip address for L2 relay agent to WA from snmp
 *
 * INPUT:
 *      index      -- entry index
 *		server_ip  -- ip address of server
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	--  fail to set the relay server address.	
 *
 * NOTES: 
 *	This function is only used for SNMP, it can set relay server address for specified index
 *  If there's zero ip address in entry whose index is lower than specified index, we can't let user to set from snmp.
 */
UI32_T DHCP_MGR_SetRelayServerAddressFromSnmp(UI32_T index, UI32_T server_ip);

/* FUNCTION NAME : DHCP_MGR_GetRelayServerAddressFromSnmp
 * PURPOSE:
 *      Get global dhcp server ip address for L2 relay agent to WA from snmp
 *
 * INPUT:
 *      index      -- entry index
 *
 * OUTPUT:
 *      server_ip  -- ip address of server
 *
 * RETURN:
 *		DHCP_MGR_OK		-- successfully.
 *		DHCP_MGR_FAIL	--  fail to set the relay server address.	
 *
 * NOTES: 
 *	This function is only used for SNMP, it can get relay server address for specified index
 */
UI32_T DHCP_MGR_GetRelayServerAddressFromSnmp(UI32_T index, UI32_T *server_ip);

/* FUNCTION NAME : DHCP_MGR_DeleteGlobalRelayServerAddress
 * PURPOSE:
 *      Delete global Server addresses for DHCP L2 Relay Agent in WA
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *		DHCP_MGR_OK	-- successfully.
 *		DHCP_MGR_FAIL	--  FAIL to delete all relay server addresses.
 *
 * NOTES:1. For add relay server address, we can add all server IPs at
 *			one time. If we modify the server address twice, the last update
 *			would replace the previous setting.
 *
 *		 2. For delete relay server IP, we only allow delete all at a time
 *
 */
UI32_T DHCP_MGR_DeleteGlobalRelayServerAddress();

#endif

/* FUNCTION	NAME : DHCP_MGR_ReleaseClientLease
 * PURPOSE:
 *		Release dhcp active client lease
 *
 * INPUT:
 *		ifindex         --  vlan interface index  
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		DHCP_MGR_OK	    --  successfully.
 *		DHCP_MGR_FAIL	--  fail.    
 *      
 *
 * NOTES:
 *		this api will free dhcp engine's active lease and send DHCPRELEASE packet to dhcp server.
 *
 */
UI32_T DHCP_MGR_ReleaseClientLease(UI32_T ifindex);


/* FUNCTION	NAME : DHCP_MGR_SignalRifDestroy
 * PURPOSE:
 *		ipcfg signal DHCP rif is destroyed
 *
 * INPUT:
 *		vid_ifindex     -- vlan interface index
 *      addr            -- rif information
 * OUTPUT:
 *		N/A
 *
 * RETURN:
 *		N/A   
 *      
 *
 * NOTES:
 *		
 *
 */
void DHCP_MGR_SignalRifDestroy(UI32_T vid_ifindex, L_INET_AddrIp_T *addr);

/* FUNCTION	NAME : DHCP_MGR_SignalRifUp
 * PURPOSE:
 *		ipcfg signal DHCP rif is active
 *
 * INPUT:
 *		vid_ifindex     -- vlan interface index
 *      addr            -- rif information
 * OUTPUT:
 *		N/A
 *
 * RETURN:
 *		N/A 
 *      
 *
 * NOTES:
 *		
 *
 */
void DHCP_MGR_SignalRifUp(UI32_T vid_ifindex, L_INET_AddrIp_T *addr);

/* FUNCTION NAME : DHCP_MGR_IsSocketDirty
 * PURPOSE:
 *      To retrieve socket dirty status from engine
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
BOOL_T DHCP_MGR_IsSocketDirty(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_MGR_UpdateUCData
 * ------------------------------------------------------------------------
 * PURPOSE  : This function notify DHCP to update UC data when UC size is changed
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void DHCP_MGR_UpdateUCData(void);
#endif /* end of #ifndef _DHCP_MGR_H */
