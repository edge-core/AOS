
#ifndef LIB1X_KRC_SM_H
#define LIB1X_KRC_SM_H

typedef struct	Krc_SM_tag
{
	KRC_SM	state;	
	BOOLEAN	rxKey;
}	Krc_SM;



void lib1x_krcsm_processKey();
void lib1x_krcsm_init( Krc_SM * krc_sm );
void lib1x_trans_krcsm( Global_Params   * global, Krc_SM  * krc_sm );



#endif/*LIB1X_KRC_SM_H*/
