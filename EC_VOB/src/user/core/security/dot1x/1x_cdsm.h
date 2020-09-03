
#ifndef LIB1X_CDSM_H
#define LIB1X_CDSM_H

#include "1x_types.h"

struct Auth_Pae_tag;
struct Global_Params_tag;	/* These have been defined in 1x_common.h */

typedef struct CtrlDirSM_tag
{
	CTRL_SM_STATE		state;

	DIRECTION		adminControlledDirections;
	DIRECTION		operControlledDirections;
	BOOLEAN			bridgeDetected;

} CtrlDirSM;

void lib1x_cdsm_init( CtrlDirSM * ctrl_sm );
void lib1x_trans_cdsm( struct Auth_Pae_tag * auth_params, struct Global_Params_tag * global, CtrlDirSM * dirsm);
#endif /*LIB1X_CDSM_H*/

