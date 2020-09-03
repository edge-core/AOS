#if((SYS_CPNT_A3COM_PAE_MIB==TRUE) &&(SYS_CPNT_NETWORK_ACCESS==TRUE))
#include "sys_cpnt.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "snmp_mgr.h"
#include "networkaccess_mgr.h"
#include "leaf_a3comPae.h"
#include "A3comPae.h"

oid             a3ComPaePortTable_variables_oid[] =
    { 1, 3, 6, 1, 4, 1, 43, 10, 49 };
#define a3ComPaeOidNameLength 9
/*
 * variable4 a3ComPaePortTable_variables:
 *   this variable defines function callbacks and type return information
 *   for the  mib section
 */

#define A3COMPAEPORTASSIGNENABLE 1
#define A3COMPAEPORTVLANASSIGNMENT 2
#define A3COMPAEPORTQOSASSIGNMENT 3
struct variable3 a3ComPaePortTable_variables[] = {
    /*
     * magic number        , variable type , ro/rw , callback fn  , L, oidsuffix
     */
    {A3COMPAEPORTASSIGNENABLE, ASN_INTEGER, RWRITE, var_a3ComPaePortTable,
     3, {1, 1, 1}},
    {A3COMPAEPORTVLANASSIGNMENT, ASN_OCTET_STR, RONLY,
     var_a3ComPaePortTable, 3, {1, 1, 2}},
    {A3COMPAEPORTQOSASSIGNMENT, ASN_OCTET_STR, RONLY,
     var_a3ComPaePortTable, 3, {1, 1, 3}},
};

void
init_a3ComPaePortTable(void)
{

    DEBUGMSGTL(("a3ComPaePortTable", "Initializing\n"));
    REGISTER_MIB("a3ComPaePortTable", a3ComPaePortTable_variables,
                 variable3, a3ComPaePortTable_variables_oid);
}

#define a3ComPaePortEntry_INSTANCE_LEN  1

static BOOL_T a3ComPaePortTable_get(int      compc,
                                oid     *compl,
                                UI32_T  *index,
                                NETWORKACCESS_PaePortEntry_T   *data)
{


    if (compc !=a3ComPaePortEntry_INSTANCE_LEN)
    {
        return FALSE;
    }
    *index = (UI32_T)compl[0];
    if (NETWORKACCESS_MGR_GetPaePortEntry(*index,data)!=TRUE)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    } /*End of if */
}

static BOOL_T a3ComPaePortTable_next(int      compc,
                                 oid     *compl,
                                 UI32_T  *index,
                                 NETWORKACCESS_PaePortEntry_T    *data)
{
    oid tmp_compl[a3ComPaePortEntry_INSTANCE_LEN];

    /* Generate the instance of each table entry and find the
    * smallest instance that's larger than compc/compl.
    *
    * Step 1: Verify and extract the input key from "compc" and "compl"
    * Note: The number of input key is defined by "compc".
    *       The key for the specified instance is defined in compl.
    */
    memcpy(tmp_compl, compl, sizeof(tmp_compl));
    SNMP_MGR_ConvertRemainToZero(compc,a3ComPaePortEntry_INSTANCE_LEN, tmp_compl);

    *index = (UI32_T)tmp_compl[0];
    if (compc<a3ComPaePortEntry_INSTANCE_LEN)
    {
        if (NETWORKACCESS_MGR_GetPaePortEntry(*index, data)!=TRUE)
        {
            if (NETWORKACCESS_MGR_GetNextPaePortEntry( index, data)!=TRUE)
            {
                return FALSE;
            }
        }
    }
    else
    {
        if (NETWORKACCESS_MGR_GetNextPaePortEntry( index, data)!=TRUE)
        {
            return FALSE;
        }
    }
    return TRUE;
}

unsigned char  *
var_a3ComPaePortTable(struct variable *vp,
                      oid * name,
                      size_t * length,
                      int exact,
                      size_t * var_len, WriteMethod ** write_method)
{
    /*
     * variables we may use later
     */
    UI32_T compc=0;
    UI32_T idx1;
    oid compl[a3ComPaePortEntry_INSTANCE_LEN];
    oid best_inst[a3ComPaePortEntry_INSTANCE_LEN];
    NETWORKACCESS_PaePortEntry_T  data;

    switch (vp->magic) {
      case A3COMPAEPORTASSIGNENABLE:
        *write_method = write_a3ComPaePortAssignEnable;
        break;
    case A3COMPAEPORTVLANASSIGNMENT:
        *write_method = 0;
        break;
    case A3COMPAEPORTQOSASSIGNMENT:
        *write_method = 0;
        break;
    default:
        *write_method = 0;
        break;
    }

      SNMP_MGR_RetrieveCompl(vp->name, vp->namelen, name, *length, &compc,compl, a3ComPaePortEntry_INSTANCE_LEN);

     /*check compc, retrive compl*/
    if (exact)/*get,set*/
    {
        if (!a3ComPaePortTable_get(compc, compl, &idx1, &data))
            return NULL;
    }
    else/*getnext*/
    {
        if (!a3ComPaePortTable_next(compc, compl, &idx1, &data))
        {
            return NULL;
        }
    }
    memcpy(name, vp->name, vp->namelen*sizeof(oid));
    best_inst[0]=idx1;
    memcpy(name + vp->namelen, best_inst, a3ComPaePortEntry_INSTANCE_LEN*sizeof(oid));
    *length = vp->namelen +a3ComPaePortEntry_INSTANCE_LEN;
    /* default give len = 4, should need to modify in switch case if not equal to 4*/
    *var_len = 4;

    switch (vp->magic) {
      case A3COMPAEPORTASSIGNENABLE:
           long_return = data.pae_port_assign_enable;
           return (u_char*) &long_return;
      case A3COMPAEPORTVLANASSIGNMENT:
      {
        strncpy( return_buf,data.pae_port_vlan_assignment,SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST+1 );
        return_buf[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST]='\0';
        *var_len = strlen(return_buf);
        return (u_char*) return_buf;
      }
      case A3COMPAEPORTQOSASSIGNMENT:
      {
        strncpy( return_buf,data.pae_port_qos_assignment, SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE+1);
        return_buf[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE]='\0';
        *var_len = strlen(return_buf);
        return (u_char*) return_buf;
      }
    default:
        ERROR_MSG("");
    }
    return NULL;
}


int
write_a3ComPaePortAssignEnable(int action,
                               u_char * var_val,
                               u_char var_val_type,
                               size_t var_val_len,
                               u_char * statP, oid * name, size_t name_len)
{
    long value;
    int size;
    UI32_T oid_name_length = a3ComPaeOidNameLength  +3;

    if (name_len!=  a3ComPaePortEntry_INSTANCE_LEN + oid_name_length)
    {
        return SNMP_ERR_WRONGLENGTH;
    }

    switch (action) {

    case RESERVE1:
          if (var_val_type != ASN_INTEGER) {
              return SNMP_ERR_WRONGTYPE;
          }
          if (var_val_len > sizeof(long)) {
              return SNMP_ERR_WRONGLENGTH;
          }
          break;

    case RESERVE2:
        size = var_val_len;
        value = *(long *) var_val;
        switch(value)
        {
            case VAL_a3ComPaePortAssignEnable_true:
            case VAL_a3ComPaePortAssignEnable_false:

                break;
            default:
                return SNMP_ERR_WRONGVALUE;
        }
        break;

    case FREE:

        break;

    case ACTION:
    {
        UI32_T port;
        port = (UI32_T)name[oid_name_length];
        value = * (long *) var_val;
        if (NETWORKACCESS_MGR_SetPaePortAssignEnable( port,value  )!= TRUE)
        {
                  return SNMP_ERR_COMMITFAILED;
        }
     }
        break;

    case UNDO:
        /*
         * Back out any changes made in the ACTION case
         */
        break;

    case COMMIT:
        /*
         * Things are working well, so it's now safe to make the change
         * permanently.  Make sure that anything done here can't fail!
         */
        break;
    }
    return SNMP_ERR_NOERROR;
}
#endif
