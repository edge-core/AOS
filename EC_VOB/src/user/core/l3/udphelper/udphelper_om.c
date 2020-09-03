/* Module Name:UDPHELPER_OM.C
 * Purpose: To store UDPHELPER DATABASE.
 *
 * Notes:
 *      
 *
 *
 * History:
 *       Date       --  Modifier,   Reason
 *    2009/04/02    --- Lin.Li, Create
 *
 * Copyright(C)      Accton Corporation, 2009.
 */
#include "sys_cpnt.h"
#if (SYS_CPNT_UDP_HELPER == TRUE)

#include <sysfun.h> 
#include "udphelper_om.h"
#include "l_hisam.h"
#include "string.h"
#include "l_dlist.h"
#include "l_sort_lst.h"
#include "l_prefix.h"
#include "sysfun.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "udphelper_type.h"
#include "stdlib.h"
#include "sys_bld.h"

#define CHECK_FLAG(V,F)      ((V) & (F))
#define SET_FLAG(V,F)        (V) = (V) | (F)
#define UNSET_FLAG(V,F)      (V) = (V) & ~(F)
#define FLAG_ISSET(V,F)      (((V) & (F)) == (F))
#define LIST_LOOP(L,V,N) \
  if (L) \
    for ((N) = (L)->head; (N); (N) = (N)->next) \
      if (((V) = (N)->data) != NULL)
        
#define UDPHELPER_OM_DebugPrint(ARGS...)                          \
              do{                                                 \
                  if (udphelper_debug_om_static)                  \
                      printf(ARGS);                               \
              }while(0)
static UI32_T udphelper_debug_om_static = FALSE;
static UDPHELPER_OM_T udphelper_static;

static I32_T UDPHELPER_OM_IfCompare (void *v1, void *v2)
{
    UDPHELPER_OM_Interface_T *ck1 = v1, *ck2 = v2;
    return ck1->ifindex - ck2->ifindex;
}

static I32_T UDPHELPER_OM_HelperCompare (void *v1, void *v2)
{
    L_INET_AddrIp_T *ck1 = (L_INET_AddrIp_T*)v1, *ck2 = (L_INET_AddrIp_T*)v2;
    
    return memcmp( ck1->addr, ck2->addr, sizeof(ck1->addr) );
}

static UDPHELPER_OM_Interface_T *UDPHELPER_OM_IfLookup (UI32_T ifindex)
{
    struct L_listnode *node;
    UDPHELPER_OM_Interface_T *ck;

    if (!udphelper_static.if_list)
        return NULL;
    for (node = L_LISTHEAD (udphelper_static.if_list); node; L_NEXTNODE (node))
    {
        ck = L_GETDATA (node);
        if (ck->ifindex == ifindex)
            return ck;
    }
  
    return NULL;
}
static UI32_T *UDPHELPER_OM_HelperLookup (UDPHELPER_OM_Interface_T *udp_if, L_INET_AddrIp_T helper)
{
    struct L_listnode *node;
    L_INET_AddrIp_T *ck;

    if (!udp_if->helper_list)
        return NULL;
    for (node = L_LISTHEAD (udp_if->helper_list); node; L_NEXTNODE (node))
    {
        ck = L_GETDATA (node);
        if ( !memcmp(ck->addr, helper.addr, sizeof(helper.addr)) )
            return ck;
    }
  
    return NULL;
}
void UDPHELPER_OM_SetDebugStatus(UI32_T status)
{
    udphelper_debug_om_static = status;
    return;
}

/* FUNCTION NAME : UDP_HELPER_OM_Init
 * PURPOSE:Init UDPHELPER_OM_OSPF database, create semaphore
 *
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */      
void UDP_HELPER_OM_Init(void)
{
    UI32_T ret;
    UI32_T loop;
    
    ret = SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO,
                           &udphelper_static.udphelper_semaphore);
    if (ret != SYSFUN_OK)
    {
        printf("UDP_HELPER_OM_Init : Can't create semaphore\n");
    }
    /* Default UDP helper mechanism is disabled */
    udphelper_static.udphelper_status = FALSE;
    udphelper_static.num_helper = 0;
    /* Init if list */
    udphelper_static.if_list = NULL;
    udphelper_static.if_list = L_list_new ();
    if (!udphelper_static.if_list)
    {
        printf("UDP_HELPER_OM_Init : Can't create if list\n");
    }
    udphelper_static.if_list->cmp = UDPHELPER_OM_IfCompare;
    /* Init forward port structure */
    udphelper_static.forward_port.num_port = 0;
    /* Init forward port array */
    for ( loop = 0; loop < SYS_ADPT_UDPHELPER_MAX_FORWARD_PORT; loop ++ )
        udphelper_static.forward_port.forward_port_array[loop] = 0xFFFFFFFF;
    return;
}
/* FUNCTION NAME : UDPHELPER_OM_IfAdd
 * PURPOSE:Add l3 interface to OM.
 *
 *
 * INPUT:
 *      ifindex: the layer3 interface index.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_SUCCESS.
 *      UDPHELPER_TYPE_RESULT_CREATE_IF_LIST_FAIL
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_OM_L3IfCreate(UI32_T ifindex)
{    
    struct L_listnode *node = NULL;
    UDPHELPER_OM_Interface_T *udp_if = NULL;
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(udphelper_static.udphelper_semaphore);
    /* Check if this interface already exists */
    udp_if = UDPHELPER_OM_IfLookup(ifindex);
    if ( udp_if )
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
        return UDPHELPER_TYPE_RESULT_SUCCESS;
    }
    /* Check if the interface list is null */
    if ( !udphelper_static.if_list )
    {
        udphelper_static.if_list = L_list_new ();
        if (!udphelper_static.if_list)
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
            return UDPHELPER_TYPE_RESULT_CREATE_IF_LIST_FAIL;
        }
        udphelper_static.if_list->cmp = UDPHELPER_OM_IfCompare;
    }
    /* Begin to insert */
    udp_if = (UDPHELPER_OM_Interface_T *)malloc(sizeof(UDPHELPER_OM_Interface_T));
    if(udp_if != NULL)
    {
        memset(udp_if, 0, sizeof(UDPHELPER_OM_Interface_T));
        udp_if->ifindex = ifindex;
        node = L_listnode_add_sort (udphelper_static.if_list, udp_if);
        if (node)
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
            return UDPHELPER_TYPE_RESULT_SUCCESS;
        }
        else
            free(udp_if);
    }
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);    
    return UDPHELPER_TYPE_RESULT_MALLOC_IF_FAIL;
}

/* FUNCTION NAME : UDPHELPER_OM_IfDel
 * PURPOSE:Deletel3 interface from OM.
 *
 *
 * INPUT:
 *      ifindex: the layer3 interface index.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_SUCCESS.
 *
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_OM_L3IfDelete(UI32_T ifindex)
{
    UDPHELPER_OM_Interface_T *udp_if = NULL;
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(udphelper_static.udphelper_semaphore);
    /* Check if this interface already exists */
    udp_if = UDPHELPER_OM_IfLookup(ifindex);
    if ( !udp_if )
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
        return UDPHELPER_TYPE_RESULT_SUCCESS;
    }
    /* Begin to Delete */
    L_listnode_delete (udphelper_static.if_list, udp_if);
    free(udp_if);
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
    return UDPHELPER_TYPE_RESULT_SUCCESS;
}

/* FUNCTION NAME : UDPHELPER_OM_RifAdd
 * PURPOSE:Add ip address to OM.
 *
 *
 * INPUT:
 *      ifindex: the layer3 interface index
 *      addr: the ip address
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_IF_NOT_EXIST.
 *      UDPHELPER_TYPE_RESULT_SUCCESS
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_OM_RifCreate(UI32_T ifindex, L_INET_AddrIp_T addr)
{
    UDPHELPER_OM_Interface_T *udp_if = NULL;
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(udphelper_static.udphelper_semaphore);
    /* Check if this interface exists */
    udp_if = UDPHELPER_OM_IfLookup(ifindex);
    if ( !udp_if )
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
        return UDPHELPER_TYPE_RESULT_IF_NOT_EXIST;
    }
    /* Begin to add */
    udp_if->rif = addr;
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
    return UDPHELPER_TYPE_RESULT_SUCCESS;
}

/* FUNCTION NAME : UDPHELPER_OM_RifDel
 * PURPOSE:Delete ip address from OM.
 *
 *
 * INPUT:
 *      ifindex: the layer3 interface index
 *      addr: the ip address
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_IF_NOT_EXIST.
 *      UDPHELPER_TYPE_RESULT_ADDR_NOT_EXIST
 *      UDPHELPER_TYPE_RESULT_SUCCESS
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_OM_RifDelete(UI32_T ifindex, L_INET_AddrIp_T addr)
{
    UDPHELPER_OM_Interface_T *udp_if = NULL;
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(udphelper_static.udphelper_semaphore);
    /* Check if this interface exists */
    udp_if = UDPHELPER_OM_IfLookup(ifindex);
    if ( !udp_if )
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
        return UDPHELPER_TYPE_RESULT_IF_NOT_EXIST;
    }
    /* Check first */
    if ( memcmp(udp_if->rif.addr, addr.addr, sizeof(addr.addr))
         || udp_if->rif.preflen != addr.preflen )
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
        return UDPHELPER_TYPE_RESULT_ADDR_NOT_EXIST;
    }    
    /* Begin to add */
    memset(&udp_if->rif, 0, sizeof(udp_if->rif));
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
    return UDPHELPER_TYPE_RESULT_SUCCESS;
}

/* FUNCTION NAME : UDPHELPER_OM_SetStatus
 * PURPOSE: Enable or disable UDP helper mechanism.
 *
 *
 * INPUT:
 *      status: the status value, TRUE or FALSE
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_SUCCESS.
 *
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_OM_SetStatus(UI32_T status)
{
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(udphelper_static.udphelper_semaphore);
    /* Set status */
    udphelper_static.udphelper_status = status;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
    return UDPHELPER_TYPE_RESULT_SUCCESS;
}

/* FUNCTION NAME : UDPHELPER_OM_GetStatus
 * PURPOSE:Get UDP helper status
 *
 *
 * INPUT:
 *      status: the status value, TRUE or FALSE
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_SUCCESS.
 *
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_OM_GetStatus(UI32_T *status)
{
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(udphelper_static.udphelper_semaphore);
    /* Set status */
    *status = udphelper_static.udphelper_status;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
    return UDPHELPER_TYPE_RESULT_SUCCESS;
}

/* FUNCTION NAME : UDPHELPER_OM_AddForwardPort
 * PURPOSE:Add forward port to OM.
 *
 *
 * INPUT:
 *      port: the port number.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_FORWARD_PORT_FULL.
 *      UDPHELPER_TYPE_RESULT_SUCCESS
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_OM_AddForwardPort(UI32_T port)
{
    BOOL_T rc;
    UI16_T index;
    UI32_T loop;
    UI32_T *array;
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(udphelper_static.udphelper_semaphore);
    /* Check if the port already exists */ 
    array = udphelper_static.forward_port.forward_port_array;
    for ( loop = 0; loop < SYS_ADPT_UDPHELPER_MAX_FORWARD_PORT; loop ++ )
    {
        if (array[loop] == port)
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
            return UDPHELPER_TYPE_RESULT_SUCCESS;

        }
    }
    if ( udphelper_static.forward_port.num_port >= SYS_ADPT_UDPHELPER_MAX_FORWARD_PORT )
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
        return UDPHELPER_TYPE_RESULT_FORWARD_PORT_FULL;
    }

    /* Add port to array */
    array = udphelper_static.forward_port.forward_port_array;
    for ( loop = 0; loop < SYS_ADPT_UDPHELPER_MAX_FORWARD_PORT; loop ++ )
    {
        /* means this slot is idle */
        if (array[loop] == 0xFFFFFFFF)
        {
            array[loop] = port;
            break;
        }
    }
    udphelper_static.forward_port.num_port++;
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
    return UDPHELPER_TYPE_RESULT_SUCCESS;
}

/* FUNCTION NAME : UDPHELPER_OM_DelForwardPort
 * PURPOSE:Delete forward port to OM.
 *
 *
 * INPUT:
 *      port: the port number.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_SUCCESS.
 *
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_OM_DelForwardPort(UI32_T port)
{
    BOOL_T rc;
    UI16_T index;
    UI32_T loop;
    UI32_T *array;
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(udphelper_static.udphelper_semaphore);
    /* Delete port from array */
    array = udphelper_static.forward_port.forward_port_array;
    for ( loop = 0; loop < SYS_ADPT_UDPHELPER_MAX_FORWARD_PORT; loop ++ )
    {
        if (array[loop] == port)
        {
            array[loop] = 0xFFFFFFFF;
            break;
        }
    }
    if ( loop < SYS_ADPT_UDPHELPER_MAX_FORWARD_PORT 
        && udphelper_static.forward_port.num_port > 0 )
        udphelper_static.forward_port.num_port--;
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
    return UDPHELPER_TYPE_RESULT_SUCCESS;
}

/* FUNCTION NAME : UDPHELPER_OM_GetNextForwardPort
 * PURPOSE:Get next forward port from OM.
 *
 *
 * INPUT:
 *      port: the port number.
 *
 * OUTPUT:
 *      port: the port number.
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_SUCCESS.
 *      UDPHELPER_TYPE_RESULT_FAIL
 * NOTES:
 *      .
 */      
UI32_T UDPHELPER_OM_GetNextForwardPort(UI32_T *port)
{
    UI32_T next_port = 0xFFFFFFFF;
    UI32_T loop;
    UI32_T *array = NULL;
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(udphelper_static.udphelper_semaphore);
    if ( udphelper_static.forward_port.num_port == 0 )
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
        return UDPHELPER_TYPE_RESULT_FAIL;
    }
    /* Search port from array */
    array = udphelper_static.forward_port.forward_port_array;
    for ( loop = 0; loop < SYS_ADPT_UDPHELPER_MAX_FORWARD_PORT; loop ++ )
    {
        if (array[loop] == 0xFFFFFFFF)
            continue;
        if( array[loop] < next_port && array[loop] > *port )
            next_port = array[loop];
    }
    if ( next_port != 0xFFFFFFFF )
    {        
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
        *port = next_port;        
        return UDPHELPER_TYPE_RESULT_SUCCESS;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
    return UDPHELPER_TYPE_RESULT_FAIL;
}

/* FUNCTION NAME : UDPHELPER_OM_AddIpHelperAddress
 * PURPOSE:Add ip helper address to OM.
 *
 *
 * INPUT:
 *      ifindex: the layer3 interface index
 *      helper_addr: the ip helper address
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_IF_NOT_EXIST.
 *      UDPHELPER_TYPE_RESULT_HELPER_FULL
 *      UDPHELPER_TYPE_RESULT_CREATE_IF_HELPER_LIST_FAIL
 *      UDPHELPER_TYPE_RESULT_MALLOC_IF_FAIL
 *      UDPHELPER_TYPE_RESULT_SUCCESS
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_OM_AddIpHelperAddress(UI32_T ifindex, L_INET_AddrIp_T helper_addr)
{
    struct L_listnode *node = NULL;
    L_INET_AddrIp_T *helper = NULL;
    UDPHELPER_OM_Interface_T *udp_if = NULL;
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(udphelper_static.udphelper_semaphore);
    /* Check if this interface exists */
    udp_if = UDPHELPER_OM_IfLookup(ifindex);
    if ( !udp_if )
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
        return UDPHELPER_TYPE_RESULT_IF_NOT_EXIST;
    }
    if ( udphelper_static.num_helper >= SYS_ADPT_UDPHELPER_MAX_HELPER )
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
        return UDPHELPER_TYPE_RESULT_HELPER_FULL;
    }    
    /* Check if the helper list is null */
    if ( !udp_if->helper_list )
    {
        udp_if->helper_list = L_list_new ();
        if (!udp_if->helper_list)
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
            return UDPHELPER_TYPE_RESULT_CREATE_IF_HELPER_LIST_FAIL;
        }
        udp_if->helper_list->cmp = UDPHELPER_OM_HelperCompare;
    }
    /* Begin to insert */
    helper = (L_INET_AddrIp_T *)malloc(sizeof(L_INET_AddrIp_T));
    if(helper != NULL)
    {
        *helper = helper_addr;
        node = L_listnode_add_sort (udp_if->helper_list, helper);
        if ( node )
        {
            udphelper_static.num_helper++; 
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
            return UDPHELPER_TYPE_RESULT_SUCCESS;
        }
        else
            free(helper);
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);    
    return UDPHELPER_TYPE_RESULT_MALLOC_IF_FAIL;    
}

/* FUNCTION NAME : UDPHELPER_OM_DelIpHelperAddress
 * PURPOSE:Delete ip helper address to OM.
 *
 *
 * INPUT:
 *      ifindex: the layer3 interface index
 *      helper_addr: the ip helper address
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_IF_NOT_EXIST.
 *      UDPHELPER_TYPE_RESULT_CREATE_IF_HELPER_LIST_FAIL
 *      UDPHELPER_TYPE_RESULT_SUCCESS
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_OM_DelIpHelperAddress(UI32_T ifindex, L_INET_AddrIp_T helper_addr)
{
    L_INET_AddrIp_T *helper = NULL;
    UDPHELPER_OM_Interface_T *udp_if = NULL;
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(udphelper_static.udphelper_semaphore);
    /* Check if this interface exists */
    udp_if = UDPHELPER_OM_IfLookup(ifindex);
    if ( !udp_if )
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
        return UDPHELPER_TYPE_RESULT_IF_NOT_EXIST;
    }
    /* Check if the helper list is null */
    if ( !udp_if->helper_list )
    {
        udp_if->helper_list = L_list_new ();
        if (!udp_if->helper_list)
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
            return UDPHELPER_TYPE_RESULT_CREATE_IF_HELPER_LIST_FAIL;
        }
        udp_if->helper_list->cmp = UDPHELPER_OM_HelperCompare;
    }
    helper = UDPHELPER_OM_HelperLookup(udp_if, helper_addr);
    if ( !helper )
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
        return UDPHELPER_TYPE_RESULT_SUCCESS;
    }        
    L_listnode_delete (udp_if->helper_list, helper);
    free(helper);
    if ( udphelper_static.num_helper > 0 )
        udphelper_static.num_helper--;
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
    return UDPHELPER_TYPE_RESULT_SUCCESS;    
}

/* FUNCTION NAME : UDPHELPER_OM_GetNextHelper
 * PURPOSE:Get next helper from OM.
 *
 *
 * INPUT:
 *      ifindex: the layer3 interface index
 *      helper_addr: the ip helper address
 *
 * OUTPUT:
 *      helper_addr: the helper address
 *     
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_FAIL.
 *      UDPHELPER_TYPE_RESULT_SUCCESS
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_OM_GetNextHelper(UI32_T ifindex, L_INET_AddrIp_T *helper_addr)
{
    L_INET_AddrIp_T *ck;
    struct L_listnode *node;
    UDPHELPER_OM_Interface_T *udp_if = NULL;
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(udphelper_static.udphelper_semaphore);
    /* Check if this interface exists */
    udp_if = UDPHELPER_OM_IfLookup(ifindex);
    if ( !udp_if )
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
        return UDPHELPER_TYPE_RESULT_FAIL;
    }    
    LIST_LOOP (udp_if->helper_list, ck, node)
    {
        /* Get the first */
        if ( !helper_addr->addr[0] 
            && !helper_addr->addr[1] 
            && !helper_addr->addr[2] 
            && !helper_addr->addr[3] )
        {            
            *helper_addr = *ck;
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
            return UDPHELPER_TYPE_RESULT_SUCCESS;
        }
        /* Get the next */
        if ( memcmp(helper_addr->addr, ck->addr, sizeof(helper_addr->addr)) < 0 )
        {
            *helper_addr = *ck;
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
            return UDPHELPER_TYPE_RESULT_SUCCESS;
        }
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
    return UDPHELPER_TYPE_RESULT_FAIL;
}

/* FUNCTION NAME : UDPHELPER_OM_CheckHelper
 * PURPOSE:Check if we should do some operations for helper.
 *
 *
 * INPUT:
 *      ifindex: the layer3 interface index.
 *      dst_ip: IP destination address of this packet.
 *      port: the destination UDP port number
 * OUTPUT:
 *      output: if it return UDPHELPER_TYPE_RESULT_BROADCAST, the output parameter is the interface's address.
 *                  else if it return UDPHELPER_TYPE_RESULT_DHCP_RELAY_BACK, the output parameter is the 
 *                  index of the interface  to which this packet should be output.
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_FAIL.
 *      UDPHELPER_TYPE_RESULT_BROADCAST
 *      UDPHELPER_TYPE_RESULT_DHCP_RELAY_BACK
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_OM_CheckHelper(UI32_T ifindex, UI32_T dst_ip, UI32_T port,
                                             UI32_T *output)
{
    UI32_T *ck;
    BOOL_T rc;
    UI32_T loop;
    UI32_T *array; 
    UI32_T ip_address;
    UI32_T mask;
    struct L_listnode *node;
    UDPHELPER_OM_Interface_T *udp_if = NULL;    
    UDPHELPER_OM_Interface_T *udp_if_other = NULL;
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(udphelper_static.udphelper_semaphore);
    /* Check the global switch for UDP helper */
    if ( !udphelper_static.udphelper_status )
    {
        UDPHELPER_OM_DebugPrint("status is false.\r\n");
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
        return UDPHELPER_TYPE_RESULT_FAIL;
    }
    /* Check the forward port */
    /* Delete port from array */
    array = udphelper_static.forward_port.forward_port_array;
    for ( loop = 0; loop < SYS_ADPT_UDPHELPER_MAX_FORWARD_PORT; loop ++ )
    {
        if (array[loop] == port)
            break;
    }
    if ( loop >= SYS_ADPT_UDPHELPER_MAX_FORWARD_PORT )
    {
        UDPHELPER_OM_DebugPrint("forward port is disabled: %u.\r\n", port);
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
        return UDPHELPER_TYPE_RESULT_FAIL;
    }        
    /* Check if this interface exists */
    udp_if = UDPHELPER_OM_IfLookup(ifindex);
    if ( !udp_if )
    {
        UDPHELPER_OM_DebugPrint("interface cannot be found: %u.\r\n", ifindex);
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
        return UDPHELPER_TYPE_RESULT_FAIL;
    }    
    memcpy(&ip_address, udp_if->rif.addr, udp_if->rif.addrlen);
    L_PREFIX_MaskLen2Ip(udp_if->rif.preflen, &mask);
    /* It is broadcast packet: all-ones broadcast or subnet boradcast */
    if ( dst_ip == 0xFFFFFFFF 
        || (dst_ip & ~mask) == ~mask )
    {        
        /* Check if there is helper address on this interface */
        LIST_LOOP (udp_if->helper_list, ck, node)
        {            
            UDPHELPER_OM_DebugPrint("found the helper for boradcast UDP packets: %x.\r\n", ip_address);
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
            memcpy(output, udp_if->rif.addr, sizeof(output));
            return UDPHELPER_TYPE_RESULT_BROADCAST;
        }
    }
    else if ( port == UDPHELPER_TYPE_BOOTP_SERVER_PORT )
    {
        UDPHELPER_OM_DebugPrint("Check the dhcp relay back packets.\r\n");
        /* The dst ip address must be one of my interfaces' address */
        LIST_LOOP (udphelper_static.if_list, udp_if_other, node)
        {
            if ( !memcmp(udp_if_other->rif.addr, &dst_ip , sizeof(dst_ip))
                 && udp_if_other->ifindex != ifindex )
            {                
                UDPHELPER_OM_DebugPrint("find the output interface for relay back packets.\r\n");
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);                
                *output = udp_if_other->ifindex;
                return UDPHELPER_TYPE_RESULT_DHCP_RELAY_BACK;
            }
        }
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
    return UDPHELPER_TYPE_RESULT_FAIL;
}

/* FUNCTION NAME : UDPHELPER_OM_GetHelper
 * PURPOSE:Get helper from OM.
 *
 *
 * INPUT:
 *      ifindex: the layer3 interface index
 *      helper_addr: the ip helper address
 *
 * OUTPUT:
 *      helper_addr: the helper address
 *     
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_FAIL.
 *      UDPHELPER_TYPE_RESULT_SUCCESS
 * NOTES:
 *      None.
 */      
UI32_T UDPHELPER_OM_GetHelper(UI32_T ifindex, L_INET_AddrIp_T *helper_addr)
{
    L_INET_AddrIp_T *ck;
    struct L_listnode *node;
    UDPHELPER_OM_Interface_T *udp_if = NULL;
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(udphelper_static.udphelper_semaphore);
    /* Check if this interface exists */
    udp_if = UDPHELPER_OM_IfLookup(ifindex);
    if ( !udp_if )
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
        return UDPHELPER_TYPE_RESULT_FAIL;
    }    
    LIST_LOOP (udp_if->helper_list, ck, node)
    {
        /* Compare */
        if (!memcmp(helper_addr->addr, ck->addr, sizeof(helper_addr->addr)))
        {            
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
            return UDPHELPER_TYPE_RESULT_SUCCESS;
        }
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
    return UDPHELPER_TYPE_RESULT_FAIL;
}

/* FUNCTION NAME : UDPHELPER_OM_GetForwardPort
 * PURPOSE:Get forward port from OM.
 *
 *
 * INPUT:
 *      port: the port number.
 *
 * OUTPUT:
 *      port: the port number.
 * RETURN:
 *      UDPHELPER_TYPE_RESULT_SUCCESS.
 *      UDPHELPER_TYPE_RESULT_FAIL
 * NOTES:
 *      .
 */      
UI32_T UDPHELPER_OM_GetForwardPort(UI32_T port)
{
    UI32_T loop;
    UI32_T *array = NULL;
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(udphelper_static.udphelper_semaphore);
    if ( udphelper_static.forward_port.num_port == 0 )
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
        return UDPHELPER_TYPE_RESULT_FAIL;
    }
    /* Search port from array */
    array = udphelper_static.forward_port.forward_port_array;
    for ( loop = 0; loop < SYS_ADPT_UDPHELPER_MAX_FORWARD_PORT; loop ++ )
    {
        if (array[loop] == 0xFFFFFFFF)
            continue;
        if( array[loop] == port )
        {        
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
            return UDPHELPER_TYPE_RESULT_SUCCESS;
        }
    }    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(udphelper_static.udphelper_semaphore, original_priority);
    return UDPHELPER_TYPE_RESULT_FAIL;
}

#endif

