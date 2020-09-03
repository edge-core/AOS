/* MODULE NAME:  fdry_dhcp_snooping.c
 *
 * PURPOSE: For SNMP to access Brocade's FDRY-DHCP-SNOOP-MIB:
 * 
 * foundry(1991).products(1).switch(1).snSwitch(3).fdryDhcpSnoopMIB(36)
 *
 * NOTES:
 *
 * HISTORY (mm/dd/yyyy)
 *    04/27/2011 - Qiyao Zhong, Created
 *
 * Copyright(C)      Accton Corporation, 2011
 */

/* Note: this file originally auto-generated by mib2c using
 *        : mib2c.old-api.conf,v 1.4 2004/07/28 08:04:58 dts12 Exp $
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "leaf_fdry_dhcp_snooping.h"
#include "l_stdlib.h"

#include "snmp_mgr.h"
#include "sysORTable.h"
#include "foundry_lib.h"
#include "fdry_dhcp_snooping.h"

#if (SYS_CPNT_DHCPSNP == TRUE)
#include "dhcpsnp_pmgr.h"
#include "dhcpsnp_type.h"
#endif  /* (SYS_CPNT_DHCPSNP == TRUE) */

#if (SYS_CPNT_DHCPSNP == TRUE)  /* from near-beginning to end of file */
#if (FOUNDRY_LIB_PLATFORM_STYLE == FOUNDRY_LIB_PLATFORM_STYLE_EDGE_CORE)
/* ------------------------------------------------------------------------
 * SCALAR NODES - fdryDhcpSnoopGlobalObjects.*
 * ------------------------------------------------------------------------
 */
void init_fdryDhcpSnoopGlobalClearOper(void)
{
    static oid fdryDhcpSnoopGlobalClearOper_oid[] = { 1,3,6,1,4,1,1991,1,1,3,36,1,1,0 };

    netsnmp_register_instance(netsnmp_create_handler_registration
                              ("fdryDhcpSnoopGlobalClearOper",
                               do_fdryDhcpSnoopGlobalClearOper,
                               fdryDhcpSnoopGlobalClearOper_oid,
                               OID_LENGTH(fdryDhcpSnoopGlobalClearOper_oid),
                               HANDLER_CAN_RWRITE));
}

int do_fdryDhcpSnoopGlobalClearOper(netsnmp_mib_handler *handler,
    netsnmp_handler_registration *reginfo,
    netsnmp_agent_request_info *reqinfo,
    netsnmp_request_info *requests)
{
    /* dispatch get vs. set
     */
    switch (reqinfo->mode)
    {
        /*
         * GET REQUEST
         */
        case MODE_GET:
        {
            UI32_T var_len = 0, value = 0;

            /* hard-coded value
             */
            var_len = 4;
            memcpy(return_buf, &value, sizeof(value));
            value = VAL_fdryDhcpSnoopGlobalClearOper_valid;
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                (u_char *) return_buf, var_len);

            break;
        }

        /*
         * SET REQUEST
         *
         * multiple states in the transaction.  See:
         * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
         */
        case MODE_SET_RESERVE1:
            if (requests->requestvb->type != ASN_INTEGER)
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
                return SNMP_ERR_NOERROR;
            }
            break;

        case MODE_SET_RESERVE2:
            switch (*requests->requestvb->val.integer)
            {
                case VAL_fdryDhcpSnoopGlobalClearOper_valid:
                    break;

                case VAL_fdryDhcpSnoopGlobalClearOper_clear:
                    break;

                default:
                    netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGVALUE);
                    return SNMP_ERR_NOERROR;
            }
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
             * RESERVE2.  Something failed somewhere, and the states
             * below won't be called.
             */
            break;

        case MODE_SET_ACTION:
        {
            /* set to core layer
             */
            switch (*requests->requestvb->val.integer)
            {
                /* nothing to do
                 */
                case VAL_fdryDhcpSnoopGlobalClearOper_valid:
                    break;

                /* clear
                 */
                case VAL_fdryDhcpSnoopGlobalClearOper_clear:
                    if (DHCPSNP_PMGR_DeleteAllDynamicDhcpSnoopingBindingEntry() != DHCPSNP_TYPE_OK)
                    {
                        netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
                    }
                    break;

                /* should not bappen
                 */
                default:
                    break;
            }

            break;
        }

        case MODE_SET_COMMIT:
            break;

        case MODE_SET_UNDO:
            break;

        default:
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
#endif  /* (FOUNDRY_LIB_PLATFORM_STYLE == FOUNDRY_LIB_PLATFORM_STYLE_EDGE_CORE) */

/* ------------------------------------------------------------------------
 * TABLE NAME - fdryDhcpSnoopVlanConfigTable
 * ------------------------------------------------------------------------
 */
oid fdryDhcpSnoopVlanConfigTable_variables_oid[] = { 1,3,6,1,4,1,1991,1,1,3,36,2 };

/* variable3 fdryDhcpSnoopVlanConfigTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */
struct variable3 fdryDhcpSnoopVlanConfigTable_variables[] =
{
    /* magic number, variable type, ro/rw, callback fn, L, oidsuffix
     *     (L = length of the oidsuffix)
     */
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
    { LEAF_fdryDhcpSnoopVlanVLanId, ASN_UNSIGNED, RONLY, var_fdryDhcpSnoopVlanConfigTable, 3, { 1, 1, 1 }},
#endif /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

    { LEAF_fdryDhcpSnoopVlanDhcpSnoopEnable, ASN_INTEGER, RWRITE, var_fdryDhcpSnoopVlanConfigTable, 3, { 1, 1, 2 }},
};

void init_fdryDhcpSnoopVlanConfigTable(void)
{
    /* Register ourselves with the agent to handle our MIB tree
     */
    REGISTER_MIB("fdryDhcpSnoopVlanConfigTable", fdryDhcpSnoopVlanConfigTable_variables, variable3,
                 fdryDhcpSnoopVlanConfigTable_variables_oid);
}

#define FDRYDHCPSNOOPVLANCONFIGENTRY_INSTANCE_LEN  1

BOOL_T fdryDhcpSnoopVlanConfigTable_OidIndexToData(UI32_T exact, UI32_T compc,
    oid *compl, UI32_T *fdryDhcpSnoopVlanVLanId)
{
    /* get or set
     */
    if (exact)
    {
        /* check the index length
         */
        if (compc != FDRYDHCPSNOOPVLANCONFIGENTRY_INSTANCE_LEN) /* the constant size index */
        {
            return FALSE;
        }
    }

    *fdryDhcpSnoopVlanVLanId = compl[0];

    return TRUE;
}

/*
 * var_fdryDhcpSnoopVlanConfigTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *var_fdryDhcpSnoopVlanConfigTable(struct variable *vp,
    oid *name,
    size_t *length,
    int exact,
    size_t *var_len,
    WriteMethod **write_method)
{
    UI32_T compc = 0;
    oid compl[FDRYDHCPSNOOPVLANCONFIGENTRY_INSTANCE_LEN] = {0};
    oid best_inst[FDRYDHCPSNOOPVLANCONFIGENTRY_INSTANCE_LEN] = {0};

    /* table-specific variables
     */
    UI32_T  vid = 0;
    UI8_T   status = 0;

    /* dispatch node to set write method
     */
    switch (vp->magic)
    {
        case LEAF_fdryDhcpSnoopVlanDhcpSnoopEnable:
            *write_method = write_fdryDhcpSnoopVlanDhcpSnoopEnable;
            break;

        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl
     */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl,
        FDRYDHCPSNOOPVLANCONFIGENTRY_INSTANCE_LEN);

    /* dispatch get-exact versus get-next
     */
    if (exact) /* get or set */
    {
        /* extract index
         */
        if (! fdryDhcpSnoopVlanConfigTable_OidIndexToData(exact, compc, compl,
            &vid))
        {
            return NULL;
        }

        /* get-exact from core layer
         */
        if (DHCPSNP_PMGR_GetDhcpSnoopingStatusByVlan(vid, &status) != DHCPSNP_TYPE_OK)
        {
            return NULL;
        }
    }
    else /* get-next */
    {
        /* extract index
         */
        fdryDhcpSnoopVlanConfigTable_OidIndexToData(exact, compc, compl,
            &vid);

        /* Check the length of inputing index. If compc is less than instance
         * length, we should try get {A.B.C.0.0...}, where A.B.C was
         * obtained from the "..._OidIndexToData" function call, and
         * 0.0... was initialized in the beginning of this function.
         * This instance may exist in the core layer.
         */
        if (compc < FDRYDHCPSNOOPVLANCONFIGENTRY_INSTANCE_LEN)  /* incomplete index */
        {
            /* get-exact, in case this instance exists
             */
            if (DHCPSNP_PMGR_GetDhcpSnoopingStatusByVlan(vid, &status) != DHCPSNP_TYPE_OK)
            {
                /* get-next according to lexicographic order; if none, fail
                 */
                if (DHCPSNP_PMGR_GetNextDhcpSnoopingStatusByVlan(&vid, &status) != DHCPSNP_TYPE_OK)
                {
                    return NULL;
                }
            }
        }
        else  /* complete index */
        {
            /* get-next according to lexicographic order; if none, fail
             */
            if (DHCPSNP_PMGR_GetNextDhcpSnoopingStatusByVlan(&vid, &status) != DHCPSNP_TYPE_OK)
            {
                return NULL;
            }
        }
    }

    /* copy base OID (without index) to output
     */
    memcpy(name, vp->name, vp->namelen * sizeof(oid));

    /* assign data to the OID index
     */
    best_inst[0] = vid;
    memcpy(name + vp->namelen, best_inst, FDRYDHCPSNOOPVLANCONFIGENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + FDRYDHCPSNOOPVLANCONFIGENTRY_INSTANCE_LEN;

    /* dispatch node to read value
     */
    switch (vp->magic)
    {
#if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1)
        case LEAF_fdryDhcpSnoopVlanVLanId:
            *var_len = 4;
            long_return = vid;
            return (u_char *) &long_return;
#endif /* #if (SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE == 1) */

        case LEAF_fdryDhcpSnoopVlanDhcpSnoopEnable:
            *var_len = 4;
            long_return = status;  /* TruthValue = EnabledStatus */
            return (u_char *) &long_return;

        default:
            ERROR_MSG("");
            break;
    }

    /* return failure
     */
    return NULL;
}

int write_fdryDhcpSnoopVlanDhcpSnoopEnable(int action,
    u_char *var_val,
    u_char var_val_type,
    size_t var_val_len,
    u_char *statP,
    oid *name,
    size_t name_len)
{
    /* dispatch action
     */
    switch (action)
    {
        case RESERVE1:
            /* check type and length
             */
            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }
            break;

        case RESERVE2:
            /* check valid values
             */
            switch (*(long *) var_val)
            {
                case VAL_fdryDhcpSnoopVlanDhcpSnoopEnable_true:
                    break;

                case VAL_fdryDhcpSnoopVlanDhcpSnoopEnable_false:
                    break;

                default:
                    return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T oid_name_length = 15;
            I32_T value = 0;

            /* table-specific variables
             */
            UI32_T fdryDhcpSnoopVlanVLanId = 0;

            /* extract index
             */
            if (! fdryDhcpSnoopVlanConfigTable_OidIndexToData(TRUE,
                name_len - oid_name_length,
                &(name[oid_name_length]),
                &fdryDhcpSnoopVlanVLanId))
            {
                return SNMP_ERR_COMMITFAILED;
            }

            /* get user value
             */
            memcpy(&value, var_val, sizeof(I32_T));

            /* set to core layer
             */
            switch (value)
            {
                /* enable
                 */
                case VAL_fdryDhcpSnoopVlanDhcpSnoopEnable_true:
                    if (DHCPSNP_PMGR_EnableDhcpSnoopingByVlan(fdryDhcpSnoopVlanVLanId)
                        != DHCPSNP_TYPE_OK)
                    {
                        return SNMP_ERR_COMMITFAILED;
                    }

                    break;

                /* disable
                 */
                case VAL_fdryDhcpSnoopVlanDhcpSnoopEnable_false:
                    if (DHCPSNP_PMGR_DisableDhcpSnoopingByVlan(fdryDhcpSnoopVlanVLanId)
                        != DHCPSNP_TYPE_OK)
                    {
                        return SNMP_ERR_COMMITFAILED;
                    }

                    break;

                /* should not happen
                 */
                default:
                    return SNMP_ERR_WRONGVALUE;
            }

            break;
        }

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    /* return success
     */
    return SNMP_ERR_NOERROR;
}

/* ------------------------------------------------------------------------
 * TABLE NAME - fdryDhcpSnoopIfConfigTable
 * ------------------------------------------------------------------------
 */
oid fdryDhcpSnoopIfConfigTable_variables_oid[] = { 1,3,6,1,4,1,1991,1,1,3,36,3 };

/* variable3 fdryDhcpSnoopIfConfigTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */
struct variable3 fdryDhcpSnoopIfConfigTable_variables[] =
{
    /* magic number, variable type, ro/rw, callback fn, L, oidsuffix
     *     (L = length of the oidsuffix)
     */
    { LEAF_fdryDhcpSnoopIfTrustValue, ASN_INTEGER, RWRITE, var_fdryDhcpSnoopIfConfigTable, 3, { 1, 1, 1 }},
};

void init_fdryDhcpSnoopIfConfigTable(void)
{
    /* Register ourselves with the agent to handle our MIB tree
     */
    REGISTER_MIB("fdryDhcpSnoopIfConfigTable", fdryDhcpSnoopIfConfigTable_variables, variable3,
                 fdryDhcpSnoopIfConfigTable_variables_oid);
}

#define FDRYDHCPSNOOPIFCONFIGENTRY_INSTANCE_LEN  1

BOOL_T fdryDhcpSnoopIfConfigTable_OidIndexToData(UI32_T exact, UI32_T compc,
    oid *compl, UI32_T *ifIndex)
{
    /* get or set
     */
    if (exact)
    {
        /* check the index length
         */
        if (compc != FDRYDHCPSNOOPIFCONFIGENTRY_INSTANCE_LEN) /* the constant size index */
        {
            return FALSE;
        }
    }

    *ifIndex = compl[0];

    return TRUE;
}

/*
 * var_fdryDhcpSnoopIfConfigTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *var_fdryDhcpSnoopIfConfigTable(struct variable *vp,
    oid *name,
    size_t *length,
    int exact,
    size_t *var_len,
    WriteMethod **write_method)
{
    UI32_T compc = 0;
    oid compl[FDRYDHCPSNOOPIFCONFIGENTRY_INSTANCE_LEN] = {0};
    oid best_inst[FDRYDHCPSNOOPIFCONFIGENTRY_INSTANCE_LEN] = {0};

    /* table-specific variables
     */
    DHCPSNP_TYPE_PortInfo_T entry;
    UI32_T lport_ifindex = 0;

    /* dispatch node to set write method
     */
    switch (vp->magic)
    {
        case LEAF_fdryDhcpSnoopIfTrustValue:
            *write_method = write_fdryDhcpSnoopIfTrustValue;
            break;

        default:
            *write_method = 0;
            break;
    }

    /* check compc, retrive compl
     */
    SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc, compl,
        FDRYDHCPSNOOPIFCONFIGENTRY_INSTANCE_LEN);

    memset(&entry, 0, sizeof(entry));

    /* dispatch get-exact versus get-next
     */
    if (exact) /* get or set */
    {
        /* extract index
         */
        if (! fdryDhcpSnoopIfConfigTable_OidIndexToData(exact, compc, compl,
            &lport_ifindex))
        {
            return NULL;
        }

        /* get-exact from core layer
         */
        if (DHCPSNP_PMGR_GetPortInfo(lport_ifindex, &entry) != DHCPSNP_TYPE_OK)
        {
            return NULL;
        }
    }
    else /* get-next */
    {
        /* extract index
         */
        fdryDhcpSnoopIfConfigTable_OidIndexToData(exact, compc, compl,
            &lport_ifindex);

        /* Check the length of inputing index. If compc is less than instance
         * length, we should try get {A.B.C.0.0...}, where A.B.C was
         * obtained from the "..._OidIndexToData" function call, and
         * 0.0... was initialized in the beginning of this function.
         * This instance may exist in the core layer.
         */
        if (compc < FDRYDHCPSNOOPIFCONFIGENTRY_INSTANCE_LEN)  /* incomplete index */
        {
            /* get-exact, in case this instance exists
             */
            if (DHCPSNP_PMGR_GetPortInfo(lport_ifindex, &entry) != DHCPSNP_TYPE_OK)
            {
                /* get-next according to lexicographic order; if none, fail
                 */
                if (DHCPSNP_PMGR_GetNextPortInfo(&lport_ifindex, &entry) != DHCPSNP_TYPE_OK)
                {
                    return NULL;
                }
            }
        }
        else  /* complete index */
        {
            /* get-next according to lexicographic order; if none, fail
             */
            if (DHCPSNP_PMGR_GetNextPortInfo(&lport_ifindex, &entry) != DHCPSNP_TYPE_OK)
            {
                return NULL;
            }
        }
    }

    /* copy base OID (without index) to output
     */
    memcpy(name, vp->name, vp->namelen * sizeof(oid));

    /* assign data to the OID index
     */
    best_inst[0] = lport_ifindex;
    memcpy(name + vp->namelen, best_inst, FDRYDHCPSNOOPIFCONFIGENTRY_INSTANCE_LEN * sizeof(oid));
    *length = vp->namelen + FDRYDHCPSNOOPIFCONFIGENTRY_INSTANCE_LEN;

    /* dispatch node to read value
     */
    switch (vp->magic)
    {
        case LEAF_fdryDhcpSnoopIfTrustValue:
            *var_len = 4;
            long_return = entry.trust_status;  /* TruthValue = EnabledStatus */
            return (u_char *) &long_return;

        default:
            ERROR_MSG("");
            break;
    }

    /* return failure
     */
    return NULL;
}

int write_fdryDhcpSnoopIfTrustValue(int action,
    u_char *var_val,
    u_char var_val_type,
    size_t var_val_len,
    u_char *statP,
    oid *name,
    size_t name_len)
{
    /* dispatch action
     */
    switch (action)
    {
        case RESERVE1:
            /* check type and length
             */
            if (var_val_type != ASN_INTEGER)
            {
                return SNMP_ERR_WRONGTYPE;
            }
            break;

        case RESERVE2:
            /* check valid values
             */
            switch (*(long *) var_val)
            {
                case VAL_fdryDhcpSnoopIfTrustValue_true:
                    break;

                case VAL_fdryDhcpSnoopIfTrustValue_false:
                    break;

                default:
                    return SNMP_ERR_WRONGVALUE;
            }
            break;

        case FREE:
            break;

        case ACTION:
        {
            UI32_T oid_name_length = 15;
            I32_T value = 0;

            /* table-specific variables
             */
            UI32_T lport_ifindex = 0;
            UI8_T  status;

            /* extract index
             */
            if (! fdryDhcpSnoopIfConfigTable_OidIndexToData(TRUE,
                name_len - oid_name_length,
                &(name[oid_name_length]),
                &lport_ifindex))
            {
                return SNMP_ERR_COMMITFAILED;
            }

            /* get user value
             */
            memcpy(&value, var_val, sizeof(I32_T));

            /* convert to value for core layer
             */
            if (value == VAL_fdryDhcpSnoopIfTrustValue_true)
            {
                status = DHCPSNP_TYPE_PORT_TRUSTED;
            }
            else
            {
                status = DHCPSNP_TYPE_PORT_UNTRUSTED;
            }

            /* set to core layer
             */
            if (DHCPSNP_PMGR_SetPortTrustStatus(lport_ifindex, status) != DHCPSNP_TYPE_OK)
            {
                return SNMP_ERR_COMMITFAILED;
            }

            break;
        }

        case UNDO:
            break;

        case COMMIT:
            break;
    }

    /* return success
     */
    return SNMP_ERR_NOERROR;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - init_fdryDhcpSnoopMIB
 * ------------------------------------------------------------------------
 * PURPOSE  :   The is the entry point into this file for initialisation.
 *
 * INPUT    :   None.
 *
 * OUTPUT   :   None.
 *
 * RETURN   :   None.
 *
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
void init_fdryDhcpSnoopMIB(void)
{
    oid fdryDhcpSnoopMIB_oid[] = { 1,3,6,1,4,1,1991,1,1,3,36 };

#if (FOUNDRY_LIB_PLATFORM_STYLE == FOUNDRY_LIB_PLATFORM_STYLE_EDGE_CORE)
    /* register individual scalars
     */
    init_fdryDhcpSnoopGlobalClearOper();
#endif

    /* register individual tables
     */
    init_fdryDhcpSnoopVlanConfigTable();
    init_fdryDhcpSnoopIfConfigTable();

    /* register ourselves in the sysORTable
     */
    REGISTER_SYSOR_ENTRY(fdryDhcpSnoopMIB_oid,
        "Management Information for configuration of DHCP Snooping feature.");
}
#endif  /* (SYS_CPNT_DHCPSNP == TRUE); from near-beginning to end of file */