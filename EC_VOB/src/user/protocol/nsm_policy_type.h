/* MODULE NAME:  nsm_policy_type.h
 * PURPOSE:
 *     Define common types used in NSM POLICY.
 *
 * NOTES:
 *
 * HISTORY
 *    27/04/2011 - KH Shi, Created
 *
 * Copyright(C)      Accton Corporation, 2011
 */
#ifndef NSM_POLICY_TYPE_H
#define NSM_POLICY_TYPE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_adpt.h"
#include "l_inet.h"
#include "bgp_type.h"

/* NAME CONSTANT DECLARATIONS
 */
#define NSM_POLICY_TYPE_ACCESS_LIST_NAME_LEN   SYS_ADPT_MAX_ACCESS_LIST_NAME_LENGTH
#define NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN     SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH
#define NSM_POLICY_TYPE_PREFIX_LIST_NAME_LEN   SYS_ADPT_MAX_PREFIX_LIST_NAME_LENGTH
#define NSM_POLICY_TYPE_AS_LIST_NAME_LEN       SYS_ADPT_MAX_AS_PATH_ACCESS_LIST_NAME_LENGTH
#define NSM_POLICY_TYPE_COMMUNITY_NAME_LEN     SYS_ADPT_MAX_COMMUNITY_NAME_LENGTH

#define NSM_POLICY_TYPE_ROUTE_MAP_COMMAND_LEN  SYS_ADPT_MAX_ROUTE_MAP_COMMAND_LENGTH
#define NSM_POLICY_TYPE_ROUTE_MAP_ARG_LEN      SYS_ADPT_MAX_ROUTE_MAP_ARGUMENT_LENGTH
#define NSM_POLICY_TYPE_REGULAR_EXP_LEN        SYS_ADPT_MAX_REGULAR_EXPRESSION_LENGTH
#define NSM_POLICY_TYPE_ROUTE_MAP_DESCRIPTION_LEN 20
#define NSM_POLICY_TYPE_PREFIX_LIST_DESCRIPTION_LEN 20
/**********************************
** definitions for return value **
**********************************
*/
enum
{
    NSM_POLICY_TYPE_RESULT_OK = 0,
    NSM_POLICY_TYPE_RESULT_FAIL,
    NSM_POLICY_TYPE_RESULT_SEND_MSG_FAIL,
    NSM_POLICY_TYPE_RESULT_INVALID_COMMAND,
    NSM_POLICY_TYPE_RESULT_INVALID_ARG,
    NSM_POLICY_TYPE_RESULT_ROUTE_MAP_GOTO_BACKWARD_ERROR,
    NSM_POLICY_TYPE_RESULT_COMMUNITY_LIST_ERR_STANDARD_CONFLICT,
    NSM_POLICY_TYPE_RESULT_COMMUNITY_LIST_ERR_EXPANDED_CONFLICT,
    NSM_POLICY_TYPE_RESULT_COMMUNITY_LIST_ERR_NAME_IS_ALL_DIGIT,
};

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */



//ref plist.h

enum NSM_POLICY_TYPE_PrefixListType_E
{
  NSM_POLICY_TYPE_PREFIX_DENY,
  NSM_POLICY_TYPE_PREFIX_PERMIT,
};

enum NSM_POLICY_TYPE_PrefixNameType_E
{
  NSM_POLICY_TYPE_PREFIX_TYPE_STRING,
  NSM_POLICY_TYPE_PREFIX_TYPE_NUMBER
};

//struct prefix_list
typedef struct {
  char name[NSM_POLICY_TYPE_PREFIX_LIST_NAME_LEN +1];
  char desc[NSM_POLICY_TYPE_PREFIX_LIST_DESCRIPTION_LEN +1];

  //struct prefix_master *master;

  enum NSM_POLICY_TYPE_PrefixNameType_E type; //string or number

  int count;
  int rangecount;
  UI32_T head_seq;
  UI32_T tail_seq;
  /* struct prefix_list_entry *head;
  struct prefix_list_entry *tail;

  struct prefix_list *next;
  struct prefix_list *prev;
  */
} NSM_POLICY_TYPE_PrefixList_T;

/*
struct orf_prefix
{
  u_int32_t seq;
  u_char ge;
  u_char le;
  struct prefix p;
};
*/

//ref. plist.c
/* Each prefix-list's entry. */
//struct prefix_list_entry
typedef struct
{
  char plist_name[NSM_POLICY_TYPE_PREFIX_LIST_NAME_LEN +1];
  UI32_T seq;

  UI32_T le;
  UI32_T ge;

  enum NSM_POLICY_TYPE_PrefixListType_E type; /* deny/permit */

  UI8_T any;
  //struct prefix prefix;
  L_INET_AddrIp_T prefix;
  
  UI32_T refcnt;
  UI32_T hitcnt;

 // struct prefix_list_entry *next;
 // struct prefix_list_entry *prev;
} NSM_POLICY_TYPE_PrefixListEntry_T;

//ref routemap.c
/* Route map rule. This rule has both `match' rule and `set' rule. */
//struct route_map_rule
typedef struct
{
  char map_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1]; /* key 1 */
  /* Preference of this route map rule. */
  UI8_T index_type; /* key 2 */
  UI32_T index_pref; /* key 2 */
  UI32_T seq_index;

  /* Rule type. */
  //struct route_map_rule_cmd *cmd;

  /* Route map rule name (e.g. as-path, metric) */
  char cmd_str[NSM_POLICY_TYPE_ROUTE_MAP_ARG_LEN +1];

  /* For pretty printing. */
  char rule_str[NSM_POLICY_TYPE_ROUTE_MAP_ARG_LEN+1];

  /* Pre-compiled match rule. */
  //void *value;

  /* Linked list. */
  //struct route_map_rule *next;
  //struct route_map_rule *prev;
} NSM_POLICY_TYPE_RouteMapRule_T;

//ref routemap.h
/* Route map rule structure for matching and setting. */
//struct route_map_rule_cmd

#if 0
typedef struct
{
  /* Route map rule name (e.g. as-path, metric) */
  const char str[NSM_POLICY_TYPE_ROUTE_MAP_ARG_LEN +1];

  /* Function for value set or match. */
  route_map_result_t (*func_apply)(void *, struct prefix *, 
				   route_map_object_t, void *);

  /* Compile argument and return result as void *. */
  void *(*func_compile)(const char *);

  /* Free allocated value by func_compile (). */
  void (*func_free)(void *);
} NSM_POLICY_TYPE_RouteMapRuleCmd;
#endif


/* Route map apply error. */
enum
{
  /* Route map rule is missing. */
  NSM_POLICY_TYPE_RMAP_RULE_MISSING = 1,

  /* Route map rule can't compile */
  NSM_POLICY_TYPE_RMAP_COMPILE_ERROR
};

/* Route map's type. */
//enum route_map_type
enum NSM_POLICY_TYPE_RouteMapType
{
  NSM_POLICY_TYPE_RMAP_PERMIT,
  NSM_POLICY_TYPE_RMAP_DENY,
  NSM_POLICY_TYPE_RMAP_ANY
};

typedef enum
{
  NSM_POLICY_TYPE_RMAP_EXIT,
  NSM_POLICY_TYPE_RMAP_GOTO,
  NSM_POLICY_TYPE_RMAP_NEXT
} NSM_POLICY_TYPE_RouteMapEnd_T;


/* Route map index structure. */
//struct route_map_index
typedef struct
{
  //struct route_map *map;
  char map_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1];
  char description[NSM_POLICY_TYPE_ROUTE_MAP_DESCRIPTION_LEN +1];

  /* Preference of this route map rule. */
  int pref;

  /* Route map type permit or deny. */
  enum NSM_POLICY_TYPE_RouteMapType type;			

  /* Do we follow old rules, or hop forward? */
  NSM_POLICY_TYPE_RouteMapEnd_T exitpolicy;

  /* If we're using "GOTO", to where do we go? */
  int nextpref;

  /* If we're using "CALL", to which route-map do ew go? */
  char nextrm[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1];

  /* Matching rule list. */
  //struct route_map_rule_list match_list;
  //struct route_map_rule_list set_list;

  /* Make linked list. */
  //struct route_map_index *next;
  //struct route_map_index *prev;
} NSM_POLICY_TYPE_RouteMapIndex_T;

/* Route map list structure. */
//struct route_map
typedef struct
{
  /* Name of route map. */
  char name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1];

  /* Route map's rule. */
  //struct route_map_index *head;
  //struct route_map_index *tail;

  /* Make linked list. */
  //struct route_map *next;
  //struct route_map *prev;
} NSM_POLICY_TYPE_RouteMap_T;


#endif

