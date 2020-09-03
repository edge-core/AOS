#ifndef RFC3635_H
#define RFC3635_H


/* Function declarations - dot3StatsTable
 */
void             init_dot3StatsTable(void);
FindVarMethod    var_dot3StatsTable;


/* Function declarations - dot3HCStatsTable
 */
void             init_dot3HCStatsTable(void);
FindVarMethod    var_dot3HCStatsTable;


/* dot3StatsTable
 */
#define DOT3STATSINDEX                        1
#define DOT3STATSALIGNMENTERRORS              2
#define DOT3STATSFCSERRORS                    3
#define DOT3STATSSINGLECOLLISIONFRAMES        4
#define DOT3STATSMULTIPLECOLLISIONFRAMES      5
#define DOT3STATSSQETESTERRORS                6
#define DOT3STATSDEFERREDTRANSMISSIONS        7
#define DOT3STATSLATECOLLISIONS               8
#define DOT3STATSEXCESSIVECOLLISIONS          9
#define DOT3STATSINTERNALMACTRANSMITERRORS    10
#define DOT3STATSCARRIERSENSEERRORS           11
#define DOT3STATSFRAMETOOLONGS                13
#define DOT3STATSINTERNALMACRECEIVEERRORS     16
#define DOT3STATSETHERCHIPSET                 17
#define DOT3STATSSYMBOLERRORS                 18
#define DOT3STATSDUPLEXSTATUS                 19
#define DOT3STATSRATECONTROLABILITY           20
#define DOT3STATSRATECONTROLSTATUS            21


/* dot3HCStatsTable
 */
#define DOT3HCSTATSALIGNMENTERRORS              1
#define DOT3HCSTATSFCSERRORS                    2
#define DOT3HCSTATSINTERNALMACTRANSMITERRORS    3
#define DOT3HCSTATSFRAMETOOLONGS                4
#define DOT3HCSTATSINTERNALMACRECEIVEERRORS     5
#define DOT3HCSTATSSYMBOLERRORS                 6


/* function declarations */
void init_dot3PauseTable(void);
FindVarMethod var_dot3PauseTable;
WriteMethod write_dot3PauseAdminMode;

#define DOT3PAUSEADMINMODE       1
#define DOT3PAUSEOPERMODE        2
#define DOT3INPAUSEFRAMES        3
#define DOT3OUTPAUSEFRAMES       4

#endif  /* end of #ifndef RFC3635_H */
