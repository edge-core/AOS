#if (SYS_CPNT_UDPHELPER == TRUE)

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "leaf_a3comSwitchingUDPHelper.h"
#include "A3comSwitchingUDPHelper.h"
#include "udphelper_mgr.h"
#include "snmp_mgr.h"
#include "sysORTable.h"


#define    a3ComUdpHelperEntry_INSTANCE_LEN    5
#define    a3ComUdpHelperEntry_OID_NAME_LEN    14

/*
 * a3ComUdpHelperTable_variables_oid:
 *     This is the top level oid that we want to register under.
 *     This is essentially a prefix, with the suffix appearing in the variable below.
 */
oid a3ComUdpHelperTable_variables_oid[] = { 1, 3, 6, 1, 4, 1, 43, 29, 4, 15, 3 };

/*
 * variable3 a3ComUdpHelperTable_variables:
 *     This variable defines function callbacks and type return information for the  mib section
 */
struct variable3 a3ComUdpHelperTable_variables[] =
{
    /* magic number             variable type     ro/rw      callback fn              L  oidsuffix */
    {A3COMUDPHELPERPORT,        ASN_INTEGER,      RONLY,     var_a3ComUdpHelperTable, 3, {1, 1, 1}},
    {A3COMUDPHELPERIPADDRESS,   ASN_IPADDRESS,    RONLY,     var_a3ComUdpHelperTable, 3, {1, 1, 2}},
    {A3COMUDPHELPERROWSTATUS,   ASN_INTEGER,      RWRITE,    var_a3ComUdpHelperTable, 3, {1, 1, 3}},
};


/*  Initializes the a3ComUdpHelperTable module */
void init_a3ComUdpHelperTable(void)
{
    static oid udphelper_modules_oid[] = {1, 3, 6, 1, 4, 1, 43, 29, 4, 15, 3};

    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("a3ComUdpHelperTable", a3ComUdpHelperTable_variables,
                 variable3, a3ComUdpHelperTable_variables_oid);

    REGISTER_SYSOR_ENTRY(udphelper_modules_oid, "The A3COM446 SWITCHING UDPHELPER MIB");
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - a3ComUdpHelperTable_get
 *-------------------------------------------------------------------------
 * PURPOSE  : Get an entry of the a3comUdpHelperTable
 * INPUT    : compc: the counts of index
 *            *compl: component list
 *            *data: data to pass out
 * OUTPUT   : None
 * RETURN   : True or false
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T a3ComUdpHelperTable_get(int                                    compc,
                                      oid                                    *compl,
                                      UDPHELPER_MGR_UdpHelperEntry_T    *data)
{
    /* Check the counts of index
    */
    if (compc != a3ComUdpHelperEntry_INSTANCE_LEN)
    {
        return FALSE;
    }

    /* Index assignments
    */
    data->udp_helper_port = compl[0];
    data->udp_helper_ip_address = (compl[1] <<24) | (compl[2] <<16) | (compl[3] <<8) | compl[4];
    data->udp_helper_ip_address=L_STDLIB_Hton32(data->udp_helper_ip_address);

    /* Get the value
    */
    if (UDPHELPER_MGR_GetUdpHelperEntry(data) != UDPHELPER_MGR_OK)
    {
        return FALSE;
    }

    return TRUE;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - a3ComUdpHelperTable_next
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next entry of the a3comUdpHelperTable
 * INPUT    : compc: the counts of index
 *            *compl: component list
 *            *data: data to pass out
 * OUTPUT   : None
 * RETURN   : True or false
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T a3ComUdpHelperTable_next(int                                    compc,
                                       oid                                    *compl,
                                       UDPHELPER_MGR_UdpHelperEntry_T    *data)
{
    oid tmp_compl[a3ComUdpHelperEntry_INSTANCE_LEN];

    memcpy(tmp_compl, compl, sizeof(tmp_compl));
    /* Index range check */
    SNMP_MGR_checkCompl(0, 0, tmp_compl, MAX_a3ComUdpHelperPort);
    SNMP_MGR_checkCompl(1, 4, tmp_compl, 255);
    /* Convert the remaining index to zero if the compl is not given enough number of index.
     * This is used to solve the getnext issue.
     */
    SNMP_MGR_ConvertRemainToZero(compc, a3ComUdpHelperEntry_INSTANCE_LEN, tmp_compl);
    /* Index assignments */
    data->udp_helper_port = tmp_compl[0];
    data->udp_helper_ip_address = (tmp_compl[1] << 24) | (tmp_compl[2] << 16) | (tmp_compl[3] << 8) | tmp_compl[4];
    data->udp_helper_ip_address=L_STDLIB_Hton32(data->udp_helper_ip_address);

    /* Index count check */
    if (compc < a3ComUdpHelperEntry_INSTANCE_LEN)
    {
        if (UDPHELPER_MGR_GetUdpHelperEntry(data) != UDPHELPER_MGR_OK)
        {
            if (UDPHELPER_MGR_GetNextUdpHelperEntry(data) != UDPHELPER_MGR_OK)
            {
                return FALSE;
            }
        }
    }

    else
    {
        if (UDPHELPER_MGR_GetNextUdpHelperEntry(data) != UDPHELPER_MGR_OK)
        {
            return FALSE;
        }
    }

    return TRUE;
}


/*
 * var_a3ComUdpHelperTable():
 *     Handle this table separately from the scalar value case.
 *     The workings of this are basically the same as for var_ above.
 */
unsigned char * var_a3ComUdpHelperTable(struct variable *vp,
                                        oid             *name,
                                        size_t          *length,
                                        int             exact,
                                        size_t          *var_len,
                                        WriteMethod     **write_method)
{
    UI32_T compc = 0;
    oid compl[a3ComUdpHelperEntry_INSTANCE_LEN];
    oid best_inst[a3ComUdpHelperEntry_INSTANCE_LEN];
    UDPHELPER_MGR_UdpHelperEntry_T entry;

    memset(&entry, 0, sizeof(UDPHELPER_MGR_UdpHelperEntry_T));

    /* Assign write method */
    switch (vp->magic)
    {
        case A3COMUDPHELPERPORT:
            *write_method = 0;
            break;
        case A3COMUDPHELPERIPADDRESS:
            *write_method = 0;
            break;
        case A3COMUDPHELPERROWSTATUS:
            *write_method = write_a3ComUdpHelperRowStatus;
            break;
        default:
            break;
    }

    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl, a3ComUdpHelperEntry_INSTANCE_LEN);

    /* get, set */
    if (exact)
    {
        if (!a3ComUdpHelperTable_get(compc, compl, &entry))
        {
            return NULL;
        }
    }

    /* getnext */
    else
    {
        if (!a3ComUdpHelperTable_next(compc, compl, &entry))
        {
            return NULL;
        }
    }

    best_inst[0] = entry.udp_helper_port;
    best_inst[1] = (UI8_T) (entry.udp_helper_ip_address >> 24);
    best_inst[2] = (UI8_T) (entry.udp_helper_ip_address >> 16);
    best_inst[3] = (UI8_T) (entry.udp_helper_ip_address >> 8);
    best_inst[4] = (UI8_T) entry.udp_helper_ip_address;
    memcpy(name, vp->name, vp->namelen * sizeof(oid));
    memcpy(name + vp->namelen, best_inst, a3ComUdpHelperEntry_INSTANCE_LEN * sizeof(oid));
    *length  = vp->namelen + a3ComUdpHelperEntry_INSTANCE_LEN;
    *var_len = 4;

    /* This is where we do the value assignments for the mib results. */
    switch (vp->magic)
    {
        case A3COMUDPHELPERPORT:
            long_return = entry.udp_helper_port;
            return (u_char*) &long_return;
        case A3COMUDPHELPERIPADDRESS:
            long_return = entry.udp_helper_ip_address;
            return (u_char*) &long_return;
        case A3COMUDPHELPERROWSTATUS:
            long_return = entry.udp_helper_row_status;
            return (u_char*) &long_return;
        default:
            ERROR_MSG("");
    }

    return NULL;
}


int write_a3ComUdpHelperRowStatus(int       action,
                                  u_char    *var_val,
                                  u_char    var_val_type,
                                  size_t    var_val_len,
                                  u_char    *statP,
                                  oid       *name,
                                  size_t    name_len)
{
    UI32_T value;

    if ((char)name[a3ComUdpHelperEntry_OID_NAME_LEN - 1] != A3COMUDPHELPERROWSTATUS)
    {
        return SNMP_ERR_NOTWRITABLE;
    }

    switch (action)
    {
        case RESERVE1:
        {
            /* Input type check */
            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }

            if (var_val_len > sizeof(long))
            {
                return SNMP_ERR_WRONGLENGTH;
            }

            break;
        }

        case RESERVE2:
        {
            value = *(long *) var_val;

            /* Acceptable value range check */
            if ((value < VAL_a3ComUdpHelperRowStatus_active) || (value > VAL_a3ComUdpHelperRowStatus_destroy))
                return SNMP_ERR_WRONGVALUE;

            break;
        }

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
        {
            UI32_T index1, index2;

            value = *(long *) var_val;
            index1 = name[a3ComUdpHelperEntry_OID_NAME_LEN];
            index2 = (name[a3ComUdpHelperEntry_OID_NAME_LEN + 1] << 24) |
                     (name[a3ComUdpHelperEntry_OID_NAME_LEN + 2] << 16) |
                     (name[a3ComUdpHelperEntry_OID_NAME_LEN + 3] << 8) |
                     name[a3ComUdpHelperEntry_OID_NAME_LEN + 4];
            index2=L_STDLIB_Hton32(index2);

            if (UDPHELPER_MGR_SetUdpHelperRowStatus(index1, index2, value) != UDPHELPER_MGR_OK)
                return SNMP_ERR_COMMITFAILED;

            break;
        }

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


#endif  /* end of #if (SYS_CPNT_UDPHELPER == TRUE) */
