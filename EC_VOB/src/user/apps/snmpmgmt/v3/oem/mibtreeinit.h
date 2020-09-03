#ifndef _MIB_TREE_INIT_H_
#define _MIB_TREE_INIT_H_

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "sys_adpt.h"
#include "leaf_sys.h"

#define __REGISTER_MIB(descr, var, vartype, theoid, theoidlen)                  \
  if (register_mib(descr, (struct variable *) var, sizeof(struct vartype),      \
               sizeof(var)/sizeof(struct vartype),                              \
               theoid, theoidlen) != MIB_REGISTERED_OK )                        \
	DEBUGMSGTL(("register_mib", "%s registration failed\n", descr));

#define _REGISTER_MIB(descr, var, vartype, theoid)                              \
  {                                                                             \
    UI32_T new_oidlen = OID_LENGTH(theoid);                                     \
    init_ReplacePrivateMibRoot(board_privmib_root, board_privmib_root_len,      \
                             theoid, &new_oidlen);                              \
    __REGISTER_MIB(descr, var, vartype, theoid, new_oidlen);                    \
  }

#define _REGISTER_READ_ONLY(descr, handler, theoid) \
  {                                                                             \
    UI32_T new_oidlen = OID_LENGTH(theoid);                                     \
    init_ReplacePrivateMibRoot(board_privmib_root, board_privmib_root_len,      \
                             theoid, &new_oidlen);                              \
    netsnmp_register_read_only_instance(netsnmp_create_handler_registration     \
                                        (descr, handler,                        \
                                         theoid,                                \
                                         new_oidlen,                            \
                                         HANDLER_CAN_RONLY));                   \
  }

#define _REGISTER_RWRITE(descr, handler, theoid, y)                             \
    {                                                                           \
        UI32_T new_oidlen = OID_LENGTH(theoid);                                 \
        init_ReplacePrivateMibRoot(board_privmib_root, board_privmib_root_len,  \
                                 theoid, &new_oidlen);                          \
        netsnmp_register_instance(netsnmp_create_handler_registration           \
                                  (descr,                                       \
                                   handler,                                     \
                                   theoid,                                      \
                                   new_oidlen,                                  \
                                   HANDLER_CAN_RWRITE));                        \
    }

#define __ASSERT_EXPR( val, exp )                                               \
    {                                                                           \
        if ((val) == 0)                                                         \
        {                                                                       \
            printf("%s(%d): %s Assert failed", __FILE__, __LINE__, #exp);       \
        }                                                                       \
    }

#define __INVALID_PARAMETER( exp )   __ASSERT_EXPR( 0, exp )
#define __VALIDATE_RETURN(expr, retexpr)                                    \
    {                                                                       \
        int _Expr_val=!!(expr);                                             \
        if ( !( _Expr_val ) )                                               \
        {                                                                   \
            __INVALID_PARAMETER(#expr);                                     \
            return ( retexpr );                                             \
        }                                                                   \
    }

static void init_PrintOid(const oid *node_oid, UI32_T node_oidlen)
{
    if (node_oid)
    {
        UI32_T i;

        for (i = 0; i < node_oidlen; i++)
        {
            printf("%s%u", (i != 0 ? "." : ""), node_oid[i]);
        }

        fflush(stdout);
    }
}

static BOOL_T init_ReplacePrivateMibRoot(const oid *board_privmib_root, UI32_T board_privmib_root_len, oid *reg_oid, UI32_T *reg_oid_len)
{
    const oid sys_adpt_privmib_root[] = { SYS_ADPT_PRIVATEMIB_OID };
    const UI32_T sys_adpt_privmib_root_len = (sizeof(sys_adpt_privmib_root) / sizeof(sys_adpt_privmib_root[0]));

    UI32_T new_req_oid_len;

    __VALIDATE_RETURN(board_privmib_root != NULL, FALSE);
    __VALIDATE_RETURN(reg_oid != NULL, FALSE);
    __VALIDATE_RETURN(reg_oid_len != NULL, FALSE);
    __VALIDATE_RETURN(sys_adpt_privmib_root_len <= *reg_oid_len, FALSE);

    new_req_oid_len = board_privmib_root_len + (*reg_oid_len - sys_adpt_privmib_root_len);
    if (*reg_oid_len < new_req_oid_len)
    {
        init_PrintOid(reg_oid, *reg_oid_len);
        printf(": No enough space to convert private MIB root(");
        init_PrintOid(board_privmib_root, board_privmib_root_len);
        printf(")\r\n");
        return FALSE;
    }

    /* the len of oid per board should less than SYS_ADPT_PRIVATEMIB_OID
    */
    if (board_privmib_root_len != sys_adpt_privmib_root_len)
    {
        memmove(reg_oid + board_privmib_root_len,
            reg_oid + sys_adpt_privmib_root_len,
            (*reg_oid_len - sys_adpt_privmib_root_len) * sizeof(oid));
    }

    memcpy(reg_oid, board_privmib_root, board_privmib_root_len * sizeof(oid));
    *reg_oid_len = new_req_oid_len;

    return TRUE;
}
#undef __ASSERT_EXPR
#undef __INVALID_PARAMETER
#undef __VALIDATE_RETURN

#endif // _MIB_TREE_INIT_H_

