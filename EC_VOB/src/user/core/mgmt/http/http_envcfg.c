/***********************************************************************/
/*                                                                     */
/*   MODULE: configs\std\envcfg.c                                      */
/*   DATE:   96/04/23                                                  */
/*   PURPOSE:                                                          */
/*                                                                     */
/*---------------------------------------------------------------------*/
/*                                                                     */
/*          Copyright 1991 - 1993, Integrated Systems, Inc.            */
/*                      ALL RIGHTS RESERVED                            */
/*                                                                     */
/*   Permission is hereby granted to licensees of Integrated Systems,  */
/*   Inc. products to use or abstract this computer program for the    */
/*   sole purpose of implementing a product based on Integrated        */
/*   Systems, Inc. products.   No other rights to reproduce, use,      */
/*   or disseminate this computer program, whether in part or in       */
/*   whole, are granted.                                               */
/*                                                                     */
/*   Integrated Systems, Inc. makes no representation or warranties    */
/*   with respect to the performance of this computer program, and     */
/*   specifically disclaims any responsibility for any damages,        */
/*   special or consequential, connected with the use of this program. */
/*                                                                     */
/***********************************************************************/
/*---------------------------------------------------------------------------------
GENERAL DESCRIPTION:
    This is a set of generic routines to work with a linked list of name = value pairs.
    Very effecient memory use.  A few ptrs are statically allocated with the envcfg
    struct, then as variables are added, memory is dynamically allocated for each
    string, and envfield required.  See documentation on how/when to use envcfg.
----------------------------------------------------------------------------------*/

#include "http_loc.h"

#define MAX_FIELD_LEN   HTTP_REQ_POSTSIZE //spk modified 2002.02.20 - need to be same with HTTP_REQ_POSTSIZE

/*---------------------------------------------------------------------------------
Name:   init_env
Description:    Initialization.  Set initial field values.
Return: none
----------------------------------------------------------------------------------*/
void init_env(envcfg_t *envcfg)
{
    envcfg->head = NULL;
    envcfg->tail = NULL;
}

/*---------------------------------------------------------------------------------
Name:  free_env
Description:  Go throught fields and release all memory allocated.  Verify deletion
    of all fields.
Return: 0 on success, -1 on failure
----------------------------------------------------------------------------------*/
int free_env(envcfg_t *envcfg)
{
    envfield *field;
    envfield *tmp;

    field = envcfg->head;
    while (field) {
        tmp = field;
        field = field->next;

        switch (tmp->type) {
            case HTTP_ENVFIELD_VALUE_TYPE_PTR:
            /* By PTR type, the caller shoule free pointer self.
             */
            if (tmp->name) {
                L_MM_Free(tmp->name);
                tmp->name = NULL;
            }
            break;

            default:
            if (tmp->name) {
                L_MM_Free(tmp->name);
                tmp->name = NULL;
            }

            if (tmp->value) {
                L_MM_Free(tmp->value);
                tmp->value = NULL;
            }

            break;
        }
        L_MM_Free (tmp);
    }

    envcfg->head = envcfg->tail = NULL;

    return 0;
}

/*---------------------------------------------------------------------------------
 Name:  set_env
 Description:    Initialize an environmental variable.  Verify size of name and value
 are within limits, then allocate memory for each component.  If variable is already
 defined, an error message is displayed and then returns -1.

 #if 0 (* ZHONG Qiyao, 1998-05-08, cannot allow NULL *)
 NOTE:   You can set the value equal to NULL, but the name must be defined.
 #endif

 Return: 0 on success, error code on failure
 ----------------------------------------------------------------------------------*/
int set_env(envcfg_t *envcfg, const char *name, const char *value)
{
    return set_env_prefix(envcfg, NULL, name, value);
}

/*---------------------------------------------------------------------------------
 Name:  set_env_prefix_ui32
 Description:    Initialize an environmental variable.  Verify size of name and value
 are within limits, then allocate memory for each component.  If variable is already
 defined, an error message is displayed and then returns -1.
 Return: 0 on success, error code on failure
 ----------------------------------------------------------------------------------*/
int set_env_prefix_ui32(envcfg_t *envcfg, const char *prefix, UI32_T name, const char *value)
{
    enum
    {
        LIMIT_MAX_UI32_STR_LEN = 10
    };

    char str_name[LIMIT_MAX_UI32_STR_LEN + 1];

    snprintf(str_name, sizeof(str_name), "%lu", name);
    str_name[sizeof(str_name) - 1] = '\0';

    return set_env_prefix(envcfg, prefix, str_name, value);
}

/*---------------------------------------------------------------------------------
 Name:  set_env_prefix
 Description:    Initialize an environmental variable.  Verify size of name and value
 are within limits, then allocate memory for each component.  If variable is already
 defined, an error message is displayed and then returns -1.

 #if 0 (* ZHONG Qiyao, 1998-05-08, cannot allow NULL *)
 NOTE:   You can set the value equal to NULL, but the name must be defined.
 #endif

 Return: 0 on success, error code on failure
 ----------------------------------------------------------------------------------*/
int set_env_prefix(envcfg_t *envcfg, const char *prefix, const char *name, const char *value)
{
    size_t prefix_len;
    size_t name_len;
    size_t value_len;

    envfield *field;

    ASSERT(name);
    ASSERT(value);

    if (!prefix) {
        prefix = "";
    }

    prefix_len = strlen(prefix);
    name_len = strlen(name);
    value_len = strlen(value);

    if (MAX_FIELD_LEN <= (prefix_len + name_len)) {
        return -1;
    }

    if (MAX_FIELD_LEN <= value_len) {
        return -1;
    }

    field = (envfield *) L_MM_Malloc(sizeof(struct envfield), L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_SET_ENV));
    if(field == NULL) {
        goto fail;
    }

    memset(field, 0, sizeof(struct envfield));

    field->name = (char *) L_MM_Malloc(prefix_len + name_len + 1, L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_SET_ENV));

    if (field->name == NULL) {
        goto fail;
    }

    field->value = (char *) L_MM_Malloc(value_len + 1, L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_SET_ENV));

    if (field->value == NULL) {
        goto fail;
    }

    strncpy(field->name, prefix, prefix_len);
    strncpy(field->name + prefix_len, name, name_len);
    field->name[prefix_len + name_len] = '\0';

    field->type = HTTP_ENVFIELD_VALUE_TYPE_STRING;
    strncpy(field->value, value, value_len);
    field->value[value_len] = '\0';

    /* set pointers to reflect new env variable in linked list */

    if(envcfg->head == NULL) {  /* this must be the first in the list */
        envcfg->head = field;
        field->prev = NULL;
    }
    else {                      /* there is at least one env var defined */
        field->prev = envcfg->tail;
        envcfg->tail->next = field;
    }

    envcfg->tail = field;       /* always set the tail to the new env variable */
    field->next = NULL;         /* always set the new field next ptr to NULL   */

    return 0;

fail:
    if (field && field->value) {
        L_MM_Free(field->value);
        field->value = NULL;
    }

    if (field && field->name) {
        L_MM_Free(field->name);
        field->name = NULL;
    }

    if (field) {
        L_MM_Free(field);
        field = NULL;
    }
    return -1;
}

/*---------------------------------------------------------------------------------
 Name:  get_env
 Description:  Search through environmental variables for givin name.  Get value of
 environmental variable specified in name.
 Return: ptr to value if found, or NULL if not found
 ----------------------------------------------------------------------------------*/
char *get_env(envcfg_t *envcfg, const char *name)
{
    return get_env_prefix(envcfg, NULL, name);
}

/*---------------------------------------------------------------------------------
 Name:  get_env_prefix
 Description:  Search through environmental variables for givin name.  Get value of
 environmental variable specified in name.
 Return: ptr to value if found, or NULL if not found
 ----------------------------------------------------------------------------------*/
char *get_env_prefix(envcfg_t *envcfg, const char *prefix, const char *name)
{
    size_t prefix_len;

    envfield *field;

    ASSERT(name);

    if (!prefix) {
        prefix = "";
    }

    prefix_len = strlen(prefix);

    field = envcfg->head;
    while(field != NULL){
        if (strncmp(field->name, prefix, prefix_len) == 0 &&
            strcmp(field->name + prefix_len, name) == 0) {

            return field->value;    /* found value */
        }
        field = field->next;
    }

    return NULL;  /* not found */
}

char *get_env_prefix_ui32(envcfg_t *envcfg, const char *prefix, UI32_T name)
{
    enum
    {
        LIMIT_MAX_UI32_STR_LEN = 10
    };

    char str_name[LIMIT_MAX_UI32_STR_LEN + 1];

    snprintf(str_name, sizeof(str_name), "%lu", name);
    str_name[sizeof(str_name) - 1] = '\0';

    return get_env_prefix(envcfg, prefix, str_name);
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - unset_env
 *------------------------------------------------------------------------------
 * PURPOSE:  Unset an environmental variable.
 *           Search for the name specified, if found release memory allocated to
 *           hold variable, and then fix linked list.
 * INPUT:    envcfg       -- Environmental configuration.
 *           name         -- Name of the variable.
 * OUTPUT:   None.
 * RETURN:   0            -- Success.
 *           1            -- Undefined type of envfield.
 *           -1           -- Not found key in envcfg.
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
int unset_env(envcfg_t *envcfg, const char *name)
{
    return unset_env_prefix(envcfg, NULL, name);
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - unset_env_prefix
 *------------------------------------------------------------------------------
 * PURPOSE:  Unset an environmental variable.
 *           Search for the name specified, if found release memory allocated to
 *           hold variable, and then fix linked list.
 * INPUT:    envcfg       -- Environmental configuration.
 *           name         -- Name of the variable.
 * OUTPUT:   None.
 * RETURN:   0            -- Success.
 *           1            -- Undefined type of envfield.
 *           -1           -- Not found key in envcfg.
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
int unset_env_prefix(envcfg_t *envcfg, const char *prefix, const char *name)
{
    size_t prefix_len;

    envfield *field;

    ASSERT(name);

    if (!prefix) {
        prefix = "";
    }

    prefix_len = strlen(prefix);

    field = envcfg->head;
    while (field) {
        if (strncmp(field->name, prefix, prefix_len) == 0 &&
            strcmp(field->name + prefix_len, name) == 0) {

            /* adjust ptrs in linked list
             */
            if (field->prev != NULL) {
                field->prev->next = field->next;
            }

            if (field->next != NULL) {
                field->next->prev = field->prev;
            }

            if (envcfg->head == field) {
                envcfg->head = field->next;
            }

            if (envcfg->tail == field) {
                envcfg->tail = field->prev;
            }

            /* release memory
             */
            if (field->type == HTTP_ENVFIELD_VALUE_TYPE_STRING) {

                if (field->name) {
                    L_MM_Free(field->name);
                    field->name = NULL;
                }

                if (field->value) {
                    L_MM_Free(field->value);
                    field->value = NULL;
                }
            }
            else if (field->type == HTTP_ENVFIELD_VALUE_TYPE_PTR) {
                if (field->name) {
                    L_MM_Free(field->name);
                    field->name = NULL;
                }
            }
            else {
                return 1;
            }

            L_MM_Free(field);
            field = NULL;

            return 0;    /* found value */
        }

        field = field->next;
    }

    return -1;
}

/*---------------------------------------------------------------------------------
Name:   print_env
Description:  Go through linked list of environmental variables, and print name
    and value to stdout.  Also print number of environmental variables defined.
Return: none
----------------------------------------------------------------------------------*/
void print_env(envcfg_t *envcfg)
{
envfield *field;

    printf("\nEnvironmental Variables\n");
    field = envcfg->head;
    while(field != NULL){

        printf("%s = %s\n", field->name, field->value);
        field = field->next;
    }

    printf("\n\n");
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - set_env_token
 *------------------------------------------------------------------------------
 * PURPOSE:  Set key and value who both in TOKEN_T type to envcfg.
 * INPUT:    token_name   -- Key of the variable.
 *           token_value  -- Value of the variable.
 * OUTPUT:   envcfg       -- Environmental configuration.
 * RETURN:   0            -- Success.
 *           -1           -- Input data is not correct.
 *           1            -- Allocate memory for envfield failed.
 *           2            -- Allocate memory for envfield->name failed.
 *           3            -- Allocate memory for envfield->value failed.
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
int set_env_token(envcfg_t *envcfg, TOKEN_T *token_name, TOKEN_T *token_value)
{
    envfield *field;

    if (token_name->p == NULL ||
        token_name->len == 0)
    {
        return -1;
    }

    field = (envfield *) L_MM_Malloc (sizeof(struct envfield), L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_SET_ENV_TOKEN));

    if(field == NULL)
    {
        return 1;
    }

    if(token_name->len > 0)
    {
        field->name = (char *) L_MM_Malloc (token_name->len  + 1, L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_SET_ENV_TOKEN));
    }
    else
    {
        field->name = NULL;
    }

    if(field->name == NULL)
    {
        L_MM_Free(field);
        field = NULL;
        return 2;
    }

    field->value = (char *) L_MM_Malloc(token_value->len + 1, L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_SET_ENV_TOKEN));

    if(field->value == NULL)
    {
        L_MM_Free(field->name);
        field->name = NULL;
        L_MM_Free(field);
        field = NULL;
        return 3;
    }

    strncpy(field->name, token_name->p, token_name->len);
    field->name[token_name->len] = '\0';
    strncpy(field->value, token_value->p, token_value->len);
    field->value[token_value->len] = '\0';
    field->type = HTTP_ENVFIELD_VALUE_TYPE_STRING;

    /* set pointers to reflect new env variable in linked list
     */
    if (envcfg->head == NULL)
    {
        /* this must be the first in the list
         */
        envcfg->head = field;
        field->prev = NULL;
    }
    else
    {
        /* there is at least one env var defined
         */
        field->prev = envcfg->tail;
        envcfg->tail->next = field;
    }

    /* always set the tail to the new env variable
     */
    envcfg->tail = field;

    /* always set the new field next ptr to NULL
     */
    field->next = NULL;

    return 0;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - set_env_ptr
 *------------------------------------------------------------------------------
 * PURPOSE:  Set key and value (pointer) of an environmental variable to envcfg.
 * INPUT:    key          -- Key of the variable.
 *           ptr          -- Value (pointer) of the variable.
 * OUTPUT:   envcfg       -- Environmental configuration.
 * RETURN:   0            -- Success.
 *           -1           -- Length of key is 0.
 *           1            -- Allocate memory for envfield failed.
 *           2            -- Allocate memory for envfield->name failed.
 *           3            -- Allocate memory for envfield->ptrValue failed.
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
//
// FIXME: Replace TOKEN type to const char * for key
//
int
set_env_ptr(envcfg_t *envcfg, TOKEN_T *key, void *ptr)
{
    envfield *field;

    /* check length of key
     */
    if (key->len == 0)
    {
        return -1;
    }

    field = (envfield *) L_MM_Malloc(sizeof(envfield), L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_SET_ENV_PTR));
    memset(field, 0, sizeof(envfield));

    if (field == NULL)
    {
        return 1;
    }

    field->name = (char *) L_MM_Malloc(key->len + 1, L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_SET_ENV_PTR));

    if (field->name == NULL)
    {
        L_MM_Free(field);
        field = NULL;
        return 2;
    }

    strncpy(field->name, key->p, key->len);
    field->name[key->len] = '\0';
    field->ptrValue = ptr;
    field->type = HTTP_ENVFIELD_VALUE_TYPE_PTR;

    /* set pointers to reflect new env variable in linked list
     */

    if (envcfg->head == NULL)
    {
        /* this must be the first in the list
         */
        envcfg->head = field;
        field->prev = NULL;
    }
    else
    {
        /* there is at least one env var defined
         */
        field->prev = envcfg->tail;
        envcfg->tail->next = field;
    }

    /* always set the tail to the new env variable
     */
    envcfg->tail = field;

    /* always set the new field next ptr to NULL
     */
    field->next = NULL;

    return 0;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - get_env_ptr
 *------------------------------------------------------------------------------
 * PURPOSE:  Get value (pointer) from envcfg by key.
 * INPUT:    envcfg       -- Environmental configuration.
 *           key          -- Key of the variable.
 * OUTPUT:   None.
 * RETURN:   ptrValue     -- Value (pointer) of the variable.
 *           NULL         -- Not found key in envcfg.
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
void *
get_env_ptr(envcfg_t *envcfg, char *key)
{
    envfield *field;

    field = envcfg->head;

    while (field != NULL)
    {
        if (strcmp(field->name, key) == 0)
        {
            return field->ptrValue;
        }

        field = field->next;
    }
    return NULL;
}
