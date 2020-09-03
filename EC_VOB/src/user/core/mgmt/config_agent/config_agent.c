#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "config_agent_private.h"

CONFDB_RETURN_CODE_T config_agent_doc_init(const config_agent_parse_context_ptr_t ctx, config_agent_doc_ptr_t doc)
{
    assert(doc);

    memset(doc, 0, sizeof(*doc));
    doc->source.type = ctx->type;

    return CONFDB_SUCCESS;
}

config_agent_doc_ptr_t config_agent_doc_new(const config_agent_parse_context_ptr_t ctx)
{
    config_agent_doc_ptr_t doc = (config_agent_doc_ptr_t)malloc(sizeof(config_agent_doc_t));

    if (doc) {
        config_agent_doc_init(ctx, doc);
    }

    return doc;
}

void config_agent_doc_free(config_agent_doc_ptr_t doc)
{
    if (!doc) {
        return;
    }

    if (doc->json) {
        json_decref(doc->json);
    }

    free(doc);
}

CONFDB_RETURN_CODE_T config_agent_parse_context_init(config_agent_parse_context_ptr_t ctx, config_agent_parse_context_type_t type, const config_agent_parse_options_ptr_t options)
{
    assert(ctx);

    memset(ctx, 0, sizeof(*ctx));
    ctx->type = type;

    if (options) {
        memcpy(&ctx->options, options, sizeof(ctx->options));
    }

    return CONFDB_SUCCESS;
}

config_agent_parse_context_ptr_t config_agent_parse_context_new(config_agent_parse_context_type_t type, const config_agent_parse_options_ptr_t options)
{
    config_agent_parse_context_ptr_t ctx = (config_agent_parse_context_ptr_t)malloc(sizeof(config_agent_context_t));

    if (ctx) {
        config_agent_parse_context_init(ctx, type, options);
    }

    return ctx;
}

void config_agent_parse_context_free(config_agent_parse_context_ptr_t ctx)
{
    if (!ctx) {
        return;
    }

    free(ctx);
}

//config_agent_parse_context_ptr_t config_agent_parse_context_deep_copy(const config_agent_parse_context_ptr_t other)
//{
//    config_agent_parse_context_ptr_t ctx;
//
//    assert(other);
//
//    ctx = config_agent_parse_context_new(other->type);
//
//    if (ctx) {
//        memcpy(ctx, other, sizeof(config_agent_context_t));
//    }
//
//    return ctx;
//}

CONFDB_RETURN_CODE_T config_agent_parse_context_clear_error(config_agent_parse_context_ptr_t ctx)
{
    assert(ctx);

    memset(&ctx->error, 0, sizeof(ctx->error));
    ctx->error.line = -1;
    ctx->error.column = -1;
    ctx->error.position = -1;

    return CONFDB_SUCCESS;
}

CONFDB_RETURN_CODE_T config_agent_parse_context_set_text_error(config_agent_parse_context_ptr_t ctx, const char* text)
{
    uint32_t size = sizeof(ctx->error.text);

    config_agent_parse_context_clear_error(ctx);

    strncpy(ctx->error.text, text, sizeof(ctx->error.text));
    if (ctx->error.text[size - 1] != '\0') {
        ctx->error.text[size - 1] = '\0';
        ctx->error.text[size - 2] = '.';
        ctx->error.text[size - 3] = '.';
        ctx->error.text[size - 4] = '.';
    }

    return CONFDB_SUCCESS;
}

CONFDB_RETURN_CODE_T config_agent_parse_context_set_parse_error(config_agent_parse_context_ptr_t ctx, int line, int column, int position, const char* text)
{
    uint32_t size = sizeof(ctx->error.text);

    config_agent_parse_context_clear_error(ctx);

    ctx->error.line = line;
    ctx->error.column = column;
    ctx->error.position = position;

    strncpy(ctx->error.text, text, sizeof(ctx->error.text));
    if (ctx->error.text[size - 1] != '\0') {
        ctx->error.text[size - 1] = '\0';
        ctx->error.text[size - 2] = '.';
        ctx->error.text[size - 3] = '.';
        ctx->error.text[size - 4] = '.';
    }

    return CONFDB_SUCCESS;
}

#if defined(CONFIG_AGENT_HAVE_INIH)
static int config_agent_ini_parse_callback_fn(void* user, const char* section, const char* name,
    const char* value)
{
    config_agent_doc_ptr_t doc = (config_agent_doc_ptr_t)user;
    json_t* js_sec;
    json_t* js_val;

    assert(doc);

    if (!doc->json) {
        doc->json = json_object();

        if (!doc->json) {
            return 0;
        }
    }

    if (section[0] != '\0') {
        js_sec = json_object_get(doc->json, section);

        if (!js_sec) {
            js_sec = json_object();

            if (!js_sec) {
                return 0;
            }

            json_object_set_new(doc->json, section, js_sec);
        }
    }
    else {
        js_sec = doc->json;
    }

    js_val = json_string(value);
    if (!js_val) {
        return 0;
    }

    json_object_set_new(js_sec, name, js_val);

    return 1;
}

config_agent_doc_ptr_t config_agent_ini_parse_fstream(config_agent_parse_context_ptr_t ctx, FILE* input)
{
    config_agent_doc_ptr_t doc;
    int rc;

    doc = config_agent_doc_new(ctx);

    if (!doc) {
        config_agent_parse_context_set_text_error(ctx, "Out of memory (doc)");
        return doc;
    }

    rc = ini_parse_file(input, config_agent_ini_parse_callback_fn, doc);
    if (rc) {
        config_agent_parse_context_set_text_error(ctx, "Parse failed");
        goto error;
    }

    if (0) {
    error:
        if (doc) {
            config_agent_doc_free(doc);
            doc = NULL;
        }
    }

    return doc;
}

config_agent_parse_context_ptr_t config_agent_ini_parse_context_create(const config_agent_parse_options_ptr_t options)
{
    config_agent_parse_context_ptr_t ctx;

    ctx = config_agent_parse_context_new(CONFIG_AGENT_INI, options);
    if (ctx) {
        ctx->fn.parse_fstream = config_agent_ini_parse_fstream;
    }

    return ctx;
}
#endif // CONFIG_AGENT_HAVE_INIH

#if defined(CONFIG_AGENT_HAVE_LIBXML2)
/* Strip whitespace chars off end of given string, in place. Return s. */
static char* rstrip(char* s)
{
    char* p = s + strlen(s);
    while (p > s && isspace((unsigned char)(*--p)))
        *p = '\0';
    return s;
}

/* Return pointer to first non-whitespace char in given string. */
static char* lskip(const char* s)
{
    while (*s && isspace((unsigned char)(*s)))
        s++;
    return (char*)s;
}

static char* strdup_trim(const char* s)
{
    char* str;

    assert(s);

    str = strdup(s);
    if (str) {
        str = rstrip(str);
        str = lskip(str);
    }

    return str;
}

static char* strcat_new(const char* s1, const char* s2)
{
    char* str;
    size_t len1 = 0;
    size_t len2 = 0;

    len1 = s1 ? strlen(s1) : 0;
    len2 = s1 ? strlen(s2) : 0;

    str = (char*)calloc(len1 + len2 + 1, 1);
    if (str) {
        char* p = str;

        if (s1) {
            memcpy(p, s1, len1);
            p += len1;
        }

        if (s2) {
            memcpy(p, s2, len2);
        }
    }

    return str;
}

static void config_agent_xml_to_json(xmlNode* a_node, json_t* js_node)
{
    xmlNode* cur_node = NULL;

    json_t* node_value;
    json_t* attributes = NULL;
    json_t* child_nodes = NULL;

    node_value = json_object_get(js_node, "nodeValue");

    assert(node_value);

    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {

        if (cur_node->type == XML_ELEMENT_NODE) {
            json_t* js_cur_node;
            json_t* js_cur_attribues = NULL;
            json_t* js_cur_child_nodes = NULL;

            //printf("node type: Element, name: %s\n", cur_node->name);

            if (!child_nodes) {
                child_nodes = json_array();
                json_object_set_new(js_node, "childNodes", child_nodes);
            }

            js_cur_node = json_object();
            json_array_append_new(child_nodes, js_cur_node);

            json_object_set_new(js_cur_node, "nodeName", json_string((char*)cur_node->name));
            json_object_set_new(js_cur_node, "nodeValue", json_string(""));

            xmlAttr* attribute = cur_node->properties;
            while (attribute) {
                xmlChar* value = xmlNodeListGetString(a_node->doc, attribute->children, 1);
                //do something with value

                if (!js_cur_attribues) {
                    js_cur_attribues = json_object();
                    json_object_set_new(js_cur_node, "attributes", js_cur_attribues);
                }

                json_object_set_new(js_cur_attribues, (char*)attribute->name, json_string((char*)value));

                xmlFree(value);
                attribute = attribute->next;
            }

            config_agent_xml_to_json(cur_node->children, js_cur_node);
        }

        if (cur_node->type == XML_TEXT_NODE) {
            const char* orig_str = json_string_value(node_value);

            char* new_text = strdup_trim((char*)cur_node->content);

            char* new_node_value = strcat_new(orig_str, new_text);

            if (new_node_value) {
                // update
                json_string_set(node_value, new_node_value);
                free(new_node_value);
            }

            if (new_text) {
                free(new_text);
            }
        }
    }
}

config_agent_doc_ptr_t config_agent_xml_parse_fstream(config_agent_parse_context_ptr_t ctx, FILE* input)
{
    config_agent_doc_ptr_t doc;
    int fd;

    xmlParserCtxtPtr ctxt = NULL;
    xmlDoc* xml_doc = NULL;

    doc = config_agent_doc_new(ctx);

    if (!doc) {
        config_agent_parse_context_set_text_error(ctx, "Out of memory (doc)");
        return doc;
    }

    fd = fileno(input);

    ctxt = xmlNewParserCtxt();
    if (ctxt == NULL) {
        config_agent_parse_context_set_text_error(ctx, "Out of memory (xml)");
        goto error;
    }

    xml_doc = xmlCtxtReadFd(ctxt, fd, NULL, NULL, XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
    if (!xml_doc) {
        config_agent_parse_context_set_parse_error(ctx, ctxt->lastError.line, ctxt->lastError.int2, -1, ctxt->lastError.message);
        goto error;
    }

    if (xml_doc) {
        xmlNode* root_element = NULL;

        root_element = xmlDocGetRootElement(xml_doc);
        doc->json = json_object();

        if (root_element) {
            json_object_set_new(doc->json, "nodeName", json_string((char*)root_element->name));
            json_object_set_new(doc->json, "nodeValue", json_string(""));

            config_agent_xml_to_json(root_element->children, doc->json);
        }

        xmlFreeDoc(xml_doc);
        xml_doc = NULL;
    }

    if (0) {
    error:
        if (doc) {
            config_agent_doc_free(doc);
            doc = NULL;
        }
    }

    if (ctxt) {
        xmlFreeParserCtxt(ctxt);
    }

    return doc;
}

config_agent_parse_context_ptr_t config_agent_xml_parse_context_create(const config_agent_parse_options_ptr_t options)
{
    config_agent_parse_context_ptr_t ctx;

    ctx = config_agent_parse_context_new(CONFIG_AGENT_XML, options);
    if (ctx) {
        ctx->fn.parse_fstream = config_agent_xml_parse_fstream;
    }

    return ctx;
}
#endif // CONFIG_AGENT_HAVE_LIBXML2

config_agent_doc_ptr_t config_agent_json_parse_string(config_agent_parse_context_ptr_t ctx, const char* str)
{
    config_agent_doc_ptr_t doc;

    doc = config_agent_doc_new(ctx);

    if (!doc) {
        config_agent_parse_context_set_text_error(ctx, "Out of memory (doc)");
        return doc;
    }

#if defined(CONFIG_AGENT_HAVE_LIBUCL)
    {
        struct ucl_parser* parser;
        ssize_t r;
        unsigned char* emitted = NULL;

        parser = ucl_parser_new(UCL_PARSER_DEFAULT);

        r = strlen(str);
        ucl_parser_add_chunk(parser, (unsigned char*)str, r);

        if (ucl_parser_get_error(parser) != NULL) {
            config_agent_parse_context_set_parse_error(ctx, ucl_parser_get_linenum(parser),
                ucl_parser_get_column(parser),
                -1,
                ucl_parser_get_error(parser));
        } else {
            ucl_object_t* obj;
            obj = ucl_parser_get_object(parser);

            emitted = ucl_object_emit(obj, UCL_EMIT_JSON_COMPACT);
            ucl_object_unref(obj);
        }

        ucl_parser_free(parser);

        if (!emitted) {
            goto error;
        }

        {
            json_error_t error;

            doc->json = json_loads((char*)emitted, JSON_DISABLE_EOF_CHECK, &error);
            free(emitted);
            emitted = NULL;

            if (!doc->json) {
                config_agent_parse_context_set_parse_error(ctx, error.line, error.column, error.position, error.text);
                goto error;
            }
        }
    }
#else
    {
        json_error_t error;

        doc->json = json_loads(str, JSON_DISABLE_EOF_CHECK, &error);

        if (!doc->json) {
            config_agent_parse_context_set_parse_error(ctx, error.line, error.column, error.position, error.text);
            goto error;
        }
    }
#endif // CONFIG_AGENT_HAVE_LIBUCL

    if (0) {
    error:
        if (doc) {
            config_agent_doc_free(doc);
            doc = NULL;
        }
    }

    return doc;
}

config_agent_doc_ptr_t config_agent_json_parse_fstream(config_agent_parse_context_ptr_t ctx, FILE* input)
{
    config_agent_doc_ptr_t doc;

    doc = config_agent_doc_new(ctx);

    if (!doc) {
        config_agent_parse_context_set_text_error(ctx, "Out of memory (doc)");
        return doc;
    }

#if defined(CONFIG_AGENT_HAVE_LIBUCL)
    {
        struct ucl_parser* parser;
        //ssize_t r;
        unsigned char* emitted = NULL;
        int fd = fileno(input);

        parser = ucl_parser_new(UCL_PARSER_DEFAULT);

        ucl_parser_add_fd(parser, fd);

        if (ucl_parser_get_error(parser) != NULL) {
            config_agent_parse_context_set_parse_error(ctx, ucl_parser_get_linenum(parser),
                ucl_parser_get_column(parser),
                -1,
                ucl_parser_get_error(parser));
        }
        else {
            ucl_object_t* obj;
            obj = ucl_parser_get_object(parser);

            emitted = ucl_object_emit(obj, UCL_EMIT_JSON_COMPACT);
            ucl_object_unref(obj);
        }

        ucl_parser_free(parser);

        if (!emitted) {
            goto error;
        }

        {
            json_error_t error;

            doc->json = json_loads((char*)emitted, JSON_DISABLE_EOF_CHECK, &error);
            free(emitted);
            emitted = NULL;

            if (!doc->json) {
                config_agent_parse_context_set_parse_error(ctx, error.line, error.column, error.position, error.text);
                goto error;
            }
        }
    }
#else
    {
        json_error_t error;

        doc->json = json_loadf(input, 0, &error);

        if (!doc->json) {
            config_agent_parse_context_set_parse_error(ctx, error.line, error.column, error.position, error.text);
            goto error;
        }
    }
#endif // CONFIG_AGENT_HAVE_LIBUCL

    if (0) {
    error:
        if (doc) {
            config_agent_doc_free(doc);
            doc = NULL;
        }
    }

    return doc;
}

config_agent_parse_context_ptr_t config_agent_json_parse_context_create(const config_agent_parse_options_ptr_t options)
{
    config_agent_parse_context_ptr_t ctx;

    ctx = config_agent_parse_context_new(CONFIG_AGENT_JSON, options);
    if (ctx) {
        ctx->fn.parse_string = config_agent_json_parse_string;
        ctx->fn.parse_fstream = config_agent_json_parse_fstream;
    }

    return ctx;
}

#if defined(CONFIG_AGENT_HAVE_BSON)
config_agent_doc_ptr_t config_agent_bson_read_buffer(config_agent_parse_context_ptr_t ctx, const uint8_t* buf, uint32_t len)
{
    config_agent_doc_ptr_t doc;

    bson_t* bson = NULL;
    char* str = NULL;

    json_error_t error;

    doc = config_agent_doc_new(ctx);

    if (!doc) {
        config_agent_parse_context_set_text_error(ctx, "Out of memory (doc)");
        return doc;
    }

    bson = bson_new_from_data(buf, len);
    if (!bson) {
        config_agent_parse_context_set_text_error(ctx, "Out of memory (bson)");
        goto error;
    }

    {
        size_t _len;
        str = bson_as_json(bson, &_len);
        if (!str) {
            config_agent_parse_context_set_text_error(ctx, "Wrong BSON format");
            goto error;
        }
    }

    doc->json = json_loads(str, 0, &error);
    if (!doc->json) {
        config_agent_parse_context_set_parse_error(ctx, error.line, error.column, error.position, error.text);
        goto error;
    }

    if (0) {
    error:
        if (doc) {
            config_agent_doc_free(doc);
            doc = NULL;
        }
    }

    if (str) {
        bson_free(str);
        str = NULL;
    }

    if (bson) {
        bson_destroy(bson);
        bson = NULL;
    }

    return doc;
}

config_agent_doc_ptr_t config_agent_bson_read_fstream(config_agent_parse_context_ptr_t ctx, FILE* file)
{
    config_agent_doc_ptr_t doc;

    //bson_t *bson = NULL;
    char* str = NULL;

    json_error_t error;

    int fd;

    doc = config_agent_doc_new(ctx);

    if (!doc) {
        config_agent_parse_context_set_text_error(ctx, "Out of memory (doc)");
        return doc;
    }

    fd = fileno(file);

    bson_reader_t* reader = bson_reader_new_from_fd(fd, false);

    const bson_t* b;
    while ((b = bson_reader_read(reader, NULL))) {

        str = bson_as_json(b, NULL);

        if (!str) {
            config_agent_parse_context_set_text_error(ctx, "Wrong BSON format");
            goto error;
        }

        doc->json = json_loads(str, 0, &error);
        if (!doc->json) {
            config_agent_parse_context_set_parse_error(ctx, error.line, error.column, error.position, error.text);
            goto error;
        }

        bson_free(str);
        str = NULL;

        break; // read one bson in a file
    }

    if (0) {
    error:
        if (doc) {
            config_agent_doc_free(doc);
            doc = NULL;
        }
    }

    if (str) {
        bson_free(str);
        str = NULL;
    }

    if (reader) {
        bson_reader_destroy(reader);
    }

    return doc;
}

config_agent_parse_context_ptr_t config_agent_bson_parse_context_create(const config_agent_parse_options_ptr_t options)
{
    config_agent_parse_context_ptr_t ctx;

    ctx = config_agent_parse_context_new(CONFIG_AGENT_BSON, options);
    if (ctx) {
        ctx->fn.parse_buffer = config_agent_bson_read_buffer;
        ctx->fn.parse_fstream = config_agent_bson_read_fstream;
    }

    return ctx;
}
#endif // CONFIG_AGENT_HAVE_BSON

config_agent_parse_context_ptr_t config_agent_parse_context_create(config_agent_parse_context_type_t type, const config_agent_parse_options_ptr_t options)
{
    switch (type) {
#if defined(CONFIG_AGENT_HAVE_INIH)
    case CONFIG_AGENT_INI:
        return config_agent_ini_parse_context_create(options);
#endif // CONFIG_AGENT_HAVE_INIH

#if defined(CONFIG_AGENT_HAVE_LIBXML2)
    case CONFIG_AGENT_XML:
        return config_agent_xml_parse_context_create(options);
#endif // CONFIG_AGENT_HAVE_LIBXML2

    case CONFIG_AGENT_JSON:
        return config_agent_json_parse_context_create(options);

#if defined(CONFIG_AGENT_HAVE_BSON)
    case CONFIG_AGENT_BSON:
        return config_agent_bson_parse_context_create(options);
#endif // CONFIG_AGENT_HAVE_BSON

    default:
        break;
    }

    return NULL;
}

config_agent_doc_ptr_t config_agent_parse_buffer(config_agent_parse_context_ptr_t ctx, const uint8_t* buf, uint32_t buflen)
{
    assert(ctx);

    if (!ctx->fn.parse_buffer) {
        return NULL;
    }

    return ctx->fn.parse_buffer(ctx, buf, buflen);
}

config_agent_doc_ptr_t config_agent_parse_string(config_agent_parse_context_ptr_t ctx, const char* str)
{
    if (!ctx->fn.parse_string) {
        return NULL;
    }

    return ctx->fn.parse_string(ctx, str);
}

config_agent_doc_ptr_t config_agent_parse_fstream(config_agent_parse_context_ptr_t ctx, FILE* input)
{
    assert(ctx);

    if (!ctx->fn.parse_fstream) {
        return NULL;
    }

    return ctx->fn.parse_fstream(ctx, input);
}

config_agent_doc_ptr_t config_agent_parse_file(config_agent_parse_context_ptr_t ctx, const char* path)
{
    FILE* file;
    config_agent_doc_ptr_t doc;

    assert(ctx);
    if (!ctx->fn.parse_fstream) {
        return NULL;
    }

    file = fopen(path, "rb");
    if (!file) {
        return NULL;
    }

    doc = ctx->fn.parse_fstream(ctx, file);
    fclose(file);
    return doc;
}

static const char* config_agent_path_get_file_extension(const char* path)
{
    return strrchr(path, '.');
}

static bool config_agent_path_is_absolute(const char* path)
{
#if defined(CONFIG_AGENT_PLATFORM_WINDOWS)
    return (2 <= strlen(path) && path[1] == ':' && path[2] == '\\');
#else
    return (1 <= strlen(path) && path[0] == '/');
#endif
}

static int config_agent_path_join(char* buf, size_t buflen, const char* p1, const char* p2)
{
    return snprintf(buf, buflen, "%s%s", p1, p2);
}

static config_agent_parse_context_type_t config_agent_file_extension_to_type(const char* ext)
{
#if defined(CONFIG_AGENT_PLATFORM_WINDOWS)
#define strcasecmp(s1, s2) _strcmpi(s1, s2)
#endif

    if (strcasecmp(ext, "ini") == 0) {
        return CONFIG_AGENT_INI;
    }

    if (strcasecmp(ext, "xml") == 0) {
        return CONFIG_AGENT_XML;
    }

    if (strcasecmp(ext, "json") == 0) {
        return CONFIG_AGENT_JSON;
    }

    if (strcasecmp(ext, "bson") == 0) {
        return CONFIG_AGENT_BSON;
    }

    return CONFIG_AGENT_UNKNOWN;
}

#if defined(CONFIG_AGENT_HAVE_LIBUCL)
static ucl_object_t* config_agent_string_to_ucl_object(const char *str)
{
    ucl_object_t *obj = NULL;
    size_t len = strlen(str);

    struct ucl_parser* parser = ucl_parser_new(UCL_PARSER_DEFAULT);
    ucl_parser_add_chunk(parser, (unsigned char*)str, len);

    if (ucl_parser_get_error(parser) != NULL) {
        assert(0);
    }
    else {
        obj = ucl_parser_get_object(parser);
        ucl_parser_free(parser);
    }

    return obj;
}

static ucl_object_t* config_agent_json_to_ucl_object(const json_t *json)
{
    char* str;
    ucl_object_t *obj = NULL;

    str = json_dumps(json, JSON_COMPACT);

    if (str) {
        obj = config_agent_string_to_ucl_object(str);
        free(str);
    }

    return obj;
}
#endif // CONFIG_AGENT_HAVE_LIBUCL

bool config_agent_validate_doc(config_agent_parse_context_ptr_t ctx, config_agent_doc_ptr_t doc)
{
#if defined(CONFIG_AGENT_HAVE_LIBUCL)
    assert(doc && doc->json);

    if (ctx->options.schema) {
        ucl_object_t *obj = config_agent_json_to_ucl_object(doc->json);
        ucl_object_t *schema = config_agent_string_to_ucl_object(ctx->options.schema);

        if (obj && schema) {
            struct ucl_schema_error err;
            memset(&err, 0, sizeof(err));

            ucl_object_validate(schema, obj, &err);

            if (err.code == UCL_SCHEMA_OK) {
                doc->valid = 1;
            }
            else {
                const char *err_obj = ucl_object_key(err.obj);
                char *emitted = (char*) ucl_object_emit(err.obj, UCL_EMIT_JSON_COMPACT);
                size_t emitted_len = strlen(emitted);

                if (50 < emitted_len) {
                    emitted[45] = '.';
                    emitted[46] = '.';
                    emitted[47] = '.';
                    emitted[48] = emitted[emitted_len - 1];
                    emitted[49] = '\0';
                }

                snprintf(doc->validate_msg, sizeof(doc->validate_msg), "%s( %s ) %s", err_obj, emitted, err.msg);
                //strncpy(doc->validate_msg, err.msg, sizeof(doc->validate_msg) - 1);
                doc->validate_msg[sizeof(doc->validate_msg) - 1] = '\0';

                free(emitted);
            }
        }

        ucl_object_unref(obj);
        ucl_object_unref(schema);
    }
#endif

    return doc->valid == 1 ? true : false;
}

config_agent_doc_ptr_t config_agent_read_document(const char* docname, config_agent_parse_options_ptr_t options)
{
    FILE* file;

    config_agent_parse_context_ptr_t ctx = NULL;
    config_agent_parse_context_type_t type = CONFIG_AGENT_UNKNOWN;
    config_agent_doc_ptr_t doc = NULL;

    const char* prefix = CONFIG_AGENT_WORKING_DIR;
    const char* ext;

    char path[MAX_PATH + 1];

    if (config_agent_path_is_absolute(docname)) {
        prefix = "";
    }

    config_agent_path_join(path, sizeof(path), prefix, docname);

    ext = config_agent_path_get_file_extension(path);

    if (ext) {
        ext += 1;
        type = config_agent_file_extension_to_type(ext);
    }

    if (type == CONFIG_AGENT_UNKNOWN) {
        struct {
            const char* ext;
            config_agent_parse_context_type_t type;
        } exts[] = {
            { "ini",
                CONFIG_AGENT_INI },
            { "xml",
                CONFIG_AGENT_XML },
            { "json",
                CONFIG_AGENT_JSON },
            { "bson",
                CONFIG_AGENT_BSON }
        };

        uint32_t i;

        for (i = 0; i < _countof(exts); ++i) {
            char _path[MAX_PATH + 1];

            snprintf(_path, sizeof(_path), "%s.%s", path, exts[i].ext);

            file = fopen(_path, "rb");
            if (file) {
                type = exts[i].type;
                strncpy(path, _path, sizeof(path) - 1);
                path[sizeof(path) - 1] = '\0';
                break;
            }

        }
    } else {
        file = fopen(path, "rb");
    }

    if (!file) {
        return NULL;
    }

    assert(type <= CONFDB_PARSE_CONTEXT_MAX);

    ctx = config_agent_parse_context_create(type, options);
    if (ctx) {
        doc = config_agent_parse_fstream(ctx, file);
        if (doc) {
            strncpy(doc->source.path, path, sizeof(doc->source.path) - 1);
            doc->source.path[sizeof(doc->source.path) - 1] = '\0';
        }

        if (doc && doc->json) {
            config_agent_validate_doc(ctx, doc);

            if (!doc->valid && ctx->options.schema) {
                ;
            }
        }

        config_agent_parse_context_free(ctx);
    }

    fclose(file);

    return doc;
}

const config_agent_get_string_options_t* config_agent_json_default_get_string_options()
{
    static const config_agent_get_string_options_t default_options = {
        NULL,       // default_value
        0,          // min_length
        UINT32_MAX, // max_length
        NULL        // pattern
    };

    return &default_options;
}

#if defined(CONFIG_AGENT_HAVE_PCRE2)
static pcre2_code * config_agent_re_compile(const char *pattern)
{
    pcre2_code *re;
    PCRE2_SPTR _pattern;

    int errornumber;
    PCRE2_SIZE erroroffset;

    _pattern = (PCRE2_SPTR)pattern;

    re = pcre2_compile(_pattern,              /* the pattern */
        PCRE2_ZERO_TERMINATED,/* indicates pattern is zero-terminated */
        0,                    /* default options */
        &errornumber,         /* for error number */
        &erroroffset,         /* for error offset */
        NULL);                /* use default compile context */

    return re;
}

static bool config_agent_re_test(pcre2_code *re, const char *str)
{
    PCRE2_SPTR subject;     /* the appropriate width (8, 16, or 32 bits). */
    size_t subject_length;

    pcre2_match_data *match_data;

    int rc;

    subject = (PCRE2_SPTR)str;
    subject_length = strlen((char *)subject);

    match_data = pcre2_match_data_create_from_pattern(re, NULL);
    rc = pcre2_match(re,                   /* the compiled pattern */
        subject,              /* the subject string */
        subject_length,       /* the length of the subject */
        0,                    /* start at offset 0 in the subject */
        0,                    /* default options */
        match_data,           /* block for storing the result */
        NULL);                /* use default match context */

    pcre2_match_data_free(match_data);   /* Release memory used for the match */

    return (0 < rc) ? true : false;
}
#endif // CONFIG_AGENT_HAVE_PCRE2

const char* config_agent_json_get_string(const json_t* object, const char* key, const config_agent_get_string_options_t* options)
{
    const char *str;
    json_t *val;

    options = options ? options : config_agent_json_default_get_string_options();

    str = options->default_value ? options->default_value : NULL;

    val = json_object_get(object, key);

    if (val && json_is_string(val)) {
        size_t len;

        str = json_string_value(val);
        len = strlen(str);

        if (str && (len < options->min_length)) {
            str = NULL;
        }

        if (str && (options->max_length < len)) {
            str = NULL;
        }

#if defined(CONFIG_AGENT_HAVE_REGEX)
        if (str && options->pattern) {
#if defined(CONFIG_AGENT_HAVE_PCRE2)
            // Use pcre2-posix and pcreposix Will failed in most case
            // at real switch ...
            // Use pcre2 instead of first.
            pcre2_code *re = config_agent_re_compile(options->pattern);

            if (!config_agent_re_test(re, str)) {
                str = NULL;
            }

            pcre2_code_free(re);
#else
            regex_t reg;

            if (regcomp(&reg, options->pattern, REG_EXTENDED | REG_NOSUB) == 0) {
                if (regexec(&reg, str, 0, NULL, 0) != 0) {
                    str = NULL;
                }
                regfree(&reg);
            }
#endif // CONFIG_AGENT_HAVE_PCRE2
        }
#endif // CONFIG_AGENT_HAVE_REGEX
    }

    return str;
}

const config_agent_get_array_options_t* config_agent_json_default_get_array_options()
{
    static const config_agent_get_array_options_t default_options = {
        UINT32_MAX, // max_items
    };

    return &default_options;
}

const json_t* config_agent_json_get_array(const json_t* object, const char* key, const config_agent_get_array_options_t* options)
{
    json_t* ary;

    options = options ? options : config_agent_json_default_get_array_options();

    ary = json_object_get(object, key);
    if (ary && json_is_array(ary)) {
        if (options->max_items < json_array_size(ary)) {
            ary = NULL;
        }
    }

    return ary;
}

