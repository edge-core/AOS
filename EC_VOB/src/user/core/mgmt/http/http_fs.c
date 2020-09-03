#if 0
/* MODULE NAME:  http_fs.c
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



/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"
#include "sys_type.h"
#include "sysfun.h"

#if (SYS_CPNT_HTTPS == TRUE)
//#include <ssl.h>
//#include <err.h>
//#include <http_def.h>
#include "http_certificate_file.c"
#include "http_pass_phrase_file.c"
#include "fs.h"
#include "fs_type.h"
#include "http_type.h"

#endif /* if SYS_CPNT_HTTPS */





/* NAMING CONSTANT DECLARATIONS
 */










/* MACRO FUNCTION DECLARATIONS
 */









/* DATA TYPE DECLARATIONS
 */











/* LOCAL SUBPROGRAM DECLARATIONS 
 */










/* STATIC VARIABLE DECLARATIONS 
 */

static UI32_T	Http_Fs_SemId;

#if (SYS_CPNT_HTTPS == TRUE)

static UI8_T	Files_Buffer[FILES_LENGTH];
static UI8_T	Server_Certificate_Temp_Buffer[SERVER_CERTIFICATE_FILE_LENGTH];
static UI8_T	Server_Private_Key_Temp_Buffer[SERVER_PRIVATE_KEY_FILE_LENGTH];
static UI8_T	Server_Pass_Phrase_Temp_Buffer[SERVER_PASS_PHRASE_FILE_LENGTH];
//static UI8_T	*CERTIFICATE_FILENAME = "$certificate";

#endif /* if SYS_CPNT_HTTPS */










/* EXPORTED SUBPROGRAM BODIES
 */

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
BOOL_T HTTP_FS_Init(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    


    /* BODY */
    if ( SYSFUN_CreateSem (/*SEM_FULL*/ 1, SYSFUN_SEM_FIFO, &Http_Fs_SemId) != SYSFUN_OK )
    {
		printf("Create Http_Om_SemId error \n");
        return FALSE;
    }

    return TRUE;
}



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
BOOL_T HTTP_FS_Enter_Critical_Section(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    


    /* BODY */
    if (SYSFUN_GetSem(Http_Fs_SemId, SYSFUN_TIMEOUT_WAIT_FOREVER) != SYSFUN_OK)
    {
        // Error
        return FALSE;
    }

    return TRUE;
}



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
BOOL_T HTTP_FS_Leave_Critical_Section(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    


    /* BODY */
    if (SYSFUN_SetSem(Http_Fs_SemId) != SYSFUN_OK)
    {
        // Error
        return FALSE;
    }

    return TRUE;
}



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
BOOL_T HTTP_FS_Init_Files()
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    UI8_T *offset;
    


    /* BODY */
    memset(Files_Buffer,0,FILES_LENGTH);
    offset = Files_Buffer;

	strcpy(offset,CERTIFICATE_FILE);
	offset = offset + SERVER_CERTIFICATE_FILE_LENGTH;

    strcpy(offset,PRIVATE_KEY_FILE);
	offset = offset + SERVER_PRIVATE_KEY_FILE_LENGTH;
    
	strcpy(offset,PASS_PHRASE);
	offset = offset + SERVER_PASS_PHRASE_FILE_LENGTH;
	
/*Isiah. 2002-06-06 */
	strcpy(offset,PKEY_512B_1);
	offset = offset + TEMP_RSA_KEY_512B_FILE_LENGTH;
    
	strcpy(offset,PKEY_512B_2);
	offset = offset + TEMP_RSA_KEY_512B_FILE_LENGTH;
    
	strcpy(offset,PKEY_512B_3);
	offset = offset + TEMP_RSA_KEY_512B_FILE_LENGTH;
    
	strcpy(offset,PKEY_512B_4);
	offset = offset + TEMP_RSA_KEY_512B_FILE_LENGTH;
    
	strcpy(offset,PKEY_768B_1);
	offset = offset + TEMP_RSA_KEY_768B_FILE_LENGTH;
    
	strcpy(offset,PKEY_768B_2);
	offset = offset + TEMP_RSA_KEY_768B_FILE_LENGTH;
    
	strcpy(offset,PKEY_768B_3);
	offset = offset + TEMP_RSA_KEY_768B_FILE_LENGTH;
    
	strcpy(offset,PKEY_768B_4);
	offset = offset + TEMP_RSA_KEY_768B_FILE_LENGTH;
    
	strcpy(offset,PKEY_1024B_1);
	offset = offset + TEMP_RSA_KEY_1024B_FILE_LENGTH;
    
	strcpy(offset,PKEY_1024B_2);
	offset = offset + TEMP_RSA_KEY_1024B_FILE_LENGTH;

	strcpy(offset,PKEY_1024B_3);
	offset = offset + TEMP_RSA_KEY_1024B_FILE_LENGTH;

	strcpy(offset,PKEY_1024B_4);
    
	FS_WriteFile(1,CERTIFICATE_FILENAME,"",FS_FILE_TYPE_PRIVATE,Files_Buffer,FILES_LENGTH,RESERVED_FILES_LENGTH);
    
    return TRUE;
}



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
BOOL_T HTTP_FS_Read_Files(UI8_T *buffer)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    UI8_T *buffer_offset,*files_offset;
    UI32_T len;
    UI32_T i;
    


    /* BODY */
    memset(Files_Buffer,0,FILES_LENGTH);
    
    FS_ReadFile(1,CERTIFICATE_FILENAME,Files_Buffer,FILES_LENGTH,&len);
    
    buffer_offset = buffer;
    files_offset = Files_Buffer;
    
    strcpy(buffer_offset,files_offset);
    buffer_offset = buffer_offset + SERVER_CERTIFICATE_FILE_LENGTH;
    files_offset = files_offset + SERVER_CERTIFICATE_FILE_LENGTH;

    strcpy(buffer_offset,files_offset);
    buffer_offset = buffer_offset + SERVER_PRIVATE_KEY_FILE_LENGTH;
    files_offset = files_offset + SERVER_PRIVATE_KEY_FILE_LENGTH;

    strcpy(buffer_offset,files_offset);
    buffer_offset = buffer_offset + SERVER_PASS_PHRASE_FILE_LENGTH;
    files_offset = files_offset + SERVER_PASS_PHRASE_FILE_LENGTH;

 /*Isiah. 2002-06-06 */
    for ( i=0 ; i<NUMBER_OF_TEMP_KEY ; i++ )
    {
    	strcpy(buffer_offset,files_offset);
        buffer_offset = buffer_offset + TEMP_RSA_KEY_512B_FILE_LENGTH;
        files_offset = files_offset + TEMP_RSA_KEY_512B_FILE_LENGTH;
    }

    for ( i=0 ; i<NUMBER_OF_TEMP_KEY ; i++ )
    {
    	strcpy(buffer_offset,files_offset);
        buffer_offset = buffer_offset + TEMP_RSA_KEY_768B_FILE_LENGTH;
        files_offset = files_offset + TEMP_RSA_KEY_768B_FILE_LENGTH;
    }

    for ( i=0 ; i<NUMBER_OF_TEMP_KEY-1 ; i++ )
    {
    	strcpy(buffer_offset,files_offset);
        buffer_offset = buffer_offset + TEMP_RSA_KEY_1024B_FILE_LENGTH;
        files_offset = files_offset + TEMP_RSA_KEY_1024B_FILE_LENGTH;
    }

    strcpy(buffer_offset,files_offset);

    return TRUE;
    
}



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
BOOL_T HTTP_FS_Set_All_Temp_PublicKey_Pair(UI8_T **tmpKey)
{
    /* LOCAL CONSTANT DECLARATIONS
    */



    /* LOCAL VARIABLES DECLARATIONS 
    */
    UI8_T *offset;
    UI32_T len;
    FS_File_Attr_T file_attr;
    UI32_T i;
    


    /* BODY */
    memset(Files_Buffer,0,FILES_LENGTH);
    offset = Files_Buffer;
    
    strcpy(file_attr.file_name,CERTIFICATE_FILENAME);
    file_attr.file_type_mask = FS_FILE_TYPE_MASK(FS_FILE_TYPE_PRIVATE);
    FS_GetFileInfo(1,&file_attr);
    if ( (file_attr.create_time == 0) || (file_attr.file_size != FILES_LENGTH) )
    {
        HTTP_FS_Init_Files();
    }

	FS_ReadFile(1,CERTIFICATE_FILENAME,Files_Buffer,FILES_LENGTH,&len);

    offset = offset + SERVER_CERTIFICATE_FILE_LENGTH;
    offset = offset + SERVER_PRIVATE_KEY_FILE_LENGTH;
	offset = offset + SERVER_PASS_PHRASE_FILE_LENGTH;

    strcpy(offset,tmpKey[0]);
    offset = offset + TEMP_RSA_KEY_512B_FILE_LENGTH;	
    strcpy(offset,tmpKey[1]);
    offset = offset + TEMP_RSA_KEY_512B_FILE_LENGTH;	
    strcpy(offset,tmpKey[2]);
    offset = offset + TEMP_RSA_KEY_512B_FILE_LENGTH;	
    strcpy(offset,tmpKey[3]);
    offset = offset + TEMP_RSA_KEY_512B_FILE_LENGTH;	
    
    strcpy(offset,tmpKey[4]);
    offset = offset + TEMP_RSA_KEY_768B_FILE_LENGTH;
    strcpy(offset,tmpKey[5]);
    offset = offset + TEMP_RSA_KEY_768B_FILE_LENGTH;
    strcpy(offset,tmpKey[6]);
    offset = offset + TEMP_RSA_KEY_768B_FILE_LENGTH;
    strcpy(offset,tmpKey[7]);
    offset = offset + TEMP_RSA_KEY_768B_FILE_LENGTH;
    
    strcpy(offset,tmpKey[8]);
    offset = offset + TEMP_RSA_KEY_1024B_FILE_LENGTH;
    strcpy(offset,tmpKey[9]);
    offset = offset + TEMP_RSA_KEY_1024B_FILE_LENGTH;
    strcpy(offset,tmpKey[10]);
    offset = offset + TEMP_RSA_KEY_1024B_FILE_LENGTH;
    strcpy(offset,tmpKey[11]);

#if 0
	if ( nKeyLen == 512 )
	{
	    for ( i=1 ; i<index ; i++ )
	    {
        	offset = offset + TEMP_RSA_KEY_512B_FILE_LENGTH;
	    }
		strcpy(offset,pkey);
	}
	else if ( nKeyLen == 768 )
	{
	    offset = offset + (4*TEMP_RSA_KEY_512B_FILE_LENGTH);
	    for ( i=1 ; i<index ; i++ )
	    {
        	offset = offset + TEMP_RSA_KEY_768B_FILE_LENGTH;
	    }
		strcpy(offset,pkey);
	}
	else /* nkeyLen == 1024 */
	{
	    offset = offset + (4*TEMP_RSA_KEY_512B_FILE_LENGTH);
	    offset = offset + (4*TEMP_RSA_KEY_768B_FILE_LENGTH);
	    for ( i=1 ; i<index ; i++ )
	    {
        	offset = offset + TEMP_RSA_KEY_1024B_FILE_LENGTH;
	    }
		strcpy(offset,pkey);
	}
#endif
	
	/*FS_DeleteFile(1,CERTIFICATE_FILENAME);*/
	FS_WriteFile(1,CERTIFICATE_FILENAME,"",FS_FILE_TYPE_PRIVATE,Files_Buffer,FILES_LENGTH,RESERVED_FILES_LENGTH);
	
	return TRUE;
}



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
BOOL_T HTTP_FS_Set_Server_Certificate(UI8_T *newcert)
{
    /* LOCAL CONSTANT DECLARATIONS
    */



    /* LOCAL VARIABLES DECLARATIONS 
    */
    UI8_T *offset;
    


    /* BODY */
    offset = Files_Buffer;
	strcpy(offset,newcert);
    
	/*FS_DeleteFile(1,CERTIFICATE_FILENAME);*/
	FS_WriteFile(1,CERTIFICATE_FILENAME,"",FS_FILE_TYPE_PRIVATE,Files_Buffer,FILES_LENGTH,RESERVED_FILES_LENGTH);
	
	return TRUE;
}



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
BOOL_T HTTP_FS_Set_Server_Private_Key(UI8_T *key, UI8_T *passwd)
{
    /* LOCAL CONSTANT DECLARATIONS
    */



    /* LOCAL VARIABLES DECLARATIONS 
    */
    UI8_T *offset;
    UI32_T len;
    


    /* BODY */
    offset = Files_Buffer;
    offset = offset + SERVER_CERTIFICATE_FILE_LENGTH;

	strcpy(offset,key);
    offset = offset + SERVER_PRIVATE_KEY_FILE_LENGTH;
	
	strcpy(offset,passwd);
	
	/*FS_DeleteFile(1,CERTIFICATE_FILENAME);*/
	FS_WriteFile(1,CERTIFICATE_FILENAME,"",FS_FILE_TYPE_PRIVATE,Files_Buffer,FILES_LENGTH,RESERVED_FILES_LENGTH);
	
	return TRUE;
}



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
BOOL_T HTTP_FS_Get_Server_Certificate_Pointer(UI8_T **buffer)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    


    /* BODY */
	*buffer = Server_Certificate_Temp_Buffer;
    return TRUE;
   
}



/* FUNCTION NAME:  HTTP_FS_Get_Server_Private_Key_Pointer
 * PURPOSE: 
 *          Get buffer pointer of server Private Key.
 *
 * INPUT:   
 *          none.
 *
 * OUTPUT:  
 *          UI8_T ** -- server PrivateKey Buffer
 *               
 * RETURN:  
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_FS_Get_Server_Private_Key_Pointer(UI8_T **buffer)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    


    /* BODY */
	*buffer = Server_Private_Key_Temp_Buffer;
    return TRUE;
   
}



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
BOOL_T HTTP_FS_Get_Server_Pass_Phrase_Pointer(UI8_T **buffer)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    


    /* BODY */
	*buffer = Server_Pass_Phrase_Temp_Buffer;
    return TRUE;
   
}



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
BOOL_T HTTP_FS_Set_Certificate_to_Flash()
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    UI8_T *offset;
    


    /* BODY */
//    memset(Files_Buffer,0,FILES_LENGTH);
    offset = Files_Buffer;

	strcpy(offset,Server_Certificate_Temp_Buffer);
	offset = offset + SERVER_CERTIFICATE_FILE_LENGTH;

    strcpy(offset,Server_Private_Key_Temp_Buffer);
	offset = offset + SERVER_PRIVATE_KEY_FILE_LENGTH;
    
	strcpy(offset,Server_Pass_Phrase_Temp_Buffer);
    
	FS_WriteFile(1,CERTIFICATE_FILENAME,"",FS_FILE_TYPE_PRIVATE,Files_Buffer,FILES_LENGTH,RESERVED_FILES_LENGTH);
    
    return TRUE;
}






#endif /* if SYS_CPNT_HTTPS */






















/* LOCAL SUBPROGRAM BODIES
 */


#endif /* #if 0 */








