
#ifndef DOT1X_MACBASED_PTSM_H
#define DOT1X_MACBASED_PTSM_H

/*New Timer State Machine */
void DOT1XMAC_Ptsm_InitTick();
void DOT1XMAC_Ptsm_RunStateMachine(UI32_T lport);
BOOL_T DOT1XMAC_Ptsm_SetTimer(Global_Params *global,UI8_T timer_type,UI8_T flag);
BOOL_T DOT1XMAC_Ptsm_GetTimer(Global_Params *global,UI8_T timer_type,UI8_T *flag);
BOOL_T DOT1XMAC_Ptsm_IsTimeout(UI32_T lport,UI8_T timer_type);
void DOT1XMAC_Ptsm_ResetTimer(UI32_T lport,UI8_T timer_type,UI32_T seconds); 

#endif /*DOT1X_MACBASED_PTSM_H*/
