
#ifndef LIB1x_KXSM_H
#define LIB1x_KXSM_H

#include "1x_types.h"

struct Auth_Pae_tag;
struct Global_Params_tag;

typedef struct Auth_keyxmitSM_tag
{
	AUTH_KEYSM	state;
	BOOLEAN		keyAvailable;
	BOOLEAN		keyTxEnabled;
} Auth_KeyxmitSM;


void lib1x_trans_kxsm( struct Auth_Pae_tag * auth_pae, struct Global_Params_tag * global, Auth_KeyxmitSM * key_sm );
void lib1x_kxsm_init( Auth_KeyxmitSM * key_sm );
void lib1x_kxsm_key_transmit( struct Auth_Pae_tag * auth_pae, struct Global_Params_tag * global, Auth_KeyxmitSM *  key_sm );

void lib1x_authxmitsm_txKey( struct Auth_Pae_tag * auth_pae, int currentId);


#endif /*LIB1x_KXSM_H*/
