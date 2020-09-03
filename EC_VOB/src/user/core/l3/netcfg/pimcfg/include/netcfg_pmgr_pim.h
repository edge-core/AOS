/*-----------------------------------------------------------------------------
* FILE NAME: NETCFG_PMGR_PIM.H
*-----------------------------------------------------------------------------
* PURPOSE:
*    This file provides APIs for other process or CSC group to access NETCFG_MGR_PIM and NETCFG_OM_PIM service.
*    In Linux platform, the communication between CSC group are done via IPC.
*    Other CSC can call NETCFG_PMGR_XXX for APIs NETCFG_MGR_XXX provided by NETCFG
*
* NOTES:
*    None.
*
* HISTORY:
*    2008/05/18     --- Lin.Li, Create
*
* Copyright(C);      Accton Corporation, 2008
*-----------------------------------------------------------------------------
*/
#ifndef NETCFG_PMGR_PIM_H
#define NETCFG_PMGR_PIM_H

BOOL_T NETCFG_PMGR_PIM_InitiateProcessResource(void);
UI32_T NETCFG_PMGR_EnablePimDenseModeOnIf(UI32_T ifindex);
UI32_T NETCFG_PMGR_DisablePimDenseModeOnIf(UI32_T ifindex);
UI32_T NETCFG_PMGR_SetPimIfHelloIntv(UI32_T ifindex, UI32_T interval);
UI32_T NETCFG_PMGR_SetPimIfHelloHoldtime(UI32_T ifindex, UI32_T interval);
UI32_T NETCFG_PMGR_SetPimIfNeighborFilter(UI32_T ifindex, UI8_T *filter);
UI32_T NETCFG_PMGR_UnSetPimIfNeighborFilter(UI32_T ifindex);
UI32_T NETCFG_PMGR_PimExcludeGenid(UI32_T ifindex, BOOL_T exclude_b);
UI32_T NETCFG_PMGR_SetPimIfDrPriority(UI32_T ifindex, UI32_T priority);
UI32_T NETCFG_PMGR_UnSetPimIfDrPriority(UI32_T ifindex);
UI32_T NETCFG_PMGR_EnablePimSparseModeOnIf(UI32_T ifindex);
UI32_T NETCFG_PMGR_DisablePimSparseModeOnIf(UI32_T ifindex);
UI32_T NETCFG_PMGR_EnablePimStateRefreshOnIf(UI32_T ifindex);
UI32_T NETCFG_PMGR_DisablePimStateRefreshOnIf(UI32_T ifindex);
UI32_T NETCFG_PMGR_SetPimIfStateRefreshOriginalInterval(UI32_T ifindex, UI32_T interval);
UI32_T NETCFG_PMGR_UnSetPimIfStateRefreshOriginalInterval(UI32_T ifindex);
UI32_T NETCFG_PMGR_SetPimAcceptReigsterList(UI8_T *filter);
UI32_T NETCFG_PMGR_UnSetPimAcceptReigsterList(void);
UI32_T NETCFG_PMGR_EnablePimCrpPrefix(void);
UI32_T NETCFG_PMGR_DisablePimCrpPrefix(void);
UI32_T NETCFG_PMGR_PimIgnoreRpSetPriority(void);
UI32_T NETCFG_PMGR_PimNoIgnoreRpSetPriority(void);
UI32_T NETCFG_PMGR_SetPimJoinPruneInterval(UI32_T interval);
UI32_T NETCFG_PMGR_UnSetPimJoinPruneInterval(void);
UI32_T NETCFG_PMGR_SetPimRegisterRateLimit(UI32_T limit);
UI32_T NETCFG_PMGR_UnSetPimRegisterRateLimit(void);
UI32_T NETCFG_PMGR_SetPimRegisterSuppressionTime(UI32_T time);
UI32_T NETCFG_PMGR_UnSetPimRegisterSuppressionTime(void);
UI32_T NETCFG_PMGR_SetPimRpAddress(UI32_T addr);
UI32_T NETCFG_PMGR_UnSetPimRpAddress(UI32_T addr);
UI32_T NETCFG_PMGR_SetPimRegisterKAT(UI32_T time);
UI32_T NETCFG_PMGR_UnSetPimRegisterKAT(void);
UI32_T NETCFG_PMGR_SetPimRegisterChecksumGroupList(UI8_T *list);
UI32_T NETCFG_PMGR_DisablePimRegisterChecksum(void);
UI32_T NETCFG_PMGR_EnablePimRegisterChecksum(void);
UI32_T NETCFG_PMGR_SetPimRpCandidate(UI8_T *ifName);
UI32_T NETCFG_PMGR_SetPimRpCandidateGroupAddr(UI8_T *ifName, UI32_T groupAddr, UI32_T maskAddr);
UI32_T NETCFG_PMGR_UnSetPimRpCandidate(UI8_T *ifName);
UI32_T NETCFG_PMGR_UnSetPimRpCandidateGroupAddr(UI8_T *ifName, UI32_T groupAddr, UI32_T maskAddr);
UI32_T NETCFG_PMGR_EnableCandidateBsr(UI8_T *ifName);
UI32_T NETCFG_PMGR_DisableCandidateBsr(void);
UI32_T NETCFG_PMGR_SetCandidateBsrHash(UI8_T *ifName, UI32_T hash);
UI32_T NETCFG_PMGR_SetCandidateBsrPriority(UI8_T *ifName, UI32_T priority);
UI32_T NETCFG_PMGR_SetPimSptThresholdInfinity(void);
UI32_T NETCFG_PMGR_UnSetPimSptThresholdInfinity(void);
UI32_T NETCFG_PMGR_SetPimSptThresholdInfinityGroupList(UI8_T *list);

#endif /* NETCFG_PMGR_PIM_H */

