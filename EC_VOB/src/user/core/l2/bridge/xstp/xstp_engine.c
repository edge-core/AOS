/*-----------------------------------------------------------------------------
 * Module Name: xstp_engine.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Definitions for handling the state machine processes for XSTP.
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    05/30/2001 - Allen Cheng, Created
 *    06/12/2002 - Kelly Chen, Added
 *    02-09-2004 - Kelly Chen, Revise the implementations of 802.1w/1s according to the IEEE 802.1D/D3
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-----------------------------------------------------------------------------
 */


#include <stdio.h>
#include <string.h>
#include "sysfun.h"
#include "sys_type.h"
#include "backdoor_mgr.h"
#include "swctrl.h"
#include "xstp_type.h"
#include "xstp_om_private.h"
#include "xstp_engine.h"
#include "xstp_uty.h"
#include "sys_callback_mgr.h"  /*because the head file of SYS_CALLBACK_MGR_LportTcChangeCallback function does not included in the file */
#include "sys_module.h"
#ifdef  XSTP_TYPE_PROTOCOL_RSTP
/* According to the definition in 17.16.2 and 17.16.3 , the FwdDelay and HelloTime
 * are the Bridge Forward Delay and Hello Time components of Bridge Times.
 * In the implementation we use the Forward Delay and Hello Time components of Port Times
 * in order to have the consistent behavior with 802.1d (STP).
 * In 802.1d (STP), the Forward Delay and Hello Time are dominated by the Bridge Hello
 * Time of the Root Bridge via received BPDU from the root.
 */
#define HelloTime       pom_ptr->port_times.hello_time
#define FwdDelay        pom_ptr->port_times.forward_delay
#endif /* XSTP_TYPE_PROTOCOL_RSTP */

#define XSTP_ENGINE_SM_DEBUG_END    0xFF
typedef struct
{
    UI8_T   status;
    char    *debug_string;
} XSTP_ENGINE_DebugStruct_T;




/* ===================================================================== */
/* ===================================================================== */
/* IEEE 802.1W */
#ifdef XSTP_TYPE_PROTOCOL_RSTP

/* Debugging the State Machine */
static  XSTP_ENGINE_DebugStruct_T   XSTP_ENGINE_DebugPIM[] =
{
    {   XSTP_ENGINE_SM_PIM_STATE_DISABLED,                      "PIM-DISABLED"                      },
    {   XSTP_ENGINE_SM_PIM_STATE_AGED,                          "PIM-AGED"                          },
    {   XSTP_ENGINE_SM_PIM_STATE_UPDATE,                        "PIM-UPDATE"                        },
    {   XSTP_ENGINE_SM_PIM_STATE_CURRENT,                       "PIM-CURRENT"                       },
    {   XSTP_ENGINE_SM_PIM_STATE_SUPERIOR,                      "PIM-SUPERIOR"                      },
    {   XSTP_ENGINE_SM_PIM_STATE_REPEAT,                        "PIM-REPEAT"                        },
    {   XSTP_ENGINE_SM_PIM_STATE_AGREEMENT,                     "PIM-AGREEMENT"                     },
    {   XSTP_ENGINE_SM_PIM_STATE_RECEIVE,                       "PIM-RECEIVE"                       },
    {   XSTP_ENGINE_SM_DEBUG_END,                               ""                                  }
};

static  XSTP_ENGINE_DebugStruct_T   XSTP_ENGINE_DebugPRS[] =
{
    {   XSTP_ENGINE_SM_PRS_STATE_INIT_BRIDGE,                   "PRS-INIT_BRIDGE"                   },
    {   XSTP_ENGINE_SM_PRS_STATE_ROLE_SELECTION,                "PRS-ROLE_SELECTION"                },
    {   XSTP_ENGINE_SM_DEBUG_END,                               ""                                  }
};

static  XSTP_ENGINE_DebugStruct_T   XSTP_ENGINE_DebugPRT[] =
{
    {   XSTP_ENGINE_SM_PRT_STATE_INIT_PORT,                     "PRT-INIT_PORT"                     },
    {   XSTP_ENGINE_SM_PRT_STATE_BLOCK_PORT,                    "PRT-BLOCK_PORT"                    },
    {   XSTP_ENGINE_SM_PRT_STATE_BACKUP_PORT,                   "PRT-BACKUP_PORT"                   },
    {   XSTP_ENGINE_SM_PRT_STATE_BLOCKED_PORT,                  "PRT-BLOCKED_PORT"                  },
    {   XSTP_ENGINE_SM_PRT_STATE_ROOT_PROPOSED,                 "PRT-ROOT_PROPOSED"                 },
    {   XSTP_ENGINE_SM_PRT_STATE_ROOT_AGREED,                   "PRT-ROOT_AGREED"                   },
    {   XSTP_ENGINE_SM_PRT_STATE_ROOT_FORWARD,                  "PRT-ROOT_FORWARD"                  },
    {   XSTP_ENGINE_SM_PRT_STATE_ROOT_LEARN,                    "PRT-ROOT_LEARN"                    },
    {   XSTP_ENGINE_SM_PRT_STATE_ROOT_PORT,                     "PRT-ROOT_PORT"                     },
    {   XSTP_ENGINE_SM_PRT_STATE_REROOT,                        "PRT-REROOT"                        },
    {   XSTP_ENGINE_SM_PRT_STATE_REROOTED,                      "PRT-REROOTED"                      },
    {   XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_PROPOSE,            "PRT-DESIGNATED_PROPOSE"            },
    {   XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_SYNCED,             "PRT-DESIGNATED_SYNCED"             },
    {   XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_RETIRED,            "PRT-DESIGNATED_RETIRED"            },
    {   XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_FORWARD,            "PRT-DESIGNATED_FORWARD"            },
    {   XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_LEARN,              "PRT-DESIGNATED_LEARN"              },
    {   XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_LISTEN,             "PRT-DESIGNATED_LISTEN"             },
    {   XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_PORT,               "PRT-DESIGNATED_PORT"               },
    {   XSTP_ENGINE_SM_DEBUG_END,                               ""                                  }
};

static  XSTP_ENGINE_DebugStruct_T   XSTP_ENGINE_DebugPST[] =
{
    {   XSTP_ENGINE_SM_PST_STATE_DISCARDING,                    "PST-DISCARDING"                    },
    {   XSTP_ENGINE_SM_PST_STATE_LEARNING,                      "PST-LEARNING"                      },
    {   XSTP_ENGINE_SM_PST_STATE_FORWARDING,                    "PST-FORWARDING"                    },
    {   XSTP_ENGINE_SM_DEBUG_END,                               ""                                  }
};

static  XSTP_ENGINE_DebugStruct_T   XSTP_ENGINE_DebugTCM[] =
{
    {   XSTP_ENGINE_SM_TCM_STATE_INIT,                          "TCM-INIT"                          },
    {   XSTP_ENGINE_SM_TCM_STATE_INACTIVE,                      "TCM-INACTIVE"                      },
    {   XSTP_ENGINE_SM_TCM_STATE_ACTIVE,                        "TCM-ACTIVE"                        },
    {   XSTP_ENGINE_SM_TCM_STATE_DETECTED,                      "TCM-DETECTED"                      },
    {   XSTP_ENGINE_SM_TCM_STATE_NOTIFIED_TCN,                  "TCM-NOTIFIED_TCN"                  },
    {   XSTP_ENGINE_SM_TCM_STATE_NOTIFIED_TC,                   "TCM-NOTIFIED_TC"                   },
    {   XSTP_ENGINE_SM_TCM_STATE_PROPAGATING,                   "TCM-PROPAGATING"                   },
    {   XSTP_ENGINE_SM_TCM_STATE_ACKNOWLEDGED,                  "TCM-ACKNOWLEDGED"                  },
    {   XSTP_ENGINE_SM_DEBUG_END,                               ""                                  }
};

static  XSTP_ENGINE_DebugStruct_T   XSTP_ENGINE_DebugPPM[] =
{
    {   XSTP_ENGINE_SM_PPM_STATE_INIT,                          "PPM-INIT"                          },
    {   XSTP_ENGINE_SM_PPM_STATE_SEND_RSTP,                     "PPM-SEND_RSTP"                     },
    {   XSTP_ENGINE_SM_PPM_STATE_SENDING_RSTP,                  "PPM-SENDING_RSTP"                  },
    {   XSTP_ENGINE_SM_PPM_STATE_SEND_STP,                      "PPM-SEND_STP"                      },
    {   XSTP_ENGINE_SM_PPM_STATE_SENDING_STP,                   "PPM-SENDING_STP"                   },
    {   XSTP_ENGINE_SM_DEBUG_END,                               ""                                  }
};

static  XSTP_ENGINE_DebugStruct_T   XSTP_ENGINE_DebugPTX[] =
{
    {   XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_INIT,                 "PTX-TRANSMIT_INIT"                 },
    {   XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_PERIODIC,             "PTX-TRANSMIT_PERIODIC"             },
    {   XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_CONFIG,               "PTX-TRANSMIT_CONFIG"               },
    {   XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_TCN,                  "PTX-TRANSMIT_TCN"                  },
    {   XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_RSTP,                 "PTX-TRANSMIT_RSTP"                 },
    {   XSTP_ENGINE_SM_PTX_STATE_IDLE,                          "PTX-IDLE"                          },
    {   XSTP_ENGINE_SM_DEBUG_END,                               ""                                  }
};

static  void    XSTP_ENGINE_InitBridgeStateMachine(XSTP_OM_InstanceData_T *om_ptr, UI32_T system_id_extension);
/*
static  void    XSTP_ENGINE_InitPortStateMachines(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
*/
static  BOOL_T  XSTP_ENGINE_StateMachinePTI(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static  BOOL_T  XSTP_ENGINE_StateMachinePIM(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static  BOOL_T  XSTP_ENGINE_StateMachinePRS(XSTP_OM_InstanceData_T *om_ptr);
static  BOOL_T  XSTP_ENGINE_StateMachinePRT(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static  BOOL_T  XSTP_ENGINE_StateMachinePST(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static  BOOL_T  XSTP_ENGINE_StateMachineTCM(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static  BOOL_T  XSTP_ENGINE_StateMachinePPM(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static  BOOL_T  XSTP_ENGINE_StateMachinePTX(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
#endif /* XSTP_TYPE_PROTOCOL_RSTP */
/* ===================================================================== */
/* ===================================================================== */
/* IEEE 802.1S */
#ifdef XSTP_TYPE_PROTOCOL_MSTP

/* Debugging the State Machine */
static  XSTP_ENGINE_DebugStruct_T   XSTP_ENGINE_DebugPRX[] =
{
    {   XSTP_ENGINE_SM_PRX_STATE_DISCARD,                       "PRX-DISCARD"                       },
    {   XSTP_ENGINE_SM_PRX_STATE_RECEIVE,                       "PRX-RECEIVE"                       },
    {   XSTP_ENGINE_SM_DEBUG_END,                               ""                                  }
};

static  XSTP_ENGINE_DebugStruct_T   XSTP_ENGINE_DebugPPM[] =
{
    {   XSTP_ENGINE_SM_PPM_STATE_INIT,                          "PPM-INIT"                          },
    {   XSTP_ENGINE_SM_PPM_STATE_SEND_RSTP,                     "PPM-SEND_RSTP"                     },
    {   XSTP_ENGINE_SM_PPM_STATE_SENDING_RSTP,                  "PPM-SENDING_RSTP"                  },
    {   XSTP_ENGINE_SM_PPM_STATE_SEND_STP,                      "PPM-SEND_STP"                      },
    {   XSTP_ENGINE_SM_PPM_STATE_SENDING_STP,                   "PPM-SENDING_STP"                   },
    {   XSTP_ENGINE_SM_DEBUG_END,                               ""                                  }
};

static  XSTP_ENGINE_DebugStruct_T   XSTP_ENGINE_DebugPTX[] =
{
    {   XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_INIT,                 "PTX-TRANSMIT_INIT"                 },
    {   XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_PERIODIC,             "PTX-TRANSMIT_PERIODIC"             },
    {   XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_CONFIG,               "PTX-TRANSMIT_CONFIG"               },
    {   XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_TCN,                  "PTX-TRANSMIT_TCN"                  },
    {   XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_RSTP,                 "PTX-TRANSMIT_RSTP"                 },
    {   XSTP_ENGINE_SM_PTX_STATE_IDLE,                          "PTX-IDLE"                          },
    {   XSTP_ENGINE_SM_DEBUG_END,                               ""                                  }
};

static  XSTP_ENGINE_DebugStruct_T   XSTP_ENGINE_DebugPIM[] =
{
    {   XSTP_ENGINE_SM_PIM_STATE_DISABLED,                      "PIM-DISABLED"                      },
    {   XSTP_ENGINE_SM_PIM_STATE_ENABLED,                       "PIM-ENABLED"                       },
    {   XSTP_ENGINE_SM_PIM_STATE_AGED,                          "PIM-AGED"                          },
    {   XSTP_ENGINE_SM_PIM_STATE_UPDATE,                        "PIM-UPDATE"                        },
    {   XSTP_ENGINE_SM_PIM_STATE_CURRENT,                       "PIM-CURRENT"                       },
    {   XSTP_ENGINE_SM_PIM_STATE_SUPERIOR_DESIGNATED,           "PIM-SUPERIOR_DESIGNATED"           },
    {   XSTP_ENGINE_SM_PIM_STATE_REPEATED_DESIGNATED,           "PIM-REPEATED_DESIGNATED"           },
    {   XSTP_ENGINE_SM_PIM_STATE_ROOT,                          "PIM-ROOT"                          },
    {   XSTP_ENGINE_SM_PIM_STATE_OTHER,                          "PIM-OTHER"                        },
    {   XSTP_ENGINE_SM_PIM_STATE_RECEIVE,                       "PIM-RECEIVE"                       },
    {   XSTP_ENGINE_SM_DEBUG_END,                               ""                                  }
};

static  XSTP_ENGINE_DebugStruct_T   XSTP_ENGINE_DebugPRS[] =
{
    {   XSTP_ENGINE_SM_PRS_STATE_INIT_BRIDGE,                   "PRS-INIT_BRIDGE"                   },
    {   XSTP_ENGINE_SM_PRS_STATE_RECEIVE,                       "PRS-RECEIVE"                       },
    {   XSTP_ENGINE_SM_DEBUG_END,                               ""                                  }
};

static  XSTP_ENGINE_DebugStruct_T   XSTP_ENGINE_DebugPRT[] =
{
    {   XSTP_ENGINE_SM_PRT_STATE_INIT_PORT,                     "PRT-INIT_PORT"                     },
    {   XSTP_ENGINE_SM_PRT_STATE_BLOCK_PORT,                    "PRT-BLOCK_PORT"                    },
    {   XSTP_ENGINE_SM_PRT_STATE_BACKUP_PORT,                   "PRT-BACKUP_PORT"                   },
    {   XSTP_ENGINE_SM_PRT_STATE_ALTERNATE_PROPOSED,            "PRT-ALTERNATE_PROPOSED"            },
    {   XSTP_ENGINE_SM_PRT_STATE_ALTERNATE_AGREED,              "PRT-ALTERNATE_AGREED"              },
    {   XSTP_ENGINE_SM_PRT_STATE_BLOCKED_PORT,                  "PRT-BLOCKED_PORT"                  },
    {   XSTP_ENGINE_SM_PRT_STATE_PROPOSED,                      "PRT-PROPOSED"                      },
    {   XSTP_ENGINE_SM_PRT_STATE_PROPOSING,                     "PRT-PROPOSING"                     },
    {   XSTP_ENGINE_SM_PRT_STATE_AGREES,                        "PRT-AGREES"                        },
    {   XSTP_ENGINE_SM_PRT_STATE_SYNCED,                        "PRT-SYNCED"                        },
    {   XSTP_ENGINE_SM_PRT_STATE_REROOT,                        "PRT-REROOT"                        },
    {   XSTP_ENGINE_SM_PRT_STATE_FORWARD,                       "PRT-FORWARD"                       },
    {   XSTP_ENGINE_SM_PRT_STATE_LEARN,                         "PRT-LEARN"                         },
    {   XSTP_ENGINE_SM_PRT_STATE_LISTEN,                        "PRT-LISTEN"                        },
    {   XSTP_ENGINE_SM_PRT_STATE_REROOTED,                      "PRT-REROOTED"                      },
    {   XSTP_ENGINE_SM_PRT_STATE_ROOT,                          "PRT-ROOT"                          },
    {   XSTP_ENGINE_SM_PRT_STATE_ACTIVE_PORT,                   "PRT-ACTIVE_PORT"                   },
    {   XSTP_ENGINE_SM_DEBUG_END,                               ""                                  }
};

static  XSTP_ENGINE_DebugStruct_T   XSTP_ENGINE_DebugPST[] =
{
    {   XSTP_ENGINE_SM_PST_STATE_DISCARDING,                    "PST-DISCARDING"                    },
    {   XSTP_ENGINE_SM_PST_STATE_LEARNING,                      "PST-LEARNING"                      },
    {   XSTP_ENGINE_SM_PST_STATE_FORWARDING,                    "PST-FORWARDING"                    },
    {   XSTP_ENGINE_SM_DEBUG_END,                               ""                                  }
};

static  XSTP_ENGINE_DebugStruct_T   XSTP_ENGINE_DebugTCM[] =
{
    {   XSTP_ENGINE_SM_TCM_STATE_INIT,                          "TCM-INIT"                          },
    {   XSTP_ENGINE_SM_TCM_STATE_INACTIVE,                      "TCM-INACTIVE"                      },
    {   XSTP_ENGINE_SM_TCM_STATE_ACTIVE,                        "TCM-ACTIVE"                        },
    {   XSTP_ENGINE_SM_TCM_STATE_DETECTED,                      "TCM-DETECTED"                      },
    {   XSTP_ENGINE_SM_TCM_STATE_NOTIFIED_TCN,                  "TCM-NOTIFIED_TCN"                  },
    {   XSTP_ENGINE_SM_TCM_STATE_NOTIFIED_TC,                   "TCM-NOTIFIED_TC"                   },
    {   XSTP_ENGINE_SM_TCM_STATE_PROPAGATING,                   "TCM-PROPAGATING"                   },
    {   XSTP_ENGINE_SM_TCM_STATE_ACKNOWLEDGED,                  "TCM-ACKNOWLEDGED"                  },
    {   XSTP_ENGINE_SM_DEBUG_END,                               ""                                  }
};

static  void    XSTP_ENGINE_InitBridgeStateMachine(XSTP_OM_InstanceData_T *om_ptr, UI32_T system_id_extension);
/*
static  void    XSTP_ENGINE_InitPortStateMachines(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
*/
static  BOOL_T  XSTP_ENGINE_StateMachinePTI(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static  BOOL_T  XSTP_ENGINE_StateMachinePRX(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static  BOOL_T  XSTP_ENGINE_StateMachinePIM(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static  BOOL_T  XSTP_ENGINE_StateMachinePRS(XSTP_OM_InstanceData_T *om_ptr);
static  BOOL_T  XSTP_ENGINE_StateMachinePRT(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static  BOOL_T  XSTP_ENGINE_StateMachinePST(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static  BOOL_T  XSTP_ENGINE_StateMachineTCM(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static  BOOL_T  XSTP_ENGINE_StateMachinePPM(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static  BOOL_T  XSTP_ENGINE_StateMachinePTX(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
#endif /* XSTP_TYPE_PROTOCOL_MSTP */

static  XSTP_ENGINE_DebugStruct_T   XSTP_ENGINE_DebugBDM[] =
{
    {   XSTP_ENGINE_SM_BDM_STATE_EDGE,                          "BDM-EDGE"                          },
    {   XSTP_ENGINE_SM_BDM_STATE_NOT_EDGE,                      "BDM-NOT_EDGE"                      },
    {   XSTP_ENGINE_SM_DEBUG_END,                               ""                                  }
};

static  BOOL_T  XSTP_ENGINE_StateMachineBDM(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/* ===================================================================== */
/* ===================================================================== */

static  void    XSTP_ENGINE_DebugStateMachine(UI32_T mstid, UI32_T lport, XSTP_ENGINE_DebugStruct_T *dbg_table, UI8_T state);





/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_InitAllInstances
 *-------------------------------------------------------------------------
 * PURPOSE  : Initialize all the instances
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 */
void XSTP_ENGINE_InitAllInstances(void)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    UI32_T                  xstid;

    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
    {
        BACKDOOR_MGR_Printf("\r\nXSTP_ENGINE_InitAllInstances::Processing...");
    }

    xstid               = XSTP_TYPE_CISTID;
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    do
    {
        if (om_ptr->instance_exist)
        {
            XSTP_ENGINE_InitStateMachine(om_ptr, xstid);
        }
    }while(XSTP_OM_GetNextInstanceInfoPtr(&xstid, &om_ptr));

    return;
} /* End of XSTP_ENGINE_InitAllInstances */




/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_InitStateMachine
 *-------------------------------------------------------------------------
 * PURPOSE  : Initialize the state machines
 * INPUT    : om_ptr    -- om pointer for this instance
 *            xstid     -- Spanning tree identifier
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 */
void XSTP_ENGINE_InitStateMachine(XSTP_OM_InstanceData_T *om_ptr, UI32_T xstid)
{
    UI16_T                      index;

    /* Init Bridge State Machine */
    XSTP_ENGINE_InitBridgeStateMachine(om_ptr, xstid);

    /* Init Port State Machine */
    for (index = 0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++)
    {
         /* if ( XSTP_OM_IsMemberPortOfInstance(om_ptr, index+1) ) */
            XSTP_ENGINE_InitPortStateMachines(om_ptr, index+1);
    }

    return;
} /* XSTP_ENGINE_InitStateMachine */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_PTIStateMachineProgress
 *-------------------------------------------------------------------------
 * PURPOSE  : Motivate the port timer state machine
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETUEN   : TRUE if any of the timers is expired, else FALSE
 * NOTES    : None
 */
BOOL_T  XSTP_ENGINE_PTIStateMachineProgress(XSTP_OM_InstanceData_T *om_ptr)
{
    UI32_T  lport;
    BOOL_T  result;
    BOOL_T  timeout;
    XSTP_OM_PortVar_T   *pom_ptr;

    timeout = FALSE;

    if (om_ptr->bridge_info.common->begin)
        return timeout;

    if (om_ptr->instance_exist)
    {
        /* Performance Improvement
        for (lport = 1; lport <= XSTP_TYPE_MAX_NUM_OF_LPORT; lport++)
        */
        lport = 0;
        while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
        {
            pom_ptr = &(om_ptr->port_info[lport-1]);
            if (    (pom_ptr->is_member)
                &&  (pom_ptr->common->port_spanning_tree_status == VAL_staPortSystemStatus_enabled)
               )
            {
                result  = XSTP_ENGINE_StateMachinePTI(om_ptr, lport);
                timeout = timeout || result;
            }
        }
    }

    return timeout;
} /* End of XSTP_ENGINE_PTIStateMachineProgress */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_StateMachineProgress
 *-------------------------------------------------------------------------
 * PURPOSE  : Motivate the state machines
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 */
void    XSTP_ENGINE_StateMachineProgress(XSTP_OM_InstanceData_T *om_ptr)
{
    UI32_T  lport;
    UI16_T   sm_progress_bmp;
    XSTP_OM_PortVar_T   *pom_ptr;

    if (om_ptr->bridge_info.common->begin)
        return;

    if (om_ptr->instance_exist)
    {
        do
        {
            sm_progress_bmp = 0;
            if ( XSTP_ENGINE_StateMachinePRS(om_ptr) )
                sm_progress_bmp |= 1 << 0;
            /* Performance Improvement
            for (lport = 1; lport <= XSTP_TYPE_MAX_NUM_OF_LPORT; lport++)
            */
            lport = 0;
            while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
            {
                pom_ptr = &(om_ptr->port_info[lport-1]);
                if (pom_ptr->common->port_spanning_tree_status == VAL_staPortSystemStatus_enabled)
                {
                    if (XSTP_UTY_Cist(om_ptr) )
                    {
                        #ifdef XSTP_TYPE_PROTOCOL_MSTP
                        if ( XSTP_ENGINE_StateMachinePRX(om_ptr, lport) )
                            sm_progress_bmp |= 1 << 7;
                        #endif /* XSTP_TYPE_PROTOCOL_MSTP */
                        if ( XSTP_ENGINE_StateMachinePPM(om_ptr, lport) )
                            sm_progress_bmp |= 1 << 6;
                        if ( XSTP_ENGINE_StateMachinePTX(om_ptr, lport) )
                            sm_progress_bmp |= 1 << 5;
                        if (XSTP_ENGINE_StateMachineBDM(om_ptr, lport)  )
                            sm_progress_bmp |= 1 << 8;
                    }
                    if (om_ptr->port_info[lport-1].is_member)
                    {
                        if ( XSTP_ENGINE_StateMachinePIM(om_ptr, lport) )
                            sm_progress_bmp |= 1 << 4;
                        if ( XSTP_ENGINE_StateMachinePRT(om_ptr, lport) )
                            sm_progress_bmp |= 1 << 3;
                        if ( XSTP_ENGINE_StateMachinePST(om_ptr, lport) )
                            sm_progress_bmp |= 1 << 2;
                        if ( XSTP_ENGINE_StateMachineTCM(om_ptr, lport) )
                            sm_progress_bmp |= 1 << 1;
                    } /* End of if */
                } /* End of if */
            } /* End of for */
        } while (sm_progress_bmp);
    } /* End of if (om_ptr->instance_exist) */

    return;
} /* End of XSTP_ENGINE_StateMachineProgress */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_CheckStateMachineBegin
 *-------------------------------------------------------------------------
 * PURPOSE  : Check the state machine begin status and reset it
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETUEN   : TRUE if the state machine is at the begin status, else FALSE
 * NOTES    : None
 */
BOOL_T  XSTP_ENGINE_CheckStateMachineBegin(XSTP_OM_InstanceData_T *om_ptr)
{
    BOOL_T  begin;

    begin   = om_ptr->bridge_info.common->begin;
    om_ptr->bridge_info.common->begin   = FALSE;

    return  begin;
} /* End of XSTP_ENGINE_CheckStateMachineBegin */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_SetPortState
 *-------------------------------------------------------------------------
 * PURPOSE  : Check the state machine begin status and reset it
 * INPUT    : om_ptr        -- om pointer for this instance
 *            learning      --
 *            forwarding    --
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 */
void    XSTP_ENGINE_SetPortState(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, BOOL_T learning, BOOL_T forwarding)
{
    XSTP_OM_PortVar_T       *pom_ptr;

    pom_ptr = &(om_ptr->port_info[lport-1]);

#if (SYS_CPNT_EAPS == TRUE)
    {
        UI32_T  port_role;

        if (  (TRUE == XSTP_OM_GetEthRingPortRole(lport, &port_role))
            &&(port_role !=  XSTP_TYPE_ETH_RING_PORT_ROLE_NONE)
           )
        {
            /* spanning tree will be disabled on the ring port,
             *  so update OM only...
             */
            pom_ptr->learning   = learning;
            pom_ptr->forwarding = forwarding;
            return;
        }
    }
#endif
    if (learning)
    {
        if(!forwarding) /*already forwarding, needn't set to chip learning again*/
        XSTP_UTY_EnableLearning(om_ptr, lport);

        pom_ptr->learning       = TRUE;
        if (forwarding)
        {
            XSTP_UTY_EnableForwarding(om_ptr, lport);
            pom_ptr->forwarding = TRUE;
        }
        else
        {
            XSTP_UTY_DisableForwarding(om_ptr, lport);
            pom_ptr->forwarding = FALSE;
        }
    }
    else
    {
        if(!learning && forwarding) /*set not-forwarding, needn't to set learning*/
            XSTP_UTY_DisableLearning(om_ptr, lport);
        pom_ptr->learning       = FALSE;
        if (!forwarding)
        {
            XSTP_UTY_DisableForwarding(om_ptr, lport);
            pom_ptr->forwarding = FALSE;
        }
    }

    return;
} /* End of XSTP_ENGINE_SetPortState */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_SetMstEnableStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Check the state machine begin status and reset it
 * INPUT    : state         --
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 */
void    XSTP_ENGINE_SetMstEnableStatus(BOOL_T state)
{
    XSTP_UTY_SetMstEnableStatus(state);

    return;
} /* End of XSTP_ENGINE_SetMstEnableStatus */

/* ===================================================================== */
/* Local Functions */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_DebugStateMachine
 *-------------------------------------------------------------------------
 * PURPOSE  : Print the messages for debugging the state machine
 * INPUT    : lport     -- lport (0 means all ports)
 *            dbg_table -- debug message table
 *            state     -- state
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 */
static  void    XSTP_ENGINE_DebugStateMachine(UI32_T mstid, UI32_T lport, XSTP_ENGINE_DebugStruct_T *dbg_table, UI8_T state)
{
    UI8_T   index;
    BOOL_T  finished;

    if (XSTP_OM_StateDebugMst(mstid) && XSTP_OM_StateDebugPort(lport) && XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGSM))
    {
        index       = 0;
        finished    = FALSE;
        while ( (!finished) && (dbg_table[index].status < XSTP_ENGINE_SM_DEBUG_END) )
        {
            if (dbg_table[index].status == state)
            {
                finished    = TRUE;
                BACKDOOR_MGR_Printf("\r\nXSTP_ENGINE_DebugStateMachine::[mstid %2ld][port %3ld]%s", mstid, lport, dbg_table[index].debug_string);
            }
            else
            {
                index++;
            }
        }
    }

    return;
} /* XSTP_ENGINE_DebugStateMachine */


/* ===================================================================== */
/* ===================================================================== */
/* IEEE 802.1W */
#ifdef XSTP_TYPE_PROTOCOL_RSTP

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_InitBridgeStateMachine
 *-------------------------------------------------------------------------
 * PURPOSE  : Initialize the bridge state machine variables
 * INPUT    : om_ptr    -- om pointer for this instance
 *            system_id_extension   -- System ID Extension (see 13.23.2)
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 */
static  void    XSTP_ENGINE_InitBridgeStateMachine(XSTP_OM_InstanceData_T *om_ptr, UI32_T system_id_extension)
{
    UI8_T                   mac_addr[6];
    XSTP_TYPE_BridgeId_T    bridge_id;
    XSTP_TYPE_PortId_T      zero_port_id;

    om_ptr->bridge_info.time_since_topology_change  = (UI32_T)SYSFUN_GetSysTick();
    om_ptr->bridge_info.topology_change_count       = 0;
    om_ptr->bridge_info.trap_flag_tc                = FALSE;
    om_ptr->bridge_info.trap_flag_new_root          = FALSE;

    om_ptr->bridge_info.sms_prs                     = XSTP_ENGINE_SM_PRS_STATE_INIT_BRIDGE;
    om_ptr->bridge_info.migrate_time                = XSTP_TYPE_DEFAULT_MIGRATE_TIME;
    om_ptr->bridge_info.common->tx_hold_count       = XSTP_TYPE_DEFAULT_TX_HOLD_COUNT;
    om_ptr->bridge_info.common->begin               = TRUE;

    /* bridge_id, zero_port_id */
    SWCTRL_GetCpuMac(mac_addr);

    if (!om_ptr->bridge_info.static_bridge_priority)
    {
        /* Bridge ID, System ID Extension, Bridge Address */
        XSTP_OM_BRIDGE_ID_FORMAT( bridge_id, XSTP_TYPE_DEFAULT_BRIDGE_PRIORITY, system_id_extension, mac_addr);

        /* bridge_identifier */
        memcpy(&om_ptr->bridge_info.bridge_identifier, &bridge_id, XSTP_TYPE_BRIDGE_ID_LENGTH);

        XSTP_OM_PORT_ID_FORMAT(zero_port_id, 0, 0);

        /* bridge_priority_vector */
        XSTP_OM_PRIORITY_VECTOR_FORMAT( om_ptr->bridge_info.bridge_priority, bridge_id, 0, bridge_id, zero_port_id, zero_port_id);
    }

    /* bridge_times */
    XSTP_OM_TIMERS_FORMAT(  om_ptr->bridge_info.bridge_times,
                            0,
                            XSTP_TYPE_DEFAULT_MAX_AGE,
                            XSTP_TYPE_DEFAULT_HELLO_TIME,
                            XSTP_TYPE_DEFAULT_FORWARD_DELAY);

    /* root_port_id */
    memcpy(&om_ptr->bridge_info.root_port_id, &zero_port_id, XSTP_TYPE_PORT_ID_LENGTH);

    /* root_priority_vector */
    memcpy(&om_ptr->bridge_info.root_priority, &om_ptr->bridge_info.bridge_priority, sizeof(XSTP_TYPE_PriorityVector_T));

    /* root_times */
    XSTP_OM_TIMERS_FORMAT(  om_ptr->bridge_info.root_times,
                            0,
                            XSTP_TYPE_DEFAULT_MAX_AGE,
                            XSTP_TYPE_DEFAULT_HELLO_TIME,
                            XSTP_TYPE_DEFAULT_FORWARD_DELAY);

    om_ptr->bridge_info.trap_flag_tc                = FALSE;
    om_ptr->bridge_info.trap_flag_new_root          = FALSE;

    return;
} /* End of XSTP_ENGINE_InitBridgeStateMachine */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_InitPortStateMachines
 *-------------------------------------------------------------------------
 * PURPOSE  : Initialize the port state machine variables
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- logical port number
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 */
void    XSTP_ENGINE_InitPortStateMachines(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    UI32_T  path_cost;
    XSTP_TYPE_PortId_T      my_port_id;

    /* 17.16.5 port_path_cost */
    if (!om_ptr->port_info[lport-1].static_path_cost)
    {
        XSTP_OM_GetLportDefaultPathCost(lport, &path_cost);
        om_ptr->port_info[lport-1].port_path_cost   = path_cost;
    }

    if (!om_ptr->port_info[lport-1].static_port_priority)
    {
        XSTP_OM_PORT_ID_FORMAT(my_port_id, XSTP_TYPE_DEFAULT_PORT_PRIORITY, lport);
        /* 17.18.16 port_id */
        memcpy(&om_ptr->port_info[lport-1].port_id, &my_port_id, XSTP_TYPE_PORT_ID_LENGTH);
    }

    /* State machine status */
    om_ptr->port_info[lport-1].sms_pti              = XSTP_ENGINE_SM_PTI_STATE_ONE_SECOND;
    om_ptr->port_info[lport-1].sms_pim              = XSTP_ENGINE_SM_PIM_STATE_DISABLED;
    om_ptr->port_info[lport-1].sms_prt              = XSTP_ENGINE_SM_PRT_STATE_INIT_PORT;
    om_ptr->port_info[lport-1].sms_pst              = XSTP_ENGINE_SM_PST_STATE_DISCARDING;
    om_ptr->port_info[lport-1].sms_tcm              = XSTP_ENGINE_SM_TCM_STATE_INIT;
    om_ptr->port_info[lport-1].sms_ppm              = XSTP_ENGINE_SM_PPM_STATE_INIT;
    om_ptr->port_info[lport-1].sms_ptx              = XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_INIT;
    /* 802.1D-2004 17.25 */
    if (om_ptr->port_info[lport-1].common->admin_edge == TRUE)
        om_ptr->port_info[lport-1].sms_bdm          = XSTP_ENGINE_SM_BDM_STATE_EDGE;
    else
        om_ptr->port_info[lport-1].sms_bdm          = XSTP_ENGINE_SM_BDM_STATE_NOT_EDGE;

    /* 17.15 State machine timers */
    om_ptr->port_info[lport-1].fd_while             = 0;
    om_ptr->port_info[lport-1].hello_when           = 0;
    om_ptr->port_info[lport-1].mdelay_while         = 0;
    om_ptr->port_info[lport-1].rb_while             = 0;
    om_ptr->port_info[lport-1].rcvd_info_while      = 0;
    om_ptr->port_info[lport-1].rr_while             = 0;
    om_ptr->port_info[lport-1].tc_while             = 0;

    /* 17.18.1 agreed */
    om_ptr->port_info[lport-1].agreed               = FALSE;

    /* 17.18.2 designated_priority */
    XSTP_OM_PRIORITY_VECTOR_FORMAT( om_ptr->port_info[lport-1].designated_priority,
                                    om_ptr->bridge_info.bridge_identifier,
                                    0,
                                    om_ptr->bridge_info.bridge_identifier,
                                    om_ptr->port_info[lport-1].port_id,
                                    om_ptr->port_info[lport-1].port_id);

    /* 17.18.3 designated_times */
    XSTP_OM_TIMERS_FORMAT(  om_ptr->port_info[lport-1].designated_times,
                            0,
                            om_ptr->bridge_info.bridge_times.max_age,
                            om_ptr->bridge_info.bridge_times.hello_time,
                            om_ptr->bridge_info.bridge_times.forward_delay);

    /* 17.18.4 forward, 17.18.5 forwarding */
    om_ptr->port_info[lport-1].forward              = FALSE;
    om_ptr->port_info[lport-1].forwarding           = FALSE;

    /* 17.18.6 info_is */
    om_ptr->port_info[lport-1].info_is              = XSTP_ENGINE_PORTVAR_INFO_IS_DISABLED;

    /* 17.18.7 init_pm */
    om_ptr->port_info[lport-1].common->init_pm      = FALSE;

    /* 17.18.8 learn, 17.18.9 learning */
    om_ptr->port_info[lport-1].learn                = FALSE;
    om_ptr->port_info[lport-1].learning             = FALSE;

    /* 17.18.10 mcheck */
    om_ptr->port_info[lport-1].common->mcheck       = FALSE;

    /* 17.18.11 msg_priority */
    XSTP_OM_PRIORITY_VECTOR_FORMAT( om_ptr->port_info[lport-1].msg_priority,
                                    om_ptr->bridge_info.bridge_identifier,
                                    0,
                                    om_ptr->bridge_info.bridge_identifier,
                                    om_ptr->port_info[lport-1].port_id,
                                    om_ptr->port_info[lport-1].port_id);

    /* 17.18.12 msg_times */
    XSTP_OM_TIMERS_FORMAT(  om_ptr->port_info[lport-1].msg_times,
                            0,
                            om_ptr->bridge_info.bridge_times.max_age,
                            om_ptr->bridge_info.bridge_times.hello_time,
                            om_ptr->bridge_info.bridge_times.forward_delay);

    /* 17.18.13 new_info */
    om_ptr->port_info[lport-1].new_info             = FALSE;

    /*17.18.14 oper_edge
    Init. the oper_edge value in XSTP_OM_InitEdgePortAndLinkType().
    It should not be cleared on disabling the stp or port except in initializing phase.
    om_ptr->port_info[lport-1].common->oper_edge    = FALSE;
    */

    /* 17.18.15 port_enabled
     * No condition except the following 3 conditions changes the port_enabled variable:
     * 1. port admin_state changed by users
     * 2. set port_enabled=FALSE when enter_transaction_state
     * 3. set port_enabled=TRUE if the port exists when enter_master_mode
     */
    /*
    om_ptr->port_info[lport-1].port_enabled         = FALSE;
    */

    /* 17.18.17 port_priority */
    XSTP_OM_PRIORITY_VECTOR_FORMAT( om_ptr->port_info[lport-1].port_priority,
                                    om_ptr->bridge_info.bridge_identifier,
                                    0,
                                    om_ptr->bridge_info.bridge_identifier,
                                    om_ptr->port_info[lport-1].port_id,
                                    om_ptr->port_info[lport-1].port_id);

    /* 17.18.18 port_times */
    XSTP_OM_TIMERS_FORMAT(  om_ptr->port_info[lport-1].port_times,
                            0,
                            om_ptr->bridge_info.bridge_times.max_age,
                            om_ptr->bridge_info.bridge_times.hello_time,
                            om_ptr->bridge_info.bridge_times.forward_delay);

    /* 17.18.19 proposed, 17.18.20 proposing */
    om_ptr->port_info[lport-1].proposed             = FALSE;
    om_ptr->port_info[lport-1].proposing            = FALSE;


    /* 17.18.21 rcvd_bpdu, 17.18.22 rcvd_msg, 17.18.23 rcvd_rstp */
    /* 17.18.24 rcvd_stp, 17.18.25 rcvd_tc, 17.18.26 rcvd_tc_ack, 17.18.27 rcvd_tcn */
    om_ptr->port_info[lport-1].common->rcvd_bpdu    = FALSE;
    om_ptr->port_info[lport-1].rcvd_msg             = XSTP_ENGINE_PORTVAR_RCVD_MSG_OTHER_MSG;
    om_ptr->port_info[lport-1].rcvd_rstp            = FALSE;
    om_ptr->port_info[lport-1].rcvd_stp             = FALSE;
    om_ptr->port_info[lport-1].rcvd_tc              = FALSE;
    om_ptr->port_info[lport-1].rcvd_tc_ack          = FALSE;
    om_ptr->port_info[lport-1].rcvd_tcn             = FALSE;

    /* 17.18.28 re_root, 17.18.29 reselect */
    om_ptr->port_info[lport-1].re_root              = FALSE;
    om_ptr->port_info[lport-1].reselect             = FALSE;

    /* 17.18.30 role, 17.18.31 selected, 17.18.32 selected_role */
    om_ptr->port_info[lport-1].role                 = XSTP_ENGINE_PORTVAR_ROLE_DISABLED;
    om_ptr->port_info[lport-1].selected             = FALSE;
    om_ptr->port_info[lport-1].selected_role        = XSTP_ENGINE_PORTVAR_ROLE_DISABLED;

    /* 17.18.33 send_rstp */
    om_ptr->port_info[lport-1].common->send_rstp    = FALSE;

    /* 17.18.34 sync, 17.18.35 synced */
    om_ptr->port_info[lport-1].sync                 = FALSE;
    om_ptr->port_info[lport-1].synced               = FALSE;

    /* 17.18.36 tc, 17.18.37 tc_ack, 17.18.38 tc_prop */
    om_ptr->port_info[lport-1].tc                   = FALSE;
    om_ptr->port_info[lport-1].tc_ack               = FALSE;
    om_ptr->port_info[lport-1].tc_prop              = FALSE;

    /* 17.18.39 tick */
    om_ptr->port_info[lport-1].common->tick         = FALSE;

    /* 17.18.40 tx_count */
    om_ptr->port_info[lport-1].tx_count             = 0;

    /* 17.18.41 updt_info */
    om_ptr->port_info[lport-1].updt_info            = FALSE;

    /* 1D/D3 17.19.6 disputed */
    om_ptr->port_info[lport-1].disputed             = FALSE;

#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
    om_ptr->port_info[lport-1].common->root_inconsistent = FALSE;
#endif

    return;
} /* End of XSTP_ENGINE_InitPortStateMachines */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_StateMachinePTI
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the state machine process
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport (1..max)
 * OUTPUT   : None
 * RETUEN   : TRUE if any of the timers is expired, else FALSE
 * NOTES    : Ref to the description in 17.20, IEEE Std 802.1w-2001
 */
static  BOOL_T  XSTP_ENGINE_StateMachinePTI(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI8_T               next_status;
    BOOL_T              result;
    BOOL_T              timeout;
    BOOL_T              sm_progress;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    next_status = pom_ptr->sms_pti;
    timeout     = FALSE;

    do
    {
        sm_progress = FALSE;
        switch (next_status)
        {
            case XSTP_ENGINE_SM_PTI_STATE_TICK:
                result  = XSTP_UTY_DecreaseTimer(&pom_ptr->hello_when);
                timeout = timeout || result;
                result  = XSTP_UTY_DecreaseTimer(&pom_ptr->tc_while);
                timeout = timeout || result;
                result  = XSTP_UTY_DecreaseTimer(&pom_ptr->fd_while);
                timeout = timeout || result;
                result  = XSTP_UTY_DecreaseTimer(&pom_ptr->rcvd_info_while);
                timeout = timeout || result;
                result  = XSTP_UTY_DecreaseTimer(&pom_ptr->rr_while);
                timeout = timeout || result;
                result  = XSTP_UTY_DecreaseTimer(&pom_ptr->rb_while);
                timeout = timeout || result;
                result  = XSTP_UTY_DecreaseTimer(&pom_ptr->mdelay_while);
                timeout = timeout || result;
                result  = XSTP_UTY_DecreaseTimer(&pom_ptr->tx_count);
                /* It triggles nothing when this timer becomes zero */
                /*
                timeout = timeout || result;
                */
                result  = XSTP_UTY_DecreaseTimer(&pom_ptr->common->edge_delay_while);
                timeout = timeout || result;
#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
                if (pom_ptr->common->bpdu_guard_auto_recovery == XSTP_TYPE_PORT_BPDU_GUARD_AUTO_RECOVERY_ENABLED)
                {
                    if (result = XSTP_UTY_DecreaseTimer32(&pom_ptr->common->bpdu_guard_auto_recovery_while) == TRUE)
                    {
                        XSTP_UTY_SetBpduGuardRecoverPortList(lport);
                    }
                    timeout = timeout || result;
                }
#endif /* #if (SYS_CPNT_STP_BPDU_GUARD == TRUE) */
                /* Progress */
                /* UCT */
                next_status             = XSTP_ENGINE_SM_PTI_STATE_ONE_SECOND;

            case XSTP_ENGINE_SM_PTI_STATE_ONE_SECOND:
                pom_ptr->common->tick   = FALSE;
            case XSTP_ENGINE_SM_PTI_STATE_PROGRESS_ONE_SECOND:
                /* Progress */
                if (pom_ptr->common->tick == TRUE)
                {
                    sm_progress         = TRUE;
                    next_status         = XSTP_ENGINE_SM_PTI_STATE_TICK;
                }
                else
                    next_status         = XSTP_ENGINE_SM_PTI_STATE_PROGRESS_ONE_SECOND;
                break;

            default:
                break;
        }
    } while (sm_progress);

    pom_ptr->sms_pti    = next_status;

    return timeout;
} /* End of XSTP_ENGINE_StateMachinePTI */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_StateMachinePIM
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the state machine process
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport (1..max)
 * OUTPUT   : None
 * RETUEN   : TRUE if the state machine progresses, else FALSE
 * NOTES    : Ref to the description in 17.21, IEEE Std 802.1w-2001
 */
static  BOOL_T  XSTP_ENGINE_StateMachinePIM(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI8_T               next_status;
    BOOL_T              sm_progress, uct, uct_progress;

    pom_ptr         = &(om_ptr->port_info[lport-1]);
    next_status     = pom_ptr->sms_pim;
    sm_progress     = TRUE;
    uct_progress    = FALSE;

    do
    {
        uct = FALSE;
        XSTP_ENGINE_DebugStateMachine(om_ptr->instance_id, lport, XSTP_ENGINE_DebugPIM, next_status);
        switch (next_status)
        {
            /* DISABLED */
            case XSTP_ENGINE_SM_PIM_STATE_DISABLED:
                /* Action */
                pom_ptr->common->rcvd_bpdu = pom_ptr->rcvd_rstp = pom_ptr->rcvd_stp = FALSE;
                memcpy(&pom_ptr->port_priority, &pom_ptr->designated_priority,  sizeof(XSTP_TYPE_PriorityVector_T) );
                memcpy(&pom_ptr->port_times,    &pom_ptr->designated_times,     sizeof(XSTP_TYPE_Timers_T) );
                pom_ptr->updt_info = pom_ptr->proposing = FALSE;
                pom_ptr->agreed = pom_ptr->proposed = FALSE;
                pom_ptr->rcvd_info_while    = 0;
                pom_ptr->info_is            = XSTP_ENGINE_PORTVAR_INFO_IS_DISABLED;
                pom_ptr->reselect           = TRUE;
                pom_ptr->selected           = FALSE;
            case XSTP_ENGINE_SM_PIM_STATE_PROGRESS_DISABLED:
                /* Progress */
                if (pom_ptr->updt_info)
                    next_status = XSTP_ENGINE_SM_PIM_STATE_DISABLED;
                else
                if (    (pom_ptr->common->port_enabled)
                     && (pom_ptr->common->link_up)
                   )
                    next_status = XSTP_ENGINE_SM_PIM_STATE_AGED;
                else
                if (pom_ptr->common->rcvd_bpdu)
                    next_status = XSTP_ENGINE_SM_PIM_STATE_DISABLED;
                else
                {
                    next_status = XSTP_ENGINE_SM_PIM_STATE_PROGRESS_DISABLED;
                    sm_progress = FALSE;
                }
                break;

            /* AGED */
            case XSTP_ENGINE_SM_PIM_STATE_AGED:
                /* Action */
                pom_ptr->info_is    = XSTP_ENGINE_PORTVAR_INFO_IS_AGED;
                pom_ptr->reselect   = TRUE;
                pom_ptr->selected   = FALSE;
#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
                if (pom_ptr->common->root_guard_status == TRUE)
                {
                    pom_ptr->common->root_inconsistent = FALSE;
                }
#endif /* #if (SYS_CPNT_STP_ROOT_GUARD == TRUE) */
            case XSTP_ENGINE_SM_PIM_STATE_PROGRESS_AGED:
                /* Progress */
                if (pom_ptr->selected && pom_ptr->updt_info)
                    next_status = XSTP_ENGINE_SM_PIM_STATE_UPDATE;
                else
                {
                    next_status = XSTP_ENGINE_SM_PIM_STATE_PROGRESS_AGED;
                    sm_progress = FALSE;
                }
                break;

            /* UPDATE */
            case XSTP_ENGINE_SM_PIM_STATE_UPDATE:
                /* Action */
                memcpy(&pom_ptr->port_priority, &pom_ptr->designated_priority,  sizeof(XSTP_TYPE_PriorityVector_T) );
                memcpy(&pom_ptr->port_times,    &pom_ptr->designated_times,     sizeof(XSTP_TYPE_Timers_T) );
                pom_ptr->updt_info  = FALSE;
                pom_ptr->info_is    = XSTP_ENGINE_PORTVAR_INFO_IS_MINE;
                pom_ptr->agreed = pom_ptr->agreed &&XSTP_UTY_BetterOrSameInfo(om_ptr, lport);
                pom_ptr->synced = pom_ptr->synced&& pom_ptr->agreed;
                pom_ptr->proposed   = pom_ptr->proposing = FALSE;
                pom_ptr->new_info   = TRUE;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status = XSTP_ENGINE_SM_PIM_STATE_CURRENT;
                break;

            /* CURRENT */
            case XSTP_ENGINE_SM_PIM_STATE_CURRENT:
                /* Action */
                /* None */
            case XSTP_ENGINE_SM_PIM_STATE_PROGRESS_CURRENT:
                /* Progress */
                if (pom_ptr->selected && pom_ptr->updt_info)
                    next_status     = XSTP_ENGINE_SM_PIM_STATE_UPDATE;
                else
                if (   (   (pom_ptr->info_is == XSTP_ENGINE_PORTVAR_INFO_IS_RECEIVED)
                        && (pom_ptr->rcvd_info_while == 0)
                        && (!(pom_ptr->updt_info))
                        && (!(pom_ptr->common->rcvd_bpdu))
                       )
                   )
                    next_status     = XSTP_ENGINE_SM_PIM_STATE_AGED;
                else
                if ( pom_ptr->common->rcvd_bpdu && !(pom_ptr->updt_info) )
                    next_status     = XSTP_ENGINE_SM_PIM_STATE_RECEIVE;
                else
                {
                    next_status     = XSTP_ENGINE_SM_PIM_STATE_PROGRESS_CURRENT;
                    sm_progress     = FALSE;
                }

                break;

            /* SUPERIOR */
            case XSTP_ENGINE_SM_PIM_STATE_SUPERIOR:
                /* Action */
#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
                if (   (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
                    && (pom_ptr->common->root_guard_status == TRUE)
                   )
                {
                    pom_ptr->common->root_inconsistent  = TRUE;
                    pom_ptr->fd_while                   = FwdDelay;
                    pom_ptr->agreed  = pom_ptr->synced = FALSE;
                    XSTP_UTY_UpdtRcvdInfoWhile(om_ptr, lport);
                    pom_ptr->info_is    = XSTP_ENGINE_PORTVAR_INFO_IS_RECEIVED;
                }
                else
#endif /* #if (SYS_CPNT_STP_ROOT_GUARD == TRUE) */
                {
                    memcpy(&pom_ptr->port_priority, &pom_ptr->msg_priority, sizeof(XSTP_TYPE_PriorityVector_T) );
                    memcpy(&pom_ptr->port_times,    &pom_ptr->msg_times,    sizeof(XSTP_TYPE_Timers_T) );
                    XSTP_UTY_UpdtRcvdInfoWhile(om_ptr, lport);
                    pom_ptr->agreed     = pom_ptr->proposing = pom_ptr->synced = FALSE;
                    pom_ptr->proposed   = XSTP_UTY_RecordProposed(om_ptr, lport);
                    pom_ptr->info_is    = XSTP_ENGINE_PORTVAR_INFO_IS_RECEIVED;
                    pom_ptr->reselect   = TRUE;
                    pom_ptr->selected   = FALSE;
                }
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PIM_STATE_CURRENT;
                break;

            /* REPEAT */
            case XSTP_ENGINE_SM_PIM_STATE_REPEAT:
                /* Action */
                pom_ptr->proposed   = XSTP_UTY_RecordProposed(om_ptr, lport);
                XSTP_UTY_UpdtRcvdInfoWhile(om_ptr, lport);
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PIM_STATE_CURRENT;
                break;

            /* AGREEMENT */
            case XSTP_ENGINE_SM_PIM_STATE_AGREEMENT:
                /* Action */
                pom_ptr->agreed     = TRUE;
                pom_ptr->proposing  = FALSE;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PIM_STATE_CURRENT;
                break;

            /* RECEIVE */
            case XSTP_ENGINE_SM_PIM_STATE_RECEIVE:
                /* Action */
                pom_ptr->rcvd_msg = XSTP_UTY_RcvBpdu(om_ptr, lport);
                XSTP_UTY_UpdtBpduVersion(om_ptr, lport);
                XSTP_UTY_SetTcFlags(om_ptr, lport);
                pom_ptr->common->rcvd_bpdu = FALSE;
            case XSTP_ENGINE_SM_PIM_STATE_PROGRESS_RECEIVE:
                /* Progress */
                if (pom_ptr->rcvd_msg == XSTP_ENGINE_PORTVAR_RCVD_MSG_SUPERIOR_DESIGNATED_MSG)
                    next_status = XSTP_ENGINE_SM_PIM_STATE_SUPERIOR;
                else
                if (pom_ptr->rcvd_msg == XSTP_ENGINE_PORTVAR_RCVD_MSG_REPEATED_DESIGNATED_MSG)
                    next_status = XSTP_ENGINE_SM_PIM_STATE_REPEAT;
                else
                if (pom_ptr->rcvd_msg == XSTP_ENGINE_PORTVAR_RCVD_MSG_CONFIRMED_ROOT_MSG)
                    next_status = XSTP_ENGINE_SM_PIM_STATE_AGREEMENT;
                else
                if (pom_ptr->rcvd_msg == XSTP_ENGINE_PORTVAR_RCVD_MSG_OTHER_MSG)
                    next_status = XSTP_ENGINE_SM_PIM_STATE_CURRENT;
                else
                {
                    next_status = XSTP_ENGINE_SM_PIM_STATE_CURRENT;
                    sm_progress = FALSE;
                }
                break;

            default:
                break;
        } /* End of switch */
    } while (uct);

    pom_ptr->sms_pim    = next_status;

    return (sm_progress || uct_progress);
} /* End of XSTP_ENGINE_StateMachinePIM */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_StateMachinePRS
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the state machine process
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETUEN   : TRUE if the state machine progresses, else FALSE
 * NOTES    : Ref to the description in 17.22, IEEE Std 802.1w-2001
 */
static  BOOL_T  XSTP_ENGINE_StateMachinePRS(XSTP_OM_InstanceData_T *om_ptr)
{
    XSTP_OM_BridgeVar_T *bom_ptr;
    UI8_T               next_status;
    BOOL_T              sm_progress, uct_progress;

    bom_ptr         = &(om_ptr->bridge_info);
    next_status     = bom_ptr->sms_prs;
    sm_progress     = TRUE;
    uct_progress    = FALSE;

    XSTP_ENGINE_DebugStateMachine(om_ptr->instance_id, 0, XSTP_ENGINE_DebugPRS, next_status);
    switch (next_status)
    {
        /* INIT_BRIDGE */
        case XSTP_ENGINE_SM_PRS_STATE_INIT_BRIDGE:
            /* Action */
            XSTP_UTY_UpdtRoleDisabledBridge(om_ptr);
            /* Progress */
            /* UCT */
            next_status     = XSTP_ENGINE_SM_PRS_STATE_ROLE_SELECTION;
            uct_progress    = TRUE;

        /* ROLE_SELECTION */
        case XSTP_ENGINE_SM_PRS_STATE_ROLE_SELECTION:
            /* Action */
            XSTP_UTY_ClearReselectBridge(om_ptr);
            XSTP_UTY_UpdtRolesBridge(om_ptr);
            XSTP_UTY_SetSelectedBridge(om_ptr);
        case XSTP_ENGINE_SM_PRS_STATE_PROGRESS_ROLE_SELECTION:
            /* Progress */
            if ( XSTP_UTY_ReselectForAnyPort(om_ptr) )
                next_status = XSTP_ENGINE_SM_PRS_STATE_ROLE_SELECTION;
            else
            {
                next_status = XSTP_ENGINE_SM_PRS_STATE_PROGRESS_ROLE_SELECTION;
                sm_progress = FALSE;
            }
            break;

        default:
            break;
    }

    bom_ptr->sms_prs   = next_status;

    return (sm_progress || uct_progress);
} /* End of XSTP_ENGINE_StateMachinePRS */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_StateMachinePRT
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the state machine process
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport (1..max)
 * OUTPUT   : None
 * RETUEN   : TRUE if the state machine progresses, else FALSE
 * NOTES    : Ref to the description in 17.23, IEEE Std 802.1w-2001
 */
static  BOOL_T  XSTP_ENGINE_StateMachinePRT(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI8_T               next_status;
    BOOL_T              sm_progress, uct, uct_progress;

    pom_ptr         = &(om_ptr->port_info[lport-1]);
    next_status     = pom_ptr->sms_prt;
    sm_progress     = TRUE;
    uct_progress    = FALSE;

    do
    {
        uct = FALSE;

        if (pom_ptr->role != pom_ptr->selected_role)
        {
            if (    (pom_ptr->selected)
                 && (!pom_ptr->updt_info)
               )
            {
                if (   (pom_ptr->selected_role == XSTP_ENGINE_PORTVAR_ROLE_DISABLED)
                    || (pom_ptr->selected_role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
                    || (pom_ptr->selected_role == XSTP_ENGINE_PORTVAR_ROLE_BACKUP)
                   )
                {
                    next_status = XSTP_ENGINE_SM_PRT_STATE_BLOCK_PORT;
                }
                else
                if (pom_ptr->selected_role == XSTP_ENGINE_PORTVAR_ROLE_ROOT)
                {
                    next_status = XSTP_ENGINE_SM_PRT_STATE_ROOT_PORT;
                }
                else
                if (pom_ptr->selected_role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
                {
                    next_status = XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_PORT;
                }
                else
                {
                    /* Error */
                    uct_progress    = FALSE;
                    sm_progress     = FALSE;
                    break;
                }
            }
            else
            {
                /* Don't continue the following operation because it is not in stable state */
                uct_progress    = FALSE;
                sm_progress     = FALSE;
                break;
            }
        } /* End of if (role != selectedRole) */

        XSTP_ENGINE_DebugStateMachine(om_ptr->instance_id, lport, XSTP_ENGINE_DebugPRT, next_status);
        switch (next_status)
        {
            /* Disabled, alternate, and backup role */
            case XSTP_ENGINE_SM_PRT_STATE_INIT_PORT:
                /* Action */
                pom_ptr->role       = XSTP_ENGINE_PORTVAR_ROLE_DISABLED;
                pom_ptr->synced     = FALSE;
                pom_ptr->sync       = pom_ptr->re_root  = TRUE;
                pom_ptr->rr_while   = pom_ptr->fd_while = FwdDelay;
                pom_ptr->rb_while   = 0;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_BLOCK_PORT;
                break;

            case XSTP_ENGINE_SM_PRT_STATE_BLOCK_PORT:
                /* Action */
                pom_ptr->role       = pom_ptr->selected_role;
                pom_ptr->learn      = pom_ptr->forward = FALSE;
            case XSTP_ENGINE_SM_PRT_STATE_PROGRESS_BLOCK_PORT:
                /* Progress */
                if (    pom_ptr->selected
                    &&  !pom_ptr->updt_info
                    &&  !pom_ptr->learning
                    &&  !pom_ptr->forwarding
                   )
                    next_status = XSTP_ENGINE_SM_PRT_STATE_BLOCKED_PORT;
                else
                {
                    next_status     = XSTP_ENGINE_SM_PRT_STATE_PROGRESS_BLOCK_PORT;
                    sm_progress     = FALSE;
                }
                break;

            case XSTP_ENGINE_SM_PRT_STATE_BACKUP_PORT:
                /* Action */
                pom_ptr->rb_while   = 2 * HelloTime;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_BLOCKED_PORT;
                break;

            case XSTP_ENGINE_SM_PRT_STATE_BLOCKED_PORT:
                /* Action */
                pom_ptr->fd_while   = FwdDelay;
                pom_ptr->synced     = TRUE;
                pom_ptr->rr_while   = 0;
                pom_ptr->sync = pom_ptr->re_root = FALSE;
            case XSTP_ENGINE_SM_PRT_STATE_PROGRESS_BLOCKED_PORT:
                /* Progress */
                if (pom_ptr->selected && !pom_ptr->updt_info)
                {
                    if (    (pom_ptr->fd_while != FwdDelay)
                        ||  pom_ptr->sync
                        ||  pom_ptr->re_root
                        ||  !pom_ptr->synced
                       )
                        next_status = XSTP_ENGINE_SM_PRT_STATE_BLOCKED_PORT;
                    else
                    if (    (pom_ptr->rb_while != 2*HelloTime)
                        &&  (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_BACKUP)
                       )
                        next_status = XSTP_ENGINE_SM_PRT_STATE_BACKUP_PORT;
                    else
                    {
                        next_status = XSTP_ENGINE_SM_PRT_STATE_PROGRESS_BLOCKED_PORT;
                        sm_progress = FALSE;
                    }
                }
                else
                {
                    next_status     = XSTP_ENGINE_SM_PRT_STATE_PROGRESS_BLOCKED_PORT;
                    sm_progress     = FALSE;
                }
                break;

            /* Root port role */
            case XSTP_ENGINE_SM_PRT_STATE_ROOT_PROPOSED:
                /* Action */
                XSTP_UTY_SetSyncBridge(om_ptr);
                pom_ptr->proposed   = FALSE;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_ROOT_PORT;
                break;

            case XSTP_ENGINE_SM_PRT_STATE_ROOT_AGREED:
                /* Action */
                pom_ptr->proposed   = pom_ptr->sync = FALSE;
                pom_ptr->synced     = TRUE;
                pom_ptr->new_info   = pom_ptr->common->send_rstp; /* patch. The new_info value only set when tc_while is on for root port connected to 1D switch */
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_ROOT_PORT;
                break;

            case XSTP_ENGINE_SM_PRT_STATE_ROOT_FORWARD:
                /* Action */
                pom_ptr->fd_while   = 0;
                pom_ptr->forward    = TRUE;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_ROOT_PORT;
                break;

            case XSTP_ENGINE_SM_PRT_STATE_ROOT_LEARN:
                /* Action */
                pom_ptr->fd_while   = FwdDelay;
                pom_ptr->learn      = TRUE;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_ROOT_PORT;
                break;

            case XSTP_ENGINE_SM_PRT_STATE_ROOT_PORT:
                /* Action */
                pom_ptr->role       = XSTP_ENGINE_PORTVAR_ROLE_ROOT;
                pom_ptr->rr_while   = FwdDelay;
            case XSTP_ENGINE_SM_PRT_STATE_PROGRESS_ROOT_PORT:
                /* Progress */
                if (pom_ptr->selected && !pom_ptr->updt_info)
                {
                    if (pom_ptr->proposed && !pom_ptr->synced)
                        next_status = XSTP_ENGINE_SM_PRT_STATE_ROOT_PROPOSED;
                    else
                    if (    (pom_ptr->proposed  && XSTP_UTY_AllSyncedForOthers(om_ptr, lport) )
                        ||  (!pom_ptr->synced   && XSTP_UTY_AllSyncedForOthers(om_ptr, lport) )
                       )
                        next_status = XSTP_ENGINE_SM_PRT_STATE_ROOT_AGREED;
                    else
                    if (!pom_ptr->forward && !pom_ptr->re_root)
                        next_status = XSTP_ENGINE_SM_PRT_STATE_REROOT;
                    else
                    if (    (   (pom_ptr->fd_while == 0)
                             || (   (pom_ptr->rb_while == 0)
                                 && (XSTP_UTY_ReRootedForOthers(om_ptr, lport)
                                )
                             && (XSTP_OM_GetForceVersion() >= 2)
                            )
                            )
                        &&  pom_ptr->learn
                        &&  !pom_ptr->forward
                       )
                        next_status = XSTP_ENGINE_SM_PRT_STATE_ROOT_FORWARD;
                    else
                    if (    (   (pom_ptr->fd_while == 0)
                             || (   (pom_ptr->rb_while == 0)
                                 && (XSTP_UTY_ReRootedForOthers(om_ptr, lport)
                                )
                             && (XSTP_OM_GetForceVersion() >= 2)
                            )
                            )
                        &&  !pom_ptr->learn
                       )
                        next_status = XSTP_ENGINE_SM_PRT_STATE_ROOT_LEARN;
                    else
                    if (pom_ptr->re_root && pom_ptr->forward)
                        next_status = XSTP_ENGINE_SM_PRT_STATE_REROOTED;
                    else
                    if (pom_ptr->rr_while != FwdDelay)
                        next_status = XSTP_ENGINE_SM_PRT_STATE_ROOT_PORT;
                    else
                    {
                        next_status = XSTP_ENGINE_SM_PRT_STATE_PROGRESS_ROOT_PORT;
                        sm_progress = FALSE;
                    }
                }
                else
                {
                    next_status = XSTP_ENGINE_SM_PRT_STATE_PROGRESS_ROOT_PORT;
                    sm_progress = FALSE;
                }
                break;

            case XSTP_ENGINE_SM_PRT_STATE_REROOT:
                /* Action */
                XSTP_UTY_SetReRootBridge(om_ptr);
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_ROOT_PORT;
                break;

            case XSTP_ENGINE_SM_PRT_STATE_REROOTED:
                /* Action */
                pom_ptr->re_root    = FALSE;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_ROOT_PORT;
                break;

            /* Designated port role */
            case XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_PROPOSE:
                /* Action */
                pom_ptr->proposing  = TRUE;
                pom_ptr->new_info   = TRUE;
                pom_ptr->common->edge_delay_while = XSTP_UTY_NewEdgeDelayWhile(om_ptr, lport); /* 802.1D-2004 17.29.3 */
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_PORT;
                break;

            case XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_SYNCED:
                /* Action */
                pom_ptr->rr_while   = 0;
                pom_ptr->synced     = TRUE;
                pom_ptr->sync       = FALSE;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_PORT;
                break;

            case XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_RETIRED:
                /* Action */
                pom_ptr->re_root    = FALSE;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_PORT;
                break;

            case XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_FORWARD:
                /* Action */
                pom_ptr->forward    = TRUE;
                pom_ptr->fd_while   = 0;
                pom_ptr->agreed     = pom_ptr->common->send_rstp;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_PORT;
                break;

            case XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_LEARN:
                /* Action */
                pom_ptr->learn      = TRUE;
                pom_ptr->fd_while   = FwdDelay;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_PORT;
                break;

            case XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_LISTEN:
                /* Action */
                pom_ptr->learn      = pom_ptr->forward
                                    = pom_ptr->disputed
                                    = FALSE;

                if (pom_ptr->common->loopback_detect_mode == VAL_staLoopbackDetectionPortReleaseMode_auto)
                    pom_ptr->common->loopback_block = FALSE;

                pom_ptr->fd_while   = FwdDelay;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_PORT;
                break;

            case XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_PORT:
                /* Action */
                pom_ptr->role       = XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED;
            case XSTP_ENGINE_SM_PRT_STATE_PROGRESS_DESIGNATED_PORT:
                /* Progress */
                if (pom_ptr->selected && !pom_ptr->updt_info)
                {
                    if (    !pom_ptr->forward
                        &&  !pom_ptr->agreed
                        &&  !pom_ptr->proposing
                        &&  !pom_ptr->common->oper_edge
                       )
                        next_status = XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_PROPOSE;
                    else
                    if (    (   !pom_ptr->learning
                             && !pom_ptr->forwarding
                             && !pom_ptr->synced
                            )
                        ||  (pom_ptr->agreed            && !pom_ptr->synced)
                        ||  (pom_ptr->common->oper_edge && !pom_ptr->synced)
                        ||  (pom_ptr->sync              && pom_ptr->synced)
                       )
                        next_status = XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_SYNCED;
                    else
                    if ( (pom_ptr->rr_while == 0) && pom_ptr->re_root)
                        next_status = XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_RETIRED;
                    else
                    if (    (   (pom_ptr->fd_while == 0)
                             || pom_ptr->agreed
                             || pom_ptr->common->oper_edge
                            )
                        &&  (   (pom_ptr->rr_while == 0)
                             || !pom_ptr->re_root
                            )
                        &&  !pom_ptr->sync
                        &&  (pom_ptr->learn && !pom_ptr->forward)
                       )
                        next_status = XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_FORWARD;
                    else
                    if (    (   (pom_ptr->fd_while == 0)
                             || pom_ptr->agreed
                             || pom_ptr->common->oper_edge
                            )
                        &&  (   (pom_ptr->rr_while == 0)
                             || !pom_ptr->re_root
                            )
                        &&  !pom_ptr->sync
                        &&  !pom_ptr->learn
                       )
                        next_status = XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_LEARN;
                    else
                    if (    (   (pom_ptr->sync && !pom_ptr->synced)
                             || (   pom_ptr->re_root
                                 && (pom_ptr->rr_while != 0)
                                )
                             || (pom_ptr->common->loopback_block)
                             || (pom_ptr->disputed)
#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
                             || (pom_ptr->common->root_inconsistent == TRUE)
#endif /* #if (SYS_CPNT_STP_ROOT_GUARD == TRUE) */
                            )
                        &&  !pom_ptr->common->oper_edge
                        &&  (pom_ptr->learn || pom_ptr->forward)
                       )
                        next_status = XSTP_ENGINE_SM_PRT_STATE_DESIGNATED_LISTEN;
                    else
                    {
                        next_status = XSTP_ENGINE_SM_PRT_STATE_PROGRESS_DESIGNATED_PORT;
                        sm_progress = FALSE;
                    }
                }
                else
                {
                    next_status = XSTP_ENGINE_SM_PRT_STATE_PROGRESS_DESIGNATED_PORT;
                    sm_progress = FALSE;
                }
                break;

            default:
                break;
        } /* End of switch */
    } while (uct);

    pom_ptr->sms_prt    = next_status;

    return (sm_progress || uct_progress);
} /* End of XSTP_ENGINE_StateMachinePRT */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_StateMachinePST
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the state machine process
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport (1..max)
 * OUTPUT   : None
 * RETUEN   : TRUE if the state machine progresses, else FALSE
 * NOTES    : Ref to the description in 17.24, IEEE Std 802.1w-2001
 */
static  BOOL_T  XSTP_ENGINE_StateMachinePST(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI8_T               next_status;
    BOOL_T              sm_progress, uct, uct_progress;

    pom_ptr = &(om_ptr->port_info[lport-1]);
    next_status     = pom_ptr->sms_pst;
    sm_progress     = TRUE;
    uct_progress    = FALSE;

    do
    {
        uct = FALSE;
        XSTP_ENGINE_DebugStateMachine(om_ptr->instance_id, lport, XSTP_ENGINE_DebugPST, next_status);
        switch (next_status)
        {
            case XSTP_ENGINE_SM_PST_STATE_DISCARDING:
                /* Action */
                XSTP_UTY_DisableLearning(om_ptr, lport);
                pom_ptr->learning   = FALSE;
                XSTP_UTY_DisableForwarding(om_ptr, lport);
                pom_ptr->forwarding = FALSE;
            case XSTP_ENGINE_SM_PST_STATE_PROGRESS_DISCARDING:
                /* Progress */
                if (pom_ptr->learn)
                    next_status = XSTP_ENGINE_SM_PST_STATE_LEARNING;
                else
                {
                    next_status     = XSTP_ENGINE_SM_PST_STATE_PROGRESS_DISCARDING;
                    sm_progress     = FALSE;
                }
                break;

            case XSTP_ENGINE_SM_PST_STATE_LEARNING:
                /* Action */
                XSTP_UTY_EnableLearning(om_ptr, lport);
                pom_ptr->learning   = TRUE;
            case XSTP_ENGINE_SM_PST_STATE_PROGRESS_LEARNING:
                /* Progress */
                if (pom_ptr->forward)
                    next_status     = XSTP_ENGINE_SM_PST_STATE_FORWARDING;
                else
                if (!pom_ptr->learn)
                    next_status     = XSTP_ENGINE_SM_PST_STATE_DISCARDING;
                else
                {
                    next_status     = XSTP_ENGINE_SM_PST_STATE_PROGRESS_LEARNING;
                    sm_progress     = FALSE;
                }
                break;

            case XSTP_ENGINE_SM_PST_STATE_FORWARDING:
                /* Action */
                XSTP_UTY_EnableForwarding(om_ptr, lport);
                pom_ptr->forwarding = TRUE;
            case XSTP_ENGINE_SM_PST_STATE_PROGRESS_FORWARDING:
                /* Progress */
                if (!pom_ptr->forward)
                    next_status     = XSTP_ENGINE_SM_PST_STATE_DISCARDING;
                else
                {
                    next_status     = XSTP_ENGINE_SM_PST_STATE_PROGRESS_FORWARDING;
                    sm_progress     = FALSE;
                }
                break;

            default:
                break;
        } /* End of switch */
    } while (uct);

    pom_ptr->sms_pst    = next_status;

    return (sm_progress || uct_progress);
} /* End of XSTP_ENGINE_StateMachinePST */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_StateMachineTCM
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the state machine process
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport (1..max)
 * OUTPUT   : None
 * RETUEN   : TRUE if the state machine progresses, else FALSE
 * NOTES    : Ref to the description in 17.25, IEEE Std 802.1w-2001
 */
static  BOOL_T  XSTP_ENGINE_StateMachineTCM(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI8_T               next_status;
    BOOL_T              sm_progress, uct, uct_progress;

    pom_ptr = &(om_ptr->port_info[lport-1]);
    next_status     = pom_ptr->sms_tcm;
    sm_progress     = TRUE;
    uct_progress    = FALSE;

    do
    {
        uct = FALSE;
        XSTP_ENGINE_DebugStateMachine(om_ptr->instance_id, lport, XSTP_ENGINE_DebugTCM, next_status);
        switch (next_status)
        {
            /* INIT */
            case XSTP_ENGINE_SM_TCM_STATE_INIT:
                /* Action */
                XSTP_UTY_Flush(om_ptr, lport);
                pom_ptr->tc_while   = 0;
                pom_ptr->tc_ack     = FALSE;
            case XSTP_ENGINE_SM_TCM_STATE_PROGRESS_INIT:        /* Follow IEEE P802.1D/D3, page 171, Figure 17-25 TCM machine */
                /* Progress */
                if (pom_ptr->learn)
                    next_status         = XSTP_ENGINE_SM_TCM_STATE_INACTIVE;
                else
                {
                    next_status     = XSTP_ENGINE_SM_TCM_STATE_PROGRESS_INIT;
                    sm_progress     = FALSE;
                }
                break;

            /* INACTIVE */
            case XSTP_ENGINE_SM_TCM_STATE_INACTIVE:
                /* Action */
                pom_ptr->rcvd_tc    = pom_ptr->rcvd_tcn
                                    = pom_ptr->rcvd_tc_ack
                                    = pom_ptr->tc_prop
                                    = FALSE;
            case XSTP_ENGINE_SM_TCM_STATE_PROGRESS_INACTIVE:
                /* Progress */
                if (    pom_ptr->rcvd_tc
                    ||  pom_ptr->rcvd_tcn
                    ||  pom_ptr->rcvd_tc_ack
                    ||  pom_ptr->tc_prop
                   )
                    next_status = XSTP_ENGINE_SM_TCM_STATE_INACTIVE;
                else
                if (    (pom_ptr->role != XSTP_ENGINE_PORTVAR_ROLE_ROOT)
                    &&  (pom_ptr->role != XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
                    &&  (!(     pom_ptr->learn
                            ||  pom_ptr->learning
                          )
                        )
                    &&  (!(     pom_ptr->rcvd_tc
                            ||  pom_ptr->rcvd_tcn
                            ||  pom_ptr->rcvd_tc_ack
                            ||  pom_ptr->tc_prop
                           )
                         )
                   )
                    next_status     = XSTP_ENGINE_SM_TCM_STATE_INIT;
                else
                if (    (   (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ROOT)
                         || (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
                        )
                    &&  pom_ptr->forward
                    &&  !(pom_ptr->common->oper_edge)
                   )
                    next_status     = XSTP_ENGINE_SM_TCM_STATE_DETECTED;
                else
                {
                    next_status     = XSTP_ENGINE_SM_TCM_STATE_PROGRESS_INACTIVE;
                    sm_progress     = FALSE;
                }
                break;

            /* DETECTED */
            case XSTP_ENGINE_SM_TCM_STATE_DETECTED:
                /* Action */
                pom_ptr->tc_while   = XSTP_UTY_NewTcWhile(om_ptr, lport);
                /*move from XSTP_UTY_IncTopologyChangeCount to here,
                  so that it only triger on rx port but not all prop port*/
                XSTP_OM_SetTrapFlagTc(TRUE);
                XSTP_OM_SetTcCausePort(lport);

                XSTP_UTY_SetTcPropBridge(om_ptr, lport);
                /* Set newInfo=TRUE according to 1D/D3 Figure 17-25 DETECTED state of Topology Change state machine */
                pom_ptr->new_info   = TRUE;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_TCM_STATE_ACTIVE;
                break;

            /* ACTIVE */
            case XSTP_ENGINE_SM_TCM_STATE_ACTIVE:
                /* Action */
                /* None */
            case XSTP_ENGINE_SM_TCM_STATE_PROGRESS_ACTIVE:
                /* Progress */
                if (    (    (pom_ptr->role != XSTP_ENGINE_PORTVAR_ROLE_ROOT)
                         &&  (pom_ptr->role != XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
                        )
                    ||  pom_ptr->common->oper_edge
                   )
                    next_status     = XSTP_ENGINE_SM_TCM_STATE_INACTIVE;
                else
                if (pom_ptr->rcvd_tcn)
                    next_status     = XSTP_ENGINE_SM_TCM_STATE_NOTIFIED_TCN;
                else
                if (pom_ptr->rcvd_tc)
                    next_status     = XSTP_ENGINE_SM_TCM_STATE_NOTIFIED_TC;
                else
                if (pom_ptr->tc_prop && !pom_ptr->common->oper_edge)
                    next_status     = XSTP_ENGINE_SM_TCM_STATE_PROPAGATING;
                else
                if (pom_ptr->rcvd_tc_ack)
                    next_status     = XSTP_ENGINE_SM_TCM_STATE_ACKNOWLEDGED;
                else
                {
                    next_status     = XSTP_ENGINE_SM_TCM_STATE_PROGRESS_ACTIVE;
                    sm_progress     = FALSE;
                }
                break;

            /* NOTIFIED_TCN */
            case XSTP_ENGINE_SM_TCM_STATE_NOTIFIED_TCN:
                /* Action */
                pom_ptr->tc_while   = XSTP_UTY_NewTcWhile(om_ptr, lport);
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_TCM_STATE_NOTIFIED_TC;
                break;

            /* NOTIFIED_TC */
            case XSTP_ENGINE_SM_TCM_STATE_NOTIFIED_TC:
                /* Action */
                pom_ptr->rcvd_tcn   = pom_ptr->rcvd_tc = FALSE;
                if (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
                    pom_ptr->tc_ack = TRUE;
                XSTP_UTY_SetTcPropBridge(om_ptr, lport);
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_TCM_STATE_ACTIVE;
                break;

            /* PROPAGATING */
            case XSTP_ENGINE_SM_TCM_STATE_PROPAGATING:
                /* Action */
                pom_ptr->tc_while   = XSTP_UTY_NewTcWhile(om_ptr, lport);
                XSTP_UTY_Flush(om_ptr, lport);
                pom_ptr->tc_prop    = FALSE;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_TCM_STATE_ACTIVE;
                break;

            /* ACKNOWLEDGED */
            case XSTP_ENGINE_SM_TCM_STATE_ACKNOWLEDGED:
                /* Action */
                pom_ptr->tc_while   = 0;
                pom_ptr->rcvd_tc_ack= FALSE;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_TCM_STATE_ACTIVE;
                break;

            default:
                break;
        } /* End of switch */
    } while (uct);

    pom_ptr->sms_tcm    = next_status;

    return (sm_progress || uct_progress);
} /* End of XSTP_ENGINE_StateMachineTCM */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_StateMachinePPM
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the state machine process
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport (1..max)
 * OUTPUT   : None
 * RETUEN   : TRUE if the state machine progresses, else FALSE
 * NOTES    : Ref to the description in 17.26, IEEE Std 802.1w-2001
 */
static  BOOL_T  XSTP_ENGINE_StateMachinePPM(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_BridgeVar_T     *bom_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    UI8_T                   next_status;
    UI8_T                   force_version;
    BOOL_T                  sm_progress, uct, uct_progress;

    bom_ptr         = &(om_ptr->bridge_info);
    pom_ptr         = &(om_ptr->port_info[lport-1]);
    next_status     = pom_ptr->sms_ppm;
    sm_progress     = TRUE;
    uct_progress    = FALSE;

    do
    {
        uct = FALSE;
        XSTP_ENGINE_DebugStateMachine(om_ptr->instance_id, lport, XSTP_ENGINE_DebugPPM, next_status);
        switch (next_status)
        {
            case XSTP_ENGINE_SM_PPM_STATE_INIT:
                /* Action */
                pom_ptr->common->init_pm    = TRUE;
                pom_ptr->common->mcheck     = FALSE;
            case XSTP_ENGINE_SM_PPM_STATE_PROGRESS_INIT:
                /* Progress */
                force_version = XSTP_OM_GetForceVersion();
                if (pom_ptr->common->port_enabled)
                {
                    if (force_version < 2)
                        next_status = XSTP_ENGINE_SM_PPM_STATE_SEND_STP;
                    else                                                    /* force_version >= 2  */
                        next_status = XSTP_ENGINE_SM_PPM_STATE_SEND_RSTP;
                }
                else                                                        /* (pom_ptr->port_enabled = FALSE)  */
                {
                    next_status = XSTP_ENGINE_SM_PPM_STATE_PROGRESS_INIT;
                    sm_progress = FALSE;
                }
                break;

            case XSTP_ENGINE_SM_PPM_STATE_SEND_RSTP:
                /* Action */
                pom_ptr->mdelay_while = bom_ptr->migrate_time;
                pom_ptr->common->mcheck = pom_ptr->common->init_pm = FALSE;
                pom_ptr->common->send_rstp = TRUE;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PPM_STATE_SENDING_RSTP;
                break;

            case XSTP_ENGINE_SM_PPM_STATE_SENDING_RSTP:
                /* Action */
                pom_ptr->rcvd_rstp  = pom_ptr->rcvd_stp = FALSE;
            case XSTP_ENGINE_SM_PPM_STATE_PROGRESS_SENDING_RSTP:
                /* Progress */
                if ( (pom_ptr->mdelay_while != 0) && (pom_ptr->rcvd_stp || pom_ptr->rcvd_rstp) )
                    next_status     = XSTP_ENGINE_SM_PPM_STATE_SENDING_RSTP;
                else
                if ( ((pom_ptr->mdelay_while == 0) && pom_ptr->rcvd_stp) || (XSTP_OM_GetForceVersion() < 2) )
                    next_status     = XSTP_ENGINE_SM_PPM_STATE_SEND_STP;
                else
                if (pom_ptr->common->mcheck)
                    next_status     = XSTP_ENGINE_SM_PPM_STATE_SEND_RSTP;
                else
                {
                    next_status     = XSTP_ENGINE_SM_PPM_STATE_PROGRESS_SENDING_RSTP;
                    sm_progress     = FALSE;
                }
                break;

            case XSTP_ENGINE_SM_PPM_STATE_SEND_STP:
                /* Action */
                pom_ptr->mdelay_while       = bom_ptr->migrate_time;
                pom_ptr->common->send_rstp  = FALSE;
                pom_ptr->common->init_pm    = FALSE;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PPM_STATE_SENDING_STP;
                break;

            case XSTP_ENGINE_SM_PPM_STATE_SENDING_STP:
                /* Action */
                pom_ptr->rcvd_rstp  = pom_ptr->rcvd_stp = FALSE;
            case XSTP_ENGINE_SM_PPM_STATE_PROGRESS_SENDING_STP:
                /* Progress */
                if ( (pom_ptr->mdelay_while != 0) && (pom_ptr->rcvd_stp || pom_ptr->rcvd_rstp) )
                    next_status     = XSTP_ENGINE_SM_PPM_STATE_SENDING_STP;
                else
                if ( (pom_ptr->mdelay_while == 0) && pom_ptr->rcvd_rstp )
                    next_status     = XSTP_ENGINE_SM_PPM_STATE_SEND_RSTP;
                else
                if (pom_ptr->common->mcheck)
                    next_status     = XSTP_ENGINE_SM_PPM_STATE_SEND_RSTP;
                else
                {
                    next_status     = XSTP_ENGINE_SM_PPM_STATE_PROGRESS_SENDING_STP;
                    sm_progress     = FALSE;
                }
                break;

            default:
                break;
        } /* End of switch */
    } while (uct);

    pom_ptr->sms_ppm    = next_status;

    return (sm_progress || uct_progress);
} /* End of XSTP_ENGINE_StateMachinePPM */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_StateMachinePTX
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the state machine process
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport (1..max)
 * OUTPUT   : None
 * RETUEN   : TRUE if the state machine progresses, else FALSE
 * NOTES    : Ref to the description in 17.27, IEEE Std 802.1w-2001
 */
static  BOOL_T  XSTP_ENGINE_StateMachinePTX(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_BridgeVar_T     *bom_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    UI8_T                   next_status;
    BOOL_T                  sm_progress, uct, uct_progress;

    bom_ptr         = &(om_ptr->bridge_info);
    pom_ptr         = &(om_ptr->port_info[lport-1]);
    next_status     = pom_ptr->sms_ptx;
    sm_progress     = TRUE;
    uct_progress    = FALSE;

    do
    {
        uct = FALSE;
        XSTP_ENGINE_DebugStateMachine(om_ptr->instance_id, lport, XSTP_ENGINE_DebugPTX, next_status);
        switch (next_status)
        {
            case XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_INIT:
                /* Action */
                pom_ptr->new_info   = FALSE;
                pom_ptr->hello_when = 0;
                pom_ptr->tx_count   = 0;

                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PTX_STATE_IDLE;
                break;

            case XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_PERIODIC:
                /* Action */
                pom_ptr->new_info   =   pom_ptr->new_info
                                     || (   (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
                                         || (   (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ROOT)
                                             && (pom_ptr->tc_while != 0)
                                            )
                                        );

                pom_ptr->hello_when = HelloTime;


                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PTX_STATE_IDLE;
                break;

            case XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_CONFIG:
                /* Action */
                pom_ptr->new_info   = FALSE;
                XSTP_UTY_TxConfig(om_ptr, lport);
                pom_ptr->tx_count++;
                pom_ptr->tc_ack     = FALSE;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PTX_STATE_IDLE;
                break;

            case XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_TCN:
                /* Action */
                pom_ptr->new_info   = FALSE;
                XSTP_UTY_TxTcn(om_ptr, lport);
                pom_ptr->tx_count++;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PTX_STATE_IDLE;
                break;

            case XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_RSTP:
                /* Action */
                pom_ptr->new_info   = FALSE;
                XSTP_UTY_TxRstp(om_ptr, lport);
                pom_ptr->tx_count++;
                pom_ptr->tc_ack     = FALSE;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PTX_STATE_IDLE;
                break;

            case XSTP_ENGINE_SM_PTX_STATE_IDLE:
                /* Action */
                /* None */
            case XSTP_ENGINE_SM_PTX_STATE_PROGRESS_IDLE:
                /* Progress */
                if (pom_ptr->selected && !pom_ptr->updt_info
#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
                    && (pom_ptr->common->root_inconsistent == FALSE)
#endif /* #if (SYS_CPNT_STP_ROOT_GUARD == TRUE) */
                   )
                {
                    if (pom_ptr->hello_when == 0)
                        next_status = XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_PERIODIC;
                    else
                    if (    pom_ptr->common->send_rstp
                        &&  pom_ptr->new_info
                        &&  (pom_ptr->tx_count < bom_ptr->common->tx_hold_count)
                        &&  (pom_ptr->hello_when != 0)
                        &&  (   (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
                             || (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ROOT)
                            )
                       )
                        next_status = XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_RSTP;
                    else
                    if (    !pom_ptr->common->send_rstp
                        &&  pom_ptr->new_info
                        &&  (pom_ptr->tx_count < bom_ptr->common->tx_hold_count)
                        &&  (pom_ptr->hello_when != 0)
                        &&  (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ROOT)
                       )
                        next_status = XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_TCN;
                    else
                    if (    !pom_ptr->common->send_rstp
                        &&  pom_ptr->new_info
                        &&  (pom_ptr->tx_count < bom_ptr->common->tx_hold_count)
                        &&  (pom_ptr->hello_when != 0)
                        &&  (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
                       )
                        next_status = XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_CONFIG;
                    else
                    {
                        next_status = XSTP_ENGINE_SM_PTX_STATE_PROGRESS_IDLE;
                        sm_progress = FALSE;
                    }
                }
                else
                {
                    next_status = XSTP_ENGINE_SM_PTX_STATE_PROGRESS_IDLE;
                    sm_progress = FALSE;
                }
                break;

            default:
                break;
        } /* End of switch */
    } while (uct);

    pom_ptr->sms_ptx    = next_status;

    return (sm_progress || uct_progress);
} /* End of XSTP_ENGINE_StateMachinePTX */

#endif /* XSTP_TYPE_PROTOCOL_RSTP */










/* ===================================================================== */
/* ===================================================================== */
/* IEEE 802.1S */
#ifdef XSTP_TYPE_PROTOCOL_MSTP

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_RestartStateMachine
 *-------------------------------------------------------------------------
 * PURPOSE  : State machine restart means read configuration again,
 *            not clear all the configuration parameters and use default.
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : Call the function when user change MST Configuration Indentifier.
 */
void XSTP_ENGINE_RestartStateMachine(void)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    UI32_T                  xstid;
    UI32_T                  lport;
    XSTP_OM_PortVar_T       *pom_ptr;

    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
    {
        BACKDOOR_MGR_Printf("\r\nXSTP_ENGINE_RestartStateMachine::Processing...");
    }
    xstid               = XSTP_TYPE_CISTID;
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    do
    {
        if (om_ptr->instance_exist)
        {
            if (XSTP_UTY_Cist(om_ptr))
            {
                /* 13.23.1 begin */
                om_ptr->bridge_info.common->begin               = TRUE;
            }
            /* State machine status */
            om_ptr->bridge_info.sms_prs                         = XSTP_ENGINE_SM_PRS_STATE_INIT_BRIDGE;

            /* Performance Improvement
            for (lport = 1; lport <= XSTP_TYPE_MAX_NUM_OF_LPORT; lport++)
            */
            lport = 0;
            while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
            {
                pom_ptr     = &(om_ptr->port_info[lport-1]);
                /* State machine status */
                pom_ptr->sms_pti                = XSTP_ENGINE_SM_PTI_STATE_ONE_SECOND;
                pom_ptr->sms_prx                = XSTP_ENGINE_SM_PRX_STATE_DISCARD;
                pom_ptr->sms_pim                = XSTP_ENGINE_SM_PIM_STATE_DISABLED;
                pom_ptr->sms_prt                = XSTP_ENGINE_SM_PRT_STATE_INIT_PORT;
                pom_ptr->sms_pst                = XSTP_ENGINE_SM_PST_STATE_DISCARDING;
                pom_ptr->sms_tcm                = XSTP_ENGINE_SM_TCM_STATE_INIT;
                pom_ptr->sms_ppm                = XSTP_ENGINE_SM_PPM_STATE_INIT;
                pom_ptr->sms_ptx                = XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_INIT;
            }
        }
    }while(XSTP_OM_GetNextInstanceInfoPtr(&xstid, &om_ptr));
    return;
} /* End of XSTP_ENGINE_RestartStateMachine */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_InitBridgeStateMachine
 *-------------------------------------------------------------------------
 * PURPOSE  : Initialize the bridge state machine variables
 * INPUT    : om_ptr    -- om pointer for this instance
 *            system_id_extension   -- System ID Extension (see 13.23.2)
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 */
static  void    XSTP_ENGINE_InitBridgeStateMachine(XSTP_OM_InstanceData_T *om_ptr, UI32_T system_id_extension)
{

    UI8_T                   mac_addr[6];
    XSTP_TYPE_BridgeId_T    bridge_id;
    XSTP_TYPE_BridgeId_T    null_bridge_id;
    XSTP_TYPE_PortId_T      zero_port_id;
    UI8_T                   remaining_hops;

    remaining_hops = (UI8_T) XSTP_OM_GetMaxHopCount();
    om_ptr->bridge_info.time_since_topology_change  = (UI32_T)SYSFUN_GetSysTick();
    om_ptr->bridge_info.topology_change_count       = 0;

    om_ptr->bridge_info.trap_flag_tc                = FALSE;
    om_ptr->bridge_info.trap_flag_new_root          = FALSE;

    /* bridge_id, zero_port_id */
    SWCTRL_GetCpuMac(mac_addr);

    if (!om_ptr->bridge_info.static_bridge_priority)
    {
        /* Bridge ID, System ID Extension, Bridge Address */
        XSTP_OM_BRIDGE_ID_FORMAT( bridge_id, XSTP_TYPE_DEFAULT_BRIDGE_PRIORITY, system_id_extension, mac_addr);

        /* bridge_identifier */
        memcpy(&om_ptr->bridge_info.bridge_identifier, &bridge_id, XSTP_TYPE_BRIDGE_ID_LENGTH);

        XSTP_OM_PORT_ID_FORMAT(zero_port_id, 0, 0);

        /* bridge_priority_vector */
        if (XSTP_UTY_Cist(om_ptr))
        {
            XSTP_OM_PRIORITY_VECTOR_FORMAT( om_ptr->bridge_info.bridge_priority, bridge_id, 0, bridge_id, 0, bridge_id, zero_port_id, zero_port_id);
        }
        else
        {
            memset(&null_bridge_id, 0, XSTP_TYPE_BRIDGE_ID_LENGTH);
            XSTP_OM_PRIORITY_VECTOR_FORMAT( om_ptr->bridge_info.bridge_priority, null_bridge_id, 0, bridge_id, 0, bridge_id, zero_port_id, zero_port_id);
        }
    }

    /* bridge_times */
    if (XSTP_UTY_Cist(om_ptr))
    {
        XSTP_OM_TIMERS_FORMAT(  om_ptr->bridge_info.bridge_times,
                                0,
                                XSTP_TYPE_DEFAULT_MAX_AGE,
                                XSTP_TYPE_DEFAULT_HELLO_TIME,
                                XSTP_TYPE_DEFAULT_FORWARD_DELAY,
                                remaining_hops);
    }
    else
    {
        XSTP_OM_TIMERS_FORMAT(  om_ptr->bridge_info.bridge_times,
                                0,
                                0,
                                0,
                                0,
                                remaining_hops);
    }
    /* root_port_id */
    memcpy(&om_ptr->bridge_info.root_port_id, &zero_port_id, XSTP_TYPE_PORT_ID_LENGTH);

    /* root_priority_vector */
    memcpy(&om_ptr->bridge_info.root_priority, &om_ptr->bridge_info.bridge_priority, sizeof(XSTP_TYPE_PriorityVector_T));

    /* root_times */
    memcpy(&om_ptr->bridge_info.root_times, &om_ptr->bridge_info.bridge_times, sizeof(XSTP_TYPE_Timers_T));

    om_ptr->bridge_info.sms_prs                     = XSTP_ENGINE_SM_PRS_STATE_INIT_BRIDGE;

    if (XSTP_UTY_Cist(om_ptr))
    {
        /* 13.22 (c) tx_hold_count, 13.22 (d) migrate_time */
        om_ptr->bridge_info.common->tx_hold_count       = XSTP_TYPE_DEFAULT_TX_HOLD_COUNT;
        om_ptr->bridge_info.common->migrate_time        = XSTP_TYPE_DEFAULT_MIGRATE_TIME;
        /* 13.23.1 begin */
        om_ptr->bridge_info.common->begin               = TRUE;
    }
    return;
} /* End of XSTP_ENGINE_InitBridgeStateMachine */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_InitPortStateMachines
 *-------------------------------------------------------------------------
 * PURPOSE  : Initialize the port state machine variables
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- logical port number
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 */
void    XSTP_ENGINE_InitPortStateMachines(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    UI32_T  path_cost;
    XSTP_OM_PortVar_T       *pom_ptr;
    XSTP_TYPE_BridgeId_T    bridge_id;
    XSTP_TYPE_PortId_T      my_port_id;
    UI8_T                   remaining_hops;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    remaining_hops = (UI8_T) XSTP_OM_GetMaxHopCount();

    if (!pom_ptr->static_internal_path_cost)
    {
        /* 13.22 (h) internal_port_path_cost */
        XSTP_OM_GetLportDefaultPathCost(lport, &path_cost);
        pom_ptr->internal_port_path_cost          = path_cost;
    }

    if (!pom_ptr->static_port_priority)
    {
        XSTP_OM_PORT_ID_FORMAT(my_port_id, XSTP_TYPE_DEFAULT_PORT_PRIORITY, lport);
        /* 13.24 (ae) port_id */
        memcpy(&pom_ptr->port_id, &my_port_id, XSTP_TYPE_PORT_ID_LENGTH);
    }

    /* State machine status */
    pom_ptr->sms_pti                = XSTP_ENGINE_SM_PTI_STATE_ONE_SECOND;
    pom_ptr->sms_prx                = XSTP_ENGINE_SM_PRX_STATE_DISCARD;
    pom_ptr->sms_pim                = XSTP_ENGINE_SM_PIM_STATE_DISABLED;
    pom_ptr->sms_prt                = XSTP_ENGINE_SM_PRT_STATE_INIT_PORT;
    pom_ptr->sms_pst                = XSTP_ENGINE_SM_PST_STATE_DISCARDING;
    pom_ptr->sms_tcm                = XSTP_ENGINE_SM_TCM_STATE_INIT;
    pom_ptr->sms_ppm                = XSTP_ENGINE_SM_PPM_STATE_INIT;
    pom_ptr->sms_ptx                = XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_INIT;
    /* 802.1D-2004 17.25 */
    if (pom_ptr->common->admin_edge == TRUE)
        pom_ptr->sms_bdm                = XSTP_ENGINE_SM_BDM_STATE_EDGE;
    else
        pom_ptr->sms_bdm                = XSTP_ENGINE_SM_BDM_STATE_NOT_EDGE;

    /* 13.27 State machine timers */
    pom_ptr->fd_while               = 0;
    pom_ptr->rb_while               = 0;
    pom_ptr->rcvd_info_while        = 0;
    pom_ptr->rr_while               = 0;
    pom_ptr->tc_while               = 0;

    /* 13.24 (ak)(aq) designated_priority, 13.24 (al)(ar) designated_times */
    if (pom_ptr->cist == NULL)  /* CIST */
    {
        XSTP_OM_PRIORITY_VECTOR_FORMAT( pom_ptr->designated_priority,
                                        om_ptr->bridge_info.bridge_identifier,
                                        0,
                                        om_ptr->bridge_info.bridge_identifier,
                                        0,
                                        om_ptr->bridge_info.bridge_identifier,
                                        pom_ptr->port_id,
                                        pom_ptr->port_id);

        XSTP_OM_TIMERS_FORMAT(  pom_ptr->designated_times,
                                0,
                                om_ptr->bridge_info.bridge_times.max_age,
                                om_ptr->bridge_info.bridge_times.hello_time,
                                om_ptr->bridge_info.bridge_times.forward_delay,
                                remaining_hops);
    }
    else /* MSTI */
    {
        memset(&bridge_id, 0, XSTP_TYPE_BRIDGE_ID_LENGTH);
        XSTP_OM_PRIORITY_VECTOR_FORMAT( pom_ptr->designated_priority,
                                        bridge_id,
                                        0,
                                        om_ptr->bridge_info.bridge_identifier,
                                        0,
                                        om_ptr->bridge_info.bridge_identifier,
                                        pom_ptr->port_id,
                                        pom_ptr->port_id);

        XSTP_OM_TIMERS_FORMAT(  pom_ptr->designated_times,
                                0,
                                0,
                                0,
                                0,
                                remaining_hops);
    }

    /* 13.24 (am)(as) msg_priority */
    memcpy(&pom_ptr->msg_priority, &pom_ptr->designated_priority, sizeof(XSTP_TYPE_PriorityVector_T));

    /* 13.24 (an)(at) msg_times */
    memcpy(&pom_ptr->msg_times, &pom_ptr->designated_times, sizeof(XSTP_TYPE_Timers_T));

    /* 13.24 (ao)(au) port_priority */
    memcpy(&pom_ptr->port_priority, &pom_ptr->designated_priority, sizeof(XSTP_TYPE_PriorityVector_T));

    /* 13.24 (ap)(av) port_times */
    memcpy(&pom_ptr->port_times, &pom_ptr->designated_times, sizeof(XSTP_TYPE_Timers_T));

    if (XSTP_UTY_Cist(om_ptr))
    {
        if (!pom_ptr->common->static_external_path_cost)
        {
            pom_ptr->common->external_port_path_cost    = path_cost;
        }
        pom_ptr->common->hello_when     = 0;
        pom_ptr->common->mdelay_while   = 0;
        pom_ptr->common->timeout_mdelay_while       = FALSE;
        pom_ptr->common->timeout_hello_when         = FALSE;

        /* 13.24(a) tick, 13.24(b) tx_count, */
        pom_ptr->common->tick           = FALSE;
        pom_ptr->common->tx_count       = 0;

        /*13.24(c) oper_edge
        Init. the oper_edge value in XSTP_OM_InitEdgePortAndLinkType().
        It should not be cleared on disabling the stp or port except in initializing phase.
        pom_ptr->common->oper_edge      = FALSE;
        */

        /* 13.24(d), 17.18.15 port_enabled
        * No condition except the following 3 conditions changes the port_enabled variable:
        * 1. port admin_state changed by users
        * 2. set port_enabled=FALSE when enter_transaction_state
        * 3. set port_enabled=TRUE if the port exists when enter_master_mode
        */
        /*
        pom_ptr->common->port_enabled     = FALSE;
        */

        /* 13.24 (e) info_internal, 13.24 (f) new_info_cist,    13.24 (g) new_info_msti,    13.24 (h) rcvd_internal */
        /* 13.24 (i) init_pm,       13.24 (j) rcvd_bpdu,        13.24 (k) rcvd_rstp,        13.24 (l) rcvd_stp      */
        /* 13.24 (m) rcvd_tc_ack,   13.24 (n) rcvd_tcn,         13.24 (o) send_rstp,        13.24 (p) tc_ack        */
        /* mcheck */
        pom_ptr->common->info_internal  = FALSE;
        pom_ptr->common->new_info_cist  = FALSE;
        pom_ptr->common->new_info_msti  = FALSE;
        pom_ptr->common->rcvd_internal  = FALSE;
        pom_ptr->common->init_pm        = FALSE;
        pom_ptr->common->rcvd_bpdu      = FALSE;
        pom_ptr->common->rcvd_rstp      = FALSE;
        pom_ptr->common->rcvd_stp       = FALSE;
        pom_ptr->common->rcvd_tc_ack    = FALSE;
        pom_ptr->common->rcvd_tcn       = FALSE;
        pom_ptr->common->send_rstp      = FALSE;
        pom_ptr->common->tc_ack         = FALSE;
        pom_ptr->common->mcheck         = FALSE;
#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
        pom_ptr->common->root_inconsistent  = FALSE;
#endif
    }

    /* 13.24 (q) forward,           13.24 (r) forwarding,       13.24 (s) info_is       */
    /* 13.24 (t) learn,             13.24 (u) learning,         13.24 (v) proposed      */
    /* 13.24 (w) proposing,         13.24 (x) rcvd_tc,          13.24 (y) re_root       */
    /* 13.24 (z) reselect                                                               */
    /* 13.24 (aa) selected,         13.24 (ab) tc_prop,         13.24 (ac) updt_info    */
    /* 13.24 (ad) agreed,           13.24 (af) rcvd_info        13.24 (ag) role         */
    /* 13.24 (ah) selected_role,    13.24 (ai) sync,            13.24 (aj) synced       */
    /* 13.24 (aw) agree,            13.24 (ax) changed_master,  13.24 (ay) rcvd_msg     */
    /* 13.24.13 msti_master,        13.24.14 msti_mastered                              */
    pom_ptr->forward                = FALSE;
    pom_ptr->forwarding             = FALSE;
    pom_ptr->info_is                = XSTP_ENGINE_PORTVAR_INFO_IS_DISABLED;
    pom_ptr->learn                  = FALSE;
    pom_ptr->learning               = FALSE;
    pom_ptr->proposed               = FALSE;
    pom_ptr->proposing              = FALSE;
    pom_ptr->rcvd_tc                = FALSE;
    pom_ptr->re_root                = FALSE;
    pom_ptr->reselect               = FALSE;
    pom_ptr->selected               = FALSE;
    pom_ptr->tc_prop                = FALSE;
    pom_ptr->updt_info              = FALSE;
    pom_ptr->agreed                 = FALSE;
    pom_ptr->rcvd_info              = XSTP_ENGINE_PORTVAR_RCVD_INFO_OTHER_INFO;
    pom_ptr->role                   = XSTP_ENGINE_PORTVAR_ROLE_DISABLED;
    pom_ptr->selected_role          = XSTP_ENGINE_PORTVAR_ROLE_DISABLED;
    pom_ptr->sync                   = FALSE;
    pom_ptr->synced                 = FALSE;
    pom_ptr->agree                  = FALSE;
    pom_ptr->changed_master         = FALSE;
    pom_ptr->rcvd_msg               = FALSE;
    pom_ptr->msti_master            = FALSE;
    pom_ptr->msti_mastered          = FALSE;
    pom_ptr->disputed               = FALSE;
    return;
} /* End of XSTP_ENGINE_InitPortStateMachines */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_StateMachinePTI
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the state machine process
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport (1..max)
 * OUTPUT   : None
 * RETUEN   : TRUE if any of the timers is expired, else FALSE
 * NOTES    : Ref to the description in 13.27, IEEE Std 802.1s(D14.1)-2002
 */
static  BOOL_T  XSTP_ENGINE_StateMachinePTI(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI8_T               next_status;
    BOOL_T              result;
    BOOL_T              timeout;
    BOOL_T              sm_progress;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    next_status = pom_ptr->sms_pti;
    timeout     = FALSE;

    do
    {
        sm_progress = FALSE;
        switch (next_status)
        {
            case XSTP_ENGINE_SM_PTI_STATE_TICK:
                if (XSTP_UTY_Cist(om_ptr))
                {
                    pom_ptr->common->timeout_hello_when = XSTP_UTY_DecreaseTimer(&pom_ptr->common->hello_when);
                }
                result  = pom_ptr->common->timeout_hello_when;
                timeout = timeout || result;
                result  = XSTP_UTY_DecreaseTimer(&pom_ptr->tc_while);
                timeout = timeout || result;
                result  = XSTP_UTY_DecreaseTimer(&pom_ptr->fd_while);
                timeout = timeout || result;
                result  = XSTP_UTY_DecreaseTimer(&pom_ptr->rcvd_info_while);
                timeout = timeout || result;
                result  = XSTP_UTY_DecreaseTimer(&pom_ptr->rr_while);
                timeout = timeout || result;
                result  = XSTP_UTY_DecreaseTimer(&pom_ptr->rb_while);
                timeout = timeout || result;
                if (XSTP_UTY_Cist(om_ptr))
                {
                    result  = XSTP_UTY_DecreaseTimer(&pom_ptr->common->tx_count);
                    /* It triggers nothing when this timer becomes zero */
                    /*
                    timeout = timeout || result;
                    */

                    pom_ptr->common->timeout_mdelay_while   = XSTP_UTY_DecreaseTimer(&pom_ptr->common->mdelay_while);

                    result  = XSTP_UTY_DecreaseTimer(&pom_ptr->common->edge_delay_while);
                    timeout = timeout || result;

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
                    if (pom_ptr->common->bpdu_guard_auto_recovery == XSTP_TYPE_PORT_BPDU_GUARD_AUTO_RECOVERY_ENABLED)
                    {
                        result = XSTP_UTY_DecreaseTimer32(&pom_ptr->common->bpdu_guard_auto_recovery_while);
                        if (result == TRUE)
                        {
                            XSTP_UTY_SetBpduGuardRecoverPortList(lport);
                        }
                        timeout = timeout || result;
                    }
#endif /* #if (SYS_CPNT_STP_BPDU_GUARD == TRUE) */
                }
                result  = pom_ptr->common->timeout_mdelay_while;
                timeout = timeout || result;
                /* Progress */
                /* UCT */
                next_status     = XSTP_ENGINE_SM_PTI_STATE_ONE_SECOND;

            case XSTP_ENGINE_SM_PTI_STATE_ONE_SECOND:
                    pom_ptr->common->tick   = FALSE;

            case XSTP_ENGINE_SM_PTI_STATE_PROGRESS_ONE_SECOND:
                /* Progress */
                if (pom_ptr->common->tick == TRUE)
                {
                    sm_progress = TRUE;
                    next_status = XSTP_ENGINE_SM_PTI_STATE_TICK;
                }
                else
                    next_status = XSTP_ENGINE_SM_PTI_STATE_PROGRESS_ONE_SECOND;
                break;

            default:
                break;
        }
    } while (sm_progress);

    pom_ptr->sms_pti    = next_status;

    return timeout;
} /* End of XSTP_ENGINE_StateMachinePTI */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_StateMachinePRX
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the state machine process
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport (1..max)
 * OUTPUT   : None
 * RETUEN   : TRUE if the state machine progresses, else FALSE
 * NOTES    : Ref to the description in 13.28, IEEE Std 802.1s(D14.1)-2002
 */
static  BOOL_T  XSTP_ENGINE_StateMachinePRX(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T       *pom_ptr;
    UI8_T                   next_status;
    BOOL_T                  sm_progress, uct, uct_progress;

    pom_ptr         = &(om_ptr->port_info[lport-1]);
    next_status     = pom_ptr->sms_prx;
    sm_progress     = TRUE;
    uct_progress    = FALSE;

    do
    {
        uct = FALSE;
        XSTP_ENGINE_DebugStateMachine(om_ptr->instance_id, lport, XSTP_ENGINE_DebugPRX, next_status);
        switch (next_status)
        {
            /* DISCARD */
            case XSTP_ENGINE_SM_PRX_STATE_DISCARD:
                /* Action */
                pom_ptr->common->rcvd_bpdu  = pom_ptr->common->rcvd_rstp
                                            = pom_ptr->common->rcvd_stp
                                            = FALSE;
                XSTP_UTY_ClearAllRcvdMsgs();                    /* 13.26.3 */
                pom_ptr->common->edge_delay_while = SYS_DFLT_STP_MIGRATE_TIME;              /*802.1D-2004 17.23 */
            case XSTP_ENGINE_SM_PRX_STATE_PROGRESS_DISCARD:
                /* Progress */
                if (pom_ptr->common->rcvd_bpdu)
                {
                    if (!(pom_ptr->common->port_enabled))
                        next_status = XSTP_ENGINE_SM_PRX_STATE_DISCARD;
                    else
                    if (!XSTP_UTY_RcvdAnyMsg(lport))            /* 13.25.7 */
                        next_status = XSTP_ENGINE_SM_PRX_STATE_RECEIVE;
                    else
                    {
                        next_status = XSTP_ENGINE_SM_PRX_STATE_PROGRESS_DISCARD;
                        sm_progress = FALSE;
                    }
                }
                else
                {
                    next_status = XSTP_ENGINE_SM_PRX_STATE_PROGRESS_DISCARD;
                    sm_progress = FALSE;
                }
                break;

            /* RECEIVE */
            case XSTP_ENGINE_SM_PRX_STATE_RECEIVE:
                /* Action */
                XSTP_UTY_UpdtBpduVersion(om_ptr, lport);                                    /* 13.26 (g), 17.19.18 */
                /* pom_ptr->common->rcvd_internal = XSTP_UTY_FromSameRegion(om_ptr, lport);*/    /* 13.26.5 */
                pom_ptr->common->rcvd_internal = XSTP_UTY_FromSameRegionAndUpdateReselect(om_ptr, lport);
                XSTP_UTY_SetRcvdMsgs(om_ptr, lport);                                        /* 13.26.15 */
                XSTP_UTY_SetTcFlags(om_ptr, lport);                                         /* 13.26.19 */
                pom_ptr->common->rcvd_bpdu = FALSE;
                pom_ptr->common->edge_delay_while = SYS_DFLT_STP_MIGRATE_TIME;              /*802.1D-2004 17.23 */
            case XSTP_ENGINE_SM_PRX_STATE_PROGRESS_RECEIVE:
                /* Progress */
                if (    (!pom_ptr->common->port_enabled)
                     && (    (pom_ptr->common->rcvd_bpdu)
                          || (pom_ptr->common->edge_delay_while != SYS_DFLT_STP_MIGRATE_TIME)
                        )
                   )
                {
                    next_status = XSTP_ENGINE_SM_PRX_STATE_DISCARD;
                }
                else
                if (    pom_ptr->common->port_enabled
                     && pom_ptr->common->rcvd_bpdu
                     && (!XSTP_UTY_RcvdAnyMsg(lport))
                   )
                {
                    next_status = XSTP_ENGINE_SM_PRX_STATE_RECEIVE;
                }
                else
                {
                    next_status = XSTP_ENGINE_SM_PRX_STATE_PROGRESS_RECEIVE;
                    sm_progress = FALSE;
                }
                break;

            default:
                break;
        } /* End of switch */
    } while (uct);

    pom_ptr->sms_prx    = next_status;

    return (sm_progress || uct_progress);
} /* End of XSTP_ENGINE_StateMachinePRX */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_StateMachinePPM
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the state machine process
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport (1..max)
 * OUTPUT   : None
 * RETUEN   : TRUE if the state machine progresses, else FALSE
 * NOTES    : Ref to the description in 13.29, IEEE Std 802.1s(D14.1)-2002
 */
static  BOOL_T  XSTP_ENGINE_StateMachinePPM(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_BridgeVar_T     *bom_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    UI8_T                   next_status;
    UI8_T                   force_version;
    BOOL_T                  sm_progress, uct, uct_progress;

    bom_ptr         = &(om_ptr->bridge_info);
    pom_ptr         = &(om_ptr->port_info[lport-1]);
    next_status     = pom_ptr->sms_ppm;
    sm_progress     = TRUE;
    uct_progress    = FALSE;

    do
    {
        uct = FALSE;
        XSTP_ENGINE_DebugStateMachine(om_ptr->instance_id, lport, XSTP_ENGINE_DebugPPM, next_status);
        switch (next_status)
        {
            case XSTP_ENGINE_SM_PPM_STATE_INIT:
                /* Action */
                pom_ptr->common->init_pm    = TRUE;
                pom_ptr->common->mcheck     = FALSE;
            case XSTP_ENGINE_SM_PPM_STATE_PROGRESS_INIT:
                /* Progress */
                force_version = XSTP_OM_GetForceVersion();
                if (pom_ptr->common->port_enabled)
                {
                    if (force_version < 2)
                        next_status = XSTP_ENGINE_SM_PPM_STATE_SEND_STP;
                    else                                                    /* force_version >= 2  */
                        next_status = XSTP_ENGINE_SM_PPM_STATE_SEND_RSTP;
                }
                else                                                        /* (pom_ptr->common->port_enabled = FALSE)  */
                {
                    next_status = XSTP_ENGINE_SM_PPM_STATE_PROGRESS_INIT;
                    sm_progress = FALSE;
                }
                break;

            case XSTP_ENGINE_SM_PPM_STATE_SEND_RSTP:
                /* Action */
                pom_ptr->common->mdelay_while   = bom_ptr->common->migrate_time;
                pom_ptr->common->mcheck         = pom_ptr->common->init_pm
                                                = FALSE;
                pom_ptr->common->send_rstp      = TRUE;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PPM_STATE_SENDING_RSTP;
                break;

            case XSTP_ENGINE_SM_PPM_STATE_SENDING_RSTP:
                /* Action */
                pom_ptr->common->rcvd_rstp      = pom_ptr->common->rcvd_stp
                                                = FALSE;
            case XSTP_ENGINE_SM_PPM_STATE_PROGRESS_SENDING_RSTP:
                /* Progress */
                if (    (pom_ptr->common->mdelay_while != 0)
                    &&  (pom_ptr->common->rcvd_stp || pom_ptr->common->rcvd_rstp)
                   )
                    next_status     = XSTP_ENGINE_SM_PPM_STATE_SENDING_RSTP;
                else
                if (    (   (pom_ptr->common->mdelay_while == 0)
                         && pom_ptr->common->rcvd_stp
                        )
                    ||  (XSTP_OM_GetForceVersion() < 2)
                   )
                    next_status     = XSTP_ENGINE_SM_PPM_STATE_SEND_STP;
                else
                if (pom_ptr->common->mcheck)
                    next_status     = XSTP_ENGINE_SM_PPM_STATE_SEND_RSTP;
                else
                {
                    next_status     = XSTP_ENGINE_SM_PPM_STATE_PROGRESS_SENDING_RSTP;
                    sm_progress     = FALSE;
                }
                break;

            case XSTP_ENGINE_SM_PPM_STATE_SEND_STP:
                /* Action */
                pom_ptr->common->mdelay_while   = bom_ptr->common->migrate_time;
                pom_ptr->common->send_rstp      = FALSE;
                pom_ptr->common->init_pm        = FALSE;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PPM_STATE_SENDING_STP;
                break;

            case XSTP_ENGINE_SM_PPM_STATE_SENDING_STP:
                /* Action */
                pom_ptr->common->rcvd_rstp  = pom_ptr->common->rcvd_stp
                                            = FALSE;
            case XSTP_ENGINE_SM_PPM_STATE_PROGRESS_SENDING_STP:
                /* Progress */
                if (    (pom_ptr->common->mdelay_while != 0)
                    &&  (pom_ptr->common->rcvd_stp || pom_ptr->common->rcvd_rstp)
                   )
                    next_status     = XSTP_ENGINE_SM_PPM_STATE_SENDING_STP;
                else
                if ( (pom_ptr->common->mdelay_while == 0) && pom_ptr->common->rcvd_rstp )
                    next_status     = XSTP_ENGINE_SM_PPM_STATE_SEND_RSTP;
                else
                if (pom_ptr->common->mcheck)
                    next_status     = XSTP_ENGINE_SM_PPM_STATE_SEND_RSTP;
                else
                {
                    next_status     = XSTP_ENGINE_SM_PPM_STATE_PROGRESS_SENDING_STP;
                    sm_progress     = FALSE;
                }
                break;

            default:
                break;
        } /* End of switch */
    } while (uct);

    pom_ptr->sms_ppm    = next_status;

    return (sm_progress || uct_progress);
} /* End of XSTP_ENGINE_StateMachinePPM */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_StateMachinePTX
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the state machine process
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport (1..max)
 * OUTPUT   : None
 * RETUEN   : TRUE if the state machine progresses, else FALSE
 * NOTES    : Ref to the description in 13.30, IEEE Std 802.1s(D14.1)-2002
 */
static  BOOL_T  XSTP_ENGINE_StateMachinePTX(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_BridgeVar_T     *bom_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    UI8_T                   next_status;
    BOOL_T                  sm_progress, uct, uct_progress;

    bom_ptr         = &(om_ptr->bridge_info);
    pom_ptr         = &(om_ptr->port_info[lport-1]);
    next_status     = pom_ptr->sms_ptx;
    sm_progress     = TRUE;
    uct_progress    = FALSE;

    do
    {
        uct = FALSE;
        XSTP_ENGINE_DebugStateMachine(om_ptr->instance_id, lport, XSTP_ENGINE_DebugPTX, next_status);
        switch (next_status)
        {
            case XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_INIT:
                /* Action */
                pom_ptr->common->new_info_cist  = pom_ptr->common->new_info_msti
                                                = FALSE;
                pom_ptr->common->hello_when     = 0;
                pom_ptr->common->tx_count       = 0;

                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PTX_STATE_IDLE;
                break;

            case XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_PERIODIC:
                /* Action */
                pom_ptr->common->new_info_cist  =   pom_ptr->common->new_info_cist
                                                    || (   (   (pom_ptr->tc_while != 0)
                                                            && (XSTP_UTY_CistRootPort(om_ptr, lport))   /* 13.25.4 */
                                                           )
                                                        || (XSTP_UTY_CistDesignatedPort(om_ptr, lport)) /* 13.25.3 */
                                                       );
                pom_ptr->common->new_info_msti  =   pom_ptr->common->new_info_msti
                                                    || (   XSTP_UTY_MstiDesignatedPort(lport)           /* 13.25.6 */
                                                        || (   XSTP_UTY_MstiRootPort(lport)             /* 13.25.5 */
                                                            && (!XSTP_UTY_MstiTcWhileIsZero(lport))
                                                           )
                                                       );
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PTX_STATE_IDLE;
                break;

            case XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_CONFIG:
                /* Action */
                pom_ptr->common->new_info_cist  = pom_ptr->common->new_info_msti
                                                = FALSE;
                XSTP_UTY_TxConfig(om_ptr, lport);       /* 13.26.21 */
                pom_ptr->common->tx_count++;
                pom_ptr->common->tc_ack         = FALSE;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PTX_STATE_IDLE;
                break;

            case XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_TCN:
                /* Action */
                pom_ptr->common->new_info_cist  = pom_ptr->common->new_info_msti
                                                = FALSE;
                XSTP_UTY_TxTcn(om_ptr, lport);          /* 13.26 (a) the same as 1w */
                pom_ptr->common->tx_count++;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PTX_STATE_IDLE;
                break;

            case XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_RSTP:
                /* Action */
                pom_ptr->common->new_info_cist  = pom_ptr->common->new_info_msti
                                                = FALSE;
                XSTP_UTY_TxMstp(om_ptr, lport);                             /* 13.26.22 */
                pom_ptr->common->tx_count++;
                pom_ptr->common->tc_ack         = FALSE;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PTX_STATE_IDLE;
                break;

            case XSTP_ENGINE_SM_PTX_STATE_IDLE:
                /* Action */
                pom_ptr->common->hello_when = XSTP_UTY_GetHelloTime(om_ptr, lport); /* 802.1Q-2005 Figure 13-12 p.181 */
            case XSTP_ENGINE_SM_PTX_STATE_PROGRESS_IDLE:
                /* Progress */
                if (pom_ptr->selected && !(pom_ptr->updt_info)
#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
                    && (pom_ptr->common->root_inconsistent == FALSE)
#endif /* #if (SYS_CPNT_STP_ROOT_GUARD == TRUE) */
                   )
                {
                    if (pom_ptr->common->hello_when == 0)
                        next_status = XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_PERIODIC;
                    else
                    if  (   pom_ptr->common->send_rstp
                         && (   pom_ptr->common->new_info_cist
                             || (   pom_ptr->common->new_info_msti
                                 && (pom_ptr->selected_role != XSTP_ENGINE_PORTVAR_ROLE_MASTER) /* 802.1Q-2005 Figure 13-12 p.181 */
                                )
                            )
                         && (pom_ptr->common->tx_count < bom_ptr->common->tx_hold_count)
                         && (pom_ptr->common->hello_when != 0)
                        )
                        next_status = XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_RSTP;
                    else
                    if (    !pom_ptr->common->send_rstp
                        &&  pom_ptr->common->new_info_cist
                        &&  (pom_ptr->common->tx_count < bom_ptr->common->tx_hold_count)
                        &&  (pom_ptr->common->hello_when != 0)
                        &&  XSTP_UTY_CistRootPort(om_ptr, lport)                    /* 13.25.3 */
                       )
                        next_status = XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_TCN;
                    else
                    if (    !pom_ptr->common->send_rstp
                        &&  pom_ptr->common->new_info_cist
                        &&  (pom_ptr->common->tx_count < bom_ptr->common->tx_hold_count)
                        &&  (pom_ptr->common->hello_when != 0)
                        &&  XSTP_UTY_CistDesignatedPort(om_ptr, lport)              /* 13.25.4 */
                       )
                        next_status = XSTP_ENGINE_SM_PTX_STATE_TRANSMIT_CONFIG;
                    else
                    {
                        next_status = XSTP_ENGINE_SM_PTX_STATE_PROGRESS_IDLE;
                        sm_progress = FALSE;
                    }
                }
                else
                {
                    next_status = XSTP_ENGINE_SM_PTX_STATE_PROGRESS_IDLE;
                    sm_progress = FALSE;
                }
                break;

            default:
                break;
        } /* End of switch */
    } while (uct);

    pom_ptr->sms_ptx    = next_status;

    return (sm_progress || uct_progress);
} /* End of XSTP_ENGINE_StateMachinePTX */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_StateMachinePIM
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the state machine process
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport (1..max)
 * OUTPUT   : None
 * RETUEN   : TRUE if the state machine progresses, else FALSE
 * NOTES    : Ref to the description in 13.31, IEEE Std 802.1s-2002
 */
static  BOOL_T  XSTP_ENGINE_StateMachinePIM(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI8_T               next_status;
    BOOL_T              sm_progress, uct, uct_progress;

    pom_ptr         = &(om_ptr->port_info[lport-1]);
    next_status     = pom_ptr->sms_pim;
    sm_progress     = TRUE;
    uct_progress    = FALSE;


    do
    {
        uct = FALSE;
        XSTP_ENGINE_DebugStateMachine(om_ptr->instance_id, lport, XSTP_ENGINE_DebugPIM, next_status);
        switch (next_status)
        {
            /* DISABLED */
            case XSTP_ENGINE_SM_PIM_STATE_DISABLED:
                /* Action */
                pom_ptr->rcvd_msg           = FALSE;
                pom_ptr->proposing          = pom_ptr->proposed
                                            = pom_ptr->agree
                                            = pom_ptr->agreed
                                            = FALSE;
                pom_ptr->rcvd_info_while    = 0;
                pom_ptr->info_is            = XSTP_ENGINE_PORTVAR_INFO_IS_DISABLED;
                pom_ptr->reselect           = TRUE;
                pom_ptr->selected           = FALSE;
            case XSTP_ENGINE_SM_PIM_STATE_PROGRESS_DISABLED:
                /* Progress */
                if (pom_ptr->rcvd_msg)
                    next_status = XSTP_ENGINE_SM_PIM_STATE_DISABLED;
                else
                if (pom_ptr->common->port_enabled && pom_ptr->common->link_up)
                    next_status = XSTP_ENGINE_SM_PIM_STATE_AGED;
                else
                {
                    next_status = XSTP_ENGINE_SM_PIM_STATE_PROGRESS_DISABLED;
                    sm_progress = FALSE;
                }
                break;

            /* AGED */
            case XSTP_ENGINE_SM_PIM_STATE_AGED:
                /* Action */
                pom_ptr->info_is    = XSTP_ENGINE_PORTVAR_INFO_IS_AGED;
                pom_ptr->reselect   = TRUE;
                pom_ptr->selected   = FALSE;
#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
                if (pom_ptr->common->root_guard_status == TRUE)
                {
                    pom_ptr->common->root_inconsistent = FALSE;
                }
#endif /* #if (SYS_CPNT_STP_ROOT_GUARD == TRUE) */
            case XSTP_ENGINE_SM_PIM_STATE_PROGRESS_AGED:
                /* Progress */
                if (pom_ptr->selected && pom_ptr->updt_info)
                    next_status = XSTP_ENGINE_SM_PIM_STATE_UPDATE;
                else
                {
                    next_status = XSTP_ENGINE_SM_PIM_STATE_PROGRESS_AGED;
                    sm_progress = FALSE;
                }
                break;

            /* UPDATE */
            case XSTP_ENGINE_SM_PIM_STATE_UPDATE:
                /* Action */
                pom_ptr->proposing      = pom_ptr->proposed = FALSE;
                /*pom_ptr->synced         = FALSE;*/                                              /* +++ Follow 1D/D3 Figure 17-18, need to sync code to 1w part */
                pom_ptr->sync           = pom_ptr->changed_master;

                /*
                The following two lines are added by ETS: Paul Diamond at 08/16/2004
                They are used to resolve the temporary looping issue found by Paul Moskal
                */
                if (pom_ptr->info_is != XSTP_ENGINE_PORTVAR_INFO_IS_MINE)
                     pom_ptr->agreed = FALSE;

                pom_ptr->agreed         =   (   pom_ptr->agreed
                                             && XSTP_UTY_BetterOrSameInfoXst(om_ptr, lport)       /* XXX, 13.26.1, 13.26.2 */
                                             && !(pom_ptr->changed_master)
                                            );
                pom_ptr->synced         = pom_ptr->synced && pom_ptr->agreed;                     /* +++ Follow 1D/D3 Figure 17-18, need to sync code to 1w part */
                memcpy(&pom_ptr->port_priority, &pom_ptr->designated_priority,  sizeof(XSTP_TYPE_PriorityVector_T) );
                memcpy(&pom_ptr->port_times,    &pom_ptr->designated_times,     sizeof(XSTP_TYPE_Timers_T) );
                /*
                xstPortPriority         = xstDesignatedPriority;
                xstPortTimes            = xstDesignatedTimes;
                */
                pom_ptr->changed_master = pom_ptr->updt_info = FALSE;
                pom_ptr->info_is        = XSTP_ENGINE_PORTVAR_INFO_IS_MINE;

                /* newInfoXst = TRUE, determine that it is newInfoCist or newInfoMsti*/
                if (pom_ptr->cist == NULL)
                    pom_ptr->common->new_info_cist     = TRUE;
                else
                    pom_ptr->common->new_info_msti     = TRUE;

                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status = XSTP_ENGINE_SM_PIM_STATE_CURRENT;
                break;

            /* CURRENT */
            case XSTP_ENGINE_SM_PIM_STATE_CURRENT:
                /* Action */
                /* None */
            case XSTP_ENGINE_SM_PIM_STATE_PROGRESS_CURRENT:
                /* Progress */
                if (pom_ptr->selected && pom_ptr->updt_info)
                    next_status     = XSTP_ENGINE_SM_PIM_STATE_UPDATE;
                else
                if (   (   (pom_ptr->info_is == XSTP_ENGINE_PORTVAR_INFO_IS_RECEIVED)
                        && (pom_ptr->rcvd_info_while == 0)
                        && (!(pom_ptr->updt_info))
                        && (!XSTP_UTY_RcvdXstInfo(om_ptr, lport)) /* rcvdXstInfo, 13.25.8, 13.25.9 */
                       )
#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
                    || (pom_ptr->common->root_inconsistent == TRUE)
#endif /* #if (SYS_CPNT_STP_ROOT_GUARD == TRUE) */
                   )
                    next_status     = XSTP_ENGINE_SM_PIM_STATE_AGED;
                else
                if ( pom_ptr->rcvd_msg && !XSTP_UTY_UpdtXstInfo(om_ptr, lport) )  /* rcvdXstMsg, updtXstInfo, 13.25.11, 13.25.12 */
                    next_status     = XSTP_ENGINE_SM_PIM_STATE_RECEIVE;
                else
                {
                    next_status     = XSTP_ENGINE_SM_PIM_STATE_PROGRESS_CURRENT;
                    sm_progress     = FALSE;
                }
                break;

            /* SUPERIOR_DESIGNATED */
            case XSTP_ENGINE_SM_PIM_STATE_SUPERIOR_DESIGNATED:
                /* Action */
#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
                if (   (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
                    && (pom_ptr->common->root_guard_status == TRUE)
                   )
                {
                    pom_ptr->common->root_inconsistent = TRUE;
                    pom_ptr->rcvd_msg = FALSE;
                    pom_ptr->agreed = FALSE; /* prevent it enter LEARN state at PRT */
                    pom_ptr->fd_while = XSTP_UTY_GetFwdDelay(om_ptr, lport);
                }
                else
#endif /* #if (SYS_CPNT_STP_ROOT_GUARD == TRUE) */
                {
                    pom_ptr->common->info_internal  = pom_ptr->common->rcvd_internal;
                    pom_ptr->proposing              = FALSE;
                    XSTP_UTY_RecordProposalXst(om_ptr, lport);                                       /* 13.26.13, 13.26.14 */
                    pom_ptr->agree                  = pom_ptr->agree && XSTP_UTY_BetterOrSameInfoXst(om_ptr, lport);    /* betterorsameXstInfo()*/
                    XSTP_UTY_RecordAgreementXst(om_ptr, lport);                                      /* 13.26.9, 13.26.10 */
                    pom_ptr->synced         = pom_ptr->synced && pom_ptr->agreed;
                    memcpy(&pom_ptr->port_priority, &pom_ptr->msg_priority,  sizeof(XSTP_TYPE_PriorityVector_T) );
                    memcpy(&pom_ptr->port_times,    &pom_ptr->msg_times,     sizeof(XSTP_TYPE_Timers_T) );
                    XSTP_UTY_UpdtRcvdInfoWhileXst(om_ptr, lport);                   /* updtRcvdInfoWhile();*/
                    pom_ptr->info_is        = XSTP_ENGINE_PORTVAR_INFO_IS_RECEIVED;
                    pom_ptr->reselect       = TRUE;
                    pom_ptr->selected       = FALSE;
                    pom_ptr->rcvd_msg       = FALSE;
                }

                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PIM_STATE_CURRENT;
                break;

            /* REPEATED_DESIGNATED */
            case XSTP_ENGINE_SM_PIM_STATE_REPEATED_DESIGNATED:
                /* Action */
                pom_ptr->common->info_internal  = pom_ptr->common->rcvd_internal;
                XSTP_UTY_RecordProposalXst(om_ptr, lport);                          /* XXX 13.26.13, 13.26.14*/
                XSTP_UTY_RecordAgreementXst(om_ptr, lport);                         /* XXX 13.26.9, 13.26.10*/
                XSTP_UTY_UpdtRcvdInfoWhileXst(om_ptr, lport);                       /* XXX updtRcvdInfoWhile();13.26.23, 13.26.24*/
                pom_ptr->rcvd_msg       = FALSE;

                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PIM_STATE_CURRENT;
                break;

            /* ROOT */
            case XSTP_ENGINE_SM_PIM_STATE_ROOT:
                /* Action */
                XSTP_UTY_RecordAgreementXst(om_ptr, lport);                     /* XXX 13.26.9, 13.26.10 */
                pom_ptr->rcvd_msg    = FALSE;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PIM_STATE_CURRENT;
                break;

            /* OTHER */
            case XSTP_ENGINE_SM_PIM_STATE_OTHER:
                /* Action */
                pom_ptr->rcvd_msg    = FALSE;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PIM_STATE_CURRENT;
                break;

            /* RECEIVE */
            case XSTP_ENGINE_SM_PIM_STATE_RECEIVE:
                /* Action */
                pom_ptr->rcvd_info  = XSTP_UTY_RcvInfoXst(om_ptr, lport);     /* rcvInfoXst, 13.26.7, 13.26.8 */
                XSTP_UTY_RecordMasteredXst(om_ptr, lport);                    /* recordMasteredXst(), 13.26.11, 13.26.12*/
            case XSTP_ENGINE_SM_PIM_STATE_PROGRESS_RECEIVE:
                /* Progress */
                if (pom_ptr->rcvd_info == XSTP_ENGINE_PORTVAR_RCVD_INFO_SUPERIOR_DESIGNATED_INFO)
                    next_status = XSTP_ENGINE_SM_PIM_STATE_SUPERIOR_DESIGNATED;
                else
                if (pom_ptr->rcvd_info == XSTP_ENGINE_PORTVAR_RCVD_INFO_REPEATED_DESIGNATED_INFO)
                    next_status = XSTP_ENGINE_SM_PIM_STATE_REPEATED_DESIGNATED;
                else
                if (pom_ptr->rcvd_info == XSTP_ENGINE_PORTVAR_RCVD_INFO_ROOT_INFO)
                    next_status = XSTP_ENGINE_SM_PIM_STATE_ROOT;
                else
                if (pom_ptr->rcvd_info == XSTP_ENGINE_PORTVAR_RCVD_INFO_OTHER_INFO)
                    next_status = XSTP_ENGINE_SM_PIM_STATE_OTHER;
                else
                {
                    next_status = XSTP_ENGINE_SM_PIM_STATE_OTHER;
                    sm_progress = FALSE;
                }
                break;

            default:
                break;
        } /* End of switch */
    } while (uct);

    pom_ptr->sms_pim    = next_status;

    return (sm_progress || uct_progress);
} /* End of XSTP_ENGINE_StateMachinePIM */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_StateMachinePRS
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the state machine process
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETUEN   : TRUE if the state machine progresses, else FALSE
 * NOTES    : Ref to the description in 13.32, IEEE Std 802.1s(D14.1)-2002
 */
static  BOOL_T  XSTP_ENGINE_StateMachinePRS(XSTP_OM_InstanceData_T *om_ptr)
{
    XSTP_OM_BridgeVar_T *bom_ptr;
    UI8_T               next_status;
    BOOL_T              sm_progress, uct_progress;

    bom_ptr         = &(om_ptr->bridge_info);
    next_status     = bom_ptr->sms_prs;
    sm_progress     = TRUE;
    uct_progress    = FALSE;

    XSTP_ENGINE_DebugStateMachine(om_ptr->instance_id, 0, XSTP_ENGINE_DebugPRS, next_status);
    switch (next_status)
    {
        /* INIT_BRIDGE */
        case XSTP_ENGINE_SM_PRS_STATE_INIT_BRIDGE:
            /* Action */
            XSTP_UTY_UpdtRoleDisabledTree(om_ptr);                 /*xxx XSTP_UTY_UpdtRoleDisabledBridge(om_ptr)*/
            /* Progress */
            /* UCT */
            next_status     = XSTP_ENGINE_SM_PRS_STATE_RECEIVE;
            uct_progress    = TRUE;

        /* RECEIVE */
        case XSTP_ENGINE_SM_PRS_STATE_RECEIVE:
            /* Action */
            XSTP_UTY_ClearReselectTree(om_ptr);     /* xxx XSTP_UTY_ClearReselectBridge(om_ptr) */
            XSTP_UTY_UpdtRolesXst(om_ptr);          /* xxx XSTP_UTY_UpdtRolesBridge(om_ptr) */
            XSTP_UTY_SetSelectedTree(om_ptr);       /* xxx XSTP_UTY_SetSelectedBridge(om_ptr) */
        case XSTP_ENGINE_SM_PRS_STATE_PROGRESS_RECEIVE:
            /* Progress */
            if ( XSTP_UTY_ReselectForAnyPort(om_ptr)) /*the same as 1w */
                next_status = XSTP_ENGINE_SM_PRS_STATE_RECEIVE;
            else
            {
                next_status = XSTP_ENGINE_SM_PRS_STATE_PROGRESS_RECEIVE;
                sm_progress = FALSE;
            }
            break;

        default:
            break;
    }

    bom_ptr->sms_prs   = next_status;

    return (sm_progress || uct_progress);
} /* End of XSTP_ENGINE_StateMachinePRS */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_StateMachinePRT
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the state machine process
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport (1..max)
 * OUTPUT   : None
 * RETUEN   : TRUE if the state machine progresses, else FALSE
 * NOTES    : Ref to the description in 13.33, IEEE Std 802.1s(D14.1)-2002
 */
static  BOOL_T  XSTP_ENGINE_StateMachinePRT(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI8_T               next_status;
    BOOL_T              sm_progress, uct, uct_progress;

    pom_ptr         = &(om_ptr->port_info[lport-1]);
    next_status     = pom_ptr->sms_prt;
    sm_progress     = TRUE;
    uct_progress    = FALSE;

    do
    {
        uct = FALSE;

        if (pom_ptr->role != pom_ptr->selected_role)
        {
            if (    (pom_ptr->selected)
                 && (!pom_ptr->updt_info)
               )
            {
                if (    (pom_ptr->selected_role == XSTP_ENGINE_PORTVAR_ROLE_DISABLED)
                    ||  (pom_ptr->selected_role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
                    ||  (pom_ptr->selected_role == XSTP_ENGINE_PORTVAR_ROLE_BACKUP)
                   )
                {
                    next_status = XSTP_ENGINE_SM_PRT_STATE_BLOCK_PORT;
                }
                else
                if (    (pom_ptr->selected_role == XSTP_ENGINE_PORTVAR_ROLE_ROOT)
                    ||  (pom_ptr->selected_role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
                    ||  (pom_ptr->selected_role == XSTP_ENGINE_PORTVAR_ROLE_MASTER)
                   )
                {
                    next_status = XSTP_ENGINE_SM_PRT_STATE_ACTIVE_PORT;
                }
                else
                {
                    /* Error */
                    uct_progress    = FALSE;
                    sm_progress     = FALSE;
                    break;
                }
            }
            else
            {
                /* Don't continue the following operation because it is not in stable state */
                uct_progress    = FALSE;
                sm_progress     = FALSE;
                break;
            }
        } /* End of if (role != selectedRole) */

        XSTP_ENGINE_DebugStateMachine(om_ptr->instance_id, lport, XSTP_ENGINE_DebugPRT, next_status);
        switch (next_status)
        {
            /* Disabled, alternate, and backup role */
            /* INIT_PORT */
            case XSTP_ENGINE_SM_PRT_STATE_INIT_PORT:
                /* Action */
                pom_ptr->role       = pom_ptr->selected_role = XSTP_ENGINE_PORTVAR_ROLE_DISABLED;
                pom_ptr->reselect   = TRUE;
                pom_ptr->synced     = FALSE;
                pom_ptr->sync       = pom_ptr->re_root  = TRUE;
                pom_ptr->rr_while   = pom_ptr->fd_while = XSTP_UTY_GetFwdDelay(om_ptr, lport);
                pom_ptr->rb_while   = 0;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_BLOCK_PORT;
                break;

            /* BLOCK_PORT */
            case XSTP_ENGINE_SM_PRT_STATE_BLOCK_PORT:
                /* Action */
                pom_ptr->role       = pom_ptr->selected_role;
                pom_ptr->learn      = pom_ptr->forward = FALSE;
            case XSTP_ENGINE_SM_PRT_STATE_PROGRESS_BLOCK_PORT:
                /* Progress */
                if (    pom_ptr->selected
                    &&  !pom_ptr->updt_info
                    &&  !pom_ptr->learning
                    &&  !pom_ptr->forwarding
                   )
                    next_status = XSTP_ENGINE_SM_PRT_STATE_BLOCKED_PORT;
                else
                {
                    next_status     = XSTP_ENGINE_SM_PRT_STATE_PROGRESS_BLOCK_PORT;
                    sm_progress     = FALSE;
                }
                break;

            /* BACKUP_PORT */
            case XSTP_ENGINE_SM_PRT_STATE_BACKUP_PORT:
                /* Action */
                pom_ptr->rb_while   = 2 * XSTP_UTY_GetHelloTime(om_ptr, lport);
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_BLOCKED_PORT;
                break;

            /* ALTERNATE_PROPOSED */
            case XSTP_ENGINE_SM_PRT_STATE_ALTERNATE_PROPOSED: /* 802.1Q-2005, Fig.13-19 */
                /* Action */
                XSTP_UTY_SetSyncTree(om_ptr);
                pom_ptr->proposed   = FALSE;
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_BLOCKED_PORT;
                break;

            /* ALTERNATE_AGREED */
            case XSTP_ENGINE_SM_PRT_STATE_ALTERNATE_AGREED: /* 802.1Q-2005, Fig.13-19 */
                /* Action */
                pom_ptr->proposed   = FALSE;
                pom_ptr->agree      = TRUE;
                /* newInfoXst = TRUE, determine that it is newInfoCist or newInfoMsti*/
                if (pom_ptr->cist == NULL)
                    pom_ptr->common->new_info_cist     = TRUE;
                else
                    pom_ptr->common->new_info_msti     = TRUE;
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_BLOCKED_PORT;
                break;

            /* BLOCKED_PORT */
            case XSTP_ENGINE_SM_PRT_STATE_BLOCKED_PORT:
                /* Action */
                pom_ptr->fd_while   = XSTP_UTY_GetFwdDelay(om_ptr, lport);
                pom_ptr->synced     = TRUE;
                pom_ptr->rr_while   = 0;
                pom_ptr->sync = pom_ptr->re_root = FALSE;
            case XSTP_ENGINE_SM_PRT_STATE_PROGRESS_BLOCKED_PORT:
                /* Progress */
                if (pom_ptr->selected && !pom_ptr->updt_info)
                {
                    if (    (pom_ptr->fd_while != XSTP_UTY_GetFwdDelay(om_ptr, lport))
                        ||  pom_ptr->sync
                        ||  pom_ptr->re_root
                        ||  !pom_ptr->synced
                       )
                        next_status = XSTP_ENGINE_SM_PRT_STATE_BLOCKED_PORT;
                    else
                    if (    (pom_ptr->rb_while != (2*XSTP_UTY_GetHelloTime(om_ptr, lport)))
                        &&  (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_BACKUP)
                       )
                        next_status = XSTP_ENGINE_SM_PRT_STATE_BACKUP_PORT;
                    else
                    if (    (   (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
                             || (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_BACKUP)
                            )
                        &&  (pom_ptr->proposed && !pom_ptr->agree)
                       )
                        next_status = XSTP_ENGINE_SM_PRT_STATE_ALTERNATE_PROPOSED;
                    else
                    if (    (   (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
                             || (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_BACKUP)
                            )
                        &&  (   (pom_ptr->proposed && pom_ptr->agree)
                             || (XSTP_UTY_AllSynced(om_ptr, lport) && !pom_ptr->agree)
                            )
                       )
                        next_status = XSTP_ENGINE_SM_PRT_STATE_ALTERNATE_AGREED;
                    else
                    {
                        next_status = XSTP_ENGINE_SM_PRT_STATE_PROGRESS_BLOCKED_PORT;
                        sm_progress = FALSE;
                    }
                }
                else
                {
                    next_status     = XSTP_ENGINE_SM_PRT_STATE_PROGRESS_BLOCKED_PORT;
                    sm_progress     = FALSE;
                }
                break;

            /* root and designated role */
            /* PROPOSED */
            case XSTP_ENGINE_SM_PRT_STATE_PROPOSED:
                /* Action */
                XSTP_UTY_SetSyncTree(om_ptr);          /*xxx XSTP_UTY_SetSyncBridge(om_ptr); */
                pom_ptr->proposed   = FALSE;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_ACTIVE_PORT;
                break;

            /* PROPOSING */
            case XSTP_ENGINE_SM_PRT_STATE_PROPOSING:
                /* Action */
                pom_ptr->proposing  = TRUE;
                /* newInfoXst = TRUE, determine that it is newInfoCist or newInfoMsti*/
                if (pom_ptr->cist == NULL)
                    pom_ptr->common->new_info_cist     = TRUE;
                else
                    pom_ptr->common->new_info_msti     = TRUE;
                pom_ptr->common->edge_delay_while = XSTP_UTY_NewEdgeDelayWhile(om_ptr, lport); /* 802.1D-2004 17.29.3 */
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_ACTIVE_PORT;
                break;

            /* AGREES */
            case XSTP_ENGINE_SM_PRT_STATE_AGREES:
                /* Action */
                pom_ptr->proposed   = pom_ptr->sync = FALSE; /* Follow 1D/D3 Figure 17-21 */
                pom_ptr->agree      = TRUE;
                /* newInfoXst = TRUE, determine that it is newInfoCist or newInfoMsti*/
                if (pom_ptr->cist == NULL)
                    pom_ptr->common->new_info_cist     = pom_ptr->common->send_rstp;  /* patch. The new_info value only set when tc_while is on for root port connected to 1D switch */
                else
                    pom_ptr->common->new_info_msti     = pom_ptr->common->send_rstp;  /* patch. The new_info value only set when tc_while is on for root port connected to 1D switch */

                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_ACTIVE_PORT;
                break;

            /* SYNCED */
            case XSTP_ENGINE_SM_PRT_STATE_SYNCED:
                /* Action */
                if (pom_ptr->role != XSTP_ENGINE_PORTVAR_ROLE_ROOT)
                    pom_ptr->rr_while   = 0;
                pom_ptr->synced     = TRUE;
                pom_ptr->sync       = FALSE;

                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_ACTIVE_PORT;
                break;

            /* REROOT */
            case XSTP_ENGINE_SM_PRT_STATE_REROOT:
                /* Action */
                XSTP_UTY_SetReRootTree(om_ptr);    /* XXX XSTP_UTY_SetReRootBridge(om_ptr); */
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_ACTIVE_PORT;
                break;

            /* FORWARD */
            case XSTP_ENGINE_SM_PRT_STATE_FORWARD:
                /* Action */
                pom_ptr->forward    = TRUE;
                pom_ptr->fd_while   = 0;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_ACTIVE_PORT;
                break;

            /* LEARN */
            case XSTP_ENGINE_SM_PRT_STATE_LEARN:
                /* Action */
                pom_ptr->learn      = TRUE;
                pom_ptr->fd_while   = XSTP_UTY_GetFwdDelay(om_ptr, lport);
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_ACTIVE_PORT;
                break;

            /* LISTEN */
            case XSTP_ENGINE_SM_PRT_STATE_LISTEN:
                /* Action */
                pom_ptr->learn      = pom_ptr->forward
                                    = pom_ptr->disputed
                                    = FALSE;
                pom_ptr->fd_while   = XSTP_UTY_GetFwdDelay(om_ptr, lport);
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_ACTIVE_PORT;
                break;

            /* REROOTED */
            case XSTP_ENGINE_SM_PRT_STATE_REROOTED:
                /* Action */
                pom_ptr->re_root    = FALSE;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_ACTIVE_PORT;
                break;

            /* ROOT */
            case XSTP_ENGINE_SM_PRT_STATE_ROOT:
                /* Action */
                pom_ptr->rr_while   = XSTP_UTY_GetFwdDelay(om_ptr, lport);
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_PRT_STATE_ACTIVE_PORT;
                break;

            /* ACTIVE_PORT */
            case XSTP_ENGINE_SM_PRT_STATE_ACTIVE_PORT:
                /* Action */
                pom_ptr->role       = pom_ptr->selected_role;

            case XSTP_ENGINE_SM_PRT_STATE_PROGRESS_ACTIVE_PORT:
                /* Progress */
                if (pom_ptr->selected && !pom_ptr->updt_info)
                {
                    if (pom_ptr->proposed && !pom_ptr->agree)
                        next_status = XSTP_ENGINE_SM_PRT_STATE_PROPOSED;
                    else
                    if (    !pom_ptr->forward
                        &&  !pom_ptr->agreed
                        &&  !pom_ptr->proposing
                        &&  !pom_ptr->common->oper_edge
                        &&  (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
                       )
                        next_status = XSTP_ENGINE_SM_PRT_STATE_PROPOSING;
                    else
                    if (   (pom_ptr->proposed && pom_ptr->agree)
                        || (XSTP_UTY_AllSynced(om_ptr, lport) && !pom_ptr->agree)  /* Follow 1D/D3 Figure 17-21 */
                       )
                        next_status = XSTP_ENGINE_SM_PRT_STATE_AGREES;
                    else
                    if (    (   !pom_ptr->learning
                             && !pom_ptr->forwarding
                             && !pom_ptr->synced
                             && (pom_ptr->role != XSTP_ENGINE_PORTVAR_ROLE_ROOT)
                            )
                        ||  (pom_ptr->agreed    && !pom_ptr->synced)
                        ||  (pom_ptr->common->oper_edge && !pom_ptr->synced)
                        ||  (pom_ptr->sync      && pom_ptr->synced)
                       )
                        next_status = XSTP_ENGINE_SM_PRT_STATE_SYNCED;
                    else
                    if (    !(pom_ptr->forward)
                        &&  !(pom_ptr->re_root)
                        &&  (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ROOT)
                       )
                        next_status = XSTP_ENGINE_SM_PRT_STATE_REROOT;
                    /* Make LISTEN state be prior to FORWARD, LEARN state to ensure when LISTEN state condition is met.
                       state machine will enter LISTEN first. 2007.1
                     */
                    else
                    if (  (pom_ptr->learn || pom_ptr->forward)
                        && !pom_ptr->common->oper_edge
                        && (pom_ptr->role != XSTP_ENGINE_PORTVAR_ROLE_ROOT)
                        && (  (pom_ptr->sync && !pom_ptr->synced)
                            ||(   pom_ptr->re_root
                               && (pom_ptr->rr_while != 0)
                              )
                            ||(pom_ptr->common->loopback_block)
                            ||(pom_ptr->disputed)
#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
                            ||(pom_ptr->common->root_inconsistent == TRUE)
#endif /* #if (SYS_CPNT_STP_ROOT_GUARD == TRUE) */
                           )
                       )
                        next_status = XSTP_ENGINE_SM_PRT_STATE_LISTEN;
                    else
                    if (    pom_ptr->learn
                        &&  !pom_ptr->forward
                        &&  (   (pom_ptr->fd_while == 0)
                             || (   (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ROOT)
                                 && (   (pom_ptr->rb_while == 0)
                                     && XSTP_UTY_ReRooted(om_ptr, lport)       /*xxx the same as 1w XSTP_UTY_ReRootedForOthers*/
                                    )
                                 && (XSTP_OM_GetForceVersion() >= 2)
                                )
                             || (   pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED
                                 && (   pom_ptr->agreed
                                     || pom_ptr->common->oper_edge
                                    )
                                 && (   (pom_ptr->rr_while == 0)
                                     || !pom_ptr->re_root
                                    )
                                 && !pom_ptr->sync
                                )
                             || (   pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_MASTER
                                 && XSTP_UTY_AllSynced(om_ptr, lport)
                                )
                            )
                       )
                        next_status = XSTP_ENGINE_SM_PRT_STATE_FORWARD;
                    else
                    if (    !pom_ptr->learn
                        &&  (   (pom_ptr->fd_while == 0)
                             || (   (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ROOT)
                                 && (   (pom_ptr->rb_while == 0)
                                     && XSTP_UTY_ReRooted(om_ptr, lport)       /*xxx the same as 1w XSTP_UTY_ReRootedForOthers*/
                                    )
                                 && (XSTP_OM_GetForceVersion() >= 2)
                                )
                             || (   (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
                                 && (   pom_ptr->agreed
                                     || pom_ptr->common->oper_edge
                                    )
                                 && (   (pom_ptr->rr_while == 0)
                                     || !pom_ptr->re_root
                                    )
                                 && !pom_ptr->sync
                                )
                             || (   (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_MASTER)
                                 && XSTP_UTY_AllSynced(om_ptr, lport)
                                )
                            )
                       )
                        next_status = XSTP_ENGINE_SM_PRT_STATE_LEARN;
                    else
                    if (    pom_ptr->re_root
                        &&  (   (   (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ROOT)
                                 && pom_ptr->forward
                                )
                             || (   (   (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
                                     || (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_MASTER)
                                    )
                                 && (pom_ptr->rr_while == 0)
                                )
                            )
                       )
                        next_status = XSTP_ENGINE_SM_PRT_STATE_REROOTED;
                    else
                    if (    (pom_ptr->rr_while != XSTP_UTY_GetFwdDelay(om_ptr, lport))
                        &&  (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ROOT)
                       )
                        next_status = XSTP_ENGINE_SM_PRT_STATE_ROOT;
                    else
                    {
                        next_status = XSTP_ENGINE_SM_PRT_STATE_PROGRESS_ACTIVE_PORT;
                        sm_progress = FALSE;
                    }
                }
                else
                {
                    next_status = XSTP_ENGINE_SM_PRT_STATE_PROGRESS_ACTIVE_PORT;
                    sm_progress = FALSE;
                }
                break;

            default:
                break;
        } /* End of switch */
    } while (uct);

    pom_ptr->sms_prt    = next_status;

    return (sm_progress || uct_progress);
} /* End of XSTP_ENGINE_StateMachinePRT */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_StateMachinePST
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the state machine process
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport (1..max)
 * OUTPUT   : None
 * RETUEN   : TRUE if the state machine progresses, else FALSE
 * NOTES    : Ref to the description in 13.34, IEEE Std 802.1s(D14.1)-2002
 */
static  BOOL_T  XSTP_ENGINE_StateMachinePST(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI8_T               next_status;
    BOOL_T              sm_progress, uct, uct_progress;

    pom_ptr = &(om_ptr->port_info[lport-1]);
    next_status     = pom_ptr->sms_pst;
    sm_progress     = TRUE;
    uct_progress    = FALSE;

    do
    {
        uct = FALSE;
        XSTP_ENGINE_DebugStateMachine(om_ptr->instance_id, lport, XSTP_ENGINE_DebugPST, next_status);
        switch (next_status)
        {
            /* DISCARDING */
            case XSTP_ENGINE_SM_PST_STATE_DISCARDING:
                /* Action */
                XSTP_UTY_DisableLearning(om_ptr, lport);        /*xxx the same as 1w */
                pom_ptr->learning   = FALSE;
                XSTP_UTY_DisableForwarding(om_ptr, lport);      /*xxx the same as 1w */
                pom_ptr->forwarding = FALSE;
            case XSTP_ENGINE_SM_PST_STATE_PROGRESS_DISCARDING:
                /* Progress */
                if (pom_ptr->learn)
                    next_status = XSTP_ENGINE_SM_PST_STATE_LEARNING;
                else
                {
                    next_status     = XSTP_ENGINE_SM_PST_STATE_PROGRESS_DISCARDING;
                    sm_progress     = FALSE;
                }
                break;

            /* LEARNING */
            case XSTP_ENGINE_SM_PST_STATE_LEARNING:
                /* Action */
                XSTP_UTY_EnableLearning(om_ptr, lport);     /*xxx the same as 1w */
                pom_ptr->learning   = TRUE;
            case XSTP_ENGINE_SM_PST_STATE_PROGRESS_LEARNING:
                /* Progress */
                if (pom_ptr->forward)
                    next_status     = XSTP_ENGINE_SM_PST_STATE_FORWARDING;
                else
                if (!pom_ptr->learn)
                    next_status     = XSTP_ENGINE_SM_PST_STATE_DISCARDING;
                else
                {
                    next_status     = XSTP_ENGINE_SM_PST_STATE_PROGRESS_LEARNING;
                    sm_progress     = FALSE;
                }
                break;

            /* FORWARDING */
            case XSTP_ENGINE_SM_PST_STATE_FORWARDING:
                /* Action */
                XSTP_UTY_EnableForwarding(om_ptr, lport);   /*xxx the same as 1w */
                pom_ptr->forwarding = TRUE;
            case XSTP_ENGINE_SM_PST_STATE_PROGRESS_FORWARDING:
                /* Progress */
                if (!pom_ptr->forward)
                    next_status     = XSTP_ENGINE_SM_PST_STATE_DISCARDING;
                else
                {
                    next_status     = XSTP_ENGINE_SM_PST_STATE_PROGRESS_FORWARDING;
                    sm_progress     = FALSE;
                }
                break;

            default:
                break;
        } /* End of switch */
    } while (uct);

    pom_ptr->sms_pst    = next_status;

    return (sm_progress || uct_progress);
} /* End of XSTP_ENGINE_StateMachinePST */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_StateMachineTCM
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the state machine process
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport (1..max)
 * OUTPUT   : None
 * RETUEN   : TRUE if the state machine progresses, else FALSE
 * NOTES    : Ref to the description in 13.35, IEEE Std 802.1s(D14.1)-2002
 */
static  BOOL_T  XSTP_ENGINE_StateMachineTCM(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI8_T               next_status;
    BOOL_T              sm_progress, uct, uct_progress;

    pom_ptr = &(om_ptr->port_info[lport-1]);
    next_status     = pom_ptr->sms_tcm;
    sm_progress     = TRUE;
    uct_progress    = FALSE;

    do
    {
        uct = FALSE;
        XSTP_ENGINE_DebugStateMachine(om_ptr->instance_id, lport, XSTP_ENGINE_DebugTCM, next_status);
        switch (next_status)
        {
            /* INIT */
            case XSTP_ENGINE_SM_TCM_STATE_INIT:
                /* Action */
                XSTP_UTY_Flush(om_ptr, lport);
                pom_ptr->tc_while   = 0;
                if (XSTP_UTY_Cist(om_ptr))                       /* 13.25.2 */
                    pom_ptr->common->tc_ack     = FALSE;
            case XSTP_ENGINE_SM_TCM_STATE_PROGRESS_INIT:        /* Follow IEEE P802.1D/D3, page 171, Figure 17-25 TCM machine */
                /* Progress */
                if (pom_ptr->learn)
                    next_status         = XSTP_ENGINE_SM_TCM_STATE_INACTIVE;
                else
                {
                    next_status     = XSTP_ENGINE_SM_TCM_STATE_PROGRESS_INIT;
                    sm_progress     = FALSE;
                }
                break;

            /* INACTIVE */
            case XSTP_ENGINE_SM_TCM_STATE_INACTIVE:
                /* Action */
                if (XSTP_UTY_Cist(om_ptr))                       /* 13.25.2 */
                {
                    pom_ptr->rcvd_tc    = pom_ptr->common->rcvd_tcn
                                        = pom_ptr->common->rcvd_tc_ack
                                        = FALSE;
                }
                pom_ptr->rcvd_tc        = pom_ptr->tc_prop
                                        = FALSE;
            case XSTP_ENGINE_SM_TCM_STATE_PROGRESS_INACTIVE:
                /* Progress */
                if (    pom_ptr->rcvd_tc
                    ||  (pom_ptr->common->rcvd_tcn && XSTP_UTY_Cist(om_ptr))
                    ||  (pom_ptr->common->rcvd_tc_ack && XSTP_UTY_Cist(om_ptr))
                    ||  pom_ptr->tc_prop
                   )
                    next_status = XSTP_ENGINE_SM_TCM_STATE_INACTIVE;
                else
                if (    (pom_ptr->role != XSTP_ENGINE_PORTVAR_ROLE_ROOT)
                    &&  (pom_ptr->role != XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
                    &&  (pom_ptr->role != XSTP_ENGINE_PORTVAR_ROLE_MASTER)
                    &&  (!(     pom_ptr->learn
                            ||  pom_ptr->learning
                          )
                        )
                    &&  (!(     pom_ptr->rcvd_tc
                            ||  (pom_ptr->common->rcvd_tcn && XSTP_UTY_Cist(om_ptr))
                            ||  (pom_ptr->common->rcvd_tc_ack && XSTP_UTY_Cist(om_ptr))
                            ||  pom_ptr->tc_prop
                           )
                         )
                   )
                    next_status     = XSTP_ENGINE_SM_TCM_STATE_INIT;
                else
                if (    (    (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ROOT)
                         ||  (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
                         ||  (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_MASTER)
                        )
                    &&  pom_ptr->forward
                    &&  !(pom_ptr->common->oper_edge)
                    && pom_ptr->common->link_up /*for auto edge patch, because port role will not change immediate when port link down, but BDE will cuase port ot no-oper-edge*/
                   )
                    next_status     = XSTP_ENGINE_SM_TCM_STATE_DETECTED;
                else
                {
                    next_status     = XSTP_ENGINE_SM_TCM_STATE_PROGRESS_INACTIVE;
                    sm_progress     = FALSE;
                }
                break;

            /* DETECTED */
            case XSTP_ENGINE_SM_TCM_STATE_DETECTED:
                /* Action */
                pom_ptr->tc_while   = XSTP_UTY_NewTcWhile(om_ptr, lport); /* 13.26.6 */
                /*move from XSTP_UTY_IncTopologyChangeCount to here,
                  so that it only triger on rx port but not all prop port*/
                XSTP_OM_SetTrapFlagTc(TRUE);
                XSTP_OM_SetTcCausePort(lport);

                /*for IGMPSnooping
                 *send msg to mucast .the parameters are is_root, port,tc_timer.
                 *1.the tc change notification occurs at two place ,
                 *    a. port state change from inactive to active
                 *    b. port that has rec tc
                 */
                SYS_CALLBACK_MGR_LportTcChangeCallback(SYS_MODULE_XSTP,((XSTP_TYPE_MSTP_MODE==XSTP_OM_GetForceVersion())? TRUE:FALSE),om_ptr->instance_id,lport,XSTP_UTY_RootBridge(om_ptr),pom_ptr->tc_while,om_ptr->instance_vlans_mapped);

                XSTP_UTY_SetTcPropTree(om_ptr, lport);                    /* 13.26.20 */
                /* Set newInfo=TRUE according to 1D/D3 Figure 17-25 DETECTED state of Topology Change state machine */
                /* Determine that it is newInfoCist or newInfoMsti*/
                if (pom_ptr->cist == NULL)
                    pom_ptr->common->new_info_cist     = TRUE;
                else
                    pom_ptr->common->new_info_msti     = TRUE;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_TCM_STATE_ACTIVE;
                break;

            /* ACTIVE */
            case XSTP_ENGINE_SM_TCM_STATE_ACTIVE:
                /* Action */
                /* None */
            case XSTP_ENGINE_SM_TCM_STATE_PROGRESS_ACTIVE:
                /* Progress */
                if (    (    (pom_ptr->role != XSTP_ENGINE_PORTVAR_ROLE_ROOT)
                         &&  (pom_ptr->role != XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
                         &&  (pom_ptr->role != XSTP_ENGINE_PORTVAR_ROLE_MASTER)
                        )
                    ||  pom_ptr->common->oper_edge
                   )
                    next_status     = XSTP_ENGINE_SM_TCM_STATE_INACTIVE;
                else
                if (XSTP_UTY_Cist(om_ptr) && (pom_ptr->common->rcvd_tcn))
                    next_status     = XSTP_ENGINE_SM_TCM_STATE_NOTIFIED_TCN;
                else
                if (pom_ptr->rcvd_tc)
                    next_status     = XSTP_ENGINE_SM_TCM_STATE_NOTIFIED_TC;
                else
                if (pom_ptr->tc_prop && !pom_ptr->common->oper_edge)
                    next_status     = XSTP_ENGINE_SM_TCM_STATE_PROPAGATING;
                else
                if (XSTP_UTY_Cist(om_ptr) && (pom_ptr->common->rcvd_tc_ack))
                    next_status     = XSTP_ENGINE_SM_TCM_STATE_ACKNOWLEDGED;
                else
                {
                    next_status     = XSTP_ENGINE_SM_TCM_STATE_PROGRESS_ACTIVE;
                    sm_progress     = FALSE;
                }
                break;

            /* NOTIFIED_TCN */
            case XSTP_ENGINE_SM_TCM_STATE_NOTIFIED_TCN:
                /* Action */
                pom_ptr->tc_while   = XSTP_UTY_NewTcWhile(om_ptr, lport); /* 13.26.6 */
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_TCM_STATE_NOTIFIED_TC;
                break;

            /* NOTIFIED_TC */
            case XSTP_ENGINE_SM_TCM_STATE_NOTIFIED_TC:
                /* Action */
                /*for IGMPSnooping
                              *send msg to mucast .the parameters are is_root, port,tc_timer.
                 *when port that has rec tc or tcn
                              */
                if (pom_ptr->rcvd_tc
                	|| ((XSTP_UTY_Cist(om_ptr)
                	     &&pom_ptr->common->rcvd_tcn)
                	   )
                   )
                {
                    /*because the port received tc won't have tc_while, here we calculate tc_while direclty*/
                    if (    (pom_ptr->common->oper_point_to_point_mac)
                        &&  (pom_ptr->common->send_rstp)
                       )
                    {
                        SYS_CALLBACK_MGR_LportTcChangeCallback(SYS_MODULE_XSTP,
                        	                                   ((XSTP_TYPE_MSTP_MODE==XSTP_OM_GetForceVersion())? TRUE:FALSE),
                        	                                   om_ptr->instance_id,
                        	                                   lport,
                        	                                   XSTP_UTY_RootBridge(om_ptr),
                        	                                   (pom_ptr->port_times.hello_time + 1),
                        	                                   om_ptr->instance_vlans_mapped);
                    }
                    else
                    {
                        SYS_CALLBACK_MGR_LportTcChangeCallback(SYS_MODULE_XSTP,
                        	                                   ((XSTP_TYPE_MSTP_MODE==XSTP_OM_GetForceVersion())? TRUE:FALSE),
                        	                                   om_ptr->instance_id,
                        	                                   lport,
                        	                                   XSTP_UTY_RootBridge(om_ptr),
                        	                                   (om_ptr->bridge_info.root_times.max_age + om_ptr->bridge_info.root_times.forward_delay),
                        	                                   om_ptr->instance_vlans_mapped);
                    }
                }

                if (XSTP_UTY_Cist(om_ptr))               /* 13.25.2 */
                    pom_ptr->common->rcvd_tcn   = FALSE;
                pom_ptr->rcvd_tc = FALSE;
                if (    (XSTP_UTY_Cist(om_ptr))
                    &&  (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
                   )
                    pom_ptr->common->tc_ack = TRUE;
                XSTP_UTY_SetTcPropTree(om_ptr, lport);  /* 13.26.20 */
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_TCM_STATE_ACTIVE;
                break;

            /* PROPAGATING */
            case XSTP_ENGINE_SM_TCM_STATE_PROPAGATING:
                /* Action */
                pom_ptr->tc_while   = XSTP_UTY_NewTcWhile(om_ptr, lport); /* 13.26.6 */
                XSTP_UTY_Flush(om_ptr, lport);
                pom_ptr->tc_prop    = FALSE;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_TCM_STATE_ACTIVE;
                break;

            /* ACKNOWLEDGED */
            case XSTP_ENGINE_SM_TCM_STATE_ACKNOWLEDGED:
                /* Action */
                pom_ptr->tc_while   = 0;
                pom_ptr->common->rcvd_tc_ack= FALSE;
                /* Progress */
                /* UCT */
                uct                 = TRUE;
                uct_progress        = TRUE;
                next_status         = XSTP_ENGINE_SM_TCM_STATE_ACTIVE;
                break;

            default:
                break;
        } /* End of switch */
    } while (uct);

    pom_ptr->sms_tcm    = next_status;

    return (sm_progress || uct_progress);
} /* End of XSTP_ENGINE_StateMachineTCM */

#endif /* XSTP_TYPE_PROTOCOL_MSTP */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_ENGINE_StateMachineBDM
 *-------------------------------------------------------------------------
 * PURPOSE  : Implement Bridge Detection state machine
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport (1..max)
 * OUTPUT   : None
 * RETUEN   : TRUE if this state machine needs to progress, else FALSE
 * NOTES    : Ref to the description in 17.25, IEEE Std 802.1D-2004
 */
static  BOOL_T  XSTP_ENGINE_StateMachineBDM(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI8_T               next_status;
    BOOL_T              sm_progress;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    next_status = pom_ptr->sms_bdm;
    sm_progress     = TRUE;

    XSTP_ENGINE_DebugStateMachine(om_ptr->instance_id, lport, XSTP_ENGINE_DebugBDM, next_status);
    switch (next_status)
    {
        case XSTP_ENGINE_SM_BDM_STATE_EDGE:
            pom_ptr->common->oper_edge = TRUE;
            /* Progress */
        case XSTP_ENGINE_SM_BDM_STATE_PROGRESS_EDGE:
            if (   (   ((!pom_ptr->common->port_enabled) || (!pom_ptr->common->link_up))
                    && (!pom_ptr->common->admin_edge)
                   )
                || (!pom_ptr->common->oper_edge)
               )
            {
                next_status = XSTP_ENGINE_SM_BDM_STATE_NOT_EDGE;
            }
            else
            {
                next_status = XSTP_ENGINE_SM_BDM_STATE_PROGRESS_EDGE;
                sm_progress = FALSE;
            }
            break;

        case XSTP_ENGINE_SM_BDM_STATE_NOT_EDGE:
            pom_ptr->common->oper_edge = FALSE;
            /* Progress */
        case XSTP_ENGINE_SM_BDM_STATE_PROGRESS_NOT_EDGE:
            if (    (    ((!pom_ptr->common->port_enabled) || (!pom_ptr->common->link_up))
                      && (pom_ptr->common->admin_edge)
                    )
                 || (    (pom_ptr->common->edge_delay_while == 0)
                      && pom_ptr->common->auto_edge
                      && pom_ptr->common->send_rstp
                      && pom_ptr->proposing
                      && (!pom_ptr->common->loopback_block)
                    )
               )
            {
                next_status = XSTP_ENGINE_SM_BDM_STATE_EDGE;
            }
            else
            {
                next_status = XSTP_ENGINE_SM_BDM_STATE_PROGRESS_NOT_EDGE;
                sm_progress = FALSE;
            }
            break;

        default:
            break;
    }

    pom_ptr->sms_bdm = next_status;

    return sm_progress;
} /* End of XSTP_ENGINE_StateMachineBDM */

/* ===================================================================== */
/* ===================================================================== */
