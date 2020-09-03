#ifndef SYS_BNR_PMGR_H
#define SYS_BNR_PMGR_H
/* ------------------------------------------------------------------------
 *  FILE NAME  -  sys_bnr_pmgr.h
 * ------------------------------------------------------------------------
 * PURPOSE:
 *
 *  History 
 *
 *
 * ------------------------------------------------------------------------
 * Copyright(C)                             ACCTON Technology Corp. , 2007
 * ------------------------------------------------------------------------
 */


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_BNR_MGR_SetSysBnrMsg
 * ---------------------------------------------------------------------
 * PURPOSE: Set system banner message for specified type
 * INPUT	: sys_bnr_type - key to specifiy banner type (MOTD, Enable, Incoming)
              *msg  - the banner message of specified type
 * OUTPUT	: None
 * RETUEN:  : TRUE  -- success
 *            FALSE -- failure
 * NOTES	: None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_BNR_PMGR_SetSysBnrMsg(SYS_BNR_MGR_TYPE_T sys_bnr_type, UI8_T *msg);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_BNR_MGR_SetSysBnrMsgDelimitingChar
 * ---------------------------------------------------------------------
 * PURPOSE: Set system banner message delimiting character for specified type
 * INPUT	: sys_bnr_type - key to specifiy banner type (MOTD, Enable, Incoming)
              delimitingChar  - the delimiting character of specified type
 * OUTPUT	: None
 * RETUEN:  : TRUE  -- success
 *            FALSE -- failure
 * NOTES	: None
 * ---------------------------------------------------------------------
 */
 BOOL_T SYS_BNR_PMGR_SetSysBnrMsgDelimitingChar(SYS_BNR_MGR_TYPE_T sys_bnr_type, UI8_T delimitingChar);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_BNR_MGR_GetSysBnrMsgDelimitingChar
 * ---------------------------------------------------------------------
 * PURPOSE: Set system banner message delimiting character for specified type
 * INPUT	: sys_bnr_type - key to specifiy banner type (MOTD, Enable, Incoming)
              *delimitingChar  - the delimiting character of specified type
 * OUTPUT	: *delimitingChar  - the delimiting character of specified type
 * RETUEN:  : TRUE  -- success
 *            FALSE -- failure
 * NOTES	: None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_BNR_PMGR_GetSysBnrMsgDelimitingChar(SYS_BNR_MGR_TYPE_T sys_bnr_type, UI8_T *delimitingChar);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_BNR_MGR_GetNextSysBnrMsgByID
 * ---------------------------------------------------------------------
 * PURPOSE: get system banner message for specified type, section ID, and buffer 
 * INPUT	: sys_bnr_type - key to specifiy banner type (MOTD, Enable, Incoming)
              *section_id   - key to specifiy getting which section 
              buffer_size  - key to specifiy size to split data 
 * OUTPUT	: *msg  - the banner message of specified type
 * OUTPUT	: *section_id  -  current section_id
 * RETURN	: TRUE
 * NOTES	: If section ID is zero, it means to get first section data. 
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_BNR_PMGR_GetNextSysBnrMsgByID(SYS_BNR_MGR_TYPE_T sys_bnr_type, UI8_T *msg, UI32_T *section_id, UI32_T buffer_size);

#endif  /* End of SYS_BNR_MGR_H */

