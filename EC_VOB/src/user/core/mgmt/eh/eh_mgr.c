/* MODULE NAME:  eh_mgr.c
 * PURPOSE:
 *    EH is a short name for "Error Handler".  
 *    EH provides processing request for display of clear and meaningful error
 *    message due to failure of an operation executed by UI.
 *
 *    EH supports to output error message in different languages and encoding. 
 *
 * Notes: 
 *      1. An system-wised EH message buffer is allocated and binded to a UI
 *         thread when a UI thread is spawned. 
 *      2. Each UI thread can hold one and only one UI error message in the
 *         system-wised EH message buffer. Note that the UI error message will 
 *         be cleared after processing the request for display error message.
 *     	3. Scenario of usage.
 *         (1)in caller (executed in UI thread context)  :
 *            call service function...
 *            if return an error status
 *            then
 *               EH_MGR_Process_Error_Info  (...);
 *         (2)in service routine :
 *            if there is an error can't continuously processing...
 *               EH_MGR_Handle_Exception (...);
 *               return (error_status);
 *
 * History:                                                               
 *        Date          -- Modifier,    Reason
 *        2013.2.5      -- Charlie Chen Create.
 * Copyright(C)      Edge-Core Networks, 2013
 */

#if 0 /* turn on for doing EH unit test, also need to turn on EH_UNIT_TEST in utility_group.c */
#define EH_UNIT_TEST 1
#endif

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>

/* libxml2 header files -- START */
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
/* libxml2 header files -- END   */
#include <iconv.h> /* iconv header file */

#include "sys_type.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "l_mm.h"

#include "eh_type.h"
#include "eh_mgr.h"
#include "eh_om.h"

#include "backdoor_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* absolute path name and file name of the error message file
 */
#define EH_MGR_ERR_MSG_FILENAME "/usr/eh/eh_msg.xml"

/* constants for namespace of the error message file
 */
#define EH_MGR_ERR_MSG_FILE_NS_PREFIX "eh"
#define EH_MGR_ERR_MSG_FILE_NS_HREF   "http://www.edgecore.com/eh"

/* xpath expression format string for locating the message
 */
#define EH_MGR_LOCATE_MSG_BY_UI_MSG_NO_XPATH_EXPR_FORMAT_STR "/eh:EH/eh:message[eh:UI_Msg_No=%lu]/eh:string/eh:%s/text()"

/* buffer length for resulting xpath expression string
 */
#define EH_MGR_LOCATE_MSG_BY_UI_MSG_NO_XPATH_STR_BUF_LEN    68

/* In function EH_MGR_GenerateMsg(), the encoding of the input message format
 */
#define EH_MGR_GENERATE_MSG_INPUT_ENCODING        EH_TYPE_ENCODING_UTF_8

/* In function EH_MGR_GenerateMsg(), the encoding of the intermediate encoding
 * type in order to generate the result message from the printf-style format
 * string
 */
#define EH_MGR_GENERATE_MSG_INTERMEDIATE_ENCODING EH_TYPE_ENCODING_WCHAR_T

/* default size for XML message buffer
 */
#define EH_MGR_DEFAULT_XML_BUFFER_CREATE_SIZE 256

/* MACRO FUNCTION DECLARATIONS
 */
#define EH_MGR_EnterCriticalSection() SYSFUN_TakeSem(eh_mgr_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define EH_MGR_LeaveCriticalSection() SYSFUN_GiveSem(eh_mgr_sem_id)


/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static EH_TYPE_RET_T EH_MGR_GetMsgFormat( UI8_T eh_buf_idx,
    EH_OM_ErrorMsgEntry_T* entry_p, EH_TYPE_LANGUAGE_T lang,
    const char** msg_format_pp);

static EH_TYPE_RET_T EH_MGR_GenerateMsg(const EH_OM_ErrorMsgEntry_T* entry_p,
    const char* msg_format_p, EH_TYPE_ENCODING_T encoding_for_outmsg,
    char **msg_buf_pp);

static const char* EH_MGR_GetLangTageName(EH_TYPE_LANGUAGE_T lang);
static const char* EH_MGR_GetIconvEncodeName(EH_TYPE_ENCODING_T encoding);

static void EH_MGR_BACKDOOR_PrintXPathNodes(xmlNodeSetPtr nodes);
static BOOL_T EH_MGR_BACKDOOR_ExecuteXPathExpression(const char* filename,
    const char* xpathExpr, const char* ns_prefix_p, const char* ns_href_p);

/* STATIC VARIABLE DECLARATIONS
 */
static UI32_T eh_mgr_sem_id;
static BOOL_T isLibXmlInit=FALSE;
static xmlDocPtr xml_doc_p=NULL;

/* EXPORTED SUBPROGRAM BODIES
 */
/*---------------------------------------------------------------------------------
 * FUNCTION NAME: EH_MGR_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource in the context of the calling process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   None
 *---------------------------------------------------------------------------------*/
void EH_MGR_AttachSystemResources(void)
{
    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_EH, &eh_mgr_sem_id)
        != SYSFUN_OK)
    {
        printf("%s: get sem id fail.\r\n", __FUNCTION__);
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - EH_MGR_Process_Error_Info 
 *-------------------------------------------------------------------------
 * PURPOSE  : Process the request to output error message in specified language
 *            and encoding. 
 * INPUT    : cookie        -- A cookie that will be passed as an argument when
 *                             the specified function pointer output_func_p is
 *                             called.   
 *            output_fun_p  -- A function pointer that will be called with
 *                             cookie and the error message to be output.
 *                             In usual, output_fun_p is responsible for display
 *                             the error message on the appropriate UI.
 *            lang          -- The language of the output error message.
 *            encoding      -- The character encoding of the output error
 *                             message.
 * OUTPUT   : None
 * RETURN   : EH_TYPE_RET_OK                  --
 *                An error message is got from EH message buffer which is binded
 *                to the caller UI thread. The error message in this EH message
 *                buffer will be cleared after returning from this function. 
 *            EH_TYPE_RET_NO_ERR_MSG          --
 *                No error message is found in EH message buffer which is binded
 *                to the caller UI thread.
 *            EH_TYPE_RET_NO_BOUND_EH_MSG_BUF --
 *                Cannot locate the EH message buffer which is bound to the
 *                calling thread.
 *            EH_TYPE_RET_INVALID_INPUT_ARG --
 *                An error occurred due to invalid one or more input arguments.
 *            EH_TYPE_RET_NOT_SUPPORT       --
 *                The requested operation is not supported.
 *            EH_TYPE_RET_INVALID_UI_MSG_NO --
 *                The specified UI message number is invalid.
 *            EH_TYPE_RET_GENERATE_MSG_ERROR--
 *                An error occurred when generated the error message.
 * NOTE     : 1. This function shall only be invoked by CLI and WEB
 *            2. Current support language:
 *               EH_TYPE_LANGUAGE_EN_EN
 *            3. Current support encoding:
 *               EH_TYPE_ENCODING_UTF_8
 *            4. The EH message buffer which is binded to the caller UI thread
 *               will be cleared no matter the return status is successful or
 *               failure.
 *-------------------------------------------------------------------------*/
EH_TYPE_RET_T EH_MGR_Process_Error_Info(void* cookie,
    EH_TYPE_OutputMessageFunction_T output_func_p,
    EH_TYPE_LANGUAGE_T lang,
    EH_TYPE_ENCODING_T encoding)
{
    const char *msg_format_p=NULL;
    char *msg_buf_p=NULL;
    EH_OM_ErrorMsgEntry_T entry;
    EH_TYPE_OutputMessage_T output_msg;
    EH_TYPE_RET_T rc, ret=EH_TYPE_RET_OK;
    UI8_T eh_buf_idx=SYSFUN_TYPE_INVALID_EH_BUFFER_INDEX;

    /* Call SYSFUN API to get EH Msg Buf Idx
     */
    if (SYSFUN_GetEHBufIndex(&eh_buf_idx) != SYSFUN_OK)
    {
        printf("%s:Failed to get eh_buf_idx.\r\n", __FUNCTION__);
        return EH_TYPE_RET_NO_BOUND_EH_MSG_BUF;
    }

    /* sanity check
     */
    if (output_func_p==NULL)
    {
        printf("%s:invalid output_func_p.\r\n", __FUNCTION__);
        ret = EH_TYPE_RET_INVALID_INPUT_ARG;
        goto exit;
    }

    if (lang >= EH_TYPE_LANGUAGE_MAX_NUM)
    {
        printf("%s:Invalid language(%d).\r\n",
            __FUNCTION__, (int)lang);
        ret = EH_TYPE_RET_INVALID_INPUT_ARG;
        goto exit;
    }
    else if (lang > EH_TYPE_LANGUAGE_EN_EN)
    {
        printf("%s:Not support yet for language(%d).\r\n",
            __FUNCTION__, (int)lang);
        ret = EH_TYPE_RET_NOT_SUPPORT;
        goto exit;
    }

    if (encoding>=EH_TYPE_ENCODING_MAX_NUM)
    {
        printf("%s:Invalid encoding(%d).\r\n",
            __FUNCTION__, (int)encoding);
        ret = EH_TYPE_RET_INVALID_INPUT_ARG;
        goto exit;
    }

    if (encoding!=EH_TYPE_ENCODING_UTF_8)
    {
        printf("%s:Not support yet for encoding(%d).\r\n",
            __FUNCTION__, (int)encoding);
        ret = EH_TYPE_RET_NOT_SUPPORT;
        goto exit;
    }

    /* Get EH message entry from OM
     */
    rc = EH_OM_GetErrorMsgEntry(eh_buf_idx, &entry);
    if (rc != EH_TYPE_RET_OK)
    {
        printf("%s(%d) EH_OM_GetErrorMsgEntry error, rc=%d\r\n", __FUNCTION__,
            __LINE__, (int)rc);
        ret = EH_TYPE_RET_NO_ERR_MSG;
        goto exit;
    }

    /* get the message format by UI Message Number
     */
    rc = EH_MGR_GetMsgFormat(eh_buf_idx, &entry, lang, &msg_format_p);
    if (rc != EH_TYPE_RET_OK)
    {
        printf("%s(%d) EH_MGR_GetMsgFormat error, rc=%d\r\n", __FUNCTION__,
            __LINE__, (int)rc);
        ret = EH_TYPE_RET_INVALID_UI_MSG_NO;
        goto exit;
    }

    /* Generate output message
     */
    rc = EH_MGR_GenerateMsg(&entry, msg_format_p, encoding, &msg_buf_p);
    if (rc != EH_TYPE_RET_OK)
    {
        printf("%s(%d) EH_MGR_GenerateMsg error, rc=%d\r\n", __FUNCTION__,
            __LINE__, (int)rc);
        ret = EH_TYPE_RET_GENERATE_MSG_ERROR;
        goto exit;
    }

    output_msg.msg_p = msg_buf_p;
    output_msg.encoding = encoding;

    /* Call output_func_p with cookie and output message
     */
    (*output_func_p)(cookie, &output_msg);
exit:
    /* Clear error message entry in OM
     */
    if ((eh_buf_idx != SYSFUN_TYPE_INVALID_EH_BUFFER_INDEX) &&
        (EH_OM_ClearErrorMsgEntry(eh_buf_idx) != EH_TYPE_RET_OK))
    {
        printf("%s(%d): Failed to clear error message in OM.\r\n",
            __FUNCTION__,__LINE__);
    }

    /* free message buffer
     */
    if (msg_buf_p)
        L_MM_Free(msg_buf_p);

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - EH_MGR_Handle_Error 
 *-------------------------------------------------------------------------
 * PURPOSE  : When an error occurs in the execution of the operation, it is
 *            able to call this function to set a clear and specific error
 *            message to OM for the reason of failure. This error message will
 *            be displayed on UI by a UI thread finally.
 * INPUT    : module_id           -- the module id of the caller.
 *            function_no         -- the caller's function no, defined by each
 *                                   module itself.
 *            ui_msg_no           -- Refer to "UI Message ID" in header comment
 *                                   of eh_type.h
 *            msg_flag            -- Not used now. Reserved for future
 *                                   development. Set as 0 now.
 *            num_of_arg          -- Number of element in array args.
 *            args                -- Array of arguments for printf like format
 *                                   string. Can be NULL if num_of_arg is 0.  
 * OUTPUT   : None
 * RETURN   :
 *  EH_TYPE_RET_OK                   --  The error info is set to EH successfully.
 *  EH_TYPE_RET_INVALID_INPUT_ARG    --  Invalid input argument.
 *  EH_TYPE_RET_NO_BOUND_EH_MSG_BUF  --  Cannot locate the EH message buffer
 *                                       bound to the caller thread.
 *  EH_TYPE_RET_SET_OM_FAILED        --  Failed to set OM.
 * NOTE     : The maximum number of argument allowed for the API is
 *            EH_TYPE_MAX_NUMBER_OF_ARG_FOR_UI_MSG. The requirement for
 *            increasing EH_TYPE_MAX_NUMBER_OF_ARG_FOR_UI_MSG needs to be
 *            reviewed by SA.
 *-------------------------------------------------------------------------*/
EH_TYPE_RET_T EH_MGR_Handle_Error(UI32_T module_id, UI32_T function_no,
    EH_TYPE_UIMsgNumber_T ui_msg_no, UI32_T msg_flag, UI32_T num_of_arg,
    UI32_T args[])
{
    EH_OM_ErrorMsgEntry_T entry;
    UI8_T eh_buf_idx;

    /* sanity check
     */
    if (num_of_arg > EH_TYPE_MAX_NUMBER_OF_ARG_FOR_UI_MSG)
    {
        printf("%s: Illegal num_of_arg(%lu)\r\n", __FUNCTION__, num_of_arg);
        return EH_TYPE_RET_INVALID_INPUT_ARG;
    }
    if ((num_of_arg!=0) && (args == NULL))
    {
        printf("%s: Illegal argument args\r\n", __FUNCTION__);
        return EH_TYPE_RET_INVALID_INPUT_ARG;
    }

    /* Get EH buf index bound to the caller thread
     */
    if (SYSFUN_GetEHBufIndex(&eh_buf_idx) != SYSFUN_OK)
    {
        return EH_TYPE_RET_NO_BOUND_EH_MSG_BUF;
    }

    /* Set the error info to OM with the EH buf index
     */
    memset(&entry, 0, sizeof(entry));
    entry.module_id = (UI8_T)module_id;
    entry.msg_flag = (UI8_T)msg_flag;
    entry.function_no = (UI8_T)function_no;
    entry.num_of_arg = (UI8_T)num_of_arg;
    if (num_of_arg!=0)
        memcpy(&entry.args, args, sizeof(UI32_T)*num_of_arg);

    entry.ui_msg_no = ui_msg_no;

    if (EH_OM_SetErrorMsgEntry(eh_buf_idx, &entry) != EH_TYPE_RET_OK)
        return EH_TYPE_RET_SET_OM_FAILED;

    return EH_TYPE_RET_OK;
}

/* LOCAL SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - EH_MGR_GetMsgFormat 
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the format string of the error message according to UI Message
 *            Number.
 * INPUT    : eh_buf_idx          -- Error message buffer index of the calling
 *                                   UI thread.
 *            entry_p             -- Error message info for the calling UI
 *                                   thread.
 *            lang                -- language of the output message.
 * OUTPUT   : msg_format_pp       -- the format string of the error message in
 *                                   UTF-8 encoding.
 * RETURN   :
 *  EH_TYPE_RET_OK                   --  Get message format string succesfully.
 *  EH_TYPE_RET_INVALID_UI_MSG_NO    --  The specified UI message number is invalid.
 *  EH_TYPE_RET_INVALID_INPUT_ARG    --  Invalid input argument.
 *  EH_TYPE_RET_INTERNAL_DATA_ERROR  --  Parse xml file error.
 *
 * NOTE     :
 *  1. entry_p->xml_msg_buf_p will be initialized if its value is
 *     EH_TYPE_INVALID_XML_BUFFER_PTR. The updated entry will be set to
 *     EH_OM.
 *-------------------------------------------------------------------------*/
static EH_TYPE_RET_T EH_MGR_GetMsgFormat( UI8_T eh_buf_idx,
    EH_OM_ErrorMsgEntry_T* entry_p, EH_TYPE_LANGUAGE_T lang,
    const char** msg_format_pp)
{
    EH_TYPE_RET_T ret = EH_TYPE_RET_OK, rc;
    xmlBufferPtr xml_buf_p=NULL;
    xmlXPathContextPtr xpathCtx=NULL;
    xmlXPathObjectPtr xpathObj=NULL;
    const char* lang_tag_name_p=NULL;
    char xpath_str_buf[EH_MGR_LOCATE_MSG_BY_UI_MSG_NO_XPATH_STR_BUF_LEN];

    /* Do init for libxml if it has not been initialized yet
     */
    EH_MGR_EnterCriticalSection();
    if (isLibXmlInit==FALSE)
    {
        isLibXmlInit=TRUE;
        xmlInitParser();
        LIBXML_TEST_VERSION
        xml_doc_p = xmlParseFile(EH_MGR_ERR_MSG_FILENAME);
        if (xml_doc_p == NULL)
        {
            printf("%s(%d) Failed to parse file %s\r\n", __FUNCTION__, __LINE__,
                EH_MGR_ERR_MSG_FILENAME);
            EH_MGR_LeaveCriticalSection();
            return EH_TYPE_RET_INTERNAL_DATA_ERROR;
        }        
    }
    EH_MGR_LeaveCriticalSection();

    if (xml_doc_p == NULL)
    {
        printf("%s(%d) Failed to parse file %s\r\n", __FUNCTION__, __LINE__,
            EH_MGR_ERR_MSG_FILENAME);
        return EH_TYPE_RET_INTERNAL_DATA_ERROR;
    }

    /* init xml_buf_p
     */
    xml_buf_p = (xmlBufferPtr) EH_OM_GetXmlBufPtr(eh_buf_idx);
    if (xml_buf_p == NULL)
    {
        xml_buf_p = xmlBufferCreateSize(EH_MGR_DEFAULT_XML_BUFFER_CREATE_SIZE);
        if (xml_buf_p == NULL)
        {
            printf("%s(%d) Failed to create xml buffer.\r\n", __FUNCTION__, __LINE__);
            return EH_TYPE_RET_OUT_OF_MEMORY;
        }
        rc = EH_OM_SetXmlBufPtr(eh_buf_idx, xml_buf_p);
        if (rc != EH_TYPE_RET_OK)
        {
            xmlBufferFree(xml_buf_p);
            return EH_TYPE_RET_SET_OM_FAILED;
        }

    }

    /* sanity check
     */
    if (msg_format_pp==NULL)
    {
        printf("%s(%d)msg_format_pp is NULL.\r\n", __FUNCTION__, __LINE__);
        return EH_TYPE_RET_INVALID_INPUT_ARG;
    }

    lang_tag_name_p = EH_MGR_GetLangTageName(lang);
    if (lang_tag_name_p == NULL)
    {
        printf("%s(%d)Invalid lang id(%d)\r\n", __FUNCTION__, __LINE__, (int)lang);
        return EH_TYPE_RET_INVALID_INPUT_ARG;
    }

    xpathCtx = xmlXPathNewContext(xml_doc_p);
    if (xpathCtx == NULL)
    {
        printf("%s(%d) Failed to create xpath context.\r\n", __FUNCTION__, __LINE__);
        return EH_TYPE_RET_OUT_OF_MEMORY;
    }

    if (xmlXPathRegisterNs(xpathCtx, BAD_CAST EH_MGR_ERR_MSG_FILE_NS_PREFIX, BAD_CAST EH_MGR_ERR_MSG_FILE_NS_HREF) != 0)
    {
        printf("%s(%d) Failed to parse file %s\r\n", __FUNCTION__, __LINE__,
            EH_MGR_ERR_MSG_FILENAME);
        ret = EH_TYPE_RET_INTERNAL_DATA_ERROR;
        goto exit;
    }

    snprintf(xpath_str_buf, EH_MGR_LOCATE_MSG_BY_UI_MSG_NO_XPATH_STR_BUF_LEN,
        EH_MGR_LOCATE_MSG_BY_UI_MSG_NO_XPATH_EXPR_FORMAT_STR,
        entry_p->ui_msg_no, lang_tag_name_p);
    xpath_str_buf[EH_MGR_LOCATE_MSG_BY_UI_MSG_NO_XPATH_STR_BUF_LEN-1] = '\0';

    xpathObj = xmlXPathEvalExpression(BAD_CAST xpath_str_buf, xpathCtx);
    if ((xpathObj == NULL) || (xpathObj->nodesetval == NULL) || (xpathObj->nodesetval->nodeNr==0))
    {
        printf("%s(%d)Cannot find the message of the spcified ui msg no(%lu) and lang(%d)\r\n",
            __FUNCTION__, __LINE__, (UI32_T)(entry_p->ui_msg_no), (int)lang);
        ret = EH_TYPE_RET_INTERNAL_DATA_ERROR;
        goto exit;
    }

    if (xpathObj->nodesetval->nodeTab[0]->type != XML_TEXT_NODE)
    {
        printf("%s(%d)Cannot find the message of the spcified ui msg no(%lu) and lang(%d). XML file incorrect content\r\n",
            __FUNCTION__, __LINE__, (UI32_T)(entry_p->ui_msg_no), (int)lang);
        ret = EH_TYPE_RET_INTERNAL_DATA_ERROR;
        goto exit;
    }

    xmlBufferEmpty(xml_buf_p);
    if (xmlNodeBufGetContent(xml_buf_p, xpathObj->nodesetval->nodeTab[0])!=0)
    {
        printf("%s(%d)xmlNodeBufGetContent error for message of the spcified ui msg no(%lu) and lang(%d)\r\n",
            __FUNCTION__, __LINE__, (UI32_T)(entry_p->ui_msg_no), (int)lang);
        ret = EH_TYPE_RET_INTERNAL_DATA_ERROR;
        goto exit;
    }

    *msg_format_pp = (const char*)xmlBufferContent(xml_buf_p);
exit:
    if (xpathObj!=NULL)
    {
        xmlXPathFreeObject(xpathObj);
    }
    if (xpathCtx!=NULL)
    {
        xmlXPathFreeContext(xpathCtx);
    }
    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - EH_MGR_GenerateMsg 
 *-------------------------------------------------------------------------
 * PURPOSE  : Generate the output error message.
 * INPUT    : entry_p             -- error message info for the calling UI
 *                                   thread
 *            msg_format_p        -- format string of the error message in UTF-8
 *                                   encoding.
 *            encoding_for_outmsg -- encoding for output message(i.e. msg_buf_pp)
 * OUTPUT   : msg_buf_pp          -- the output error message in encoding
 *                                   specified in encoding_for_outmsg
 * RETURN   :
 *  EH_TYPE_RET_OK                   --  Generate the error message successfully.
 *  EH_TYPE_RET_INVALID_INPUT_ARG    --  Invalid input argument.
 *  EH_TYPE_RET_GENERATE_MSG_ERROR   --  An error occurs when generating the
 *                                       output message.
 *  EH_TYPE_RET_OUT_OF_MEMORY        --  Error caused by out of memory.
 *
 * NOTE     : 1. Caller must call L_MM_Free against msg_buf_pp when this
 *               function returns EH_TYPE_RET_OK and msg_buf_pp will not be
 *               referenced any more.
 *            2. Need to take care of evaluation of required buffer length
 *               if "%s" or "%ls" is supported.
 *
 *-------------------------------------------------------------------------*/
static EH_TYPE_RET_T EH_MGR_GenerateMsg(const EH_OM_ErrorMsgEntry_T* entry_p,
    const char* msg_format_p, EH_TYPE_ENCODING_T encoding_for_outmsg,
    char **msg_buf_pp)
{
    int    rc;
    int    required_wchar_fmt_buf_len; /* required length for wchar_fmt_buf_p in unit of wchar_t */
    int    required_wchar_msg_buf_len; /* required length for wchar_msg_buf_p in unit of wchar_t */
    int    output_msg_buf_len;         /* length of output_msg_buf_p in unit of unsigned char */
    wchar_t *wchar_fmt_buf_p=NULL; /* buffer for message format in wchar_t */
    wchar_t *wchar_msg_buf_p=NULL; /* buffer for result message in wchar_t */
    char* output_msg_buf_p=NULL; /* buffer for result message in encoding of encoding_for_outmsg */
    iconv_t iconv_handle = (iconv_t)-1;
    size_t  iconv_rc;
    char *inbuf_p, *outbuf_p;
    size_t inbytesleft, outbytesleft;
    const char* iconv_encoding_name_from_p=NULL;
    const char* iconv_encoding_name_to_p=NULL;
    
    EH_TYPE_RET_T ret = EH_TYPE_RET_OK;

    /* sanity check
     */
    if (msg_format_p == NULL)
    {
        printf("%s(%d) msg_format_p is NULL\r\n", __FUNCTION__, __LINE__);
        return EH_TYPE_RET_INVALID_INPUT_ARG;
    }
    if (msg_buf_pp == NULL)
    {
        printf("%s(%d) msg_buf_pp is NULL\r\n", __FUNCTION__, __LINE__);
        return EH_TYPE_RET_INVALID_INPUT_ARG;
    }
    *msg_buf_pp=NULL;

    if (entry_p == NULL)
    {
        printf("%s(%d) entry_p is NULL\r\n", __FUNCTION__, __LINE__);
        return EH_TYPE_RET_INVALID_INPUT_ARG;
    }

    /* UTF-8 string also terminated with '\0', use strlen() to get occupied byte
     * number in msg_format_p
     * On Linux/Unix system, size of wchar_t are 4 bytes and use UCS4 encoding.
     * a conservative estimation for required buffer size for wchar_fmt_buf_p is
     * 4*(occupied size of msg_format_p)
     */
    required_wchar_fmt_buf_len = 4 * strlen(msg_format_p);
    if (required_wchar_fmt_buf_len==0)
    {
        printf("%s(%d) Empty string in msg_format_p\r\n", __FUNCTION__, __LINE__);
        return EH_TYPE_RET_INVALID_INPUT_ARG;
    }

    /* prepare buffer for wchar_fmt_buf_p
     */
    wchar_fmt_buf_p = (wchar_t*)L_MM_Malloc((UI32_T)(required_wchar_fmt_buf_len*sizeof(wchar_t)),
        L_MM_USER_ID2(SYS_MODULE_EH, EH_TYPE_TRACE_ID_EH_MGR_GENERATE_MSG1));
    if (wchar_fmt_buf_p==NULL)
    {
        printf("%s(%d) L_MM_Malloc failed for size %lu\r\n", __FUNCTION__,
            __LINE__, (UI32_T)(required_wchar_fmt_buf_len*sizeof(wchar_t)));
        return EH_TYPE_RET_OUT_OF_MEMORY;
    }

    iconv_encoding_name_to_p = EH_MGR_GetIconvEncodeName(EH_MGR_GENERATE_MSG_INTERMEDIATE_ENCODING);
    if (iconv_encoding_name_to_p==NULL)
    {
        printf("%s(%d) EH_MGR_GetIconvEncodeName error for encoding enum value %d\r\n",
            __FUNCTION__, __LINE__, (int)encoding_for_outmsg);
        ret = EH_TYPE_RET_CHAR_ENCODING_CONVERT_ERROR;
        goto exit;
    }

    iconv_encoding_name_from_p = EH_MGR_GetIconvEncodeName(EH_MGR_GENERATE_MSG_INPUT_ENCODING);
    if (iconv_encoding_name_to_p==NULL)
    {
        printf("%s(%d) EH_MGR_GetIconvEncodeName error for encoding enum value %d\r\n",
            __FUNCTION__, __LINE__, (int)EH_MGR_GENERATE_MSG_INTERMEDIATE_ENCODING);
        ret = EH_TYPE_RET_CHAR_ENCODING_CONVERT_ERROR;
        goto exit;
    }

    /* convert UTF-8 to wchar_t for msg_format_p
     * the message format string in wchar_t will be put in wchar_fmt_buf_p
     */

    iconv_handle = iconv_open(iconv_encoding_name_to_p,
        iconv_encoding_name_from_p);

    if (iconv_handle == (iconv_t)-1)
    {
        printf("%s(%d) iconv_open error.\r\n", __FUNCTION__,
            __LINE__);
        ret = EH_TYPE_RET_CHAR_ENCODING_CONVERT_ERROR;
        goto exit;
    }

    inbuf_p = (char*)msg_format_p;
    inbytesleft = (size_t)(strlen(msg_format_p));
    outbuf_p = (char*)wchar_fmt_buf_p;
    outbytesleft = (size_t)(required_wchar_fmt_buf_len*sizeof(wchar_t));
    iconv_rc = iconv(iconv_handle, &inbuf_p, &inbytesleft, &outbuf_p,
        &outbytesleft);
    if (iconv_rc == (size_t)(-1))
    {
        printf("%s(%d) iconv error.(errno=%d)\r\n", __FUNCTION__, __LINE__,
            errno);
        ret = EH_TYPE_RET_CHAR_ENCODING_CONVERT_ERROR;
        goto exit;
    }
    iconv_close(iconv_handle);
    iconv_handle = (iconv_t)-1;

    if (outbytesleft<sizeof(wchar_t))
    {
        printf("%s(%d)left buffer size too small(=%d)\r\n", __FUNCTION__, __LINE__,
            (int)outbytesleft);
        ret = EH_TYPE_RET_CHAR_ENCODING_CONVERT_ERROR;
        goto exit;
    }
    /* add NULL terminated char in wchar_fmt_buf_p
     */
    memset(outbuf_p, 0, sizeof(wchar_t));

    /* GLIBC swprintf() cannot return the ouput message length if the output
     * message is truncated. Assumes that each "%d" argument might expand up to
     * 12 digits. Thus the required length in wchar_t is
     * wcslen(msg_format_p) + (num_of_arg*10) + 1
     * "+1" is for NULL terminated wchar_t.
     */
    required_wchar_msg_buf_len = wcslen(wchar_fmt_buf_p) + ((entry_p->num_of_arg)*10) + 1;

    wchar_msg_buf_p = (wchar_t*)L_MM_Malloc((UI32_T)(required_wchar_msg_buf_len*sizeof(wchar_t)),
        L_MM_USER_ID2(SYS_MODULE_EH, EH_TYPE_TRACE_ID_EH_MGR_GENERATE_MSG2));

    if (wchar_msg_buf_p == NULL)
    {
        printf("%s(%d)L_MM_Malloc error for size %lu\r\n", __FUNCTION__, __LINE__,
            (UI32_T)(required_wchar_msg_buf_len*sizeof(wchar_t)));
        ret = EH_TYPE_RET_OUT_OF_MEMORY;
        goto exit;
    }

    /* For current design, EH_TYPE_MAX_NUMBER_OF_ARG_FOR_UI_MSG is 3.
     * Assumes that all of the arguments are valid and pass to swprintf.
     * There is no harm if any of the argument is invalid as long as the
     * message format string is defined properly.
     */
    rc=swprintf(wchar_msg_buf_p, required_wchar_msg_buf_len, wchar_fmt_buf_p,
        entry_p->args[0], entry_p->args[1], entry_p->args[2]);

    if (rc == -1)
    {
        printf("%s(%d) swprintf returns error.\r\n", __FUNCTION__, __LINE__);
        ret = EH_TYPE_RET_GENERATE_MSG_ERROR;
        goto exit;
    }

    /* prepare buffer for output_msg_buf_p (encoding decided by encoding_for_outmsg)
     * a conservative estimation for required buffer size is
     * 2*(required_wchar_msg_buf_len*sizeof(wchar_t))
     */
    output_msg_buf_len = 2*(required_wchar_msg_buf_len*sizeof(wchar_t));
    output_msg_buf_p = (char*)L_MM_Malloc((UI32_T)output_msg_buf_len,
        L_MM_USER_ID2(SYS_MODULE_EH, EH_TYPE_TRACE_ID_EH_MGR_GENERATE_MSG3));

    if (output_msg_buf_p == NULL)
    {
        printf("%s(%d)L_MM_Malloc error for size %d\r\n", __FUNCTION__, __LINE__,
            output_msg_buf_len);
        ret = EH_TYPE_RET_OUT_OF_MEMORY;
        goto exit;
    }

    /* convert wchar_msg_buf_p in "WCHAR_T encoding" to output_msg_buf_p in
     * "encoding_for_outmsg encoding"
     */
    iconv_encoding_name_to_p = EH_MGR_GetIconvEncodeName(encoding_for_outmsg);
    if (iconv_encoding_name_to_p==NULL)
    {
        printf("%s(%d) EH_MGR_GetIconvEncodeName error for encoding enum value %d\r\n",
            __FUNCTION__, __LINE__, (int)encoding_for_outmsg);
        ret = EH_TYPE_RET_CHAR_ENCODING_CONVERT_ERROR;
        goto exit;
    }

    iconv_encoding_name_from_p = EH_MGR_GetIconvEncodeName(EH_MGR_GENERATE_MSG_INTERMEDIATE_ENCODING);
    if (iconv_encoding_name_to_p==NULL)
    {
        printf("%s(%d) EH_MGR_GetIconvEncodeName error for encoding enum value %d\r\n",
            __FUNCTION__, __LINE__, (int)EH_MGR_GENERATE_MSG_INTERMEDIATE_ENCODING);
        ret = EH_TYPE_RET_CHAR_ENCODING_CONVERT_ERROR;
        goto exit;
    }

    iconv_handle = iconv_open(iconv_encoding_name_to_p,
        iconv_encoding_name_from_p);
    if (iconv_handle == (iconv_t)-1)
    {
        printf("%s(%d) iconv_open error.\r\n", __FUNCTION__,
            __LINE__);
        ret = EH_TYPE_RET_CHAR_ENCODING_CONVERT_ERROR;
        goto exit;
    }

    inbuf_p = (char*)wchar_msg_buf_p;
    inbytesleft = (size_t)(wcslen(wchar_msg_buf_p)*sizeof(wchar_t));
    outbuf_p = output_msg_buf_p;
    outbytesleft = (size_t)(output_msg_buf_len);
    iconv_rc = iconv(iconv_handle, &inbuf_p, &inbytesleft, &outbuf_p,
        &outbytesleft);

    if (iconv_rc == (size_t)(-1))
    {
        printf("%s(%d) iconv error.(errno=%d)\r\n", __FUNCTION__, __LINE__,
            errno);
        ret = EH_TYPE_RET_CHAR_ENCODING_CONVERT_ERROR;
        goto exit;
    }
    iconv_close(iconv_handle);
    iconv_handle = (iconv_t)-1;

    /* the size of the NULL terminated char is determined by the encoding,
     * assumes the size is wchar_t (UCS-4, 4 bytes), which should be enough
     * for all encoding.
     */
    if (outbytesleft<sizeof(wchar_t))
    {
        printf("%s(%d)left buffer size too small(=%d)\r\n", __FUNCTION__, __LINE__,
            (int)outbytesleft);
        ret = EH_TYPE_RET_CHAR_ENCODING_CONVERT_ERROR;
        goto exit;
    }
    /* add NULL terminated char in wchar_fmt_buf_p
     */
    memset(outbuf_p, 0, sizeof(wchar_t));

    /* let *msg_buf_pp point to the resulting output message
     * buffer(i.e. output_msg_buf_p)
     */
    *msg_buf_pp = output_msg_buf_p;

exit:
    if (wchar_fmt_buf_p!=NULL)
        L_MM_Free(wchar_fmt_buf_p);

    if (wchar_msg_buf_p!=NULL)
        L_MM_Free(wchar_msg_buf_p);

    if (iconv_handle != (iconv_t)-1)
    {
        iconv_close(iconv_handle);
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - EH_MGR_GetLangTageName 
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the language tag name used in error message xml file
 *            for the specified language.
 * INPUT    : lang                   --  The language id.
 * RETURN   : Non NULL               --  The langauge tag name for the given
 *                                       language id.
 *            NULL                   --  Invalid language id.
 * NOTE     : None
 *-------------------------------------------------------------------------*/
static const char* EH_MGR_GetLangTageName(EH_TYPE_LANGUAGE_T lang)
{
    static char* lang_tag_name[EH_TYPE_LANGUAGE_MAX_NUM+1] =
    {
        EH_TYPE_LANG_LIST(EH_TYPE_LANG_TAG_NAME)
    };

    if (lang >= EH_TYPE_LANGUAGE_MAX_NUM)
        return NULL;

    return lang_tag_name[lang];
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - EH_MGR_GetIconvEncodeName 
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the encoding name defined in iconv according to the specified
 *            encoding enum value.
 * INPUT    : encoding               --  encoding
 * RETURN   : Non NULL               --  The encoding name for the given
 *                                       encoding enum value.
 *            NULL                   --  Invalid encoding enum value.
 * NOTE     : None
 *-------------------------------------------------------------------------*/
static const char* EH_MGR_GetIconvEncodeName(EH_TYPE_ENCODING_T encoding)
{
    static char* iconv_encoding_name[EH_TYPE_ENCODING_MAX_NUM+1] =
    {
        EH_TYPE_ENCODING_LIST(EH_TYPE_ENCODING_NAME)
    };

    if (encoding >= EH_TYPE_ENCODING_MAX_NUM)
        return NULL;

    return iconv_encoding_name[encoding];
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - EH_MGR_BACKDOOR_PrintXPathNodes 
 *-------------------------------------------------------------------------
 * PURPOSE  : Print nodes got by XPath.
 * INPUT    : nodes                  --  Nodes got by XPath.
 * RETURN   : None
 * NOTE     : This function is used by backdoor.
 *-------------------------------------------------------------------------*/
static void EH_MGR_BACKDOOR_PrintXPathNodes(xmlNodeSetPtr nodes)
{
    xmlNodePtr cur;
    int size;
    int i;
    char* node_content_p;

    size = (nodes) ? nodes->nodeNr : 0;

    BACKDOOR_MGR_Printf("Result (%d nodes):\n", size);
    for (i = 0; i < size; ++i)
    {
        if (nodes->nodeTab[i]==NULL)
        {
            BACKDOOR_MGR_Printf("nodeTab[%d] is NULL.\r\n", i);
            continue;
        }

        if (nodes->nodeTab[i]->type == XML_NAMESPACE_DECL)
        {
            xmlNsPtr ns;

            ns = (xmlNsPtr)nodes->nodeTab[i];
            cur = (xmlNodePtr)ns->next;
            if (cur->ns)
            {
                BACKDOOR_MGR_Printf("= namespace \"%s\"=\"%s\" for node %s:%s\n",
                        ns->prefix, ns->href, cur->ns->href, cur->name);
            }
            else
            {
                BACKDOOR_MGR_Printf("= namespace \"%s\"=\"%s\" for node %s\n",
                        ns->prefix, ns->href, cur->name);
            }
        }
        else if (nodes->nodeTab[i]->type == XML_ELEMENT_NODE)
        {
            cur = nodes->nodeTab[i];
            if (cur->ns)
            {
                BACKDOOR_MGR_Printf("= element node \"%s:%s\"\n",
                        cur->ns->href, cur->name);
            }
            else
            {
                BACKDOOR_MGR_Printf("= element node \"%s\"\n",
                        cur->name);
            }
        }
        else if (nodes->nodeTab[i]->type == XML_TEXT_NODE)
        {
            cur = nodes->nodeTab[i];
            node_content_p = (char*)xmlNodeGetContent(cur);
            if (node_content_p != NULL)
            {
                if (cur->ns)
                {
                    BACKDOOR_MGR_Printf("= text node \"%s:%s\" : val= %s\n",
                            cur->ns->href, cur->name, node_content_p);
                }
                else
                {
                    BACKDOOR_MGR_Printf("= text node \"%s\" : val= %s\n",
                            cur->name, node_content_p);                
                }
                xmlFree(node_content_p);
                node_content_p=NULL;
            }
            else
                BACKDOOR_MGR_Printf("Failed to get node content of TEXT node.\n");
        }
        else
        {
            cur = nodes->nodeTab[i];
            BACKDOOR_MGR_Printf("= node \"%s\": type %d\n", cur->name, cur->type);
        }
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - EH_MGR_BACKDOOR_ExecuteXPathExpression 
 *-------------------------------------------------------------------------
 * PURPOSE  : Execute XPath expression on the given xml file.
 * INPUT    : filename         --  XML filename including absolute path.
 *            xpathExpr        --  XPath expression to be applied on the XML file
 *            ns_prefix_p      --  Prefix name for XML namespace
 *            ns_href_p        --  Href for the specified ns_prefix_p.
 * RETURN   : TRUE - Successfully
 *            FALSE- Failed
 * NOTE     : This function is used by backdoor.
 *-------------------------------------------------------------------------*/
static BOOL_T EH_MGR_BACKDOOR_ExecuteXPathExpression(const char* filename,
    const char* xpathExpr, const char* ns_prefix_p, const char* ns_href_p)
{
    xmlDocPtr doc;
    xmlXPathContextPtr xpathCtx;
    xmlXPathObjectPtr xpathObj;

    if (filename==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d):invalid filename\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (xpathExpr==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d):invalid xpathExpr\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    /* Load XML document */
    doc = xmlParseFile(filename);
    if (doc == NULL)
    {
        BACKDOOR_MGR_Printf("Error: unable to parse file \"%s\"\r\n", filename);
        return FALSE;
    }

    /* Create xpath evaluation context */
    xpathCtx = xmlXPathNewContext(doc);
    if (xpathCtx == NULL)
    {
        BACKDOOR_MGR_Printf("Error: unable to create new XPath context\r\n");
        xmlFreeDoc(doc);
        return FALSE;
    }

    /* Register namespaces from list (if any) */
    if (ns_prefix_p != NULL)
    {
        if (xmlXPathRegisterNs(xpathCtx, BAD_CAST ns_prefix_p, BAD_CAST ns_href_p) != 0)
        {
            BACKDOOR_MGR_Printf("Error: unable to register NS with prefix=\"%s\" and href=\"%s\"\r\n", ns_prefix_p, ns_prefix_p);
            xmlXPathFreeContext(xpathCtx);
            xmlFreeDoc(doc);
            return FALSE;
        }
    }

    /* Evaluate xpath expression */
    xpathObj = xmlXPathEvalExpression(BAD_CAST xpathExpr, xpathCtx);
    if (xpathObj == NULL)
    {
        BACKDOOR_MGR_Printf("Error: unable to evaluate xpath expression \"%s\"\r\n", xpathExpr);
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        return FALSE;
    }

    /* Print results */
    EH_MGR_BACKDOOR_PrintXPathNodes(xpathObj->nodesetval);

    /* Cleanup */
    xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(doc);

    return TRUE;
}

#if defined(EH_UNIT_TEST)

#define UI_THREAD_IPCMSGQ_KEY            SYS_BLD_CSCGROUP_IPCMSGQ_KEY_MAX
#define CSC_MGR_GROUP_THREAD_IPCMSGQ_KEY SYS_BLD_CSCGROUP_IPCMSGQ_KEY_MAX+1

enum
{
    EH_TEST_CMD_OK,
    EH_TEST_CMD_ERROR
};

typedef struct
{
    UI32_T cmd;
    UI32_T ui_msg_id;
    UI32_T num_of_args;
    UI32_T args[EH_TYPE_MAX_NUMBER_OF_ARG_FOR_UI_MSG];
    EH_TYPE_LANGUAGE_T lang;
    EH_TYPE_ENCODING_T encoding;
} EH_TEST_IpcMsg_T;

static SYSFUN_MsgQ_T ui_ipc_msgq_handle;
static SYSFUN_MsgQ_T csc_group_mgr_ipc_msgq_handle;
static UI32_T ui_thread_id=0;
static UI32_T mgr_thread_id=0;

static void ui_thread_task_main(void);
static void ui_thread_output_func(void* cookie, EH_TYPE_OutputMessage_T *output_msg_p);

static void csc_group_mgr_thread_task_main(void);

static void csc_mgr_ok_pfunc(void);
static void csc_mgr_error_pfunc(EH_TEST_IpcMsg_T *ipcmsg_p);

static void csc_mgr_ok_func(void);
static void csc_mgr_error_func(EH_TEST_IpcMsg_T *msg_p);



/* UI thread functions
 */
void create_ui_thread(void)
{
    UI32_T rc;

    rc = SYSFUN_SpawnUIThread(SYS_BLD_PROCESS_DEFAULT_PRIORITY,
                              SYSFUN_SCHED_DEFAULT,
                              "test_ui_thread",
                              SYS_BLD_TASK_LARGE_STACK_SIZE,
                              0,
                              &ui_thread_task_main,
                              NULL,
                              &ui_thread_id);
    if (rc != SYSFUN_OK)
    {
        printf("%s(%d): SYSFUN_SpawnUIThread error, rc=%lu\r\n", __FUNCTION__,
            __LINE__, rc);
    }
    printf("thread 'test_ui_thread' spawned(tid=%lu)\r\n", ui_thread_id);
    return;
}

#define UI_THREAD_EVENT_CALL_CSC_OK        BIT_0 /* emulate calling CSC and returns OK */

#define UI_THREAD_CMD_CALL_CSC_ERROR       1     /* call CSC MGR API with error */
#define UI_THREAD_CMD_OUTPUT_ERROR_MSG     2     /* force to output error message from EH */

static void ui_thread_task_main(void)
{
    UI32_T        rcv_event=0, rc;
    EH_TYPE_RET_T eh_rc;
    UI8_T         msg_buf[SYSFUN_SIZE_OF_MSG(sizeof(EH_TEST_IpcMsg_T))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)&msg_buf[0];
    
    if(SYSFUN_CreateMsgQ(UI_THREAD_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ui_ipc_msgq_handle)!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_CreateMsgQ fail. Task terminated\n", __FUNCTION__);
        return;
    }

    while(TRUE)
    {
        SYSFUN_ReceiveEvent (SYSFUN_SYSTEM_EVENT_IPCMSG |
                             UI_THREAD_EVENT_CALL_CSC_OK
                             ,SYSFUN_EVENT_WAIT_ANY,
                             SYSFUN_TIMEOUT_WAIT_FOREVER,
                             &rcv_event);

        if (rcv_event & UI_THREAD_EVENT_CALL_CSC_OK)
        {
            csc_mgr_ok_pfunc();
            rcv_event ^= UI_THREAD_EVENT_CALL_CSC_OK;
        }

        if (rcv_event & SYSFUN_SYSTEM_EVENT_IPCMSG)
        {

            rc = SYSFUN_ReceiveMsg(ui_ipc_msgq_handle, 0, SYSFUN_TIMEOUT_NOWAIT,
                sizeof(EH_TEST_IpcMsg_T), msgbuf_p);
            if (rc == SYSFUN_OK)
            {
                EH_TEST_IpcMsg_T *eh_msgbuf_p = (EH_TEST_IpcMsg_T*)msgbuf_p->msg_buf;

                switch (eh_msgbuf_p->cmd)
                {
                    case UI_THREAD_CMD_CALL_CSC_ERROR:
                        /* emulate calling CSC and returns ERROR
                         */
                        csc_mgr_error_pfunc(eh_msgbuf_p);
                        break;
                    case UI_THREAD_CMD_OUTPUT_ERROR_MSG:
                        eh_rc=EH_MGR_Process_Error_Info(NULL, ui_thread_output_func,
                        eh_msgbuf_p->lang, eh_msgbuf_p->encoding);
                        if (eh_rc!=EH_TYPE_RET_OK)
                        {
                            printf("EH_MGR_Process_Error_Info() returns error, rc=%d\r\n",
                                (int)eh_rc);
                        }
                        break;
                    default:
                        printf("%s(%d)Unknown cmd(=%lu)\r\n", __FUNCTION__,
                            __LINE__, eh_msgbuf_p->cmd);
                        break;
                }

                msgbuf_p->msg_size=0;
                rc = SYSFUN_SendResponseMsg(ui_ipc_msgq_handle, msgbuf_p);
                if (rc != SYSFUN_OK)
                {
                    printf("%s(%d)SYSFUN_SendResponseMsg error.(rc=%lu)\r\n",
                        __FUNCTION__, __LINE__, rc);
                }
            }
            else
            {
                printf("%s(%d)SYSFUN_ReceiveMsg error.(rc=%lu)\r\n", __FUNCTION__,
                    __LINE__, rc);
            }
            
        }
    };

    printf("thread 'test_ui_thread' teminated\r\n");
}

static void ui_thread_output_func(void* cookie, EH_TYPE_OutputMessage_T *output_msg_p)
{
    if (output_msg_p->encoding != EH_TYPE_ENCODING_UTF_8)
    {
        printf("%s(%d)Unexpected encoding = %d\r\n", __FUNCTION__, __LINE__,
            output_msg_p->encoding);
        return ;
    }

    printf("%s\r\n", output_msg_p->msg_p);

    printf("%s(%d) Done.\r\n", __FUNCTION__, __LINE__);
}

/* CSC GROUP MGR thread functions
 */
void create_csc_group_mgr_thread(void)
{
    UI32_T rc;

    rc = SYSFUN_SpawnThread(SYS_BLD_PROCESS_DEFAULT_PRIORITY,
                            SYSFUN_SCHED_DEFAULT,
                            "test_csc_group_thread",
                            SYS_BLD_TASK_LARGE_STACK_SIZE,
                            0,
                            &csc_group_mgr_thread_task_main,
                            NULL,
                            &mgr_thread_id);
    if (rc != SYSFUN_OK)
    {
        printf("%s(%d): SYSFUN_SpawnThread error, rc=%lu\r\n", __FUNCTION__,
            __LINE__, rc);
    }
    printf("thread 'test_csc_group_thread' spawned(tid=%lu)\r\n", mgr_thread_id);
    return;
}

static void csc_group_mgr_thread_task_main(void)
{
    UI32_T rcv_event, rc;
    UI8_T         msg_buf[SYSFUN_SIZE_OF_MSG(sizeof(EH_TEST_IpcMsg_T))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)&msg_buf[0];

    if(SYSFUN_CreateMsgQ(CSC_MGR_GROUP_THREAD_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &csc_group_mgr_ipc_msgq_handle)!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_CreateMsgQ fail. Task terminated\n", __FUNCTION__);
        return;
    }

    while(TRUE)
    {
        SYSFUN_ReceiveEvent (SYSFUN_SYSTEM_EVENT_IPCMSG
                             ,SYSFUN_EVENT_WAIT_ANY,
                             SYSFUN_TIMEOUT_WAIT_FOREVER,
                             &rcv_event);

            rc = SYSFUN_ReceiveMsg(csc_group_mgr_ipc_msgq_handle, 0, SYSFUN_TIMEOUT_NOWAIT,
                sizeof(EH_TEST_IpcMsg_T), msgbuf_p);
            if (rc == SYSFUN_OK)
            {
                EH_TEST_IpcMsg_T *eh_msgbuf_p = (EH_TEST_IpcMsg_T*)msgbuf_p->msg_buf;

                switch (eh_msgbuf_p->cmd)
                {
                    case EH_TEST_CMD_OK:
                        csc_mgr_ok_func();
                        break;
                    case EH_TEST_CMD_ERROR:
                        csc_mgr_error_func(eh_msgbuf_p);
                        break;
                    default:
                        printf("%s(%d)Unknown cmd(=%d)\r\n", __FUNCTION__,
                            __LINE__, (int)(eh_msgbuf_p->cmd));
                        break;
                }
                /* send response msg
                 */
                msgbuf_p->msg_size = 0;
                rc = SYSFUN_SendResponseMsg(csc_group_mgr_ipc_msgq_handle, msgbuf_p);
                if (rc != SYSFUN_OK)
                {
                    printf("%s(%d):SYSFUN_SendResponseMsg error(rc=%d)\r\n",
                        __FUNCTION__, __LINE__, (int)(rc));
                }
            }
            else
            {
                printf("%s(%d)SYSFUN_ReceiveMsg error.(rc=%lu)\r\n", __FUNCTION__,
                    __LINE__, rc);
            }

    }
}

/* CSC PMGR function
 */
static void csc_mgr_pfunc_util(EH_TEST_IpcMsg_T *ipcmsg_p)
{
    UI32_T        rc;
    UI8_T         msg_buf[SYSFUN_SIZE_OF_MSG(sizeof(EH_TEST_IpcMsg_T))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)&msg_buf[0];
    EH_TEST_IpcMsg_T *eh_msgbuf_p = (EH_TEST_IpcMsg_T*)msgbuf_p->msg_buf;

    *eh_msgbuf_p = *ipcmsg_p;
    msgbuf_p->msg_size = sizeof(EH_TEST_IpcMsg_T);
    rc = SYSFUN_SendRequestMsg(csc_group_mgr_ipc_msgq_handle, msgbuf_p,
        SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
        sizeof(EH_TEST_IpcMsg_T), msgbuf_p);

    if (rc != SYSFUN_OK)
    {
        printf("%s(%d) SYSFUN_SendRequestMsg error. (cmd=%lu,rc=%lu)\r\n",
            __FUNCTION__, __LINE__, ipcmsg_p->cmd, rc);
    }
}

static void csc_mgr_ok_pfunc(void)
{
    EH_TEST_IpcMsg_T ipcmsg;

    ipcmsg.cmd = EH_TEST_CMD_OK;
    csc_mgr_pfunc_util(&ipcmsg);
}

static void csc_mgr_error_pfunc(EH_TEST_IpcMsg_T *ipcmsg_p)
{
    ipcmsg_p->cmd = EH_TEST_CMD_ERROR;
    csc_mgr_pfunc_util(ipcmsg_p);
}

/* CSC function
 */
static void csc_mgr_ok_func(void)
{
    printf("%s() is called.\r\n", __FUNCTION__);
}

static void csc_mgr_error_func(EH_TEST_IpcMsg_T *msg_p)
{
    EH_TYPE_RET_T rc;

    rc = EH_MGR_Handle_Error(123, 456, msg_p->ui_msg_id, 0, msg_p->num_of_args,
        msg_p->args);
    if (rc != EH_TYPE_RET_OK)
    {
        printf("%s(%d) EH_MGR_Handle_Error returns error. (rc=%d)\r\n",
            __FUNCTION__, __LINE__, (int)rc);
    }
    printf("%s() is called.\r\n", __FUNCTION__);
}

/* Unit Test Backdoor Function
 */
static void eh_unit_test_backdoor_function(void)
{
    UI32_T rc, i;
    BOOL_T is_exit=FALSE;
    char buf[16];

    while (is_exit==FALSE)
    {
        BACKDOOR_MGR_Printf("EH UNIT TEST Backdoor\r\n");
        BACKDOOR_MGR_Printf("---------------------\r\n");
        BACKDOOR_MGR_Printf("1. Spawn threads for unit test\r\n");
        BACKDOOR_MGR_Printf("2. UI thread call CSC function and returns OK.\r\n");
        BACKDOOR_MGR_Printf("3. UI thread call CSC function and returns ERROR.\r\n");
        BACKDOOR_MGR_Printf("4. Force UI thread show error message from EH.\r\n");
        BACKDOOR_MGR_Printf("5. Execute XPath Expression Test\r\n");
        BACKDOOR_MGR_Printf("q. exit backdoor\r\n");
        BACKDOOR_MGR_Printf("Select:");

        if (BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1) == FALSE)
        {
            BACKDOOR_MGR_Printf("Input error.\r\n");
            continue;
        }
        BACKDOOR_MGR_Printf("\r\n");

        switch (buf[0])
        {
            case '1':
                create_ui_thread();
                create_csc_group_mgr_thread();
                break;
            case '2':
                if (ui_thread_id==0)
                {
                    BACKDOOR_MGR_Printf("UI thread for unit test has not been spawned yet.\r\n");
                    break;
                }

                rc = SYSFUN_SendEvent(ui_thread_id, UI_THREAD_EVENT_CALL_CSC_OK);
                if (rc != SYSFUN_OK)
                    BACKDOOR_MGR_Printf("SYSFUN_SendEvent error.(rc=%lu)\r\n", rc);
                break;
            case '3':
            {
                UI8_T         msg_buf[SYSFUN_SIZE_OF_MSG(sizeof(EH_TEST_IpcMsg_T))];
                SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)&msg_buf[0];
                EH_TEST_IpcMsg_T *eh_msgbuf_p = (EH_TEST_IpcMsg_T*)msgbuf_p->msg_buf;

                if (ui_thread_id==0)
                {
                    BACKDOOR_MGR_Printf("UI thread for unit test has not been spawned yet.\r\n");
                    break;
                }

                BACKDOOR_MGR_Printf("UI MSG ID=");
                if (BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1)==FALSE)
                {
                    BACKDOOR_MGR_Printf("%s(%d)Input error.\r\n", __FUNCTION__, __LINE__);
                    break;
                }
                eh_msgbuf_p->ui_msg_id = (UI32_T)atol(buf);
                BACKDOOR_MGR_Printf("\r\n");

                BACKDOOR_MGR_Printf("Number of argument=");
                if (BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1)==FALSE)
                {
                    BACKDOOR_MGR_Printf("%s(%d)Input error.\r\n", __FUNCTION__, __LINE__);
                    break;
                }
                eh_msgbuf_p->num_of_args= (UI32_T)atol(buf);
                BACKDOOR_MGR_Printf("\r\n");

                if (eh_msgbuf_p->num_of_args>EH_TYPE_MAX_NUMBER_OF_ARG_FOR_UI_MSG)
                {
                    BACKDOOR_MGR_Printf("Illegal number of argument.(Max Value=%d)\r\n", EH_TYPE_MAX_NUMBER_OF_ARG_FOR_UI_MSG);
                    break;
                }

                for (i=0; i<eh_msgbuf_p->num_of_args; i++)
                {
                    BACKDOOR_MGR_Printf("argument %lu=", i);
                    if (BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1)==FALSE)
                    {
                        BACKDOOR_MGR_Printf("%s(%d)Input error.\r\n", __FUNCTION__, __LINE__);
                        break;
                    }
                    eh_msgbuf_p->args[i]= (UI32_T)atol(buf);
                    BACKDOOR_MGR_Printf("\r\n");
                }
                if (i==eh_msgbuf_p->num_of_args)
                {
                    BACKDOOR_MGR_Printf("%s(%d): Send message to ui thread.\r\n",
                        __FUNCTION__, __LINE__);

                    eh_msgbuf_p->cmd = UI_THREAD_CMD_CALL_CSC_ERROR;
                    msgbuf_p->msg_size = sizeof(EH_TEST_IpcMsg_T);
                    rc = SYSFUN_SendRequestMsg(ui_ipc_msgq_handle, msgbuf_p,
                        SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                        sizeof(EH_TEST_IpcMsg_T), msgbuf_p);
                    if (rc != SYSFUN_OK)
                    {
                        BACKDOOR_MGR_Printf("%s(%d):SYSFUN_SendRequestMsg error(rc=%lu)\r\n",
                            __FUNCTION__, __LINE__, rc);
                    }
                }
            }
                break;

            case '4':
            {
                UI8_T         msg_buf[SYSFUN_SIZE_OF_MSG(sizeof(EH_TEST_IpcMsg_T))];
                SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)&msg_buf[0];
                EH_TEST_IpcMsg_T *eh_msgbuf_p = (EH_TEST_IpcMsg_T*)msgbuf_p->msg_buf;

                if (ui_thread_id==0)
                {
                    BACKDOOR_MGR_Printf("UI thread for unit test has not been spawned yet.\r\n");
                    break;
                }

                BACKDOOR_MGR_Printf("Lang=");
                if (BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1)==FALSE)
                {
                    BACKDOOR_MGR_Printf("%s(%d)Input error.\r\n", __FUNCTION__, __LINE__);
                    break;
                }
                eh_msgbuf_p->lang = (EH_TYPE_LANGUAGE_T)atoi(buf);
                BACKDOOR_MGR_Printf("\r\n");

                BACKDOOR_MGR_Printf("Encoding=");
                if (BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1)==FALSE)
                {
                    BACKDOOR_MGR_Printf("%s(%d)Input error.\r\n", __FUNCTION__, __LINE__);
                    break;
                }
                eh_msgbuf_p->encoding = (EH_TYPE_ENCODING_T)atoi(buf);
                BACKDOOR_MGR_Printf("\r\n");

                eh_msgbuf_p->cmd = UI_THREAD_CMD_OUTPUT_ERROR_MSG;
                msgbuf_p->msg_size = sizeof(EH_TEST_IpcMsg_T);
                rc = SYSFUN_SendRequestMsg(ui_ipc_msgq_handle, msgbuf_p,
                    SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                    sizeof(EH_TEST_IpcMsg_T), msgbuf_p);
                if (rc != SYSFUN_OK)
                {
                    BACKDOOR_MGR_Printf("%s(%d):SYSFUN_SendRequestMsg error(rc=%lu)\r\n",
                        __FUNCTION__, __LINE__, rc);
                }
            }
                break;
            case '5':
            {
                char buf_filename[80];
                char buf_xpathExpr[80];
                char buf_ns_prefix[80];
                char buf_ns_href[80];
                char *buf_ns_prefix_p=NULL;
                char * buf_ns_href_p=NULL;
                

                BACKDOOR_MGR_Printf("XML filename(including absolute path):");
                rc = BACKDOOR_MGR_RequestKeyIn(buf_filename, sizeof(buf_filename)-1);
                BACKDOOR_MGR_Printf("\r\n");
                if (rc == FALSE)
                {
                    BACKDOOR_MGR_Printf("Input error on filename.\r\n");
                    break;
                }

                BACKDOOR_MGR_Printf("XPath expression:");
                rc = BACKDOOR_MGR_RequestKeyIn(buf_xpathExpr, sizeof(buf_xpathExpr)-1);
                BACKDOOR_MGR_Printf("\r\n");
                if (rc == FALSE)
                {
                    BACKDOOR_MGR_Printf("Input error on Xpath expression.\r\n");
                    break;
                }

                BACKDOOR_MGR_Printf("Namespace prefix(Enter '0' if not set):");
                rc = BACKDOOR_MGR_RequestKeyIn(buf_ns_prefix, sizeof(buf_ns_prefix)-1);
                BACKDOOR_MGR_Printf("\r\n");
                if (rc == FALSE)
                {
                    BACKDOOR_MGR_Printf("Input error on Namespace prefix.\r\n");
                    break;
                }

                if (strcmp(buf_ns_prefix, "0")!=0)
                {
                    buf_ns_prefix_p = &(buf_ns_prefix[0]);
                    BACKDOOR_MGR_Printf("Namespace href for prefix '%s':", buf_ns_prefix);
                    rc = BACKDOOR_MGR_RequestKeyIn(buf_ns_href, sizeof(buf_ns_href)-1);
                    BACKDOOR_MGR_Printf("\r\n");
                    if (rc == FALSE)
                    {
                        BACKDOOR_MGR_Printf("Input error on Namespace href.\r\n");
                        break;
                    }
                    buf_ns_href_p = &(buf_ns_href[0]);
                }
                rc = EH_MGR_BACKDOOR_ExecuteXPathExpression(buf_filename,
                    buf_xpathExpr, buf_ns_prefix_p, buf_ns_href_p);
                if (rc==TRUE)
                    BACKDOOR_MGR_Printf("Execute XPath expression successfully.\r\n");

            }
                break;

            case 'q':
                is_exit=TRUE;
                break;
            default:
                BACKDOOR_MGR_Printf("Invalid option.\r\n");
                break;
        }
    }; /* end of while (is_exit==FALSE) */


}

void eh_unit_test_register_backdoor(UI32_T ipc_msgq_key)
{
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("eh_unit_test",
        ipc_msgq_key, eh_unit_test_backdoor_function);
}

#endif /* end of #if defined(EH_UNIT_TEST) */

