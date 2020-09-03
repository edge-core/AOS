/* MODULE NAME: datastore_util.h
 * PURPOSE:
 *   Provide some functions to read/write a XML file.
 * NOTES:
 *   None
 *
 * HISTORY:
 *    mm/dd/yy
 *    01/27/15 -- Kelly Chen, Create
 *
 * Copyright(C) Accton Corporation, 2015
 */

#ifndef DATASTORE_UTIL_H
#define DATASTORE_UTIL_H

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include "sys_type.h"
/* NAMING CONSTANT DECLARATIONS
 */
 
/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    DATASTORE_UTIL_ERROR_NONE = 0, /* Success. */
    DATASTORE_UTIL_ERROR_PARAM,    /* Invalid parameter. */
    DATASTORE_UTIL_ERROR_FAIL,     /* Operation Fail. */
} DATASTORE_UTIL_Error_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
void DATASTORE_UTIL_InitDatastore(char *datastore_path);
DATASTORE_UTIL_Error_T DATASTORE_UTIL_AddNode(char *xml_file, char *xpath_expr, char* child_info, char* href, char* prefix);
DATASTORE_UTIL_Error_T DATASTORE_UTIL_RemoveNode(char *xml_file, char *xpath_expr, char *content, BOOL_T is_key);
DATASTORE_UTIL_Error_T DATASTORE_UTIL_GetNodeByXpath(char *xml_file, char *xpath_expr, char *file_path, BOOL_T *exist);
DATASTORE_UTIL_Error_T DATASTORE_UTIL_CopyNodeByXpath(char *xml_file, char *dest_xpath_expr, char *src_xpath_expr);
DATASTORE_UTIL_Error_T DATASTORE_UTIL_ModifyNodeByXpath(char *xml_file, char *xpath_expr, char *content);

#endif /* End of DATASTORE_UTIL_H */