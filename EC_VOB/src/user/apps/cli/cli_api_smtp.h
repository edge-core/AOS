/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Logging_Sendmail                            
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "logging sendmail"                              
 * INPUT    : cmd_idx, *arg[], *ctrl_P                                                             
 * OUTPUT   :                                                               
 * RETURN   :                            
 * NOTES    : 
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Logging_Sendmail(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Logging_Sendmail_DestinationEmail                           
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "logging sendmail destination-email"                              
 * INPUT    : cmd_idx, *arg[], *ctrl_P                                                             
 * OUTPUT   :                                                               
 * RETURN   :                            
 * NOTES    : 
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Logging_Sendmail_DestinationEmail(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Logging_Sendmail_Host                         
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "logging sendmail host"                              
 * INPUT    : cmd_idx, *arg[], *ctrl_P                                                             
 * OUTPUT   :                                                               
 * RETURN   :                            
 * NOTES    : 
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Logging_Sendmail_Host(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Logging_Sendmail_Level                           
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "logging sendmail level"                              
 * INPUT    : cmd_idx, *arg[], *ctrl_P                                                             
 * OUTPUT   :                                                               
 * RETURN   :                            
 * NOTES    : 
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Logging_Sendmail_Level(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Logging_Sendmail_SourceEmail                             
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "logging sendmail source-email"                              
 * INPUT    : cmd_idx, *arg[], *ctrl_P                                                             
 * OUTPUT   :                                                               
 * RETURN   :                            
 * NOTES    : 
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Logging_Sendmail_SourceEmail(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_Logging_Sendmail                         
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "show logging sendmail"                              
 * INPUT    : cmd_idx, *arg[], *ctrl_P                                                             
 * OUTPUT   :                                                               
 * RETURN   :                            
 * NOTES    : 
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Show_Logging_Sendmail(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

