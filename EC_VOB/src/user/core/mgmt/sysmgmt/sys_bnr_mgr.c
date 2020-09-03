/* ------------------------------------------------------------------------
 *  FILE NAME  -  SYS_BNR_MGR.C				           						
 * ------------------------------------------------------------------------
 * PURPOSE: This package provides the services to handle the system banner message
 *          functions
 *
 * Notes: This module will support all the system banner message functions includes
 *        1) MOTD (Message of the day)
 *        2) Enable (after enter priviledge) --> not implement
 *        3) Incoming (when telnet connect, show message in console) --> not implement
 *
 *  History
 *
 *   BHU     11/07/2004      new created
 * 
 * ------------------------------------------------------------------------
 * Copyright(C)							  	ACCTON Technology Corp. , 2004      
 * ------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */
#include "stdio.h"
#include <string.h>
#include <stdlib.h>
#include "sys_hwcfg.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "sys_bld.h"
#include "sys_time.h"
#include "sys_bnr_mgr.h"
#include "l_stdlib.h"

/* UI message */
#include "sys_module.h"
//#include "syslog_type.h"
//#include "eh_type.h"
//#include "eh_mgr.h"



/*static UI32_T sys_bnr_mgr_sem_id;*/

/*
sys_bnr_msg save below three type banner message 

 1) MOTD (Message of the day)
 2) Enable (after enter priviledge) --> not implement
 3) Incoming (when telnet connect, show message in console) --> not implement
*/
static UI8_T  sys_bnr_msg[SYS_BNR_MGR_MAX_TYPE][SYS_ADPT_MAX_MOTD_LENGTH + 1];
static UI8_T  sys_bnr_msg_delimiting_character[SYS_BNR_MGR_MAX_TYPE][1];
                                           

SYSFUN_DECLARE_CSC                    /* declare variables used for transition mode  */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_BNR_MGR_Init
 * ---------------------------------------------------------------------
 * PURPOSE: This function will init the system resource
 * 																		
 * INPUT : None                                     				
 * OUTPUT: None                          					
 * RETURN: TRUE/FALSE                                               		
 * NOTES: 1. This routine will initialize the All Banner Message
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_BNR_MGR_Init(void)
{
       
    memset(sys_bnr_msg, 0, SYS_BNR_MGR_MAX_TYPE * (SYS_ADPT_MAX_MOTD_LENGTH + 1));       
    return TRUE;    
 
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_BNR_MGR_EnterMasterMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will enter master mode																		
 * INPUT:   None                                     				
 * OUTPUT:  None                          					
 * RETURN:  None                                               		
 * NOTES:   None  
 * ---------------------------------------------------------------------*/
void SYS_BNR_MGR_EnterMasterMode(void)
{
    /* set mgr in master mode */
    SYSFUN_ENTER_MASTER_MODE();
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_BNR_MGR_EnterSlaveMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will enter slave mode 																		
 * INPUT:   None                                     				
 * OUTPUT:  None                          					
 * RETURN:  None                                               		
 * NOTES:   None  
 * ---------------------------------------------------------------------
 */
void SYS_BNR_MGR_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE();
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_BNR_MGR_SetTransitionMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the SYS_BNR into the transition mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void SYS_BNR_MGR_SetTransitionMode()
{
    SYSFUN_SET_TRANSITION_MODE();
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_BNR_MGR_EnterTransitionMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will enter transition mode																	
 * INPUT:   None                                     				
 * OUTPUT:  None                          					
 * RETURN:  None                                               		
 * NOTES:   None  
 * ---------------------------------------------------------------------
 */
void SYS_BNR_MGR_EnterTransitionMode(void)
{
    /* wait other callers leave */
    SYSFUN_ENTER_TRANSITION_MODE();
    memset(sys_bnr_msg, 0, SYS_BNR_MGR_MAX_TYPE * (SYS_ADPT_MAX_MOTD_LENGTH + 1));           
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_BNR_MGR_GetSysBnrMsg
 * ---------------------------------------------------------------------
 * PURPOSE: get system banner message for specified type
 * INPUT	: sys_bnr_type - key to specifiy banner type (MOTD, Enable, Incoming)
 * OUTPUT	: *msg  - the banner message of specified type
 * RETURN	: TRUE
 * NOTES	: None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_BNR_MGR_GetSysBnrMsg(SYS_BNR_MGR_TYPE_T sys_bnr_type, UI8_T *msg)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
    	return FALSE;
    }
    else
    { 
        if(sys_bnr_msg[sys_bnr_type][0] == '\0') /*no data*/
        {
    	    return FALSE;            
        }          
        strcpy((char *)msg, (char *)sys_bnr_msg[sys_bnr_type]);
        return TRUE;
    }
}



/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_BNR_MGR_GetNextSysBnrMsgByID
 * ---------------------------------------------------------------------
 * PURPOSE: get system banner message for specified type, section ID, and buffer 
 * INPUT	: sys_bnr_type - key to specifiy banner type (MOTD, Enable, Incoming)
              *section_id   - key to specifiy getting which section 
              buffer_size  - key to specifiy size to split data 
 * OUTPUT	: *msg  - the banner message of specified type
 * OUTPUT	: *section_id  -  current section_id
 * RETURN	: TRUE
 * NOTES	: If section ID is zero, it means to get first section data. 
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_BNR_MGR_GetNextSysBnrMsgByID(SYS_BNR_MGR_TYPE_T sys_bnr_type, UI8_T *msg, UI32_T *section_id, UI32_T buffer_size)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
    	return FALSE;
    }
    else
    { 
        if(sys_bnr_msg[sys_bnr_type][0] == '\0') /*no data*/
        {
    	    return FALSE;            
        }    
      
        if( (*section_id) * buffer_size >= strlen((char *)sys_bnr_msg[sys_bnr_type]) )
        {
    	    return FALSE;            
        }
        else
        {                                        
            strncpy((char *)msg, (char *)&(sys_bnr_msg[sys_bnr_type][(*section_id) * buffer_size]), buffer_size);
        }

        *section_id = *section_id + 1;        
        return TRUE;
    }
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_BNR_MGR_SetSysBnrMsg
 * ---------------------------------------------------------------------
 * PURPOSE: Set system banner message for specified type
 * INPUT	: sys_bnr_type - key to specifiy banner type (MOTD, Enable, Incoming)
              *msg  - the banner message of specified type
 * OUTPUT	: None
 * RETUEN:  : TRUE  -- success
 *            FALSE -- failure
 * NOTES	: msg is NULL point, then means to reset banner message
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_BNR_MGR_SetSysBnrMsg(SYS_BNR_MGR_TYPE_T sys_bnr_type, UI8_T *msg)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
    	return FALSE;
    }
    else
    {
        if(msg == NULL)
        {
            memset(sys_bnr_msg[sys_bnr_type], 0, SYS_ADPT_MAX_MOTD_LENGTH);                     
        }
        else
        {
            strncpy((char *)sys_bnr_msg[sys_bnr_type], (char *)msg, SYS_ADPT_MAX_MOTD_LENGTH);              
        }        
    }
    return TRUE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_BNR_MGR_GetRunningSysBnrMsg
 * ---------------------------------------------------------------------
 * PURPOSE: Get running system banner message for specified type
 * INPUT	: sys_bnr_type - key to specifiy banner type (MOTD, Enable, Incoming)
              *msg  - the banner message of specified type
 * OUTPUT	: *msg  - the banner message of specified type
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES	: None
 * ---------------------------------------------------------------------
 */


UI32_T SYS_BNR_MGR_GetRunningSysBnrMsg(SYS_BNR_MGR_TYPE_T sys_bnr_type, UI8_T *msg)
{
    if (SYS_BNR_MGR_GetSysBnrMsg(sys_bnr_type, msg) == FALSE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else if (msg[0] == '\0')
    {
        /* If banner message is same as default one, return NO_CHANGE
         * Note that default banner message is null .
         */
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}



/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_BNR_MGR_SetSysBnrMsgDelimitingChar
 * ---------------------------------------------------------------------
 * PURPOSE: Set system banner message delimiting character for specified type
 * INPUT	: sys_bnr_type - key to specifiy banner type (MOTD, Enable, Incoming)
              delimitingChar  - the delimiting character of specified type
 * OUTPUT	: None
 * RETUEN:  : TRUE  -- success
 *            FALSE -- failure
 * NOTES	: None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_BNR_MGR_SetSysBnrMsgDelimitingChar(SYS_BNR_MGR_TYPE_T sys_bnr_type, UI8_T delimitingChar)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
    	return FALSE;
    }
    else
    {
        sys_bnr_msg_delimiting_character[sys_bnr_type][0] = delimitingChar;          
    }
    return TRUE;
}



/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_BNR_MGR_GetSysBnrMsgDelimitingChar
 * ---------------------------------------------------------------------
 * PURPOSE: Get system banner message delimiting character for specified type
 * INPUT	: sys_bnr_type - key to specifiy banner type (MOTD, Enable, Incoming)
              *delimitingChar  - the delimiting character of specified type
 * OUTPUT	: *delimitingChar  - the delimiting character of specified type
 * RETUEN:  : TRUE  -- success
 *            FALSE -- failure
 * NOTES	: None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_BNR_MGR_GetSysBnrMsgDelimitingChar(SYS_BNR_MGR_TYPE_T sys_bnr_type, UI8_T *delimitingChar)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
    	return FALSE;
    }
    else
    {
        *delimitingChar = sys_bnr_msg_delimiting_character[sys_bnr_type][0];         
    }
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SYS_BNR_MGR_HandleIPCReqMsg
 *-------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for sys_bnr mgr.
 * INPUT   : ipcmsg_p  --  input request ipc message buffer
 * OUTPUT  : ipcmsg_p  --  output response ipc message buffer
 * RETUEN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_BNR_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    if(ipcmsg_p == NULL)
        return FALSE;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        SYS_BNR_MGR_MSG_RETVAL(ipcmsg_p) = SYS_BNR_MGR_IPC_RESULT_FAIL;
        ipcmsg_p->msg_size = SYS_BNR_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
        return TRUE;
    }
    switch(ipcmsg_p->cmd)
    {
        case SYS_BNR_MGR_IPC_CMD_SETSYSBNRMSG:
        {
            SYS_BNR_MGR_IPCMsg_SysBnrMsg_T *data_p = SYS_BNR_MGR_MSG_DATA(ipcmsg_p);
            SYS_BNR_MGR_MSG_RETVAL(ipcmsg_p) = SYS_BNR_MGR_SetSysBnrMsg(data_p->sys_bnr_type,data_p->msg);
            ipcmsg_p->msg_size = SYS_BNR_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case SYS_BNR_MGR_IPC_CMD_SETSYSBNRMSGDELIMITINGCHAR:
        {
            SYS_BNR_MGR_IPCMsg_SysBnrMsgDelimitingChar_T *data_p = SYS_BNR_MGR_MSG_DATA(ipcmsg_p);
            SYS_BNR_MGR_MSG_RETVAL(ipcmsg_p) = SYS_BNR_MGR_SetSysBnrMsgDelimitingChar(data_p->sys_bnr_type,data_p->delimitingChar);
            ipcmsg_p->msg_size = SYS_BNR_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case SYS_BNR_MGR_IPC_CMD_GETSYSBNRMSGDELIMITINGCHAR:
        {
            SYS_BNR_MGR_IPCMsg_SysBnrMsgDelimitingChar_T *data_p = SYS_BNR_MGR_MSG_DATA(ipcmsg_p);
            SYS_BNR_MGR_MSG_RETVAL(ipcmsg_p) = SYS_BNR_MGR_GetSysBnrMsgDelimitingChar(data_p->sys_bnr_type,&data_p->delimitingChar);
            ipcmsg_p->msg_size = SYS_BNR_MGR_GET_MSGBUFSIZE(SYS_BNR_MGR_IPCMsg_SysBnrMsgDelimitingChar_T) ;
            break;
        }    
        case SYS_BNR_MGR_IPC_CMD_GETNEXTSYSBNRMSGBYID:
        {
            SYS_BNR_MGR_IPCMsg_NextSysBnrMsgByID_T *data_p = SYS_BNR_MGR_MSG_DATA(ipcmsg_p);
            SYS_BNR_MGR_MSG_RETVAL(ipcmsg_p) = SYS_BNR_MGR_GetNextSysBnrMsgByID(data_p->sys_bnr_type,data_p->msg,&data_p->section_id,data_p->buffer_size);
            ipcmsg_p->msg_size = SYS_BNR_MGR_GET_MSGBUFSIZE(SYS_BNR_MGR_IPCMsg_NextSysBnrMsgByID_T) ;
            break;
        }           
        default:
            SYSFUN_Debug_Printf("%s(): Invalid cmd(%d).\n", __FUNCTION__, (int)(ipcmsg_p->cmd));
            SYS_BNR_MGR_MSG_RETVAL(ipcmsg_p) = SYS_BNR_MGR_IPC_RESULT_FAIL;
            ipcmsg_p->msg_size = SYS_BNR_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
    }
    return TRUE;
}



