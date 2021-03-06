/*
 * Note: this file originally auto-generated by mib2c using
 *        : mib2c.scalar.conf,v 1.5 2002/07/18 14:18:52 dts12 Exp $
 */
#ifndef DOT1QBASE_H
#define DOT1QBASE_H

/* function declarations */
void init_dot1qBase(void);
Netsnmp_Node_Handler get_dot1qVlanVersionNumber;
Netsnmp_Node_Handler get_dot1qMaxSupportedVlans;
Netsnmp_Node_Handler do_dot1qGvrpStatus;
Netsnmp_Node_Handler get_dot1qMaxVlanId;
Netsnmp_Node_Handler get_dot1qNumVlans;

/* function declarations */
void init_dot1qFdbTable(void);
FindVarMethod var_dot1qFdbTable;
FindVarMethod var_dot1qFdbTable;

/* function declarations */
void init_dot1qTpFdbTable(void);
FindVarMethod var_dot1qTpFdbTable;
FindVarMethod var_dot1qTpFdbTable;

/* function declarations */
void init_dot1qStaticUnicastTable(void);
FindVarMethod var_dot1qStaticUnicastTable;
FindVarMethod var_dot1qStaticUnicastTable;
WriteMethod write_dot1qStaticUnicastAllowedToGoTo;
WriteMethod write_dot1qStaticUnicastStatus;

/* function declarations */
void init_dot1qVlan(void);
Netsnmp_Node_Handler get_dot1qVlanNumDeletes;
Netsnmp_Node_Handler do_dot1qConstraintTypeDefault;
Netsnmp_Node_Handler get_dot1qNextFreeLocalVlanIndex;
Netsnmp_Node_Handler do_dot1qConstraintSetDefault;

/* function declarations */
void init_dot1qVlanCurrentTable(void);
FindVarMethod var_dot1qVlanCurrentTable;
FindVarMethod var_dot1qVlanCurrentTable;

/* function declarations */
void init_dot1qVlanStaticTable(void);
FindVarMethod var_dot1qVlanStaticTable;
FindVarMethod var_dot1qVlanStaticTable;
WriteMethod write_dot1qVlanStaticName;
WriteMethod write_dot1qVlanStaticEgressPorts;
WriteMethod write_dot1qVlanForbiddenEgressPorts;
WriteMethod write_dot1qVlanStaticUntaggedPorts;
WriteMethod write_dot1qVlanStaticRowStatus;

/* function declarations */
void init_dot1qPortVlanTable(void);
FindVarMethod var_dot1qPortVlanTable;
FindVarMethod var_dot1qPortVlanTable;
WriteMethod write_dot1qPvid;
WriteMethod write_dot1qPortAcceptableFrameTypes;
WriteMethod write_dot1qPortIngressFiltering;
WriteMethod write_dot1qPortGvrpStatus;

/*add for get the SNMP name index*/
int get_dot1qVlanStatic_mib_index(struct variable *vp,
                              oid * name,size_t * length,UI32_T *vid,int exact);
                              
/*add for get the SNMP name index*/
int get_dot1qVlanCurrent_mib_index(struct variable *vp,
                            oid * name,size_t * length,UI32_T *vid,int exact);
#endif /* DOT1QBASE_H */
