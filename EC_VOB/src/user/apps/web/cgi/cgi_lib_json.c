static char *CGI_LIB_JSON_GetXmlContent(xmlNodePtr cur);

static UI32_T CGI_LIB_JSON_ParseString(xmlNodePtr node, char *str, UI32_T cch_str, HTTP_Connection_T *http_connection, int sock);
static UI32_T CGI_LIB_JSON_ParseValue(xmlNodePtr node, char *str, UI32_T cch_str, HTTP_Connection_T *http_connection, int sock);

static UI32_T CGI_LIB_JSON_InternalStringify(xmlNodePtr node, int (*fn)(HTTP_Connection_T *http_connection, int sock, const char *text), HTTP_Connection_T *http_connection, int sock, char *buf, size_t cchbuf);

static UI32_T CGI_LIB_JSON_ObjectStringify(xmlNodePtr node, int (*fn)(HTTP_Connection_T *http_connection, int sock, const char *text), HTTP_Connection_T *http_connection, int sock, char *buf, size_t cchbuf);
static UI32_T CGI_LIB_JSON_MembersStringify(xmlNodePtr node, int (*fn)(HTTP_Connection_T *http_connection, int sock, const char *text), HTTP_Connection_T *http_connection, int sock, char *buf, size_t cchbuf);

static UI32_T CGI_LIB_JSON_ArrayStringify(xmlNodePtr node, int (*fn)(HTTP_Connection_T *http_connection, int sock, const char *text), HTTP_Connection_T *http_connection, int sock, char *buf, size_t cchbuf);
static UI32_T CGI_LIB_JSON_ElementsStringify(xmlNodePtr node, int (*fn)(HTTP_Connection_T *http_connection, int sock, const char *text), HTTP_Connection_T *http_connection, int sock, char *buf, size_t cchbuf);

static UI32_T CGI_LIB_JSON_ValueStringify(xmlNodePtr node, int (*fn)(HTTP_Connection_T *http_connection, int sock, const char *text), HTTP_Connection_T *http_connection, int sock, char *buf, size_t cchbuf);

static UI32_T CGI_LIB_JSON_StringStringify(xmlNodePtr node, int (*fn)(HTTP_Connection_T *http_connection, int sock, const char *text), HTTP_Connection_T *http_connection, int sock, char *buf, size_t cchbuf);
static UI32_T CGI_LIB_JSON_NumberStringify(xmlNodePtr node, int (*fn)(HTTP_Connection_T *http_connection, int sock, const char *text), HTTP_Connection_T *http_connection, int sock, char *buf, size_t cchbuf);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - CGI_LIB_JSON_GetXmlContent
 *------------------------------------------------------------------------------
 * PURPOSE:  Get children's content of element node whose chilren is content node.
 * INPUT:    cur                     -- Node.
 * OUTPUT:   None.
 * RETURN:   cur->children->content  -- Success.
 *           NULL                    -- children node is not content node.
 * NOTE:     Simplify xmlNodeGetContent().
 *------------------------------------------------------------------------------
 */
static char *CGI_LIB_JSON_GetXmlContent(xmlNodePtr cur)
{
    if (cur == NULL)
    {
        return NULL;
    }

    if (cur->type == XML_ELEMENT_NODE)
    {
        if (cur->children != NULL &&
            cur->children->type == XML_TEXT_NODE)
        {
            return ((char *) cur->children->content);
        }
    }

    return NULL;
}

#define CLI_LIB_JSON_WRITE(new_str) {                                   \
    if (str[0] != '\0') {                                               \
        if (cgi_SendText(http_connection, sock, (const char *) str) != 0) {   \
            return CORE_SSI_FAIL;                                       \
        }                                                               \
        str[0] = '\0';                                                  \
    }                                                                   \
    if (cgi_SendText(http_connection, sock, (const char *) new_str) != 0) {   \
        return CORE_SSI_FAIL;                                           \
    }                                                                   \
}

#define CGI_LIB_JSON_WRITE(new_str) {                                   \
    if (buf[0] != '\0') {                                               \
        if (fn(http_connection, sock, (I8_T *) buf) != 0) {             \
            return CORE_SSI_FAIL;                                       \
        }                                                               \
        buf[0] = '\0';                                                  \
    }                                                                   \
    if (fn(http_connection, sock, (I8_T *) new_str) != 0) {             \
        return CORE_SSI_FAIL;                                           \
    }                                                                   \
}

#define CGI_XML_NODE_TYPE_TABLE \
    CGI_XML_NODE_TYPE(OBJECT = 0,   object) \
    CGI_XML_NODE_TYPE(ARRAY,        array)  \
    CGI_XML_NODE_TYPE(KEY,          key)    \
    CGI_XML_NODE_TYPE(STRING,       string) \
    CGI_XML_NODE_TYPE(NUMBER,       number) \
    CGI_XML_NODE_TYPE(BOOLEAN,      boolean)

typedef enum
{
    CGI_XML_NODE_TYPE_MIN = 0,

#define CGI_XML_NODE_TYPE(type, typestring) CGI_XML_NODE_TYPE_##type,

    CGI_XML_NODE_TYPE_TABLE

#undef CGI_XML_NODE_TYPE

    CGI_XML_NODE_TYPE_UNKNOWN

} CGI_XML_NODE_TYPE_T;

const static char *node_type_string_array[] =
{
#define CGI_XML_NODE_TYPE(type, typestring) #typestring,

    CGI_XML_NODE_TYPE_TABLE

#undef CGI_XML_NODE_TABLE

    "unknown"
};

static CGI_XML_NODE_TYPE_T CGI_LIB_JSON_NodeTypeOf(xmlNodePtr node)
{
    int type;

    for (type = CGI_XML_NODE_TYPE_MIN; type < _countof(node_type_string_array); ++type)
    {
        if (xmlStrcmp(node->name, BAD_CAST node_type_string_array[type]) == 0)
        {
            return (CGI_XML_NODE_TYPE_T) type;
        }
    }

    return CGI_XML_NODE_TYPE_UNKNOWN;
}

static BOOL_T CGI_LIB_JSON_IsSameNodeType(xmlNodePtr node, CGI_XML_NODE_TYPE_T type)
{
    if (strcmp((char *) node->name, node_type_string_array[type]) == 0)
    {
        return TRUE;
    }

    return FALSE;
}

static const char * CGI_LIB_JSON_TypeString(CGI_XML_NODE_TYPE_T type)
{
    if (_countof(node_type_string_array) <= type)
    {
        type = CGI_XML_NODE_TYPE_UNKNOWN;
    }

    return node_type_string_array[type];
}

static xmlNodePtr CGI_LIB_JSON_Object(xmlNodePtr parent)
{
    const char *sztype = CGI_LIB_JSON_TypeString(CGI_XML_NODE_TYPE_OBJECT);

    ASSERT(strcmp(sztype, "object") == 0);

    if (parent)
    {
        return xmlNewChild(parent, NULL, BAD_CAST sztype, NULL);
    }

    return xmlNewNode(NULL, BAD_CAST sztype);
}

static xmlNodePtr CGI_LIB_JSON_Array(xmlNodePtr parent)
{
    const char *sztype = CGI_LIB_JSON_TypeString(CGI_XML_NODE_TYPE_ARRAY);

    ASSERT(strcmp(sztype, "array") == 0);

    if (parent)
    {
        return xmlNewChild(parent, NULL, BAD_CAST sztype, NULL);
    }

    return xmlNewNode(NULL, BAD_CAST sztype);
}

static xmlNodePtr CGI_LIB_JSON_CreateKeyNode(xmlNodePtr parent, const char *text)
{
    const char *sztype = CGI_LIB_JSON_TypeString(CGI_XML_NODE_TYPE_KEY);

    ASSERT(strcmp(sztype, "key") == 0);

    return xmlNewTextChild(parent, NULL, BAD_CAST sztype, BAD_CAST text);
}

static xmlNodePtr CGI_LIB_JSON_String(xmlNodePtr parent, const char *text)
{
    const char *sztype = CGI_LIB_JSON_TypeString(CGI_XML_NODE_TYPE_STRING);
    xmlNodePtr ptr;

    ASSERT(strcmp(sztype, "string") == 0);

    if (parent)
    {
        return xmlNewTextChild(parent, NULL, BAD_CAST sztype, BAD_CAST text);
    }

    ptr = xmlNewNode(NULL, BAD_CAST sztype);

    if (ptr)
    {
        xmlNodeAddContent(ptr, BAD_CAST text);
    }

    return ptr;
}

static xmlNodePtr CGI_LIB_JSON_Number(xmlNodePtr parent, const char *text)
{
    const char *sztype = CGI_LIB_JSON_TypeString(CGI_XML_NODE_TYPE_NUMBER);
    xmlNodePtr ptr;

    ASSERT(strcmp(sztype, "number") == 0);

    if (parent)
    {
        return xmlNewTextChild(parent, NULL, BAD_CAST sztype, BAD_CAST text);
    }

    ptr = xmlNewNode(NULL, BAD_CAST sztype);

    if (ptr)
    {
        xmlNodeAddContent(ptr, BAD_CAST text);
    }

    return ptr;
}

static xmlNodePtr CGI_LIB_JSON_CreateBooleanNode(xmlNodePtr parent, const char *text)
{
    const char *sztype = CGI_LIB_JSON_TypeString(CGI_XML_NODE_TYPE_BOOLEAN);
    xmlNodePtr ptr;

    ASSERT(strcmp(sztype, "boolean") == 0);

    if (parent)
    {
        return xmlNewTextChild(parent, NULL, BAD_CAST sztype, BAD_CAST text);
    }

    ptr = xmlNewNode(NULL, BAD_CAST sztype);

    if (ptr)
    {
        xmlNodeAddContent(ptr, BAD_CAST text);
    }

    return ptr;
}

static UI32_T CGI_LIB_JSON_InternalStringify(xmlNodePtr node, int (*fn)(HTTP_Connection_T *http_connection, int sock, const char *text), HTTP_Connection_T *http_connection, int sock, char *buf, size_t cchbuf)
{
    ASSERT(buf);
    ASSERT(cchbuf > 0);

    if (!node)
    {
        return CORE_SSI_FAIL;
    }

    return CGI_LIB_JSON_ValueStringify(node, fn, http_connection, sock, buf, cchbuf);
}

static UI32_T CGI_LIB_JSON_ObjectStringify(xmlNodePtr node, int (*fn)(HTTP_Connection_T *http_connection, int sock, const char *text), HTTP_Connection_T *http_connection, int sock, char *buf, size_t cchbuf)
{
    ASSERT(node);
    ASSERT(buf);
    ASSERT(cchbuf > 0);

    CGI_LIB_JSON_WRITE("{");

    CGI_LIB_JSON_MembersStringify(node->children, fn, http_connection, sock, buf, cchbuf);

    buf[0] = '\0';
    CGI_LIB_JSON_WRITE("}");

    return CORE_NO_ERROR;
}

static UI32_T CGI_LIB_JSON_MembersStringify(xmlNodePtr node, int (*fn)(HTTP_Connection_T *http_connection, int sock, const char *text), HTTP_Connection_T *http_connection, int sock, char *buf, size_t cchbuf)
{
    ASSERT(buf);
    ASSERT(cchbuf > 0);

    while (node)
    {
        CGI_XML_NODE_TYPE_T first_type;
        CGI_XML_NODE_TYPE_T second_type;

        first_type = CGI_LIB_JSON_NodeTypeOf(node);

        /* The children should be composed by <key, value> pair.
         * Ignore the malformed node before read it.
         */
        if (first_type != CGI_XML_NODE_TYPE_KEY)
        {
            node = node->next;
            continue;
        }

        if (node->next == NULL)
        {
            break;
        }

        second_type = CGI_LIB_JSON_NodeTypeOf(node->next);

        if (second_type == CGI_XML_NODE_TYPE_KEY)
        {
            node = node->next;
            continue;
        }

        if (second_type == CGI_XML_NODE_TYPE_UNKNOWN)
        {
            node = node->next->next;
            continue;
        }

        /* If parse failed for a key node, ignore it.
         */
        if (CGI_LIB_JSON_StringStringify(node, fn, http_connection, sock, buf, cchbuf) != CORE_NO_ERROR)
        {
            node = node->next;
            continue;
        }

        CGI_LIB_JSON_WRITE(":");

        /* If parse failed for a value node this time because we can't
         * undo here (always flush key string). So use a null object to
         * instead of this error.
         */
        if (CGI_LIB_JSON_ValueStringify(node->next, fn, http_connection, sock, buf, cchbuf) != CORE_NO_ERROR)
        {
            CGI_LIB_JSON_WRITE("null");
        }

        node = node->next->next;

        if (node && buf[0] != ',')
        {
            SYSFUN_Strncat(buf, ",", cchbuf - strlen(buf) - 1);
        }
    }

    return CORE_NO_ERROR;
}

static UI32_T CGI_LIB_JSON_ArrayStringify(xmlNodePtr node, int (*fn)(HTTP_Connection_T *http_connection, int sock, const char *text), HTTP_Connection_T *http_connection, int sock, char *buf, size_t cchbuf)
{
    ASSERT(node);
    ASSERT(buf);
    ASSERT(cchbuf > 0);

    CGI_LIB_JSON_WRITE("[");

    CGI_LIB_JSON_ElementsStringify(node->children, fn, http_connection, sock, buf, cchbuf);

    buf[0] = '\0';
    CGI_LIB_JSON_WRITE("]");

    return CORE_NO_ERROR;
}

static UI32_T CGI_LIB_JSON_ElementsStringify(xmlNodePtr node, int (*fn)(HTTP_Connection_T *http_connection, int sock, const char *text), HTTP_Connection_T *http_connection, int sock, char *buf, size_t cchbuf)
{
    ASSERT(buf);
    ASSERT(cchbuf > 0);

    while (node)
    {
        if (CGI_LIB_JSON_ValueStringify(node, fn, http_connection, sock, buf, cchbuf) != CORE_NO_ERROR)
        {
            node = node->next;
            continue;
        }

        node = node->next;

        if (node && buf[0] != ',')
        {
            SYSFUN_Strncat(buf, ",", cchbuf - strlen(buf) - 1);
        }
    }

    return CORE_NO_ERROR;
}

static UI32_T CGI_LIB_JSON_ValueStringify(xmlNodePtr node, int (*fn)(HTTP_Connection_T *http_connection, int sock, const char *text), HTTP_Connection_T *http_connection, int sock, char *buf, size_t cchbuf)
{
    CGI_XML_NODE_TYPE_T node_type;

    ASSERT(node);
    ASSERT(buf);
    ASSERT(cchbuf > 0);

    node_type = CGI_LIB_JSON_NodeTypeOf(node);

    switch (node_type)
    {
        case CGI_XML_NODE_TYPE_STRING:
            return CGI_LIB_JSON_StringStringify(node, fn, http_connection, sock, buf, cchbuf);

        case CGI_XML_NODE_TYPE_NUMBER:
        case CGI_XML_NODE_TYPE_BOOLEAN:
            return CGI_LIB_JSON_NumberStringify(node, fn, http_connection, sock, buf, cchbuf);

        case CGI_XML_NODE_TYPE_OBJECT:
            return CGI_LIB_JSON_ObjectStringify(node, fn, http_connection, sock, buf, cchbuf);

        case CGI_XML_NODE_TYPE_ARRAY:
            return CGI_LIB_JSON_ArrayStringify(node, fn, http_connection, sock, buf, cchbuf);

        default:
            break;
    }

    return CORE_SSI_FAIL;
}

static UI32_T CGI_LIB_JSON_StringStringify(xmlNodePtr node, int (*fn)(HTTP_Connection_T *http_connection, int sock, const char *text), HTTP_Connection_T *http_connection, int sock, char *buf, size_t cchbuf)
{
    char *content;
    char escape_str[CGI_MAX_ESCAPE_STR + 1] = {0};

    ASSERT(node);
    ASSERT(buf);
    ASSERT(cchbuf > 0);

    content = CGI_LIB_JSON_GetXmlContent(node);

    if (!content)
    {
        return CORE_SSI_FAIL;
    }

    cgi_stresc(escape_str, content);

    CGI_LIB_JSON_WRITE("\"");
    CGI_LIB_JSON_WRITE(escape_str);
    CGI_LIB_JSON_WRITE("\"");

    return CORE_NO_ERROR;
}

static UI32_T CGI_LIB_JSON_NumberStringify(xmlNodePtr node, int (*fn)(HTTP_Connection_T *http_connection, int sock, const char *text), HTTP_Connection_T *http_connection, int sock, char *buf, size_t cchbuf)
{
    char *content;

    ASSERT(node);
    ASSERT(buf);
    ASSERT(cchbuf > 0);

    content = CGI_LIB_JSON_GetXmlContent(node);

    if (!content)
    {
        return CORE_SSI_FAIL;
    }

    CGI_LIB_JSON_WRITE(content);

    return CORE_NO_ERROR;
}

static xmlNodePtr CGI_LIB_JSON_FindChildrenNode(xmlNodePtr node, CGI_XML_NODE_TYPE_T type, const char *content)
{
    xmlNodePtr child;

    if (!node || !content)
    {
        return NULL;
    }

    for (child = xmlGetLastChild(node); child; child = child->prev)
    {
        if (CGI_LIB_JSON_IsSameNodeType(child, type) == TRUE)
        {
            const char *child_content = CGI_LIB_JSON_GetXmlContent(child);

            if (child_content && strcmp(child_content, content) == 0) {
                return child;
            }
        }
    }

    return NULL;
}

static xmlNodePtr CGI_LIB_JSON_GetObjectNode(xmlNodePtr node, const char *key)
{
    xmlNodePtr key_node = CGI_LIB_JSON_FindChildrenNode(node, CGI_XML_NODE_TYPE_KEY, key);

    if (!key_node)
    {
        key_node = CGI_LIB_JSON_CreateKeyNode(node, key);
    }

    if (!key_node->next ||
        CGI_LIB_JSON_NodeTypeOf(key_node->next) != CGI_XML_NODE_TYPE_OBJECT)
    {
        xmlNodePtr value_node = CGI_LIB_JSON_Object(NULL);
        xmlAddNextSibling(key_node, value_node);
    }

    return key_node->next;
}

static CGI_CORETYPE_T CGI_LIB_JSON_SetStringValue(xmlNodePtr node, const char *key, const char *value)
{
    xmlNodePtr key_node;
    xmlNodePtr value_node;

    if (!node)
    {
        return CORE_NULL_BUFFER;
    }

    key_node = CGI_LIB_JSON_FindChildrenNode(node, CGI_XML_NODE_TYPE_KEY, key);

    if (key_node)
    {
        if (key_node->next)
        {
            CGI_XML_NODE_TYPE_T key_next_type = CGI_LIB_JSON_NodeTypeOf(key_node->next);
            if (key_next_type != CGI_XML_NODE_TYPE_KEY)
            {
                xmlNodePtr next_node = key_node->next;

                xmlUnlinkNode(next_node);
                xmlFreeNode(next_node);
            }
        }
    }
    else
    {
        key_node = CGI_LIB_JSON_CreateKeyNode(node, key);
    }

    value_node = CGI_LIB_JSON_String(NULL, value);
    xmlAddNextSibling(key_node, value_node);

    return CORE_NO_ERROR;
}

static CGI_CORETYPE_T CGI_LIB_JSON_SetNumberValue(xmlNodePtr node, const char *key, const char *value)
{
    xmlNodePtr key_node;
    xmlNodePtr value_node;

    if (!node)
    {
        return CORE_NULL_BUFFER;
    }

    key_node = CGI_LIB_JSON_FindChildrenNode(node, CGI_XML_NODE_TYPE_KEY, key);

    if (key_node)
    {
        if (key_node->next)
        {
            CGI_XML_NODE_TYPE_T key_next_type = CGI_LIB_JSON_NodeTypeOf(key_node->next);
            if (key_next_type != CGI_XML_NODE_TYPE_KEY)
            {
                xmlNodePtr next_node = key_node->next;

                xmlUnlinkNode(next_node);
                xmlFreeNode(next_node);
            }
        }
    }
    else
    {
        key_node = CGI_LIB_JSON_CreateKeyNode(node, key);
    }

    value_node = CGI_LIB_JSON_Number(NULL, value);
    xmlAddNextSibling(key_node, value_node);

    return CORE_NO_ERROR;
}

static CGI_CORETYPE_T CGI_LIB_JSON_SetBooleanValue(xmlNodePtr node, const char *key, const char *value)
{
    xmlNodePtr key_node;
    xmlNodePtr value_node;

    if (!node)
    {
        return CORE_NULL_BUFFER;
    }

    key_node = CGI_LIB_JSON_FindChildrenNode(node, CGI_XML_NODE_TYPE_KEY, key);

    if (key_node)
    {
        if (key_node->next)
        {
            CGI_XML_NODE_TYPE_T key_next_type = CGI_LIB_JSON_NodeTypeOf(key_node->next);
            if (key_next_type != CGI_XML_NODE_TYPE_KEY)
            {
                xmlNodePtr next_node = key_node->next;

                xmlUnlinkNode(next_node);
                xmlFreeNode(next_node);
            }
        }
    }
    else
    {
        key_node = CGI_LIB_JSON_CreateKeyNode(node, key);
    }

    value_node = CGI_LIB_JSON_CreateBooleanNode(NULL, value);
    xmlAddNextSibling(key_node, value_node);

    return CORE_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


UI32_T CGI_LIB_JSON_Stringify(xmlNodePtr node, int (*fn)(HTTP_Connection_T *http_connection, int sock, const char *text), HTTP_Connection_T *http_connection, int sock)
{
    UI32_T result;
    char buf[10] = {0};

    result = CGI_LIB_JSON_InternalStringify(node, fn, http_connection, sock, buf, sizeof(buf));
    return result;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - CGI_LIB_JSON_ParseString
 *------------------------------------------------------------------------------
 * PURPOSE:  Print string node in JSON form.
 * INPUT:    node           -- XML Node
 *           cch_str        -- Max length of output string
 *           sock           -- Sock.
 * OUTPUT:   str            -- Output string
 * RETURN:   CORE_NO_ERROR  -- Success.
 *           CORE_SSI_FAIL  -- Send text failed.
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
static UI32_T CGI_LIB_JSON_ParseString(xmlNodePtr node, char *str, UI32_T cch_str, HTTP_Connection_T *http_connection, int sock)
{
    char   *content;
    char   escape_str[CGI_MAX_ESCAPE_STR + 1] = {0};

    ASSERT(node);

    content = CGI_LIB_JSON_GetXmlContent(node);

    if (!content)
    {
        return CORE_SSI_FAIL;
    }

    cgi_stresc(escape_str, content);

    CLI_LIB_JSON_WRITE("\"");
    CLI_LIB_JSON_WRITE(escape_str);
    CLI_LIB_JSON_WRITE("\"");

    return CORE_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - CGI_LIB_JSON_ParseValue
 *------------------------------------------------------------------------------
 * PURPOSE:  Print data of each XML node in JSON form.
 * INPUT:    node           -- XML Node
 *           cch_str        -- Max length of output string
 *           sock           -- Sock.
 * OUTPUT:   str            -- Output string
 * RETURN:   CORE_NO_ERROR  -- Success.
 *           CORE_SSI_FAIL  -- Send text failed.
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
static UI32_T CGI_LIB_JSON_ParseValue(xmlNodePtr node, char *str, UI32_T cch_str, HTTP_Connection_T *http_connection, int sock)
{
    xmlNodePtr child;
    CGI_XML_NODE_TYPE_T node_type;

    ASSERT(cch_str > 0);

    if (node == NULL)
    {
        return CORE_SSI_FAIL;
    }

    node_type = CGI_LIB_JSON_NodeTypeOf(node);

    switch (node_type)
    {
        case CGI_XML_NODE_TYPE_OBJECT:
        {
            CLI_LIB_JSON_WRITE("{");

            child = node->children;

            while (child)
            {
                CGI_XML_NODE_TYPE_T first_child_type;
                CGI_XML_NODE_TYPE_T second_child_type;

                first_child_type = CGI_LIB_JSON_NodeTypeOf(child);

                /* The children should be composed by <key, value> pair.
                 * Ignore the malformed node before read it.
                 */
                if (first_child_type != CGI_XML_NODE_TYPE_KEY)
                {
                    child = child->next;
                    continue;
                }

                if (child->next == NULL)
                {
                    break;
                }

                second_child_type = CGI_LIB_JSON_NodeTypeOf(child->next);

                if (second_child_type == CGI_XML_NODE_TYPE_KEY)
                {
                    child = child->next;
                    continue;
                }

                if (second_child_type == CGI_XML_NODE_TYPE_UNKNOWN)
                {
                    child = child->next->next;
                    continue;
                }

                /* If parse failed for a key node, ignore it.
                 */
                if (CGI_LIB_JSON_ParseString(child, str, cch_str, http_connection, sock) != CORE_NO_ERROR)
                {
                    child = child->next;
                    continue;
                }

                CLI_LIB_JSON_WRITE(":");

                /* If parse failed for a value node this time because we can't
                 * undo here (always flush key string). So use a null object to
                 * instead of this error.
                 */
                if (CGI_LIB_JSON_ParseValue(child->next, str, cch_str, http_connection, sock) != CORE_NO_ERROR)
                {
                    CLI_LIB_JSON_WRITE("null");
                }

                child = child->next->next;

                //
                // TODO: Remove the str (not need to pass to next function.
                //       and make sure the comma can be add correctly.
                //
                if (child && str[0] != ',')
                {
                    SYSFUN_Strncat(str, ",", cch_str - strlen(str) - 1);
                }
            }

            str[0] = '\0';
            CLI_LIB_JSON_WRITE("}");
            break;
        }
        case CGI_XML_NODE_TYPE_ARRAY:
        {
            CLI_LIB_JSON_WRITE("[");

            child = node->children;
            while (child)
            {
                if (CGI_LIB_JSON_ParseValue(child, str, cch_str, http_connection, sock) != CORE_NO_ERROR)
                {
                    child = child->next;
                    continue;
                }

                child = child->next;

                if (child && str[0] != ',')
                {
                    SYSFUN_Strncat(str, ",", cch_str - strlen(str) - 1);
                }
            }

            str[0] = '\0';
            CLI_LIB_JSON_WRITE("]");
            break;
        }
        case CGI_XML_NODE_TYPE_STRING:
        {
            return CGI_LIB_JSON_ParseString(node, str, cch_str, http_connection, sock);
        }
        case CGI_XML_NODE_TYPE_NUMBER:
        case CGI_XML_NODE_TYPE_BOOLEAN:
        {
            char *content;

            content = CGI_LIB_JSON_GetXmlContent(node);

            if (!content)
            {
                return CORE_SSI_FAIL;
            }

            CLI_LIB_JSON_WRITE(content);
            break;
        }
        case CGI_XML_NODE_TYPE_KEY:
        default:
        {
            return CORE_SSI_FAIL;
        }
    }

    return CORE_NO_ERROR;
}
