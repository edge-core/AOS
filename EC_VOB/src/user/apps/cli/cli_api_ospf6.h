#ifndef CLI_API_OSPF6_H
#define CLI_API_OSPF6_H

/* Macros for lsa->advertisement (packet data), 
 * should use OSPF_PNT_XXX_GET()
*/
#define LSA_PREFIX_BYTES(PL)		((((PL) + 31) / 32) * 4)
#define LSA_PREFIX_SPACE(P)		(LSA_PREFIX_BYTES ((P)->length) + 4)
#define LSA_PREFIX_LENGTH_GET(L,O)	OSPF_PNT_UCHAR_GET ((L), (O))
#define LSA_PREFIX_ADDRESS_COPY(A,L,O)                                        \
    memcpy((A), (L) + (O) + 4,                             \
                 LSA_PREFIX_BYTES (LSA_PREFIX_LENGTH_GET ((L), (O))))

#define LSA_PREFIX_SIZE(L,O)                                                  \
    (4 + LSA_PREFIX_BYTES (LSA_PREFIX_LENGTH_GET ((L), (O))))
#define LSA_PREFIX_GET(P,L,O)                                                 \
    do {                                                                      \
      memset((P), 0, sizeof(*P));                 \
      (P)->length = OSPF_PNT_UCHAR_GET ((L), (O));                                 \
      (P)->options = OSPF_PNT_UCHAR_GET ((L), (O) + 1);                            \
      (P)->u.zero = OSPF_PNT_UINT16_GET ((L), (O) + 2);                            \
      if ((P)->length > 0)                                                    \
        LSA_PREFIX_ADDRESS_COPY (&((P)->addr), (L), (O));                     \
    } while (0)



UI32_T CLI_API_L3_Ip_Ospf6_Interface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ospf6_AbrType(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ospf6_Area(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ospf6_DefaultMetric(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ospf6_MaxCurrentDD(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ospf6_PassiveInterface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ospf6_Redistribute(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ospf6_RouterId(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ospf6_Timer(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Show_Ip_Ospf6(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Router_Ospf6(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Ip_Ospf6_RouterInterface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Ospf6_Clear_Ospf6_Process(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

#endif
