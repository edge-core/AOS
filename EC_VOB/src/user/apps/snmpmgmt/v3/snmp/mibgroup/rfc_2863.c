/*
 *  Interfaces MIB group implementation - interfaces.c
 *
 */

#include <net-snmp/net-snmp-config.h>

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/auto_nlist.h>


#include "l_snmp.h"
#include "rfc_2863.h"
#include "l_stdlib.h"
#include "l_charset.h"
#include "struct.h"
#include "util_funcs.h"
#include "sysORTable.h"
#include "if_mgr.h"
#include "if_pmgr.h"
#include "sys_type.h"
#include "snmp_mgr.h"
#include "vlan_type.h"

struct variable3 interfaces_variables[] = {
    {IFNUMBER, ASN_INTEGER, RONLY, var_interfaces, 1, {1}},
    {IFINDEX, ASN_INTEGER, RONLY, var_ifEntry, 3, {2, 1, 1}},
    {IFDESCR, ASN_OCTET_STR, RONLY, var_ifEntry, 3, {2, 1, 2}},
    {IFTYPE, ASN_INTEGER, RONLY, var_ifEntry, 3, {2, 1, 3}},
    {IFMTU, ASN_INTEGER, RONLY, var_ifEntry, 3, {2, 1, 4}},
    {IFSPEED, ASN_GAUGE, RONLY, var_ifEntry, 3, {2, 1, 5}},
    {IFPHYSADDRESS, ASN_OCTET_STR, RONLY, var_ifEntry, 3, {2, 1, 6}},
    {IFADMINSTATUS, ASN_INTEGER, RWRITE, var_ifEntry, 3, {2, 1, 7}},
    {IFOPERSTATUS, ASN_INTEGER, RONLY, var_ifEntry, 3, {2, 1, 8}},
    {IFLASTCHANGE, ASN_TIMETICKS, RONLY, var_ifEntry, 3, {2, 1, 9}},
    {IFINOCTETS, ASN_COUNTER, RONLY, var_ifEntry, 3, {2, 1, 10}},
    {IFINUCASTPKTS, ASN_COUNTER, RONLY, var_ifEntry, 3, {2, 1, 11}},
    {IFINNUCASTPKTS, ASN_COUNTER, RONLY, var_ifEntry, 3, {2, 1, 12}},
    {IFINDISCARDS, ASN_COUNTER, RONLY, var_ifEntry, 3, {2, 1, 13}},
    {IFINERRORS, ASN_COUNTER, RONLY, var_ifEntry, 3, {2, 1, 14}},
    {IFINUNKNOWNPROTOS, ASN_COUNTER, RONLY, var_ifEntry, 3, {2, 1, 15}},
    {IFOUTOCTETS, ASN_COUNTER, RONLY, var_ifEntry, 3, {2, 1, 16}},
    {IFOUTUCASTPKTS, ASN_COUNTER, RONLY, var_ifEntry, 3, {2, 1, 17}},
    {IFOUTNUCASTPKTS, ASN_COUNTER, RONLY, var_ifEntry, 3, {2, 1, 18}},
    {IFOUTDISCARDS, ASN_COUNTER, RONLY, var_ifEntry, 3, {2, 1, 19}},
    {IFOUTERRORS, ASN_COUNTER, RONLY, var_ifEntry, 3, {2, 1, 20}},
    {IFOUTQLEN, ASN_GAUGE, RONLY, var_ifEntry, 3, {2, 1, 21}},
    {IFSPECIFIC, ASN_OBJECT_ID, RONLY, var_ifEntry, 3, {2, 1, 22}}
};

/*
 * Define the OID pointer to the top of the mib tree that we're
 * registering underneath, and the OID of the MIB module
 */
static oid             interfaces_variables_oid[] = { SNMP_OID_MIB2, 2 };
static oid             interfaces_module_oid[] = { SNMP_OID_MIB2, 31 };

static WriteMethod     writeIfEntry;

typedef struct _conf_if_list {
    char           *name;
    int             type;
    u_long          speed;
    struct _conf_if_list *next;
} conf_if_list;

/*----------------------------------------------------------------------------*/
/* FUNCTION NAME - str_is_invlid                                              */
/* PURPOSE  :  check if the string has any invalid value (The '?' and ' ' is  */
/*                                                                   invlid)  */
/*                                                                            */
/* ARG      :                                                                 */
/*                 1. value the string which need to check                    */
/* RETURN   :                                                                 */
/*                0 --no invlidaalpha -1----has invlidalpha                   */
/* NOTES    :                                                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/

int if_str_is_invlid(char * value)
{

  if(value==NULL || *value=='\0')
    return -1;
  while(*value!='\0')
  {
      if((*value == '?')||(*value == ' ')||(*value == ':'))
        return -1;
     value++;
  }
  return 0;
}




void
init_interfaces(void)
{
    /*
     * register ourselves with the agent to handle our mib tree
     */
    REGISTER_MIB("mibII/interfaces", interfaces_variables, variable3,
                 interfaces_variables_oid);
    REGISTER_SYSOR_ENTRY(interfaces_module_oid,
                         "The MIB module to describe generic objects for network interface sub-layers");


}





/*
 * header_ifEntry(...
 * Arguments:
 * vp     IN      - pointer to variable entry that points here
 * name    IN/OUT  - IN/name requested, OUT/name found
 * length  IN/OUT  - length of IN/OUT oid's
 * exact   IN      - TRUE if an exact match was requested
 * var_len OUT     - length of variable or 0 if function returned
 * write_method
 *
 */

static int
header_ifEntry(struct variable *vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method,
               IF_MGR_IfEntry_T  *entry)
{
   int             result;
   unsigned int ifIndex = 0;

   if(!entry || l_snmp_index_get(vp,name,length,&ifIndex,exact) < 0 )
       return MATCH_FAILED;
  /*
     * find "next" interface
     */

    entry->if_index = ifIndex;
     if(!exact){
         if (IF_PMGR_GetNextIfEntry( entry)!=TRUE)
           return MATCH_FAILED;

         l_snmp_index_set(vp,name,length,entry->if_index);
    }
    else
    {
      if(IF_PMGR_GetIfEntry(entry)!=TRUE)
            return MATCH_FAILED;
    }

    *write_method = 0;
    *var_len = sizeof(long);    /* default to 'long' results */

    return TRUE;
}



u_char         *
var_interfaces(struct variable * vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method)
{

    if (header_generic(vp, name, length, exact, var_len, write_method) ==
        MATCH_FAILED)
        return NULL;

    switch (vp->magic) {
        case IFNUMBER:
        {
            UI32_T interfaceCount;
            if (IF_PMGR_GetIfNumber( & interfaceCount) != TRUE)
            {
                return NULL;
            }
            else
            {
                long_return = interfaceCount;
                return (u_char *) & long_return;
            }
        }
        default:
            DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_interfaces\n",
                    vp->magic));
    }
    return NULL;
}

        /*********************
	 *
	 *  System specific implementation functions
	 *
	 *********************/

u_char         *
var_ifEntry(struct variable *vp,
            oid * name,
            size_t * length,
            int exact, size_t * var_len, WriteMethod ** write_method)
{
   IF_MGR_IfEntry_T  entry;
    int             interface;



   memset( &entry, 0, sizeof(entry));

    if(MATCH_FAILED == header_ifEntry(vp, name, length, exact, var_len, write_method,&entry))
        return NULL;

    switch (vp->magic) {
    case IFINDEX:
        long_return = entry.if_index;
        return (u_char *) & long_return;
    case IFDESCR:
        *var_len = strlen((char *)entry.if_descr);
        strcpy((char *)return_buf, (char *)entry.if_descr);
        return (u_char *) return_buf;
    case IFTYPE:
/*
 *Fix bug:ES4827G-FLF-ZZ-00435
 *Problem:Node ifType show wrong values for vlan.
 *RootCause:Because here we set other vaule for this node.
 *Solution:change this value get path.
 *changed file:rfc_2863.c,if_mgr.c
 *approved by :Hard.Sun
 *Fixed by:Jinhua.Wei
 */
        if (entry.if_type== VLAN_L2_IFTYPE)
            long_return = VAL_vlanStaticInterfaceType_l2vlan;
        else if (entry.if_type == VLAN_L3_IP_IFTYPE)
            long_return = VAL_vlanStaticInterfaceType_l3ipvlan;
        else
          long_return = entry.if_type;
        return (u_char *) & long_return;
    case IFMTU:
           long_return = entry.if_mtu;
        return (u_char *) & long_return;
    case IFSPEED:
         long_return = entry.if_speed;
        return (u_char *) & long_return;
    case IFPHYSADDRESS:
     *var_len = 6;
      memcpy(return_buf, entry.if_phys_address,6);
      return (u_char *) return_buf;
    case IFADMINSTATUS:
     *write_method = writeIfEntry;
       long_return = entry.if_admin_status;
       return (u_char *) & long_return;
    case IFOPERSTATUS:
        long_return = entry.if_oper_status;
        return (u_char *) & long_return;
    case IFLASTCHANGE:
        long_return = entry.if_last_change;
        return (u_char *) & long_return;
    case IFINOCTETS:
         long_return = entry.if_in_octets;
         return (u_char *) & long_return;
    case IFINUCASTPKTS:
         long_return = entry.if_in_ucast_pkts;
         return (u_char *) & long_return;
    case IFINNUCASTPKTS:
         long_return = entry.if_in_nucast_pkts;
         return (u_char *) & long_return;
    case IFINDISCARDS:
        long_return = entry.if_in_discards;
         return (u_char *) & long_return;
    case IFINERRORS:
         long_return = entry.if_in_errors;
         return (u_char *) & long_return;
    case IFINUNKNOWNPROTOS:
         long_return = entry.if_in_unknown_protos;
         return (u_char *) & long_return;
    case IFOUTOCTETS:
          long_return = entry.if_out_octets;
         return (u_char *) & long_return;
    case IFOUTUCASTPKTS:
         long_return = entry.if_out_ucast_pkts;
         return (u_char *) & long_return;
    case IFOUTNUCASTPKTS:
          long_return = entry.if_out_nucast_pkts;
         return (u_char *) & long_return;
    case IFOUTDISCARDS:
          long_return = entry.if_out_discards;
         return (u_char *) & long_return;
    case IFOUTERRORS:
          long_return = entry.if_out_errors;
         return (u_char *) & long_return;
    case IFOUTQLEN:
          long_return = entry.if_out_qlen;
         return (u_char *) & long_return;
    case IFSPECIFIC:
    {
             static oid dummy[2];
             dummy[0] = 0;
             dummy[1] = 0;

               *var_len = 2* sizeof(oid);
               return (u_char *) dummy;
    }
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_ifEntry\n",
                    vp->magic));
    }
    return NULL;
}



static int      saveIndex = 0;


void
Interface_Scan_Init(void)
{
#if !defined(hpux11)
//    auto_nlist(IFNET_SYMBOL, (char *) &ifnetaddr, sizeof(ifnetaddr));
#endif
    saveIndex = 0;
}

#if 0
int
Interface_Scan_Get_Count(void)
{
    time_t          time_now = time(NULL);

    if (!Interface_Count || (time_now > scan_time + 60)) {
        scan_time = time_now;
        Interface_Scan_Init();
        Interface_Count = 0;
#if 0
        while (Interface_Scan_Next(NULL, NULL, NULL, NULL) != 0) {
            Interface_Count++;
        }
#endif
 }
    return (Interface_Count);
}
#endif

int
writeIfEntry(int action,
             u_char * var_val,
             u_char var_val_type,
             size_t var_val_len,
             u_char * statP, oid * name, size_t name_len)
{
//    MIB_IFROW       ifEntryRow;
   UI32_T ifadminstatus;
    if ((char) name[9] != IFADMINSTATUS)
     {
        return SNMP_ERR_NOTWRITABLE;
    }

    ifadminstatus = *(long *)var_val;
    switch (action)
     {
    case RESERVE1:             /* Check values for acceptability */
        if (var_val_type != ASN_INTEGER) {
            snmp_log(LOG_ERR, "not integer\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(long)) {

           return SNMP_ERR_WRONGLENGTH;
        }

        /*
         * The dwAdminStatus member can be MIB_IF_ADMIN_STATUS_UP or MIB_IF_ADMIN_STATUS_DOWN
         */

      if ((ifadminstatus!= 1) &&(ifadminstatus !=2))
      {
            //snmp_log(LOG_ERR, "not supported admin state\n");
            return SNMP_ERR_WRONGVALUE;
        }
        break;

    case RESERVE2:             /* Allocate memory and similar resources */
        break;

    case ACTION:
        /*
         * Save the old value, in case of UNDO
         */
        if (IF_PMGR_SetIfAdminStatus (name[10], ifadminstatus) != TRUE)
           return SNMP_ERR_COMMITFAILED;

      //  oldadmin_status = admin_status;
      //  admin_status = (int) *var_val;
        break;

    case UNDO:                 /* Reverse the SET action and free resources */
//           admin_status = oldadmin_status;
        break;

    case COMMIT:               /* Confirm the SET, performing any irreversible actions,
                                 * and free resources */
//        ifEntryRow.dwIndex = (int) name[10];
//        ifEntryRow.dwAdminStatus = admin_status;
        /*
         * Only UP and DOWN status are supported. Thats why done in COMMIT
         */


#if 0
        if (SetIfEntry(&ifEntryRow) != NO_ERROR) {
            snmp_log(LOG_ERR,
                     "Error in writeIfEntry case COMMIT with index: %d & adminStatus %d\n",
                     ifEntryRow.dwIndex, ifEntryRow.dwAdminStatus);
            return SNMP_ERR_COMMITFAILED;
        }
#endif
    case FREE:                 /* Free any resources allocated */
        /*
         * No resources have been allocated
         */
        break;
    }
    return SNMP_ERR_NOERROR;
}                               /* end of writeIfEntry */

/*
 * ifXTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */
oid ifMIB_modules_oid[] = {SNMP_OID_MIB2, 31};


oid ifXTable_variables_oid[] = { 1,3,6,1,2,1,31,1 };

/*
 * variable4 ifXTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */

struct variable3 ifXTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */

#define IFNAME		1
{IFNAME,  ASN_OCTET_STR,  RONLY,   var_ifXTable, 3,  { 1, 1, 1 }},
#define IFINMULTICASTPKTS		2
{IFINMULTICASTPKTS,  ASN_COUNTER,  RONLY,   var_ifXTable, 3,  { 1, 1, 2 }},
#define IFINBROADCASTPKTS		3
{IFINBROADCASTPKTS,  ASN_COUNTER,  RONLY,   var_ifXTable, 3,  { 1, 1, 3 }},
#define IFOUTMULTICASTPKTS		4
{IFOUTMULTICASTPKTS,  ASN_COUNTER,  RONLY,   var_ifXTable, 3,  { 1, 1, 4 }},
#define IFOUTBROADCASTPKTS		5
{IFOUTBROADCASTPKTS,  ASN_COUNTER,  RONLY,   var_ifXTable, 3,  { 1, 1, 5 }},
#define IFHCINOCTETS		6
{IFHCINOCTETS,  ASN_COUNTER64,  RONLY,   var_ifXTable, 3,  { 1, 1, 6 }},
#define IFHCINUCASTPKTS		7
{IFHCINUCASTPKTS,  ASN_COUNTER64,  RONLY,   var_ifXTable, 3,  { 1, 1, 7 }},
#define IFHCINMULTICASTPKTS		8
{IFHCINMULTICASTPKTS,  ASN_COUNTER64,  RONLY,   var_ifXTable, 3,  { 1, 1, 8 }},
#define IFHCINBROADCASTPKTS		9
{IFHCINBROADCASTPKTS,  ASN_COUNTER64,  RONLY,   var_ifXTable, 3,  { 1, 1, 9 }},
#define IFHCOUTOCTETS		10
{IFHCOUTOCTETS,  ASN_COUNTER64,  RONLY,   var_ifXTable, 3,  { 1, 1, 10 }},
#define IFHCOUTUCASTPKTS		11
{IFHCOUTUCASTPKTS,  ASN_COUNTER64,  RONLY,   var_ifXTable, 3,  { 1, 1, 11 }},
#define IFHCOUTMULTICASTPKTS		12
{IFHCOUTMULTICASTPKTS,  ASN_COUNTER64,  RONLY,   var_ifXTable, 3,  { 1, 1, 12 }},
#define IFHCOUTBROADCASTPKTS		13
{IFHCOUTBROADCASTPKTS,  ASN_COUNTER64,  RONLY,   var_ifXTable, 3,  { 1, 1, 13 }},
#define IFLINKUPDOWNTRAPENABLE		14
{IFLINKUPDOWNTRAPENABLE,  ASN_INTEGER,  RWRITE,  var_ifXTable, 3,  { 1, 1, 14 }},
#define IFHIGHSPEED		15
{IFHIGHSPEED,  ASN_GAUGE,  RONLY,   var_ifXTable, 3,  { 1, 1, 15 }},
#define IFPROMISCUOUSMODE		16
{IFPROMISCUOUSMODE,  ASN_INTEGER,  RWRITE,  var_ifXTable, 3,  { 1, 1, 16 }},
#define IFCONNECTORPRESENT		17
{IFCONNECTORPRESENT,  ASN_INTEGER,  RONLY,   var_ifXTable, 3,  { 1, 1, 17 }},
#define IFALIAS		18
{IFALIAS,  ASN_OCTET_STR,  RWRITE,  var_ifXTable, 3,  { 1, 1, 18 }},
#define IFCOUNTERDISCONTINUITYTIME		19
{IFCOUNTERDISCONTINUITYTIME,  ASN_TIMETICKS,  RONLY,   var_ifXTable, 3,  { 1, 1, 19 }},
};
/*    (L = length of the oidsuffix) */


/** Initializes the ifXTable module */
void
init_ifXTable(void)
{

    DEBUGMSGTL(("ifXTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("ifXTable", ifXTable_variables, variable3,
               ifXTable_variables_oid);

    REGISTER_SYSOR_ENTRY(ifMIB_modules_oid,
                         "The Interface Extenstion MIB.");
    /* place any other initialization junk you need here */
}

/*
 * header_ifEntry(...
 * Arguments:
 * vp     IN      - pointer to variable entry that points here
 * name    IN/OUT  - IN/name requested, OUT/name found
 * length  IN/OUT  - length of IN/OUT oid's
 * exact   IN      - TRUE if an exact match was requested
 * var_len OUT     - length of variable or 0 if function returned
 * write_method
 *
 */

static int
header_ifXTable(struct variable *vp,
               oid * name,
               size_t * length,
               int exact,IF_MGR_IfXEntry_T *entry)
{
    unsigned int ifIndex = 0;

    if(!entry || l_snmp_index_get(vp,name,length,&ifIndex,exact) < 0 )
        return MATCH_FAILED;

    entry->if_index = ifIndex;

    if(!exact){

        if(IF_PMGR_GetNextIfXEntry(entry)!=TRUE)
            return MATCH_FAILED;

        l_snmp_index_set(vp,name,length,entry->if_index);

    }else{

        if(IF_PMGR_GetIfXEntry(entry)!=TRUE)
            return MATCH_FAILED;
    }

    return MATCH_SUCCEEDED;
}


/*
 * var_ifXTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_ifXTable(struct variable *vp,
    	    oid     *name,
    	    size_t  *length,
    	    int     exact,
    	    size_t  *var_len,
    	    WriteMethod **write_method)
{
    IF_MGR_IfXEntry_T  entry;

    if(MATCH_FAILED == header_ifXTable(vp,name,length,exact,&entry))
        return NULL;
    /*the value must be null . but I am not sure . Tony.lei*/
    *write_method = 0;
    *var_len = sizeof(long);

    switch(vp->magic)
     {
        case IFNAME:
             strcpy((char *)return_buf, (char *)entry.if_name);
             *var_len= strlen((char *)return_buf);
             return (u_char*)return_buf;
        case IFINMULTICASTPKTS:
             long_return = entry.if_in_multicast_pkts;
             return (u_char*) &long_return;
        case IFINBROADCASTPKTS:
             long_return = entry.if_in_broadcast_pkts;
             return (u_char*) &long_return;
        case IFOUTMULTICASTPKTS:
             long_return = entry.if_out_multicast_pkts;
             return (u_char*) &long_return;
        case IFOUTBROADCASTPKTS:
             long_return = entry.if_out_broadcast_pkts;
             return (u_char*) &long_return;
        case IFHCINOCTETS:
             SNMP_MGR_UI64_T_TO_COUNTER64(long64_return, entry.if_hc_in_octets);
             *var_len = sizeof(long64_return);
             return (u_char*) &long64_return;
        case IFHCINUCASTPKTS:
             SNMP_MGR_UI64_T_TO_COUNTER64(long64_return, entry.if_hc_in_ucast_pkts);
             *var_len = sizeof(long64_return);
             return (u_char*) &long64_return;
        case IFHCINMULTICASTPKTS:
             SNMP_MGR_UI64_T_TO_COUNTER64(long64_return, entry.if_hc_in_multicast_pkts);
             *var_len = sizeof(long64_return);
             return (u_char*) &long64_return;
        case IFHCINBROADCASTPKTS:
             SNMP_MGR_UI64_T_TO_COUNTER64(long64_return, entry.if_hc_in_broadcast_pkts);
             *var_len = sizeof(long64_return);
             return (u_char*) &long64_return;
        case IFHCOUTOCTETS:
             SNMP_MGR_UI64_T_TO_COUNTER64(long64_return, entry.if_hc_out_octets);
             *var_len = sizeof(long64_return);
             return (u_char*) &long64_return;
        case IFHCOUTUCASTPKTS:
             SNMP_MGR_UI64_T_TO_COUNTER64(long64_return, entry.if_hc_out_ucast_pkts);
             *var_len = sizeof(long64_return);
             return (u_char*) &long64_return;
        case IFHCOUTMULTICASTPKTS:
             SNMP_MGR_UI64_T_TO_COUNTER64(long64_return, entry.if_hc_out_multicast_pkts);
             *var_len = sizeof(long64_return);
             return (u_char*) &long64_return;
        case IFHCOUTBROADCASTPKTS:
             SNMP_MGR_UI64_T_TO_COUNTER64(long64_return, entry.if_hc_out_broadcast_pkts);
             *var_len = sizeof(long64_return);
             return (u_char*) &long64_return;
        case IFLINKUPDOWNTRAPENABLE:
             *write_method = write_ifLinkUpDownTrapEnable;
             long_return = entry.if_link_up_down_trap_enable;
             return (u_char*) &long_return;
        case IFHIGHSPEED:
             long_return = entry.if_high_speed;
             return (u_char*) &long_return;
        case IFPROMISCUOUSMODE:
             *write_method = write_ifPromiscuousMode;
             long_return = entry.if_promiscuous_mode;
             return (u_char*) &long_return;
        case IFCONNECTORPRESENT:
             long_return = entry.if_connector_present;
             return (u_char*) &long_return;
        case IFALIAS:
             *write_method = write_ifAlias;
             strcpy((char *)return_buf, (char *)entry.if_alias);
             *var_len = strlen((char *)return_buf);
             return (u_char*) return_buf;
        case IFCOUNTERDISCONTINUITYTIME:
             long_return = entry.if_counter_discontinuity_time;
             return (u_char*) &long_return;
         default:
             ERROR_MSG("");
    }
    return NULL;
}


int
write_ifLinkUpDownTrapEnable(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    long value;

       if ((char) name[10] != IFLINKUPDOWNTRAPENABLE)
     {
        return SNMP_ERR_NOTWRITABLE;
    }

    if (name_len!=  12)
    {
        return SNMP_ERR_WRONGLENGTH;
    }


    switch ( action ) {
        case RESERVE1:
          if (var_val_type != ASN_INTEGER) {
              return SNMP_ERR_WRONGTYPE;
          }
          if (var_val_len > sizeof(long)) {
              return SNMP_ERR_WRONGLENGTH;
          }
          break;

        case RESERVE2:
          value = *(long *)var_val;
          if( (value !=1) && ( value !=2))
             return SNMP_ERR_WRONGVALUE;

          break;

        case FREE:
             /* Release any resources that have been allocated */
          break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */
             value = *(long *)var_val;

#if (SYS_CPNT_IF_LINK_TRAP_PORT_BASE == FALSE)
            if (IF_PMGR_SetIfLinkUpDownTrapEnableGlobal(value) != TRUE)
            {
                return SNMP_ERR_COMMITFAILED;
            }
#else
            if (IF_PMGR_SetIfLinkUpDownTrapEnable(name[11], value) != TRUE)
            {
                return SNMP_ERR_COMMITFAILED;
            }
#endif /* #if (SYS_CPNT_IF_LINK_TRAP_PORT_BASE == FALSE) */

          break;

        case UNDO:
             /* Back out any changes made in the ACTION case */
          break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
          break;
    }
    return SNMP_ERR_NOERROR;
}
int
write_ifPromiscuousMode(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    long value;

        if ((char) name[10] != IFPROMISCUOUSMODE)
     {
        return SNMP_ERR_NOTWRITABLE;
    }

    if (name_len!=  12)
    {
        return SNMP_ERR_WRONGLENGTH;
    }
    switch ( action ) {
        case RESERVE1:
          if (var_val_type != ASN_INTEGER) {
              return SNMP_ERR_WRONGTYPE;
          }
          if (var_val_len > sizeof(long)) {
              return SNMP_ERR_WRONGLENGTH;
          }
          break;

        case RESERVE2:
          value = *(long *)var_val;
          if ( (value !=1 ) && (value !=2))
             return SNMP_ERR_WRONGVALUE;
          break;

        case FREE:
             /* Release any resources that have been allocated */
          break;

        case ACTION:
             /*
              * The variable has been stored in 'value' for you to use,
              * and you have just been asked to do something with it.
              * Note that anything done here must be reversable in the UNDO case
              */

          /*The set operation of this variable not support*/
          return SNMP_ERR_COMMITFAILED;
          break;

        case UNDO:
             /* Back out any changes made in the ACTION case */
          break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
          break;
    }
    return SNMP_ERR_NOERROR;
}
int
write_ifAlias(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
    char  value[MAXSIZE_ifAlias+1];
    int size;


    /* check 1: check if the input index is exactly match, if not return fail*/
    if (name_len!=  12)
    {
        return SNMP_ERR_WRONGLENGTH;
    }
    switch ( action ) {
        case RESERVE1:
          if (var_val_type != ASN_OCTET_STR) {
              return SNMP_ERR_WRONGTYPE;
          }
          if (var_val_len >(MAXSIZE_ifAlias)* sizeof(char)) {
              return SNMP_ERR_WRONGLENGTH;
          }
          break;

        case RESERVE2:
          size  = var_val_len;


          break;

        case FREE:
             /* Release any resources that have been allocated */
          break;

        case ACTION:
             size  = var_val_len;
             memcpy( value, var_val, size);
             value[size]= '\0';

             if (value[0] != 0 && !L_CHARSET_IsValidGenericString(value))
             {
                return SNMP_ERR_BADVALUE;
             }

             if (IF_PMGR_SetIfAlias(name[11], (UI8_T *)value) != TRUE)
                return SNMP_ERR_COMMITFAILED;
             break;

        case UNDO:
             /* Back out any changes made in the ACTION case */
          break;

        case COMMIT:
             /*
              * Things are working well, so it's now safe to make the change
              * permanently.  Make sure that anything done here can't fail!
              */
          break;
    }
    return SNMP_ERR_NOERROR;
}

/*
 * ifStackTable_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */

oid ifStackTable_variables_oid[] = { 1,3,6,1,2,1,31,1};

/*
 * variable4 ifStackTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */

struct variable3 ifStackTable_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */

#define IFSTACKHIGHERLAYER		1
{IFSTACKHIGHERLAYER,  ASN_INTEGER,  NOACCESS,   var_ifStackTable, 3,  { 2, 1, 1 }},
#define IFSTACKLOWERLAYER		2
{IFSTACKLOWERLAYER,  ASN_INTEGER,  NOACCESS,   var_ifStackTable, 3,  { 2, 1, 2 }},
#define IFSTACKSTATUS		3
{IFSTACKSTATUS,  ASN_INTEGER,  RWRITE,  var_ifStackTable, 3,  { 2, 1, 3 }},
};
/*    (L = length of the oidsuffix) */


/** Initializes the ifStackTable module */
void
init_ifStackTable(void)
{

    DEBUGMSGTL(("ifStackTable", "Initializing\n"));

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("ifStackTable", ifStackTable_variables, variable3,
               ifStackTable_variables_oid);

    /* place any other initialization junk you need here */
}

static int
header_ifStackTable(struct variable *vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len,IF_MGR_IfStackEntry_T  *entry)
{
    UI32_T index1 ,index2;

     if(!entry || l_snmp_2_index_get(vp,name,length,&index1,&index2,exact) < 0 )
         return MATCH_FAILED;

    /*
     * find "next" interface
     */
     entry->if_stack_higher_layer = index1;
     entry->if_stack_lower_layer = index2;
   if(!exact){

        if ( IF_PMGR_GetNextIfStackEntry( entry)!=TRUE)
          return MATCH_FAILED;

       l_snmp_2_index_set(vp,name,length,entry->if_stack_higher_layer,entry->if_stack_lower_layer);

    }
    else
    {
       if(IF_PMGR_GetIfStackEntry(entry)!=TRUE)
            return MATCH_FAILED;
    }

    *var_len = sizeof(long);    /* default to 'long' results */

    return TRUE;

}



/*
 * var_ifStackTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *
var_ifStackTable(struct variable *vp,
    	    oid     *name,
    	    size_t  *length,
    	    int     exact,
    	    size_t  *var_len,
    	    WriteMethod **write_method)
{
    /* variables we may use later */

    IF_MGR_IfStackEntry_T  entry;


   /*Since this table allow for entry that does not exist, (creation). we need to know the write method first*/
    switch(vp->magic)
     {
    case IFSTACKSTATUS:
        *write_method = write_ifStackStatus;
     break;

    }

    memset( &entry, 0, sizeof(entry));

    if( MATCH_FAILED == header_ifStackTable(vp, name, length, exact, var_len,&entry))
        return NULL;

    /*
   * this is where we do the value assignments for the mib results.
   */
    switch(vp->magic) {
    case IFSTACKSTATUS:
       long_return = entry.if_stack_status;
        return (u_char*) &long_return;
    default:
      ERROR_MSG("");
    }
    return NULL;
}

int write_ifStackStatus(int action,
    u_char *var_val,
    u_char var_val_type,
    size_t var_val_len,
    u_char *statP,
    oid *name,
    size_t name_len)
{
    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }
            break;

        case RESERVE2:
            switch (*(long *)var_val)
            {
                case VAL_ifStackStatus_active:
                    break;

                case VAL_ifStackStatus_notInService:
                    break;

                case VAL_ifStackStatus_notReady:
                    break;

                case VAL_ifStackStatus_createAndGo:
                    break;

                case VAL_ifStackStatus_createAndWait:
                    break;

                case VAL_ifStackStatus_destroy:
                    break;

                default:
                    return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            break;

        case ACTION:
            /*
             * The variable has been stored in 'value' for you to use,
             * and you have just been asked to do something with it.
             * Note that anything done here must be reversable in the UNDO case
             */
            /*
             * Not Support the set operation of this variable, kinghong
             */
            return SNMP_ERR_COMMITFAILED;

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    return SNMP_ERR_NOERROR;
}

/*********************
*ifMIBObjects
**********************/
/** Initializes the ifMIBObjects module */
void
init_ifMIBObjects(void)
{
    static oid      ifTableLastChange_oid[] =
        { 1, 3, 6, 1, 2, 1, 31, 1, 5, 0 };
    static oid      ifStackLastChange_oid[] =
        { 1, 3, 6, 1, 2, 1, 31, 1, 6, 0 };

    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("ifTableLastChange",
                                         get_ifTableLastChange,
                                         ifTableLastChange_oid,
                                         OID_LENGTH(ifTableLastChange_oid),
                                         HANDLER_CAN_RONLY));
    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("ifStackLastChange",
                                         get_ifStackLastChange,
                                         ifStackLastChange_oid,
                                         OID_LENGTH(ifStackLastChange_oid),
                                         HANDLER_CAN_RONLY));
}

int
get_ifTableLastChange(netsnmp_mib_handler *handler,
                      netsnmp_handler_registration *reginfo,
                      netsnmp_agent_request_info *reqinfo,
                      netsnmp_request_info *requests)
{
    /*
     * We are never called for a GETNEXT if it's registered as a
     * "instance", as it's "magically" handled for us.
     */

    /*
     * a instance handler also only hands us one request at a time, so
     * we don't need to loop over a list of requests; we'll only get one.
     */

    switch (reqinfo->mode) {

    case MODE_GET:
    {
        UI32_T value;
        if (IF_PMGR_GetIfTableLastChange(&value))
        {
            long_return=value;
            snmp_set_var_typed_value(requests->requestvb, ASN_TIMETICKS, (u_char *)&long_return, sizeof(long_return));
        }
        else
        {
            if(SNMP_MGR_IsDebugMode())
                printf("get_ifTableLastChange:IF_PMGR_GetIfTableLastChange return false\n");
        	return SNMP_ERR_GENERR;
        }
	    break;
     }
    default:
        /*
         * we should never get here, so this is a really bad error
         */
        return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
get_ifStackLastChange(netsnmp_mib_handler *handler,
                      netsnmp_handler_registration *reginfo,
                      netsnmp_agent_request_info *reqinfo,
                      netsnmp_request_info *requests)
{
    /*
     * We are never called for a GETNEXT if it's registered as a
     * "instance", as it's "magically" handled for us.
     */

    /*
     * a instance handler also only hands us one request at a time, so
     * we don't need to loop over a list of requests; we'll only get one.
     */

    switch (reqinfo->mode) {

    case MODE_GET:
    {
        UI32_T value;
        if (IF_PMGR_GetIfStackLastChange(&value))
        {
            long_return=value;
            snmp_set_var_typed_value(requests->requestvb, ASN_TIMETICKS, (u_char *)&long_return, sizeof(long_return));
        }
        else
        {
            if(SNMP_MGR_IsDebugMode())
                printf("get_ifStackLastChange:IF_PMGR_GetIfStackLastChange return false\n");
        	return SNMP_ERR_GENERR;
        }
	    break;
     }
    default:
        /*
         * we should never get here, so this is a really bad error
         */
        return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
