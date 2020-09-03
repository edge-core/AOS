#if((SYS_CPNT_A3COM_PAE_MIB==TRUE) &&(SYS_CPNT_NETWORK_ACCESS==TRUE))
#ifndef A3COMPAE_H
#define AA3COMPAE_H
void            init_a3ComPaePortTable(void);
FindVarMethod   var_a3ComPaePortTable;
WriteMethod     write_a3ComPaePortAssignEnable;
#endif                      
#endif 

