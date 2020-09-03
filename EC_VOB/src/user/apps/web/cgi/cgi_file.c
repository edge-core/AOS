/* ----------------------------------------------------------------------
 * FILE NAME: cgi\cg_file.c

 * AUTHOR: Chung, Daniel K. (Zhong Qiyao)
 * PLATFORM: Microsoft Windows 95, with pSOS Diab compiler for 68K/PowerPC.

 * ABSTRACT:
 * This is part of the embedded program for pSOS series.
 * Entire firmware is integrated by project manager.

 * Module for Fileless CGI.
 * File to send file to socket.

 * CLASSES AND FUNCTIONS:


 * HISTORY:
 * 1999-03-08 (Mon): Created by Daniel K. Chung.

 * COPYRIGHT (C) Accton Technology Corporation, 1999.
 * ---------------------------------------------------------------------- */
#include "sysfun.h"
#include "l_mm.h"
#include "l_inet.h"
#include "fs.h"
#include "cgi.h"
#include "cgi_file.h"
#include "cgi_real.h"
#include "cgi_cache.h"
#include "cgi_sem.h"
#include "cgi_debug.h"

#define IS_THIS_DEBUG_ON()  (CGI_DEBUG_GetFlag() & CGI_DEBUG_FILE)

#define CGI_FILE_ENTER_CRITICAL_SECTION()                                \
    cgi_file_orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(CGI_SEM_GetId());

#define CGI_FILE_LEAVE_CRITICAL_SECTION()                              \
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(CGI_SEM_GetId(), cgi_file_orig_priority);


#define CGI_FILE_RETURN_AND_LEAVE_CRITICAL_SECTION(RET_VAL) { \
    CGI_FILE_LEAVE_CRITICAL_SECTION(); \
    return (RET_VAL); \
}

#define CGI_FILE_RETURN_AND_LEAVE_CRITICAL_SECTION_WITHOUT_RETUEN_VALUE() { \
    CGI_FILE_LEAVE_CRITICAL_SECTION(); \
    return; \
}


#define CGI_FILE_DEFAULT_PAGE_CFG_FILE  "/default_page.cfg"

/* ----------------------------------------------------------------------
 * Forward declarations.
 * ---------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
 * Constants.
 * ---------------------------------------------------------------------- */

enum CGI_LINEMODE_T {
    CGI_LINEMODE_NORMAL, CGI_LINEMODE_REAL, CGI_LINEMODE_REALLOOP, CGI_LINEMODE_INVALID_QUERY
};

enum
{
    CGI_FILE_MAX_CACHE_SIZE   = 3 * 1024 * 1024,  /* in bytes */
};


/* ----------------------------------------------------------------------
 * Types.
 * ---------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
 * Global variables (must be kept contant).
 * ---------------------------------------------------------------------- */
static const char * cgi_file_bin_ext_list[] =
{
    ".gif",
    ".jpg",
    ".jpeg",
    ".bmp",
    ".png",
    ".bin",
    ".bix",
};

static CGI_CACHE_T *cgi_file_cache;
static CGI_FILE_PATH_T *cgi_file_default_page;
UI32_T cgi_file_orig_priority;

/* ----------------------------------------------------------------------
 * cgi_file_lineend: Find end of line in file.
 * ---------------------------------------------------------------------- */

char * cgi_file_lineend (char *pLineBegin, char *pFileEnd)
{
    char *pLineEnd = pLineBegin;

    while ((pLineEnd < pFileEnd) && (*pLineEnd != '\n'))
    {
        pLineEnd++;
    }

    return pLineEnd; /* may be 1 after last byte of file */
}

/* ----------------------------------------------------------------------
 * cgi_file_token: Get token from file.
 * ---------------------------------------------------------------------- */

void cgi_file_token (char **pBegin, char **pEnd, char *pFileEnd)
{
    char *pTokenBegin, *pTokenEnd;

    /* find begin */
    pTokenBegin = *pBegin;

    while ((pTokenBegin < pFileEnd) &&
        ! (isalpha (*pTokenBegin) || isdigit (*pTokenBegin)) )
    {
        pTokenBegin++;
    }

    /* find end */
    pTokenEnd = pTokenBegin;

    while ((pTokenEnd < pFileEnd) &&
        (isalpha (*pTokenEnd) || isdigit (*pTokenEnd)) )
    {
        pTokenEnd++;
    }

    /* return actual begin and end */
    /* may be 1 after last byte of file */
    *pBegin = pTokenBegin;
    *pEnd = pTokenEnd;
}

/* ----------------------------------------------------------------------
 * cgi_file_RealTable: Table of "real" markers.

 * MUST BE IN ASCENDING ALPHABETICAL ORDER.
 * ---------------------------------------------------------------------- */
CGI_REAL_TABLE_T cgi_file_RealTable [] =
{
    {"REAL",            CGI_LINEMODE_REAL,      0,  {FROM_NONE,  FROM_NONE, 0},  0},
    {"REALINDEX",       CGI_LINEMODE_REAL,      1,  {FROM_FILE,  FROM_NONE, 0},  0},
    {"REALINDEXINDEX",  CGI_LINEMODE_REAL,      2,  {FROM_FILE,  FROM_FILE, 0},  0},
    {"REALINDEXLOOP",   CGI_LINEMODE_REALLOOP,  2,  {FROM_FILE,  FROM_NONE, 0},  2},
    {"REALINDEXLOOPINDEX", CGI_LINEMODE_REALLOOP, 3,  {FROM_FILE, FROM_NONE, FROM_FILE},  2},/* erica 09/18/02 for 1w*/
    {"REALINDEXLOOPQUERY", CGI_LINEMODE_REALLOOP, 3,  {FROM_FILE, FROM_NONE, FROM_QUERY}, 2}, /* autwo 07/11/02 for 1w & 1s*/
    {"REALINDEXQUERY",  CGI_LINEMODE_REAL,      2,  {FROM_FILE, FROM_QUERY,0},   0}, /*autwo4 for mirror_port.htm */
    {"REALINDEXQUERYQUERY",  CGI_LINEMODE_REAL,      3,  {FROM_FILE, FROM_QUERY,FROM_QUERY},   0}, /* stella add */
    {"REALLOOP",        CGI_LINEMODE_REALLOOP,  1,  {FROM_NONE,  FROM_NONE, 0},  1},
    {"REALQUERY",       CGI_LINEMODE_REAL,      1,  {FROM_QUERY, FROM_NONE, 0},  0},
    {"REALQUERYINDEX",  CGI_LINEMODE_REAL,  	2,  {FROM_QUERY, FROM_FILE, 0},  0},
    {"REALQUERYINDEXLOOP",  CGI_LINEMODE_REALLOOP, 	3,  {FROM_QUERY, FROM_FILE, 0},  3},
    {"REALQUERYLOOP",   CGI_LINEMODE_REALLOOP,  2,  {FROM_QUERY, FROM_NONE, 0},  2},
    {"REALQUERYLOOPINDEX", CGI_LINEMODE_REALLOOP, 3,  {FROM_QUERY, FROM_NONE, FROM_FILE},  2},/* erica 09/18/02 for 1w*/
    {"REALQUERYLOOPQUERY", CGI_LINEMODE_REALLOOP, 3,  {FROM_QUERY, FROM_NONE, FROM_QUERY}, 2}, /* autwo 07/11/02 for 1w & 1s*/
    {"REALQUERYQUERY",  CGI_LINEMODE_REAL,      2,  {FROM_QUERY, FROM_QUERY,0},  0},
    {"REALQUERYQUERYINDEX", CGI_LINEMODE_REAL, 3,  {FROM_QUERY, FROM_QUERY, FROM_FILE}, 0}, /* autwo 07/11/02 for 1w & 1s*/
    {"REALQUERYQUERYQUERY", CGI_LINEMODE_REAL, 3,  {FROM_QUERY, FROM_QUERY, FROM_QUERY}, 0}, /* autwo 07/11/02 for 1w & 1s*/
};

/* ----------------------------------------------------------------------
 * cgi_file_KeyCompar: Global comparison function for binary search.
 * ---------------------------------------------------------------------- */
int cgi_file_KeyCompar (const void *key,const void *member)
{
    char       *szKey = (char *) key;
    CGI_REAL_TABLE_T *pTableMember = (CGI_REAL_TABLE_T *) member;
    int ret = strcmp (szKey, pTableMember->szName);
    return ret;
}

/* ----------------------------------------------------------------------
 * cgi_file_sendtext: Send text file to socket.
 * ---------------------------------------------------------------------- */
void cgi_file_sendtext (HTTP_Connection_T *http_connection, int sock, envcfg_t *envcfg, envcfg_t *envQuery, char *pData, CGI_SIZE_T nSize)
{
    char            *pLineBegin = pData;
    char            *pLineEnd;
    char            szTokenBuf[3][30];
    char            *pTokenBegin, *pTokenEnd;
    char            *pFileEnd = pData + nSize;  /* 1 after last byte */
    int             nLineMode = CGI_LINEMODE_NORMAL;
    char            *pRealBegin, *pRealEnd;
    CGI_REAL_INFO_T realVar;
    char            *pVal, *pEnd;
    char            szName [20];
    CGI_REAL_TABLE_T *pTable, *pFound;
    CGI_SIZE_T      nTableSize;
    int             i;

   /* send header */
   cgi_SendHeader(http_connection, sock, -1, envcfg);

    for (i = 0; i < 3; i++)
    {
        szTokenBuf [i][0] = '\0';
    }

    /* for each line */
    while (pLineBegin < pFileEnd)
    {
        /* find end of line */
        pLineEnd = cgi_file_lineend (pLineBegin, pFileEnd);

        /* check mode */

        switch (nLineMode)
        {
        case CGI_LINEMODE_NORMAL:

            /* check fake marker (at beginning of line)
             */
            if (pLineEnd - pLineBegin >= 9 &&
                (strncmp(pLineBegin, "<!--#FAKE", 9) == 0 ||
                 strncmp(pLineBegin, "/*--#FAKE", 9) == 0) )
            {
                /* prepare for next line
                 */
                pLineBegin = pLineEnd + 1;

              	while (pLineBegin < pFileEnd)
                {
                    /* find end of line
                     */
                    pLineEnd = cgi_file_lineend(pLineBegin, pFileEnd);

                    /* check end fake marker (at beginning of line)
                     */
                    if (pLineEnd - pLineBegin >= 12 &&
                        (strncmp(pLineBegin, "<!--#ENDFAKE", 12) == 0 ||
                         strncmp(pLineBegin, "/*--#ENDFAKE", 12) == 0) )
                    {
                        pLineBegin = pLineEnd + 1;
                        goto LineEnd;
                    }

                    /* prepare for next line
                     */
                    pLineBegin = pLineEnd + 1;

                }
            }
            /* check real marker (at beginning of line) */
            else if (pLineEnd - pLineBegin >= 5 &&
                (strncmp(pLineBegin, "<!--#", 5) == 0 ||
                strncmp(pLineBegin, "/*--#", 5) == 0) )
            {
                /* check REAL */
                pTokenBegin = pLineBegin + 5;
                cgi_file_token (&pTokenBegin, &pTokenEnd, pFileEnd);

	            pTable = cgi_file_RealTable;
                nTableSize = sizeof (cgi_file_RealTable);

                // get real key
                memcpy (szName, pTokenBegin, pTokenEnd - pTokenBegin);
                szName[pTokenEnd - pTokenBegin] = 0;

                /* use binary search */
	            if ((pFound = (CGI_REAL_TABLE_T *) bsearch (szName, pTable,
		                        nTableSize / sizeof (CGI_REAL_TABLE_T),
		                        sizeof (CGI_REAL_TABLE_T),
		                        &cgi_file_KeyCompar)) != NULL)
                {
                    //get index variable and loop keey
                    /* set real begin */
                    pRealBegin = pLineEnd + 1;

                    realVar.indexQt = pFound->indexQt;
                    nLineMode = pFound->mode;

					/* reset all strings */
					for (i = 0; i < 3; i++)
					{
						szTokenBuf [i][0] = '\0';
					}

                    for (i = 0; i<pFound->indexQt;i++)
                    {
                        // get the first token
                        pTokenBegin = pTokenEnd;

                        cgi_file_token (&pTokenBegin, &pTokenEnd, pFileEnd);
                        memcpy (szTokenBuf[i], pTokenBegin, pTokenEnd - pTokenBegin);
                        szTokenBuf[i][pTokenEnd - pTokenBegin] = '\0';

						/* direct value, e.g. 1 */
                        if (pFound->indexType[i] == FROM_FILE)
                        {
                            realVar.indexArr [i] = strtoul (szTokenBuf[i], (char **)&pEnd, 10);
                        }

                        /* query value, e.g. port */
                        else if (pFound->indexType[i] == FROM_QUERY)
                        {
                            if ((pVal = (char *)cgi_query_lookup (envQuery, (I8_T *) szTokenBuf[i])) == NULL)
                            {
                                nLineMode = CGI_LINEMODE_INVALID_QUERY;
                                goto LineEnd;
                            }

                            /* integer */
                            if (strchr (pVal, '.') == 0)
                            {
                            	realVar.indexArr [i] = strtoul (pVal, (char **)&pEnd, 10);
                            }

                            /* IP address */
                            else
                            {
                                /* IP -> UI32 NETWORK ORDER */
                                L_INET_Aton((UI8_T *) pVal, &(realVar.indexArr [i]));
                            }
                        }

                        /* no value */
                        else
                        {
                            realVar.indexArr [i] = 0;
                        }
                    }

                    // get loop key

                    if (pFound->loopToken >= 1 && pFound->loopToken <= 3)
                    {
                        realVar.loopkeyP = szTokenBuf[pFound->loopToken - 1];
                        realVar.loopToken = pFound->loopToken;      /* autwo 07/11/02 */
                    }
                    else
                    {
                        realVar.loopkeyP = 0;
                        realVar.loopToken = 0;   /* autwo 07/11/02 */
                    }
                    goto LineEnd;

                }
                else
                {
                    goto LineEnd;
                }
            }
            /* normal line */
            else
            {
                /* note: use cgi_SendBin, because string does not have '\0' */
                if (cgi_SendBin(http_connection, sock, pLineEnd - pLineBegin, (unsigned char *) pLineBegin) != 0)
                {
                    goto Exit;
                }

                /* can use cgi_SendText */
                if (cgi_SendText(http_connection, sock, (const char *)"\n") != 0)
                {
                    goto Exit;
                }
                goto LineEnd;
            }

            break;

        case CGI_LINEMODE_REAL:
        case CGI_LINEMODE_REALLOOP:
        case CGI_LINEMODE_INVALID_QUERY:

            /* check ENDREAL */
            if (pLineEnd - pLineBegin >= 5 &&
                (strncmp (pLineBegin, "<!--#", 5) == 0 ||
                strncmp (pLineBegin, "*//*#", 5) == 0) )
            {
                /* check token */
                pTokenBegin = pLineBegin + 5;
                cgi_file_token (&pTokenBegin, &pTokenEnd, pFileEnd);

                if (pTokenEnd - pTokenBegin == 7 &&
                    (strncmp (pTokenBegin, "ENDREAL", 7) == 0))
                {
                    /* set real end */
                    pRealEnd = pLineBegin;

                    /* send real lines */
                    switch (nLineMode)
                    {
                    case CGI_LINEMODE_REAL:
                        if (cgi_real_send (http_connection, sock, envcfg, envQuery, pRealBegin, pRealEnd,
                            &realVar) != 0)
                            goto Exit;

                        break;

                    case CGI_LINEMODE_REALLOOP:
                        if (cgi_real_sendloop (http_connection, sock, envcfg, envQuery, pRealBegin, pRealEnd,
                            &realVar) != 0)
                            goto Exit;

                        break;

                    default:
                        break;
                    }

                    /* restore line mode */
                    nLineMode = CGI_LINEMODE_NORMAL;
                }
            }

            break;

        default:
            break;
        }

LineEnd:
        /* prepare for next line */
        if (pLineEnd == pFileEnd)
        {
            /* no more */
            goto Exit;
        }
        else
        {
            /* skip '\n' */
            pLineBegin = pLineEnd + 1;
        }
    }

Exit:
    cgi_response_end(http_connection);
}

/* ----------------------------------------------------------------------
 * cgi_file_sendbin: Send binary file to socket.
 * ---------------------------------------------------------------------- */

void cgi_file_sendbin(HTTP_Connection_T *http_connection, int sock, envcfg_t *envcfg, envcfg_t *envQuery, UI8_T *pData, CGI_SIZE_T nSize)
{
    cgi_SendHeader(http_connection, sock, nSize, envcfg);
    cgi_SendBin(http_connection, sock, nSize, pData);
    cgi_response_end(http_connection);
}

/* ----------------------------------------------------------------------
 * CGI_FILE_GetFileExt: Get file extension.
 * ---------------------------------------------------------------------- */

const char *
CGI_FILE_GetFileExt(const char *path)
{
    return strrchr(path, '.');
}

/*------------------------------------------------------------------------------
 * Routine Name : CGI_FILE_GetDefaultPage
 *------------------------------------------------------------------------------
 * Function : Get default page
 * Input    : None
 * Output   : None
 * Return   : Default page file path object; NULL
 * Note     : None
 *------------------------------------------------------------------------------
 */
static CGI_FILE_PATH_T *
CGI_FILE_GetDefaultPage();

static CGI_FILE_TYPE_T
CGI_FILE_GetFileType(const char *path)
{
    const char *ext = CGI_FILE_GetFileExt(path);
    UI32_T i;

    if (NULL != ext)
    {
        for (i = 0; i < _countof(cgi_file_bin_ext_list); ++i)
        {
            if (strcmp(ext, cgi_file_bin_ext_list[i]) == 0)
            {
                return CGI_BIN_FILE;
            }
        }
    }

    return CGI_TEXT_FILE;
}

#if CGI_FILE_DEBUG
static void
CGI_FILE_DumpFileSummary(
    const char *file_path,
    CGI_FILE_INFO_T *file_info)
{
    ASSERT(NULL != file_path);
    ASSERT(NULL != file_info);

    printf("filePath:\"%s\", type:\"%s file\", size:\"%lu\", refCount:\"%lu\"\n",
        file_path,
        CGI_TEXT_FILE == file_info->type ? "text" : "bin",
        file_info->co.size,
        file_info->co.ref_count);
}

static void
CGI_FILE_HexDump(
    UI8_T *buffer,
    UI32_T index,
    UI32_T width)
{
    UI32_T i;
    UI32_T spacer;

    for (i=0; i<index; i++)
    {
        printf("%02x ",buffer[i]);
    }

    for (spacer=index; spacer<width; spacer++)
    {
        printf("	");
    }

    printf(": ");

    for (i=0; i<index; i++)
    {
        if (buffer[i] < 32) printf(".");
        else printf("%c",buffer[i]);
    }

    printf("\n");
}

static void
CGI_FILE_DumpFileContext(
     CGI_FILE_INFO_T *file_info)
{
    ASSERT(NULL != file_info);

    if (CGI_TEXT_FILE == file_info->type)
    {
        UI32_T i;

        for (i=0; i < file_info->co.size; ++i)
        {
            printf("%c", file_info->co.ptr[i]);
        }
    }
    else
    {
        UI32_T start = 0;
        UI32_T bytes = 0;
        UI32_T size = file_info->co.size;

        while (0 < size)
        {
            if (16 < size)
            {
                bytes = 16;
            }
            else
            {
                bytes = size;
            }

            size -= bytes;

            CGI_FILE_HexDump((UI8_T *) &file_info->co.ptr[start], bytes, 1);

            start += bytes;
        }
    }
}

static void
CGI_FILE_DumpFileInfo(
    const char *file_path,
    CGI_FILE_INFO_T *file_info)
{
    ASSERT(NULL != file_path);
    ASSERT(NULL != file_info);

    CGI_FILE_DumpFileSummary(file_path, file_info);
    CGI_FILE_DumpFileContext(file_info);
}
#endif /* CGI_FILE_DEBUG */

/*------------------------------------------------------------------------------
 * Routine Name : CGI_FILE_GetFileStats
 *------------------------------------------------------------------------------
 * Function : Get file stats
 * Input    : path - File path
 * Output   : s    - File stats
 * Return   : TRUE/ FALSE
 * Note     : None
 *------------------------------------------------------------------------------
 */
static BOOL_T
CGI_FILE_GetFileStats(const char *path, CGI_FILE_STATS_T *s)
{
    struct stat sb;

    if (NULL == path || NULL == s)
    {
        return FALSE;
    }

    if (stat(path, &sb) != 0)
    {
        return FALSE;
    }

    if (!FS_ISREG(&sb))
    {
        return FALSE;
    }

    s->mtime = FS_MTIME(&sb);

    return TRUE;
}

/*------------------------------------------------------------------------------
 * Routine Name : CGI_FILE_Read
 *------------------------------------------------------------------------------
 * Function : Read file
 * Input    : None
 * Output   : None
 * Return   : TRUE/FALSE
 * Note     : None
 *------------------------------------------------------------------------------
 */
static CGI_FILE_INFO_T *
CGI_FILE_Read(const char *path)
{
    CGI_FILE_INFO_T *file;
    UI32_T fsize;
    UI32_T read_size;

    if (strcmp(path, "/") == 0)
    {
        CGI_FILE_PATH_T *dflt_page = CGI_FILE_GetDefaultPage();

        if (NULL != dflt_page)
        {
            path = dflt_page->co.ptr;
        }
    }

    if (FS_RETURN_OK != FS_GetWebFileSize(path, &fsize))
    {
        CGI_DEBUG_LOG("Get %s failed", path);
        return NULL;
    }

    file = (CGI_FILE_INFO_T *) calloc(sizeof(*file), 1);
    if (NULL == file)
    {
        return NULL;
    }
    MM_alloc(file, sizeof(*file), SYSFUN_TaskIdSelf(), __FILE__, __LINE__, __builtin_return_address( 0 ));

    file->co.ptr = (char *) malloc(fsize);

    if (NULL == file->co.ptr)
    {
        MM_free(file, SYSFUN_TaskIdSelf());
        free(file);
        return NULL;
    }
    MM_alloc(file->co.ptr, sizeof(fsize), SYSFUN_TaskIdSelf(), __FILE__, __LINE__, __builtin_return_address( 0 ));

    file->co.size = fsize;
    file->co.ref_count = 1;

    if (FS_RETURN_OK != FS_ReadWebFile(path, (UI8_T *) file->co.ptr, fsize, &read_size))
    {
        MM_free(file->co.ptr, SYSFUN_TaskIdSelf());
        free(file->co.ptr);

        MM_free(file, SYSFUN_TaskIdSelf());
        free(file);
        return NULL;
    }

    file->type = CGI_FILE_GetFileType(path);


    CGI_FILE_GetFileStats(path, &file->stats);

    return file;
}

/*------------------------------------------------------------------------------
 * Routine Name : CGI_FILE_Send
 *------------------------------------------------------------------------------
 * Function : Send file
 * Input    : None
 * Output   : None
 * Return   : TRUE/FALSE
 * Note     : None
 *------------------------------------------------------------------------------
 */
static void
CGI_FILE_Send(
    HTTP_Connection_T *http_connection,
    int sock,
    int auth,
    envcfg_t *envcfg,
    envcfg_t *envQuery,
    const char *file_path,
    CGI_FILE_INFO_T *file_info)
{
    ASSERT(NULL != file_path);
    ASSERT(NULL != file_info);

#if CGI_FILE_DEBUG
    CGI_FILE_DumpFileInfo(file_path, file_info);
#else
    if (file_info->type == CGI_TEXT_FILE)
    {
        cgi_file_sendtext (http_connection, sock, envcfg, envQuery, file_info->co.ptr, file_info->co.size);
    }
    else
    {
        cgi_file_sendbin (http_connection, sock, envcfg, envQuery, (UI8_T *) file_info->co.ptr, file_info->co.size);
    }

#endif /* CGI_FILE_DEBUG */
}

/*------------------------------------------------------------------------------
 * Routine Name : CGI_FILE_CacheFileDataFree
 *------------------------------------------------------------------------------
 * Function : Free file data object
 * Input    : file_info -   file data object
 * Output   : None
 * Return   : None
 * Note     : None
 *------------------------------------------------------------------------------
 */
static void
CGI_FILE_CacheFileDataFree(
    CGI_FILE_INFO_T *file_info)
{
    CGI_CACHE_MfreeFileData(file_info);
}

/*------------------------------------------------------------------------------
 * Routine Name : CGI_FILE_GetDefaultPage
 *------------------------------------------------------------------------------
 * Function : Get default page
 * Input    : None
 * Output   : None
 * Return   : Default page file path object; NULL
 * Note     : None
 *------------------------------------------------------------------------------
 */
static CGI_FILE_PATH_T *
CGI_FILE_GetDefaultPage()
{
    if (NULL == cgi_file_default_page)
    {
        CGI_FILE_INFO_T *dflt_page = CGI_FILE_Read( CGI_FILE_DEFAULT_PAGE_CFG_FILE );

        if (NULL != dflt_page)
        {
            cgi_file_default_page = (CGI_FILE_PATH_T *) malloc(sizeof(*cgi_file_default_page));

            if (NULL != cgi_file_default_page)
            {
                MM_alloc(cgi_file_default_page, sizeof(*cgi_file_default_page), SYSFUN_TaskIdSelf(), __FILE__, __LINE__, __builtin_return_address( 0 ));

                cgi_file_default_page->co.ptr = (char *) malloc(dflt_page->co.size + 1);
                cgi_file_default_page->co.size = dflt_page->co.size + 1;

                MM_alloc(cgi_file_default_page->co.ptr, dflt_page->co.size + 1, SYSFUN_TaskIdSelf(), __FILE__, __LINE__, __builtin_return_address( 0 ));

                if (NULL != cgi_file_default_page->co.ptr)
                {
                    memcpy(cgi_file_default_page->co.ptr, dflt_page->co.ptr, dflt_page->co.size);
                    cgi_file_default_page->co.ptr[dflt_page->co.size] = '\0';
                    cgi_file_default_page->co.ref_count = 1;
                }
                else
                {
                    MM_free(cgi_file_default_page, SYSFUN_TaskIdSelf());
                    free(cgi_file_default_page);
                    cgi_file_default_page = NULL;
                }
            }

            CGI_FILE_CacheFileDataFree(dflt_page);
        }
    }

    return cgi_file_default_page;
}

/*------------------------------------------------------------------------------
 * Routine Name : CGI_FILE_CacheCreate
 *------------------------------------------------------------------------------
 * Function : Create a file cache
 * Input    : None
 * Output   : None
 * Return   : CGI_OK if succeeded; Error code if failed
 * Note     : None
 *------------------------------------------------------------------------------
 */
static CGI_RETVAL_T
CGI_FILE_CacheCreate()
{
    CGI_FILE_ENTER_CRITICAL_SECTION();

    if (NULL != cgi_file_cache)
    {
        CGI_FILE_RETURN_AND_LEAVE_CRITICAL_SECTION(CGI_OK);
    }

    cgi_file_cache = (CGI_CACHE_T *) malloc(sizeof(*cgi_file_cache));
    if (NULL == cgi_file_cache)
    {
        CGI_DEBUG_LOG("Out of memory");
        CGI_FILE_RETURN_AND_LEAVE_CRITICAL_SECTION(CGI_FAIL);
    }

    MM_alloc(cgi_file_cache, sizeof(*cgi_file_cache), SYSFUN_TaskIdSelf(), __FILE__, __LINE__, __builtin_return_address( 0 ));

    if (FALSE == CGI_CACHE_Create(cgi_file_cache, CGI_FILE_Read, CGI_FILE_GetFileStats, CGI_FILE_MAX_CACHE_SIZE))
    {
        CGI_DEBUG_LOG("Failed to create cache");
        CGI_FILE_RETURN_AND_LEAVE_CRITICAL_SECTION(CGI_FAIL);
    }

    CGI_FILE_RETURN_AND_LEAVE_CRITICAL_SECTION(CGI_OK);
}

/*------------------------------------------------------------------------------
 * Routine Name : CGI_FILE_CacheLookup
 *------------------------------------------------------------------------------
 * Function : Lookup a file
 * Input    : path  -   file path
 *            ent   -   file
 * Output   : None
 * Return   : CGI_OK if succeeded; Error code if failed
 * Note     : None
 *------------------------------------------------------------------------------
 */
static CGI_RETVAL_T
CGI_FILE_CacheLookup(
    const char *path,
    CGI_CACHE_Entry_T *ent)
{
    CGI_FILE_ENTER_CRITICAL_SECTION();

    if (NULL == cgi_file_cache)
    {
        CGI_DEBUG_LOG("Cache was not exist");
        CGI_FILE_RETURN_AND_LEAVE_CRITICAL_SECTION(CGI_FAIL);
    }

    if (FALSE == CGI_CACHE_Lookup(cgi_file_cache, path, ent))
    {
        CGI_FILE_RETURN_AND_LEAVE_CRITICAL_SECTION(CGI_NOT_FOUND);
    }

    CGI_FILE_RETURN_AND_LEAVE_CRITICAL_SECTION(CGI_OK);
}

/*------------------------------------------------------------------------------
 * Routine Name : CGI_FILE_CacheClear
 *------------------------------------------------------------------------------
 * Function : Clear all entries in cache
 * Input    : None
 * Output   : None
 * Return   : None
 * Note     : None
 *------------------------------------------------------------------------------
 */
void
CGI_FILE_CacheClear()
{
    CGI_FILE_ENTER_CRITICAL_SECTION();

    if (NULL == cgi_file_cache)
    {
        CGI_FILE_RETURN_AND_LEAVE_CRITICAL_SECTION_WITHOUT_RETUEN_VALUE();
    }

    CGI_CACHE_Destroy(cgi_file_cache);

    MM_free(cgi_file_cache, SYSFUN_TaskIdSelf());
    free(cgi_file_cache);
    cgi_file_cache = NULL;

    CGI_FILE_RETURN_AND_LEAVE_CRITICAL_SECTION_WITHOUT_RETUEN_VALUE();
}

/*------------------------------------------------------------------------------
 * Routine Name : CGI_FILE_GetCacheInfo
 *------------------------------------------------------------------------------
 * Function : Get file cache stat. information
 * Input    : None
 * Output   : info  - stat. information
 * Return   : CGI_OK; Error code
 * Note     : None
 *------------------------------------------------------------------------------
 */
int
CGI_FILE_GetCacheInfo(
    CGI_CACHE_INFO_T *info)
{
    ASSERT(NULL != info);

    CGI_FILE_ENTER_CRITICAL_SECTION();

    if (NULL == cgi_file_cache)
    {
        CGI_DEBUG_LOG("Cache was not exist");
        CGI_FILE_RETURN_AND_LEAVE_CRITICAL_SECTION(CGI_FAIL);
    }

    info->capacity_in_bytes = cgi_file_cache->capacity;
    info->size_in_bytes = cgi_file_cache->size;
    info->hit = cgi_file_cache->hit;
    info->miss = cgi_file_cache->miss;
    info->access = info->hit + info->miss;

    CGI_FILE_RETURN_AND_LEAVE_CRITICAL_SECTION(CGI_OK);
}

/*------------------------------------------------------------------------------
 * Routine Name : CGI_FILE_Main
 *------------------------------------------------------------------------------
 * Function : Read html(gif, xml, ...) file from flash disk
 * Input    : sock      -   Socket identifier
 *            auth      -   Authorisation level
 *            envcfg    -   Environment configuration
 *            envQuery  -   Query parameters.
 * Output   : None
 * Return   :  1: Have sent screen.
 *             0: Have not sent screen.
 *            -1: Error.
 * Note     : None
 *------------------------------------------------------------------------------
 */
int
CGI_FILE_Main(
    HTTP_Connection_T *http_connection,
    int sock,
    int auth,
    envcfg_t *envcfg,
    envcfg_t *envQuery)
{
    const char *path = get_env(envcfg, "PATH_INFO");
    CGI_CACHE_Entry_T ent;

    if (path == NULL)
    {
        cgi_response_end(http_connection);
        return CGI_FAIL;
    }

    CGI_DEBUG_LOG("Request %s", path);

    CGI_FILE_CacheCreate();

    if (CGI_OK != CGI_FILE_CacheLookup(path, &ent))
    {
        CGI_DEBUG_LOG("File %s not found", path);
        return CGI_NOT_FOUND;
    }

    CGI_FILE_Send(http_connection, sock, auth, envcfg, envQuery, path, ent.file_info);

    CGI_FILE_CacheFileDataFree(ent.file_info);

    cgi_response_end(http_connection);
    return CGI_OK;
}

