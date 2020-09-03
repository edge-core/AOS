#ifndef  LIB1X_BAUTH_SM_H
#define  LIB1X_BAUTH_SM_H 


#define	LIB1X_BSM_SUPPTIMEOUT		30
#define	LIB1X_BSM_SVRTIMEOUT		30
#define	LIB1X_BSM_MAXREQ			2

struct Auth_Pae_tag;
struct Global_Params_tag;

typedef struct Bauth_SM_tag
{
	BAUTH_SM_STATE		state;

	int			reqCount;
					/* A counter used to determine how many EAP Request packets
					   have been sent to the Supplicant without receiving a response.*/
	BOOL_T			rxResp; /* if a EAPOL PDU of type EAP packet rcvd from supp carrying a Request*/
	BOOL_T			aSuccess; 	/* if Accept pkt recvd from Auth Server*/
	BOOL_T			aFail;		/* true if reject pkt rcvd from auth svr.*/
	BOOL_T			aReq;		/* true if eap req pkt rcvd frm auth svr.*/
	int			idFromServer;	/* most recent EAP success, failure or req pkt rcvd frm auth svr.*/
//	int			suppTimeout;
	int			serverTimeout;
//	int			maxReq;
	BOOL_T			reqState;
} Bauth_SM;

/* The functions exported.*/
void lib1x_bauthsm_init( Bauth_SM * bauth_sm );
void lib1x_bauthsm( struct Auth_Pae_tag * , struct Global_Params_tag * , Bauth_SM * );
BOOLEAN lib1x_trans_bauthsm( struct Auth_Pae_tag * , struct Global_Params_tag *, Bauth_SM * );
void lib1x_exec_bauthsm( struct Auth_Pae_tag * , struct Global_Params_tag * , Bauth_SM * );
void lib1x_bauthsm_abortAuth();
void lib1x_bauthsm_txReq( Global_Params * global, int identifier );
void lib1x_bauthsm_sendRespToServer( Global_Params * global, int identifier );
#endif /*LIB1X_BAUTH_SM_H*/
