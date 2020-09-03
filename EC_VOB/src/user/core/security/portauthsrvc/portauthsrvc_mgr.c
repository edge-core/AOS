/*-----------------------------------------------------------------------------
 * Module Name: portauthsrvc_mgr.c
 *-----------------------------------------------------------------------------
 * PURPOSE: This is the body to implement Auto VLAN and QoS Assignment
 *-----------------------------------------------------------------------------
 * NOTES:
 * 1. This portauthsrvc_mgr.c is in charge of Auto VLAN and QoS Assignment.
 * 2. portauthsrvc_mgr.c will not call any upper layer API.
 * 3. portauthsrvc_mgr.c will call API of VLAN to set auto vlans to physical ports.
 * 4. portauthsrvc_mgr.c will call API of Differv to set auto QoS to physical ports.
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    11/08/2004 - Rene Wang, Created
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2004
 *-----------------------------------------------------------------------------
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_NETACCESS == TRUE)
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "portauthsrvc_mgr.h"
#include "l_link_lst.h"
#include "l_sort_lst.h"
#include "l_stdlib.h"
#include "l_md5.h"
#include "portauthsrvc_os.h"
#include "netaccess_backdoor.h"

#if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE)
#include "vlan_om.h"
#endif

#define PORTAUTH_PVID       2
#define PORTAUTH_TAGGED     1
#define PORTAUTH_UNTAGGED   0
#define MAX_VLAN_COUNT      255

#define LOG(fmt, args...) \
    {                                       \
        if(debug()) {printf(fmt, ##args);}  \
    }

typedef struct
{
    UI32_T pvid;
    L_LINK_LST_List_T vlan_lst;

    BOOL_T (*add) ( L_LINK_LST_List_T *list, void *element);
    BOOL_T (*del) ( L_LINK_LST_List_T *list, void *element);
    BOOL_T (*get_1st) ( L_LINK_LST_List_T *list, void *element);
    BOOL_T (*get_next) ( L_LINK_LST_List_T *list, void *element);

    PORTAUTHSRVC_OS_Vlan_Exec_T check;
    PORTAUTHSRVC_OS_Vlan_Exec_T apply;
    PORTAUTHSRVC_OS_Vlan_Exec_T restore;
}PORTAUTHSRVC_MGR_VlanProfile_T;

typedef struct
{
    UI32_T  vlan_id;
    UI8_T   type;
} PORTAUTHSRVC_MGR_Vlan_T;

typedef struct PORTAUTHSRVC_MGR_QosProfile_S
{
    L_LINK_LST_List_T cmd_list;
}PORTAUTHSRVC_MGR_QosProfile_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

static UI32_T portauthsrvc_mgr_dbg_no;

void PORTAUTHSRVC_MGR_InitiateSystemResource()
{
    NETACCESS_BACKDOOR_Register("portauthsrvc_mgr", &portauthsrvc_mgr_dbg_no);

    PORTAUTHSRVC_OS_Init();
}

static BOOL_T debug()
{
    //static flag = 0;

    //return flag;
    return NETACCESS_BACKDOOR_IsOn(portauthsrvc_mgr_dbg_no);
}

static void PORTAUTHSRVC_MGR_StrTrim(char **str)
{
    char *p = *str;

    /* trim left
     */
    while (*p)
    {
        if (*p != ' ' && *p != '\t')
        {
            *str = p;
            break;
        }
        ++p;
    }

    /* trim right
     */
    p = *str;
    while (*p)
    {
        if (*p == ' ' || *p == '\t')
        {
            *p = '\0';
            break;
        }
        ++p;
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PORTAUTHSRVC_MGR_StrCmp
 * ---------------------------------------------------------------------
 * PURPOSE: Compare two string
 * INPUT  : str1_1      -- part 1 of string 1
 *          str1_2      -- part 2 of string 1
 *          str2        -- string 2
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  :
 * ---------------------------------------------------------------------
 */
BOOL_T PORTAUTHSRVC_MGR_StrCmp(const char *str1_1, const char *str1_2, const char *str2)
{
    while (*str1_1 == *str2)
    {
        ++str2;
        if (*str1_1++ == '\0')
        {
            if (*str1_2 == '\0')
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }

        }
    }

    if (*str1_1 != '\0')
    {
        return FALSE;
    }

    while (*str1_2 == *str2++)
    {
        if (*str1_2++ == '\0')
        {
            return TRUE;
        }
    }

    return FALSE;
}

static BOOL_T PORTAUTHSRVC_MGR_Vlan_LLst_Compare(
    void*inlist_element,
    void *input_element
    )
{
    PORTAUTHSRVC_MGR_Vlan_T *a = (PORTAUTHSRVC_MGR_Vlan_T *)inlist_element;
    PORTAUTHSRVC_MGR_Vlan_T *b = (PORTAUTHSRVC_MGR_Vlan_T *)input_element;

    return (memcmp(a, b, sizeof(PORTAUTHSRVC_MGR_Vlan_T)) == 0)?TRUE:FALSE;
}

static int PORTAUTHSRVC_MGR_Vlan_SLst_Compare(
    void*inlist_element,
    void *input_element
    )
{
    PORTAUTHSRVC_MGR_Vlan_T *a = (PORTAUTHSRVC_MGR_Vlan_T *)inlist_element;
    PORTAUTHSRVC_MGR_Vlan_T *b = (PORTAUTHSRVC_MGR_Vlan_T *)input_element;

    return memcmp(a, b, sizeof(PORTAUTHSRVC_MGR_Vlan_T));
}

static BOOL_T PORTAUTHSRVC_MGR_Vlan_LLst_Set(
    L_LINK_LST_List_T *list,
    void *element
    )
{
    PORTAUTHSRVC_MGR_Vlan_T *pvid = (PORTAUTHSRVC_MGR_Vlan_T *)element;

    if (pvid->type == PORTAUTH_PVID)
    {
        PORTAUTHSRVC_MGR_Vlan_T untag;

        memcpy(&untag, pvid, sizeof(untag));
        untag.type = PORTAUTH_UNTAGGED;

        L_LINK_LST_Set(list, &untag, L_LINK_LST_APPEND);
    }

    return L_LINK_LST_Set(list, element, L_LINK_LST_APPEND);
}

BOOL_T PORTAUTHSRVC_MGR_Vlan_HavePvid(PORTAUTHSRVC_MGR_VlanProfile_T *profile)
{
    PORTAUTHSRVC_MGR_Vlan_T vlan;
    BOOL_T rc = profile->get_1st(&profile->vlan_lst, (void*)&vlan);

    while (rc)
    {
        if (vlan.type == PORTAUTH_PVID)
        {
            return TRUE;
        }

        rc = profile->get_next(&profile->vlan_lst, (void*)&vlan);
    }
    return FALSE;
}

BOOL_T PORTAUTHSRVC_MGR_Vlan_HaveOtherPvid(
    PORTAUTHSRVC_MGR_VlanProfile_T *profile,
    PORTAUTHSRVC_MGR_Vlan_T *new_vlan
    )
{
    PORTAUTHSRVC_MGR_Vlan_T vlan;
    BOOL_T rc = profile->get_1st(&profile->vlan_lst, (void*)&vlan);

    if (new_vlan->type != PORTAUTH_PVID)
        return FALSE;

    while (rc)
    {
        if (vlan.type == PORTAUTH_PVID)
        {
            if (vlan.vlan_id != new_vlan->vlan_id)
                return TRUE;

            break;
        }

        rc = profile->get_next(&profile->vlan_lst, (void*)&vlan);
    }
    return FALSE;
}

void PORTAUTHSRVC_MGR_Vlan_AddPvid(PORTAUTHSRVC_MGR_VlanProfile_T *profile)
{
    PORTAUTHSRVC_MGR_Vlan_T vlan;
    BOOL_T rc = profile->get_1st(&profile->vlan_lst, (void*)&vlan);

    while (rc)
    {
        if (vlan.type == PORTAUTH_UNTAGGED)
        {
            vlan.type = PORTAUTH_PVID;
            profile->add(&profile->vlan_lst, (void*)&vlan);
            break;
        }
        rc = profile->get_next(&profile->vlan_lst, (void*)&vlan);
    }
}

/* NOTE: token must be an non-empty string
 */
static BOOL_T PORTAUTHSRVC_MGR_GetVlanEntry(
    char *token,
    PORTAUTHSRVC_MGR_Vlan_T *new_vlan
    )
{
    size_t last = strlen(token) - 1;
    char postfix = token[last];

    switch (postfix)
    {
        case 'u': case 'U':
        new_vlan->type = PORTAUTH_UNTAGGED;

        token[last] = '\0';
        break;

        case 'p': case 'P':
        new_vlan->type = PORTAUTH_PVID;

        token[last] = '\0';
        break;

        case 't': case 'T':
        new_vlan->type = PORTAUTH_TAGGED;

        token[last] = '\0';
        break;

        default:
        /* A VLAN string without a t, u, or p suffix is considered to join
           VLAN with untagged.
         */
        new_vlan->type = PORTAUTH_UNTAGGED;
        break;
    }

    if (!L_STDLIB_StrIsDigit(token))
    {
        LOG("%s not a vlan string\n", __FUNCTION__);
        return FALSE;
    }

    new_vlan->vlan_id = atoi(token);
    return TRUE;
}

static BOOL_T PORTAUTHSRVC_MGR_Vlan_SetVlanList(
    PORTAUTHSRVC_MGR_VlanProfile_T *profile,
    char *token
    )
{
    BOOL_T  rc;

    PORTAUTHSRVC_MGR_Vlan_T new_vlan;

    PORTAUTHSRVC_MGR_StrTrim(&token);
    if (token[0] == '\0')
        return TRUE;

    LOG("%s token=%s\n", __FUNCTION__, token);

    if (!PORTAUTHSRVC_MGR_GetVlanEntry(token, &new_vlan))
        return FALSE;

    /* If there is two or more VLAN string with p suffix in the list,
       it will be take as malformed.
     */
    if (PORTAUTHSRVC_MGR_Vlan_HaveOtherPvid(profile, &new_vlan))
    {
        LOG("%s have too many pvid\n", __FUNCTION__);
        return FALSE;
    }

    rc = profile->add(&profile->vlan_lst, (void*)&new_vlan);
    return rc;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PORTAUTHSRVC_MGR_Vlan_StringToProfile
 * ---------------------------------------------------------------------
 * PURPOSE: This function should Convert and formalize returned VLAN string
 *          to a sort PORTAUTHSRVC_MGR_Vlan_T array.
 * INPUT  : vlan_string_p     -- returned VLAN string.
 *          max_size_of_entry -- max size of the output array.
 * OUTPUT : entry_p           -- a sort PORTAUTHSRVC_MGR_Vlan_T array that
 *                               terminal with the data member vlan_id of
 *                               an element is 0.
 * RETURN : TRUE/FALSE
 * NOTES  :
 * ---------------------------------------------------------------------
 */
static BOOL_T PORTAUTHSRVC_MGR_Vlan_StringToProfile(
    const char *str,
    PORTAUTHSRVC_MGR_VlanProfile_T *profile
    )
{
    char *p;
    size_t endp;
    char copy_str[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1] = {0};

    if (str == NULL || profile == NULL)
        return FALSE;

    strncpy(copy_str, str, SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST);
    p = copy_str;

    endp = strcspn(p, " ");
    while (p[endp] == ' ')
    {
        p[endp] = '\0';

        if (!PORTAUTHSRVC_MGR_Vlan_SetVlanList(profile, p))
        {
            return FALSE;
        }

        p += endp+1;
        endp = strcspn(p, " ");
    }

    if (!PORTAUTHSRVC_MGR_Vlan_SetVlanList(profile, p))
    {
        return FALSE;
    }

    if (!PORTAUTHSRVC_MGR_Vlan_HavePvid(profile))
    {
        PORTAUTHSRVC_MGR_Vlan_AddPvid(profile);
    }

    return TRUE;
}

BOOL_T PORTAUTHSRVC_MGR_Vlan_CreateProfile(
    PORTAUTHSRVC_MGR_VlanProfile_T *profile
    )
{
    PORTAUTHSRVC_OS_Vlan_Command_T *impl;
    memset(profile, 0, sizeof(PORTAUTHSRVC_MGR_VlanProfile_T));

    if (!L_LINK_LST_Create(&profile->vlan_lst,
        4096,
        sizeof(PORTAUTHSRVC_MGR_Vlan_T),
        PORTAUTHSRVC_MGR_Vlan_LLst_Compare,
        FALSE
        ))
    {
        return FALSE;
    }

    impl = PORTAUTHSRVC_OS_Vlan_Command();
    profile->add = PORTAUTHSRVC_MGR_Vlan_LLst_Set;
    profile->del = L_LINK_LST_Delete;
    profile->get_1st = L_LINK_LST_Get_1st;
    profile->get_next= L_LINK_LST_Get_Next;

    profile->check = impl->check;
    profile->apply = impl->commit;
    profile->restore = impl->restore;

    return TRUE;
}

void PORTAUTHSRVC_MGR_Vlan_FreeProfile(PORTAUTHSRVC_MGR_VlanProfile_T *profile)
{
    L_LINK_LST_Destroy((void*)&profile->vlan_lst);
}

static void PORTAUTHSRVC_MGR_Vlan_ProfileToString(
    PORTAUTHSRVC_MGR_VlanProfile_T *profile,
    char formal_str[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST*2+1])
{
    L_SORT_LST_List_T sort_lst;
    PORTAUTHSRVC_MGR_Vlan_T vlan;
    char temp[20] = {0}; /* space + ui32 + char + terminal */
    BOOL_T first = TRUE;
    BOOL_T rc;

    formal_str[0] = 0;

    if (!L_SORT_LST_Create(&sort_lst,
        4096,
        sizeof(PORTAUTHSRVC_MGR_Vlan_T),
        PORTAUTHSRVC_MGR_Vlan_SLst_Compare
        ))
    {
        return;
    }

    rc = profile->get_1st(&profile->vlan_lst, (void*)&vlan);
    while(rc)
    {
        L_SORT_LST_Set(&sort_lst, (void*)&vlan);
        rc = profile->get_next(&profile->vlan_lst, (void*)&vlan);
    }

    rc = L_SORT_LST_Get_1st(&sort_lst, (void*)&vlan);
    while(rc)
    {
        sprintf(temp, "%s%lu%c",
            first?"":" ",
            vlan.vlan_id,
            (vlan.type == PORTAUTH_PVID) ? 'p' :
            (vlan.type == PORTAUTH_TAGGED) ? 't' : 'u'
            );

        strcat(formal_str, temp);

        rc = L_SORT_LST_Get_Next(&sort_lst, (void*)&vlan);

        first = FALSE;
    }

    L_SORT_LST_Delete_All(&sort_lst);

    LOG("%s formal_str=%s\n", __FUNCTION__, formal_str);
}

static void PORTAUTHSRVC_MGR_Vlan_VlanOmLst_Add(
    VLAN_OM_VLIST_T **lst,
    UI32_T vid
    )
{
    VLAN_OM_VLIST_T *new;

    new = malloc(sizeof(VLAN_OM_VLIST_T));
    if (new == 0)
        return;

    new->vlan_id = vid;
    new->next = *lst;

    *lst = new;
}

static void PORTAUTHSRVC_MGR_Vlan_VlanOmLst_Free(VLAN_OM_VLIST_T **lst)
{
    VLAN_OM_VLIST_T *p = *lst;

    while (p)
    {
        VLAN_OM_VLIST_T *t;

        t = p->next;
        free(p);
        p = t;
    }

    *lst = NULL;
}

static void PORTAUTHSRVC_MGR_Vlan_ProfileToVlanOmList(
    PORTAUTHSRVC_MGR_VlanProfile_T *profile,
    UI32_T *pvid,
    VLAN_OM_VLIST_T **tag_lst,
    VLAN_OM_VLIST_T **untag_lst
    )
{
    PORTAUTHSRVC_MGR_Vlan_T vlan;
    BOOL_T have;

    *pvid = 0;

    have = profile->get_1st(&profile->vlan_lst, (void*)&vlan);
    while (have)
    {
        if (vlan.type == PORTAUTH_UNTAGGED)
        {
            PORTAUTHSRVC_MGR_Vlan_VlanOmLst_Add(untag_lst, vlan.vlan_id);
        }
        else if (vlan.type == PORTAUTH_TAGGED)
        {
            PORTAUTHSRVC_MGR_Vlan_VlanOmLst_Add(tag_lst, vlan.vlan_id);
        }
        else if (vlan.type == PORTAUTH_PVID)
        {
            *pvid = vlan.vlan_id;
        }

        have = profile->get_next(&profile->vlan_lst, (void*)&vlan);
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PORTAUTHSRVC_MGR_Vlan_IsValidVlanListString
 * ---------------------------------------------------------------------
 * PURPOSE: Validates the VLAN list string.
 * INPUT  : ifindex -- lport number
 *          str     -- VLAN list string
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T PORTAUTHSRVC_MGR_Vlan_IsValidVlanListString(UI32_T ifindex, const char *str)
{
    UI32_T pvid;
    VLAN_OM_VLIST_T *untag_lst = NULL;
    VLAN_OM_VLIST_T *tag_lst = NULL;
    PORTAUTHSRVC_MGR_VlanProfile_T profile;
    BOOL_T rc;

    PORTAUTHSRVC_MGR_Vlan_CreateProfile(&profile);
    if (!PORTAUTHSRVC_MGR_Vlan_StringToProfile(str, &profile))
    {
        PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
        return FALSE;
    }

    PORTAUTHSRVC_MGR_Vlan_ProfileToVlanOmList(&profile, &pvid, &tag_lst, &untag_lst);

    if (untag_lst == NULL && tag_lst == NULL)
    {
        PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
        return TRUE;
    }

    rc = profile.check(ifindex, pvid, tag_lst, untag_lst);

    PORTAUTHSRVC_MGR_Vlan_VlanOmLst_Free(&untag_lst);
    PORTAUTHSRVC_MGR_Vlan_VlanOmLst_Free(&tag_lst);
    PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
    return rc;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PORTAUTHSRVC_MGR_Vlan_StringToMd5
 * ---------------------------------------------------------------------
 * PURPOSE: Get a MD5 digit from a VLAN list string.
 * INPUT  : str     -- VLAN list string
 * OUTPUT : digit   -- MD5 digit
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T PORTAUTHSRVC_MGR_Vlan_StringToMd5(const char *str, UI8_T digit[16])
{
    PORTAUTHSRVC_MGR_VlanProfile_T profile;
    char *formal_str;

    formal_str = malloc(SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST * 2 + 1);
    if (formal_str == 0)
    {
        return FALSE;
    }

    PORTAUTHSRVC_MGR_Vlan_CreateProfile(&profile);
    if (!PORTAUTHSRVC_MGR_Vlan_StringToProfile(str, &profile))
    {
        free(formal_str);
        PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
        return FALSE;
    }

    PORTAUTHSRVC_MGR_Vlan_ProfileToString(&profile, formal_str);

    L_MD5_MDString(digit, (UI8_T*)formal_str, (UI32_T)strlen(formal_str));

    free(formal_str);
    PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PORTAUTHSRVC_MGR_Vlan_SetToOper
 * ---------------------------------------------------------------------
 * PURPOSE: Applies VLAN list.
 * INPUT  : ifindex -- lport number
 *          str     -- VLAN list string
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  :
 * ---------------------------------------------------------------------
 */
BOOL_T PORTAUTHSRVC_MGR_Vlan_SetToOper(UI32_T ifindex, const char *str)
{
    UI32_T pvid;
    VLAN_OM_VLIST_T *untag_lst = NULL;
    VLAN_OM_VLIST_T *tag_lst = NULL;
    PORTAUTHSRVC_MGR_VlanProfile_T profile;
    BOOL_T rc;

    PORTAUTHSRVC_MGR_Vlan_CreateProfile(&profile);
    if (!PORTAUTHSRVC_MGR_Vlan_StringToProfile(str, &profile))
    {
        PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
        return FALSE;
    }

    PORTAUTHSRVC_MGR_Vlan_ProfileToVlanOmList(&profile, &pvid, &tag_lst, &untag_lst);

    if (untag_lst == NULL && tag_lst == NULL)
    {
        PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
        return TRUE;
    }

    rc = profile.apply(ifindex, pvid, tag_lst, untag_lst);

    PORTAUTHSRVC_MGR_Vlan_VlanOmLst_Free(&untag_lst);
    PORTAUTHSRVC_MGR_Vlan_VlanOmLst_Free(&tag_lst);
    PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
    return rc;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PORTAUTHSRVC_MGR_Vlan_SetToAdmin
 * ---------------------------------------------------------------------
 * PURPOSE: Restores VLAN.
 * INPUT  : ifindex -- lport number
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  :
 * ---------------------------------------------------------------------
 */
BOOL_T PORTAUTHSRVC_MGR_Vlan_SetToAdmin(UI32_T ifindex)
{
    UI32_T pvid = 0;
    VLAN_OM_VLIST_T *untag_lst = NULL;
    VLAN_OM_VLIST_T *tag_lst = NULL;
    PORTAUTHSRVC_MGR_VlanProfile_T profile;
    BOOL_T rc;

    PORTAUTHSRVC_MGR_Vlan_CreateProfile(&profile);
    rc = profile.restore(ifindex, pvid, tag_lst, untag_lst);
    PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
    return rc;
}

static BOOL_T PORTAUTHSRVC_MGR_Qos_LLst_Compare(void*inlist_element, void *input_element)
{
    PORTAUTHSRVC_OS_Qos_Command_T *a = (PORTAUTHSRVC_OS_Qos_Command_T *)inlist_element;
    PORTAUTHSRVC_OS_Qos_Command_T *b = (PORTAUTHSRVC_OS_Qos_Command_T *)input_element;

    if (strcmp(a->cmd_str, b->cmd_str) == 0)
        return TRUE;

    return FALSE;
}

static int PORTAUTHSRVC_MGR_Qos_SLst_Compare(void*inlist_element, void *input_element)
{
    PORTAUTHSRVC_OS_Qos_Command_T *a = (PORTAUTHSRVC_OS_Qos_Command_T *)inlist_element;
    PORTAUTHSRVC_OS_Qos_Command_T *b = (PORTAUTHSRVC_OS_Qos_Command_T *)input_element;

    return strcmp(a->cmd_str, b->cmd_str);
}

static BOOL_T PORTAUTHSRVC_MGR_Qos_LLst_IsExist(L_LINK_LST_List_T *lst, PORTAUTHSRVC_OS_Qos_Command_T *new_cmd)
{
    PORTAUTHSRVC_OS_Qos_Command_T cmd;
    BOOL_T rc = L_LINK_LST_Get_1st(lst, (void*)&cmd);

    while(rc)
    {
        if (PORTAUTHSRVC_MGR_Qos_LLst_Compare((void*)&cmd, (void*)new_cmd))
            return TRUE;

        rc = L_LINK_LST_Get_Next(lst, (void*)&cmd);
    }

    return FALSE;
}

static BOOL_T PORTAUTHSRVC_MGR_Qos_Match(PORTAUTHSRVC_MGR_QosProfile_T *profile_p, const char *key, const char *val)
{
    UI32_T i;

    for (i=0; i<PORTAUTHSRVC_OS_Qos_CommandNumber(); ++i)
    {
        PORTAUTHSRVC_OS_Qos_Command_T *qos = PORTAUTHSRVC_OS_Qos_Command(i);

        if ( (*val == '\0' && TRUE == PORTAUTHSRVC_MGR_StrCmp(key, "", qos->cmd_str)) ||
             (*val != '\0' && TRUE == PORTAUTHSRVC_MGR_StrCmp(key, "=", qos->cmd_str)))
        {
            PORTAUTHSRVC_OS_Qos_Command_T new_cmd = {0};

            LOG("%s found. key=\"%s\" val=\"%s\"\n", __FUNCTION__, key, val);

            new_cmd.cmd_str = qos->cmd_str;
            if (!PORTAUTHSRVC_MGR_Qos_LLst_IsExist(&profile_p->cmd_list, &new_cmd))
            {
                new_cmd.cmd_str = malloc(strlen(qos->cmd_str) + 1);
                if (new_cmd.cmd_str == NULL)
                {
                    break;
                }

                new_cmd.val_str = malloc(strlen(val) + 1);
                if (new_cmd.val_str == NULL)
                {
                    free(new_cmd.cmd_str);
                    break;
                }

                strcpy(new_cmd.cmd_str, qos->cmd_str);
                strcpy(new_cmd.val_str, val);
                new_cmd.check  = qos->check;
                new_cmd.commit = qos->commit;
                new_cmd.restore= qos->restore;

                L_LINK_LST_Set(&profile_p->cmd_list, (void*)&new_cmd, L_LINK_LST_APPEND);
            }
            break;
        }
    }

    return TRUE;
}

static void PORTAUTHSRVC_MGR_Qos_SetProfile(PORTAUTHSRVC_MGR_QosProfile_T *profile, char *str)
{
    size_t eq = strcspn(str, "=");
    char *key;
    char *val;

    key = str;

    if (str[eq] == '=')
    {
        str[eq] = '\0';
        val = str + eq + 1;
    }
    else
    {
        val = str + eq; /* last character */
    }

    PORTAUTHSRVC_MGR_StrTrim(&key);
    PORTAUTHSRVC_MGR_StrTrim(&val);

    LOG("%s profile = \"%s=%s\"\n", __FUNCTION__, key, val);

    PORTAUTHSRVC_MGR_Qos_Match(profile, key, val);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PORTAUTHSRVC_MGR_Qos_GetProfileFromString
 * ---------------------------------------------------------------------
 * PURPOSE: Get the QoS profiles from RADIUS returned string.
 * INPUT  : string      -- input string
 * OUTPUT : profile_p   -- QoS profile
 * RETURN : TRUE/FALSE
 * NOTES  :
 * ---------------------------------------------------------------------
 */
static BOOL_T PORTAUTHSRVC_MGR_Qos_GetProfileFromString(const char *str, PORTAUTHSRVC_MGR_QosProfile_T *profile_p)
{
    char copy_str[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE + 1];

    size_t endp;
    char *p;

    if (NULL == profile_p || NULL == str)
    {
        return FALSE;
    }

    memset(profile_p, 0, sizeof(PORTAUTHSRVC_MGR_QosProfile_T));
    L_LINK_LST_Create(&profile_p->cmd_list,
        PORTAUTHSRVC_OS_Qos_CommandNumber(),
        sizeof(PORTAUTHSRVC_OS_Qos_Command_T),
        PORTAUTHSRVC_MGR_Qos_LLst_Compare,
        TRUE);

    if (str[0] == '\0')
    {
        return TRUE;
    }

    strncpy(copy_str, str, SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE);
    copy_str[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE] = '\0';

    LOG("%s copy_str=\"%s\"\n", __FUNCTION__, copy_str);

    p = copy_str;

    endp = strcspn(p, ";");
    while (p[endp] == ';')
    {
        p[endp] = '\0';

        PORTAUTHSRVC_MGR_Qos_SetProfile(profile_p, p);

        p += endp+1;
        endp = strcspn(p, ";");
    }

    PORTAUTHSRVC_MGR_Qos_SetProfile(profile_p, p);

    return TRUE;
}

static void PORTAUTHSRVC_MGR_Qos_FreeProfile(PORTAUTHSRVC_MGR_QosProfile_T *profile_p)
{
    if (profile_p)
    {
        PORTAUTHSRVC_OS_Qos_Command_T cmd;

        while (L_LINK_LST_Remove_1st(&profile_p->cmd_list, (void*)&cmd))
        {
            if (cmd.cmd_str)
            {
                free(cmd.cmd_str);
                cmd.cmd_str = NULL;
            }

            if (cmd.val_str)
            {
                free(cmd.val_str);
                cmd.val_str = NULL;
            }
        }
        L_LINK_LST_Destroy(&profile_p->cmd_list);
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PORTAUTHSRVC_MGR_Qos_ProfileToString
 * ---------------------------------------------------------------------
 * PURPOSE: Convert the QoS profile to formal string.
 * INPUT  : profile_p   -- QoS profile
 * OUTPUT : formal_str  -- formal string (be sorted)
 * RETURN : None.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
static void PORTAUTHSRVC_MGR_Qos_ProfileToString(
    PORTAUTHSRVC_MGR_QosProfile_T *profile_p,
    char formal_str[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE+1]
    )
{
    formal_str[0] = '\0';

    if (profile_p)
    {
        L_SORT_LST_List_T sort_lst;
        PORTAUTHSRVC_OS_Qos_Command_T cmd;
        BOOL_T rc;

        if (!L_SORT_LST_Create(&sort_lst,
            PORTAUTHSRVC_OS_Qos_CommandNumber(),
            sizeof(PORTAUTHSRVC_OS_Qos_Command_T),
            PORTAUTHSRVC_MGR_Qos_SLst_Compare
            ))
        {
            return;
        }

        /* sort
         */
        rc = L_LINK_LST_Get_1st(&profile_p->cmd_list, (void*)&cmd);
        while(rc)
        {
            L_SORT_LST_Set(&sort_lst, (void*)&cmd);
            rc = L_LINK_LST_Get_Next(&profile_p->cmd_list, (void*)&cmd);
        }

        /* convert to string
         */
        rc = L_SORT_LST_Get_1st(&sort_lst, (void*)&cmd);
        while(rc)
        {
            strcat(formal_str, cmd.cmd_str);
            strcat(formal_str, cmd.val_str);
            strcat(formal_str, ";");

            rc = L_SORT_LST_Get_Next(&sort_lst, (void*)&cmd);
        }
        L_SORT_LST_Delete_All(&sort_lst);

        LOG("%s out=\"%s\"\n", __FUNCTION__, formal_str);
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PORTAUTHSRVC_MGR_Qos_IsValidProfileString
 * ---------------------------------------------------------------------
 * PURPOSE: Validates the QoS profile string.
 * INPUT  : ifindex -- lport number
 *          str     -- QoS profile string
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T PORTAUTHSRVC_MGR_Qos_IsValidProfileString(UI32_T ifindex, const char *str)
{
    PORTAUTHSRVC_MGR_QosProfile_T profile;
    PORTAUTHSRVC_OS_Qos_Command_T cmd;
    BOOL_T rc;
    BOOL_T valid = TRUE;

    if (!PORTAUTHSRVC_MGR_Qos_GetProfileFromString(str, &profile))
        return FALSE;

    rc = L_LINK_LST_Get_1st(&profile.cmd_list, (void*)&cmd);
    while(rc)
    {
        if (!cmd.check(ifindex, cmd.cmd_str, cmd.val_str))
        {
            valid = FALSE;
            break;
        }
        rc = L_LINK_LST_Get_Next(&profile.cmd_list, (void*)&cmd);
    }

    PORTAUTHSRVC_MGR_Qos_FreeProfile(&profile);
    return valid;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PORTAUTHSRVC_MGR_Qos_StringToMd5
 * ---------------------------------------------------------------------
 * PURPOSE: Get a MD5 digit from a QoS profile string.
 * INPUT  : str     -- QoS profile string
 * OUTPUT : digit   -- MD5 digit
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T PORTAUTHSRVC_MGR_Qos_StringToMd5(const char *str, unsigned char digit[16])
{
    PORTAUTHSRVC_MGR_QosProfile_T profile;
    char formal_str[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE + 1];

    if (!PORTAUTHSRVC_MGR_Qos_GetProfileFromString(str, &profile))
        return FALSE;

    PORTAUTHSRVC_MGR_Qos_ProfileToString(&profile, formal_str);

    L_MD5_MDString(digit, (UI8_T*)formal_str, (UI32_T)strlen(formal_str));

    PORTAUTHSRVC_MGR_Qos_FreeProfile(&profile);
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PORTAUTHSRVC_MGR_Qos_SetToOper
 * ---------------------------------------------------------------------
 * PURPOSE: Applies QoS profile.
 * INPUT  : ifindex -- lport number
 *          str     -- QoS profile string
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  :
 * ---------------------------------------------------------------------
 */
BOOL_T PORTAUTHSRVC_MGR_Qos_SetToOper(UI32_T ifindex, const char *str)
{
    PORTAUTHSRVC_MGR_QosProfile_T profile;
    PORTAUTHSRVC_OS_Qos_Command_T cmd;
    BOOL_T rc;

    if (!PORTAUTHSRVC_MGR_Qos_GetProfileFromString(str, &profile))
        return FALSE;

    rc = L_LINK_LST_Get_1st(&profile.cmd_list, (void*)&cmd);
    while(rc)
    {
        if (!cmd.commit(ifindex, cmd.cmd_str, cmd.val_str))
        {
            PORTAUTHSRVC_MGR_Qos_SetToAdmin(ifindex);
            PORTAUTHSRVC_MGR_Qos_FreeProfile(&profile);
            return FALSE;
        }
        rc = L_LINK_LST_Get_Next(&profile.cmd_list, (void*)&cmd);
    }

    PORTAUTHSRVC_MGR_Qos_FreeProfile(&profile);
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PORTAUTHSRVC_MGR_Qos_SetToAdmin
 * ---------------------------------------------------------------------
 * PURPOSE: Restores QoS profile.
 * INPUT  : ifindex -- lport number
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  :
 * ---------------------------------------------------------------------
 */
BOOL_T PORTAUTHSRVC_MGR_Qos_SetToAdmin(UI32_T ifindex)
{
    UI32_T i;

    for (i=0; i<PORTAUTHSRVC_OS_Qos_CommandNumber(); ++i)
    {
        PORTAUTHSRVC_OS_Qos_Command_T *qos = PORTAUTHSRVC_OS_Qos_Command(i);
        qos->restore(ifindex, qos->cmd_str, qos->val_str);
    }

    return TRUE;
}


#define ASSERT_TRUE(x)  {\
    if (!(x)){printf("**check failed** %s:%d\n", __FUNCTION__, __LINE__);}\
}

void PORTAUTHSRVC_MGR_Vlan_Unit_Test()
{
    if (1)
    {
        PORTAUTHSRVC_MGR_VlanProfile_T profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1];
        char except[] =
            "1u 1p 2u 3u"
            ;

        PORTAUTHSRVC_MGR_Vlan_CreateProfile(&profile);

        PORTAUTHSRVC_MGR_Vlan_StringToProfile("1p 1u 2u 3u", &profile);
        PORTAUTHSRVC_MGR_Vlan_ProfileToString(&profile, result);

        ASSERT_TRUE(strcmp(result, except) == 0);

        PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
    }

    if (1)
    {
        PORTAUTHSRVC_MGR_VlanProfile_T profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1];
        char except[] =
            "1u 1p 2u"
            ;

        PORTAUTHSRVC_MGR_Vlan_CreateProfile(&profile);

        PORTAUTHSRVC_MGR_Vlan_StringToProfile(" 1p  1u   2u ", &profile);
        PORTAUTHSRVC_MGR_Vlan_ProfileToString(&profile, result);

        ASSERT_TRUE(strcmp(result, except) == 0);

        PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
    }

    if (1)
    {
        PORTAUTHSRVC_MGR_VlanProfile_T profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1];
        char except[] =
            "1u 1p 2u 3u"
            ;

        PORTAUTHSRVC_MGR_Vlan_CreateProfile(&profile);

        PORTAUTHSRVC_MGR_Vlan_StringToProfile("\t1p\t1u \t2u\t 3", &profile);
        PORTAUTHSRVC_MGR_Vlan_ProfileToString(&profile, result);

        ASSERT_TRUE(strcmp(result, except) == 0);

        PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
    }

    if (1)
    {
        PORTAUTHSRVC_MGR_VlanProfile_T profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1];
        char except[] =
            "1u 2u 2p 3u"
            ;

        PORTAUTHSRVC_MGR_Vlan_CreateProfile(&profile);

        PORTAUTHSRVC_MGR_Vlan_StringToProfile("1u 2p 2u 3u", &profile);
        PORTAUTHSRVC_MGR_Vlan_ProfileToString(&profile, result);

        ASSERT_TRUE(strcmp(result, except) == 0);

        PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
    }

    if (1)
    {
        PORTAUTHSRVC_MGR_VlanProfile_T profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1];
        char except[] =
            "1u 1p"
            ;

        PORTAUTHSRVC_MGR_Vlan_CreateProfile(&profile);

        PORTAUTHSRVC_MGR_Vlan_StringToProfile("1p 1u 1u", &profile);
        PORTAUTHSRVC_MGR_Vlan_ProfileToString(&profile, result);

        ASSERT_TRUE(strcmp(result, except) == 0);

        PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
    }

    if (1)
    {
        PORTAUTHSRVC_MGR_VlanProfile_T profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1];
        char except[] =
            "1u 1t 1p"
            ;

        PORTAUTHSRVC_MGR_Vlan_CreateProfile(&profile);

        PORTAUTHSRVC_MGR_Vlan_StringToProfile("1p 1t 1t", &profile);
        PORTAUTHSRVC_MGR_Vlan_ProfileToString(&profile, result);

        ASSERT_TRUE(strcmp(result, except) == 0);

        PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
    }

    if (1)
    {
        PORTAUTHSRVC_MGR_VlanProfile_T profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1];
        char except[] =
            "1u 1p"
            ;

        PORTAUTHSRVC_MGR_Vlan_CreateProfile(&profile);

        PORTAUTHSRVC_MGR_Vlan_StringToProfile("1p 1p 1u", &profile);
        PORTAUTHSRVC_MGR_Vlan_ProfileToString(&profile, result);

        ASSERT_TRUE(strcmp(result, except) == 0);

        PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
    }

    if (1)
    {
        PORTAUTHSRVC_MGR_VlanProfile_T profile;

        PORTAUTHSRVC_MGR_Vlan_CreateProfile(&profile);

        ASSERT_TRUE(PORTAUTHSRVC_MGR_Vlan_StringToProfile("1p 1p 1u 2p 2u 3u", &profile) == FALSE);

        PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
    }

    if (1)
    {
        PORTAUTHSRVC_MGR_VlanProfile_T profile;

        PORTAUTHSRVC_MGR_Vlan_CreateProfile(&profile);

        ASSERT_TRUE(PORTAUTHSRVC_MGR_Vlan_StringToProfile("1a", &profile) == FALSE);

        PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
    }

    if (1)
    {
        PORTAUTHSRVC_MGR_VlanProfile_T profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1];
        char except[] =
            "1u 1p"
            ;

        PORTAUTHSRVC_MGR_Vlan_CreateProfile(&profile);

        PORTAUTHSRVC_MGR_Vlan_StringToProfile("1", &profile);
        PORTAUTHSRVC_MGR_Vlan_ProfileToString(&profile, result);

        ASSERT_TRUE(strcmp(result, except) == 0);

        PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
    }

    if (1)
    {
        PORTAUTHSRVC_MGR_VlanProfile_T profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1];
        char except[] =
            "1u 1p"
            ;

        PORTAUTHSRVC_MGR_Vlan_CreateProfile(&profile);

        PORTAUTHSRVC_MGR_Vlan_StringToProfile("1 1u", &profile);
        PORTAUTHSRVC_MGR_Vlan_ProfileToString(&profile, result);

        ASSERT_TRUE(strcmp(result, except) == 0);

        PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
    }

    if (1)
    {
        PORTAUTHSRVC_MGR_VlanProfile_T profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1];
        char except[] =
            "1u 1p"
            ;

        PORTAUTHSRVC_MGR_Vlan_CreateProfile(&profile);

        PORTAUTHSRVC_MGR_Vlan_StringToProfile("1 1p", &profile);
        PORTAUTHSRVC_MGR_Vlan_ProfileToString(&profile, result);

        ASSERT_TRUE(strcmp(result, except) == 0);

        PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
    }

    if (1)
    {
        PORTAUTHSRVC_MGR_VlanProfile_T profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1];
        char except[] =
            "1u 1p"
            ;

        PORTAUTHSRVC_MGR_Vlan_CreateProfile(&profile);

        PORTAUTHSRVC_MGR_Vlan_StringToProfile("1p", &profile);
        PORTAUTHSRVC_MGR_Vlan_ProfileToString(&profile, result);

        ASSERT_TRUE(strcmp(result, except) == 0);

        PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
    }

    if (1)
    {
        PORTAUTHSRVC_MGR_VlanProfile_T profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1];
        char except[] =
            "1u 1p"
            ;

        PORTAUTHSRVC_MGR_Vlan_CreateProfile(&profile);

        PORTAUTHSRVC_MGR_Vlan_StringToProfile("1p 1", &profile);
        PORTAUTHSRVC_MGR_Vlan_ProfileToString(&profile, result);

        ASSERT_TRUE(strcmp(result, except) == 0);

        PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
    }

    if (1)
    {
        PORTAUTHSRVC_MGR_VlanProfile_T profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1];
        char except[] =
            "1t 2u 2p"
            ;

        PORTAUTHSRVC_MGR_Vlan_CreateProfile(&profile);

        PORTAUTHSRVC_MGR_Vlan_StringToProfile("1t 2", &profile);
        PORTAUTHSRVC_MGR_Vlan_ProfileToString(&profile, result);

        ASSERT_TRUE(strcmp(result, except) == 0);

        PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
    }

    if (1)
    {
        PORTAUTHSRVC_MGR_VlanProfile_T profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1];
        char except[] =
            "1t 2t 3t"
            ;

        PORTAUTHSRVC_MGR_Vlan_CreateProfile(&profile);

        PORTAUTHSRVC_MGR_Vlan_StringToProfile("1t 2t 3t", &profile);
        PORTAUTHSRVC_MGR_Vlan_ProfileToString(&profile, result);

        ASSERT_TRUE(strcmp(result, except) == 0);

        PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
    }

    if (1)
    {
        PORTAUTHSRVC_MGR_VlanProfile_T profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1];
        char except[] =
            "1u 2u 3u 3p"
            ;

        PORTAUTHSRVC_MGR_Vlan_CreateProfile(&profile);

        PORTAUTHSRVC_MGR_Vlan_StringToProfile("3 2 1", &profile);
        PORTAUTHSRVC_MGR_Vlan_ProfileToString(&profile, result);

        ASSERT_TRUE(strcmp(result, except) == 0);

        PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
    }

    if (1)
    {
        PORTAUTHSRVC_MGR_VlanProfile_T profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1];
        char except[] =
            "1u 2u 2p 3u 4u"
            ;

        PORTAUTHSRVC_MGR_Vlan_CreateProfile(&profile);

        PORTAUTHSRVC_MGR_Vlan_StringToProfile("2 4 3 1", &profile);
        PORTAUTHSRVC_MGR_Vlan_ProfileToString(&profile, result);

        ASSERT_TRUE(strcmp(result, except) == 0);

        PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
    }

    if (1)
    {
        PORTAUTHSRVC_MGR_VlanProfile_T profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST * 2 + 1];
        char in_str[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1] = {0};
        int i;

        for (i=0; ;++i)
        {
            char temp[20];

            sprintf(temp, "%d ", i);

            if (SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST < strlen(in_str) + strlen(temp))
                break;
            strcat(in_str, temp);
        }

        PORTAUTHSRVC_MGR_Vlan_CreateProfile(&profile);

        PORTAUTHSRVC_MGR_Vlan_StringToProfile(in_str, &profile);
        PORTAUTHSRVC_MGR_Vlan_ProfileToString(&profile, result);
        printf("max length of formal string = %d\n", strlen(result));
        PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
    }

    if (1)
    {
        PORTAUTHSRVC_MGR_VlanProfile_T profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST * 2 + 1];
        char in_str[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1] = {0};
        int i;

        for (i=0; ;++i)
        {
            char temp[20];

            sprintf(temp, "%d", i);

            if (SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST < strlen(in_str) + strlen(temp))
                break;
            strcat(in_str, temp);
        }

        PORTAUTHSRVC_MGR_Vlan_CreateProfile(&profile);

        PORTAUTHSRVC_MGR_Vlan_StringToProfile(in_str, &profile);
        PORTAUTHSRVC_MGR_Vlan_ProfileToString(&profile, result);
        PORTAUTHSRVC_MGR_Vlan_FreeProfile(&profile);
    }


    if (1)
    {
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1];
        char except[] =
            "4 3 2 1"
            ;
        VLAN_OM_VLIST_T *lst = NULL;
        VLAN_OM_VLIST_T *p;
        char temp[10];

        PORTAUTHSRVC_MGR_Vlan_VlanOmLst_Add(&lst, 1);
        PORTAUTHSRVC_MGR_Vlan_VlanOmLst_Add(&lst, 2);
        PORTAUTHSRVC_MGR_Vlan_VlanOmLst_Add(&lst, 3);
        PORTAUTHSRVC_MGR_Vlan_VlanOmLst_Add(&lst, 4);

        result[0] = '\0';
        for (p=lst; p; p = p->next)
        {
            sprintf(temp, "%lu%s", p->vlan_id, (p->next)?" ":"");
            strcat(result, temp);
        }

        ASSERT_TRUE(strcmp(result, except) == 0);

        PORTAUTHSRVC_MGR_Vlan_VlanOmLst_Free(&lst);
        ASSERT_TRUE(strcmp(result, except) == 0);
    }

    if (1)
    {
        ASSERT_TRUE(PORTAUTHSRVC_MGR_Vlan_IsValidVlanListString(
            1,
            "1p 1u 2u"
            ) == TRUE);
    }

    if (1)
    {
        ASSERT_TRUE(PORTAUTHSRVC_MGR_Vlan_IsValidVlanListString(
            1,
            "   "
            ) == TRUE);
        ASSERT_TRUE(PORTAUTHSRVC_MGR_Vlan_IsValidVlanListString(
            1,
            ""
            ) == TRUE);
        ASSERT_TRUE(PORTAUTHSRVC_MGR_Vlan_IsValidVlanListString(
            1,
            "\t"
            ) == TRUE);
    }

    if (1)
    {
        ASSERT_TRUE(PORTAUTHSRVC_MGR_Vlan_SetToOper(
            1,
            "1p 1u 2u"
            ) == TRUE);
    }

    if (1)
    {
        ASSERT_TRUE(PORTAUTHSRVC_MGR_Vlan_SetToAdmin(1) == TRUE);
    }
}

void PORTAUTHSRVC_MGR_Qos_Unit_Test()
{
    if (1){
        PORTAUTHSRVC_MGR_QosProfile_T qos_profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE + 1];
        char except[] =
            ""
            ;

        PORTAUTHSRVC_MGR_Qos_GetProfileFromString(
            "service-policy-in="
            , &qos_profile);
        PORTAUTHSRVC_MGR_Qos_ProfileToString(&qos_profile, result);
        ASSERT_TRUE(strcmp(result, except) == 0);

        PORTAUTHSRVC_MGR_Qos_FreeProfile(&qos_profile);
    }

    if (1){
        PORTAUTHSRVC_MGR_QosProfile_T qos_profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE + 1];
        char except[] =
            "service-policy-in=p1;"
            ;

        PORTAUTHSRVC_MGR_Qos_GetProfileFromString(
            "service-policy-in=;service-policy-in=p1"
            , &qos_profile);
        PORTAUTHSRVC_MGR_Qos_ProfileToString(&qos_profile, result);
        ASSERT_TRUE(strcmp(result, except) == 0);

        PORTAUTHSRVC_MGR_Qos_FreeProfile(&qos_profile);
    }

    if (1){
        PORTAUTHSRVC_MGR_QosProfile_T qos_profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE + 1];
        char except[] =
            "service-policy-in=p1;"
            ;

        PORTAUTHSRVC_MGR_Qos_GetProfileFromString(
            "service-policy-in=p1;service-policy-in=p2"
            , &qos_profile);
        PORTAUTHSRVC_MGR_Qos_ProfileToString(&qos_profile, result);
        ASSERT_TRUE(strcmp(result, except) == 0);

        PORTAUTHSRVC_MGR_Qos_FreeProfile(&qos_profile);
    }

    if(1){
        PORTAUTHSRVC_MGR_QosProfile_T qos_profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE + 1];
        char except[] =
            "ip-access-group-in=a1;"
            "rate-limit-input=100;"
            ;

        PORTAUTHSRVC_MGR_Qos_GetProfileFromString(
            "rate-limit-input=100;"
            "ip-access-group-in=a1;"
            , &qos_profile);
        PORTAUTHSRVC_MGR_Qos_ProfileToString(&qos_profile, result);
        ASSERT_TRUE(strcmp(result, except) == 0);

        PORTAUTHSRVC_MGR_Qos_FreeProfile(&qos_profile);
    }

    if(1){
        PORTAUTHSRVC_MGR_QosProfile_T qos_profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE + 1];
        char except[] =
            "ip-access-group-in=a1;"
            "ipv6-access-group-in=a2;"
            "mac-access-group-in=a3;"
            "rate-limit-input=100;"
            "service-policy-in=p1;"
            "switchport-priority-default=5;"
            ;

        PORTAUTHSRVC_MGR_Qos_GetProfileFromString(
            "service-policy-in=p1;"
            "switchport-priority-default=5;"
            "rate-limit-input=100;"
            "ip-access-group-in=a1;"
            "ipv6-access-group-in=a2;"
            "mac-access-group-in=a3;"
            , &qos_profile);
        PORTAUTHSRVC_MGR_Qos_ProfileToString(&qos_profile, result);
        ASSERT_TRUE(strcmp(result, except) == 0);

        PORTAUTHSRVC_MGR_Qos_FreeProfile(&qos_profile);
    }

    if(1){
        PORTAUTHSRVC_MGR_QosProfile_T qos_profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE + 1];
        char except[] =
            "ip-access-group-in=a1;"
            "rate-limit-input=100;"
            ;

        PORTAUTHSRVC_MGR_Qos_GetProfileFromString(
            "rate-limit-input=100;"
            "ip-access-group-in=a1;"
            , &qos_profile);
        PORTAUTHSRVC_MGR_Qos_ProfileToString(&qos_profile, result);
        ASSERT_TRUE(strcmp(result, except) == 0);

        PORTAUTHSRVC_MGR_Qos_FreeProfile(&qos_profile);
    }

    if(1){
        unsigned char a[16];
        unsigned char b[16];

        PORTAUTHSRVC_MGR_Qos_StringToMd5(
            "service-policy-in=p1;"
            "rate-limit-input=100;"
            , a);

        PORTAUTHSRVC_MGR_Qos_StringToMd5(
            "rate-limit-input=100;"
            "service-policy-in=p1;"
            , b);

        ASSERT_TRUE(memcmp(a, b, sizeof(a)) == 0);
    }


    if(1){
        PORTAUTHSRVC_MGR_QosProfile_T qos_profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE + 1];
        char except[] =
            "service-policy-in=p1;";

        PORTAUTHSRVC_MGR_Qos_GetProfileFromString(
            " service-policy-in =  p1 ;"
            , &qos_profile);
        PORTAUTHSRVC_MGR_Qos_ProfileToString(&qos_profile, result);
        ASSERT_TRUE(strcmp(result, except) == 0);
        PORTAUTHSRVC_MGR_Qos_FreeProfile(&qos_profile);
    }

    if(1){
        PORTAUTHSRVC_MGR_QosProfile_T qos_profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE + 1];
        char except[] =
            "service-policy-in=p1;";

        PORTAUTHSRVC_MGR_Qos_GetProfileFromString(
            "\tservice-policy-in\t=\tp1\t;"
            , &qos_profile);
        PORTAUTHSRVC_MGR_Qos_ProfileToString(&qos_profile, result);
        ASSERT_TRUE(strcmp(result, except) == 0);
        PORTAUTHSRVC_MGR_Qos_FreeProfile(&qos_profile);
    }

    if(1){
        PORTAUTHSRVC_MGR_QosProfile_T qos_profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE + 1];
        char except[] =
            "";

        PORTAUTHSRVC_MGR_Qos_GetProfileFromString(
            "service-policy-in=;"
            , &qos_profile);
        PORTAUTHSRVC_MGR_Qos_ProfileToString(&qos_profile, result);
        ASSERT_TRUE(strcmp(result, except) == 0);
        PORTAUTHSRVC_MGR_Qos_FreeProfile(&qos_profile);
    }

    if(1){
        PORTAUTHSRVC_MGR_QosProfile_T qos_profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE + 1];
        char except[] =
            "shutdown;";

        PORTAUTHSRVC_MGR_Qos_GetProfileFromString(
            "shutdown;"
            , &qos_profile);
        PORTAUTHSRVC_MGR_Qos_ProfileToString(&qos_profile, result);
        ASSERT_TRUE(strcmp(result, except) == 0);
        PORTAUTHSRVC_MGR_Qos_FreeProfile(&qos_profile);
    }

    if(1){
        PORTAUTHSRVC_MGR_QosProfile_T qos_profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE + 1];
        char except[] =
            "";

        PORTAUTHSRVC_MGR_Qos_GetProfileFromString(
            "shutdown=true;"
            , &qos_profile);
        PORTAUTHSRVC_MGR_Qos_ProfileToString(&qos_profile, result);
        ASSERT_TRUE(strcmp(result, except) == 0);
        PORTAUTHSRVC_MGR_Qos_FreeProfile(&qos_profile);
    }

    if(1){
        PORTAUTHSRVC_MGR_QosProfile_T qos_profile;
        char result[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE + 1];
        char except[] =
            "shutdown;";

        PORTAUTHSRVC_MGR_Qos_GetProfileFromString(
            " shutdown  ;"
            , &qos_profile);
        PORTAUTHSRVC_MGR_Qos_ProfileToString(&qos_profile, result);
        ASSERT_TRUE(strcmp(result, except) == 0);
        PORTAUTHSRVC_MGR_Qos_FreeProfile(&qos_profile);
    }

    if(1){

        PORTAUTHSRVC_MGR_Qos_IsValidProfileString(
            1,
            "service-policy-in=p1;"
            "switchport-priority-default=5;"
            "rate-limit-input=100;"
            "ip-access-group-in=a1;"
            "ipv6-access-group-in=a2;"
            "mac-access-group-in=a3;"
            "shutdown;"
            );

        PORTAUTHSRVC_MGR_Qos_SetToOper(
            1,
            "service-policy-in=p1;"
            "switchport-priority-default=5;"
            "rate-limit-input=100;"
            "ip-access-group-in=a1;"
            "ipv6-access-group-in=a2;"
            "mac-access-group-in=a3;"
            "shutdown;"
            );

        PORTAUTHSRVC_MGR_Qos_SetToAdmin(1);
    }
}

void PORTAUTHSRVC_MGR_Unit_Test_Main()
{
    PORTAUTHSRVC_MGR_Vlan_Unit_Test();
    PORTAUTHSRVC_MGR_Qos_Unit_Test();
}

#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
