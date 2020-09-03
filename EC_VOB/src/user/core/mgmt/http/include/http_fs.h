/* MODULE NAME: http_fs.h
* PURPOSE:
*   {1. What is covered in this file - function and scope.}
*   {2. Related documents or hardware information}
* NOTES:
*     {Something must be known or noticed}
*   {1. How to use these functions - Give an example.}
*   {2. Sequence of messages if applicable.}
*   {3. Any design limitation}
*   {4. Any performance limitation}
*   {5. Is it a reusable component}
*
* CREATOR:  Isiah           Date 2002-04
*
* Copyright(C)      Accton Corporation, 2002
*/



#ifndef HTTP_FS_H

#define HTTP_FS_H



/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"
#include "sys_type.h"
//#include "http_type.h"

//#if (SYS_CPNT_HTTPS == TRUE)
//#include <ssl.h>
//#endif /* if (SYS_CPNT_HTTPS == TRUE) */





/* NAMING CONSTANT DECLARATIONS
 */





/* MACRO FUNCTION DECLARATIONS
 */



/* DATA TYPE DECLARATIONS
 */



/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-----------------------------------------------------------------------*/
/* FUNCTION NAME:  HTTP_FS_Init
 * PURPOSE:
 *          Initiate the semaphore for HTTP_FS objects
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in HTTP_MGR_Init.
 */
BOOL_T HTTP_FS_Init(void);



/* FUNCTION NAME:  HTTP_FS_Enter_Critical_Section
 * PURPOSE:
 *          Enter critical section before a task invokes the http_fs objects.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_FS_Enter_Critical_Section(void);



/* FUNCTION NAME:  HTTP_FS_Leave_Critical_Section
 * PURPOSE:
 *          Leave critical section after a task invokes the http_fs objects.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_FS_Leave_Critical_Section(void);



/*-----------------------------------------------------------------------*/





/*-----------------------------------------------------------------------*/
#if (SYS_CPNT_HTTPS == TRUE)

/* FUNCTION NAME:  HTTP_FS_Init_Files
 * PURPOSE:
 *          Set default files to Files_Buffer and file system.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          Check if there is alrady allocated space existed. If not, then call this API to allocate one.
 */
BOOL_T HTTP_FS_Init_Files();



/* FUNCTION NAME:  HTTP_FS_Read_Files
 * PURPOSE:
 *          Get all files from file system to Files_Buffer.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI8_T * -- Files_Buffer
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_FS_Read_Files(UI8_T *);



/* FUNCTION NAME:  HTTP_FS_Set_All_Temp_PublicKey_Pair
 * PURPOSE:
 *          Store all temp RSA key to file system.
 *
 * INPUT:
 *          UI8_T ** -- array of pointer of temp PublicKey Pair.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in HTTP_MGR_Set_All_Temp_PublicKey_Pair_To_Flash().
 */
BOOL_T HTTP_FS_Set_All_Temp_PublicKey_Pair(UI8_T **tmpKey);



/* FUNCTION NAME:  HTTP_FS_Set_Server_Certificate
 * PURPOSE:
 *          Store server certificate to file system and Files_Buffer.
 *
 * INPUT:
 *          UI8_T *  -- server certificate.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in HTTP_MGR_Set_Server_Certificate.
 */
BOOL_T HTTP_FS_Set_Server_Certificate(UI8_T *);



/* FUNCTION NAME:  HTTP_FS_Set_Server_Private_Key
 * PURPOSE:
 *          Store server private key to file system and Files_Buffer.
 *
 * INPUT:
 *          UI8_T *  -- private key.
 *
 *          UI8_T *  -- pass phrase.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in HTTP_MGR_Set_Server_Private_Key.
 */
BOOL_T HTTP_FS_Set_Server_Private_Key(UI8_T *, UI8_T *);



/* FUNCTION NAME:  HTTP_FS_Get_Server_certificate_Pointer
 * PURPOSE:
 *          Get buffer pointer of server certificate.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI8_T ** -- server certificate Buffer
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_FS_Get_Server_Certificate_Pointer(UI8_T **);



/* FUNCTION NAME:  HTTP_FS_Get_Server_Private_Key_Pointer
 * PURPOSE:
 *          Get buffer pointer of server Private Key.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI8_T ** -- server Private Key Buffer
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_FS_Get_Server_Private_Key_Pointer(UI8_T **);



/* FUNCTION NAME:  HTTP_FS_Get_Server_Pass_Phrase_Pointer
 * PURPOSE:
 *          Get buffer pointer of server Pass Phrase.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI8_T ** -- server Pass Phrase Buffer
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_FS_Get_Server_Pass_Phrase_Pointer(UI8_T **);



/* FUNCTION NAME:  HTTP_FS_Set_Certificate_to_Flash
 * PURPOSE:
 *          Set new files to Files_Buffer and file system.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_FS_Set_Certificate_to_Flash();







#endif /* if (SYS_CPNT_HTTPS == TRUE)*/
/*-----------------------------------------------------------------------*/








#endif /* #ifndef HTTP_FS_H */









