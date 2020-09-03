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
 * in SYS_ADPT_MAX_NBR_OF_RMON_EVENT_ENTRY. This constant is used in the function 
 * init_event for initilize the maximum entry of Rmon 's group. 
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
#include <ctype.h>
#else
/* system dependent header, kinghong add*/
#include "trap_mgr.h"
#include "trap_event.h"
#include "sys_adpt.h"
#include "snmp_mgr.h"
#endif /*endof #ifndef VXWORKS*/

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

/* system dependent header, kinghong add*/
#include "trap_mgr.h"
#include "trap_event.h"
#include "sys_adpt.h"

#include "util_funcs.h"

#include "event.h"

/*
 * Implementation headers 
 */
#include "agutil_api.h"
#include "row_api.h"
#include "agutil.h"
#include <ctype.h>
#include "rows.h"

/*
 * File scope definitions section 
 */

/*
 * from MIB compilation 
 */
#define eventEntryFirstIndexBegin       11

#define EVENTINDEX            3
#define EVENTDESCRIPTION      4
#define EVENTTYPE             5
#define EVENTCOMMUNITY        6
#define EVENTLASTTIMESENT     7
#define EVENTOWNER            8
#define EVENTSTATUS           9

#define Leaf_event_description  2
#define MIN_event_description   0
#define MAX_event_description   127
#define Leaf_event_type         3
#define Leaf_event_community    4
#define MIN_event_community     0
#define MAX_event_community     SYS_ADPT_MAX_RMON_OWNER_STR_LEN
#define Leaf_event_last_time_sent 5
#define Leaf_eventOwner        6
#define Leaf_eventStatus       7


#define LOGEVENTINDEX         3
#define LOGINDEX              4
#define LOGTIME               5
#define LOGDESCRIPTION        6


/*
 * defaults & limitations 
 */

#define MAX_LOG_ENTRIES_PER_CTRL	200

typedef struct data_struct_t {
    struct data_struct_t *next;
    u_long          data_index;
    u_long          log_time;
    char           *log_description;
} DATA_ENTRY_T;

typedef enum {
    EVENT_NONE = 1,
    EVENT_LOG,
    EVENT_TRAP,
    EVENT_LOG_AND_TRAP
} EVENT_TYPE_T;

typedef struct {
    char           *event_description;
    char           *event_community;
    EVENT_TYPE_T    event_type;
    u_long          event_last_time_sent;

    SCROLLER_T      scrlr;
#if 0
    u_long          event_last_logged_index;
    u_long          event_number_of_log_entries;
    DATA_ENTRY_T   *log_list;
    DATA_ENTRY_T   *last_log_ptr;
#endif
} CRTL_ENTRY_T;


/* some old rmon API for generate event decribtion, add by kinghong*/
static int concatString(UI8_T* toThis, int  toThisLen, UI8_T* addThis, int  addThisLen) ;

#if 0 /*eli,no use*/
static char    StringNumbers[12] = 
{'0','1','2','3','4','5','6','7','8','9',0,0};
#endif

//get the ascii value of numbers and then connect them into the address
//pointed by toThis
static int  concatNumber(UI8_T* toThis, int  toThisLen, int addThisNumber) ;
static void get_event_entry(RMON_ENTRY_T *rmon_p, SNMP_MGR_RmonEventEntry_T *entry_p);
static BOOL_T create_event_entry(UI32_T id, UI32_T type, char *community_p, char *description_p, char *owner_p, UI32_T status);

/*
 * Main section 
 */

static TABLE_DEFINTION_T EventCtrlTable;
static TABLE_DEFINTION_T *table_ptr = &EventCtrlTable;

/*
 * Control Table RowApi Callbacks 
 */

static int
data_destructor(SCROLLER_T * scrlr, void *free_me)
{
    DATA_ENTRY_T   *lptr = free_me;

    if (lptr->log_description)
        AGFREE(lptr->log_description);

    return 0;
}

int
event_Create(RMON_ENTRY_T * eptr)
{                               /* create the body: alloc it and set defaults */
    CRTL_ENTRY_T   *body;

    eptr->body = AGMALLOC(sizeof(CRTL_ENTRY_T));
    memset(eptr->body,0,sizeof(CRTL_ENTRY_T));
    if (!eptr->body)
        return -3;
    body = (CRTL_ENTRY_T *) eptr->body;

    /*
     * set defaults 
     */

    body->event_description = NULL;
    body->event_community = AGSTRDUP("public");
    /*
     * ag_trace ("Dbg: created event_community=<%s>", body->event_community); 
     */
    body->event_type = EVENT_NONE;
    #ifndef VXWORKS
    ROWDATAAPI_init(&body->scrlr,
                    MAX_LOG_ENTRIES_PER_CTRL,
                    MAX_LOG_ENTRIES_PER_CTRL,
                    sizeof(DATA_ENTRY_T), &data_destructor);
    #else
    ROWDATAAPI_init(&body->scrlr,
                    SYS_ADPT_MAX_NBR_OF_RMON_EVENT_LOG_ENTRY,
                    SYS_ADPT_MAX_NBR_OF_RMON_EVENT_LOG_ENTRY,
                    sizeof(DATA_ENTRY_T), &data_destructor);
     #endif
    return 0;
}

int
event_Clone(RMON_ENTRY_T * eptr)
{                               /* copy entry_bod -> clone */
    CRTL_ENTRY_T   *body = (CRTL_ENTRY_T *) eptr->body;
    CRTL_ENTRY_T   *clone = (CRTL_ENTRY_T *) eptr->tmp;

    if (body->event_description)
        clone->event_description = AGSTRDUP(body->event_description);

    if (body->event_community)
        clone->event_community = AGSTRDUP(body->event_community);
    return 0;
}

int
event_Copy(RMON_ENTRY_T * eptr)
{
    CRTL_ENTRY_T   *body = (CRTL_ENTRY_T *) eptr->body;
    CRTL_ENTRY_T   *clone = (CRTL_ENTRY_T *) eptr->tmp;

    if (body->event_type != clone->event_type) {
        body->event_type = clone->event_type;
    }

    if (clone->event_description) {
        if (body->event_description)
            AGFREE(body->event_description);
        body->event_description = AGSTRDUP(clone->event_description);
    }

    if (clone->event_community) {
        if (body->event_community)
            AGFREE(body->event_community);
        body->event_community = AGSTRDUP(clone->event_community);
    }

    return 0;
}

int
event_Delete(RMON_ENTRY_T * eptr)
{
    CRTL_ENTRY_T   *body = (CRTL_ENTRY_T *) eptr;

    if (body->event_description)
        AGFREE(body->event_description);

    if (body->event_community)
        AGFREE(body->event_community);

    return 0;
}

int
event_Activate(RMON_ENTRY_T * eptr)
{                               /* init logTable */
    CRTL_ENTRY_T   *body = (CRTL_ENTRY_T *) eptr->body;

    ROWDATAAPI_set_size(&body->scrlr,
                        body->scrlr.data_requested,
                        RMON1_ENTRY_VALID == eptr->status);

    return 0;
}

int
event_Deactivate(RMON_ENTRY_T * eptr)
{                               /* free logTable */
    CRTL_ENTRY_T   *body = (CRTL_ENTRY_T *) eptr->body;

    /*
     * free data list 
     */
    ROWDATAAPI_descructor(&body->scrlr);

    return 0;
}

static int
write_eventControl(int action, u_char * var_val, u_char var_val_type,
                   size_t var_val_len, u_char * statP,
                   oid * name, size_t name_len)
{
    long            long_temp;
    char           *char_temp;
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
    if (name[eventEntryFirstIndexBegin - 1] == Leaf_eventStatus)
    {
         /* This is the 'set' value from MIB*/
        memcpy(&long_temp, var_val, sizeof(long));
         
        /* Step 1: Check current Status before do anything*/
        hdr = ROWAPI_find(table_ptr, name[eventEntryFirstIndexBegin]);   
        
        /*Step 2: Check if the data exist*/
        if (!hdr) /*: not exist, only can perform create request */
        {
            if (long_temp != RMON1_ENTRY_CREATE_REQUEST )
            {
                return SNMP_ERR_BADVALUE;
            }
           
        }
        /*:already exist, block any request that want to set create request.
         */
        else if ((long_temp == RMON1_ENTRY_CREATE_REQUEST) && (action == 0))
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
        return ROWAPI_do_another_action(name, eventEntryFirstIndexBegin,
                                        action, &prev_action,
                                        table_ptr, sizeof(CRTL_ENTRY_T));

    case RESERVE2:
        /*
         * get values from PDU, check them and save them in the cloned entry 
         */
        long_temp = name[eventEntryFirstIndexBegin];
        leaf_id = (int) name[eventEntryFirstIndexBegin - 1];
        hdr = ROWAPI_find(table_ptr, long_temp);        /* it MUST be OK */
        cloned_body = (CRTL_ENTRY_T *) hdr->tmp;
        body = (CRTL_ENTRY_T *) hdr->body;
          /* kinghong add to check if status is valid, then forbidden to write except the status*/
          if( (hdr->status == 1) &&( leaf_id!=Leaf_eventStatus))
               return SNMP_ERR_BADVALUE;
               
        switch (leaf_id) {
        case Leaf_event_description:
            char_temp = AGMALLOC(1 + MAX_event_description);
            if (!char_temp)
                return SNMP_ERR_TOOBIG;
            snmp_status = AGUTIL_get_string_value(var_val, var_val_type,
                                                  var_val_len,
                                                  MAX_event_description,
                                                  1, NULL, char_temp);
            if (SNMP_ERR_NOERROR != snmp_status) {
                AGFREE(char_temp);
                return snmp_status;
            }

            if (cloned_body->event_description)
                AGFREE(cloned_body->event_description);

            cloned_body->event_description = AGSTRDUP(char_temp);
            /*
             * ag_trace ("rx: event_description=<%s>", cloned_body->event_description); 
             */
            AGFREE(char_temp);

            break;
        case Leaf_event_type:
            snmp_status = AGUTIL_get_int_value(var_val, var_val_type,
                                               var_val_len,
                                               EVENT_NONE,
                                               EVENT_LOG_AND_TRAP,
                                               &long_temp);
            if (SNMP_ERR_NOERROR != snmp_status) {
                return snmp_status;
            }
            cloned_body->event_type = long_temp;
            break;
        case Leaf_event_community:
            char_temp = AGMALLOC(1 + MAX_event_community);
            if (!char_temp)
                return SNMP_ERR_TOOBIG;
            snmp_status = AGUTIL_get_string_value(var_val, var_val_type,
                                                  var_val_len,
                                                  MAX_event_community,
                                                  1, NULL, char_temp);
            if (SNMP_ERR_NOERROR != snmp_status) {
                AGFREE(char_temp);
                return snmp_status;
            }

            if (cloned_body->event_community)
                AGFREE(cloned_body->event_community);

            cloned_body->event_community = AGSTRDUP(char_temp);
            AGFREE(char_temp);

            break;
        case Leaf_eventOwner:
            if (hdr->new_owner)
                AGFREE(hdr->new_owner);
            hdr->new_owner = AGMALLOC(MAX_OWNERSTRING + 1);
            if (!hdr->new_owner)
                return SNMP_ERR_TOOBIG;
            snmp_status = AGUTIL_get_string_value(var_val, var_val_type,
                                                  var_val_len,
                                                  MAX_OWNERSTRING,
                                                  1, NULL, hdr->new_owner);
            if (SNMP_ERR_NOERROR != snmp_status) {
                return snmp_status;
            }

            break;
        case Leaf_eventStatus:
            snmp_status = AGUTIL_get_int_value(var_val, var_val_type,
                                               var_val_len,
                                               RMON1_ENTRY_VALID,
                                               RMON1_ENTRY_INVALID,
                                               &long_temp);
            if (SNMP_ERR_NOERROR != snmp_status) {
                return snmp_status;
            }
            hdr->new_status = long_temp;
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

unsigned char  *
var_eventTable(struct variable *vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method)
{
    static long     long_ret;
    static CRTL_ENTRY_T theEntry;
    RMON_ENTRY_T   *hdr;

    *write_method = write_eventControl;
    hdr = ROWAPI_header_ControlEntry(vp, name, length, exact, var_len,
                                     table_ptr,
                                     &theEntry, sizeof(CRTL_ENTRY_T));
    if (!hdr)
        return NULL;

    *var_len = sizeof(long);    /* default */

    switch (vp->magic) {
    case EVENTINDEX:
        long_ret = hdr->ctrl_index;
        return (unsigned char *) &long_ret;
    case EVENTDESCRIPTION:
        if (theEntry.event_description) {
            *var_len = strlen(theEntry.event_description);
            return (unsigned char *) theEntry.event_description;
        } else {
            *var_len = 0;
            return (unsigned char *) "";
        }
    case EVENTTYPE:
        long_ret = theEntry.event_type;
        return (unsigned char *) &long_ret;
    case EVENTCOMMUNITY:
        if (theEntry.event_community) {
            *var_len = strlen(theEntry.event_community);
            return (unsigned char *) theEntry.event_community;
        } else {
            *var_len = 0;
            return (unsigned char *) "";
        }
    case EVENTLASTTIMESENT:
        long_ret = theEntry.event_last_time_sent;
        return (unsigned char *) &long_ret;
    case EVENTOWNER:
        if (hdr->owner) {
            *var_len = strlen(hdr->owner);
            return (unsigned char *) hdr->owner;
        } else {
            *var_len = 0;
            return (unsigned char *) "";
        }
    case EVENTSTATUS:
        long_ret = hdr->status;
        return (unsigned char *) &long_ret;
    default:
        ag_trace("EventControlTable: unknown vp->magic=%d",
                 (int) vp->magic);
        ERROR_MSG("");
    }
    return NULL;
}

static SCROLLER_T *
event_extract_scroller(void *v_body)
{
    CRTL_ENTRY_T   *body = (CRTL_ENTRY_T *) v_body;
    return &body->scrlr;
}

unsigned char  *
var_logTable(struct variable *vp,
             oid * name,
             size_t * length,
             int exact, size_t * var_len, WriteMethod ** write_method)
{
    static long     long_ret;
    static DATA_ENTRY_T theEntry;
    RMON_ENTRY_T   *hdr;
    CRTL_ENTRY_T   *ctrl;

    *write_method = NULL;
    hdr = ROWDATAAPI_header_DataEntry(vp, name, length, exact, var_len,
                                      table_ptr,
                                      &event_extract_scroller,
                                      sizeof(DATA_ENTRY_T), &theEntry);
    if (!hdr)
        return NULL;

    ctrl = (CRTL_ENTRY_T *) hdr->body;

    *var_len = sizeof(long);    /* default */

    switch (vp->magic) {
    case LOGEVENTINDEX:
        long_ret = hdr->ctrl_index;
        return (unsigned char *) &long_ret;
    case LOGINDEX:
        long_ret = theEntry.data_index;
        return (unsigned char *) &long_ret;
    case LOGTIME:
        long_ret = theEntry.log_time;
        return (unsigned char *) &long_ret;
    case LOGDESCRIPTION:
        if (theEntry.log_description) {
            *var_len = strlen(theEntry.log_description);
            return (unsigned char *) theEntry.log_description;
        } else {
            *var_len = 0;
            return (unsigned char *) "";
        }
    default:
        ERROR_MSG("");
    }

    return NULL;
}

/*
 * External API section 
 */
#if 0 /*eli, not use*/
static char    *
create_explanaition(CRTL_ENTRY_T * evptr, u_char is_rising,
                    u_long alarm_index, u_long event_index,
                    oid * alarmed_var,
                    size_t alarmed_var_length,
                    u_long value, u_long the_threshold,
                    u_long sample_type, char *alarm_descr)
{
#define UNEQ_LENGTH	(1 + 11 + 4 + 11 + 1 + 20)
    char            expl[UNEQ_LENGTH];
    static char     c_oid[SPRINT_MAX_LEN];
    size_t          sz;
    char           *descr;
    register char  *pch;
    register char  *tmp;


    snprint_objid(c_oid, sizeof(c_oid)-1, alarmed_var, alarmed_var_length);
    c_oid[sizeof(c_oid)-1] = '\0';
    for (pch = c_oid;;) {
        tmp = strchr(pch, '.');
        if (!tmp)
            break;
        if (isdigit(tmp[1]) || '"' == tmp[1])
            break;
        pch = tmp + 1;
    }

    snprintf(expl, UNEQ_LENGTH, "=%ld %s= %ld :%ld, %ld",
             (unsigned long) value,
             is_rising ? ">" : "<",
             (unsigned long) the_threshold,
             (long) alarm_index, (long) event_index);
    sz = 3 + strlen(expl) + strlen(pch);
    if (alarm_descr)
        sz += strlen(alarm_descr);

    descr = AGMALLOC(sz);
    if (!descr) {
        ag_trace("Can't allocate event description");
        return NULL;
    }

    if (alarm_descr) {
        strcpy(descr, alarm_descr);
        strcat(descr, ":");
    } else
        *descr = '\0';

    strcat(descr, pch);
    strcat(descr, expl);
    return descr;
}
#endif

extern void     send_enterprise_trap_vars(int, int, oid *, int,
                                          netsnmp_variable_list *);
#if 0 /*eli,not use*/
static netsnmp_variable_list *
oa_bind_var(netsnmp_variable_list * prev,
            void *value, int type, size_t sz_val, oid * oid, size_t sz_oid)
{
    netsnmp_variable_list *var;

    var = (netsnmp_variable_list *) malloc(sizeof(netsnmp_variable_list));
    if (!var) {
        ag_trace("FATAL: cannot malloc in oa_bind_var\n");
        exit(-1);               /* Sorry :( */
    }
    memset(var, 0, sizeof(netsnmp_variable_list));
    var->next_variable = prev;
    snmp_set_var_objid(var, oid, sz_oid);
    snmp_set_var_value(var, (u_char *) value, sz_val);
    var->type = type;

    return var;
}

#endif

#ifdef VXWORKS
/* kinghong modified the original  event_send_trap to fit our system*/

static void
event_send_trap(CRTL_ENTRY_T * evptr, u_char is_rising,
                u_int alarm_index,
                u_int value, u_int the_threshold,
                oid * alarmed_var, size_t alarmed_var_length,
                u_int sample_type)
 {
     TRAP_EVENT_TrapData_T trapData;


	if ( is_rising)
	{

		trapData.trap_type = TRAP_EVENT_RISING_ALARM;
		trapData.community_specified = TRUE;
		/*kinghong added, 2002-07-30 */
		if(evptr->event_community == NULL)
		{
		  trapData.community[0]='\0';
		}
		else
		{
		   if(strlen(evptr->event_community) > MAXSIZE_trapDestCommunity_2)
		   {/* sorry! community length is too long */
		     
			return;
	   	   }
		   memcpy(trapData.community,
			  evptr->event_community,
			  strlen(evptr->event_community));
		   trapData.community[strlen(evptr->event_community)] = '\0';
		}
		trapData.u.rising_alarm.instance_alarm_index = alarm_index;
		trapData.u.rising_alarm.alarm_index = alarm_index;
		trapData.u.rising_alarm.instance_alarm_variable = alarm_index;
		if(alarmed_var_length > SYS_ADPT_MAX_OID_COUNT)
		{/* sorry! variable length is too long */
		   
			return;
		}
		memcpy(trapData.u.rising_alarm.alarm_variable,
			   alarmed_var,
			   (alarmed_var_length * 4));
		trapData.u.rising_alarm.alarm_variable_len = alarmed_var_length;
		trapData.u.rising_alarm.instance_alarm_sample_type = alarm_index;
		trapData.u.rising_alarm.alarm_sample_type =sample_type;
		trapData.u.rising_alarm.instance_alarm_value = alarm_index;
		trapData.u.rising_alarm.alarm_value = value;
		trapData.u.rising_alarm.instance_alarm_rising_falling_threshold = alarm_index;
		trapData.u.rising_alarm.alarm_rising_falling_threshold = the_threshold;

		SNMP_MGR_ReqSendTrap(&trapData);
	}
	else
	{
		trapData.trap_type = TRAP_EVENT_FALLING_ALARM;
		trapData.community_specified = TRUE;
		/*kinghong, added 2002-7-30*/
		if(evptr->event_community == NULL)
		{
		  trapData.community[0]='\0';
		}
		else
		{
		   if(strlen(evptr->event_community) > MAXSIZE_trapDestCommunity_2)
		   {/* sorry! community length is too long */
		     
			return;
	   	   }
		   memcpy(trapData.community,
			  evptr->event_community,
			  strlen(evptr->event_community));
		   trapData.community[strlen(evptr->event_community)] = '\0';
		}
		trapData.u.falling_alarm.instance_alarm_index =alarm_index;
		trapData.u.falling_alarm.alarm_index =alarm_index;
		trapData.u.falling_alarm.instance_alarm_variable = alarm_index;
		if(alarmed_var_length > SYS_ADPT_MAX_OID_COUNT)
		{/* sorry! variable length is too long */
		  
			return;
		}
		memcpy(trapData.u.falling_alarm.alarm_variable,
			  alarmed_var,
			   (alarmed_var_length * 4));
		trapData.u.falling_alarm.alarm_variable_len =alarmed_var_length;
		trapData.u.falling_alarm.instance_alarm_sample_type = alarm_index;
		trapData.u.falling_alarm.alarm_sample_type = sample_type;
		trapData.u.falling_alarm.instance_alarm_value = alarm_index;
		trapData.u.falling_alarm.alarm_value = value;
		trapData.u.falling_alarm.instance_alarm_rising_falling_threshold = alarm_index;
		trapData.u.falling_alarm.alarm_rising_falling_threshold =the_threshold;

		SNMP_MGR_ReqSendTrap(&trapData);
   	}

 }              
#else
static void
event_send_trap(CRTL_ENTRY_T * evptr, u_char is_rising,
                u_int alarm_index,
                u_int value, u_int the_threshold,
                oid * alarmed_var, size_t alarmed_var_length,
                u_int sample_type)
{
    static oid      rmon1_trap_oid[] = { 1, 3, 6, 1, 2, 1, 16, 0, 0 };
    static oid      alarm_index_oid[] =
        { 1, 3, 6, 1, 2, 1, 16, 3, 1, 1, 1 };
    static oid      alarmed_var_oid[] =
        { 1, 3, 6, 1, 2, 1, 16, 3, 1, 1, 3 };
    static oid      sample_type_oid[] =
        { 1, 3, 6, 1, 2, 1, 16, 3, 1, 1, 4 };
    static oid      value_oid[] = { 1, 3, 6, 1, 2, 1, 16, 3, 1, 1, 5 };
    static oid      threshold_oid[] = { 1, 3, 6, 1, 2, 1, 16, 3, 1, 1, 7 };     /* rising case */
    netsnmp_variable_list *top = NULL;
    register int    iii;

    /*
     * set the last 'oid' : risingAlarm or fallingAlarm 
     */
    if (is_rising) {
        iii = OID_LENGTH(rmon1_trap_oid);
        rmon1_trap_oid[iii - 1] = 1;
        iii = OID_LENGTH(threshold_oid);
        threshold_oid[iii - 1] = 7;
    } else {
        iii = OID_LENGTH(rmon1_trap_oid);
        rmon1_trap_oid[iii - 1] = 0;
        iii = OID_LENGTH(threshold_oid);
        threshold_oid[iii - 1] = 8;
    }

    /*
     * build the var list 
     */
    top = oa_bind_var(top, &alarm_index, ASN_INTEGER, sizeof(u_int),
                      alarm_index_oid, OID_LENGTH(alarm_index_oid));

    top =
        oa_bind_var(top, alarmed_var, ASN_OBJECT_ID,
                    sizeof(oid) * alarmed_var_length, alarmed_var_oid,
                    OID_LENGTH(alarmed_var_oid));

    top = oa_bind_var(top, &sample_type, ASN_INTEGER, sizeof(u_int),
                      sample_type_oid, OID_LENGTH(sample_type_oid));

    top = oa_bind_var(top, &value, ASN_INTEGER, sizeof(u_int),
                      value_oid, OID_LENGTH(value_oid));

    top = oa_bind_var(top, &the_threshold, ASN_INTEGER, sizeof(u_int),
                      threshold_oid, OID_LENGTH(threshold_oid));


    send_enterprise_trap_vars(SNMP_TRAP_ENTERPRISESPECIFIC, 0,
                              rmon1_trap_oid,
                              OID_LENGTH(rmon1_trap_oid), top);
    ag_trace("rmon trap has been sent");
    snmp_free_varbind(top);

}
#endif /*end of #ifdef VXWORKS*/

static void
event_save_log(CRTL_ENTRY_T * body, char *event_descr)
{
    register DATA_ENTRY_T *lptr;

    lptr = ROWDATAAPI_locate_new_data(&body->scrlr);
    if (!lptr) {
        ag_trace("Err: event_save_log:cannot locate ?");
        return;
    }

    lptr->log_time = body->event_last_time_sent;
    /*kinghong, added. if already have memory, use the old one and no need to allocate*/
    if ((lptr->log_description) && ( strncmp ( lptr->log_description, "RMON:",5) == 0))
    {
        strcpy( lptr->log_description, event_descr);
    }
    else
    {
    lptr->log_description = AGSTRDUP(event_descr);
    }
    lptr->data_index = ROWDATAAPI_get_total_number(&body->scrlr);

    /*
     * ag_trace ("log has been saved, data_index=%d", (int) lptr->data_index); 
     */
}

int
event_api_send_alarm(u_char is_rising,
                     u_long alarm_index,
                     u_long event_index,
                     oid * alarmed_var,
                     size_t alarmed_var_length,
                     u_long sample_type,
                     u_long value, u_long the_threshold,  u_long interval, char *alarm_descr)
{
    RMON_ENTRY_T   *eptr;
    CRTL_ENTRY_T   *evptr;

    if (!event_index)
        return SNMP_ERR_NOSUCHNAME;

#if 0
    ag_trace("event_api_send_alarm(%d,%d,%d,'%s')",
             (int) is_rising, (int) alarm_index, (int) event_index,
             alarm_descr);
#endif
    eptr = ROWAPI_find(table_ptr, event_index);
    if (!eptr) {
        /*
         * ag_trace ("event cannot find entry %ld", event_index); 
         */
        return SNMP_ERR_NOSUCHNAME;
    }

    evptr = (CRTL_ENTRY_T *) eptr->body;
    evptr->event_last_time_sent = AGUTIL_sys_up_time();


    if (EVENT_TRAP == evptr->event_type
        || EVENT_LOG_AND_TRAP == evptr->event_type) 
        {
        event_send_trap(evptr, is_rising, alarm_index, value,
                        the_threshold, alarmed_var, alarmed_var_length,
                        sample_type);
    }

    if (EVENT_LOG == evptr->event_type
        || EVENT_LOG_AND_TRAP == evptr->event_type)
         {
        /* kinghong marked out the orignal explain and use the old style to create explain*/


       #if 0
        register char  *explain;
        explain = create_explanaition(evptr, is_rising,
                                      alarm_index, event_index,
                                      alarmed_var, alarmed_var_length,
                                      value, the_threshold,
                                      sample_type, alarm_descr);
        #endif
       
        register int next;
        register UI32_T  i;
        register UI32_T  len;
        register UI32_T* array;
        UI8_T        buf[256];
      
        next    = 0;
        len     = alarmed_var_length;
        array   = &(alarmed_var[0]);
        
        next = concatString (buf,next, (UI8_T*)"RMON:",5);
        for (i = 0; i < len; i++) 
        {
           if (i !=0)
           {
            next = concatString (buf,next,(UI8_T*)".",1);
           }
            next = concatNumber(buf,next,array[i]);
        }
        if (sample_type == 2) 
        {
            next = concatString(buf,next,(UI8_T*)" (delta = ",10);
        }
        else 
        {
            next = concatString(buf,next,(UI8_T*)" (value = ",10);
        }
        next = concatNumber(buf,next,value);
        if (is_rising)
             {
             next = concatString(buf,next,(UI8_T*)", Rising Threshold = ",21);
             }
        else
            {
             next = concatString(buf,next,(UI8_T*)", Falling Threshold = ",21);
             }
        next = concatNumber(buf,next,the_threshold);
        next = concatString(buf,next,(UI8_T*)", interval = ",13);
        next = concatNumber(buf,next,interval);
        next = concatString(buf,next,(UI8_T*)")[alarmIndex.",13);
        next = concatNumber(buf,next,alarm_index);
        next = concatString(buf,next,(UI8_T*)"]",1);
        buf[next]='\0';
        /*
         * if (explain) ag_trace ("Dbg:'%s'", explain); 
         */
        event_save_log(evptr, (char *)buf);
   //     if (explain)
     //       AGFREE(explain);
    }

    return SNMP_ERR_NOERROR;
}

#if 1                           /* debug, but may be used for init. TBD: may be token snmpd.conf ? */
int
add_event_entry(int ctrl_index,
                char *event_description,
                EVENT_TYPE_T event_type, char *event_community)
{
    register RMON_ENTRY_T *eptr;
    register CRTL_ENTRY_T *body;
    int             ierr;

    ierr = ROWAPI_new(table_ptr, ctrl_index);
    if (ierr) {
        ag_trace("ROWAPI_new failed with %d", ierr);
        return ierr;
    }

    eptr = ROWAPI_find(table_ptr, ctrl_index);
    if (!eptr) {
        ag_trace("ROWAPI_find failed");
        return -4;
    }

    body = (CRTL_ENTRY_T *) eptr->body;

    /*
     * set parameters 
     */

    if (event_description) {
        if (body->event_description)
            AGFREE(body->event_description);
        body->event_description = AGSTRDUP(event_description);
    }

    if (event_community) {
        if (body->event_community)
            AGFREE(body->event_community);
        body->event_community = AGSTRDUP(event_community);
    }

    body->event_type = event_type;

    eptr->new_status = RMON1_ENTRY_VALID;
    ierr = ROWAPI_commit(table_ptr, ctrl_index);
    if (ierr) {
        ag_trace("ROWAPI_commit failed with %d", ierr);
    }

    return ierr;
}
#endif

/*
 * Registration & Initializatio section 
 */

oid             eventTable_variables_oid[] =
    { 1, 3, 6, 1, 2, 1, 16, 9, 1 };
oid             logTable_variables_oid[] = { 1, 3, 6, 1, 2, 1, 16, 9, 2 };

struct variable2 eventTable_variables[] = {
    /*
     * magic number        , variable type, ro/rw , callback fn  ,           L, oidsuffix 
     */
    {EVENTINDEX, ASN_INTEGER, RONLY, var_eventTable, 2, {1, 1}},
    {EVENTDESCRIPTION, ASN_OCTET_STR, RWRITE, var_eventTable, 2, {1, 2}},
    {EVENTTYPE, ASN_INTEGER, RWRITE, var_eventTable, 2, {1, 3}},
    {EVENTCOMMUNITY, ASN_OCTET_STR, RWRITE, var_eventTable, 2, {1, 4}},
    {EVENTLASTTIMESENT, ASN_TIMETICKS, RONLY, var_eventTable, 2, {1, 5}},
    {EVENTOWNER, ASN_OCTET_STR, RWRITE, var_eventTable, 2, {1, 6}},
    {EVENTSTATUS, ASN_INTEGER, RWRITE, var_eventTable, 2, {1, 7}}
};

struct variable2 logTable_variables[] = {
    /*
     * magic number        , variable type, ro/rw , callback fn  ,           L, oidsuffix 
     */
    {LOGEVENTINDEX, ASN_INTEGER, RONLY, var_logTable, 2, {1, 1}},
    {LOGINDEX, ASN_INTEGER, RONLY, var_logTable, 2, {1, 2}},
    {LOGTIME, ASN_TIMETICKS, RONLY, var_logTable, 2, {1, 3}},
    {LOGDESCRIPTION, ASN_OCTET_STR, RONLY, var_logTable, 2, {1, 4}}

};

void
init_event(void)
{
    REGISTER_MIB("eventTable", eventTable_variables, variable2,
                 eventTable_variables_oid);
    REGISTER_MIB("logTable", logTable_variables, variable2,
                 logTable_variables_oid);
    #ifndef VXWORKS
    ROWAPI_init_table(&EventCtrlTable, "Event", 0, &event_Create, &event_Clone, &event_Delete, NULL,    /* &event_Validate, */
                      &event_Activate, &event_Deactivate, &event_Copy);
    #else
    ROWAPI_init_table(&EventCtrlTable, "Event", SYS_ADPT_MAX_NBR_OF_RMON_EVENT_ENTRY, &event_Create, &event_Clone, &event_Delete, NULL,    /* &event_Validate, */
                      &event_Activate, &event_Deactivate, &event_Copy);
    #endif
#if 0
    add_event_entry(3, "Alarm", EVENT_LOG_AND_TRAP, NULL);
    /*
     * add_event_entry (5, ">=", EVENT_LOG_AND_TRAP, NULL); 
     */
#endif
}

#ifdef VXWORKS
/* kinghong port the following API from RMON to create log entry*/


static int concatString(UI8_T* toThis, int  toThisLen, UI8_T* addThis, int  addThisLen) 
{
    /*returns new length*/

    memcpy (&toThis[toThisLen], addThis, addThisLen);
    return(toThisLen+addThisLen);
}




//get the ascii value of numbers and then connect them into the address
//pointed by toThis
static int  concatNumber(UI8_T* toThis, int  toThisLen, int addThisNumber) 
{
    /*returns newLen*/

    if (addThisNumber == 0) 
    {
        toThis[toThisLen] = '0';
        toThisLen++;
    }
    if (addThisNumber < 0) 
    {
        toThis[toThisLen] = '-';
        toThisLen++;
        addThisNumber *= -1;
    }

    sprintf(((char *)toThis + toThisLen),"%d",addThisNumber);
    toThisLen = strlen((char *)toThis);

    return(toThisLen);
}


/* add by kinghong to create default*/
void EVENT_CreateDefaultEntry()
{
    return; /* default we don't create any event entry*/
}


void EVENT_DeleteAllRow()
{
	register RMON_ENTRY_T *eptr;
	int i;
	
	for (i = 1; i <= SYS_ADPT_MAX_NBR_OF_RMON_HISTORY_ENTRY; i++)
	{
	   eptr = ROWAPI_find(table_ptr, i);
	   if (eptr)
	   {
  	       ROWAPI_delete_clone(table_ptr, i);
               rowapi_delete(eptr);
           }
        }
}
#endif

BOOL_T EVENT_GetEventTable(SNMP_MGR_RmonEventEntry_T *entry_p)
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

        get_event_entry(rmon_p, entry_p);
        return TRUE;
    }

    return FALSE;
}

BOOL_T EVENT_GetNextEventTable(SNMP_MGR_RmonEventEntry_T *entry_p)
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

        get_event_entry(rmon_p, entry_p);
        return TRUE;
    }

    return FALSE;
}

BOOL_T EVENT_CreateEventEntry(SNMP_MGR_RmonEventEntry_T *entry_p)
{
    if (FALSE == create_event_entry(entry_p->id,
        entry_p->type,
        entry_p->community,
        entry_p->description,
        entry_p->owner,
        entry_p->status))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL_T EVENT_DeleteEventEntry(UI32_T index)
{
    RMON_ENTRY_T *rmon_p;

    rmon_p = ROWAPI_find(table_ptr, index);

    if (FALSE == rmon_p)
    {
        return FALSE;
    }

    ROWAPI_delete_clone(table_ptr, index);
    rowapi_delete(rmon_p);

    return TRUE;
}

BOOL_T EVENT_IsEventEntryModified(SNMP_MGR_RmonEventEntry_T *entry_p)
{
    /* the extra added entry
     */
    return TRUE;
}

BOOL_T delete_rmon_event_table(unsigned long ctrl_index)
{
    RMON_ENTRY_T *hdr;

    hdr = ROWAPI_find(table_ptr, ctrl_index);
    if (!hdr)
    {
        return FALSE;
    }
    ROWAPI_delete_clone(table_ptr, ctrl_index);
    rowapi_delete(hdr);
    return TRUE;
}

static void get_event_entry(RMON_ENTRY_T *rmon_p, SNMP_MGR_RmonEventEntry_T *entry_p)
{
    CRTL_ENTRY_T ctrl_entry;

    memcpy(&ctrl_entry, rmon_p->body, sizeof(ctrl_entry));
    entry_p->id = rmon_p->ctrl_index;

    if (NULL == ctrl_entry.event_description)
    {
        entry_p->description[0] = '\0';
    }
    else
    {
        strncpy(entry_p->description, ctrl_entry.event_description, MAXSIZE_eventDescription);
        entry_p->description[MAXSIZE_eventDescription] = '\0';
    }

    entry_p->type = ctrl_entry.event_type;

    if (NULL == ctrl_entry.event_community)
    {
        entry_p->community[0] = '\0';
    }
    else
    {
        strncpy(entry_p->community, ctrl_entry.event_community, sizeof(entry_p->community)-1);
        entry_p->community[sizeof(entry_p->community)-1] = '\0';
    }

    entry_p->last_time_sent = ctrl_entry.event_last_time_sent;

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

static BOOL_T create_event_entry(UI32_T id, UI32_T type, char *community_p, char *description_p, char *owner_p, UI32_T status)
{
    RMON_ENTRY_T *rmon_p;
    CRTL_ENTRY_T *body_p;

    if (   (NULL == community_p)
        || (NULL == description_p)
        || (NULL == owner_p))
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
    body_p->event_type = type;
    body_p->event_community = AGSTRDUP(community_p);
    body_p->event_description = AGSTRDUP(description_p);

    rmon_p->new_owner = AGSTRDUP(owner_p);
    rmon_p->new_status = status;

    if (SNMP_ERR_NOERROR != ROWAPI_commit(table_ptr, id))
    {
        ROWAPI_delete_clone(table_ptr, id);
        rowapi_delete(rmon_p);
        return FALSE;
    }

    return TRUE;
}
