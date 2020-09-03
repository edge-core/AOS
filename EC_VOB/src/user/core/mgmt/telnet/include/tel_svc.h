/*===========================================================================
Filename:       INCL\TELNET\TEL_SVC.H
Purpose:        Definition of TELNET service
Reference:
=============================================================================
LOG:

    WHO             WHEN                                WHAT
------------- -------- ------------------------------------------------------
  Mike Yu         07.03.96      Created
============================================================================*/
/* the message types from UI to TELNET  */
#define UI_ACK_OPEN                     1       /* UI accpet the open session request   */
#define UI_REJ_OPEN                     2       /* UI reject the open session request   */
#define UI_DATA                         3       /* UI send data to telnet               */
#define UI_REQ_CLOSE					4       /* UI request to close the session      */
#define UI_ACK_CLOSE					5       /* UI ACK telnet's close request        */
#define	TN_RT_TCP						6		/* TN request itself ReRX to TCP		*/

/* the message types from TCP to telnet */
#define TCP_DATA                        0       /* TCP send data to telnet              */
                                                /* same as old TCP service              */

#define NUM_OF_HDL_OF_TN        7       /* number of handle of TN mailbox               */

/* Configuration related MACROs */
#define TEL_MAX_SESSION_NO      3       /* the max number of telnet sessions    */

/* function defintions of TCP call back routine */
#define TCP_INCOMING            0
#define TCP_REQ_CLOSE           1
#define TCP_ACK_CLOSE           2

/* mailbos string name of telnet service, other processes can use this string
 * to retrieve the corresponding MID
 */
#define TELNET_SVC              "TN_MBX"
