
#include "eh_mgr.h"
#include "eh_type.h"
#include "dhcp_type.h"

static int DHCP_ERROR_FLAG = 0;

void error(char *text)
{
	if (DHCP_ERROR_FLAG)
	  printf("\n %s", text);
	  
	EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown, 
								 EH_TYPE_MSG_INVALID, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR), 
                                 text);
}

void warn(char *text)
{
	if (DHCP_ERROR_FLAG)
	 printf("\n %s", text);
	 
	EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown, 
								 EH_TYPE_MSG_INVALID, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR), 
                                 text);
}

void DHCP_ERROR_Print(char *text)
{
	if (DHCP_ERROR_FLAG)
	  printf("\n %s", text);
	  
	EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown, 
								 EH_TYPE_MSG_INVALID, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR), 
                                 text);
}

void DHCP_ERROR_Warning(char *text)
{
    if (DHCP_ERROR_FLAG)
	printf("\n %s", text);
	
	EH_MGR_Handle_Exception1(SYS_MODULE_DHCP, DHCP_TYPE_EH_Unknown, 
								 EH_TYPE_MSG_INVALID, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR), 
                                 text);
}

void DHCP_ERROR_SetFlag(int flag)
{
	DHCP_ERROR_FLAG = flag;
	
}

void DHCP_ERROR_GetFlag(int *flag)
{
	*flag = DHCP_ERROR_FLAG;	
}
