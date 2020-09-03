#ifndef _DEV_NMTRDRV_PMGR_H_
#define _DEV_NMTRDRV_PMGR_H_

#include "sys_type.h"
#include "sys_cpnt.h"
#include "dev_nmtrdrv.h"
#include "swdrv_type.h"

enum
{
    DEV_NMTRDRV_PMGR_RESP_RESULT_FAIL,
};

#define DEV_NMTRDRV_PMGR_REQ_CMD_SIZE       sizeof(((DEV_NMTRDRV_PMGR_IPCMSG_T *)0)->type)
#define DEV_NMTRDRV_PMGR_RESP_RESULT_SIZE   sizeof(((DEV_NMTRDRV_PMGR_IPCMSG_T *)0)->type)

typedef struct
{

    union
    {
        UI32_T cmd;          /*cmd fnction id*/
        BOOL_T result_bool;  /*respond bool return*/
        UI32_T result_ui32;  /*respond ui32 return*/
        UI32_T result_i32;  /*respond i32 return*/
        UI64_T result_ui64;
    }type;

    union
    {
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }clearportcounter;
        union
        {
            struct
            {
                UI32_T vid;
            }req;
            struct
            {
            }resp;
        }clearvlancounter;
        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }clearallcounters;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T start_port;
                UI32_T end_port;
            }req;
            struct
            {
                SWDRV_IfTableStats_T  stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
            }resp;
        }getiftablestats;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T start_port;
                UI32_T end_port;
            }req;
            struct
            {
                SWDRV_IfXTableStats_T  stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
            }resp;
        }getifxtablestats;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T start_port;
                UI32_T end_port;
            }req;
            struct
            {
                SWDRV_RmonStats_T  stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
            }resp;
        }getrmonstats;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T start_port;
                UI32_T end_port;
            }req;
            struct
            {
                SWDRV_EtherlikeStats_T  stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
            }resp;
        }getetherlikestats;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T start_port;
                UI32_T end_port;
            }req;
            struct
            {
                SWDRV_EtherlikePause_T  stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
            }resp;
        }getetherlikepause;
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T start_port;
                UI32_T end_port;
            }req;
            struct
            {
                SWDRV_IfPerQStats_T stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
            }resp;
        }getifperqstats;
#endif /* (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE) */
#if (SYS_CPNT_PFC == TRUE)
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T start_port;
                UI32_T end_port;
            }req;
            struct
            {
                SWDRV_PfcStats_T stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
            }resp;
        }getpfcstats;
#endif
#if (SYS_CPNT_CN == TRUE)
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T start_port;
                UI32_T end_port;
            }req;
            struct
            {
                SWDRV_QcnStats_T stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
            }resp;
        }getqcnstats;
#endif
    }data;

}DEV_NMTRDRV_PMGR_IPCMSG_T;

enum
{
    DEV_NMTRDRV_IPCCMD_CLEARPORTCOUNTER,
    DEV_NMTRDRV_IPCCMD_CLEARVLANCOUNTER,
    DEV_NMTRDRV_IPCCMD_CLEARALLCOUNTERS,
    DEV_NMTRDRV_IPCCMD_GETIFTABLESTATS,
    DEV_NMTRDRV_IPCCMD_GETIFXTABLESTATS,
    DEV_NMTRDRV_IPCCMD_GETRMONSTATS,
    DEV_NMTRDRV_IPCCMD_GETETHERLIKESTATS,
    DEV_NMTRDRV_IPCCMD_GETETHERLIKEPAUSE,
    DEV_NMTRDRV_IPCCMD_GETIFPERQSTATS,              /* SYS_CPNT_NMTR_PERQ_COUNTER */
    DEV_NMTRDRV_IPCCMD_GETPFCSTATS,                 /* SYS_CPNT_PFC */
    DEV_NMTRDRV_IPCCMD_GETQCNSTATS,                 /* SYS_CPNT_CN */
    DEV_NMTRDRV_IPCCMD_GETIFXTABLESTATSFORVLAN,
#if(SYS_CPNT_ATC_STORM == TRUE)
		DEV_NMTRDRV_IPCCMD_GETATCIFXTABLESTATS,
#endif		
};



/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_NMTRDRV_PMGR_ClearPortCounter
 *------------------------------------------------------------------------|
 * FUNCTION: This function will clear the port conuter
 * INPUT   : UI32_T unit    -   unit number
 *           UI32_T port    -   port number
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    : For clearing the port counter after system start up
 *------------------------------------------------------------------------*/
BOOL_T DEV_NMTRDRV_PMGR_ClearPortCounter(UI32_T unit, UI32_T port);


#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_NMTRDRV_PMGR_ClearVlanCounter
 *------------------------------------------------------------------------|
 * FUNCTION: This function will clear the vlan counter
 * INPUT   : UI32_T vid     -   VID; 0 to clear all vlan
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    : For clearing all the counters after system start up
 *------------------------------------------------------------------------*/
BOOL_T DEV_NMTRDRV_PMGR_ClearVlanCounter(UI32_T vid);
#endif /* (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE) */


/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_NMTRDRV_PMGR_ClearAllCounters
 *------------------------------------------------------------------------|
 * FUNCTION: This function will clear the all counters in whole system
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    : For clearing all the counters after system start up
 *------------------------------------------------------------------------*/
BOOL_T DEV_NMTRDRV_PMGR_ClearAllCounters(void);


/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_NMTRDRV_PMGR_GetIfTableStats
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the IfTable statistics
 * INPUT   : UI32_T unit            -   unit number
 *           UI32_T start_port      -   start port number       <01302002>
 *           UI32_T end_port        -   end port number         <01302002>
 *           *stats                 -   port statistics buffer
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    :
 *
 * <01302002> 1. The caller must provide enough buffer for callee to get the counters.
 *
 *------------------------------------------------------------------------*/
BOOL_T DEV_NMTRDRV_PMGR_GetIfTableStats(UI32_T unit,
                                   UI32_T start_port,               /* <01302002> */
                                   UI32_T end_port,                 /* <01302002> */
                                   SWDRV_IfTableStats_T  *stats); /* <01302002> */


/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_NMTRDRV_PMGR_GetIfXTableStats
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the IfXTable statistics
 * INPUT   : UI32_T unit            -   unit number
 *           UI32_T start_port      -   start port number       <01302002>
 *           UI32_T end_port        -   end port number         <01302002>
 *           *stats                 -   port statistics buffer
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    :
 *
 * <01302002> 1. The caller must provide enough buffer for callee to get the counters.
 *
 *------------------------------------------------------------------------*/
BOOL_T DEV_NMTRDRV_PMGR_GetIfXTableStats(UI32_T unit,
                                    UI32_T start_port,               /* <01302002> */
                                    UI32_T end_port,                 /* <01302002> */
                                    SWDRV_IfXTableStats_T  *stats);


/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_NMTRDRV_PMGR_GetRmonStats
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the RMON statistics
 * INPUT   : UI32_T unit            -   unit number
 *           UI32_T start_port      -   start port number       <01302002>
 *           UI32_T end_port        -   end port number         <01302002>
 *           *stats                 -   port statistics buffer
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    :
 *
 * <01302002> 1. The caller must provide enough buffer for callee to get the counters.
 *
 *------------------------------------------------------------------------*/
BOOL_T DEV_NMTRDRV_PMGR_GetRmonStats(UI32_T unit,
                                UI32_T start_port,               /* <01302002> */
                                UI32_T end_port,                 /* <01302002> */
                                SWDRV_RmonStats_T  *stats);


/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_NMTRDRV_PMGR_GetEtherLikeStats
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the EtherLike statistics
 * INPUT   : UI32_T unit                        -   unit number
 *           UI32_T start_port      -   start port number       <01302002>
 *           UI32_T end_port        -   end port number         <01302002>
 *           *stats                 -   port statistics buffer
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    :
 *
 * <01302002> 1. The caller must provide enough buffer for callee to get the counters.
 *
 *------------------------------------------------------------------------*/
BOOL_T DEV_NMTRDRV_PMGR_GetEtherLikeStats(UI32_T unit,
                                     UI32_T start_port,               /* <01302002> */
                                     UI32_T end_port,                 /* <01302002> */
                                     SWDRV_EtherlikeStats_T  *stats);

/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_NMTRDRV_PMGR_GetEtherLikePause
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the EtherLike pause statistics
 * INPUT   : UI32_T unit                        -   unit number
 *           UI32_T start_port      -   start port number       <01302002>
 *           UI32_T end_port        -   end port number         <01302002>
 *           *stats                 -   port statistics buffer
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    :
 *
 * <01302002> 1. The caller must provide enough buffer for callee to get the counters.
 *
 *------------------------------------------------------------------------*/
BOOL_T DEV_NMTRDRV_PMGR_GetEtherLikePause(UI32_T unit,
                                     UI32_T start_port,               /* <01302002> */
                                     UI32_T end_port,                 /* <01302002> */
                                     SWDRV_EtherlikePause_T  *stats);

#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_NMTRDRV_PMGR_GetIfPerQStats
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the statistics for each CoS queue
 * INPUT   : UI32_T unit                        -   unit number
 *           UI32_T start_port      -   start port number       <01302002>
 *           UI32_T end_port        -   end port number         <01302002>
 *           *stats                 -   port statistics buffer
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    :
 *
 * <01302002> 1. The caller must provide enough buffer for callee to get the counters.
 *
 *------------------------------------------------------------------------*/
BOOL_T DEV_NMTRDRV_PMGR_GetIfPerQStats(UI32_T unit,
                                     UI32_T start_port,               /* <01302002> */
                                     UI32_T end_port,                 /* <01302002> */
                                     SWDRV_IfPerQStats_T  *stats);
#endif /* (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE) */

#if (SYS_CPNT_PFC == TRUE)
/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_NMTRDRV_PMGR_GetPfcStats
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the PFC statistics
 * INPUT   : UI32_T unit                        -   unit number
 *           UI32_T start_port      -   start port number       <01302002>
 *           UI32_T end_port        -   end port number         <01302002>
 *           *stats                 -   port statistics buffer
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    :
 *
 * <01302002> 1. The caller must provide enough buffer for callee to get the counters.
 *
 *------------------------------------------------------------------------*/
BOOL_T DEV_NMTRDRV_PMGR_GetPfcStats(
    UI32_T unit,
    UI32_T start_port,
    UI32_T end_port,
    SWDRV_PfcStats_T *stats);
#endif /* (SYS_CPNT_PFC == TRUE) */

#if (SYS_CPNT_CN == TRUE)
/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_NMTRDRV_PMGR_GetQcnStats
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the QCN statistics
 * INPUT   : UI32_T unit                        -   unit number
 *           UI32_T start_port      -   start port number       <01302002>
 *           UI32_T end_port        -   end port number         <01302002>
 *           *stats                 -   port statistics buffer
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    :
 *
 * <01302002> 1. The caller must provide enough buffer for callee to get the counters.
 *
 *------------------------------------------------------------------------*/
BOOL_T DEV_NMTRDRV_PMGR_GetQcnStats(UI32_T unit,
                                     UI32_T start_port,               /* <01302002> */
                                     UI32_T end_port,                 /* <01302002> */
                                     SWDRV_QcnStats_T  *stats);
#endif /* (SYS_CPNT_CN == TRUE) */

#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_NMTRDRV_PMGR_GetIfXTableStatsForVlan
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the IfXTable statistics
 * INPUT   : UI32_T unit            -   unit number
 *           UI32_T start_vid       -   start vid
 *           UI32_T end_vid         -   end vid
 *           *stats                 -   statistics buffer
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T DEV_NMTRDRV_PMGR_GetIfXTableStatsForVlan(UI32_T unit,
                                   UI32_T start_vid,
                                   UI32_T end_vid,
                                   SWDRV_IfXTableStats_T  *stats);
#endif /* (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE) */

/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_NMTRDRV_PMGR_InitiateProcessResource
 *------------------------------------------------------------------------|
 * FUNCTION: Initialize for getting the ipc message queue
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *------------------------------------------------------------------------*/
void DEV_NMTRDRV_PMGR_InitiateProcessResource(void) ;
#endif

