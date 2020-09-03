/***********************************************************************/
/*                                                                     */
/*   MODULE: include\envcfg.h                                          */
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

#ifndef _HTTP_ENV_H_
#define _HTTP_ENV_H_ 1

#include "sys_type.h"

#if __cplusplus
extern "C" {
#endif

enum
{
    HTTP_ENVFIELD_VALUE_TYPE_STRING = 0,
    HTTP_ENVFIELD_VALUE_TYPE_PTR    = 1
};

struct envfield {
    char *name;                 /* name of environmental variable      */
    int  type;

    union {                     /* value of the environmental variable */
        char *value;
        void *ptrValue;
    };

    struct envfield *next;      /* ptr to next envfield in linked list */
    struct envfield *prev;      /* ptr to previous envfield in linked list */
};

typedef struct envfield envfield;

struct envcfg_t {

	envfield    *head;          /* ptr to first field in linked list */
	envfield    *tail;          /* ptr to last field in linked list  */

#if 0 /* Zhong Qiyao, 1998-11-30 */
	int         field_cnt;      /* number of fields defined          */
#endif

};
typedef struct envcfg_t envcfg_t;

/*
 * Typedef: struct TOKEN_T
 *
 * Description: pointer of token string and its length
 *
 * Fields:
 *  p            - Pointer of token.
 *  len          - Length of token.
 *
 */
typedef struct
{
    char     *p;
    int      len;
} TOKEN_T;

/*-----------------------------------------------------------------------*/
/* Function Prototypes                                                   */
/*-----------------------------------------------------------------------*/
void init_env(envcfg_t *);
int free_env(envcfg_t *);
int set_env(envcfg_t *, const char *, const char *);
int set_env_prefix_ui32(envcfg_t *envcfg, const char *prefix, UI32_T name, const char *value);
int set_env_prefix(envcfg_t *envcfg, const char *prefix, const char *name, const char *value);
int unset_env(envcfg_t *, const char *);
int unset_env_prefix(envcfg_t *envcfg, const char *prefix, const char *name);
char *get_env(envcfg_t *, const char *);
char *get_env_prefix(envcfg_t *envcfg, const char *prefix, const char *name);
char *get_env_prefix_ui32(envcfg_t *envcfg, const char *prefix, UI32_T name);
void print_env(envcfg_t *);
//void send_env(envcfg_t *envcfg, int fd);
int set_env_token(envcfg_t *, TOKEN_T *, TOKEN_T *);
int set_env_ptr(envcfg_t *, TOKEN_T *, void *);
void *get_env_ptr(envcfg_t *, char *);

#if __cplusplus
}
#endif

#endif // _HTTP_ENV_H_
