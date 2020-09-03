/* MODULE NAME:  foundry_lib.c
 *
 * PURPOSE: Library for common routines for the Brocade Foundry MIB.
 *
 * NOTES:
 *
 * HISTORY (mm/dd/yyyy)
 *    05/17/2011 - Qiyao Zhong, Created
 *
 * Copyright(C)      Accton Corporation, 2011
 */

/* system
 */
#include "leaf_sys.h"

/* driver
 */
#include "fs.h"

/* core (L2)
 */
#include "swctrl.h"
#include "swctrl_pom.h"
#include "swctrl_pmgr.h"

/* core (management)
 */
#include "stktplg_pom.h"

/* self: make sure prototypes match
 */
#include "foundry_lib.h"

/* ------------------------------------------------------------------------
 * FUNCTION NAME - FOUNDRY_LIB_DivideWithSymmetricRounding
 * ------------------------------------------------------------------------
 * PURPOSE  :   Integer division with symetric rounding,
 *              using positive integer division and symetric "drop 4, take 5",
 *              by a rounding half-integer value to an even number:
 *              3.5 -> 4; 4.5 -> 4; 4.51 -> 5; 5.5 -> 6; 6.5 -> 6
 *
 * INPUT    :   in_dividend -- input dividend (the be-divided value, I32_T)
 *              in_divisor  -- input divisor (the dividing value, UI32_T)
 *
 * RETURN   :   I32_T, quotient, rounded to nearest integer;
 *              half-integer values rounded to even number
 *
 * NOTE     :   To protect against division by 0,
 *              if "in_divisor" is 0, the return value is 0.
 *
 * REF      :  http://www.wikipedia.org/wiki/Rounding#Round_half_to_even
 * ------------------------------------------------------------------------
 */
I32_T FOUNDRY_LIB_DivideWithSymmetricRounding(I32_T in_dividend, UI32_T in_divisor)
{
    I32_T ret = 0;  /* positive or negative */
    BOOL_T gez;     /* in_dividend is greater than or equeal to zero */
    UI32_T abs_dividend, abs_quot, abs_mod, abs_quot_nearest;

    /* protection against division by zero
     */
    if (in_divisor != 0)
    {
        /* get absolute value
         */
        gez = in_dividend >= 0;

        if (gez)
        {
            abs_dividend = (UI32_T) in_dividend;  /* positive */
        }
        else
        {
            abs_dividend = (UI32_T) (- in_dividend);  /* make positive number */
        }

        /* get qoutient and modulo
         */
        abs_quot = abs_dividend / in_divisor;
        abs_mod = abs_dividend % in_divisor;

        /* symmetric treatment for 5
         */
        if (abs_mod * 2 == in_divisor)  /* is "0.5" */
        {
            if (abs_quot % 2 == 0)  /* is even number */
            {
                abs_quot_nearest = abs_quot;  /* round down */
            }
            else
            {
                abs_quot_nearest = abs_quot + 1;  /* round up */
            }
        }

        /* drop 4
         */
        else if (abs_mod * 2 < in_divisor)  /* is < "0.5" */
        {
            abs_quot_nearest = abs_quot;  /* round down */
        }

        /* take 5, but not exactly 5
         */
        else  /* is > "0.5" */
        {
            abs_quot_nearest = abs_quot + 1;  /* round up */
        }

        /* output: give back its positive or negative sign
         */
        if (gez)
        {
            ret = (I32_T) abs_quot_nearest;  /* positive */
        }
        else
        {
            ret = - (I32_T) abs_quot_nearest;  /* make negative number */
        }
    }

    /* return; single exit-point
     */
    return ret;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - FOUNDRY_LIB_ConvertAcctonToFoundryEnabledStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Converts Accton EnabledStatus to Foundry enabled status.
 *
 * INPUT    :   accton_val   -- one of these from private MIB's EnabledStatus,
 *              where <node> is any node name:
 *              VAL_<node>_enabled(1)
 *              VAL_<node>_disabled(2)
 *
 * OUTPUT   :   None.
 *
 * RETURN   :   one of these, from Brocade private MIB:
 *              0   -- disabled
 *              1   -- enabled
 *
 * NOTES    :   An illegal input will return disabled(0).
 * ------------------------------------------------------------------------
 */
UI32_T FOUNDRY_LIB_ConvertAcctonToFoundryEnabledStatus(UI32_T accton_val)
{
    /* convert Accton value to Brocade value (ignore illegal input)
     */
    if (accton_val == 1)  /* VAL_<node>_enabled(1) */
    {
        return 1;  /* Brocade "enabled", hard-coded */
    }
    else  /* VAL_<node>_disabled(2), or illegal value */
    {
        return 0;  /* Brocade "disabled", hard-coded */
    }
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - FOUNDRY_LIB_ConvertFoundryToAcctonEnabledStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Converts Foundry EnabledStatus to Foundry enabled status.
 *
 * INPUT    :   brocade_val   -- one of the following, from Brocade private MIB:
 *              0   -- disabled
 *              1   -- enabled
 *
 * OUTPUT   :   None.
 *
 * RETURN   :   one of these from private MIB's EnabledStatus,
 *              where <node> is any node name:
 *              VAL_<node>_enabled(1)
 *              VAL_<node>_disabled(2)
 *
 * NOTES    :   An illegal input will return disabled(2).
 * ------------------------------------------------------------------------
 */
UI32_T FOUNDRY_LIB_ConvertFoundryToAcctonEnabledStatus(UI32_T foundry_val)
{
    /* convert Brocade value to Accton value (ignore illegal input)
     */
    if (foundry_val == 1)  /* Brocade "enabled", hard-coded */
    {
        return 1;  /* VAL_<node>_enabled(1) */
    }
    else  /* Brocade "disabled", hard-coded */
    {
        return 2;  /* VAL_<node>_disabled(2), or illegal value */
    }
}

#if (FOUNDRY_LIB_PLATFORM_STYLE == FOUNDRY_LIB_PLATFORM_STYLE_SIMBA)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - FOUNDRY_LIB_GetSfpPort (Simba)
 * ------------------------------------------------------------------------
 * PURPOSE  :   Gets an SFP port for Simba platform.
 *              Based on the model of SWCTRL_POM_GetNextLogicalPort.
 *              Body of function based on CLI "show interfaces transceiver".
 *
 * INPUT    :   lport   -- logical port representing the SFP port
 *
 * OUTPUT   :   *unit_p         -- unit ID for this logical port
 *              *sfp_index_p    -- SFP index used by Stack Topology
 *              *present_p      -- whether a transceiver is present
 *
 * RETURN   :   TRUE    -- this port is an SFP
 *              FALSE   -- this port is not an SFP
 *
 * NOTES    :   If this function returns FALSE, output values are undefined.
 * ------------------------------------------------------------------------
 */
BOOL_T FOUNDRY_LIB_GetSfpPort(UI32_T lport, UI32_T *unit_p, UI32_T *sfp_index_p, BOOL_T *present_p)
{
    UI32_T ret = TRUE;
    UI32_T port, trunk, swctrl_ret;

    /* convert to (unit, port)
     */
    swctrl_ret = SWCTRL_POM_LogicalPortToUserPort(lport, unit_p, &port, &trunk);

    if ((swctrl_ret == SWCTRL_LPORT_UNKNOWN_PORT)
        || (swctrl_ret == SWCTRL_LPORT_TRUNK_PORT))
    {
        ret = FALSE;
    }

   /* convert to SFP index
     */
    else if (! STKTPLG_POM_UserPortToComboSfpIndex(*unit_p, port, sfp_index_p))
    {
        ret = FALSE;
    }

    /* if not present
     */
    /*!!PATCH: should pass "unit"
     */
    else if (! STKTPLG_POM_IsComboSfpPresentInternal(SYS_VAL_LOCAL_UNIT_ID, *sfp_index_p))
    {
        *present_p = FALSE;
    }

    /* now it is present
     */
    else
    {
        *present_p = TRUE;
    }

    /* return
     */
    return ret;
}
#endif  /* (FOUNDRY_LIB_PLATFORM_STYLE == FOUNDRY_LIB_PLATFORM_STYLE_SIMBA) */

#if (FOUNDRY_LIB_PLATFORM_STYLE == FOUNDRY_LIB_PLATFORM_STYLE_EDGE_CORE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - FOUNDRY_LIB_GetSfpPort (Simba)
 * ------------------------------------------------------------------------
 * PURPOSE  :   Gets an SFP port for Simba platform.
 *              Based on the model of SWCTRL_POM_GetNextLogicalPort.
 *              Body of function based on CLI "show interfaces transceiver".
 *
 * INPUT    :   lport   -- logical port representing the SFP port
 *
 * OUTPUT   :   *unit_p         -- unit ID for this logical port
 *              *sfp_index_p    -- SFP index used by Stack Topology
 *              *present_p      -- whether a transceiver is present
 *
 * RETURN   :   TRUE    -- this port is an SFP
 *              FALSE   -- this port is not an SFP
 *
 * NOTES    :   If this function returns FALSE, output values are undefined.
 * ------------------------------------------------------------------------
 */
BOOL_T FOUNDRY_LIB_GetSfpPort(UI32_T lport, UI32_T *unit_p, UI32_T *sfp_index_p, BOOL_T *present_p)
{
    UI32_T ret = TRUE;
    UI32_T port, trunk, swctrl_ret;

    /* convert to (unit, port)
     */
    swctrl_ret = SWCTRL_POM_LogicalPortToUserPort(lport, unit_p, &port, &trunk);

    if ((swctrl_ret == SWCTRL_LPORT_UNKNOWN_PORT)
        || (swctrl_ret == SWCTRL_LPORT_TRUNK_PORT))
    {
        ret = FALSE;
    }

    /* convert to SFP index
     */
    else if (! STKTPLG_POM_UserPortToSfpIndex(*unit_p, port, sfp_index_p))
    {
        ret = FALSE;
    }

    /* if not present
     */
    else if (! STKTPLG_POM_IsSfpPresent(*unit_p, *sfp_index_p))
    {
        *present_p = FALSE;
    }

    /* now it is present
     */
    else
    {
        *present_p = TRUE;
    }

    /* return
     */
    return ret;
}
#endif  /* (FOUNDRY_LIB_PLATFORM_STYLE == FOUNDRY_LIB_PLATFORM_STYLE_EDGE_CORE) */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - FOUNDRY_LIB_GetNextSfpPort
 * ------------------------------------------------------------------------
 * PURPOSE  :   Gets the next SFP port.
 *              Based on the model of SWCTRL_POM_GetNextLogicalPort.
 *              Body of function based on CLI "show interfaces transceiver".
 *
 * INPUT    :   lport   -- logical port representing the SFP port
 *
 * OUTPUT   :   *unit_p         -- unit ID for this logical port
 *              *sfp_index_p    -- SFP index used by Stack Topology
 *              *present_p      -- whether a transceiver is present
 *
 * RETURN   :   TRUE    -- this port is an SFP
 *              FALSE   -- this port is not an SFP
 *
 * NOTES    :   If this function returns FALSE, output values are undefined.
 * ------------------------------------------------------------------------
 */
BOOL_T FOUNDRY_LIB_GetNextSfpPort(UI32_T *lport_p, UI32_T *unit_p, UI32_T *sfp_index_p, BOOL_T *present_p)
{
    UI32_T ret = FALSE;

    /* keep on getting next
     */
    while (SWCTRL_POM_GetNextLogicalPort(lport_p))
    {
        /* get SFP
         */
        if (FOUNDRY_LIB_GetSfpPort(*lport_p, unit_p, sfp_index_p, present_p))
        {
            ret = TRUE;
            break;
        }
    }

    /* return
     */
    return ret;
}
