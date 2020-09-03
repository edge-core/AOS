

#ifndef LIB1x_REAUTH_SM_H
#define LIB1x_REAUTH_SM_H

#include "1x_common.h"

#define LIB1X_RSM_REAUTHPERIOD       3600    /*seconds   */
typedef struct Reauth_SM_tag
{
	REAUTH_SM_STATE		state;
} Reauth_SM;



void lib1x_reauthsm_init( Reauth_SM * reauth_sm );
void lib1x_trans_reauthsm( Global_Params * global , Reauth_SM * reauth_sm );
#endif /*LIB1x_REAUTH_SM_H*/
