

UI32_T CLI_API_Disconnect_SSH(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_IP_SSH(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_SSH(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_IP_SSH_AuthenticationRetries(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_IP_SSH_Server(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_IP_SSH_Timeout(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ip_Ssh_Save_Host-key                           
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "ip ssh save host-key"                              
 * INPUT    : cmd_idx, *arg[], *ctrl_P                                                             
 * OUTPUT   :                                                               
 * RETURN   :                            
 * NOTES    : 
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Ip_Ssh_Save_Hostkey(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ip_Ssh_Server-key_Size                        
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "ip ssh server-key size"                              
 * INPUT    : cmd_idx, *arg[], *ctrl_P                                                             
 * OUTPUT   :                                                               
 * RETURN   :                            
 * NOTES    : 
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Ip_Ssh_Serverkey_Size(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Delete_PublicKey                        
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "delete public-key"                              
 * INPUT    : cmd_idx, *arg[], *ctrl_P                                                             
 * OUTPUT   :                                                               
 * RETURN   :                            
 * NOTES    : 
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Delete_PublicKey(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ip_Ssh_Crypto_Host-key_Generate                     
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "ip ssh crypto host-key generate rsa"                              
 * INPUT    : cmd_idx, *arg[], *ctrl_P                                                             
 * OUTPUT   :                                                               
 * RETURN   :                            
 * NOTES    : 
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Ip_Ssh_Crypto_Hostkey_Generate(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ip_Ssh_Crypto_Zeroize                    
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "ip ssh crypto zeroize"                              
 * INPUT    : cmd_idx, *arg[], *ctrl_P                                                             
 * OUTPUT   :                                                               
 * RETURN   :                            
 * NOTES    : 
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Ip_Ssh_Crypto_Zeroize (UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_PublicKey                  
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "show pubilc-key"                              
 * INPUT    : cmd_idx, *arg[], *ctrl_P                                                             
 * OUTPUT   :                                                               
 * RETURN   :                            
 * NOTES    : 
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Show_PublicKey(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ip_Ssh_Crypto_Host-key_Generate_Rsa                            
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "ip ssh crypto host-key generate rsa"                              
 * INPUT    : cmd_idx, *arg[], *ctrl_P                                                             
 * OUTPUT   :                                                               
 * RETURN   :                            
 * NOTES    : 
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Ip_Ssh_Crypto_Hostkey_Generate_Rsa(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

