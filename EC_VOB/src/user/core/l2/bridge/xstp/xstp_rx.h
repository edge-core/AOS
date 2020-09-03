/*-----------------------------------------------------------------------------
 * Module Name: xstp_rx.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Definitions for the XSTP RX
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    06/20/2001 - Allen Cheng, Created
 *    06/12/2002 - Kelly Chen, Added
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-----------------------------------------------------------------------------
 */

#ifndef _XSTP_RX_H
#define _XSTP_RX_H

#include "xstp_om.h"

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_RX_ProcessBpdu
 * ------------------------------------------------------------------------
 * PURPOSE  : Process the received BPDU
 * INPUT    : om_ptr            -- om pointer for this instance
 *            lport             -- BPDU incoming port
 *            mref_handle_pp    -- BPDU mref
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : To process a received BPDU, the state machine (PIM in RSTP,
 *            or PRX in MSTP) is triggered by setting the variable rcvdBpdu.
 *            In 802.1w, this variable is not set until the BPDU is recorded.
 *            In 802.1s, this variable is set in order to trigger the PRX in
 *            which setRcvdMsgs() invokes the function to record the BPDU.
 *-------------------------------------------------------------------------
 */
void    XSTP_RX_ProcessBpdu(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, L_MM_Mref_Handle_T **mref_handle_pp);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_RX_RecordBpdu
 * ------------------------------------------------------------------------
 * PURPOSE  : Copy the received BPDU pointer to the port om
 * INPUT    : lport     -- lport
 *            bpdu      -- BPDU
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void    XSTP_RX_RecordBpdu(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, XSTP_TYPE_Bpdu_T *bpdu);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_RX_GetMstidFromBpdu
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next mstid
 * INPUT    : index     -- index of the MSTI configuration message
 *            mst_bpdu  -- mst bpdu
 * OUTPUT   : None
 * RETURN   : XSTP_TYPE_CISTID (0) if the indexed MSTI message does not exist,
 *            or the MSTID
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_RX_GetMstidFromBpdu(UI32_T index, XSTP_TYPE_MstBpdu_T *mst_bpdu);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_RX_ValidateReceivedBpdu
 * ------------------------------------------------------------------------
 * PURPOSE  : Validate the received BPDUs
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 *            bpdu
 *            length    -- packet length including 802.2 LLC (3 bytes)
 * OUTPUT   : bpdu
 * RETURN   : TRUE if the bpdu is valid, else FALSE
 * NOTE     : Ref to the description in 14.4, IEEE Std 802.1s(D14.1)-2002.
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_RX_ValidateReceivedBpdu(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, XSTP_TYPE_Bpdu_T *bpdu, UI32_T length);

#endif /* _XSTP_RX_H */
