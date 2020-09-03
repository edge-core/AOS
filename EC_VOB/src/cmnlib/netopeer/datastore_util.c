/* MODULE NAME: datastore_util.c
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


/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <libxml/tree.h>
#include <sys/sem.h>
#include <signal.h>
#include <assert.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/xmlsave.h>
#include "sys_type.h"
#include "datastore_util.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
#define FIND_NODE_ADD_HEADER "accton-netopeer"

static const xmlChar current_ns_list[] = {"ofdpa10=urn:bcm:ofdpa10:accton01 onf11=urn:onf:of111:config:yang netopeer=urn:cesnet:tmc:datastores:file"};
static void DATASTORE_UTIL_CreateDatastore(xmlDocPtr *datastore);
static int DATASTORE_UTIL_RegisterNamespaces(xmlXPathContextPtr xpathCtx, const xmlChar* nsList);


void DATASTORE_UTIL_InitDatastore(char *datastore_path)
{
	xmlDocPtr datastore = NULL;

	/* get datastore
     */
	datastore = xmlReadFile(datastore_path, NULL, XML_PARSE_NOERROR | XML_PARSE_NOWARNING | XML_PARSE_NOBLANKS | XML_PARSE_NSCLEAN);

    if (datastore == NULL)
    {
		/* datastore does not exists, create a new one
         */
		DATASTORE_UTIL_CreateDatastore(&datastore);
    }

	return;
}

static void DATASTORE_UTIL_CreateDatastore(xmlDocPtr *datastore)
{
	xmlNodePtr root, node;
	xmlNsPtr ns;

	*datastore = xmlNewDoc(BAD_CAST "1.0");
	root = xmlNewNode(NULL, BAD_CAST "datastores");
	xmlDocSetRootElement(*datastore, root);
	ns = xmlNewNs(root, BAD_CAST "urn:cesnet:tmc:datastores:file", NULL);
	xmlSetNs(root, ns);
	node = xmlNewChild(root, root->ns, BAD_CAST "running", NULL);
	xmlNewProp(node, BAD_CAST "lock", BAD_CAST "");
	node =xmlNewChild(root, root->ns, BAD_CAST "candidate", NULL);
	xmlNewProp(node, BAD_CAST "lock", BAD_CAST "");
	xmlNewProp(node, BAD_CAST "modified", BAD_CAST "false");
	node = xmlNewChild(root, root->ns, BAD_CAST "startup", NULL);
	xmlNewProp(node, BAD_CAST "lock", BAD_CAST "");
}

/* FUNCTION NAME: DATASTORE_UTIL_AddNode
 * PURPOSE: Add new child element.
 * INPUT:   xml_file -- the input XML filename.
 *          xpath_expr -- the pointer to an XPath context
 *          child_info -- content for new child
 *          href -- The URI associated
 *          prefix -- The prefix for the namespace
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 */
DATASTORE_UTIL_Error_T DATASTORE_UTIL_AddNode(char *xml_file, char *xpath_expr, char* child_info, char* href, char* prefix)
{
    int node_number = 0;
    int i = 0;
    xmlChar *xmlbuff;
    xmlDocPtr           doc = NULL, child_doc = NULL;
    xmlXPathContextPtr  xpathCtx = NULL;
    xmlXPathObjectPtr   xpathObj = NULL;
    xmlNodeSetPtr       nodes = NULL;
    xmlNodePtr          cur=NULL, root=NULL, config = NULL;
    xmlNsPtr ns = NULL;

    if ((xml_file == NULL) || (xpath_expr == NULL) || (child_info == NULL))
    {
        return DATASTORE_UTIL_ERROR_PARAM;
    }

    /* Init libxml
     */
    xmlInitParser();

    /* Load XML document
     */
    doc = xmlParseFile(xml_file);
    if (doc == NULL)
    {
        return DATASTORE_UTIL_ERROR_PARAM;
    }

    /* Create xpath evaluation context
     */
    xpathCtx = xmlXPathNewContext(doc);
    if(xpathCtx == NULL)
    {
        xmlFreeDoc(doc);
        return DATASTORE_UTIL_ERROR_FAIL;
    }

    /* Register namespaces from list
     */
    if((current_ns_list != NULL) && (DATASTORE_UTIL_RegisterNamespaces(xpathCtx, current_ns_list) < 0))
    {
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        return DATASTORE_UTIL_ERROR_FAIL;
    }

    /* Evaluate xpath expression
     */
    xpathObj = xmlXPathEvalExpression((const xmlChar *)xpath_expr, xpathCtx);
    if(xpathObj == NULL)
    {
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        return DATASTORE_UTIL_ERROR_FAIL;
    }

    nodes = xpathObj->nodesetval;
    if(nodes == NULL)
    {
        xmlXPathFreeObject(xpathObj);
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        return DATASTORE_UTIL_ERROR_FAIL;
    }

    node_number = nodes->nodeNr;
    xmlbuff = xmlStrdup(child_info);
    child_doc = xmlRecoverDoc(xmlbuff);
    if(NULL == child_doc)
    {
        xmlXPathFreeObject(xpathObj);
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        return DATASTORE_UTIL_ERROR_FAIL;
    }

    root = xmlDocGetRootElement(child_doc);
    if(NULL == root)
    {
        xmlXPathFreeObject(xpathObj);
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        return DATASTORE_UTIL_ERROR_FAIL;
    }

    for(i = 0; i < node_number; ++i)
    {
        cur = nodes->nodeTab[i];

        if ( (root->children != NULL) && (root->children->type == XML_TEXT_NODE))
        {
            config=xmlNewChild(cur, cur->ns, BAD_CAST(root->name), BAD_CAST(root->children->content));
            if (href != NULL)
            {
                ns = xmlNewNs(config, BAD_CAST href, BAD_CAST prefix);
                xmlSetNs(config, ns);
            }
        }
        else
        {
            config=xmlNewChild(cur, cur->ns, BAD_CAST root->name, NULL);
            if (href != NULL)
            {
                ns = xmlNewNs(config, BAD_CAST href, BAD_CAST prefix);
                xmlSetNs(config, ns);
            }
            for (cur = root->children; (cur != NULL); cur = cur->next)
            {
                xmlNewTextChild(config, config->ns, BAD_CAST(cur->name), BAD_CAST(cur->children->content));
            }
        }
    }

    /* Save change.
     * To avoid blanks and shall generate indentation.
     */
    xmlKeepBlanksDefault(0);
    xmlSaveFormatFile(xml_file, doc, 1);

    /* Cleanup
     */
    xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(doc);
    xmlFreeDoc(child_doc);

    /* Shutdown libxml
     */
    xmlCleanupParser();

    return DATASTORE_UTIL_ERROR_NONE;
}

/* FUNCTION NAME: DATASTORE_UTIL_RemoveNode
 * PURPOSE: Remove child element.
 * INPUT:   xml_file -- the input XML filename.
 *          xpath_expr -- the pointer to an XPath context
 *          content -- vaule of the element to be removed.
 *          is_key -- TRUE is key field.
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   For example,
 *          list X[] have two elements, one is A(key) and another is B.
 *          1. xpath_expr=A, content="a", is_key = TRUE, remove list X[a] node.
 *          2. xpath_expr=B, content="b", is_key = FALSE, remove element B of list X[a].
 */
DATASTORE_UTIL_Error_T DATASTORE_UTIL_RemoveNode(char *xml_file, char *xpath_expr, char *content, BOOL_T is_key)
{
    int node_number = 0;
    int i = 0;
    xmlDocPtr           doc = NULL;
    xmlXPathContextPtr  xpathCtx = NULL;
    xmlXPathObjectPtr   xpathObj = NULL;
    xmlNodeSetPtr       nodes = NULL;
    xmlNodePtr          cur = NULL;

    if ((xml_file == NULL) || (xpath_expr == NULL) || (content == NULL))
    {
        return DATASTORE_UTIL_ERROR_PARAM;
    }

    /* Init libxml
     */
    xmlInitParser();

    /* Load XML document
     */
    doc = xmlParseFile(xml_file);
    if (doc == NULL)
    {
        return DATASTORE_UTIL_ERROR_PARAM;
    }

    /* Create xpath evaluation context
     */
    xpathCtx = xmlXPathNewContext(doc);
    if(xpathCtx == NULL)
    {
        xmlFreeDoc(doc);
        return DATASTORE_UTIL_ERROR_FAIL;
    }

    /* Register namespaces from list
     */
    if((current_ns_list != NULL) && (DATASTORE_UTIL_RegisterNamespaces(xpathCtx, current_ns_list) < 0))
    {
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        return DATASTORE_UTIL_ERROR_FAIL;
    }

    /* Evaluate xpath expression
     */
    xpathObj = xmlXPathEvalExpression(xpath_expr, xpathCtx);
    if(xpathObj == NULL)
    {
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        return DATASTORE_UTIL_ERROR_FAIL;
    }

    nodes = xpathObj->nodesetval;
    if(nodes == NULL)
    {
        xmlXPathFreeObject(xpathObj);
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        return DATASTORE_UTIL_ERROR_FAIL;
    }

    node_number = nodes->nodeNr;
    for(i = 0; i < node_number; ++i)
    {
        cur = nodes->nodeTab[i];

        if (xmlStrcmp(cur->children->content, BAD_CAST content) == 0)
        {
            if (FALSE == is_key)
            {
                xmlUnlinkNode(cur);
                xmlFreeNode(cur);
            }
            else
            {
                xmlUnlinkNode(cur->parent);
                xmlFreeNode(cur->parent);
            }
        }
    }

    /* Save change.
     * To avoid blanks and shall generate indentation.
     */
    xmlKeepBlanksDefault(0);
    xmlSaveFormatFile(xml_file, doc, 1);

    /* Cleanup
     */
    xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(doc);

    /* Shutdown libxml
     */
    xmlCleanupParser();

    return DATASTORE_UTIL_ERROR_NONE;
}

/* FUNCTION NAME: DATASTORE_UTIL_GetNodeByXpath
 * PURPOSE: Get node element and output to a file.
 * INPUT:   xml_file -- the input XML filename.
 *          xpath_expr -- the pointer to an XPath context
 *          file_path -- write result in specifed file
 * OUTPUT:  exist -- TRUE means find.
 * RETURN:  None
 * NOTES:   None
 */
DATASTORE_UTIL_Error_T DATASTORE_UTIL_GetNodeByXpath(char *xml_file, char *xpath_expr, char *file_path, BOOL_T *exist)
{
    int node_number = 0;
    int i = 0;
    xmlDocPtr doc = NULL;
    xmlXPathContextPtr xpathCtx = NULL;
    xmlXPathObjectPtr xpathObj = NULL;
    xmlNodeSetPtr nodes = NULL;
    xmlNodePtr cur=NULL;
    FILE *result_file;

    *exist = FALSE;
    if ((xml_file == NULL) || (xpath_expr == NULL))
    {
        return DATASTORE_UTIL_ERROR_PARAM;
    }

    result_file = fopen(file_path, "w+");
    if (result_file==NULL)
    {
        return DATASTORE_UTIL_ERROR_FAIL;
    }

    /* Init libxml
     */
    xmlInitParser();

    /* Load XML document
     */
    doc = xmlParseFile(xml_file);
    if (doc == NULL)
    {
        fclose(result_file);
		return DATASTORE_UTIL_ERROR_FAIL;
    }

    /* Create xpath evaluation context
     */
    xpathCtx = xmlXPathNewContext(doc);
    if(xpathCtx == NULL)
    {
        xmlFreeDoc(doc);
        fclose(result_file);
        return DATASTORE_UTIL_ERROR_FAIL;
    }

    /* Register namespaces from list
     */
    if((current_ns_list != NULL) && (DATASTORE_UTIL_RegisterNamespaces(xpathCtx, current_ns_list) < 0))
    {
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        fclose(result_file);
        return DATASTORE_UTIL_ERROR_FAIL;
    }

    /* Evaluate xpath expression
     */
    xpathObj = xmlXPathEvalExpression(xpath_expr, xpathCtx);
    if(xpathObj == NULL)
    {
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        fclose(result_file);
        return DATASTORE_UTIL_ERROR_FAIL;
    }

    nodes = xpathObj->nodesetval;
    if(nodes == NULL)
    {
        xmlXPathFreeObject(xpathObj);
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        fclose(result_file);
        return DATASTORE_UTIL_ERROR_FAIL;
    }
    node_number = nodes->nodeNr;

    /* Write result to file
     */
    fprintf(result_file, "<%s>\n", FIND_NODE_ADD_HEADER);
    for(i = 0; i < node_number; ++i)
    {
        *exist = TRUE;
        cur = nodes->nodeTab[i];
        {
            xmlBufferPtr bufferPtr = xmlBufferCreate();
            xmlNodeDump(bufferPtr, cur->doc, (xmlNodePtr)cur, 0, 0);
            fprintf(result_file, "%s\n", (const char *)bufferPtr->content);
        }
    }
    fprintf(result_file, "</%s>\n", FIND_NODE_ADD_HEADER);

    /* Cleanup
     */
    xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(doc);
    fclose(result_file);

    /* Shutdown libxml
     */
    xmlCleanupParser();

    return DATASTORE_UTIL_ERROR_NONE;
}

/* FUNCTION NAME: DATASTORE_UTIL_ModifyNodeByXpath
 * PURPOSE: Modify content for specified node element.
 * INPUT:   xml_file -- the input XML filename.
 *          xpath_expr -- the pointer to an XPath context
 *          content -- new vaule
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 */
DATASTORE_UTIL_Error_T DATASTORE_UTIL_ModifyNodeByXpath(char *xml_file, char *xpath_expr, char *content)
{
    int node_number = 0;
    int i = 0;
    xmlDocPtr doc = NULL;
    xmlXPathContextPtr xpathCtx = NULL;
    xmlXPathObjectPtr xpathObj = NULL;
    xmlNodeSetPtr nodes = NULL;
    xmlNodePtr cur=NULL;

    if ((xml_file == NULL) || (xpath_expr == NULL))
    {
        return DATASTORE_UTIL_ERROR_PARAM;
    }

    /* Init libxml
     */
    xmlInitParser();

    /* Load XML document
     */
    doc = xmlParseFile(xml_file);
    if (doc == NULL)
    {
        return DATASTORE_UTIL_ERROR_FAIL;
    }

    /* Create xpath evaluation context
     */
    xpathCtx = xmlXPathNewContext(doc);
    if(xpathCtx == NULL)
    {
        xmlFreeDoc(doc);
        return DATASTORE_UTIL_ERROR_FAIL;
    }

    /* Register namespaces from list
     */
    if((current_ns_list != NULL) && (DATASTORE_UTIL_RegisterNamespaces(xpathCtx, current_ns_list) < 0))
    {
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        return DATASTORE_UTIL_ERROR_FAIL;
    }

    /* Evaluate xpath expression
     */
    xpathObj = xmlXPathEvalExpression(xpath_expr, xpathCtx);
    if(xpathObj == NULL)
    {
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        return DATASTORE_UTIL_ERROR_FAIL;
    }

    nodes = xpathObj->nodesetval;
    if(nodes == NULL)
    {
        xmlXPathFreeObject(xpathObj);
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        return DATASTORE_UTIL_ERROR_FAIL;
    }

    node_number = nodes->nodeNr;
    for(i = 0; i < node_number; ++i)
    {
        cur = nodes->nodeTab[i];
        if (cur->children->type == XML_TEXT_NODE)
        {
            xmlNodeSetContent(cur, content);
        }
    }

    /* Save change.
     * To avoid blanks and shall generate indentation.
     */
    xmlKeepBlanksDefault(0);
    xmlSaveFormatFile(xml_file, doc, 1);

    /* Cleanup
     */
    xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(doc);

    /* Shutdown libxml
     */
    xmlCleanupParser();

    return DATASTORE_UTIL_ERROR_NONE;
}

/* FUNCTION NAME: DATASTORE_UTIL_CopyNodeByXpath
 * PURPOSE: Copy content of src node to destination node.
 * INPUT:   xml_file -- the input XML filename.
 *          dest_xpath_expr -- the pointer to an XPath context
 *          src_xpath_expr -- the pointer to an XPath context
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   One to one copy
 */
DATASTORE_UTIL_Error_T DATASTORE_UTIL_CopyNodeByXpath(char *xml_file, char *dest_xpath_expr, char *src_xpath_expr)
{
    int dest_node_number = 0, src_node_number = 0;
    xmlDocPtr doc = NULL;
    xmlXPathContextPtr xpathCtx = NULL;
    xmlXPathObjectPtr dest_xpathObj = NULL, src_xpathObj = NULL;;
    xmlNodeSetPtr dest_nodes = NULL, src_nodes = NULL;
    xmlNodePtr dest_cur=NULL, src_cur=NULL, tmp_node=NULL;


    if ((xml_file == NULL) || (dest_xpath_expr == NULL) || (src_xpath_expr == NULL) )
    {
        return DATASTORE_UTIL_ERROR_PARAM;
    }

    /* Init libxml
     */
    xmlInitParser();

    /* Load XML document
     */
    doc = xmlParseFile(xml_file);
    if (doc == NULL)
    {
		return DATASTORE_UTIL_ERROR_FAIL;
    }

    /* Create xpath evaluation context
     */
    xpathCtx = xmlXPathNewContext(doc);
    if(xpathCtx == NULL)
    {
        xmlFreeDoc(doc);
        return DATASTORE_UTIL_ERROR_FAIL;
    }

    /* Register namespaces from list
     */
    if((current_ns_list != NULL) && (DATASTORE_UTIL_RegisterNamespaces(xpathCtx, current_ns_list) < 0))
    {
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        return DATASTORE_UTIL_ERROR_FAIL;
    }

    /* Evaluate xpath expression
     */
    dest_xpathObj = xmlXPathEvalExpression(dest_xpath_expr, xpathCtx);
    if(dest_xpathObj == NULL)
    {
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        return DATASTORE_UTIL_ERROR_FAIL;
    }

    dest_nodes = dest_xpathObj->nodesetval;
    if (dest_nodes == NULL)
    {
        xmlXPathFreeObject(dest_xpathObj);
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        return DATASTORE_UTIL_ERROR_FAIL;
    }

    dest_node_number = dest_nodes->nodeNr;

    src_xpathObj = xmlXPathEvalExpression(src_xpath_expr, xpathCtx);
    if(src_xpathObj == NULL)
    {
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        return DATASTORE_UTIL_ERROR_FAIL;
    }

    src_nodes = src_xpathObj->nodesetval;
    if (src_nodes == NULL)
    {
        xmlXPathFreeObject(src_xpathObj);
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        return DATASTORE_UTIL_ERROR_FAIL;
    }

    src_node_number = src_nodes->nodeNr;

    /* Can only copy one node
     */
    if ((dest_node_number == 1) && (src_node_number == 1))
    {
        dest_cur = dest_nodes->nodeTab[0];
        src_cur = src_nodes->nodeTab[0];
        tmp_node = xmlCopyNode(src_cur, 1);
        xmlReplaceNode(dest_cur, tmp_node);

        /* Save change.
         * To avoid blanks and shall generate indentation.
         */
        xmlKeepBlanksDefault(0);
        xmlSaveFormatFile(xml_file, doc, 1);
    }

    /* Cleanup
     */
    xmlXPathFreeObject(dest_xpathObj);
    xmlXPathFreeObject(src_xpathObj);
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(doc);

    /* Shutdown libxml
     */
    xmlCleanupParser();
    return DATASTORE_UTIL_ERROR_NONE;
}


/* FUNCTION NAME: DATASTORE_UTIL_RegisterNamespaces
 * PURPOSE: Registers namespaces from nsList.
 * INPUT:   xpathCtx -- the pointer to an XPath context.
 *          nsList -- the list of known namespaces in
 *			"<prefix1>=<href1> <prefix2>=href2> ..." format.
 * OUTPUT:  None
 * RETURN:  Returns 0 on success and a negative value otherwise.
 * NOTES:   None
 */
static int DATASTORE_UTIL_RegisterNamespaces(xmlXPathContextPtr xpathCtx, const xmlChar* nsList)
{
    xmlChar* nsListDup;
    xmlChar* prefix;
    xmlChar* href;
    xmlChar* next;

    assert(xpathCtx);
    assert(nsList);

    nsListDup = xmlStrdup(nsList);
    if(nsListDup == NULL)
    {
        return(-1);
    }

    next = nsListDup;
    while(next != NULL)
    {
        /* skip spaces
         */
        while((*next) == ' ') next++;
        if((*next) == '\0') break;

        /* find prefix
         */
        prefix = next;
        next = (xmlChar*)xmlStrchr(next, '=');
        if(next == NULL)
        {
            xmlFree(nsListDup);
            return(-1);
        }
        *(next++) = '\0';

        /* find href
         */
        href = next;
        next = (xmlChar*)xmlStrchr(next, ' ');
        if(next != NULL)
        {
            *(next++) = '\0';
        }

        /* do register namespace
         */
        if(xmlXPathRegisterNs(xpathCtx, prefix, href) != 0)
        {
            xmlFree(nsListDup);
            return(-1);
        }
	}

    xmlFree(nsListDup);
    return(0);
}
