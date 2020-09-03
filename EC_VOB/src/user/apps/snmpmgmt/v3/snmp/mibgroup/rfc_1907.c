/*
 *  System MIB group implementation - system.c
 *
 */
/*
--------------------------------------------------
--Version Number:20040318001
--EPR#:Mercury_V3-00743
--HeadLine:(ES4649-32) SNMP: set sysContact, sysName, sysLocation with a very long string (longer than 255) will casue
--exception
--root cause: in file rfc_1907.c , writeSystem function, variable buffer[SYS_STRING_LEN]
--it is declared as max length 256, if setting this buffer with a str longer than 256, will cause memory corrupt.
--resolution: add a mib variables in mibtree and mibfiles, also need to implemnet in c files
--modifed files: modify file rfc_1907.c , writeSystem function to fix it.
--Profile:ACP_V3
--------------------------------------------------
*/
#include <net-snmp/net-snmp-config.h>

#if HAVE_STDLIB_H
//#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_STRING_H
//#include <string.h>
#else
#include <strings.h>
#endif
//#include <sys/types.h>
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

//#include <ctype.h>
#if HAVE_UTSNAME_H
#include <utsname.h>
#else
#if HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "sysfun.h"
#include "util_funcs.h"
#include "rfc_1907.h"
#include "struct.h"
#include "sysORTable.h"
#include "trap_mgr.h"
#include "mib2_mgr.h"
#include "mib2_pmgr.h"
#include "mib2_pom.h"
#include "sys_type.h"
#include "snmp_mgr.h"
#include "stktplg_mgr.h"
#include "l_stdlib.h"
#include "l_charset.h"
#include "sysfun.h"
#include "sys_time.h"

        /*********************
	 *
	 *  Kernel & interface information,
	 *   and internal forward declarations
	 *
	 *********************/

#define SYS_STRING_LEN	256

oid             version_sysoid[] = { SYSTEM_MIB };

WriteMethod     writeSystem;



        /*********************
	 *
	 *  Initialisation & common implementation functions
	 *
	 *********************/

/*
 * define the structure we're going to ask the agent to register our
 * information at
 */
struct variable1 system_variables[] = {
    {VERSION_DESCR, ASN_OCTET_STR, RONLY, var_system, 1, {1}},
    {VERSIONID, ASN_OBJECT_ID, RONLY, var_system, 1, {2}},
    {UPTIME, ASN_TIMETICKS, RONLY, var_system, 1, {3}},
    {SYSCONTACT, ASN_OCTET_STR, RWRITE, var_system, 1, {4}},
    {SYSTEMNAME, ASN_OCTET_STR, RWRITE, var_system, 1, {5}},
    {SYSLOCATION, ASN_OCTET_STR, RWRITE, var_system, 1, {6}},
    {SYSSERVICES, ASN_INTEGER, RONLY, var_system, 1, {7}},
    {SYSORLASTCHANGE, ASN_TIMETICKS, RONLY, var_system, 1, {8}}
};
/*
 * Define the OID pointer to the top of the mib tree that we're
 * registering underneath
 */
oid             system_variables_oid[] = { SNMP_OID_MIB2, 1 };
oid             system_module_oid[] = { SNMP_OID_SNMPMODULES, 1 };
int             system_module_oid_len =
    sizeof(system_module_oid) / sizeof(oid);
int             system_module_count = 0;

void
init_system_mib(void)
{

    /*
     * register ourselves with the agent to handle our mib tree
     */
    REGISTER_MIB("mibII/system", system_variables, variable1,
                 system_variables_oid);

    if (++system_module_count == 3)
        REGISTER_SYSOR_ENTRY(system_module_oid,
                             "The MIB module for SNMPv2 entities");

}


        /*********************
	 *
	 *  System specific implementation functions
	 *
	 *********************/

#ifdef USING_MIBII_SYSORTABLE_MODULE
extern struct timeval sysOR_lastchange;
#endif

u_char         *
var_system(struct variable *vp,
           oid * name,
           size_t * length,
           int exact, size_t * var_len, WriteMethod ** write_method)
{
    static u_long   ulret;
    char  buffer[SYS_STRING_LEN] = { 0 };
    STK_UNIT_CFG_T device_info;

    memset(&device_info,0,sizeof(device_info));

    if (header_generic(vp, name, length, exact, var_len, write_method) ==
        MATCH_FAILED)
        return NULL;

    switch (vp->magic)
     {
    case VERSION_DESCR:
     {
     //  test_createtrapreceiver();
        //test_getnextgroup();
        //test_getnextviewname();

        if (MIB2_POM_GetSysDescr((UI8_T *)buffer) != TRUE)
    	{
    	   return NULL;
    	}
        *var_len = strlen(buffer);
        strcpy((char *)return_buf, buffer);

        return (u_char *) return_buf;
    }
    case VERSIONID:
    {

          static oid  oid_buf[32];
          UI32_T idx;
          // test_createsnmptargetparamstable();
       //   test_getnexttrapreceiver();
            if (MIB2_PMGR_GetSysObjectID( oid_buf, &idx)!= TRUE)
            return NULL;
         else
         {
          *var_len = idx* sizeof(oid);

          return (u_char *) oid_buf;

        }
    }
    case UPTIME:
    {
      //test_destoryuser1();
        //test_creategroup2();
      //test_deleteparamstable1();
     // test_removetrapreceiver1();
        UI32_T ticks;
        SYS_TIME_GetSystemUpTimeByTick(&ticks);
        ulret = ticks;
        return (u_char *) & ulret;
    }
    case SYSCONTACT:
    {
       // test_destoryuser2();
        // test_creategroup3();
        //test_createuser2();
       // test_deleteparamstable2();
      // test_removetrapreceiver2();
        *write_method = writeSystem;
          if (MIB2_POM_GetSysContact((UI8_T *)buffer) != TRUE)
          {
                return NULL;
           }
        *var_len = strlen(buffer);
         strcpy((char *)return_buf, buffer);
        return (u_char *) return_buf;

    }
    case SYSTEMNAME:
     {
        //test_getnextuser();
        //test_destroygroup1();
       // test_createcommunity1();
      // test_deleteparamstable3();
     // test_removetrapreceiver3();
        *write_method = writeSystem;
        if (MIB2_POM_GetSysName((UI8_T *)buffer) != TRUE)
        {
            return NULL;
        }
        else
        {
           *var_len = strlen(buffer);
           strcpy((char *)return_buf, buffer);
           return (u_char *) return_buf;
        }
    }
    case SYSLOCATION:
    {

        *write_method = writeSystem;
         //test_destroygroup2();
        // test_deleteparamstable4();
        if (MIB2_POM_GetSysLocation((UI8_T *)buffer)!= TRUE)
           {
           	return NULL;
           }
        else
         {
              *var_len = strlen(buffer);
              strcpy((char *)return_buf, buffer);
              return (u_char *) return_buf;
          }
    }
    case SYSSERVICES:
    {
       UI8_T  sysServices;
        //test_destroygroup3();
        //test_getnextuserbygroup1();
        if (MIB2_POM_GetSysServices(&sysServices) !=TRUE)
        {
            return NULL;
         }
       else
       {
         ulret = sysServices;
          return (u_char *) & ulret;
        }
     }


    case SYSORLASTCHANGE:
     //  test_createsocket();
       //printf(" why\n");
        //test_getnextuserbygroup2();
        ulret = netsnmp_timeval_uptime(&sysOR_lastchange);
        return ((u_char *) & ulret);

    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_system\n",
                    vp->magic));
    }
    return NULL;
}



int
writeSystem(
    int action,
    u_char * var_val,
    u_char var_val_type,
    size_t var_val_len,
    u_char * statP,
    oid * name,
    size_t name_len)
{

    // char *buf = NULL, *oldbuf = NULL;
    int  *setvar = NULL;
    char buffer[SYS_STRING_LEN];

    switch (action)
    {
        case RESERVE1:
            /* Check values for acceptability
             */
            if (var_val_type != ASN_OCTET_STR)
            {
                snmp_log(LOG_ERR, "not string\n");
                return SNMP_ERR_WRONGTYPE;
            }

            if (var_val_len > SYS_STRING_LEN - 1)
            {
                //snmp_log(LOG_ERR, "bad length\n");
                //SYSFUN_Debug_Printf("bad length from MIB1907 Write_sytem\n");
                return SNMP_ERR_WRONGLENGTH;
            }

            memcpy( buffer, var_val, var_val_len);
            buffer[var_val_len]=0;

            if (!L_STDLIB_StrIsAsciiPrintWithCount(buffer, var_val_len))
            {
                return SNMP_ERR_WRONGVALUE;
            }

            if (NULL != (strchr(buffer, '?')))
            {
                return SNMP_ERR_WRONGVALUE;
            }

            if (setvar != NULL && *setvar < 0)
            {
                /* The object is set in a read-only configuration file.
                 */
                return SNMP_ERR_NOTWRITABLE;
            }

            break;

        case RESERVE2:
            /* Allocate memory and similar resources
             */
            /* Using static strings, so nothing needs to be done
             */
            break;

        case ACTION:
            /* Perform the SET action (if reversible)
             */
            memcpy( buffer, var_val, var_val_len);
            buffer[var_val_len]=0;

            switch ((char) name[7])
            {
                case SYSCONTACT:

                    if (MIB2_PMGR_SetSysContact((UI8_T *)buffer) != TRUE)
                        return SNMP_ERR_GENERR;

                        break;

                case SYSTEMNAME:

                    if (MIB2_PMGR_SetHostName((UI8_T *)buffer) != TRUE)
                        return SNMP_ERR_GENERR;

                    break;

                case SYSLOCATION:

                    if (MIB2_PMGR_SetSysLocation((UI8_T *)buffer) != TRUE)
                        return SNMP_ERR_GENERR;

                    break;

                default:
                    return SNMP_ERR_GENERR;
            }

            break;

        case UNDO:
            /* Reverse the SET action and free resources
             */
#if 0 /*not use*/
            strcpy(buf, oldbuf);
            oldbuf[0] = 0;
#endif
            break;

        case COMMIT:

            if (setvar != NULL)
            {
                *setvar = 1;
            }

#if 0
            snmp_save_persistent(netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_APPTYPE));
            (void) snmp_call_callbacks(SNMP_CALLBACK_LIBRARY, SNMP_CALLBACK_STORE_DATA, NULL);
            snmp_clean_persistent(netsnmp_ds_get_string (NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_APPTYPE));
#endif

        case FREE:
            /* Free any resources allocated
             */
#if 0 /*not use*/
            /* No resources have been allocated, but "empty" the 'oldbuf'
             */
            oldbuf[0] = 0;
#endif
            break;

    }

    return SNMP_ERR_NOERROR;
}                               /* end of writeSystem */

/*********************
 *
 *  Initialisation & common implementation functions
 *
 *********************/

/*
 * define the structure we're going to ask the agent to register our
 * information at
 */
struct variable1 snmp_variables[] = {
    {SNMPINPKTS, ASN_COUNTER, RONLY, var_snmp, 1, {1}},
    {SNMPOUTPKTS, ASN_COUNTER, RONLY, var_snmp, 1, {2}},
    {SNMPINBADVERSIONS, ASN_COUNTER, RONLY, var_snmp, 1, {3}},
    {SNMPINBADCOMMUNITYNAMES, ASN_COUNTER, RONLY, var_snmp, 1, {4}},
    {SNMPINBADCOMMUNITYUSES, ASN_COUNTER, RONLY, var_snmp, 1, {5}},
    {SNMPINASNPARSEERRORS, ASN_COUNTER, RONLY, var_snmp, 1, {6}},
    {SNMPINTOOBIGS, ASN_COUNTER, RONLY, var_snmp, 1, {8}},
    {SNMPINNOSUCHNAMES, ASN_COUNTER, RONLY, var_snmp, 1, {9}},
    {SNMPINBADVALUES, ASN_COUNTER, RONLY, var_snmp, 1, {10}},
    {SNMPINREADONLYS, ASN_COUNTER, RONLY, var_snmp, 1, {11}},
    {SNMPINGENERRS, ASN_COUNTER, RONLY, var_snmp, 1, {12}},
    {SNMPINTOTALREQVARS, ASN_COUNTER, RONLY, var_snmp, 1, {13}},
    {SNMPINTOTALSETVARS, ASN_COUNTER, RONLY, var_snmp, 1, {14}},
    {SNMPINGETREQUESTS, ASN_COUNTER, RONLY, var_snmp, 1, {15}},
    {SNMPINGETNEXTS, ASN_COUNTER, RONLY, var_snmp, 1, {16}},
    {SNMPINSETREQUESTS, ASN_COUNTER, RONLY, var_snmp, 1, {17}},
    {SNMPINGETRESPONSES, ASN_COUNTER, RONLY, var_snmp, 1, {18}},
    {SNMPINTRAPS, ASN_COUNTER, RONLY, var_snmp, 1, {19}},
    {SNMPOUTTOOBIGS, ASN_COUNTER, RONLY, var_snmp, 1, {20}},
    {SNMPOUTNOSUCHNAMES, ASN_COUNTER, RONLY, var_snmp, 1, {21}},
    {SNMPOUTBADVALUES, ASN_COUNTER, RONLY, var_snmp, 1, {22}},
    {SNMPOUTGENERRS, ASN_COUNTER, RONLY, var_snmp, 1, {24}},
    {SNMPOUTGETREQUESTS, ASN_COUNTER, RONLY, var_snmp, 1, {25}},
    {SNMPOUTGETNEXTS, ASN_COUNTER, RONLY, var_snmp, 1, {26}},
    {SNMPOUTSETREQUESTS, ASN_COUNTER, RONLY, var_snmp, 1, {27}},
    {SNMPOUTGETRESPONSES, ASN_COUNTER, RONLY, var_snmp, 1, {28}},
    {SNMPOUTTRAPS, ASN_COUNTER, RONLY, var_snmp, 1, {29}},
    {SNMPENABLEAUTHENTRAPS, ASN_INTEGER, RWRITE, var_snmp, 1, {30}},
    {SNMPSILENTDROPS, ASN_COUNTER, RONLY, var_snmp, 1, {31}},
    {SNMPPROXYDROPS, ASN_COUNTER, RONLY, var_snmp, 1, {32}}
};

/*
 * Define the OID pointer to the top of the mib tree that we're
 * registering underneath
 */
oid             snmp_variables_oid[] = { SNMP_OID_MIB2, 11 };
#ifdef USING_MIBII_SYSTEM_MIB_MODULE
extern oid      system_module_oid[];
extern int      system_module_oid_len;
extern int      system_module_count;
#endif

#if 0
static int
snmp_enableauthentraps_store(int a, int b, void *c, void *d)
{
    char            line[SNMP_MAXBUF_SMALL];

    if (snmp_enableauthentrapsset > 0) {
        snprintf(line, SNMP_MAXBUF_SMALL, "pauthtrapenable %d",
                 snmp_enableauthentraps);
        snmpd_store_config(line);
    }
    return 0;
}
#endif

void
init_snmp_mib(void)
{
    /*
     * register ourselves with the agent to handle our mib tree
     */
    REGISTER_MIB("mibII/snmp", snmp_variables, variable1,
                 snmp_variables_oid);

#ifdef USING_MIBII_SYSTEM_MIB_MODULE
    if (++system_module_count == 3)
        REGISTER_SYSOR_TABLE(system_module_oid, system_module_oid_len,
                             "The MIB module for SNMPv2 entities");
#endif
#if 0
    snmp_register_callback(SNMP_CALLBACK_LIBRARY, SNMP_CALLBACK_STORE_DATA,
                           snmp_enableauthentraps_store, NULL);
#endif
}

/*
 * header_snmp(...
 * Arguments:
 * vp     IN      - pointer to variable entry that points here
 * name    IN/OUT  - IN/name requested, OUT/name found
 * length  IN/OUT  - length of IN/OUT oid's
 * exact   IN      - TRUE if an exact match was requested
 * var_len OUT     - length of variable or 0 if function returned
 * write_method
 *
 */

        /*********************
	 *
	 *  System specific implementation functions
	 *	(actually common!)
	 *
	 *********************/


u_char         *
var_snmp(struct variable *vp,
         oid * name,
         size_t * length,
         int exact, size_t * var_len, WriteMethod ** write_method)
{
    static long     long_ret;

    *write_method = 0;          /* assume it isnt writable for the time being */
    *var_len = sizeof(long_ret);        /* assume an integer and change later if not */

    if (header_generic(vp, name, length, exact, var_len, write_method)
        == MATCH_FAILED)
        return NULL;

    /*
     * this is where we do the value assignments for the mib results.
     */
    if (vp->magic == SNMPENABLEAUTHENTRAPS)
    {
    	UI8_T authtrap;
        *write_method = write_snmp;
        if (SNMP_MGR_GetSnmpEnableAuthenTraps(&authtrap)!=TRUE)
        {
            return NULL;
        }
        long_return = authtrap;
        return (u_char *) & long_return;
    } else if ((vp->magic >= 1)
               && (vp->magic <=
                   (STAT_SNMP_STATS_END - STAT_SNMP_STATS_START + 1))) {
        long_ret =
            snmp_get_statistic(vp->magic + STAT_SNMP_STATS_START - 1);
        return (unsigned char *) &long_ret;
    }
    return NULL;
}

/*
 * only for snmpEnableAuthenTraps:
 */


int
write_snmp(int action,
           u_char * var_val,
           u_char var_val_type,
           size_t var_val_len, u_char * statP, oid * name, size_t name_len)
{
    long            intval = 0;

    switch (action) {
    case RESERVE1:             /* Check values for acceptability */
        if (var_val_type != ASN_INTEGER) {
            DEBUGMSGTL(("mibII/snmp_mib", "%x not integer type",
                        var_val_type));
            return SNMP_ERR_WRONGTYPE;
        }

        intval = *((long *) var_val);
        if (intval != 1 && intval != 2) {
            DEBUGMSGTL(("mibII/snmp_mib", "not valid %x\n", intval));
            return SNMP_ERR_WRONGVALUE;
        }

        break;

    case RESERVE2:             /* Allocate memory and similar resources */

        /*
         * Using static variables, so nothing needs to be done
         */
        break;

    case ACTION:               /* Perform the SET action (if reversible) */

        /*
         * Save the old value, in case of UNDO
         */
         intval = * (long *) var_val;
         if (SNMP_MGR_SetSnmpEnableAuthenTraps(intval) != TRUE)
            return SNMP_ERR_COMMITFAILED;
         break;

    case UNDO:                 /* Reverse the SET action and free resources */

       // snmp_enableauthentraps = old_snmp_enableauthentraps;
        break;

    case COMMIT:
       #if 0
        snmp_enableauthentrapsset = 1;
        snmp_save_persistent(netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_APPTYPE));
        (void) snmp_call_callbacks(SNMP_CALLBACK_LIBRARY,
                                   SNMP_CALLBACK_STORE_DATA, NULL);
        snmp_clean_persistent(netsnmp_ds_get_string
                              (NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_APPTYPE));
       #endif
        break;

    case FREE:                 /* Free any resources allocated */
        break;
    }
    return SNMP_ERR_NOERROR;
}

        /*********************
	 *
	 *  Internal implementation functions - None
	 *
	 *********************/


#if 0
void test_getnexttrapreceiver()
{
	SNMP_MGR_TrapDestEntry_T entry;
	int count;
	memset(&entry, 0, sizeof(entry));

	while (SNMP_MGR_GetNextTrapReceiver(&entry) == SNMP_MGR_ERROR_OK)
	{

	 SYSFUN_Debug_Printf(" trapDestIP = %d\r\n", entry.snmp_mgr_trap_dest_address);
	 SYSFUN_Debug_Printf(" trapDestComm = %s\r\n", entry.snmp_mgr_trap_dest_community);
	 SYSFUN_Debug_Printf(" trapDestPort = %d\r\n", entry.snmp_mgr_trap_dest_port);
	  SYSFUN_Debug_Printf(" trapDestVer= %d\r\n", entry.snmp_mgr_trap_dest_version);
	   SYSFUN_Debug_Printf(" trapDestType = %d\r\n", entry.snmp_mgr_trap_dest_type);
	}

}

void test_removetrapreceiver1()
{
	UI32_T ip = 0x0a070101;
	SNMP_MGR_DeleteTrapReceiver(ip);
}

void test_removetrapreceiver2()
{
	UI32_T ip = 0x0a070102;
	SNMP_MGR_DeleteTrapReceiver(ip);
}

void test_removetrapreceiver3()
{
	UI32_T ip = 0x0a070103;
	SNMP_MGR_DeleteTrapReceiver(ip);
}
void test_createtrapreceiver()
{
    SNMP_MGR_TrapDestEntry_T entry;

    entry.snmp_mgr_trap_dest_address= 0x0a0204df;


    strcpy(entry.snmp_mgr_trap_dest_community, "public");
    entry.snmp_mgr_trap_dest_port = 162;
    entry.snmp_mgr_trap_dest_version = SNMP_MGR_SNMPV3_MODEL_V1;
    entry.snmp_mgr_trap_dest_type = SNMP_MGR_SNMPV3_NOTIFY_TRAP;
    SNMP_MGR_SetTrapReceiver(&entry);

    /* create the second host*/
    entry.snmp_mgr_trap_dest_address= 0x0a070101;
    SNMP_MGR_SetTrapReceiver(&entry);

    /* create the third_host */
    entry.snmp_mgr_trap_dest_address= 0x0a070102;
    entry.snmp_mgr_trap_dest_type = SNMP_MGR_SNMPV3_NOTIFY_INFORM;
    SNMP_MGR_SetTrapReceiver(&entry);


}
void test_getnextsnmptargetaddrtable()
{
	SNMP_MGR_SnmpTargetAddrEntry_T entry;

	memset(&entry, 0, sizeof(entry));
	 while( SNMP_MGR_GetNextSnmpTargetAddrTable( &entry)== SNMP_MGR_ERROR_OK)
        {
        SYSFUN_Debug_Printf( "snmp_target_addr_name  = %s\n", entry.snmp_target_addr_name);
        SYSFUN_Debug_Printf(" snmp_target_addr_tdomain_len =   %d\n", entry.snmp_target_addr_tdomain_len);
        SYSFUN_Debug_Printf(" snmp_target_addr_timeout =     %d\n", entry.snmp_target_addr_timeout);
        SYSFUN_Debug_Printf(" snmp_target_addr_retry_count = %d\n", entry.snmp_target_addr_retry_count);
        SYSFUN_Debug_Printf( "snmp_target_addr_tag_list  = %s\n", entry.snmp_target_addr_tag_list);
        SYSFUN_Debug_Printf( "snmp_target_addr_params  = %s\n", entry.snmp_target_addr_params);
      }
}

void test_createsnmptargetaddrtable()
{
    SNMP_MGR_SnmpTargetAddrEntry_T entry;
    oid    udp_domain_oid[7] = {1,3,6,1,6,1,1};
    UI8_T     address[6] = {10, 2,4,223,0, 162};


    strcpy( entry.snmp_target_addr_name, "traphost.10.2.4.223.public");
    memcpy(entry.snmp_target_addr_tdomain, udp_domain_oid, 7*sizeof(oid));


    entry.snmp_target_addr_tdomain_len = 7;
    memcpy(entry.snmp_target_addr_taddress,address,6);
    entry.snmp_target_addr_timeout=1500;
    entry.snmp_target_addr_retry_count = 0;
    strcpy(entry.snmp_target_addr_tag_list, "trap");
    strcpy(entry.snmp_target_addr_params, "traphost.10.2.4.223.public");
    entry.snmp_target_addr_storage_type = SNMP_MGR_SNMPV3_STORAGE_TYPE_NONVOLATILE;
    entry.snmp_target_addr_row_status = SNMP_MGR_SNMPV3_ROWSTATUS_TYPE_ACTIVE ;
    SNMP_MGR_CreateSnmpTargetAddrTable(&entry);

}



void test_getnextsnmptargetparamstable()
{
	SNMP_MGR_SnmpTargetParamsEntry_T entry;

	memset(&entry, 0, sizeof(entry));
	 while( SNMP_MGR_GetNextSnmpTargetParamsTable( &entry)== SNMP_MGR_ERROR_OK)
        {
        SYSFUN_Debug_Printf( "snmp_target_params_name  = %s\n", entry.snmp_target_params_name );
        SYSFUN_Debug_Printf(" snmp_target_params_mp_model =   %d\n", entry.snmp_target_params_mp_model);
        SYSFUN_Debug_Printf(" snmp_target_params_security_model =     %d\n", entry.snmp_target_params_security_model);
        SYSFUN_Debug_Printf(" snmp_target_params_scurity_level = %d\n", entry.snmp_target_params_security_level);
        SYSFUN_Debug_Printf( "snmp_target_params_storage_type  = %s\n", entry.snmp_target_params_storage_type);
        SYSFUN_Debug_Printf( "snmp_target_params_row_status  = %s\n", entry.snmp_target_params_row_status);
      }
}

void test_createsnmptargetparamstable()
{
    SNMP_MGR_SnmpTargetParamsEntry_T entry;



    strcpy( entry.snmp_target_params_name, "traphost.10.2.4.223.public");

    entry.snmp_target_params_mp_model = 1;
    entry.snmp_target_params_security_model=3;
    strcpy(entry.snmp_target_params_security_name, "trap");
    entry.snmp_target_params_security_level = 3;
    entry.snmp_target_params_storage_type = 3 ;
    entry.snmp_target_params_row_status = 1;
    SNMP_MGR_CreateSnmpTargetParamsTable(&entry);

}


void test_createnotifytable()
{
    SNMP_MGR_SnmpNotifyEntry_T entry;


    strcpy( entry.snmp_notify_name, "trap");
    strcpy(entry.snmp_notify_tag, "tag0");
    entry.snmp_notify_type = SNMP_MGR_SNMPV3_NOTIFY_TRAP;
    entry.snmp_notify_storage_type = SNMP_MGR_SNMPV3_STORAGE_TYPE_PERMANENT;
    entry.snmp_notify_row_status = 1;
    SNMP_MGR_CreateSnmpNotifyTable(&entry);



}


void test_deleteparamstable1()
{

      SNMP_MGR_DeleteSnmpTargetParamsTable( "t");

}

void test_deleteparamstable2()
{

      SNMP_MGR_DeleteSnmpTargetParamsTable( "tr");

}

void test_deleteparamstable3()
{

      SNMP_MGR_DeleteSnmpTargetParamsTable( "tra");

}

void test_deleteparamstable4()
{

      SNMP_MGR_DeleteSnmpTargetParamsTable( "trap");

}


void test_deletetargettable1()
{

      SNMP_MGR_DeleteSnmpTargetAddrTable( "t");

}

void test_deletetargettable2()
{

      SNMP_MGR_DeleteSnmpTargetAddrTable( "tr");

}

void test_deletetargettable3()
{

      SNMP_MGR_DeleteSnmpTargetAddrTable( "tra");

}

void test_deletetargettable4()
{

      SNMP_MGR_DeleteSnmpTargetAddrTable( "trap");

}

void test_deletenotifytable1()
{

      SNMP_MGR_DeleteSnmpNotifyTable( "t");

}

void test_deletenotifytable2()
{

      SNMP_MGR_DeleteSnmpNotifyTable( "tr");

}

void test_deletenotifytable3()
{

      SNMP_MGR_DeleteSnmpNotifyTable( "tra");

}

void test_deletenotifytable4()
{

      SNMP_MGR_DeleteSnmpNotifyTable( "trap");

}
void testgetnextnotifytable()
{
	SNMP_MGR_SnmpNotifyEntry_T entry;

	memset(&entry, 0, sizeof(entry));
	 while( SNMP_MGR_GetNextSnmpNotifyTable( &entry)== SNMP_MGR_ERROR_OK)
        {
        SYSFUN_Debug_Printf( "notifyname  = %s\n", entry.snmp_notify_name);
        SYSFUN_Debug_Printf(" notifytag = %s\n", entry.snmp_notify_tag);
        SYSFUN_Debug_Printf(" type =   %d\n", entry.snmp_notify_type);
        SYSFUN_Debug_Printf(" storage =     %d\n", entry.snmp_notify_storage_type);
        SYSFUN_Debug_Printf(" status = %d\n", entry.snmp_notify_row_status);
      }


}
void test_createuser1()
{

	SNMP_MGR_SnmpV3UserEntry_T entry;
	memset( &entry, 0, sizeof(entry));


	strcpy( entry.snmpv3_user_name, "kinghong");
	strcpy(entry.snmpv3_user_group_name, "kinghongG");
	strcpy(entry.snmpv3_user_auth_password, "kinghong");
	strcpy(entry.snmpv3_user_priv_password, "kinghong");
	entry.snmpv3_user_security_model = SNMP_MGR_SNMPV3_MODEL_V3;
	entry.snmpv3_user_security_level=SNMP_MGR_SNMPV3_SECURITY_LEVEL_AUTHPRIV;
	entry.snmpv3_user_auth_type = SNMP_MGR_SNMPV3_AUTHTYPE_MD5;
	entry.snmpv3_user_priv_type = SNMP_MGR_SNMPV3_PRIVTYPE_DES;

	entry.password_from_config = FALSE;
        SNMP_MGR_CreateSnmpV3User(&entry);
}

void test_createuser2()
{

    SNMP_MGR_SnmpV3UserEntry_T entry;
    memset( &entry, 0, sizeof(entry));


    strcpy( entry.snmpv3_user_name, "phoebe");
    strcpy(entry.snmpv3_user_group_name, "kinghongG");
    strcpy(entry.snmpv3_user_auth_password, "kinghong");
    strcpy(entry.snmpv3_user_priv_password, "kinghong");
    entry.snmpv3_user_security_model = SNMP_MGR_SNMPV3_MODEL_V3;
    entry.snmpv3_user_security_level=SNMP_MGR_SNMPV3_SECURITY_LEVEL_AUTHPRIV;
    entry.snmpv3_user_auth_type = SNMP_MGR_SNMPV3_AUTHTYPE_MD5;
    entry.snmpv3_user_priv_type = SNMP_MGR_SNMPV3_PRIVTYPE_DES;

    entry.password_from_config = FALSE;
    SNMP_MGR_CreateSnmpV3User(&entry);
}


void test_destroycommunity1()
{

	SNMP_MGR_RemoveSnmpCommunity("1");
	SNMP_MGR_RemoveSnmpCommunity("2");


}


void test_destroycommunity2()
{
                SNMP_MGR_RemoveSnmpCommunity("3");
	SNMP_MGR_RemoveSnmpCommunity("4");
	SNMP_MGR_RemoveSnmpCommunity("5");
}



void test_createcommunity2()
{
	    if ( SNMP_MGR_CreateSnmpCommunity( "3", SNMP_MGR_ACCESS_RIGHT_READ_ONLY)!= SNMP_MGR_ERROR_OK)

                      {
	       SYSFUN_Debug_Printf(" comm 3 create fail\n");
	   }
	   else
	   {
	      SYSFUN_Debug_Printf(" comm 3 create success\n");
	   }

                     if (SNMP_MGR_CreateSnmpCommunity( "4", SNMP_MGR_ACCESS_RIGHT_READ_WRITE)!= SNMP_MGR_ERROR_OK)
                       {
	       SYSFUN_Debug_Printf(" comm 4 create fail\n");
	   }
	   else
	   {
	      SYSFUN_Debug_Printf(" comm 4 create success\n");
	   }
                    if (  SNMP_MGR_CreateSnmpCommunity( "5", SNMP_MGR_ACCESS_RIGHT_READ_ONLY)!= SNMP_MGR_ERROR_OK)
                      {
	       SYSFUN_Debug_Printf(" comm 5 create fail\n");
	   }
	   else
	   {
	      SYSFUN_Debug_Printf(" comm 5 create success\n");
	   }

}

void test_createcommunity1()

{
	  if ( SNMP_MGR_CreateSnmpCommunity( "netman1", SNMP_MGR_ACCESS_RIGHT_READ_ONLY)!= SNMP_MGR_ERROR_OK)
	  {
	       SYSFUN_Debug_Printf(" comm 1 create fail\n");
	   }
	   else
	   {
	      SYSFUN_Debug_Printf(" comm 1 create success\n");
	   }
                     if (SNMP_MGR_CreateSnmpCommunity( "netman2", SNMP_MGR_ACCESS_RIGHT_READ_WRITE)!= SNMP_MGR_ERROR_OK)
                       {
	       SYSFUN_Debug_Printf(" comm 2 create fail\n");
	   }
	   else
	   {
	      SYSFUN_Debug_Printf(" comm 2 create success\n");
	   }



}


void test_createuser3()
{

    SNMP_MGR_SnmpV3UserEntry_T  entry2;


    memset( &entry2, 0, sizeof(entry2));


    strcpy( entry2.snmpv3_user_name, "kinghong3");
    strcpy(entry2.snmpv3_user_group_name, "kinghongv2group");
    strcpy(entry2.snmpv3_user_auth_password, "kinghong3");
    strcpy(entry2.snmpv3_user_priv_password, "kinghong3");
    entry2.snmpv3_user_security_model = SNMP_MGR_SNMPV3_MODEL_V2C;
    entry2.snmpv3_user_security_level=SNMP_MGR_SNMPV3_SECURITY_LEVEL_NOAUTH;
    entry2.snmpv3_user_auth_type = SNMP_MGR_SNMPV3_AUTHTYPE_SHA;
    entry2.snmpv3_user_priv_type = SNMP_MGR_SNMPV3_PRIVTYPE_DES;
    entry2.password_from_config  = FALSE;
    SNMP_MGR_CreateSnmpV3User(&entry2);
}


void test_createuserFromConfig()
{

    SNMP_MGR_SnmpV3UserEntry_T  entry2, entry3;
    int i;


    memset( &entry2, 0, sizeof(entry2));
    memset(&entry3, 0, sizeof(entry3));



    strcpy( entry3.snmpv3_user_name, "kinghong");
    snmpv3_get_engineID( entry3.snmpv3_user_engine_id, 13);

    if ( SNMP_MGR_GetSnmpV3User( &entry3)!= SNMP_MGR_ERROR_OK)
    {
        SYSFUN_Debug_Printf(" getuser fail\n");
        return;
    }



    strcpy( entry2.snmpv3_user_name, "kinghongtmp");
    strcpy(entry2.snmpv3_user_group_name, "kinghongG");
    entry2.password_from_config = TRUE;

    memcpy( entry2.snmpv3_user_auth_key, entry3.snmpv3_user_auth_key, entry3.snmpv3_user_auth_key_len);

    for ( i = 0; i <= entry3.snmpv3_user_priv_key_len; i ++)
    {
        SYSFUN_Debug_Printf("Total len = %d, priv_key[%d]= %x\n", entry3.snmpv3_user_auth_key_len, i, entry2.snmpv3_user_auth_key[i]);
    }
    memcpy(entry2.snmpv3_user_priv_key, entry3.snmpv3_user_priv_key, entry3.snmpv3_user_priv_key_len);
    for ( i = 0; i <= entry3.snmpv3_user_priv_key_len; i ++)
    {
        SYSFUN_Debug_Printf("Total len = %d, priv_key[%d]= %x\n",entry3.snmpv3_user_priv_key_len,i, entry2.snmpv3_user_priv_key[i]);
    }
    entry2.snmpv3_user_auth_key_len = entry3.snmpv3_user_auth_key_len;
    entry2.snmpv3_user_priv_key_len = entry3.snmpv3_user_priv_key_len;
    //strcpy(entry2.snmpv3_user_auth_password, "kinghong3");
    //strcpy(entry2.snmpv3_user_priv_password, "kinghong3");
    entry2.snmpv3_user_security_model = SNMP_MGR_SNMPV3_MODEL_V3;
    entry2.snmpv3_user_security_level=SNMP_MGR_SNMPV3_SECURITY_LEVEL_AUTHPRIV;
    entry2.snmpv3_user_auth_type = SNMP_MGR_SNMPV3_AUTHTYPE_MD5;
    entry2.snmpv3_user_priv_type = SNMP_MGR_SNMPV3_PRIVTYPE_DES;

    SNMP_MGR_CreateSnmpV3User(&entry2);
}

void test_destoryuser1()
{

    SNMP_MGR_DeleteSnmpV3User( SNMP_MGR_SNMPV3_MODEL_V3, "kinghong");

}

void test_destoryuser2()
{

    SNMP_MGR_DeleteSnmpV3User( SNMP_MGR_SNMPV3_MODEL_V2C, "kinghong");

}


void test_creategroup1()
{

    SNMP_MGR_SnmpV3GroupEntry_T entry;

    memset( &entry, 0, sizeof(entry));
    strcpy( entry.snmpv3_group_name, "kinghongG");
    entry.snmpv3_group_model = SNMP_MGR_SNMPV3_MODEL_V3;
    entry.snmpv3_group_security_level = SNMP_MGR_SNMPV3_SECURITY_LEVEL_NOAUTH;
    strcpy( entry.snmpv3_group_readview,  "kinghongview");
    strcpy(entry.snmpv3_group_writeview, "kinghongview");
    strcpy(entry.snmpv3_group_notifyview, "notifyview");

    SNMP_MGR_CreateSnmpV3Group(&entry);

}

void test_creategroup2()
{

    SNMP_MGR_SnmpV3GroupEntry_T entry;

    memset( &entry, 0, sizeof(entry));
    strcpy( entry.snmpv3_group_name, "abc");
    entry.snmpv3_group_model = SNMP_MGR_SNMPV3_MODEL_V3;
    entry.snmpv3_group_security_level = SNMP_MGR_SNMPV3_SECURITY_LEVEL_AUTHNOPRIV;
    strcpy( entry.snmpv3_group_readview,  "group2view");
    strcpy(entry.snmpv3_group_writeview, "group2view");
    strcpy(entry.snmpv3_group_notifyview, "notifyview");

    SNMP_MGR_CreateSnmpV3Group(&entry);

}

void test_creategroup3()
{

    SNMP_MGR_SnmpV3GroupEntry_T entry;

    memset( &entry, 0, sizeof(entry));
    strcpy( entry.snmpv3_group_name, "abc");
    entry.snmpv3_group_model = SNMP_MGR_SNMPV3_MODEL_V3;
    entry.snmpv3_group_security_level = SNMP_MGR_SNMPV3_SECURITY_LEVEL_AUTHPRIV;
    strcpy( entry.snmpv3_group_readview,  "group3view");
    strcpy(entry.snmpv3_group_writeview, "group3view");
    strcpy(entry.snmpv3_group_notifyview, "notifyview");

    SNMP_MGR_CreateSnmpV3Group(&entry);

}

void test_createview()
{
    SNMP_MGR_SnmpV3ViewEntry_T entry;

    strcpy( entry.snmpv3_view_name, "kinghongview");
    strcpy(entry.snmpv3_wildcard_subtree, "1.*");
    // strcpy(entry.snmpv3_view_mask, "");
    entry.snmpv3_view_type = SNMP_MGR_SNMPV3_VIEW_INCLUDED;
    SNMP_MGR_CreateSnmpV3View(&entry);


    strcpy( entry.snmpv3_view_name, "kinghongview");
    strcpy(entry.snmpv3_wildcard_subtree, "*.*");
    //  strcpy(entry.snmpv3_view_mask, "");
    entry.snmpv3_view_type = SNMP_MGR_SNMPV3_VIEW_INCLUDED;
    SNMP_MGR_CreateSnmpV3View(&entry);

    strcpy( entry.snmpv3_view_name, "kinghongview");
    strcpy(entry.snmpv3_wildcard_subtree, "2.*.3");
    //strcpy(entry.snmpv3_view_mask, "FF:A0");
    entry.snmpv3_view_type = SNMP_MGR_SNMPV3_VIEW_INCLUDED;
    SNMP_MGR_CreateSnmpV3View(&entry);

#if 0
    strcpy( entry.snmpv3_view_name, "kinghongview");
    strcpy(entry.snmpv3_wildcard_subtree, "1.3.6.1.2.1.2.2.1.*.2");
    //strcpy(entry.snmpv3_view_mask, "FF:A0");
    entry.snmpv3_view_type = SNMP_MGR_SNMPV3_VIEW_INCLUDED;
    SNMP_MGR_CreateSnmpV3View(&entry);


    strcpy( entry.snmpv3_view_name, "kinghongview");
    strcpy(entry.snmpv3_wildcard_subtree, "1.3.6.1.4.1.259.16.39");
    //  strcpy(entry.snmpv3_view_mask, "");
    entry.snmpv3_view_type = SNMP_MGR_SNMPV3_VIEW_INCLUDED;
    SNMP_MGR_CreateSnmpV3View(&entry);
#endif
}


void test_destroyview()
{

	SNMP_MGR_DeleteSnmpV3View("kinghongview", "1.3.6.1.6");
}

void test_destroygroup1()
{

	SNMP_MGR_DeleteSnmpV3Group("abc", SNMP_MGR_SNMPV3_MODEL_V3, SNMP_MGR_SNMPV3_SECURITY_LEVEL_NOAUTH);
}

void test_destroygroup2()
{

	SNMP_MGR_DeleteSnmpV3Group("abc", SNMP_MGR_SNMPV3_MODEL_V3, SNMP_MGR_SNMPV3_SECURITY_LEVEL_AUTHNOPRIV);
}
void test_destroygroup3()
{

	SNMP_MGR_DeleteSnmpV3Group("abc", SNMP_MGR_SNMPV3_MODEL_V3, SNMP_MGR_SNMPV3_SECURITY_LEVEL_AUTHPRIV);
}

void test_getnextuser()
{
   SNMP_MGR_SnmpV3UserEntry_T  entry;

   memset( &entry, 0 , sizeof(entry));

   while( SNMP_MGR_GetNextSnmpV3User( &entry)== SNMP_MGR_ERROR_OK)
   {
        SYSFUN_Debug_Printf( "username  = %s\n", entry.snmpv3_user_name);
        SYSFUN_Debug_Printf(" engineID   = %s\n", entry.snmpv3_user_engine_id);
        SYSFUN_Debug_Printf(" storage =   %d\n", entry.snmpv3_user_storage_type);
        SYSFUN_Debug_Printf(" model =     %d\n", entry.snmpv3_user_security_model);
        SYSFUN_Debug_Printf(" status = %d\n", entry.snmpv3_user_status);


   }
}


void test_getnextuserbygroup1()
{
   SNMP_MGR_V3_Auth_Type_T authType;
   SNMP_MGR_V3_Priv_Type_T privType;

  UI8_T user_name[MAXSIZE_SNMPV3_USER_NAME+1];


   strcpy( user_name, "");
   while( SNMP_MGR_GetNextSnmpV3UserByGroup( "DefaultROGroup", SNMP_MGR_SNMPV3_MODEL_V1,
                                             SNMP_MGR_SNMPV3_SECURITY_LEVEL_NOAUTH, user_name, &authType, &privType)== SNMP_MGR_ERROR_OK)
   {
        SYSFUN_Debug_Printf( "username  = %s\n",user_name);
        SYSFUN_Debug_Printf(" auth Type = %d\n", authType);
        SYSFUN_Debug_Printf(" priv type = %d\n",  privType);

   }
}

void test_getnextuserbygroup2()
{
   SNMP_MGR_V3_Auth_Type_T authType;
   SNMP_MGR_V3_Priv_Type_T privType;
   UI8_T user_name[MAXSIZE_SNMPV3_USER_NAME+1];


   strcpy( user_name, "");
   while( SNMP_MGR_GetNextSnmpV3UserByGroup( "kinghongG", SNMP_MGR_SNMPV3_MODEL_V3,
                                             SNMP_MGR_SNMPV3_SECURITY_LEVEL_NOAUTH, user_name, &authType, &privType)== SNMP_MGR_ERROR_OK)
   {
        SYSFUN_Debug_Printf( "username  = %s\n", user_name);
        SYSFUN_Debug_Printf(" auth Type = %d\n", authType);
        SYSFUN_Debug_Printf(" priv type = %d\n",  privType);

   }
}


void test_getnextview()
{
   SNMP_MGR_SnmpV3ViewEntry_T  entry;

   memset( &entry, 0 , sizeof(entry));

   while( SNMP_MGR_GetNextSnmpV3View( &entry)== SNMP_MGR_ERROR_OK)
   {
        SYSFUN_Debug_Printf( "snmpv3_view_name  = %s\n", entry.snmpv3_view_name);
        SYSFUN_Debug_Printf("snmpv3_wild_card_tree = %s\n", entry.snmpv3_wildcard_subtree);
        SYSFUN_Debug_Printf(" snmpv3_view_subtree   = %s\n", entry.snmpv3_view_subtree);
        SYSFUN_Debug_Printf(" snmpv3_view_subtree_len =   %d\n", entry.snmpv3_view_subtree_len);
        SYSFUN_Debug_Printf(" snmpv3_view_mask = %s\n", entry.snmpv3_view_mask);
         SYSFUN_Debug_Printf(" snmpv3_view_type = %d\n", entry.snmpv3_view_type);

   }
}

void test_getnextviewname()
{
   UI8_T  view_name[MAXSIZE_SNMPV3_VIEW_NAME+1];


   view_name[0]= '\0';
   while( SNMP_MGR_GetNextSnmpV3ViewName( view_name)== SNMP_MGR_ERROR_OK)
   {
        SYSFUN_Debug_Printf( "snmpv3_view_name  = %s\n", view_name);
   }
}

void test_getnextgroup()
{
   SNMP_MGR_SnmpV3GroupEntry_T  entry;

   memset( &entry, 0 , sizeof(entry));

   while( SNMP_MGR_GetNextSnmpV3Group( &entry)== SNMP_MGR_ERROR_OK)
   {
        SYSFUN_Debug_Printf( "snmpv3_group_name  = %s\n", entry.snmpv3_group_name);
         SYSFUN_Debug_Printf( "snmpv3_group_context_prefix  = %s\n", entry.snmpv3_group_context_prefix);
         SYSFUN_Debug_Printf(" snmpv3_group_model = %d\n", entry.snmpv3_group_model);
         SYSFUN_Debug_Printf(" snmpv3_group_security_level = %d\n", entry.snmpv3_group_security_level);
        SYSFUN_Debug_Printf(" snmpv3_group_readview   = %s\n", entry.snmpv3_group_readview);
              SYSFUN_Debug_Printf(" snmpv3_group_writeview   = %s\n", entry.snmpv3_group_writeview);
                    SYSFUN_Debug_Printf(" snmpv3_group_notifyview   = %s\n", entry.snmpv3_group_notifyview);
              SYSFUN_Debug_Printf(" snmpv3_group_status = %d\n", entry.snmpv3_group_status);


   }
}


void test_setEngineID()
{
	UI8_T  testengineid[13];
	UI32_T i;


	for ( i = 0; i<13; i++)
                    testengineid[i] = i;

	SNMP_MGR_Set_EngineID( testengineid);

}


void test_createcommunity()
{

SNMP_MGR_CreateSnmpCommunity( "netman", SNMP_MGR_ACCESS_RIGHT_READ_ONLY);
SNMP_MGR_CreateSnmpCommunity( "netman1", SNMP_MGR_ACCESS_RIGHT_READ_WRITE);
}


void test_getnextcommunity()
{
   SNMP_MGR_SnmpCommunity_T  entry;

   memset( &entry, 0 , sizeof(entry));

   while( SNMP_MGR_GetNextSnmpCommunity( &entry)== SNMP_MGR_ERROR_OK)
   {
        SYSFUN_Debug_Printf( "comm = %s\n", entry.comm_string_name);
         SYSFUN_Debug_Printf(" access right = %d\n", entry.access_right);

   }
}


#if 0

void test_tranformWildCardToString()
{
   UI8_T  source[20] = "1.3.6.*.1.*.5.9.1";
   UI8_T  dest[20];
   UI8_T mask[20];


   if ((SNMP_MGR_Transform_ViewWildcardToTree( source, dest, mask) != TRUE))
      SYSFUN_Debug_Printf(" invalid input\n");
   else
      SYSFUN_Debug_Printf("source = %s, dest = %s, mask = %s\n", source, dest, mask);




}


void test_tranformWildCardToString1()
{
   UI8_T  source[20] = "1";
   UI8_T  dest[20];
   UI8_T mask[20];


   if ((SNMP_MGR_Transform_ViewWildcardToTree( source, dest, mask) != TRUE))
      SYSFUN_Debug_Printf(" invalid input\n");
   else
      SYSFUN_Debug_Printf("source = %s, dest = %s, mask = %s\n", source, dest, mask);




}


void test_tranformStringToWildCard()
{
   UI8_T  sourcetree[30] = "1.3.6.1.2.1.2.2.1.0.2";
   UI8_T  sourcemask[30];
   UI8_T  dest[30];


   sourcemask[0] = 0xff;
   sourcemask[1] = 0xa0;
   if ((SNMP_MGR_Transform_TreeToWildCard( dest, sourcetree, sourcemask, 11) != TRUE))
      SYSFUN_Debug_Printf(" invalid input\n");
   else
      SYSFUN_Debug_Printf("sourcetree = %s, sourcemask = %s, dest = %s\n", sourcetree, sourcemask, dest);




}



void test_RMON_Get_1h()
{
int i;
UI32_T value;

for ( i = 1;  i <=24; i++)
	{

	if (   RMON_Packets1h( 1, i , &value))
	   SYSFUN_Debug_Printf(" \nPort %d RMON 1 Hours Packets = %d\n",i, value);
	else
	   SYSFUN_Debug_Printf(" Get error\n");
             }
}


#endif
#endif
