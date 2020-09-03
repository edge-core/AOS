#ifndef 	STKTPLG_BOARD_UTIL_H
#define 	STKTPLG_BOARD_UTIL_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "stktplg_board.h"

static inline BOOL_T STKTPLG_BOARD_UTIL_IsQSPFPort(
    UI8_T board_id,
    UI32_T port_id)
{
    STKTPLG_BOARD_BoardInfo_T board_info;

	if (!STKTPLG_BOARD_GetBoardInformation(board_id, &board_info))
    {
        return FALSE;
    }

    if (port_id >= board_info.qsfp_port_start && 
		port_id < (board_info.qsfp_port_start + board_info.qsfp_port_number))
    {
        return TRUE;
    }
    return FALSE;
}

static inline BOOL_T STKTPLG_BOARD_UTIL_IsBreakOutPort(
    UI8_T board_id,
    UI32_T port_id)
{
    STKTPLG_BOARD_BoardInfo_T board_info;

	if (!STKTPLG_BOARD_GetBoardInformation(board_id, &board_info))
    {
        return FALSE;
    }

    if (port_id >= board_info.break_out_port_start && 
		port_id < (board_info.break_out_port_start + board_info.qsfp_port_number*4))
    {
        return TRUE;
    }
    return FALSE;
}

static inline BOOL_T STKTPLG_BOARD_UTIL_GetBreakOutGroupPortIdRange(
    UI8_T board_id, UI32_T breakout_port_id,
    UI32_T* breakout_group_port_id_min_p,
    UI32_T* breakout_group_port_id_max_p)
{
    STKTPLG_BOARD_BoardInfo_T board_info;

	if (!STKTPLG_BOARD_GetBoardInformation(board_id, &board_info))
    {
        return FALSE;
    }

	if (board_info.break_out_port_start == 0 || breakout_port_id < board_info.break_out_port_start)
    {
        return FALSE;
    }

    *breakout_group_port_id_min_p = breakout_port_id - ((breakout_port_id - board_info.break_out_port_start)%4);
	*breakout_group_port_id_max_p = *breakout_group_port_id_min_p + 3;

	return TRUE;
}

static inline UI8_T STKTPLG_BOARD_UTIL_GetPowerNumber()
{
#if (SYS_CPNT_DETECT_POWER_NUMBER_FROM_ONLP == TRUE)
    return STKTPLG_BOARD_GetPowerNumber();
#else
    return SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT;
#endif
}

#endif /*STKTPLG_BOARD_UTIL_H*/
