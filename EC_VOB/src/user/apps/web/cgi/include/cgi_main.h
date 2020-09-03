/* ----------------------------------------------------------------------
 * FILE NAME: cg_main.h

 * AUTHOR:
 * PLATFORM:
 * ABSTRACT:
 * Module for Fileless CGI.
 *.
 * CLASSES AND FUNCTIONS:
 * USAGE:
 * ---------------------------------------------------------------------- */

#ifndef CGI_MAIN_H
#define CGI_MAIN_H

#if __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------------
 * CGI_MAIN_InitiateSystemResources: Initialize system resources.
 * ---------------------------------------------------------------------- */
BOOL_T CGI_MAIN_InitiateSystemResources (void);

/* ----------------------------------------------------------------------
 * CGI_MAIN_Create_InterCSC_Relation: Callback for creating inter-CSC
 *                                    relationships.
 * ---------------------------------------------------------------------- */
void CGI_MAIN_Create_InterCSC_Relation (void);

int cgi_SendBuffer(HTTP_Connection_T *http_connection, int nSock);

#if __cplusplus
}
#endif

#endif  /* CGI_MAIN_H */
