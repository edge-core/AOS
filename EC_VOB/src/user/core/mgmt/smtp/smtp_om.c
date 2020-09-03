/* Module Name: SMTP_OM.C
 * Purpose: Initialize the database resources and provide some Get/Set function
 *          for accessing the smtp database.
 *
 * Notes:
 *
 * History:
 *    01/22/03       -- Ricky Lin, Create
 *
 * Copyright(C)      Accton Corporation, 1999 - 2001
 *
 */

 /* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "sys_type.h"
#include "sys_dflt.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "syslog_type.h"
#include "smtp_om.h"
#include "sys_bld.h"
#include "l_mm.h"


/* NAMING CONSTANT DECLARATIONS
 */
#define SMTP_OM_MAX_QUE_NBR    (SYS_ADPT_TOTAL_NBR_OF_LPORT * 2)


/* DATA TYPE DECLARATIONS+
 */
typedef struct
{
	I32_T                     que_elements_nbr;
	SMTP_OM_QueueRecord_T     *front;
	SMTP_OM_QueueRecord_T     *rear;
} SMTP_OM_Queue_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */


/* STATIC VARIABLE DECLARATIONS
 */
static SMTP_OM_Config_T     smtp_config;
static UI32_T               smtp_om_sem_id;
static UI32_T               orig_priority;
static SMTP_OM_Queue_T      smtp_queue;
static UI32_T               smtp_task_id;

/* MACRO FUNCTIONS DECLARACTION
 */
#define SMTP_OM_LOCK()    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(smtp_om_sem_id);
#define SMTP_OM_UNLOCK()  SYSFUN_OM_LEAVE_CRITICAL_SECTION(smtp_om_sem_id,orig_priority);

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME: SMTP_OM_CreatSem
 * PURPOSE: Initiate the semaphore for SMTP objects
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTE:    None
 */
BOOL_T SMTP_OM_CreatSem(void)
{
    /* create semaphore */
    if(SYSFUN_OK!=SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_SMTP_OM, &smtp_om_sem_id))
    {
        return FALSE;
    }
    return TRUE;
} /* End of SMTP_OM_CreatSem */

/* FUNCTION NAME: SMTP_OM_Initiate_System_Resources
 * PURPOSE: This function is used to initialize the smtp database.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   initialize smtp_config.
 *
 */
void SMTP_OM_Initiate_System_Resources(void)
{
	UI32_T i;

	smtp_config.smtp_admin_status = SMTP_STATUS_ENABLE;
	smtp_config.smtp_level = SYSLOG_LEVEL_DEBUG;

	for(i=0;i<SYS_ADPT_MAX_NUM_OF_SMTP_SERVER;i++)
	{
		smtp_config.server_ipaddr[i] = SYS_DFLT_SMTP_SERVER_IP_ADDR;
	}

	strcpy((char *)smtp_config.source_emailaddr,SYS_DFLT_SMTP_SOURCE_EMAIL_ADDR);

	for(i=0;i<SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS;i++)
	{
		strcpy((char *)smtp_config.destination_emailaddr[i],SYS_DFLT_SMTP_DESTINATION_EMAIL_ADDR);
	}

	return;
}

/* FUNCTION NAME: SMTP_OM_GetNextSmtpServerIPAddr
 * PURPOSE: This function is used to get next smtp server ip address.
 * INPUT:   *ip_addr -- buffer of server ip address.
 * OUTPUT:  *ip_addr -- value of server ip address.
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
 *          SMTP_RETURN_FAIL   --  fail
 * NOTES:   1. Max number of server ip address is SYS_ADPT_MAX_NUM_OF_SMTP_SERVER.
 *          2. get one server ip address one time
 *          3. use 0 to get first ip address
 *
 */
UI32_T SMTP_OM_GetNextSmtpServerIPAddr(UI32_T *ip_addr)
{
	UI32_T  ret, same = 0, found = 0;
	UI8_T   i, temp_index = 0;

    SMTP_OM_LOCK();

	if (*ip_addr == 0)
    {
    	same = 1;
    }

    for (i=1;i<=SYS_ADPT_MAX_NUM_OF_SMTP_SERVER;i++)
    {
    	if (same == 0)
    	{
    	    //if (memcmp(&smtp_config.server_ipaddr[i-1],ip_addr,sizeof(UI32_T)) == 0)
    	    if(smtp_config.server_ipaddr[i-1] == *ip_addr)
    	    {
    	        same = 1;
    	        continue;
    	    }
    	}

    	if (same == 1)
    	{
    	    if (smtp_config.server_ipaddr[i-1] != 0)
    	    {
    	        temp_index = i-1;
    	        found = 1;
    	        break;
    	    }
    	}
    }

    if(found == 1)
    {
    	*ip_addr = smtp_config.server_ipaddr[temp_index];
    	ret = SMTP_RETURN_SUCCESS;
    }
    else
        ret = SMTP_RETURN_FAIL;

    SMTP_OM_UNLOCK();

    return ret;
}

/* FUNCTION NAME: SMTP_OM_SetSmtpServerIPAddr
 * PURPOSE: This function is used to set the smtp server ip address.
 * INPUT:   ip_addr -- value of server ip.
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_DATABASE_FULL -- database full
 * NOTES:   1. Max number of server ip address is SYS_ADPT_MAX_NUM_OF_SMTP_SERVER.
 *
 */
UI32_T SMTP_OM_SetSmtpServerIPAddr(UI32_T ip_addr)
{
	UI32_T  ret = SMTP_RETURN_SUCCESS;
	UI8_T   i, temp_index = 0, found = 0;

    SMTP_OM_LOCK();

    for (i=0;i<SYS_ADPT_MAX_NUM_OF_SMTP_SERVER;i++)
    {
        if (smtp_config.server_ipaddr[i] == ip_addr)
        {
            ret = SMTP_RETURN_INPUT_EXIST;
            break;
        }

        if (smtp_config.server_ipaddr[i] == 0)
        {
            if (found == 0)
            {
                temp_index=i;
                found = 1;
            }
        }
    }

    if (SMTP_RETURN_SUCCESS == ret)
    {
        if (found == 0)
        {
            ret = SMTP_RETURN_DATABASE_FULL;
        }
        else
        {
            smtp_config.server_ipaddr[temp_index] = ip_addr;
        }
    }

    SMTP_OM_UNLOCK();

    return ret;
}

/* FUNCTION NAME: SMTP_OM_DeleteSmtpServerIPAddr
 * PURPOSE: This function is used to delete smtp server ip address.
 * INPUT:   ip_addr -- deleted server ip.
 * OUTPUT:  NONE
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
 *          SMTP_RETURN_INPUT_NOT_EXIST   --  input not exist
 * NOTES:   1. Max number of server ip address is SYS_ADPT_MAX_NUM_OF_SMTP_SERVER.
 *          2. delete one server ip address one time
 *
 */
UI32_T SMTP_OM_DeleteSmtpServerIPAddr(UI32_T ip_addr)
{
    UI32_T  ret;
	UI8_T   i, temp_index, found = 0;

    SMTP_OM_LOCK();

    for (i=0;i<SYS_ADPT_MAX_NUM_OF_SMTP_SERVER;i++)
    {
        if (smtp_config.server_ipaddr[i] == ip_addr)
        {
            found = 1;
            temp_index = i;
            break;
        }
    }

    if(found == 1)
    {
    	smtp_config.server_ipaddr[temp_index] = SYS_DFLT_SMTP_SERVER_IP_ADDR;
        ret = SMTP_RETURN_SUCCESS;
    }
    else
    {
    	ret = SMTP_RETURN_INPUT_NOT_EXIST;
    }

    SMTP_OM_UNLOCK();

    return ret;
}

/* FUNCTION NAME: SMTP_OM_IsSmtpServerIPAddrExist
 * PURPOSE: This function is used to check if the smtp server ip address exist in database.
 * INPUT:   ip_addr -- value of server ip.
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
 *          SMTPLOG_FAIL   --  set fail
 * NOTES:   None.
 *
 */
UI32_T SMTP_OM_IsSmtpServerIPAddrExist(UI32_T ip_addr)
{
	UI32_T  same = 0;
    UI8_T   i;

	if (ip_addr == 0)
    {
    	return SMTP_RETURN_FAIL;
    }

    SMTP_OM_LOCK();

    for (i=1;i<=SYS_ADPT_MAX_NUM_OF_SMTP_SERVER;i++)
    {
    	if (smtp_config.server_ipaddr[i-1] == ip_addr)
    	{
    	    same = 1;
    	}
    }

    SMTP_OM_UNLOCK();

    if(same == 1)
    {
    	return SMTP_RETURN_SUCCESS;
    }
    else
        return SMTP_RETURN_FAIL;
}

/* FUNCTION NAME: SMTP_OM_SetSmtpAdminStatus
 * PURPOSE: This function is used to enable/disable smtp admin status.
 * INPUT:   status -- smtp admin status.
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
 * NOTES:   1. status is defined as following:
 *             SMTP_STATUS_ENABLE
 *             SMTP_STATUS_DISABLE
 *
 */
UI32_T SMTP_OM_SetSmtpAdminStatus(UI32_T status)
{
    SMTP_OM_LOCK();
	smtp_config.smtp_admin_status = status;
    SMTP_OM_UNLOCK();

    return SMTP_RETURN_SUCCESS;
}

/* FUNCTION NAME: SMTP_OM_GetSmtpAdminStatus
 * PURPOSE: This function is used to get smtp status.
 * INPUT:   *status -- output buffer of smtp admin status
 * OUTPUT:  *status -- smtp admin status.
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
 * NOTES:   1. status is defined as following:
 *             SMTP_STATUS_ENABLE
 *             SMTP_STATUS_DISABLE
 *
 */
UI32_T SMTP_OM_GetSmtpAdminStatus(UI32_T *status)
{
    SMTP_OM_LOCK();
	*status = smtp_config.smtp_admin_status;
    SMTP_OM_UNLOCK();

    return SMTP_RETURN_SUCCESS;
}

/* FUNCTION NAME: SMTP_OM_GetSmtpSourceEmailAddr
 * PURPOSE: This function is used to get smtp source email address.
 * INPUT:   *email_addr -- output buffer of smtp source email address
 * OUTPUT:  *email_addr -- value of smtp source email address.
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
 * NOTES:   1. There is only one source email address
 *          2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *
 */
UI32_T SMTP_OM_GetSmtpSourceEmailAddr(UI8_T *email_addr)
{
    SMTP_OM_LOCK();
	strcpy((char *)email_addr,(char *)smtp_config.source_emailaddr);
    SMTP_OM_UNLOCK();

    return SMTP_RETURN_SUCCESS;
}

/* FUNCTION NAME: SMTP_OM_SetSmtpSourceEmailAddr
 * PURPOSE: This function is used to set smtp source email address.
 * INPUT:   *email_addr -- value of smtp source email address
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
 * NOTES:   1. There is only one source email address
 *          2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *
 */
UI32_T SMTP_OM_SetSmtpSourceEmailAddr(UI8_T *email_addr)
{
    SMTP_OM_LOCK();
	strcpy((char *)smtp_config.source_emailaddr,(char *)email_addr);
    SMTP_OM_UNLOCK();

    return SMTP_RETURN_SUCCESS;
}

//ADD daniel 
/* FUNCTION NAME: SMTP_OM_DeleteSmtpSourceEmailAddr
 * PURPOSE: This function is used to delete smtp source email address.
 * INPUT:   *email_addr -- value of smtp source email address
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
 * NOTES:   1. There is only one source email address
 *          2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *
 */
UI32_T SMTP_OM_DeleteSmtpSourceEmailAddr(UI8_T *email_addr)
{
    SMTP_OM_LOCK();
    strcpy((char *)email_addr, SYS_DFLT_SMTP_SOURCE_EMAIL_ADDR);
	strcpy((char *)smtp_config.source_emailaddr,(char *)email_addr);
    SMTP_OM_UNLOCK();

    return SMTP_RETURN_SUCCESS;
}



/* FUNCTION NAME: SMTP_OM_GetNextSmtpDestinationEmailAddr
 * PURPOSE: This function is used to get smtp destination email address.
 * INPUT:   *email_addr -- output buffer of smtp destination email address
 * OUTPUT:  *email_addr -- smtp destination email address.
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
 *          SMTP_RETURN_FAIL    -- fail
 * NOTES:   1. Max number of destination email address is SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS
 *          2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *
 */
UI32_T SMTP_OM_GetNextSmtpDestinationEmailAddr(UI8_T *email_addr)
{
	UI32_T  ret, same = 0, found = 0;
	UI8_T   i, temp_index = 0;

	if (strcmp((char *)email_addr,"") == 0)
    {
    	same = 1;
    }

    SMTP_OM_LOCK();

    for (i=1;i<=SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS;i++)
    {
    	if (same == 0)
    	{
    	    if (strcmp((char *)smtp_config.destination_emailaddr[i-1],(char *)email_addr) == 0)
    	    {
    	        same = 1;
    	        continue;
    	    }
    	}

    	if (same == 1)
    	{
    	    if (strcmp((char *)smtp_config.destination_emailaddr[i-1],"") != 0)
    	    {
    	        temp_index = i-1;
    	        found = 1;
    	        break;
    	    }
    	}
    }

    if(found == 1)
    {
    	strcpy((char *)email_addr,(char *)smtp_config.destination_emailaddr[temp_index]);
    	ret = SMTP_RETURN_SUCCESS;
    }
    else
    {
        ret = SMTP_RETURN_FAIL;
    }

    SMTP_OM_UNLOCK();

    return ret;
}

/* FUNCTION NAME: SMTP_OM_SetSmtpDestinationEmailAddr
 * PURPOSE: This function is used to set smtp destination email address.
 * INPUT:   *email_addr -- smtp destination email address
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
 *          SMTP_RETURN_INPUT_EXIST  -- input value already exist
 *          SMTP_RETURN_DATABASE_FULL -- database full
 * NOTES:   1. Max number of destination email address is SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS
 *          2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *
 */
UI32_T SMTP_OM_SetSmtpDestinationEmailAddr(UI8_T *email_addr)
{
    UI32_T  ret = SMTP_RETURN_SUCCESS;
	UI8_T   i, temp_index = 0, found = 0;

    SMTP_OM_LOCK();

    for (i=0;i<SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS;i++)
    {
        if (strcmp((char *)smtp_config.destination_emailaddr[i],(char *)email_addr) == 0)
        {
            ret = SMTP_RETURN_INPUT_EXIST;
            break;
        }

        if (strcmp((char *)smtp_config.destination_emailaddr[i],"") == 0)
        {
            if (found == 0)
            {
                temp_index=i;
                found = 1;
            }
        }
    }

    if (ret == SMTP_RETURN_SUCCESS)
    {
        if (found == 0)
            ret = SMTP_RETURN_DATABASE_FULL;
        else
            strcpy((char *)smtp_config.destination_emailaddr[temp_index],(char *)email_addr);
    }

    SMTP_OM_UNLOCK();

    return ret;
}

/* FUNCTION NAME: SMTP_OM_DeleteSmtpDestinationEmailAddr
 * PURPOSE: This function is used to delete smtp destination email address.
 * INPUT:   *email_addr -- smtp destination email address
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
 *          SMTP_RETURN_INPUT_NOT_EXIST   --  input not exist
 * NOTES:   1. Max number of destination email address is SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS
 *          2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *
 */
UI32_T SMTP_OM_DeleteSmtpDestinationEmailAddr(UI8_T *email_addr)
{
    UI32_T  ret = SMTP_RETURN_INPUT_NOT_EXIST;
	UI8_T   i;

    SMTP_OM_LOCK();

    for (i=0;i<SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS;i++)
    {
        if (strcmp((char *)smtp_config.destination_emailaddr[i],(char *)email_addr) == 0)
        {
            strcpy((char *)smtp_config.destination_emailaddr[i],SYS_DFLT_SMTP_DESTINATION_EMAIL_ADDR);
            ret = SMTP_RETURN_SUCCESS;
        }
    }

    SMTP_OM_UNLOCK();

    return ret;
}

/* FUNCTION NAME: SMTP_OM_SetEmailSeverityLevel
 * PURPOSE: This function is used to set smtp email severity level.
 * INPUT:   level -- value of smtp email severity level.
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
 * NOTES:   1. smtp severity level symbol is defined as following:
 *          --  SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *          --  SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *          --  SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *          --  SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *          --  SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *          --  SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *          --  SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *          --  SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *          2. SYSLOG_LEVEL_EMERG is highest priority.
 *             SYSLOG_LEVEL_DEBUG is lowest priority.
 *
 */
UI32_T SMTP_OM_SetEmailSeverityLevel(UI32_T level)
{
    SMTP_OM_LOCK();
    smtp_config.smtp_level = level;
    SMTP_OM_UNLOCK();

    return SMTP_RETURN_SUCCESS;
}


//ADD daniel 
/* FUNCTION NAME: SMTP_OM_DeleteEmailSeverityLevel
 * PURPOSE: This function is used to delete smtp email severity level to default value.
 * INPUT:   level -- value of smtp email severity level.
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
 * NOTES:   1. smtp severity level symbol is defined as following:
 *          --  SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *          --  SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *          --  SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *          --  SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *          --  SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *          --  SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *          --  SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *          --  SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *          2. SYSLOG_LEVEL_EMERG is highest priority.
 *             SYSLOG_LEVEL_DEBUG is lowest priority.
 *
 */
UI32_T SMTP_OM_DeleteEmailSeverityLevel(UI32_T level) 
{
    SMTP_OM_LOCK();
    level = SYS_DFLT_SMTP_LEVEL;
    smtp_config.smtp_level = level;
    SMTP_OM_UNLOCK();

    return SMTP_RETURN_SUCCESS;
}
//end ADD


/* FUNCTION NAME: SMTP_OM_GetEmailSeverityLevel
 * PURPOSE: This function is used to get smtp email severity level.
 * INPUT:   *level -- output buffer of smtp email severity level.
 * OUTPUT:  *level -- value of smtp email severity level.
 * RETUEN:  SMTP_RETURN_SUCCESS    --  OK, Successful, Without any Error
 * NOTES:   1. smtp severity level symbol is defined as following:
 *          --  SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *          --  SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *          --  SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *          --  SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *          --  SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *          --  SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *          --  SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *          --  SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *          2. SYSLOG_LEVEL_EMERG is highest priority.
 *             SYSLOG_LEVEL_DEBUG is lowest priority.
 *
 */
UI32_T SMTP_OM_GetEmailSeverityLevel(UI32_T *level)
{
    SMTP_OM_LOCK();
    *level = smtp_config.smtp_level;
    SMTP_OM_UNLOCK();

    return SMTP_RETURN_SUCCESS;
}

/* FUNCTION NAME: SMTP_OM_QueueGetElementNbr
 * PURPOSE: This function is used to get element number of smtp_queue
 * INPUT:   None
 * OUTPUT:  None.
 * RETUEN:  element number of smtp_queue
 * NOTES:   None.
 *
 */
UI32_T SMTP_OM_QueueGetElementNbr(void)
{
    UI32_T  ret;

    SMTP_OM_LOCK();
    ret = smtp_queue.que_elements_nbr;
    SMTP_OM_UNLOCK();

    return ret;
}

/* FUNCTION NAME: SMTP_OM_QueueEnqueue
 * PURPOSE: This function is used to enqueue smtp entry.
 * INPUT:   *p -- smtp event data pointer.
 *          *q -- smtp queue pointer.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   Max queue number is SMTP_OM_MAX_QUE_NBR
 *
 */
void SMTP_OM_QueueEnqueue(SMTP_OM_QueueRecord_T *qData)
{
	if (qData == NULL)
	{
		return;
	} /* End of if */

    SMTP_OM_LOCK();

    if (smtp_queue.que_elements_nbr >= SMTP_OM_MAX_QUE_NBR)
    {
        L_MM_Free(qData);
    }
    else
    {
        if (smtp_queue.rear == (SMTP_OM_QueueRecord_T *)NULL )	 /* empty queue */
    	{
    		smtp_queue.rear = qData;
    		smtp_queue.front = qData;
    		qData->next = (SMTP_OM_QueueRecord_T *)NULL;
    	}
    	else
    	{
    		smtp_queue.rear->next = qData;
    		smtp_queue.rear = qData;
    		qData->next = (SMTP_OM_QueueRecord_T *)NULL;

    	} /* End of if */
        smtp_queue.que_elements_nbr++;
    }

    SMTP_OM_UNLOCK();
}/* End of SMTP_OM_QueueEnqueue() */

/* FUNCTION NAME: SMTP_OM_QueueDequeue
 * PURPOSE: This function is used to dequeue smtp entry.
 * INPUT:   *smtp_queue -- smtp queue.
 * OUTPUT:  None.
 * RETUEN:  smtp event data
 * NOTES:   None.
 *
 */
SMTP_OM_QueueRecord_T *SMTP_OM_QueueDequeue(void)
{
    SMTP_OM_QueueRecord_T   *smtp_data =NULL;

    SMTP_OM_LOCK();

    /* Queue is not empty
	 */
    if (smtp_queue.front != (SMTP_OM_QueueRecord_T *)NULL)
    {
        smtp_data = smtp_queue.front;		    /* Return the first element */
        smtp_queue.front = smtp_data->next;	/* Move queue head to next element	*/

        if (smtp_queue.front == (SMTP_OM_QueueRecord_T *)NULL)
    	{
            smtp_queue.rear = (SMTP_OM_QueueRecord_T *)NULL;	   /*  queue is empty */
        } /* End of if */
        smtp_queue.que_elements_nbr--;
    }

    SMTP_OM_UNLOCK();

    return smtp_data;
}/* End of SMTP_OM_QueueDequeue() */

/* FUNCTION NAME: SMTP_OM_ClearQueue
 * PURPOSE: This function is used to clear the smtp entry queue.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *
 */
void SMTP_OM_ClearQueue(void)
{
    SMTP_OM_QueueRecord_T   *smtp_data;

    if (smtp_queue.front == (SMTP_OM_QueueRecord_T *)NULL)
    {
        return;
    }

    SMTP_OM_LOCK();

    while(smtp_queue.que_elements_nbr != 0)
    {
        /* More items in the queue.*/
        smtp_data = smtp_queue.front;			   /* Return the first element */
        smtp_queue.front = smtp_data->next;		/* Move queue head to next element	*/

        L_MM_Free((void *)smtp_data);

        if (smtp_queue.front == (SMTP_OM_QueueRecord_T *)NULL)
	    {
            smtp_queue.rear = (SMTP_OM_QueueRecord_T *)NULL;	   /*  queue is empty */
        } /* End of if */
        smtp_queue.que_elements_nbr--;
    }

    SMTP_OM_UNLOCK();
}

void
SMTP_OM_SetTaskId(
    UI32_T task_id)
{
    smtp_task_id = task_id;
}

UI32_T
SMTP_OM_GetTaskId(
    void)
{
    return smtp_task_id;
}
