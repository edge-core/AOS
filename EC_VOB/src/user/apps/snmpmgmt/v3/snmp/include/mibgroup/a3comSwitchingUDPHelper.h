#if (SYS_CPNT_UDPHELPER == TRUE)

#ifndef A3COMSWITCHINGUDPHELPER_H
#define A3COMSWITCHINGUDPHELPER_H


/* Function Declarations - a3ComUdpHelperTable */
void             init_a3ComUdpHelperTable(void);
FindVarMethod    var_a3ComUdpHelperTable;
WriteMethod      write_a3ComUdpHelperRowStatus;


/* a3ComUdpHelperTable */
#define A3COMUDPHELPERPORT         1
#define A3COMUDPHELPERIPADDRESS    2
#define A3COMUDPHELPERROWSTATUS    3


#endif  /* end of #ifndef A3COMSWITCHINGUDPHELPER_H */

#endif  /* end of #if (SYS_CPNT_UDPHELPER == TRUE) */