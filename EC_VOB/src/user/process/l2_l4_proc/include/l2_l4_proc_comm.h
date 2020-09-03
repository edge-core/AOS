/*-----------------------------------------------------------------------------
 * MODULE NAME: L2_L4_PROC_COMM.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    The module provides APIs for all CSCs in L2_L4 process to get common
 *    resources of the process.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/05/17     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef L2_L4_PROC_COMM_H
#define L2_L4_PROC_COMM_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "l_threadgrp.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initialize the system resource which is common for all CSCs in
 *           L2_L4 process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T L2_L4_PROC_COMM_InitiateProcessResources(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetGvrpGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for GVRP group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetGvrpGroupTGHandle(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetL2McastGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for L2Mcast group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetL2McastGroupTGHandle(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetL2muxGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for L2mux group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetL2muxGroupTGHandle(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetL4GroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for SWCTRL group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetL4GroupTGHandle(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetLacpGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for LACP group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetLacpGroupTGHandle(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetNetaccessGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for NETACCESS group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetNetaccessGroupTGHandle(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetStaGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for STA group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetStaGroupTGHandle(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetSwctrlGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for SWCTRL group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetSwctrlGroupTGHandle(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetAmtrl3GroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for AMTRL3 group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetAmtrl3GroupTGHandle(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetUdphelperGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for UDPHELPER group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetUdphelperGroupTGHandle(void);

#if (SYS_CPNT_DAI == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetDaiGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for UDPHELPER group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetDaiGroupTGHandle(void);
#endif

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetOamGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for LACP group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetOamGroupTGHandle(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetDhcpsnpGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for DHCPSNP group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetDhcpsnpGroupTGHandle(void);

#if (SYS_CPNT_CFM == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetCfmGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for cfm group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetCfmGroupTGHandle(void);
#endif

#if (SYS_CPNT_PPPOE_IA == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetPppoeiaGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for PPPOE IA group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetPppoeiaGroupTGHandle(void);

#endif

#if (SYS_CPNT_OVSVTEP == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetOvsvtepGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for OVSVTEP group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetOvsvtepGroupTGHandle(void);

#endif /* #if (SYS_CPNT_OVSVTEP == TRUE) */

#if (SYS_CPNT_DCB_GROUP == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetDcbGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for DCB group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetDcbGroupTGHandle(void);
#endif

#if (SYS_CPNT_MLAG == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetMlagGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for MLAG group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetMlagGroupTGHandle(void);
#endif

#if (SYS_CPNT_VXLAN == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetVxlanGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for VXLAN group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetVxlanGroupTGHandle(void);
#endif

#endif /* #ifndef L2_L4_PROC_COMM_H */
