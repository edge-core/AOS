/**************************************************************
 * Copyright (C) 2001 Alex Rozin, Optical Access
 *
 *                     All Rights Reserved
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * ALEX ROZIN DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * ALEX ROZIN BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 ******************************************************************/

/* We will allocate the memory when a row is created, if it has not enough
 * memory, then a row created will failed.The maximum entries we can created is defined
 * in SYS_ADPT_MAX_NBR_OF_RMON_ALARM_ENTRY. This constant is used in the function
 * init_alarm for initilize the maximum entry of Rmon 's group.
 * All Groups need to memory allocate a RMON_ENTRY_T plus it Ctrl_T. Since the memory is
 * not pre-allocated, we should ensured the system has enough memorys reserved for RMON to
 * satisy the spec required entry.
 *
 * The detail information is as follow:
 * RMON_ENTRY_T = 44
 *
 * RMON_Statisics_Ctrl_T = 588
 *
 * RMON_HISTORY_Data ENTRY_T = 84
 * RMON_History_Ctrl_T = 656
 *
 * RMON_alarm_Ctrl_T = 560
 *
 * RMON_ EVENT_Data_ENTRY_T = 16
 * RMON_EVENT _Ctrl_T = 60
 *
 * Group Statistics: RMON_ENTRY_T +  RMON_Statisics Ctrl_T+ strlen(owners)  ~= 44+ 588+ 32= 664
 *
 *
 * History Group:    RMON_ENTRY_T + RMON_History_Ctrl_T+ NO_OF_SAMPLES* RMON_HISTORY_Data ENTRY_T+ strlen(owners)
 *                ~= 656 + 44 +   NO_OF_SAMPLES*84 + 32 = 1404
 *
 * (Note: The etherHistory Entry will not malloc until a history log is created, but we should compute the max memory
 *        need when the etherhistory Entry is Full, here assume the no. of samples is 8.)
 *
 *
 * Alarm Group:   RMON_ENTRY_T + RMON_alarm  Ctrl_T + strlen(owners) ~= 44 +560 + 32 = 636
 *
 *
 * Event Group:   RMON_ENTRY_T +  RMON event _Ctrl_T + NO_OF_LOG_ENTRIES* ( RMON_ EVENT_Data_ENTRY_T +
 *               strlen(log_description)) + strlen(owners)+ strlen(community)+ strlen(event_description)
 *
 *              ~= 44 + 60 + 8*(16+314) + 32+32+32 = 104+ 20+ 8*330+96= 2860
 * kinghong, 2003, 6/9
 */


#include "netsnmp_port.h"
#ifndef VXWORKS
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "util_funcs.h"
#include "alarm.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "l_mm.h"
#include "swctrl.h"
#include "swctrl_pom.h"
    /*
     * Implementation headers
     */
#include "agutil_api.h"
#include "row_api.h"
#include "rows.h"

    /*
     * File scope definitions section
     */
    /*
     * from MIB compilation
     */
#define alarmEntryFirstIndexBegin       11
#define MMM_MAX				0xFFFFFFFFl
#define IDalarmIndex                    1
#define IDalarmInterval                 2
#define IDalarmVariable                 3
#define IDalarmSampleType               4
#define IDalarmValue                    5
#define IDalarmStartupAlarm             6
#define IDalarmRisingThreshold          7
#define IDalarmFallingThreshold         8
#define IDalarmRisingEventIndex         9
#define IDalarmFallingEventIndex        10
#define IDalarmOwner                    11
#define IDalarmStatus                   12
#define MIN_alarmEventIndex             0
#define MAX_alarmEventIndex             65535

#define ALARM_DEFAULT_INTERVAL                  30
#define ALARM_DEFAULT_VARIABLE_WITH_KEY_ARR     {1, 3, 6, 1, 2, 1, 16, 1, 1, 1, 6, 1}
#define ALARM_DEFAULT_VARIABLE_WITHOUT_KEY_STR  "1.3.6.1.2.1.16.1.1.1.6"
#define ALARM_DEFAULT_SAMPLE_TYPE               VAL_alarmSampleType_deltaValue
#define ALARM_DEFAULT_STARTUP_ALARM             VAL_alarmStartupAlarm_risingOrFallingAlarm
#define ALARM_DEFAULT_RISING_THRESHOLD          892800
#define ALARM_DEFAULT_FALLING_THRESHOLD         446400
#define ALARM_DEFAULT_RISING_EVENT_INDEX        0
#define ALARM_DEFAULT_FALLING_EVENT_INDEX       0
#define ALARM_DEFAULT_OWNER                     ""

     typedef enum {
         SAMPLE_TYPE_ABSOLUTE =
             1,
         SAMPLE_TYPE_DELTE
     } SAMPLE_TYPE_T;

     typedef enum {
         ALARM_NOTHING =
             0,
         ALARM_RISING,
         ALARM_FALLING,
         ALARM_BOTH
     } ALARM_TYPE_T;

     typedef struct {
         u_long
             interval;
         u_long
             timer_id;
         VAR_OID_T
             var_name;
         SAMPLE_TYPE_T
             sample_type;
         ALARM_TYPE_T
             startup_type;      /* RISING | FALLING | BOTH */

         u_long
             rising_threshold;
         u_long
             falling_threshold;
         u_long
             rising_event_index;
         u_long
             falling_event_index;

         u_long
             last_abs_value;
         u_long
             value;
         ALARM_TYPE_T
             prev_alarm;        /* NOTHING | RISING | FALLING */
         u_long
             prev_value;        /* record the previous sample value */
         BOOL_T
             first_sample_flag; /* record if the first sample */
     } CRTL_ENTRY_T;

/*
 * Main section
 */

     static TABLE_DEFINTION_T
         AlarmCtrlTable;
     static TABLE_DEFINTION_T *
         table_ptr = &
         AlarmCtrlTable;

#if 0                           /* KUKU */
     static u_long
         kuku_sum =
         0,
         kuku_cnt =
         0;
#endif

static int dflt_create_ar[SYS_ADPT_MAX_NBR_OF_RMON_ALARM_ENTRY+1];

static void get_alarm_entry(RMON_ENTRY_T *rmon_p, SNMP_MGR_RmonAlarmEntry_T *entry_p);
static BOOL_T create_alarm_entry(UI32_T id, UI32_T interval, VAR_OID_T *variable_p, UI32_T sample_type, UI32_T startup_alarm, UI32_T rising_threshold, UI32_T falling_threshold, UI32_T rising_event_index, UI32_T falling_event_index, char *owner_p, UI32_T status);

/*
 * find & enjoy it in event.c
 */
     extern int
     event_api_send_alarm(u_char is_rising,
                          u_long alarm_index,
                          u_long event_index,
                          oid * alarmed_var,
                          size_t alarmed_var_length,
                          u_long sample_type,
                          u_long value,
                          u_long the_threshold,
                          u_long interval, char *alarm_descr);

#ifdef VXWORKS
/*  kinghong  modified the fetch_var_val,  the original one is not  work*/
static int       fetch_var_val(oid * name, size_t namelen, u_long * new_value)
 {
    netsnmp_subtree *tree_ptr;
    WriteMethod    *write_method=NULL;
     struct variable compat_var, *cvp = &compat_var;
    register u_char *access;
      struct variable *vp;
         int             exact = 1;
          size_t          len;

  /* for method 1*/
   Netsnmp_Node_Handler *nh;
   netsnmp_mib_handler *dump_handler=NULL;
   netsnmp_handler_registration dump_reginfo;
  netsnmp_agent_request_info  dump_reqinfo;
   netsnmp_request_info     dump_requests;
   netsnmp_variable_list dump_var;
   netsnmp_mib_handler *temp_handler;

   int ret;

   /* init the four dump structure to 0*/
//   memset(&dump_handler,0,sizeof(dump_handler));
   memset(&dump_reginfo,0,sizeof(dump_reginfo));
   memset(&dump_reqinfo,0,sizeof(dump_reqinfo));
   memset(&dump_requests,0,sizeof(dump_requests));
   memset( &dump_var,0,sizeof(dump_var));

    dump_requests.requestvb  = &dump_var;


    tree_ptr = netsnmp_subtree_find(name, namelen, NULL, "");
    if (!tree_ptr)
    {

        return SNMP_ERR_NOSUCHNAME;
    }


   /*  There are 2 method to fetch snmp variables "serialize"   and  "old_api"*/

/*method 1 begin*/
   if  ( strcmp("serialize", tree_ptr->reginfo->handler->next->handler_name) ==  0)
 {
        /* method 1, scalar variable is implement in this way using mib2c -c mib2c.scalar.conf */
        if (tree_ptr->reginfo->handler)
        {
            temp_handler=tree_ptr->reginfo->handler;
            while (temp_handler->next)
                temp_handler=  temp_handler->next;

         }
         else
         {
         	return SNMP_ERR_NOSUCHNAME;
        }

         if  (  temp_handler->access_method)
         {
                  nh =  temp_handler->access_method;
                  dump_reqinfo.mode =MODE_GET;
          }
         else
               return SNMP_ERR_NOSUCHNAME;


          ret=  (*nh) (dump_handler, &dump_reginfo, &dump_reqinfo, &dump_requests);



        if ( ret != SNMP_ERR_NOERROR)
           return SNMP_ERR_GENERR;

         if (dump_requests.requestvb)
        {
                  switch( dump_requests.requestvb->type)
                  {
                    case ASN_INTEGER:
                    case ASN_COUNTER:
                    case ASN_TIMETICKS:
                    case ASN_GAUGE:
                    case ASN_COUNTER64:
                        break;
                    default:
                        return SNMP_ERR_GENERR;
                    }

                  *new_value=*(u_long *) dump_requests.requestvb->val.integer;
                  return SNMP_ERR_NOERROR;
        }
        else
             return SNMP_ERR_NOSUCHNAME;
   }
  /* method 2 begin*/
    else if (strcmp("old_api", tree_ptr->reginfo->handler->next->handler_name) ==  0)
   {


    if (tree_ptr->reginfo->handler->next->myvoid)
       vp = (struct variable *) tree_ptr->reginfo->handler->next->myvoid;
    else
    {

      return SNMP_ERR_NOSUCHNAME;
     }
      memcpy(cvp->name, tree_ptr->reginfo->rootoid,
           tree_ptr->reginfo->rootoid_len * sizeof(oid));
    cvp->namelen = tree_ptr->reginfo->rootoid_len;
    cvp->type = vp->type;
    cvp->magic = vp->magic;
    cvp->acl = vp->acl;
    cvp->findVar = vp->findVar;

       if (vp && vp->findVar)
                access = (*(vp->findVar)) (cvp, name,
                                           &namelen, exact, &len,
                                           &write_method);
            else
                access = NULL;


                if (access)
                {


	                    /*
	                     * check 'var_len' ?
	                     */

	                    /*
	                     * check type
	                     */
	                    switch (cvp->type)
	                     {
	                    case ASN_INTEGER:
	                    case ASN_COUNTER:
	                    case ASN_TIMETICKS:
	                    case ASN_GAUGE:
	                    case ASN_COUNTER64:
	                        break;
	                    default:
	                        return SNMP_ERR_GENERR;
	                    }


                    *new_value = *(u_long *) access;
                      return SNMP_ERR_NOERROR;
                   }
                return SNMP_ERR_NOSUCHNAME;

          }/*end of method 2*/
       else
           return SNMP_ERR_NOSUCHNAME;


        }

#else
static int
fetch_var_val(oid * name, size_t namelen, u_long * new_value)
{
    netsnmp_subtree *tree_ptr;
    size_t          var_len;
    WriteMethod    *write_method;
    struct variable called_var;
    register struct variable *s_var_ptr = NULL;
    register int    iii;
    register u_char *access;


    tree_ptr = netsnmp_subtree_find(name, namelen, NULL, "");
    if (!tree_ptr) {
        ag_trace("tree_ptr is NULL");
        return SNMP_ERR_NOSUCHNAME;
    }


    memcpy(called_var.name, tree_ptr->name_a,
           tree_ptr->namelen * sizeof(oid));

    if (tree_ptr->reginfo &&
        tree_ptr->reginfo->handler &&
        tree_ptr->reginfo->handler->next &&
        tree_ptr->reginfo->handler->next->myvoid) {
        s_var_ptr = (struct variable *)tree_ptr->reginfo->handler->next->myvoid;
    }

    if (s_var_ptr) {
        if (s_var_ptr->namelen) {
                called_var.namelen =
                                   tree_ptr->namelen;
                called_var.type = s_var_ptr->type;
                called_var.magic = s_var_ptr->magic;
                called_var.acl = s_var_ptr->acl;
                called_var.findVar = s_var_ptr->findVar;
                access =
                    (*(s_var_ptr->findVar)) (&called_var, name, &namelen,
                                             1, &var_len, &write_method);

                if (access
                    && snmp_oid_compare(name, namelen, tree_ptr->end_a,
                                        tree_ptr->end_len) > 0) {
                    memcpy(name, tree_ptr->end_a, tree_ptr->end_len);
                    access = 0;
                    ag_trace("access := 0");
                }

                if (access) {

                    /*
                     * check 'var_len' ?
                     */

                    /*
                     * check type
                     */
                    switch (called_var.type) {
                    case ASN_INTEGER:
                    case ASN_COUNTER:
                    case ASN_TIMETICKS:
                    case ASN_GAUGE:
                    case ASN_COUNTER64:
                        break;
                    default:
                        ag_trace("invalid type: %d",
                                 (int) called_var.type);
                        return SNMP_ERR_GENERR;
                    }
                    *new_value = *(u_long *) access;
                    return SNMP_ERR_NOERROR;
                }
            }
        }

    return SNMP_ERR_NOSUCHNAME;
}
#endif

static void
alarm_check_var(unsigned int clientreg, void *clientarg)
{
    RMON_ENTRY_T   *hdr_ptr;
    CRTL_ENTRY_T   *body;
    u_long          new_value;
    int             ierr;

    hdr_ptr = (RMON_ENTRY_T *) clientarg;
    if (!hdr_ptr) {
        ag_trace
            ("Err: history_get_backet: hdr_ptr=NULL ? (Inserted in shock)");
        return;
    }

    body = (CRTL_ENTRY_T *) hdr_ptr->body;
    if (!body) {
        ag_trace
            ("Err: history_get_backet: body=NULL ? (Inserted in shock)");
        return;
    }

    if (RMON1_ENTRY_VALID != hdr_ptr->status) {
        ag_trace("Err: history_get_backet when entry %d is not valid ?!!",
                 (int) hdr_ptr->ctrl_index);
        snmp_alarm_unregister(body->timer_id);
        return;
    }

    ierr = fetch_var_val(body->var_name.objid,
                         body->var_name.length, &new_value);
    if (SNMP_ERR_NOERROR != ierr) {
        ag_trace("Err: Can't fetch var_name");
        return;
    }

    body->value = (SAMPLE_TYPE_ABSOLUTE == body->sample_type) ?
        new_value : new_value - body->last_abs_value;
    body->last_abs_value = new_value;
    /*
     * ag_trace ("fetched value=%ld check %ld", (long) new_value, (long) body->value);
     */
#if 0                           /* KUKU */
    kuku_sum += body->value;
    kuku_cnt++;
#endif
#if 0
/* kinghong modified the original code  with the following, the original has the following problem*/
/*1. body->sample_type always == RISING_ALARM                                                                   */
/*2. it has not check the startup type here, so rising or falling alarm will both generate even if the    */
/*    startup type is rising alarm only                                                                                                  */
/*3. it has not bind the value of body->interval to the event_api_send_alarm which we will need    */
/*    this info to generate event log                                                                             		     */

    if (ALARM_RISING != body->prev_alarm &&
        body->value >= body->rising_threshold)
     {
         body->prev_alarm = ALARM_RISING;

        if  ( (body->startup_type ==ALARM_RISING) || ( body->startup_type == ALARM_BOTH))
       {
                                                event_api_send_alarm(1, hdr_ptr->ctrl_index,
                                                 body->rising_event_index,
                                                 body->var_name.objid,
                                                 body->var_name.length,
                                                 body->sample_type, body->value,
                                                 body->rising_threshold,
     /*kinghong added*/             body->interval,
                                                 "Rising");
         }
      }
    else if (ALARM_FALLING != body->prev_alarm &&
             body->value <= body->falling_threshold)
         {
            body->prev_alarm = ALARM_FALLING;

             if  ( (body->startup_type ==ALARM_FALLING) || ( body->startup_type == ALARM_BOTH))
                 {
                                                      event_api_send_alarm(0,
                                                      hdr_ptr->ctrl_index,
                                                      body->
                                                      falling_event_index,
                                                      body->var_name.objid,
                                                      body->var_name.
                                                      length, body->sample_type,
                                                      body->value,
                                                      body->falling_threshold,
         /*kinghong added*/                           body->interval,
                                                      "Falling");
                    }
               }

#endif

    if (body->value >= body->rising_threshold)
    {
        /* ALARM_NOTHING respresents first sample */
        if((body->first_sample_flag == TRUE && (body->startup_type == ALARM_RISING || body->startup_type == ALARM_BOTH))
           || (ALARM_RISING != body->prev_alarm && body->first_sample_flag == FALSE && body->prev_value < body->rising_threshold))
        {
            event_api_send_alarm(1, hdr_ptr->ctrl_index,
                                                 body->rising_event_index,
                                                 body->var_name.objid,
                                                 body->var_name.length,
                                 body->sample_type, body->value,
                                                 body->rising_threshold,
         /*kinghong added*/      body->interval,
                                 "Rising");
        body->prev_alarm = ALARM_RISING;
        }

    }
    else if (body->value <= body->falling_threshold)
    {
        /* ALARM_NOTHING respresents first sample */
        if((body->first_sample_flag == TRUE && (body->startup_type == ALARM_FALLING || body->startup_type == ALARM_BOTH))
           || (ALARM_FALLING != body->prev_alarm && body->first_sample_flag == FALSE && body->prev_value > body->falling_threshold))
    	{
             event_api_send_alarm(0, hdr_ptr->ctrl_index,
                                  body->falling_event_index,
                                                      body->var_name.objid,
                                  body->var_name.length,
                                  body->sample_type,
                                                      body->value,
                                  body->falling_threshold,
         /*kinghong added*/       body->interval,
                                  "Falling");
        body->prev_alarm = ALARM_FALLING;
    	}
    }

 	if(body->first_sample_flag == TRUE)
		body->first_sample_flag = FALSE;

	body->prev_value = body->value;

}

/*
 * Control Table RowApi Callbacks
 */

int
alarm_Create(RMON_ENTRY_T * eptr)
{                               /* create the body: alloc it and set defaults */
    CRTL_ENTRY_T   *body;
    static VAR_OID_T DEFAULT_VAR = { 2,
        {0,0}
    };


    eptr->body = AGMALLOC(sizeof(CRTL_ENTRY_T));
    if (!eptr->body)
        return -3;
    body = (CRTL_ENTRY_T *) eptr->body;

    /*
     * set defaults
     */
    body->interval = 30;
    body->value = 0;
    memcpy(&body->var_name, &DEFAULT_VAR, sizeof(VAR_OID_T));
    body->sample_type = SAMPLE_TYPE_DELTE;
    body->startup_type = ALARM_BOTH;
    body->rising_threshold = 0;
    body->falling_threshold = 0;
    body->rising_event_index = body->falling_event_index = 0;

    body->prev_alarm = ALARM_NOTHING;

    return 0;
}

int
alarm_Validate(RMON_ENTRY_T * eptr)
{
    CRTL_ENTRY_T   *body = (CRTL_ENTRY_T *) eptr->body;
/* rising threshold must be >= falling threshold except default when rising=falling =0*/  /* check interval must not less than  0*/
    if  ((body->rising_threshold <= body->falling_threshold) &&( (body->rising_threshold !=0)||(body->falling_threshold!=0))  )
    {

        return SNMP_ERR_BADVALUE;
    }

    return 0;
}

int
alarm_Activate(RMON_ENTRY_T * eptr)
{
    CRTL_ENTRY_T   *body = (CRTL_ENTRY_T *) eptr->body;
    int             ierr;

#if 0                           /* KUKU */
    kuku_sum = 0;
    kuku_cnt = 0;
#endif
    ierr = fetch_var_val(body->var_name.objid,
                         body->var_name.length, &body->last_abs_value);
    if (SNMP_ERR_NOERROR != ierr) {
        ag_trace("Can't fetch var_name");
        return ierr;
    }

    body->prev_alarm = ALARM_NOTHING;
    body->prev_value = 0;
    body->value = 0; /* body->sample_type = SAMPLE_TYPE_DELTE */
    body->first_sample_flag = TRUE;

    if (SAMPLE_TYPE_ABSOLUTE == body->sample_type) {

        /* if generating alarm, must assign body->value value */
        body->value = body->last_abs_value;
        body->first_sample_flag = FALSE;

        /*
         * check startup alarm
         */
        if (ALARM_RISING == body->startup_type ||
            ALARM_BOTH == body->startup_type) {
            if (body->last_abs_value >= body->rising_threshold) {
                event_api_send_alarm(1, eptr->ctrl_index,
                                     body->rising_event_index,
                                     body->var_name.objid,
                                     body->var_name.length,
                                     body->sample_type, body->value,
                                     body->rising_threshold,
   /*kinghong added*/   body->interval,
                                     "Startup Rising");
                body->prev_alarm = ALARM_RISING;
            }
        }

        if (ALARM_FALLING == body->startup_type ||
            ALARM_BOTH == body->startup_type) {
            if (body->last_abs_value <= body->falling_threshold) {
                event_api_send_alarm(0, eptr->ctrl_index,
                                     body->falling_event_index,
                                     body->var_name.objid,
                                     body->var_name.length,
                                     body->sample_type, body->value,
                                     body->falling_threshold,
   /*kinghong added*/   body->interval,
                                     "Startup Falling");
                body->prev_alarm = ALARM_FALLING;
            }
        }

    }

    body->timer_id = snmp_alarm_register(body->interval, SA_REPEAT,
                                         alarm_check_var, eptr);
    return 0;
}

int
alarm_Deactivate(RMON_ENTRY_T * eptr)
{
    CRTL_ENTRY_T   *body = (CRTL_ENTRY_T *) eptr->body;

    snmp_alarm_unregister(body->timer_id);
#if 0                           /* KUKU */
    ag_trace("kuku_sum=%ld kuku_cnt=%ld sp=%ld",
             (long) kuku_sum, (long) kuku_cnt,
             (long) (kuku_sum / kuku_cnt));
#endif
    return 0;
}

int
alarm_Copy(RMON_ENTRY_T * eptr)
{
    CRTL_ENTRY_T   *body = (CRTL_ENTRY_T *) eptr->body;
    CRTL_ENTRY_T   *clone = (CRTL_ENTRY_T *) eptr->tmp;

    if (RMON1_ENTRY_VALID == eptr->status &&
        clone->rising_threshold <= clone->falling_threshold) {
        ag_trace("alarm_Copy failed: invalid thresholds");
        return SNMP_ERR_BADVALUE;
    }

    if (clone->interval != body->interval) {
        if (RMON1_ENTRY_VALID == eptr->status) {
            snmp_alarm_unregister(body->timer_id);
            body->timer_id =
                snmp_alarm_register(clone->interval, SA_REPEAT,
                                    alarm_check_var, eptr);
        }
        body->interval = clone->interval;
    }

    if (snmp_oid_compare(clone->var_name.objid, clone->var_name.length,
                         body->var_name.objid, body->var_name.length)) {
        memcpy(&body->var_name, &clone->var_name, sizeof(VAR_OID_T));
    }

    body->sample_type = clone->sample_type;
    body->startup_type = clone->startup_type;
    body->sample_type = clone->sample_type;
    body->rising_threshold = clone->rising_threshold;
    body->falling_threshold = clone->falling_threshold;
    body->rising_event_index = clone->rising_event_index;
    body->falling_event_index = clone->falling_event_index;
    /*
     * ag_trace ("alarm_Copy: rising_threshold=%lu falling_threshold=%lu",
     * body->rising_threshold, body->falling_threshold);
     */
    return 0;
}

static int
write_alarmEntry(int action, u_char * var_val, u_char var_val_type,
                 size_t var_val_len, u_char * statP,
                 oid * name, size_t name_len)
{
    long            long_tmp;
    int             leaf_id, snmp_status;
    static int      prev_action = COMMIT;
    RMON_ENTRY_T   *hdr;
    CRTL_ENTRY_T   *cloned_body;
    CRTL_ENTRY_T   *body;

    /* kinghong add the following sematic check for setting Rmon EntryStatus.
     * 1. Set Valid(1)              Permit original state:  Valid(1),UnderCreation(3).
     * 2. Set CreateRequest(2)      Permit original state:  Not Exist.
     * 3. Set UnderCreation(3)      Permit original state:  Valid(1), UnderCreation(3).
     * 4. Set Invalid(4)            Permit original state:  Valid(1), UnderCreation(3).
     */

    if (name[alarmEntryFirstIndexBegin - 1] == IDalarmStatus)
    {
         /* This is the 'set' value from MIB*/
        memcpy(&long_tmp, var_val, sizeof(long));

        /* Step 1: Check current Status before do anything*/
        hdr = ROWAPI_find(table_ptr, name[alarmEntryFirstIndexBegin]);

        /*Step 2: Check if the data exist*/
        if (!hdr) /*: not exist, only can perform create request */
        {
            if (long_tmp != RMON1_ENTRY_CREATE_REQUEST )
            {
                return SNMP_ERR_BADVALUE;
            }

        }
        /*:already exist, block any request that want to set create request.
         */
        else if ((long_tmp == RMON1_ENTRY_CREATE_REQUEST) && (action == 0))
        {
            return SNMP_ERR_BADVALUE;
        }
    }

    switch (action) {
    case RESERVE1:
    case FREE:
    case UNDO:
    case ACTION:
    case COMMIT:
    default:
        return ROWAPI_do_another_action(name, alarmEntryFirstIndexBegin,
                                        action, &prev_action,
                                        table_ptr, sizeof(CRTL_ENTRY_T));
    case RESERVE2:
        /*
         * get values from PDU, check them and save them in the cloned entry
         */
        long_tmp = name[alarmEntryFirstIndexBegin];
        leaf_id = (int) name[alarmEntryFirstIndexBegin - 1];
        hdr = ROWAPI_find(table_ptr, long_tmp);
        if (NULL == hdr)
        {
            return SNMP_ERR_NOSUCHNAME;
        }

        cloned_body = (CRTL_ENTRY_T *) hdr->tmp;
        body = (CRTL_ENTRY_T *) hdr->body;

        if (   (NULL == cloned_body)
            || (NULL == body)
            )
        {
            return SNMP_ERR_GENERR;
        }

        switch (leaf_id)
        {
            case IDalarmInterval:
            {
                UI32_T  value = * (long *) var_val;
				if (var_val_type != ASN_INTEGER)
					return SNMP_ERR_WRONGTYPE;

                snmp_status = AGUTIL_get_int_value(var_val, var_val_type,
                                                   var_val_len,
                                                   MIN_alarmInterval, MAX_alarmInterval, &long_tmp);
                if (SNMP_ERR_NOERROR != snmp_status)
                {
                    return snmp_status;
                }
                cloned_body->interval = long_tmp;
                break;
            }

            case IDalarmVariable:
            {
                u_long dump_value;
                register oid *oid_var;

                oid_var = (oid *) var_val;
				if (var_val_type != ASN_OBJECT_ID)
					return SNMP_ERR_WRONGTYPE;
                if (fetch_var_val(oid_var, var_val_len/4, &dump_value)
		            != SNMP_ERR_NOERROR)
                {
                    return SNMP_ERR_BADVALUE;
                }

                snmp_status = AGUTIL_get_oid_value(var_val, var_val_type,
                                                   var_val_len,
                                                   &cloned_body->var_name);
                if (SNMP_ERR_NOERROR != snmp_status)
                {
                    return snmp_status;
                }
                if (RMON1_ENTRY_UNDER_CREATION != hdr->status &&
                    snmp_oid_compare(cloned_body->var_name.objid,
                                     cloned_body->var_name.length,
                                     body->var_name.objid,
                                     body->var_name.length))
                {
                    return SNMP_ERR_BADVALUE;
                }
                break;
            }

            case IDalarmSampleType:
				if (var_val_type != ASN_INTEGER)
					return SNMP_ERR_WRONGTYPE;
                snmp_status = AGUTIL_get_int_value(var_val, var_val_type,
                                                   var_val_len,
                                                   SAMPLE_TYPE_ABSOLUTE,
                                                   SAMPLE_TYPE_DELTE,
                                                   &long_tmp);
                if (SNMP_ERR_NOERROR != snmp_status)
                {
                    return snmp_status;
                }
                cloned_body->sample_type = long_tmp;
                break;
            case IDalarmStartupAlarm:
				if (var_val_type != ASN_INTEGER)
					return SNMP_ERR_WRONGTYPE;
                snmp_status = AGUTIL_get_int_value(var_val, var_val_type,
                                                   var_val_len,
                                                   ALARM_RISING,
                                                   ALARM_BOTH, &long_tmp);
                if (SNMP_ERR_NOERROR != snmp_status)
                {
                    return snmp_status;
                }
                cloned_body->startup_type = long_tmp;
                break;
            case IDalarmRisingThreshold:
				if (var_val_type != ASN_INTEGER)
					return SNMP_ERR_WRONGTYPE;
                snmp_status = AGUTIL_get_int_value(var_val, var_val_type,
                                                   var_val_len,
                                                   SNMP_MGR_MIN_RMON_ALARM_RISING_THRESHOLD,
                                                   SNMP_MGR_MAX_RMON_ALARM_RISING_THRESHOLD, &long_tmp);
                if (SNMP_ERR_NOERROR != snmp_status)
                {
                    return snmp_status;
                }

                if (long_tmp <= body->falling_threshold)
                {
                    return SNMP_ERR_WRONGVALUE;
                }

                cloned_body->rising_threshold = long_tmp;
                break;
            case IDalarmFallingThreshold:
				if (var_val_type != ASN_INTEGER)
					return SNMP_ERR_WRONGTYPE;
                snmp_status = AGUTIL_get_int_value(var_val, var_val_type,
                                                   var_val_len,
                                                   SNMP_MGR_MIN_RMON_ALARM_FALLING_THRESHOLD,
                                                   SNMP_MGR_MAX_RMON_ALARM_FALLING_THRESHOLD,
                                                   &long_tmp);
                if (SNMP_ERR_NOERROR != snmp_status)
                {
                    return snmp_status;
                }

                if (body->rising_threshold <= long_tmp)
                {
                    return SNMP_ERR_WRONGVALUE;
                }

                cloned_body->falling_threshold = long_tmp;
                break;
            case IDalarmRisingEventIndex:
				if (var_val_type != ASN_INTEGER)
					return SNMP_ERR_WRONGTYPE;
                snmp_status = AGUTIL_get_int_value(var_val, var_val_type, var_val_len, MIN_alarmRisingEventIndex,   /* min. value */
                                                   MAX_alarmRisingEventIndex,       /* max. value */
                                                   &long_tmp);
                if (SNMP_ERR_NOERROR != snmp_status)
                {
                    return snmp_status;
                }
                cloned_body->rising_event_index = long_tmp;
                break;
            case IDalarmFallingEventIndex:
				if (var_val_type != ASN_INTEGER)
					return SNMP_ERR_WRONGTYPE;
                snmp_status = AGUTIL_get_int_value(var_val, var_val_type, var_val_len, MIN_alarmFallingEventIndex,   /* min. value */
                                                   MAX_alarmFallingEventIndex,       /* max. value */
                                                   &long_tmp);
                if (SNMP_ERR_NOERROR != snmp_status)
                {
                    return snmp_status;
                }
                cloned_body->falling_event_index = long_tmp;
                break;
            case IDalarmOwner:
                if (hdr->new_owner)
                    AGFREE(hdr->new_owner);
                hdr->new_owner = AGMALLOC(MAX_OWNERSTRING + 1);
				if (var_val_type != ASN_OCTET_STR)
					return SNMP_ERR_WRONGTYPE;
                if (!hdr->new_owner)
                    return SNMP_ERR_TOOBIG;
                snmp_status = AGUTIL_get_string_value(var_val, var_val_type,
                                                      var_val_len,
                                                      MAX_OWNERSTRING,
                                                      1, NULL, hdr->new_owner);
                if (SNMP_ERR_NOERROR != snmp_status)
                {
                    return snmp_status;
                }
                break;

            case IDalarmStatus:
				if (var_val_type != ASN_INTEGER)
					return SNMP_ERR_WRONGTYPE;
                snmp_status = AGUTIL_get_int_value(var_val, var_val_type,
                                                   var_val_len,
                                                   RMON1_ENTRY_VALID,
                                                   RMON1_ENTRY_INVALID,
                                                   &long_tmp);
                if (SNMP_ERR_NOERROR != snmp_status)
                {
                    return snmp_status;
                }
                hdr->new_status = long_tmp;
                break;
            default:
                ag_trace("%s:unknown leaf_id=%d\n", table_ptr->name,
                    (int) leaf_id);
                return SNMP_ERR_NOSUCHNAME;
        }                       /* of switch by 'leaf_id' */

        break;
    }                           /* of switch by actions */

    prev_action = action;
    return SNMP_ERR_NOERROR;
}

u_char         *
var_alarmEntry(struct variable * vp, oid * name, size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method)
{
    static long     long_return;
    static CRTL_ENTRY_T theEntry;
    RMON_ENTRY_T   *hdr;

    switch(vp->magic)
    {
        case IDalarmInterval:
        case IDalarmVariable:
        case IDalarmSampleType:
        case IDalarmStartupAlarm:
        case IDalarmRisingThreshold:
        case IDalarmFallingThreshold:
        case IDalarmRisingEventIndex:
        case IDalarmFallingEventIndex:
        case IDalarmOwner:
        case IDalarmStatus:
            *write_method = write_alarmEntry;
            break;
        default:
            *write_method = 0;
            break;
    }

    hdr = ROWAPI_header_ControlEntry(vp, name, length, exact, var_len,
                                     table_ptr,
                                     &theEntry, sizeof(CRTL_ENTRY_T));
    if (!hdr)
        return NULL;

    *var_len = sizeof(long);    /* default */

    switch (vp->magic) {
    case IDalarmIndex:
        long_return = hdr->ctrl_index;
        return (u_char *) & long_return;
    case IDalarmInterval:
        long_return = theEntry.interval;
        return (u_char *) & long_return;
    case IDalarmVariable:
        *var_len = sizeof(oid) * theEntry.var_name.length;
        return (unsigned char *) theEntry.var_name.objid;
        return (u_char *) & long_return;
    case IDalarmSampleType:
        long_return = theEntry.sample_type;
        return (u_char *) & long_return;
    case IDalarmValue:
        long_return = theEntry.value;
        return (u_char *) & long_return;
    case IDalarmStartupAlarm:
        long_return = theEntry.startup_type;
        return (u_char *) & long_return;
    case IDalarmRisingThreshold:
        long_return = theEntry.rising_threshold;
        return (u_char *) & long_return;
    case IDalarmFallingThreshold:
        long_return = theEntry.falling_threshold;
        return (u_char *) & long_return;
    case IDalarmRisingEventIndex:
        long_return = theEntry.rising_event_index;
        return (u_char *) & long_return;
    case IDalarmFallingEventIndex:
        long_return = theEntry.falling_event_index;
        return (u_char *) & long_return;
    case IDalarmOwner:
        if (hdr->owner) {
            *var_len = strlen(hdr->owner);
            return (unsigned char *) hdr->owner;
        } else {
            *var_len = 0;
            return (unsigned char *) "";
        }

    case IDalarmStatus:
        long_return = hdr->status;
        return (u_char *) & long_return;
    default:
        ag_trace("%s: unknown vp->magic=%d", table_ptr->name,
                 (int) vp->magic);
        ERROR_MSG("");
    };                          /* of switch by 'vp->magic' */

    return NULL;
}

/*
 * Registration & Initializatio section
 */

oid             oidalarmVariablesOid[] = { 1, 3, 6, 1, 2, 1, 16, 3 };

struct variable7 oidalarmVariables[] = {
    {IDalarmIndex, ASN_INTEGER, RONLY, var_alarmEntry, 3, {1, 1, 1}},
    {IDalarmInterval, ASN_INTEGER, RWRITE, var_alarmEntry, 3, {1, 1, 2}},
    {IDalarmVariable, ASN_OBJECT_ID, RWRITE, var_alarmEntry, 3, {1, 1, 3}},
    {IDalarmSampleType, ASN_INTEGER, RWRITE, var_alarmEntry, 3, {1, 1, 4}},
    {IDalarmValue, ASN_INTEGER, RONLY, var_alarmEntry, 3, {1, 1, 5}},
    {IDalarmStartupAlarm, ASN_INTEGER, RWRITE, var_alarmEntry, 3,
     {1, 1, 6}},
    {IDalarmRisingThreshold, ASN_INTEGER, RWRITE, var_alarmEntry, 3,
     {1, 1, 7}},
    {IDalarmFallingThreshold, ASN_INTEGER, RWRITE, var_alarmEntry, 3,
     {1, 1, 8}},
    {IDalarmRisingEventIndex, ASN_INTEGER, RWRITE, var_alarmEntry, 3,
     {1, 1, 9}},
    {IDalarmFallingEventIndex, ASN_INTEGER, RWRITE, var_alarmEntry, 3,
     {1, 1, 10}},
    {IDalarmOwner, ASN_OCTET_STR, RWRITE, var_alarmEntry, 3, {1, 1, 11}},
    {IDalarmStatus, ASN_INTEGER, RWRITE, var_alarmEntry, 3, {1, 1, 12}}
};

void
init_alarm(void)
{
    REGISTER_MIB("alarmTable", oidalarmVariables, variable7,
                 oidalarmVariablesOid);

    ROWAPI_init_table(&AlarmCtrlTable, "Alarm", SYS_ADPT_MAX_NBR_OF_RMON_ALARM_ENTRY, &alarm_Create, NULL, /* &alarm_Clone, */
                      NULL,     /* &alarm_Delete, */
                      &alarm_Validate,
                      &alarm_Activate, &alarm_Deactivate, &alarm_Copy);


}

void ALARM_CreateDefaultEntry()
{
    UI32_T i;
    VAR_OID_T variable_oid = {12, ALARM_DEFAULT_VARIABLE_WITH_KEY_ARR};

    memset(dflt_create_ar, 0, sizeof(dflt_create_ar));

    for (i = 1; i <= SYS_ADPT_MAX_NBR_OF_RMON_ALARM_ENTRY; i++)
    {
        if (FALSE == SWCTRL_POM_LogicalPortExisting(i))
        {
            continue;
        }

        variable_oid.objid[11] = i;
        create_alarm_entry(i,
            ALARM_DEFAULT_INTERVAL,
            &variable_oid,
            ALARM_DEFAULT_SAMPLE_TYPE,
            ALARM_DEFAULT_STARTUP_ALARM,
            ALARM_DEFAULT_RISING_THRESHOLD,
            ALARM_DEFAULT_FALLING_THRESHOLD,
            ALARM_DEFAULT_RISING_EVENT_INDEX,
            ALARM_DEFAULT_FALLING_EVENT_INDEX,
            ALARM_DEFAULT_OWNER,
            VAL_alarmStatus_valid);

        /* flag to record creation entries
         */
        dflt_create_ar[i] = 1;
    }
}

void ALARM_DeleteAllRow()
{
    register RMON_ENTRY_T *eptr;
    int i;

    for (i = 1; i <= SYS_ADPT_MAX_NBR_OF_RMON_ALARM_ENTRY; i++)
    {
        eptr = ROWAPI_find(table_ptr, i);
        if (NULL == eptr)
        {
            continue;
        }

        ROWAPI_delete_clone(table_ptr, i);

        eptr = ROWAPI_find(table_ptr, i);

        if (eptr != NULL)
        {
            rowapi_delete(eptr);
        }
    }
}

BOOL_T ALARM_GetAlarmTable(SNMP_MGR_RmonAlarmEntry_T *entry_p)
{
    RMON_ENTRY_T *rmon_p;

    if (NULL == entry_p)
    {
        return FALSE;
    }

    while (1)
    {
        rmon_p = ROWAPI_find(table_ptr, entry_p->id);

        if (NULL == rmon_p)
        {
            return FALSE;
        }

        get_alarm_entry(rmon_p, entry_p);
        return TRUE;
    }

    return FALSE;
}

BOOL_T ALARM_GetNextAlarmTable(SNMP_MGR_RmonAlarmEntry_T *entry_p)
{
    RMON_ENTRY_T *rmon_p;

    if (NULL == entry_p)
    {
        return FALSE;
    }

    while (1)
    {
        rmon_p = ROWAPI_next(table_ptr, entry_p->id);

        if (NULL == rmon_p)
        {
            return FALSE;
        }

        get_alarm_entry(rmon_p, entry_p);
        return TRUE;
    }

    return FALSE;
}

BOOL_T ALARM_CreateAlarmEntry(SNMP_MGR_RmonAlarmEntry_T *entry_p)
{
    VAR_OID_T variable_oid;
    UI32_T    dst_len;

    if (FALSE == SNMP_MGR_ConvertStrToOid(entry_p->variable, variable_oid.objid, sizeof(variable_oid.objid) / sizeof(oid), &dst_len))
    {
        return FALSE;
    }
    variable_oid.length=dst_len;

    if (FALSE == create_alarm_entry(entry_p->id,
        entry_p->interval,
        &variable_oid,
        entry_p->sample_type,
        entry_p->startup_alarm,
        entry_p->rising_threshold,
        entry_p->falling_threshold,
        entry_p->rising_event_index,
        entry_p->falling_event_index,
        entry_p->owner,
        entry_p->status))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL_T ALARM_DeleteAlarmEntry(UI32_T index)
{
    RMON_ENTRY_T *rmon_p;

    rmon_p = ROWAPI_find(table_ptr, index);

    if (FALSE == rmon_p)
    {
        return FALSE;
    }

    ROWAPI_delete_clone(table_ptr, index);

    rmon_p = ROWAPI_find(table_ptr, index);

    if (rmon_p != NULL)
    {
        rowapi_delete(rmon_p);
    }

    return TRUE;
}

BOOL_T ALARM_IsAlarmEntryModified(SNMP_MGR_RmonAlarmEntry_T *entry_p)
{
#if (SYS_CPNT_RMON_ALARM_DEFAULT == TRUE)
    char dflt_variable[512];

    sprintf(dflt_variable, "%s.%lu", ALARM_DEFAULT_VARIABLE_WITHOUT_KEY_STR, entry_p->id);

    /* only default entry need check has been modified or not
    */
    if ((entry_p->id <= SYS_ADPT_MAX_NBR_OF_RMON_ALARM_ENTRY)
         && (1 == dflt_create_ar[entry_p->id]))
    {
        /* default entry but has been modified
         */
        if ( (VAL_alarmStatus_valid != entry_p->status)
                || (ALARM_DEFAULT_INTERVAL != entry_p->interval)
                || (0 != strcmp(entry_p->variable, dflt_variable))
                || (ALARM_DEFAULT_SAMPLE_TYPE != entry_p->sample_type)
                || (ALARM_DEFAULT_STARTUP_ALARM != entry_p->startup_alarm)
                || (ALARM_DEFAULT_RISING_THRESHOLD != entry_p->rising_threshold)
                || (ALARM_DEFAULT_FALLING_THRESHOLD != entry_p->falling_threshold)
                || (ALARM_DEFAULT_RISING_EVENT_INDEX != entry_p->rising_event_index)
                || (ALARM_DEFAULT_FALLING_EVENT_INDEX != entry_p->falling_event_index)
                || (0 != strcmp(ALARM_DEFAULT_OWNER, entry_p->owner))
            )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
#endif /* #if (SYS_CPNT_RMON_ALARM_DEFAULT == TRUE) */

    /* the extra added entry
     */
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - ALARM_GetNextDeletedDefaultEntry
 * ---------------------------------------------------------------------
 * PURPOSE : This function is used to get the default alarm entries
 *           that are not exist anymore.
 * INPUT   : entry_p
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None.
 * ---------------------------------------------------------------------
 */
BOOL_T ALARM_GetNextDeletedDefaultEntry(SNMP_MGR_RmonAlarmEntry_T *entry_p)
{
    RMON_ENTRY_T *rmon_p;
    UI32_T i;

    for (i = entry_p->id + 1; i <= SYS_ADPT_MAX_NBR_OF_RMON_ALARM_ENTRY; i++)
    {
        if (dflt_create_ar[i] == FALSE)
        {
            continue;
        }

        rmon_p = ROWAPI_find(table_ptr, i);

        if (rmon_p == NULL)
        {
            entry_p->id = i;
            return TRUE;
        }
    }

    return FALSE;
}

static void get_alarm_entry(RMON_ENTRY_T *rmon_p, SNMP_MGR_RmonAlarmEntry_T *entry_p)
{
    CRTL_ENTRY_T ctrl_entry;

    memcpy(&ctrl_entry, rmon_p->body, sizeof(ctrl_entry));
    entry_p->id = rmon_p->ctrl_index;
    entry_p->interval = ctrl_entry.interval;
    SNMP_MGR_ConvertOidToStr(ctrl_entry.var_name.objid, ctrl_entry.var_name.length, entry_p->variable, sizeof(entry_p->variable));
    entry_p->sample_type = ctrl_entry.sample_type;
    entry_p->value = ctrl_entry.value;
    entry_p->startup_alarm = ctrl_entry.startup_type;
    entry_p->rising_threshold = ctrl_entry.rising_threshold;
    entry_p->falling_threshold = ctrl_entry.falling_threshold;
    entry_p->rising_event_index = ctrl_entry.rising_event_index;
    entry_p->falling_event_index = ctrl_entry.falling_event_index;

    if (NULL == rmon_p->owner)
    {
        entry_p->owner[0] = '\0';
    }
    else
    {
        strncpy(entry_p->owner, rmon_p->owner, sizeof(entry_p->owner)-1);
        entry_p->owner[sizeof(entry_p->owner)-1] = '\0';
    }

    entry_p->status = rmon_p->status;
}

static BOOL_T create_alarm_entry(UI32_T id, UI32_T interval, VAR_OID_T *variable_p, UI32_T sample_type, UI32_T startup_alarm, UI32_T rising_threshold, UI32_T falling_threshold, UI32_T rising_event_index, UI32_T falling_event_index, char *owner_p, UI32_T status)
{
    RMON_ENTRY_T *rmon_p;
    CRTL_ENTRY_T *body_p;
    u_long dump_value;

    if (   (NULL == variable_p)
        || (NULL == owner_p))
    {
        return FALSE;
    }

    if (SNMP_ERR_NOERROR != fetch_var_val(variable_p->objid, variable_p->length, &dump_value))
    {
        return FALSE;
    }

    if ((interval < MIN_alarmInterval) || (interval > MAX_alarmInterval)
      ||(rising_threshold < SNMP_MGR_MIN_RMON_ALARM_RISING_THRESHOLD)
      ||(rising_threshold > SNMP_MGR_MAX_RMON_ALARM_RISING_THRESHOLD)
      ||(falling_threshold < SNMP_MGR_MIN_RMON_ALARM_FALLING_THRESHOLD)
      ||(falling_threshold > SNMP_MGR_MAX_RMON_ALARM_FALLING_THRESHOLD)
      ||(rising_event_index < MIN_alarmRisingEventIndex)
      ||(rising_event_index > MAX_alarmRisingEventIndex)
      ||(falling_event_index < MIN_alarmFallingEventIndex)
      ||(falling_event_index > MAX_alarmFallingEventIndex))
    {
        return FALSE;
    }

    if (rising_threshold <= falling_threshold)
    {
        return FALSE;
    }

    rmon_p = ROWAPI_find(table_ptr, id);

    if (NULL == rmon_p)
    {
        if (SNMP_ERR_NOERROR != ROWAPI_new(table_ptr, id))
        {
            return FALSE;
        }

        rmon_p = ROWAPI_find(table_ptr, id);

        if (NULL == rmon_p)
        {
            return FALSE;
        }
    }

    body_p = (CRTL_ENTRY_T *)rmon_p->body;
    body_p->interval = interval;
    memcpy(&body_p->var_name, variable_p, sizeof(body_p->var_name));
    body_p->sample_type = sample_type;
    body_p->startup_type = startup_alarm;
    body_p->rising_threshold = rising_threshold;
    body_p->falling_threshold = falling_threshold;
    body_p->rising_event_index = rising_event_index;
    body_p->falling_event_index = falling_event_index;

    rmon_p->new_owner = AGSTRDUP(owner_p);
    rmon_p->new_status = status;

    if (SNMP_ERR_NOERROR != ROWAPI_commit(table_ptr, id))
    {
        ROWAPI_delete_clone(table_ptr, id);

        rmon_p = ROWAPI_find(table_ptr, id);

        if (rmon_p != NULL)
        {
           rowapi_delete(rmon_p);
        }
        return FALSE;
    }

    return TRUE;
}

/*
 * end of file alarm.c
 */
