/* MODULE NAME: pppoe_ia_om.c
 * PURPOSE:
 *   Definitions of OM APIs for PPPOE Intermediate Agent.
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   mm/dd/yy (A.D.)
 *   03/17/09    -- Squid Ro, Create
 *   11/26/09    -- Squid Ro, Modify for Linux platform
 *
 * Copyright(C)      Accton Corporation, 2009
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys_type.h"
#include "sys_dflt.h"
#include "sysfun.h"
#include "swctrl.h"
#include "stktplg_mgr.h"
#include "pppoe_ia_backdoor.h"
#include "pppoe_ia_om.h"
#include "l_stdlib.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTIONS DECLARACTION
 */
#define PPPOE_IA_OM_LOCK()       orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(pppoe_ia_om_semid);
#define PPPOE_IA_OM_UNLOCK()     SYSFUN_OM_LEAVE_CRITICAL_SECTION(pppoe_ia_om_semid, orig_priority);

/* DATA TYPE DECLARATIONS
 */
typedef struct PPPOE_IA_OM_GlobalCfgEntry_S
{
    UI32_T  enable_port_num;
    UI32_T  trust_port_num;
    UI32_T  untrust_port_num;
    BOOL_T  is_enable;
    UI8_T   access_node_id_len;
    UI8_T   generic_ermsg_len;
    UI8_T   access_node_id[PPPOE_IA_TYPE_MAX_ACCESS_NODE_ID_LEN+1];
    UI8_T   generic_ermsg[PPPOE_IA_TYPE_MAX_GENERIC_ERMSG_LEN+1];
    UI8_T   enable_lports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];  /* enabled           */
    UI8_T   trust_lports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];   /* enabled & trust   */
#if 0 /* not used at present */
    UI8_T   untrust_lports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]; /* enabled & untrust */
#endif
} PPPOE_IA_OM_GlobalCfgEntry_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T PPPOE_IA_OM_LocalAddPortToPorts(UI32_T lport);
static void PPPOE_IA_OM_LocalResetOnePortToDefault(UI32_T  lport);

/* STATIC VARIABLE DECLARATIONS
 */
static PPPOE_IA_OM_GlobalCfgEntry_T     pppoe_ia_om_gcfg;
static PPPOE_IA_OM_PortAdmCfgEntry_T    pppoe_ia_om_pcfg[SYS_ADPT_TOTAL_NBR_OF_LPORT];
static PPPOE_IA_OM_PortStsEntry_T       pppoe_ia_om_psts[SYS_ADPT_TOTAL_NBR_OF_LPORT];
static UI32_T                           pppoe_ia_om_semid;
static UI32_T                           orig_priority;

/* EXPORTED SUBPROGRAM BODIES
 */
/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_AddPortToPorts
 *--------------------------------------------------------------------------
 * PURPOSE: To add one specified ifindex to the lport lists. (enabled port only)
 * INPUT  : lport - 1-based ifindex to add to the lport lists
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : 1. update the enable/trust/untrust lport lists according to config
 *          2. for clear, the src_ifidx should be removed explicitly first
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_AddPortToPorts(
    UI32_T  lport)
{
    BOOL_T  ret = FALSE;

    PPPOE_IA_OM_LOCK();
    ret = PPPOE_IA_OM_LocalAddPortToPorts(lport);
    PPPOE_IA_OM_UNLOCK();

    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_ClearOM
 *--------------------------------------------------------------------------
 * PURPOSE: To clear the om database.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void PPPOE_IA_OM_ClearOM(void)
{
    memset(&pppoe_ia_om_gcfg, 0, sizeof(pppoe_ia_om_gcfg));
    memset( pppoe_ia_om_pcfg, 0, sizeof(pppoe_ia_om_pcfg));
    memset( pppoe_ia_om_psts, 0, sizeof(pppoe_ia_om_psts));
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_ClearPortConfig
 *--------------------------------------------------------------------------
 * PURPOSE: To clear the config entry for the specified ifindex.
 * INPUT  : lport - 1-based ifindex to clear
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void PPPOE_IA_OM_ClearPortConfig(
    UI32_T  lport)
{
    if ((1 <= lport) && (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        PPPOE_IA_OM_LOCK();
        memset(&pppoe_ia_om_pcfg[lport-1], 0,
               sizeof(PPPOE_IA_OM_PortAdmCfgEntry_T));
        PPPOE_IA_OM_UNLOCK();
    }
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_ClearPortStatistics
 *--------------------------------------------------------------------------
 * PURPOSE: To clear the statistics entry for the specified ifindex.
 * INPUT  : lport - 1-based ifindex to clear
 *                  (0 to clear all)
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_ClearPortStatistics(
    UI32_T  lport)
{
    BOOL_T  ret = FALSE;

    if (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT)
    {
        PPPOE_IA_OM_LOCK();
        if (0 == lport)
        {
            memset( pppoe_ia_om_psts, 0, sizeof(pppoe_ia_om_psts));
        }
        else
        {
            memset (&pppoe_ia_om_psts[lport-1], 0, sizeof(pppoe_ia_om_psts[lport-1]));
        }
        ret = TRUE;
        PPPOE_IA_OM_UNLOCK();
    }

    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_CopyPortConfigTo
 *--------------------------------------------------------------------------
 * PURPOSE: To copy one config entry from specified src_ifidx to dst_ifidx
 * INPUT  : src_ifidx - 1-based source      ifindex to copy config from
 *          dst_ifidx - 1-based destination ifindex to copy config to
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_CopyPortConfigTo(
    UI32_T  src_ifidx,
    UI32_T  dst_ifidx)
{
    BOOL_T  ret = FALSE;

    if (  ((1 <= src_ifidx) && (src_ifidx <= SYS_ADPT_TOTAL_NBR_OF_LPORT))
        &&((1 <= dst_ifidx) && (dst_ifidx <= SYS_ADPT_TOTAL_NBR_OF_LPORT))
       )
    {
        PPPOE_IA_OM_LOCK();
        memcpy (&pppoe_ia_om_pcfg[dst_ifidx-1],
                &pppoe_ia_om_pcfg[src_ifidx-1], sizeof(PPPOE_IA_OM_PortAdmCfgEntry_T));

        ret = TRUE;
        PPPOE_IA_OM_UNLOCK();
    }

    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_DelPortFromPorts
 *--------------------------------------------------------------------------
 * PURPOSE: To remove one specified ifindex from the lport lists.
 * INPUT  : src_ifidx - 1-based ifindex to remove from the lport lists
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_DelPortFromPorts(
    UI32_T  src_ifidx)
{
    UI32_T  lport_byte;
    UI8_T   lport_byte_mask;
    BOOL_T  ret = FALSE;

    if ((1 <= src_ifidx) && (src_ifidx <= SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        PPPOE_IA_OM_LOCK();

        lport_byte_mask = (1 << (7 - ((src_ifidx - 1) & 7)));
        lport_byte      = (src_ifidx - 1) >> 3;

        if (pppoe_ia_om_gcfg.enable_lports[lport_byte] & lport_byte_mask)
        {
            pppoe_ia_om_gcfg.enable_lports[lport_byte] &= ~lport_byte_mask;
            pppoe_ia_om_gcfg.enable_port_num --;
        }

        if (pppoe_ia_om_gcfg.trust_lports[lport_byte] & lport_byte_mask)
        {
            pppoe_ia_om_gcfg.trust_lports[lport_byte] &= ~lport_byte_mask;
            pppoe_ia_om_gcfg.trust_port_num--;
        }

#if 0 /* not used at present */
        if (pppoe_ia_om_gcfg.untrust_lports[lport_byte] & lport_byte_mask)
        {
            pppoe_ia_om_gcfg.untrust_lports[lport_byte] &= ~lport_byte_mask;
            pppoe_ia_om_gcfg.untrust_port_num--;
        }
#endif
        ret = TRUE;
        PPPOE_IA_OM_UNLOCK();
    }

    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_GetBoolDataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To get boolean data by field id from port or global config entry.
 * INPUT  : lport       - lport to get
 *                         0, get from the global config entry
 *                        >0, get from the port config entry
 *          field_id    - PPPOE_IA_TYPE_FLDID_E
 * OUTPUT : bool_flag_p - pointer to content of output data
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_GetBoolDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    BOOL_T  *bool_flag_p)
{
    BOOL_T  ret = FALSE;

    /* lport == 0, get from global config entry
     * 1 <= lport && lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT, get from port config entry
     */
    if (NULL != bool_flag_p)
    {
        PPPOE_IA_OM_LOCK();
        if (lport == 0)
        {
            switch(field_id)
            {
            case PPPOE_IA_TYPE_FLDID_GLOBAL_ENABLE:
                ret = TRUE;
                *bool_flag_p = pppoe_ia_om_gcfg.is_enable;
                break;
            default:
                break;
            }
        }
        else if (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT)
        {
            switch(field_id)
            {
            case PPPOE_IA_TYPE_FLDID_PORT_ENABLE:
                ret = TRUE;
                *bool_flag_p = pppoe_ia_om_pcfg[lport-1].is_enable;
                break;
            case PPPOE_IA_TYPE_FLDID_PORT_TRUST:
                ret = TRUE;
                *bool_flag_p = pppoe_ia_om_pcfg[lport-1].is_trust;
                break;
            case PPPOE_IA_TYPE_FLDID_PORT_STRIP_VENDOR:
                ret = TRUE;
                *bool_flag_p = pppoe_ia_om_pcfg[lport-1].is_strip_vtag;
                break;
#if (SYS_CPNT_PPPOE_IA_REMOTE_ID_ENHANCE == TRUE)
            case PPPOE_IA_TYPE_FLDID_PORT_RID_DELIM:
                ret = TRUE;
                *bool_flag_p = pppoe_ia_om_pcfg[lport-1].is_rid_delim_en;
                break;
#endif
            default:
                break;
            }
        }
        PPPOE_IA_OM_UNLOCK();
    }
    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_GetUi32DataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To get boolean data by field id from port or global config entry.
 * INPUT  : lport       - lport to get
 *                         0, get from the global config entry
 *                        >0, get from the port config entry
 *          field_id    - PPPOE_IA_TYPE_FLDID_E
 * OUTPUT : ui32_data_p - pointer to content of output data
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_GetUi32DataByField(
    UI32_T  lport,
    UI32_T  field_id,
    UI32_T  *ui32_data_p)
{
    BOOL_T  ret = FALSE;

    /* lport == 0, get from global config entry
     * 1 <= lport && lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT, get from port config entry
     */
    if (NULL != ui32_data_p)
    {
        PPPOE_IA_OM_LOCK();
        if (lport == 0)
        {
        }
        else if (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT)
        {
            switch(field_id)
            {
            case PPPOE_IA_TYPE_FLDID_PORT_RID_DASCII:
                ret = TRUE;
                *ui32_data_p = pppoe_ia_om_pcfg[lport-1].rid_delim_ascii;
                break;
            }
        }
        PPPOE_IA_OM_UNLOCK();
    }
    return ret;
}
/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_GetLports
 *--------------------------------------------------------------------------
 * PURPOSE: To get current lport list for specified port type
 * INPUT  : port_type   - PPPOE_IA_TYPE_PTYPE_E
 *                        port type to get
 * OUTPUT : lports      - pointer to content of output lport list
 *          port_num_p  - pointer to output port number
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_GetLports(
    UI8_T   port_type,
    UI8_T   lports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
    UI32_T  *port_num_p)
{
    BOOL_T  ret = FALSE;

    if ((NULL != lports) && (NULL != port_num_p))
    {
        PPPOE_IA_OM_LOCK();
        ret = TRUE;
        switch(port_type)
        {
        case PPPOE_IA_TYPE_PTYPE_ENABLED:
            *port_num_p = pppoe_ia_om_gcfg.enable_port_num;
            memcpy(lports, pppoe_ia_om_gcfg.enable_lports, sizeof(pppoe_ia_om_gcfg.enable_lports));
            break;
        case PPPOE_IA_TYPE_PTYPE_TRUST:
            *port_num_p = pppoe_ia_om_gcfg.trust_port_num;
            memcpy(lports, pppoe_ia_om_gcfg.trust_lports, sizeof(pppoe_ia_om_gcfg.trust_lports));
            break;
#if 0 /* not used at present */
        case PPPOE_IA_TYPE_PTYPE_UNTRUST:
            *port_num_p = pppoe_ia_om_gcfg.untrust_port_num;
            memcpy(lports, pppoe_ia_om_gcfg.untrust_lports, sizeof(pppoe_ia_om_gcfg.untrust_lports));
            break;
#endif
        default:
            ret = FALSE;
            break;
        }
        PPPOE_IA_OM_UNLOCK();

        PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_OM,
            "egr port_num/type/ret: %ld/%d/%d",
            *port_num_p, port_type, ret);
    }

    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_GetPortAdmCfgEntry
 *--------------------------------------------------------------------------
 * PURPOSE: To get one port config entry for specified lport.
 * INPUT  : lport      - 1-based lport
 * OUTPUT : pcfg_ent_p - pointer to the output buffer
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_GetPortAdmCfgEntry(
    UI32_T                          lport,
    PPPOE_IA_OM_PortAdmCfgEntry_T   *pcfg_ent_p)
{
    BOOL_T  ret = FALSE;

    if (  (NULL != pcfg_ent_p)
        &&((1 <= lport) && (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT))
       )
    {
        PPPOE_IA_OM_LOCK();
        memcpy(pcfg_ent_p, &pppoe_ia_om_pcfg[lport-1],
               sizeof(PPPOE_IA_OM_PortAdmCfgEntry_T));
        PPPOE_IA_OM_UNLOCK();
        ret = TRUE;
    }
    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_GetPortStatisticsEntry
 *--------------------------------------------------------------------------
 * PURPOSE: To get the port statistics entry for specified lport.
 * INPUT  : lport      - 1-based lport
 * OUTPUT : psts_ent_p - pointer to the output buffer
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_GetPortStatisticsEntry(
    UI32_T                      lport,
    PPPOE_IA_OM_PortStsEntry_T  *psts_ent_p)
{
    BOOL_T  ret = FALSE;

    if (  (NULL != psts_ent_p)
        &&((1 <= lport) && (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT))
       )
    {
        PPPOE_IA_OM_LOCK();
        memcpy(psts_ent_p, &pppoe_ia_om_psts[lport-1],
               sizeof(PPPOE_IA_OM_PortStsEntry_T));
        PPPOE_IA_OM_UNLOCK();
        ret = TRUE;
    }
    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_GetRunningBoolDataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To get running bool data by field id from port or global config entry.
 * INPUT  : lport       - lport
 *                         0, get from the global config entry
 *                        >0, get from the port config entry
 *          field_id    - PPPOE_IA_TYPE_FLDID_E
 * OUTPUT : bool_flag_p - pointer to content of output data
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
UI32_T PPPOE_IA_OM_GetRunningBoolDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    BOOL_T  *bool_flag_p)
{
    UI32_T  ret = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    BOOL_T  def_val = FALSE;
    BOOL_T  bool_tmp;

    if (NULL != bool_flag_p)
    {
        bool_tmp = PPPOE_IA_OM_GetBoolDataByField(lport, field_id, bool_flag_p);

        if (TRUE == bool_tmp)
        {
            switch (field_id)
            {
            case PPPOE_IA_TYPE_FLDID_GLOBAL_ENABLE:
                def_val = SYS_DFLT_PPPOE_IA_GLOBAL_STATUS;
                break;
            case PPPOE_IA_TYPE_FLDID_PORT_ENABLE:
                def_val = SYS_DFLT_PPPOE_IA_PORT_STATUS;
                break;
            case PPPOE_IA_TYPE_FLDID_PORT_TRUST:
                def_val = SYS_DFLT_PPPOE_IA_PORT_TRUST;
                break;
            case PPPOE_IA_TYPE_FLDID_PORT_STRIP_VENDOR:
                def_val = SYS_DFLT_PPPOE_IA_PORT_VENDOR_STRIP;
                break;
            }

            if (*bool_flag_p != def_val)
            {
                ret = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
            }
            else
            {
                ret = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
            }
        }
    }
    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_GetRunningUi32DataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To get running ui32 data by field id from port or global config entry.
 * INPUT  : lport       - lport
 *                         0, get from the global config entry
 *                        >0, get from the port config entry
 *          field_id    - PPPOE_IA_TYPE_FLDID_E
 * OUTPUT : ui32_data_p - pointer to content of output data
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
UI32_T PPPOE_IA_OM_GetRunningUi32DataByField(
    UI32_T  lport,
    UI32_T  field_id,
    UI32_T  *ui32_data_p)
{
    UI32_T  ret = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    UI32_T  def_val;
    BOOL_T  bool_tmp;

    if (NULL != ui32_data_p)
    {
        bool_tmp = PPPOE_IA_OM_GetUi32DataByField(lport, field_id, ui32_data_p);

        if (TRUE == bool_tmp)
        {
            switch (field_id)
            {
            case PPPOE_IA_TYPE_FLDID_PORT_RID_DASCII:
                def_val = PPPOE_IA_TYPE_DFLT_RID_DASCII;
                break;
            }

            if (*ui32_data_p != def_val)
            {
                ret = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
            }
            else
            {
                ret = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
            }
        }
    }
    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_GetRunningStrDataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To get running string data by field id from port or global config entry.
 * INPUT  : lport       - lport
 *                         0, get from the global config entry
 *                        >0, get from the port config entry
 *          field_id    - PPPOE_IA_TYPE_FLDID_E
 *          str_len_max - maximum length of buffer to receive the string
 * OUTPUT : string_p    - pointer to content of output data
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
UI32_T PPPOE_IA_OM_GetRunningStrDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    UI32_T  str_len_max,
    UI8_T   *string_p)
{
    UI32_T  ret = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    I32_T   len_max;
    BOOL_T  bool_tmp;

    if (NULL != string_p)
    {
        len_max = str_len_max;
        bool_tmp = PPPOE_IA_OM_GetAdmStrDataByField(lport, field_id, &len_max, string_p);

        if ((TRUE == bool_tmp) && (len_max > 0))
        {
            ret = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

            switch (field_id)
            {
            case PPPOE_IA_TYPE_FLDID_GLOBAL_GEN_ERMSG:
                if (0 == strcmp((char *)string_p, SYS_DFLT_PPPOE_IA_GENERIC_ERMSG))
                {
                    ret = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
                }
                break;
            case PPPOE_IA_TYPE_FLDID_GLOBAL_ACC_NODE_ID:
                if (0 == strcmp((char *)string_p, SYS_DFLT_PPPOE_IA_ACCESS_NODE_ID))
                {
                    ret = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
                }
                break;
            }
        }
        else
        {
            ret = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
        }
    }
    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_GetAdmStrDataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To get string data by field id from port or global config entry.
 * INPUT  : lport     - lport
 *                       0, get from the global config entry
 *                      >0, get from the port config entry
 *          field_id  - PPPOE_IA_TYPE_FLDID_E
 *          str_len_p - length of buffer to receive the string
 * OUTPUT : str_len_p - length of buffer to receive the string
 *          string_p  - pointer to content of output data
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_GetAdmStrDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    I32_T   *str_len_p,
    UI8_T   *string_p)
{
    BOOL_T  ret = FALSE;

    /* lport == 0, get from global config entry
     * 1 <= lport && lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT, get from port config entry
     */
    if ((NULL != str_len_p) && (NULL != string_p))
    {
        PPPOE_IA_OM_LOCK();
        if (lport == 0)
        {
            switch(field_id)
            {
            case PPPOE_IA_TYPE_FLDID_GLOBAL_ACC_NODE_ID:
                if (*str_len_p >= pppoe_ia_om_gcfg.access_node_id_len+1)
                {
                    *str_len_p = pppoe_ia_om_gcfg.access_node_id_len;
                    if (pppoe_ia_om_gcfg.access_node_id_len > 0)
                    {
                        memcpy(string_p, pppoe_ia_om_gcfg.access_node_id,
                                         pppoe_ia_om_gcfg.access_node_id_len+1);
                    }
                    else
                    {
                        string_p[0] = '\0';
                    }
                    ret = TRUE;
                }
                break;
            case PPPOE_IA_TYPE_FLDID_GLOBAL_GEN_ERMSG:
                if (*str_len_p >= pppoe_ia_om_gcfg.generic_ermsg_len+1)
                {
                    *str_len_p = pppoe_ia_om_gcfg.generic_ermsg_len;
                    if (pppoe_ia_om_gcfg.generic_ermsg_len > 0)
                    {
                        memcpy(string_p, pppoe_ia_om_gcfg.generic_ermsg,
                                         pppoe_ia_om_gcfg.generic_ermsg_len+1);
                    }
                    else
                    {
                        string_p[0] = '\0';
                    }
                    ret = TRUE;
                }
                break;
            default:
                break;
            }
        }
        else if (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT)
        {
            switch(field_id)
            {
            case PPPOE_IA_TYPE_FLDID_PORT_CIRCUIT_ID:
                if (*str_len_p >= pppoe_ia_om_pcfg[lport-1].circuit_id_len+1)
                {
                    *str_len_p = pppoe_ia_om_pcfg[lport-1].circuit_id_len;
                    if (pppoe_ia_om_pcfg[lport-1].circuit_id_len > 0)
                    {
                        memcpy(string_p, pppoe_ia_om_pcfg[lport-1].circuit_id,
                                         pppoe_ia_om_pcfg[lport-1].circuit_id_len+1);
                    }
                    else
                    {
                        string_p[0] = '\0';
                    }
                    ret = TRUE;
                }
                break;
            case PPPOE_IA_TYPE_FLDID_PORT_REMOTE_ID:
                if (*str_len_p >= pppoe_ia_om_pcfg[lport-1].remote_id_len+1)
                {
                    *str_len_p = pppoe_ia_om_pcfg[lport-1].remote_id_len;
                    if (pppoe_ia_om_pcfg[lport-1].remote_id_len > 0)
                    {
                        memcpy(string_p, pppoe_ia_om_pcfg[lport-1].remote_id,
                                         pppoe_ia_om_pcfg[lport-1].remote_id_len+1);
                    }
                    else
                    {
                        string_p[0] = '\0';
                    }
                    ret = TRUE;
                }
                break;
            default:
                break;
            }
        }
        PPPOE_IA_OM_UNLOCK();
    }
    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_ResetOnePortToDefault
 *--------------------------------------------------------------------------
 * PURPOSE: To reset the om database of one port to default.
 * INPUT  : lport - 1-based ifindex
 * OUTPUT : None
 * RETURN : None
 * NOTES  : 1. need to reset port list by other API.
 *--------------------------------------------------------------------------
 */
void PPPOE_IA_OM_ResetOnePortToDefault(
    UI32_T  lport)
{
    if (lport > 0)
    {
        PPPOE_IA_OM_LocalResetOnePortToDefault(lport-1);
    }
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_ResetToDefault
 *--------------------------------------------------------------------------
 * PURPOSE: To reset the om database to default.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : 1. called by PPPOE_IA_MGR_EnterMasterMode
 *          2. should use PPPOE_IA_ENGINE_SetDefaultConfig to
 *             setup the env for this config.
 *--------------------------------------------------------------------------
 */
void PPPOE_IA_OM_ResetToDefault(void)
{
    UI32_T  ifidx;

    strcpy((char *)pppoe_ia_om_gcfg.access_node_id, SYS_DFLT_PPPOE_IA_ACCESS_NODE_ID);
    pppoe_ia_om_gcfg.access_node_id_len = strlen((char *)pppoe_ia_om_gcfg.access_node_id);
    strcpy((char *)pppoe_ia_om_gcfg.generic_ermsg,  SYS_DFLT_PPPOE_IA_GENERIC_ERMSG);
    pppoe_ia_om_gcfg.generic_ermsg_len  = strlen((char *)pppoe_ia_om_gcfg.generic_ermsg);
    pppoe_ia_om_gcfg.is_enable          = SYS_DFLT_PPPOE_IA_GLOBAL_STATUS;

    for (ifidx =0; ifidx <SYS_ADPT_TOTAL_NBR_OF_LPORT; ifidx++)
    {
        PPPOE_IA_OM_LocalResetOnePortToDefault(ifidx);
    }
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_SetBoolDataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To set boolean data by field id to port or global config entry.
 * INPUT  : lport     - lport
 *                       0, set to the global config entry
 *                      >0, set to the port config entry
 *          field_id  - PPPOE_IA_TYPE_FLDID_E
 *          bool_flag - content of input data
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_SetBoolDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    BOOL_T  bool_flag)
{
    BOOL_T  ret = FALSE;

    /* lport == 0, set to global config entry
     * 1 <= lport && lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT, set to port config entry
     */
    PPPOE_IA_OM_LOCK();
    if (lport == 0)
    {
        switch(field_id)
        {
        case PPPOE_IA_TYPE_FLDID_GLOBAL_ENABLE:
            ret = TRUE;
            pppoe_ia_om_gcfg.is_enable = bool_flag;
            break;
        default:
            break;
        }
    }
    else if (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT)
    {
        switch(field_id)
        {
        case PPPOE_IA_TYPE_FLDID_PORT_ENABLE:
            ret = TRUE;
            pppoe_ia_om_pcfg[lport-1].is_enable = bool_flag;
            break;
        case PPPOE_IA_TYPE_FLDID_PORT_TRUST:
            ret = TRUE;
            pppoe_ia_om_pcfg[lport-1].is_trust = bool_flag;
            break;
        case PPPOE_IA_TYPE_FLDID_PORT_STRIP_VENDOR:
            ret = TRUE;
            pppoe_ia_om_pcfg[lport-1].is_strip_vtag = bool_flag;
            break;
#if (SYS_CPNT_PPPOE_IA_REMOTE_ID_ENHANCE == TRUE)
        case PPPOE_IA_TYPE_FLDID_PORT_RID_DELIM:
            ret = TRUE;
            pppoe_ia_om_pcfg[lport-1].is_rid_delim_en = bool_flag;
            break;
#endif
        default:
            break;
        }
    }
    PPPOE_IA_OM_UNLOCK();

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_OM,
        "lport/field_id/bool/ret: %ld/%ld/%d/%d",
        lport, field_id, bool_flag, ret);

    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_SetUi32DataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To set boolean data by field id to port or global config entry.
 * INPUT  : lport     - lport
 *                       0, set to the global config entry
 *                      >0, set to the port config entry
 *          field_id  - PPPOE_IA_TYPE_FLDID_E
 *          ui32_data - content of input data
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_SetUi32DataByField(
    UI32_T  lport,
    UI32_T  field_id,
    UI32_T  ui32_data)
{
    BOOL_T  ret = FALSE;

    /* lport == 0, set to global config entry
     * 1 <= lport && lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT, set to port config entry
     */
    PPPOE_IA_OM_LOCK();
    if (lport == 0)
    {
    }
    else if (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT)
    {
        switch(field_id)
        {
        case PPPOE_IA_TYPE_FLDID_PORT_RID_DASCII:
            ret = TRUE;
            pppoe_ia_om_pcfg[lport-1].rid_delim_ascii = ui32_data;
            break;
        default:
            break;
        }
    }
    PPPOE_IA_OM_UNLOCK();

    PPPOE_IA_BDR_MSG(PPPOE_IA_DBG_OM,
        "lport/field_id/ui32/ret: %ld/%ld/%ld/%d",
        lport, field_id, ui32_data, ret);

    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_SetAdmStrDataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To set string data by field id from port or global config entry.
 * INPUT  : lport     - lport
 *                         0, set to the global config entry
 *                        >0, set to the port config entry
 *          field_id  - PPPOE_IA_TYPE_FLDID_E
 *          str_len   - length of input string, 0 to reset to default
 *          string_p  - pointer to content of input string
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_SetAdmStrDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    UI8_T   *string_p,
    UI8_T   str_len)
{
    BOOL_T  ret = FALSE;

    /* lport == 0, set to global config entry
     * 1 <= lport && lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT, get from port config entry
     */
    if ((str_len == 0) || (NULL != string_p))
    {
        PPPOE_IA_OM_LOCK();
        if (lport == 0)
        {
            switch(field_id)
            {
            case PPPOE_IA_TYPE_FLDID_GLOBAL_ACC_NODE_ID:
                if (str_len <= PPPOE_IA_TYPE_MAX_ACCESS_NODE_ID_LEN)
                {
                    ret = TRUE;
                    pppoe_ia_om_gcfg.access_node_id_len = str_len;
                    if (str_len > 0)
                    {
                        memcpy(pppoe_ia_om_gcfg.access_node_id, string_p,
                               str_len+1);
                    }
                    else
                    {
                        strcpy((char *)pppoe_ia_om_gcfg.access_node_id, SYS_DFLT_PPPOE_IA_ACCESS_NODE_ID);
                        pppoe_ia_om_gcfg.access_node_id_len = strlen((char *)pppoe_ia_om_gcfg.access_node_id);
                    }
                }
                break;
            case PPPOE_IA_TYPE_FLDID_GLOBAL_GEN_ERMSG:
                if (str_len <= PPPOE_IA_TYPE_MAX_GENERIC_ERMSG_LEN)
                {
                    ret = TRUE;
                    pppoe_ia_om_gcfg.generic_ermsg_len = str_len;
                    if (str_len > 0)
                    {
                        memcpy(pppoe_ia_om_gcfg.generic_ermsg, string_p,
                               str_len+1);
                    }
                    else
                    {
                        strcpy((char *)pppoe_ia_om_gcfg.generic_ermsg,  SYS_DFLT_PPPOE_IA_GENERIC_ERMSG);
                        pppoe_ia_om_gcfg.generic_ermsg_len  = strlen((char *)pppoe_ia_om_gcfg.generic_ermsg);
                    }
                }
                break;
            default:
                break;
            }
        }
        else if (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT)
        {
            switch(field_id)
            {
            case PPPOE_IA_TYPE_FLDID_PORT_CIRCUIT_ID:
                if (str_len <= PPPOE_IA_TYPE_MAX_CIRCUIT_ID_LEN)
                {
                    ret = TRUE;
                    pppoe_ia_om_pcfg[lport-1].circuit_id_len = str_len;
                    if (str_len > 0)
                    {
                        memcpy(pppoe_ia_om_pcfg[lport-1].circuit_id, string_p,
                               str_len+1);
                    }
                    else
                    {
                        pppoe_ia_om_pcfg[lport-1].circuit_id[0] = '\0';
                    }
                }
                break;
            case PPPOE_IA_TYPE_FLDID_PORT_REMOTE_ID:
                if (str_len <= PPPOE_IA_TYPE_MAX_REMOTE_ID_LEN)
                {
                    ret = TRUE;
                    pppoe_ia_om_pcfg[lport-1].remote_id_len = str_len;
                    if (str_len > 0)
                    {
                        memcpy(pppoe_ia_om_pcfg[lport-1].remote_id, string_p,
                               str_len+1);
                    }
                    else
                    {
                        pppoe_ia_om_pcfg[lport-1].remote_id[0] = '\0';
                    }
                }
                break;
            default:
                break;
            }
        }
        PPPOE_IA_OM_UNLOCK();
    }

    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_IncreaseStatisticsByField
 *--------------------------------------------------------------------------
 * PURPOSE: To increase the statistics for specified lport and field id.
 * INPUT  : lport      - 1-based lport to update
 *          ing_fld_id - PPPOE_IA_TYPE_FLDID_E
 *                       ingress type of packet
 *          err_fld_id - PPPOE_IA_TYPE_FLDID_E
 *                       drop reason
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_IncreaseStatisticsByField(
    UI32_T  lport,
    UI32_T  ing_fld_id,
    UI32_T  err_fld_id)
{
    BOOL_T  ret = FALSE;

    if ((1 <= lport) && (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        PPPOE_IA_OM_LOCK();
        ret = TRUE;

        switch(ing_fld_id)
        {
        case PPPOE_IA_TYPE_FLDID_PORT_STS_PADI:
            pppoe_ia_om_psts[lport-1].padi++;
            break;
        case PPPOE_IA_TYPE_FLDID_PORT_STS_PADO:
            pppoe_ia_om_psts[lport-1].pado++;
            break;
        case PPPOE_IA_TYPE_FLDID_PORT_STS_PADR:
            pppoe_ia_om_psts[lport-1].padr++;
            break;
        case PPPOE_IA_TYPE_FLDID_PORT_STS_PADS:
            pppoe_ia_om_psts[lport-1].pads++;
            break;
        case PPPOE_IA_TYPE_FLDID_PORT_STS_PADT:
            pppoe_ia_om_psts[lport-1].padt++;
            break;
        default:
            ret = FALSE;
            break;
        }

        if (TRUE == ret)
        {
            pppoe_ia_om_psts[lport-1].all++;

            switch(err_fld_id)
            {
            case PPPOE_IA_TYPE_FLDID_PORT_STS_MALFORM:
                pppoe_ia_om_psts[lport-1].malform++;
                break;
            case PPPOE_IA_TYPE_FLDID_PORT_STS_REP_UNTRUST:
                pppoe_ia_om_psts[lport-1].rep_untrust++;
                break;
            case PPPOE_IA_TYPE_FLDID_PORT_STS_REQ_UNTRUST:
                pppoe_ia_om_psts[lport-1].req_untrust++;
                break;
            default:
                ret = FALSE;
                break;
            }
        }
        PPPOE_IA_OM_UNLOCK();
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_InitSemaphore
 * ------------------------------------------------------------------------
 * PURPOSE  :   Initiate the semaphore for PPPOE IA objects
 * INPUT    :   none
 *              none
 * OUTPUT   :   none
 * RETURN   :   none
 * NOTE     :   This function is invoked in PPPOE_IA_MGR_InitiateSystemResources.
 *-------------------------------------------------------------------------
 */
void PPPOE_IA_OM_InitSemaphore(void)
{
    if (SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO,
            &pppoe_ia_om_semid) != SYSFUN_OK)
    {
        printf("\n%s: get pppoe ia om sem id fail.\n", __FUNCTION__);
    }
}

/* LOCAL SUBPROGRAM BODIES
 */
/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_LocalAddPortToPorts
 *--------------------------------------------------------------------------
 * PURPOSE: To add one specified ifindex to the lport lists. (enabled port only)
 * INPUT  : lport - 1-based ifindex to add to the lport lists
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : 1. update the enable/trust/untrust lport lists according to config
 *          2. for clear, the src_ifidx should be removed explicitly first
 *--------------------------------------------------------------------------
 */
static BOOL_T PPPOE_IA_OM_LocalAddPortToPorts(
    UI32_T  lport)
{
    UI32_T  lport_byte;
    UI8_T   lport_byte_mask;
    BOOL_T  ret = FALSE;

    if ((1 <= lport) && (lport<= SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        lport_byte_mask = (1 << (7 - ((lport - 1) & 7)));
        lport_byte      = (lport - 1) >> 3;

        if (0 == (pppoe_ia_om_gcfg.enable_lports[lport_byte] & lport_byte_mask))
        {
            if (TRUE == pppoe_ia_om_pcfg[lport-1].is_enable)
            {
                pppoe_ia_om_gcfg.enable_lports[lport_byte] |= lport_byte_mask;
                pppoe_ia_om_gcfg.enable_port_num ++;

                if (TRUE == pppoe_ia_om_pcfg[lport-1].is_trust)
                {
                    pppoe_ia_om_gcfg.trust_lports[lport_byte] |= lport_byte_mask;
                    pppoe_ia_om_gcfg.trust_port_num ++;
                }
#if 0 /* not used at present */
                else
                {
                    pppoe_ia_om_gcfg.untrust_lports[lport_byte] |= lport_byte_mask;
                    pppoe_ia_om_gcfg.untrust_port_num ++;
                }
#endif
            }
        }
        ret = TRUE;
    }

    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_LocalResetOnePortToDefault
 *--------------------------------------------------------------------------
 * PURPOSE: To reset the om database of one port to default.
 * INPUT  : lport - 0-based ifindex
 * OUTPUT : None
 * RETURN : None
 * NOTES  : 1. need to reset port list by other API.
 *--------------------------------------------------------------------------
 */
static void PPPOE_IA_OM_LocalResetOnePortToDefault(
    UI32_T  lport)
{
    pppoe_ia_om_pcfg[lport].is_enable = SYS_DFLT_PPPOE_IA_PORT_STATUS;
    pppoe_ia_om_pcfg[lport].is_trust  = SYS_DFLT_PPPOE_IA_PORT_TRUST;
    pppoe_ia_om_pcfg[lport].is_strip_vtag = SYS_DFLT_PPPOE_IA_PORT_VENDOR_STRIP;
#if (SYS_CPNT_PPPOE_IA_REMOTE_ID_ENHANCE == TRUE)
    pppoe_ia_om_pcfg[lport].is_rid_delim_en = PPPOE_IA_TYPE_DFLT_RID_DELIM_EN;
    pppoe_ia_om_pcfg[lport].rid_delim_ascii = PPPOE_IA_TYPE_DFLT_RID_DASCII;
#endif
}


