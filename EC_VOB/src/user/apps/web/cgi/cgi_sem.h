#ifndef CGI_SEM_H
#define CGI_SEM_H 1

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CGI_SEM_Create
 * ------------------------------------------------------------------------
 * PURPOSE  :   Create a semaphore object
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   None
 *-------------------------------------------------------------------------
 */
BOOL_T CGI_SEM_Create(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CGI_SEM_GetId
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get semaphore ID
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   Semaphore ID
 * NOTE     :   None
 *-------------------------------------------------------------------------
 */
UI32_T CGI_SEM_GetId();

#endif /* CGI_SEM_H */

