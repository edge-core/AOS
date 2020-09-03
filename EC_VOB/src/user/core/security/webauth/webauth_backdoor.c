/* MODULE NAME: WEBAUTH_BACKDOOR.C 
 * PURPOSE: 
 *      Definitions for the WEBAUTH backdoor functions 
 * NOTES:
 *
 *
 * HISTORY:
 *    01/29/07 --  Rich Lee, Create
 * 
 * Copyright(C)      Accton Corporation, 2007 
 */


/* INCLUDE FILE DECLARATION 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sys_type.h"
#include "sysfun.h"
#include "l_stdlib.h"
#include "backdoor_mgr.h"
#include "l_inet.h"

#include "webauth_backdoor.h"
#include "webauth_om.h"
#include "webauth_mgr.h"
#include "webauth_type.h"
#include "swctrl.h"
#include "sysfun.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define MAXLINE 255
/* MACRO FUNCTION DECLARATIONS
 */
 
/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void WEBAUTH_BACKDOOR_Engine();
static void WEBAUTH_BACKDOOR_ShowCmd();
static int  WEBAUTH_BACKDOOR_GetLine();
static void WEBAUTH_BACKDOOR_GetSuccessListInfo();
static void WEBAUTH_BACKDOOR_GetPortInfo(UI32_T lport);
static void WEBAUTH_BACKDOOR_GetBlackListInfo();
static void WEBAUTH_BACKDOOR_GetTryingListInfo();
static void WEBAUTH_BACKDOOR_SetSuccessHostByPort(UI32_T lport, UI32_T ip_addr);
static void WEBAUTH_BACKDOOR_SetBlackHostByPort(UI32_T lport, UI32_T ip_addr);
static void WEBAUTH_BACKDOOR_SetTryingHostByPort(UI32_T lport, UI32_T ip_addr);
static void WEBAUTH_BACKDOOR_DeleteSuccessHostByPortArr(UI32_T lport,UI32_T ip_addr);
static void WEBAUTH_BACKDOOR_DeleteSuccessHostByPort(UI32_T lport, UI32_T ip_addr);
static void WEBAUTH_BACKDOOR_DeleteBlackHostByPort(UI32_T lport, UI32_T ip_addr);
static void WEBAUTH_BACKDOOR_DeleteTryingHostByPort(UI32_T lport,UI32_T ip_addr);
static void WEBAUTH_BACKDOOR_SetSessionTimeout(UI16_T timeout);
static void WEBAUTH_BACKDOOR_SetDebugFlag(UI16_T dbgtype);

/* STATIC VARIABLE DEFINITIONS
 */ 
/* EXPORTED SUBPROGRAM BODIES
 */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - WEBAUTH_BACKDOOR_Main
 * ------------------------------------------------------------------------
 * FUNCTION : This function is the main routine of the backdoor
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */

void WEBAUTH_BACKDOOR_Main(void)
{
    printf("WEBAUTH Backdoor!!\n");
    WEBAUTH_BACKDOOR_Engine();
    return;
}

/* LOCAL SUBPROGRAM BODIES
 */
static void WEBAUTH_BACKDOOR_Engine()
{
    BOOL_T engine_continue = TRUE;
    UI8_T ch;
    UI8_T cmd_buf[256];
    UI32_T lport, ip_addr;
	UI8_T   keyin[18];
	UI8_T ip[18];
	UI16_T timeout, dbgtype;


    while(engine_continue)
    {
        ch = 0;
        cmd_buf[0] = 0;

        WEBAUTH_BACKDOOR_ShowCmd();
        WEBAUTH_BACKDOOR_GetLine(cmd_buf, MAXLINE);
        ch = atoi((char *)cmd_buf);

        switch(ch)
        {
            case 99:
                engine_continue = FALSE;
                break;
            case 1:                
                break;
            case 2:
                WEBAUTH_BACKDOOR_GetSuccessListInfo();
                break;
            case 3:
                WEBAUTH_BACKDOOR_GetBlackListInfo();
                break;
            case 4:
                WEBAUTH_BACKDOOR_GetTryingListInfo();
                break;
            case 5:
                printf("\r\nWhich port: ");
                WEBAUTH_BACKDOOR_GetLine(cmd_buf, MAXLINE);
                lport = atoi((char *)cmd_buf);
                printf("\r\nInput ip: ");
                BACKDOOR_MGR_RequestKeyIn(keyin, 18);
                printf("\r\n");
            	strcpy((char *)ip,(char *)keyin);
                L_INET_Aton(ip, &(ip_addr));
            
                WEBAUTH_BACKDOOR_SetSuccessHostByPort(lport, ip_addr);
                
                break;

            case 6:
                printf("\r\nWhich port: ");
                WEBAUTH_BACKDOOR_GetLine(cmd_buf, MAXLINE);
                lport = atoi((char *)cmd_buf);
                printf("\r\nInput ip: ");
            	BACKDOOR_MGR_RequestKeyIn(keyin, 18);
            	printf("\r\n");
            	strcpy((char *)ip,(char *)keyin);
                L_INET_Aton(ip, &(ip_addr));
				WEBAUTH_BACKDOOR_SetBlackHostByPort(lport, ip_addr);
                
                break;

            case 7:
                printf("\r\nWhich port: ");
                WEBAUTH_BACKDOOR_GetLine(cmd_buf, MAXLINE);
                lport = atoi((char *)cmd_buf);
                printf("\r\nInput ip: ");
            	BACKDOOR_MGR_RequestKeyIn(keyin, 18);
            	printf("\r\n");
            	strcpy((char *)ip,(char *)keyin);
                L_INET_Aton(ip, &(ip_addr));
				WEBAUTH_BACKDOOR_SetTryingHostByPort(lport, ip_addr);
                
                break;
          
            case 8:
                printf("\r\nWhich port: ");
                WEBAUTH_BACKDOOR_GetLine(cmd_buf, MAXLINE);
                lport = atoi((char *)cmd_buf);
                printf("\r\nInput ip: ");
                BACKDOOR_MGR_RequestKeyIn(keyin, 18);
                printf("\r\n");
            	  strcpy((char *)ip,(char *)keyin);
                L_INET_Aton(ip, &(ip_addr));
            
                WEBAUTH_BACKDOOR_DeleteSuccessHostByPort(lport, ip_addr);
                
                break;
            case 9:
                printf("\r\nWhich port: ");
                WEBAUTH_BACKDOOR_GetLine(cmd_buf, MAXLINE);
                lport = atoi((char *)cmd_buf);
                printf("\r\nInput ip: ");
                BACKDOOR_MGR_RequestKeyIn(keyin, 18);
                printf("\r\n");
            	  strcpy((char *)ip,(char *)keyin);
                L_INET_Aton(ip, &(ip_addr));
            
                WEBAUTH_BACKDOOR_DeleteBlackHostByPort(lport, ip_addr);
                
                break;
            case 10:
                printf("\r\nWhich port: ");
                WEBAUTH_BACKDOOR_GetLine(cmd_buf, MAXLINE);
                lport = atoi((char *)cmd_buf);
                printf("\r\nInput ip: ");
                BACKDOOR_MGR_RequestKeyIn(keyin, 18);
                printf("\r\n");
            	  strcpy((char *)ip,(char *)keyin);
                L_INET_Aton(ip, &(ip_addr));
            
                WEBAUTH_BACKDOOR_DeleteTryingHostByPort(lport, ip_addr);
                
                break;
            case 11:/* show port info */
                printf("\r\nWhich port: ");
                WEBAUTH_BACKDOOR_GetLine(cmd_buf, MAXLINE);
                lport = atoi((char *)cmd_buf);
                WEBAUTH_BACKDOOR_GetPortInfo(lport);
                break;
            case 12:
                printf("\r\nWhich port: ");
                WEBAUTH_BACKDOOR_GetLine(cmd_buf, MAXLINE);
                lport = atoi((char *)cmd_buf);
                printf("\r\nInput ip: ");
                BACKDOOR_MGR_RequestKeyIn(keyin, 18);
                printf("\r\n");
            	  strcpy((char *)ip,(char *)keyin);
                L_INET_Aton(ip, &(ip_addr));
            
                WEBAUTH_BACKDOOR_DeleteSuccessHostByPortArr(lport, ip_addr);
                
                break; 
            case 13:
                printf("\r\nWhich port: ");
                WEBAUTH_BACKDOOR_GetLine(cmd_buf, MAXLINE);
                lport = atoi((char *)cmd_buf);
                printf("\r\nInput ip: ");
                BACKDOOR_MGR_RequestKeyIn(keyin, 18);
                printf("\r\n");
            	  strcpy((char *)ip,(char *)keyin);
                L_INET_Aton(ip, &(ip_addr));
            
                WEBAUTH_BACKDOOR_DeleteSuccessHostByPortArr(lport, ip_addr);
                
                break;        
	    case 14:
                printf("\r\n session timeoutt: ");
                WEBAUTH_BACKDOOR_GetLine(cmd_buf, MAXLINE);
               timeout = atoi((char *)cmd_buf);            
                WEBAUTH_BACKDOOR_SetSessionTimeout(timeout);
                
                break;        
         case 15:
                printf("\r\n1:MGR on 2:MGR off  3:OM  on 4: OM off ? ");
                WEBAUTH_BACKDOOR_GetLine(cmd_buf, MAXLINE);
                dbgtype = atoi((char *)cmd_buf);
                
                WEBAUTH_BACKDOOR_SetDebugFlag(dbgtype);
                
                break;       
            default:
                continue;
        }

    }
}



static int WEBAUTH_BACKDOOR_GetLine(char s[], int lim)
{
    int  i;
    char c = '\0';

    for(i = 0; i < lim-1 && (c = getchar()) != 0 && c !='\n'; ++i)
    {
        s[i] = c;
        printf("%c", c);
    }
    if(c == '\n')
    {
        s[i] = c;
        ++i;
    }
    s[i] = '\0';
    return i;
}

static void WEBAUTH_BACKDOOR_GetPortInfo(UI32_T lport)
{
    WEBAUTH_TYPE_Port_Info_T lport_info;
    UI32_T ret, i;
    UI8_T  buf[30]={0};
    
    ret = WEBAUTH_MGR_GetPortInfoByLPort(lport, &lport_info);
    if(ret != WEBAUTH_TYPE_RETURN_OK)
    {
        printf("\n get port info error! \r\n");
    }
    else    
    {
        printf("\r\n");
        printf("\n port: %lu", (unsigned long)lport);
        printf("\n success count: %d", lport_info.success_count);
        printf("\n black count: %d", lport_info.black_count);
        printf("\n status     : %d", lport_info.status);
        for(i=0; i< SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT; i++)
        {
            if(lport_info.success_entries_ar[i].state == WEBAUTH_TYPE_HOST_STATE_SUCCESS)
            {
                printf("\n Success Host Index:%lu",(unsigned long)i);
                L_INET_Ntoa(lport_info.success_entries_ar[i].ip, buf);
                printf("\n success entries ip : %s", buf);
                printf("\n success entries port : %lu", (unsigned long)lport_info.success_entries_ar[i].lport);
                printf("\n success entries remaining time: %d", lport_info.success_entries_ar[i].remaining_time);
                printf("\n success entries state: %d\n", lport_info.success_entries_ar[i].state);
            }
        }    
            
        for(i=0; i< SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT; i++)
        {
             if(lport_info.black_entries_ar[i].state == WEBAUTH_TYPE_HOST_STATE_BLACK)
             {
                printf("\n Black Host :%lu",(unsigned long)i);
                L_INET_Ntoa(lport_info.black_entries_ar[i].ip, buf);
                printf("\n black entries ip : %s",buf );
                printf("\n black entries port : %lu", (unsigned long)lport_info.black_entries_ar[i].lport);
                printf("\n black entries remaining time: %d", lport_info.black_entries_ar[i].remaining_time);
                printf("\n black entries state: %d\n", lport_info.black_entries_ar[i].state);
             }
        }    
        printf("\r\n");
    }
   
}
    
static void WEBAUTH_BACKDOOR_GetSuccessListInfo(void)
{
    WEBAUTH_TYPE_Host_Info_T *success_list_head_p;    
    WEBAUTH_TYPE_Host_Info_T *success_list_tmp_p;    
    UI16_T count =1;
   
    UI8_T  buf_ar[30]={0};
    success_list_head_p = WEBAUTH_OM_GetSuccessListHead();
	
	
        if(success_list_head_p == NULL)
		{
		    printf("\n success head null");
		    return;
		}
		else
		{
    		/*memcpy(success_list_tmp, &success_list, sizeof(WEBAUTH_TYPE_Host_Info_T));*/
            success_list_tmp_p = success_list_head_p;
    	}	
    		
        while(success_list_tmp_p != NULL)
        {
            printf("\n success Host Index: %d",count);
            L_INET_Ntoa(success_list_tmp_p->ip, buf_ar);                    
    		printf("\n success ip:%s",buf_ar);
    		printf("\n port      : %lu",(unsigned long)success_list_tmp_p->lport);
    		printf("\n remaining auth: %d",success_list_tmp_p->remaining_time);
    		printf("\n state    : %d",success_list_tmp_p->state);
    		printf("\r\n");
    		if(!success_list_tmp_p->next_host_p)
    		{
    			printf("\n total count : %d",count);
    			break;
    		}
    		else
    			success_list_tmp_p = success_list_tmp_p->next_host_p;    				
    			count++;
        }
}

static void WEBAUTH_BACKDOOR_GetBlackListInfo(void)
{
    WEBAUTH_TYPE_Host_Info_T    *black_list_p;    
    WEBAUTH_TYPE_Host_Info_T    *black_list_tmp_p;    
    UI16_T count =1;
   
    UI8_T  buf[30]={0};
    
    black_list_p = WEBAUTH_OM_GetBlackListHead();
	
	
        if(black_list_p == NULL)
		{
		    printf("\n black head null");
		    return;
		}
		else
		{
    		/*memcpy(success_list_tmp, &success_list, sizeof(WEBAUTH_TYPE_Host_Info_T));*/
            black_list_tmp_p = black_list_p;
    	}	
    		
        while(black_list_tmp_p != NULL)
        {
            printf("\n black Host Index: %d",count);
            L_INET_Ntoa(black_list_tmp_p->ip, buf);                    
    		printf("\n black ip:%s",buf);
    		printf("\n remaining time: %d",black_list_tmp_p->remaining_time);
    		printf("\r\n");
    		if(!black_list_tmp_p->next_host_p)
    		{
    			printf("\n total count : %d",count);
    			break;
    		}
    		else
    			black_list_tmp_p= black_list_tmp_p->next_host_p;    				
    			count++;
        }
   
}

static void WEBAUTH_BACKDOOR_GetTryingListInfo(void)
{

    WEBAUTH_TYPE_Host_Trying_T *trying_list_tmp_p;
    UI16_T count =1;    
    UI8_T  buf_ar[30]={0};
    
        trying_list_tmp_p = WEBAUTH_OM_GetTryingListHead();
	
	    
    		
        while(trying_list_tmp_p != NULL)
        {
            printf("\n trying Host Index: %d",count);
            L_INET_Ntoa(trying_list_tmp_p->ip, buf_ar);                    
    		printf("\n trying ip:%s",buf_ar);
    		printf("\n trying port:%lu",(unsigned long)trying_list_tmp_p->lport);
    		printf("\n trying attempt:%d",trying_list_tmp_p->login_attempt);
    		/*printf("\n remain auth: %d",black_list_tmp->auth_time);*/
    		printf("\r\n");
    		if(!trying_list_tmp_p->next_host_p)
    		{
    			printf("\n total count : %d",count);
    			break;
    		}
    		else
    			trying_list_tmp_p = trying_list_tmp_p->next_host_p;    				
    			count++;
        }
        WEBAUTH_OM_GetTotalTryingCount(&count);
        printf("\n Total trying count is %d", count);
}

static void WEBAUTH_BACKDOOR_ShowCmd()
{
    printf("\n 1: show system config\n");
    printf(" 2: show success list\n");
    printf(" 3: show black list\n");
    printf(" 4: show trying list\n");
    printf(" 5: set success host\n");
    printf(" 6: set black host\n");
    printf(" 7: set trying host\n");
    printf(" 8: delete success host in list\n");
    printf(" 9: delete black host\n");
    printf(" 10: delete trying host\n");
    printf(" 11: show port info\n");
    printf(" 12: delete success host in array\n");
    printf(" 13: set system parameters:\n");
    printf(" 14: set session timeout:\n");	
    printf(" 15: set debug flag:\n"); 
    printf("99: quit\n");
    printf("input: ");
}
static void WEBAUTH_BACKDOOR_SetSuccessHostByPort(UI32_T lport, UI32_T ip_addr)
{
	UI32_T ret;
	ret = WEBAUTH_MGR_CreateSuccessHostByLPort( ip_addr,lport);
	printf("\n WEBAUTH_BACKDOOR_SetSuccessHostByPort ret %lu", (unsigned long)ret);
}

static void WEBAUTH_BACKDOOR_SetBlackHostByPort(UI32_T lport, UI32_T ip_addr)
{
	UI32_T ret;
	ret = WEBAUTH_MGR_CreateBlackHostByLPort( ip_addr,lport);
	printf("\n WEBAUTH_BACKDOOR_SetBlackHostByPort ret %lu", (unsigned long)ret);
}

static void WEBAUTH_BACKDOOR_SetTryingHostByPort(UI32_T lport, UI32_T ip_addr)
{
	UI32_T ret;
	ret = WEBAUTH_MGR_CreateTryingHost( ip_addr,lport);
	printf("\n WEBAUTH_BACKDOOR_SetTryingHostByPort ret %lu", (unsigned long)ret);
}

static void WEBAUTH_BACKDOOR_DeleteSuccessHostByPort(UI32_T lport,UI32_T ip_addr)
{
	UI32_T ret;
	ret = WEBAUTH_MGR_DeleteSuccessListByHostIP(ip_addr, lport);
	printf("\n WEBAUTH_MGR_DeleteSuccessHostByLPort ret %lu", (unsigned long)ret);
}

static void WEBAUTH_BACKDOOR_DeleteSuccessHostByPortArr(UI32_T lport, UI32_T ip_addr)
{
	UI32_T ret;
	ret = WEBAUTH_OM_DeleteSuccessHostArrayByIP(lport, ip_addr);
	printf("\n WEBAUTH_MGR_DeleteSuccessHostByLPort ret %lu", (unsigned long)ret);
}

static void WEBAUTH_BACKDOOR_DeleteBlackHostByPort(UI32_T lport, UI32_T ip_addr)
{
	UI32_T ret;
	ret = WEBAUTH_MGR_DeleteBlackListByHostIP( ip_addr, lport);
	printf("\n WEBAUTH_MGR_DeleteBlackHostByLPort ret %lu", (unsigned long)ret);
}
static void WEBAUTH_BACKDOOR_DeleteTryingHostByPort(UI32_T lport, UI32_T ip_addr)
{
	UI32_T ret;
	ret = WEBAUTH_MGR_DeleteTryingHost( ip_addr, lport	);
	printf("\n WEBAUTH_MGR_DeleteTryingHostByLPort ret %lu", (unsigned long)ret);
}

static void WEBAUTH_BACKDOOR_SetSessionTimeout(UI16_T timeout)
{
	 WEBAUTH_OM_SetSessionTimeout(timeout);
}

static void WEBAUTH_BACKDOOR_SetDebugFlag(UI16_T dbgtype)
{
    switch(dbgtype)
    {
        case 1:/* mgr */
        case 2:
            WEBAUTH_MGR_SetDebugFlag((dbgtype%2));
            break;
        case 3:
        case 4:
            WEBAUTH_OM_SetDebugFlag((dbgtype%2));
            break;
        default:
            break;
    }
}
            
