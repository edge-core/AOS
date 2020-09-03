/* -------------------------------------------------------------------------------------
 * FILE NAME:  CLI_API_MANAGEMENT_IP_FILTER.h                                                               
 * -------------------------------------------------------------------------------------
 * PURPOSE:This file is the action function of "" command  
 * NOTE:
 *                                                                                      
 *   
 *  
 * HISTORY:                                                                
 * Modifier         Date                Description                                     
 * -------------------------------------------------------------------------------------
 * peggy            2-20-2003           First Created                                                       
 *                                                                                      
 * -------------------------------------------------------------------------------------    
 * Copyright(C)                 Accton Technology Corp. 2002    
 * -------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_MANAGEMENT_AllClient                            
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "management all-client"                              
 * INPUT    : cmd_idx, *arg[], *ctrl_P                                                             
 * OUTPUT   :                                                               
 * RETURN   :                            
 * NOTES    : 
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_MANAGEMENT_AllClient(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_MANAGEMENT_HttpClient                            
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "management http-client"                              
 * INPUT    : cmd_idx, *arg[], *ctrl_P                                                             
 * OUTPUT   :                                                               
 * RETURN   :                            
 * NOTES    : 
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_MANAGEMENT_HttpClient(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_MANAGEMENT_SnmpClient                            
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "management snmp-clinet"                              
 * INPUT    : cmd_idx, *arg[], *ctrl_P                                                             
 * OUTPUT   :                                                               
 * RETURN   :                            
 * NOTES    : 
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_MANAGEMENT_SnmpClient(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_MANAGEMENT_TelnetClient                            
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "management telnet-client"                              
 * INPUT    : cmd_idx, *arg[], *ctrl_P                                                             
 * OUTPUT   :                                                               
 * RETURN   :                            
 * NOTES    : 
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_MANAGEMENT_TelnetClient(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_Management_AllClient                            
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "show management all-client"                              
 * INPUT    : cmd_idx, *arg[], *ctrl_P                                                             
 * OUTPUT   :                                                               
 * RETURN   :                            
 * NOTES    : 
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Show_Management_AllClient(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_Management_HttpClient                            
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "show management http-client"                              
 * INPUT    : cmd_idx, *arg[], *ctrl_P                                                             
 * OUTPUT   :                                                               
 * RETURN   :                            
 * NOTES    : 
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Show_Management_HttpClient(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_Management_SnmpClient                            
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "show management snmp-client"                              
 * INPUT    : cmd_idx, *arg[], *ctrl_P                                                             
 * OUTPUT   :                                                               
 * RETURN   :                            
 * NOTES    : 
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Show_Management_SnmpClient(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_Management_TelnetClient                            
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "show management telnet-client"                              
 * INPUT    : cmd_idx, *arg[], *ctrl_P                                                             
 * OUTPUT   :                                                               
 * RETURN   :                            
 * NOTES    : 
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Show_Management_TelnetClient(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


/*print function for managemnet ip filter information*/
