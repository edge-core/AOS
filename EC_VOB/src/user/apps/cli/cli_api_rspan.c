#include "cli_api.h"
#include "cli_api_rspan.h"
#include "rspan_pmgr.h"
#include "rspan_om.h"
#include "stktplg_pom.h"

#if (SYS_CPNT_RSPAN == TRUE)
static UI32_T cli_api_Show_One_Rspan_Session(char session_id, RSPAN_OM_SessionEntry_T rspan_session_entry, UI32_T max_port_number, UI32_T unit, UI32_T line_num);
static UI32_T cli_api_Show_Rspan_Switch_Role(char switch_role, UI32_T line_num);
static UI32_T cli_api_Show_Rspan_Source(UI8_T src_rx_ar[], UI8_T src_tx_ar[], UI32_T max_port_number, UI32_T unit, UI32_T line_num);
static UI32_T cli_api_Show_Rspan_Source_Heading(UI8_T src_rx_ar[], UI8_T src_tx_ar[], UI8_T src_both_rx_tx_ar[], BOOL_T *empty_rx_list_p, BOOL_T *empty_tx_list_p, BOOL_T *empty_rx_tx_list_p, UI32_T max_port_number, UI32_T line_num);
static UI32_T cli_api_Show_Rspan_Conditional(UI8_T src_ar[], UI8_T src_both_rx_tx_ar[], int k, char transmission_mode);
static UI32_T cli_api_Meta_Show_Rspan_Source(UI8_T src_ar[], UI8_T src_both_rx_tx_ar[], UI32_T max_port_number, UI32_T unit, char transmission_mode, BOOL_T empty_list, UI32_T line_num);
static UI32_T cli_api_Show_Rspan_Destination(char destination, UI32_T unit, UI32_T line_num);
static UI32_T cli_api_Show_Rspan_Vlan(UI32_T remote_vid, UI32_T line_num);
static UI32_T cli_api_Show_Rspan_Uplink_Port_Heading(UI8_T uplink_ar[], UI32_T max_port_number, UI32_T unit, BOOL_T *empty_list_p, UI32_T line_num);
static UI32_T cli_api_Meta_Show_Rspan_Uplink_Port(UI8_T uplink_ar[], UI32_T max_port_number, UI32_T unit, BOOL_T empty_list, UI32_T line_num);
static UI32_T cli_api_Show_Rspan_Uplink_Port(UI8_T uplink_ar[], UI32_T max_port_number, UI32_T unit, UI32_T line_num);
static UI32_T cli_api_Show_Rspan_Destination_Tagged_Mode(char destination, char is_tagged, UI32_T line_num);
/* Add a new field called "Operation Status". 2008.1.10. by Tien.
 */
static UI32_T cli_api_Show_Rspan_Operation_Status(char session_id, UI32_T line_num);

static char cli_api_Build_Port_ID_List(UI32_T unit, UI8_T port_id_list_ar[], char *port_list_argument_p);
static char cli_api_Get_Rspan_Source_Mode(char *mode_argument_p);
static char cli_api_Set_Rspan_Source_Interface(char session_id, UI32_T max_port_number, UI8_T port_id_list_ar[], UI32_T unit, char *mode_argument_p);
static char cli_api_Unset_Rspan_Source_Interface(char session_id, UI32_T max_port_number, UI8_T port_id_list_ar[], UI32_T unit, char *mode_argument_p);
static UI8_T cli_api_Get_Rspan_Destination_Mode(char *mode_argument_p);
static char cli_api_Set_Rspan_Destination_Interface(char session_id, UI32_T max_port_number, UI8_T port_id_list_ar[], UI32_T unit, char *mode_argument_p);
static char cli_api_Unset_Rspan_Destination_Interface(char session_id, UI32_T max_port_number, UI8_T port_id_list_ar[], UI32_T unit);
static char cli_api_Get_Rspan_Remote_Vlan_Switch_Role(char *switch_role_argument_p);
static char cli_api_Set_Rspan_Remote_Vlan(char session_id, char *switch_role_argument_p, UI32_T remote_vid, UI32_T max_port_number, UI8_T port_id_list_ar[], UI32_T unit);
static char cli_api_Unset_Rspan_Remote_Vlan(char session_id, char *switch_role_argument_p, UI32_T remote_vid, UI32_T max_port_number, UI8_T port_id_list_ar[], UI32_T unit);

/* execution
 */

/*-----------------------------------------------------------
 * ROUTINE NAME - cli_api_Show_One_Rspan_Session
 *-----------------------------------------------------------
 * PURPOSE : This function displays in the concole the configuration of one Rspan session.
 * INPUT   : session_id - The Rspan session ID, currently 1 or 2.
 *         : rspan_session_entry - The data structure of an Rspan session.
 *         : max_port_number - The maximum number of ports allowed in the Rspan session.
 *         : unit - The unit ID of the switch.
 *         : line_num - The number of lines displayed so far in the console.
 * OUTPUT  : The configuration of a single Rspan session.
 * RETURN  : line_num - The number of lines displayed so far in the console.
 * NOTE    : This function is called by CLI_API_Show_Rspan().
 *           3-31-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
static UI32_T cli_api_Show_One_Rspan_Session(char session_id, RSPAN_OM_SessionEntry_T rspan_session_entry, UI32_T max_port_number, UI32_T unit, UI32_T line_num)
{
    UI8_T buff[CLI_DEF_MAX_BUFSIZE] = {0};

	CLI_LIB_PrintStr_1("RSPAN Session ID\t\t: %d", session_id);
	PROCESS_MORE_FUNC("\r\n");

    /* Reorder the fields of displaying and add a new field called "Operation Status". 2008.1.10. by Tien.
     */
    line_num=cli_api_Show_Rspan_Source(rspan_session_entry.src_rx, rspan_session_entry.src_tx, max_port_number, unit, line_num);
    if(line_num==JUMP_OUT_MORE || line_num==EXIT_SESSION_MORE)
    {
	    return line_num;
    }
    line_num=cli_api_Show_Rspan_Destination(rspan_session_entry.dst, unit, line_num);
    if(line_num==JUMP_OUT_MORE || line_num==EXIT_SESSION_MORE)
    {
	    return line_num;
    }
    line_num=cli_api_Show_Rspan_Destination_Tagged_Mode(rspan_session_entry.dst, rspan_session_entry.is_tagged, line_num);
    if(line_num==JUMP_OUT_MORE || line_num==EXIT_SESSION_MORE)
    {
	    return line_num;
    }
    line_num=cli_api_Show_Rspan_Switch_Role(rspan_session_entry.switch_role, line_num);
    if(line_num==JUMP_OUT_MORE || line_num==EXIT_SESSION_MORE)
    {
	    return line_num;
    }
    line_num=cli_api_Show_Rspan_Vlan(rspan_session_entry.remote_vid, line_num);
    if(line_num==JUMP_OUT_MORE || line_num==EXIT_SESSION_MORE)
    {
	    return line_num;
    }
    line_num=cli_api_Show_Rspan_Uplink_Port(rspan_session_entry.uplink, max_port_number, unit, line_num);
    if(line_num==JUMP_OUT_MORE || line_num==EXIT_SESSION_MORE)
    {
	    return line_num;
    }
    line_num=cli_api_Show_Rspan_Operation_Status ( session_id, line_num);

    PROCESS_MORE_FUNC("\r\n");
    return line_num;
}



/*-----------------------------------------------------------
 * ROUTINE NAME - cli_api_Show_Rspan_Switch_Role
 *-----------------------------------------------------------
 * PURPOSE : This function shows the switch role of a switch in an Rspan-aware network topology.
 * INPUT   : switch_role - role of remote VLAN uplink port.  It can be one of the following ---
 *           VAL_rspanSwitchRole_source,
 *           VAL_rspanSwitchRole_intermediate,
 *           VAL_rspanSwitchRole_destination.
 *           The default is none of the above.
 *         : line_num - The number of lines displayed so far in the console.
 * OUTPUT  : The switch role of a switch.
 * RETURN  : line_num - The number of lines displayed so far in the console.
 * NOTE    : This function is called by cli_api_Show_One_Rspan_Session().
 *           3-31-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
static UI32_T cli_api_Show_Rspan_Switch_Role(char switch_role, UI32_T line_num)
{
    UI8_T buff[CLI_DEF_MAX_BUFSIZE] = {0};

	CLI_LIB_PrintStr("Switch Role \t\t\t: ");
	switch(switch_role)
	{
		case VAL_rspanSwitchRole_source:
			CLI_LIB_PrintStr("Source");
			break;

		case VAL_rspanSwitchRole_intermediate:
			CLI_LIB_PrintStr("Intermediate");
			break;

		case VAL_rspanSwitchRole_destination:
			CLI_LIB_PrintStr("Destination");
			break;

		default:
			CLI_LIB_PrintStr("None");
			break;
	}

    PROCESS_MORE_FUNC("\r\n");
    return line_num;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - cli_api_Show_Rspan_Source_Heading
 *-----------------------------------------------------------
 * PURPOSE : This function shows the heading of the Rspan source port information.
 *           If there is no port constructed, it display "None" and
 *           it constructs the port lists (rx_list, tx_list, rx_tx_list) to be displayed by
 *           subsequent display functions.
 * INPUT   : src_rx_ar[] - The array of rx attribute of the source ports.
 *         : src_tx_ar[] - The array of tx attribute of the source ports.
 *         : src_both_rx_tx_ar[] - The array of both rx and tx attribute of the source ports.
 *         : empty_rx_list_p - a pointer to a boolean value indicating whether the rx_list is empty.
 *         : empty_tx_list_p - a pointer to a boolean value indicating whether the tx_list is empty.
 *         : empty_rx_rx_list_p - a pointer to a boolean value indicating whether both the rx_list
 *           and the tx_list are empty.
 *         : max_port_number - The maximum number of ports allowed in the Rspan session.
 *         : line_num - The number of lines displayed so far in the console.
 * OUTPUT  : The heading of the Rspan source port information.
 * RETURN  : line_num - The number of lines displayed so far in the console.
 * NOTE    : This function is called by cli_api_Show_Rspan_Source().
 *           3-31-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
static UI32_T cli_api_Show_Rspan_Source_Heading(UI8_T src_rx_ar[], UI8_T src_tx_ar[], UI8_T src_both_rx_tx_ar[], BOOL_T *empty_rx_list_p, BOOL_T *empty_tx_list_p, BOOL_T *empty_rx_tx_list_p, UI32_T max_port_number, UI32_T line_num)
{
    UI32_T rx_on;
    UI32_T tx_on;
    UI8_T both_value[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]={0};
    UI8_T buff[CLI_DEF_MAX_BUFSIZE] = {0};
    int k;

	CLI_LIB_PrintStr("Source Ports (mirrored ports)");

    for (k=1; k<=max_port_number; k++)
    {
    	rx_on=src_rx_ar[(UI32_T)((k-1)/8)] & (1 << (7 - ((k-1)%8)));
    	tx_on=src_tx_ar[(UI32_T)((k-1)/8)] & (1 << (7 - ((k-1)%8)));
        if(rx_on && tx_on)
        {
                both_value[(UI32_T)((k-1)/8)]=both_value[(UI32_T)((k-1)/8)]|rx_on;
        	*empty_rx_tx_list_p=FALSE;
        	src_both_rx_tx_ar[(UI32_T)((k-1)/8)]=both_value[(UI32_T)((k-1)/8)];
        }
        else if(rx_on)
        {
        	*empty_rx_list_p=FALSE;
        }
        else if(tx_on)
        {
        	*empty_tx_list_p=FALSE;
        }
    }

    if((*empty_rx_list_p)&&(*empty_tx_list_p)&&(*empty_rx_tx_list_p))
    {
        CLI_LIB_PrintStr("\t: None");
        PROCESS_MORE_FUNC("\r\n");
        return line_num;
    }
    else
    {
        PROCESS_MORE_FUNC("\r\n");
    }

	return line_num;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - cli_api_Show_Rspan_Conditional
 *-----------------------------------------------------------
 * PURPOSE : This function serves as an if-conditional to determine if there is information
 *           to show in an Rspan session.
 *           If the transmission mode is rx or tx, the conditional is determined by having both
 *           the source port selected and by the rx or tx attribute correspondingly.
 *           If the transmission mode is both rx and tx, the conditional is determined solely
 *           by the selection of source port.
 * INPUT   : src_ar[] - The array of the source ports. This is one of the following --- src_rx_ar[], src_tx_ar[], src_both_rx_tx_ar[].
 *         : src_both_rx_tx_ar[] - The array of both rx and tx attribute of the source ports.
 *         : k - The array index to src_ar[] and src_both_rx_tx_ar[].
 *         : transmission_mode - One of the following Rspan transmission modes:
 *           VAL_mirrorType_rx,
 *           VAL_mirrorType_tx,
 *           VAL_mirrorType_both.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE value of an if-conditional.
 * NOTE    : This function is called by cli_api_Meta_Show_Rspan_Source().
 *           7-24-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
static UI32_T cli_api_Show_Rspan_Conditional(UI8_T src_ar[], UI8_T src_both_rx_tx_ar[], int k, char transmission_mode)
{
	if(transmission_mode==VAL_mirrorType_both)
    {
        return (src_ar[(UI32_T)((k-1)/8)] & (1 << (7-((k-1)%8))) );
    }
    else
    {
    	return (src_ar[(UI32_T)((k-1)/8)] & (1 << (7-((k-1)%8))) )
            &&!(src_both_rx_tx_ar[(UI32_T)((k-1)/8)] & (1 << (7-((k-1)%8))) );
    }
}

/*-----------------------------------------------------------
 * ROUTINE NAME - cli_api_Meta_Show_Rspan_Source
 *-----------------------------------------------------------
 * PURPOSE : This function shows the information of an rx, tx, or both_rx_tx attribute of an Rspan
 *           source port.  According to the transmission mode, the ports are displayed
 *           five in a roll in the format of unit/port.  If there are no ports in a particular transmission category,
 *           "None" is displayed in the port field.
 * INPUT   : src_ar[] - The array of the source ports. This is one of the following --- src_rx_ar[], src_tx_ar[], src_both_rx_tx_ar[].
 *         : src_both_rx_tx_ar[] - The array of both rx and tx attribute of the source ports.
 *         : max_port_number - The maximum number of ports allowed in the Rspan session.
 *         : unit - The unit ID of the switch.
 *         : transmission_mode - One of the following Rspan transmission modes:
 *           VAL_mirrorType_rx,
 *           VAL_mirrorType_tx,
 *           VAL_mirrorType_both
 *         : empty_list_p - a pointer to a boolean value indicating whether the port list is empty.
 *           The port list can be only of the following lists:
 *           empty_rx_list_p - a pointer to a boolean value indicating whether the rx_list is empty.
 *           empty_tx_list_p - a pointer to a boolean value indicating whether the tx_list is empty.
 *           empty_rx_rx_list_p - a pointer to a boolean value indicating whether both the rx_list
 *           and the tx_list are empty.
 *         : line_num - The number of lines displayed so far in the console.
 * OUTPUT  : information of source ports in an Rspan session according to transmission mode.
 * RETURN  : line_num - The number of lines displayed so far in the console.
 * NOTE    : This function is called by cli_api_Show_Rspan_Source().
 *           7-24-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
static UI32_T cli_api_Meta_Show_Rspan_Source(UI8_T src_ar[], UI8_T src_both_rx_tx_ar[], UI32_T max_port_number, UI32_T unit, char transmission_mode, BOOL_T empty_list, UI32_T line_num)
{
    UI32_T port_counter = 0;
    UI8_T buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T  port, trunk_id;
    BOOL_T is_first_line = TRUE;
	BOOL_T first_entry_to_print;
    int k;

    switch (transmission_mode)
    {
        case VAL_mirrorType_rx:
        	CLI_LIB_PrintStr("  RX Only\t\t\t: ");
        	break;
        case VAL_mirrorType_tx:
        	CLI_LIB_PrintStr("  TX Only\t\t\t: ");
        	break;
        case VAL_mirrorType_both:
        	CLI_LIB_PrintStr("  BOTH\t\t\t\t: ");
        	break;
        default:
        	return line_num;
        	break;
    }

    if(empty_list)
    {
        CLI_LIB_PrintStr("None");
        PROCESS_MORE_FUNC("\r\n");
    }
    else
    {
    	CLI_LIB_PrintStr("Eth ");
    	first_entry_to_print=TRUE;
    	for (k=1 ; k<=max_port_number ; k++)
        {
             if (cli_api_Show_Rspan_Conditional(src_ar, src_both_rx_tx_ar, k, transmission_mode))
             {
                 port_counter++;
                 if (port_counter%5 == 1)
                 {
                     if(is_first_line)
                     {
                         is_first_line = FALSE;
                         CLI_LIB_PrintStr("");
                     }
                     else
                     {
                         CLI_LIB_PrintStr("                                  ");
                     }
                 }
                 if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_POM_LogicalPortToUserPort(k, &unit, &port, &trunk_id))
                 {
                    continue;
                 }
             	 if(!first_entry_to_print)
             	 {
             	     CLI_LIB_PrintStr(", ");
             	 }
             	 CLI_LIB_PrintStr_2("%lu/%lu", (unsigned long)unit, (unsigned long)port);
                 first_entry_to_print=FALSE;
                 if (port_counter % 5 == 0)
                 {
                     PROCESS_MORE_FUNC("\r\n");
                     first_entry_to_print=TRUE;
                 }
             }
        }
        if (port_counter % 5 != 0 || port_counter == 0)
        {
           PROCESS_MORE_FUNC("\r\n");
        }
    }

    return line_num;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - cli_api_Show_Rspan_Source
 *-----------------------------------------------------------
 * PURPOSE : This function shows the information of the source ports in an Rspan session.
 *           According to the transmission mode, the ports are displayed five in a roll in the format of unit/port.
 *           If there are no ports in a particular transmission category, "None" is displayed in the port field.
 * INPUT   : src_rx_ar[] - The array of rx attribute of the source ports.
 *         : src_tx_ar[] - The array of tx attribute of the source ports.
 *         : max_port_number - The maximum number of ports allowed in the Rspan session.
 *         : unit - The unit ID of the switch.
 *         : line_num - The number of lines displayed so far in the console.
 * OUTPUT  : information of source ports in an Rspan session.
 * RETURN  : line_num - The number of lines displayed so far in the console or
 *           JUMP_OUT_MORE - an enumerated value to indicate the exit of the PROCESS_MORE macro.
 *           This is a line number supposed to be greater than all displayed line numbers.
 * NOTE    : This function is called by cli_api_Show_One_Rspan_Session().
 *           7-24-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
static UI32_T cli_api_Show_Rspan_Source(UI8_T src_rx_ar[], UI8_T src_tx_ar[], UI32_T max_port_number, UI32_T unit, UI32_T line_num)
{
	UI8_T src_both_rx_tx_ar[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]={0};
	BOOL_T empty_rx_list=TRUE;
	BOOL_T empty_tx_list=TRUE;
	BOOL_T empty_rx_tx_list=TRUE;

    line_num=cli_api_Show_Rspan_Source_Heading(src_rx_ar, src_tx_ar, src_both_rx_tx_ar, &empty_rx_list, &empty_tx_list, &empty_rx_tx_list, max_port_number, line_num);
    if (line_num == JUMP_OUT_MORE || line_num == EXIT_SESSION_MORE)
    {
        return JUMP_OUT_MORE;
    }

    line_num=cli_api_Meta_Show_Rspan_Source(src_rx_ar, src_both_rx_tx_ar, max_port_number, unit, VAL_mirrorType_rx, empty_rx_list, line_num);
    if (line_num == JUMP_OUT_MORE || line_num == EXIT_SESSION_MORE)
    {
        return JUMP_OUT_MORE;
    }

    line_num=cli_api_Meta_Show_Rspan_Source(src_tx_ar, src_both_rx_tx_ar, max_port_number, unit, VAL_mirrorType_tx, empty_tx_list, line_num);
    if (line_num == JUMP_OUT_MORE || line_num == EXIT_SESSION_MORE)
    {
        return JUMP_OUT_MORE;
    }

    line_num=cli_api_Meta_Show_Rspan_Source(src_both_rx_tx_ar, NULL, max_port_number, unit, VAL_mirrorType_both, empty_rx_tx_list, line_num);
    if (line_num == JUMP_OUT_MORE || line_num == EXIT_SESSION_MORE)
    {
        return JUMP_OUT_MORE;
    }

    return line_num;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - cli_api_Show_Rspan_Destination
 *-----------------------------------------------------------
 * PURPOSE : This function shows the information of the source ports in an Rspan session.
 *           According to the transmission mode, the ports are displayed five in a roll in the format of unit/port.
 *           If there are no ports in a particular transmission category, "None" is displayed in the port field.
 * INPUT   : destination - The destination (monitoring) port.
 *         : unit - The unit ID of the switch.
 *         : line_num - The number of lines displayed so far in the console.
 * OUTPUT  : information of destination port in an Rspan session.
 * RETURN  : line_num - The number of lines displayed so far in the console.
 * NOTE    : This function is called by cli_api_Show_One_Rspan_Session().
 *           7-24-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
static UI32_T cli_api_Show_Rspan_Destination(char destination, UI32_T unit, UI32_T line_num)
{
    UI8_T buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T port, trunk_id;

    CLI_LIB_PrintStr("Destination Port (monitor port)\t: ");

    if (SWCTRL_LPORT_NORMAL_PORT == SWCTRL_POM_LogicalPortToUserPort(destination, &unit, &port, &trunk_id))
    {
        CLI_LIB_PrintStr_2("Eth %lu/%lu", (unsigned long)unit, (unsigned long)port);
    }
    else
    {
        CLI_LIB_PrintStr("None");

    }
    PROCESS_MORE_FUNC("\r\n");

    return line_num;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - cli_api_Show_Rspan_Vlan
 *-----------------------------------------------------------
 * PURPOSE : This function shows the information of the remote VLAN in an Rspan session.
 *           If there is not an Rspan VLAN configured, "None" is displayed.
 * INPUT   : remote_vid - The remote Rspan VLAN ID.
 *         : line_num - The number of lines displayed so far in the console.
 * OUTPUT  : The header of the Rspan VLAN and the VLAN ID in the console.
 * RETURN  : line_num - The number of lines displayed so far in the console.
 * NOTE    : This function is called by cli_api_Show_One_Rspan_Session().
 *           7-24-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
static UI32_T cli_api_Show_Rspan_Vlan(UI32_T remote_vid, UI32_T line_num)
{
    UI8_T buff[CLI_DEF_MAX_BUFSIZE] = {0};

    CLI_LIB_PrintStr("RSPAN VLAN\t\t\t: ");

    if(remote_vid)
    {
        CLI_LIB_PrintStr_1("%ld", (long)remote_vid);
    }
    else
    {
        CLI_LIB_PrintStr("None");
    }
    PROCESS_MORE_FUNC("\r\n");

    return line_num;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - cli_api_Show_Rspan_Uplink_Port_Heading
 *-----------------------------------------------------------
 * PURPOSE : This function shows the heading of the Rspan uplink port information.
 *           It checks if the uplink port list is empty.
 * INPUT   : uplink_ar[] - The array of uplink ports.
 *         : max_port_number - The maximum number of ports allowed in the Rspan session.
 *         : unit - The unit ID of the switch.
 *         : empty_list_p - a pointer to a boolean value indicating whether the port list is empty.
 *           The port list can be only of the following lists:
 *           empty_rx_list_p - a pointer to a boolean value indicating whether the rx_list is empty.
 *           empty_tx_list_p - a pointer to a boolean value indicating whether the tx_list is empty.
 *           empty_rx_rx_list_p - a pointer to a boolean value indicating whether both the rx_list
 *           and the tx_list are empty.
 *         : line_num - The number of lines displayed so far in the console.
 * OUTPUT  : The heading of the Rspan uplink port information.
 * RETURN  : line_num - The number of lines displayed so far in the console.
 * NOTE    : This function is called by cli_api_Show_Rspan_Uplink_Port().
 *           7-24-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
static UI32_T cli_api_Show_Rspan_Uplink_Port_Heading(UI8_T uplink_ar[], UI32_T max_port_number, UI32_T unit, BOOL_T *empty_list_p, UI32_T line_num)
{
    int k;

	CLI_LIB_PrintStr("RSPAN Uplink Ports\t\t: ");
    *empty_list_p=TRUE;

    for (k=1; k<=max_port_number&&(*empty_list_p); k++)
    {
        if(uplink_ar[(UI32_T)((k-1)/8)] & (1 << (7 - ((k-1)%8))) )
        {
        	*empty_list_p=FALSE;
        }
    }

	return line_num;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - cli_api_Meta_Show_Rspan_Uplink_Port
 *-----------------------------------------------------------
 * PURPOSE : This function shows the information of Rspan uplink ports.  The ports are displayed
 *           five in a roll in the format of unit/port.  If there are no ports in a particular transmission category,
 *           "None" is displayed in the port field.
 * INPUT   : uplink_ar[] - The array of the uplink ports.
 *         : max_port_number - The maximum number of ports allowed in the Rspan session.
 *         : unit - The unit ID of the switch.
 *         : empty_list - a boolean value indicating whether the port list is empty.
 *         : line_num - The number of lines displayed so far in the console.
 * OUTPUT  : information of uplink ports in an Rspan session.
 * RETURN  : line_num - The number of lines displayed so far in the console.
 * NOTE    : This function is called by cli_api_Show_Rspan_Uplink_Port().
 *           7-24-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
static UI32_T cli_api_Meta_Show_Rspan_Uplink_Port(UI8_T uplink_ar[], UI32_T max_port_number, UI32_T unit, BOOL_T empty_list, UI32_T line_num)
{
    UI32_T port_counter = 0;
    UI8_T buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T port, trunk_id;
    BOOL_T is_first_line = TRUE;
	BOOL_T first_entry_to_print;
    int k;

    if(empty_list)
    {
        CLI_LIB_PrintStr("None");
        PROCESS_MORE_FUNC("\r\n");
    }
    else
    {
    	CLI_LIB_PrintStr("Eth ");
    	first_entry_to_print=TRUE;
    	for (k=1 ; k<=max_port_number ; k++)
        {
             if (uplink_ar[(UI32_T)((k-1)/8)] & (1 << (7-((k-1)%8))))
             {
                 port_counter++;
                 if (port_counter%5 == 1)
                 {
                     if(is_first_line)
                     {
                         is_first_line = FALSE;
                         CLI_LIB_PrintStr("");
                     }
                     else
                     {
                         CLI_LIB_PrintStr("                                  ");
                     }
                 }
                 if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_POM_LogicalPortToUserPort(k, &unit, &port, &trunk_id))
                 {
                    continue;
                 }
             	 port=k;
             	 if(!first_entry_to_print)
             	 {
             	 	CLI_LIB_PrintStr(", ");
             	 }
             	 CLI_LIB_PrintStr_2("%lu/%lu", (unsigned long)unit, (unsigned long)port);
                 first_entry_to_print=FALSE;
                 if (port_counter % 5 == 0)
                 {
                       PROCESS_MORE_FUNC("\r\n");
                       first_entry_to_print=TRUE;
                 }
             }
        }
        if (port_counter % 5 != 0 || port_counter == 0)
        {
            PROCESS_MORE_FUNC("\r\n");
        }
    }

    return line_num;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - cli_api_Show_Rspan_Uplink_Port
 *-----------------------------------------------------------
 * PURPOSE : This function shows the information of Rspan uplink ports.  The ports are displayed
 *           five in a roll in the format of unit/port.  If there are no ports in a particular transmission category,
 *           "None" is displayed in the port field.
 * INPUT   : uplink_ar[] - The array of the uplink ports.
 *         : max_port_number - The maximum number of ports allowed in the Rspan session.
 *         : unit - The unit ID of the switch.
 *         : line_num - The number of lines displayed so far in the console.
 * OUTPUT  : information of uplink ports in an Rspan session.
 * RETURN  : line_num - The number of lines displayed so far in the console or
 *           JUMP_OUT_MORE - an enumerated value to indicate the exit of the PROCESS_MORE macro.
 *           This is a line number supposed to be greater than all displayed line numbers.
 * NOTE    : This function is called by cli_api_Show_One_Rspan_Session().
 *           7-24-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
static UI32_T cli_api_Show_Rspan_Uplink_Port(UI8_T uplink_ar[], UI32_T max_port_number, UI32_T unit, UI32_T line_num)
{
	BOOL_T empty_list;

    line_num=cli_api_Show_Rspan_Uplink_Port_Heading(uplink_ar, max_port_number, unit, &empty_list, line_num);
    line_num=cli_api_Meta_Show_Rspan_Uplink_Port(uplink_ar, max_port_number, unit, empty_list, line_num);
    if (line_num == JUMP_OUT_MORE || line_num == EXIT_SESSION_MORE)
    {
        return JUMP_OUT_MORE;
    }

    return line_num;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - cli_api_Show_Rspan_Destination_Tagged_Mode
 *-----------------------------------------------------------
 * PURPOSE : This function shows the tagged status of the destination port.
 * INPUT   : destination - The destination (monitoring) port.
 *         : is_tagged - One of the following value indicating the tagged status of the destination port.
 *           VAL_rspanDstPortTag_none,
 *           VAL_rspanDstPortTag_tagged,
 *           VAL_rspanDstPortTag_untagged.
 *         : line_num - The number of lines displayed so far in the console.
 * OUTPUT  : The tagged status of the destination port.
 * RETURN  : line_num - The number of lines displayed so far in the console.
 * NOTE    : This function is called by cli_api_Show_One_Rspan_Session().
 *           7-24-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
static UI32_T cli_api_Show_Rspan_Destination_Tagged_Mode(char destination, char is_tagged, UI32_T line_num)
{
    UI8_T buff[CLI_DEF_MAX_BUFSIZE] = {0};

    CLI_LIB_PrintStr("Destination Tagged Mode\t\t: ");

    /* Since Mib setting is by field, we don't need to consider dst interface when displaying the dst tag mode. 2008.1.11. by Tien.
     */

    /* If tagged is none, the value is zero for core, but for leaf_XXX is VAL_rspanDstPortTag_none. 2008.1.11. by Tien.
     */
    if (is_tagged == 0)
    {
        is_tagged = VAL_rspanDstPortTag_none ;
    }

    switch(is_tagged)
    {
        /* Due to new mib design, we have to addd one extra case for dst tagged field. 2008.1.11. by Tien.
         */
    	case VAL_rspanDstPortTag_none:
            CLI_LIB_PrintStr("None");
    		break;
    	case VAL_rspanDstPortTag_tagged:
            CLI_LIB_PrintStr("Tagged");
    		break;
    	case VAL_rspanDstPortTag_untagged:
    	    CLI_LIB_PrintStr("Untagged");
    		break;
    	default:
    	    CLI_LIB_PrintStr("Untagged");
    	 break;
    }
    /* If tagged is none, the value is zero for core, but for leaf_XXX is VAL_rspanDstPortTag_none. 2008.1.11. by Tien.
     */

    PROCESS_MORE_FUNC("\r\n");

    return line_num;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - cli_api_Show_Rspan_Operation_Status
 *-----------------------------------------------------------
 * PURPOSE : This function shows the operation status of the Rspan session.
 * INPUT   : session_id - The Rspan session ID, currently 1 or 2.
 *         : line_num - The number of lines displayed so far in the console.
 * OUTPUT  : The operation status of the Rspan session.
 * RETURN  : line_num - The number of lines displayed so far in the console.
 * NOTE    : This function is called by cli_api_Show_One_Rspan_Session().
 *           7-24-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
static UI32_T cli_api_Show_Rspan_Operation_Status(char session_id, UI32_T line_num)
{
    UI8_T buff[CLI_DEF_MAX_BUFSIZE] = {0};

    CLI_LIB_PrintStr("Operation Status \t\t: ");

    if (RSPAN_PMGR_IsSessionEntryCompleted (session_id) == TRUE)
    {
        CLI_LIB_PrintStr("Up");
    }
    else
    {
        CLI_LIB_PrintStr("Down");
    }

    PROCESS_MORE_FUNC("\r\n");

    return line_num;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - cli_api_Build_Port_ID_List
 *-----------------------------------------------------------
 * PURPOSE : This function builds the port ID list for the Rspan session.
 *           The purpose of building the port ID list is to accommodate a greater amount of port extension
 *           in the future.  The port ID list itself is a port bitmap accessible through bitwise operations.
 *           Otherwise, the port list argument is good enough for the subsequent processing of the Rspan session.
 *           A simple way of describing the operation is to turn on the corresponding bit in the port ID list structure if the port is selected.
 * INPUT   : unit - The unit ID of the switch.
 *         : port_id_list_ar[] - The port ID list for the Rspan session.
 *         : port_list_argument_p - The pointer to the port list argument obtained from the command line.
 * OUTPUT  : None.
 * RETURN  : TRUE - success in operations.
 *           FALSE - failure in operations.
 * NOTE    : This function is called by CLI_API_Rspan().
 *           7-24-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
static char cli_api_Build_Port_ID_List(UI32_T unit, UI8_T port_id_list_ar[], char *port_list_argument_p)
{
    UI32_T lower_val=0;
    UI32_T upper_val=0;
    UI32_T err_idx;
    char   *s_p;
    char   delimiter_ar[2]={0};
    char   token_ar[CLI_DEF_MAX_BUFSIZE]={0};
    int    k;

    memset(port_id_list_ar, 0, sizeof(UI8_T)*SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST);
    s_p=port_list_argument_p;
    s_p=strchr(s_p, '/')+1;
    delimiter_ar[0]=',';

    while(s_p!=NULL)
    {
    	memset(token_ar, 0, sizeof(token_ar));
        s_p=CLI_LIB_Get_Token(s_p, token_ar, delimiter_ar);
        CLI_LIB_Get_Lower_Upper_Value(token_ar, &lower_val, &upper_val, &err_idx);
        for(k=lower_val; k<=upper_val; k++)
        {
        	if(SWCTRL_POM_UIUserPortExisting(unit, k))
        	{
        	    port_id_list_ar[(UI32_T)((k-1)/8)] |= (1 << (7 -((k-1)%8)));
        	}
        	else
        	{
        	    return CLI_NO_ERROR;
        	}
        }
    }

    return CLI_NO_ERROR;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - cli_api_Get_Rspan_Source_Mode
 *-----------------------------------------------------------
 * PURPOSE : This function obtains the source port mode for the Rspan session through the command line input argument.
 * INPUT   : mode_argument_p - The pointer to source mode argument obtained from the command line.
 * OUTPUT  : None.
 * RETURN  : Source mode of the Rspan session.  The return value can be one of the following.
 *           VAL_mirrorType_rx,
 *           VAL_mirrorType_tx,
 *           VAL_mirrorType_both.
 * NOTE    : This function is called by cli_api_Set_Rspan_Source_Interface().
 *           7-24-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
static char cli_api_Get_Rspan_Source_Mode(char *mode_argument_p)
{
	UI8_T mode;

	if(mode_argument_p==NULL)
    {
        mode = VAL_mirrorType_both;
    }
    else
    {
        switch(*mode_argument_p)
        {
            case 'r':
            case 'R':
                mode = VAL_mirrorType_rx;
                break;
            case 't':
            case 'T':
                mode = VAL_mirrorType_tx;
                break;
            case 'b':
            case 'B':
                mode = VAL_mirrorType_both;
                break;
            default:
                mode = VAL_mirrorType_both;
                break;
        }
    }
    return mode;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - cli_api_Set_Rspan_Source_Interface
 *-----------------------------------------------------------
 * PURPOSE : This function configures (sets) the source ports for the Rspan session.
 * INPUT   : session_id - The Rspan session ID, currently 1 or 2.
 *         : max_port_number - The maximum number of ports allowed in the Rspan session.
 *         : port_id_list_ar[] - The port ID list for the Rspan session.
 *         : unit - The unit ID of the switch.
 *         : mode_argument_p - The pointer to source mode argument obtained from the command line.
 * OUTPUT  : Error messages upon failure.  None otherwise.
 * RETURN  : TRUE - success in operations.
 *           FALSE - failure in operations.
 * NOTE    : This function is called by CLI_API_Rspan().
 *           7-24-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
static char cli_api_Set_Rspan_Source_Interface(char session_id, UI32_T max_port_number, UI8_T port_id_list_ar[], UI32_T unit, char *mode_argument_p)
{
    CLI_API_EthStatus_T ret;
    UI32_T ifindex=0;
    char problem_port_ar[CLI_DEF_MAX_BUFSIZE]={0};
    UI8_T source_port;
    UI8_T mode;
    BOOL_T first_entry=TRUE;
    int k;

	for (k=1 ; k<=max_port_number ; k++)
    {
        source_port=k;
        if (port_id_list_ar[(UI32_T)((k-1)/8)] & (1 << (7-((k-1)%8))) )
        {
            if((ret=verify_ethernet(unit, source_port, &ifindex)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(ret, unit, source_port);
                return CLI_NO_ERROR;
            }
            else
            {
                mode=cli_api_Get_Rspan_Source_Mode(mode_argument_p);
                if(!RSPAN_PMGR_SetSessionSourceInterface(session_id, ifindex, mode))
                {
                	if(first_entry)
                    {
                        sprintf(problem_port_ar, "Eth %lu/%u", (unsigned long)unit, source_port);
                        first_entry=FALSE;
                    }
                    else
                    {
                        sprintf(problem_port_ar, "%s,%u", problem_port_ar, source_port);
                    }
                }
            }
        }
    }

    /* Don't need to jump out when finding the first error. 2008.1.9 by Tien.
     */
    if(strlen(problem_port_ar)>0)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_1("Failed to configure the RSPAN source at %s.\r\n", problem_port_ar);
#endif
        return CLI_NO_ERROR;
    }

	return CLI_NO_ERROR;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - cli_api_Unset_Rspan_Source_Interface
 *-----------------------------------------------------------
 * PURPOSE : This function configures (unsets) the source ports for the Rspan session.
 * INPUT   : session_id - The Rspan session ID, currently 1 or 2.
 *         : max_port_number - The maximum number of ports allowed in the Rspan session.
 *         : port_id_list_ar[] - The port ID list for the Rspan session.
 *         : unit - The unit ID of the switch.
 *         : mode_argument_p - The pointer to source mode argument obtained from the command line.
 * OUTPUT  : Error messages upon failure.  None otherwise.
 * RETURN  : TRUE - success in operations.
 *           FALSE - failure in operations.
 * NOTE    : This function is called by CLI_API_Rspan().
 *           7-24-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
static char cli_api_Unset_Rspan_Source_Interface(char session_id, UI32_T max_port_number, UI8_T port_id_list_ar[], UI32_T unit, char *mode_argument_p)
{
    CLI_API_EthStatus_T ret;
    UI32_T ifindex=0;
	char problem_port_ar[CLI_DEF_MAX_BUFSIZE]={0};
    UI8_T source_port;
    UI8_T mode;
	BOOL_T first_entry=TRUE;
    int k;

	for (k=1 ; k<=max_port_number ; k++)
    {
        source_port=k;
        if (port_id_list_ar[(UI32_T)((k-1)/8)] & (1 << (7-((k-1)%8))) )
        {
            if((ret=verify_ethernet(unit, source_port, &ifindex)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(ret, unit, source_port);
                return CLI_NO_ERROR;
            }
            else
            {
                mode=cli_api_Get_Rspan_Source_Mode(mode_argument_p);
                if(!RSPAN_PMGR_DeleteSessionSourceInterface(session_id, ifindex, mode))
                {
                	if(first_entry)
                    {
                        sprintf(problem_port_ar, "Eth %lu/%u", (unsigned long)unit, source_port);
                        first_entry=FALSE;
                    }
                    else
                    {
                        sprintf(problem_port_ar, "%s,%u", problem_port_ar, source_port);
                    }
                }
            }
        }
    }
    if(strlen(problem_port_ar)>0)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_1("Failed to delete the RSPAN source at %s.\r\n", problem_port_ar);
#endif
        return CLI_NO_ERROR;
    }

	return CLI_NO_ERROR;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - cli_api_Get_Rspan_Destination_Mode
 *-----------------------------------------------------------
 * PURPOSE : This function obtains the destination port mode for the Rspan session through the command line input argument.
 * INPUT   : mode_argument_p - The pointer to destination mode argument obtained from the command line.
 * OUTPUT  : None.
 * RETURN  : Destination port mode of the Rspan session.  The return value can be one of the following.
 *           VAL_rspanDstPortTag_tagged,
 *           VAL_rspanDstPortTag_untagged.
 * NOTE    : This function is called by cli_api_Set_Rspan_Destination_Interface().
 *           7-24-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
static UI8_T cli_api_Get_Rspan_Destination_Mode(char *mode_argument_p)
{
	UI8_T mode;

	if(mode_argument_p==NULL)
    {
        mode = VAL_rspanDstPortTag_untagged;
    }
    else
    {
        switch(*mode_argument_p)
        {
            case 't':
            case 'T':
                mode = VAL_rspanDstPortTag_tagged;
                break;
            case 'u':
            case 'U':
                mode = VAL_rspanDstPortTag_untagged;
                break;
            default:
                mode = VAL_rspanDstPortTag_untagged;
                break;
        }
    }
    return mode;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - cli_api_Set_Rspan_Destination_Interface
 *-----------------------------------------------------------
 * PURPOSE : This function configures (sets) the destination port for the Rspan session.
 * INPUT   : session_id - The Rspan session ID, currently 1 or 2.
 *         : max_port_number - The maximum number of ports allowed in the Rspan session.
 *         : port_id_list_ar[] - The port ID list for the Rspan session.
 *         : unit - The unit ID of the switch.
 *         : mode_argument_p - The pointer to destination mode argument obtained from the command line.
 * OUTPUT  : Error messages upon failure.  None otherwise.
 * RETURN  : TRUE - success in operations.
 *           FALSE - failure in operations.
 * NOTE    : This function is called by CLI_API_Rspan().
 *           7-24-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
static char cli_api_Set_Rspan_Destination_Interface(char session_id, UI32_T max_port_number, UI8_T port_id_list_ar[], UI32_T unit, char *mode_argument_p)
{
    CLI_API_EthStatus_T ret;
    UI32_T ifindex=0;
    char problem_port_ar[CLI_DEF_MAX_BUFSIZE]={0};
    UI8_T destination_port;
    UI8_T is_tagged;
    BOOL_T first_entry=TRUE;
    int k;


	for (k=1 ; k<=max_port_number ; k++)
    {
        destination_port=k;
        if (port_id_list_ar[(UI32_T)((k-1)/8)] & (1 << (7-((k-1)%8))) )
        {
            if((ret=verify_ethernet(unit, destination_port, &ifindex)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(ret, unit, destination_port);
                return CLI_NO_ERROR;
            }
            else
            {
                is_tagged=cli_api_Get_Rspan_Destination_Mode(mode_argument_p);
                if(!RSPAN_PMGR_SetSessionDestinationInterface(session_id, ifindex, is_tagged))
                {
                    if(first_entry)
                    {
                        sprintf(problem_port_ar, "Eth %lu/%u", (unsigned long)unit, destination_port);
                        first_entry=FALSE;
                    }
                    else
                    {
                        sprintf(problem_port_ar, "%s,%u", problem_port_ar, destination_port);
                    }
                }
            }
        }
    }

    /* Don't need to jump out when finding the first error. 2008.1.9 by Tien.
     */
    if(strlen(problem_port_ar)>0)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_1("Failed to configure the RSPAN destination port at %s.\r\n", problem_port_ar);
#endif
        return CLI_NO_ERROR;
    }

	return CLI_NO_ERROR;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - cli_api_Unset_Rspan_Destination_Interface
 *-----------------------------------------------------------
 * PURPOSE : This function configures (unsets) the destination port for the Rspan session.
 * INPUT   : session_id - The Rspan session ID, currently 1 or 2.
 *         : max_port_number - The maximum number of ports allowed in the Rspan session.
 *         : port_id_list_ar[] - The port ID list for the Rspan session.
 *         : unit - The unit ID of the switch.
 * OUTPUT  : Error messages upon failure.  None otherwise.
 * RETURN  : TRUE - success in operations.
 *           FALSE - failure in operations.
 * NOTE    : This function is called by CLI_API_Rspan().
 *           7-24-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
static char cli_api_Unset_Rspan_Destination_Interface(char session_id, UI32_T max_port_number, UI8_T port_id_list_ar[], UI32_T unit)
{
    CLI_API_EthStatus_T ret;
    UI32_T ifindex=0;
	char problem_port_ar[CLI_DEF_MAX_BUFSIZE]={0};
    UI8_T destination_port;
	BOOL_T first_entry=TRUE;
    int k;

	for (k=1 ; k<=max_port_number ; k++)
    {
        destination_port=k;
        if (port_id_list_ar[(UI32_T)((k-1)/8)] & (1 << (7-((k-1)%8))) )
        {
            if((ret=verify_ethernet(unit, destination_port, &ifindex)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(ret, unit, destination_port);
                return CLI_NO_ERROR;
            }
            else
            {
                if(!RSPAN_PMGR_DeleteSessionDestinationInterface(session_id, ifindex))
                {
                    if(first_entry)
                    {
                        sprintf(problem_port_ar, "Eth %lu/%u", (unsigned long)unit, destination_port);
                        first_entry=FALSE;
                    }
                    else
                    {
                        sprintf(problem_port_ar, "%s,%u", problem_port_ar, destination_port);
                    }
                }
            }
        }
    }

    if(strlen(problem_port_ar)>0)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_1("Failed to delete the RSPAN destination at %s\r\n", problem_port_ar);
#endif
        return CLI_NO_ERROR;
    }
	return CLI_NO_ERROR;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - cli_api_Get_Rspan_Remote_Vlan_Switch_Role
 *-----------------------------------------------------------
 * PURPOSE : This function obtains the remote VLAN switch role for the Rspan session through the command line input argument.
 * INPUT   : switch_role_argument_p - The pointer to switch role argument obtained from the command line.
 * OUTPUT  : None.
 * RETURN  : Remote VLAN switch role of the Rspan session.  The return value can be one of the following.
 *           VAL_rspanSwitchRole_source,
 *           VAL_rspanSwitchRole_intermediate,
 *           VAL_rspanSwitchRole_destination.
 * NOTE    : This function is called by cli_api_Set_Rspan_Remote_Vlan().
 *           7-24-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
static char cli_api_Get_Rspan_Remote_Vlan_Switch_Role(char *switch_role_argument_p)
{
	UI8_T switch_role;

    switch(*switch_role_argument_p)
    {
        case 's':
        case 'S':
            switch_role=VAL_rspanSwitchRole_source;
            break;

         case 'i':
         case 'I':
             switch_role=VAL_rspanSwitchRole_intermediate;
             break;

         case 'd':
         case 'D':
             switch_role=VAL_rspanSwitchRole_destination;
             break;

         default:
             switch_role=VAL_rspanSwitchRole_source;
             break;
    }
    return switch_role;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - cli_api_Set_Rspan_Remote_Vlan
 *-----------------------------------------------------------
 * PURPOSE : This function configures (sets) the remote VLAN for the Rspan session.
 * INPUT   : session_id - The Rspan session ID, currently 1 or 2.
 *         : switch_role_argument_p - The pointer to switch role argument obtained from the command line.
 *         : remote_vid - The remote VLAN ID of the Rspan session.
 *         : max_port_number - The maximum number of ports allowed in the Rspan session.
 *         : port_id_list_ar[] - The port ID list for the Rspan session.
 *         : unit - The unit ID of the switch.
 * OUTPUT  : Error messages upon failure.  None otherwise.
 * RETURN  : TRUE - success in operations.
 *           FALSE - failure in operations.
 * NOTE    : This function is called by CLI_API_Rspan().
 *           7-24-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
static char cli_api_Set_Rspan_Remote_Vlan(char session_id, char *switch_role_argument_p, UI32_T remote_vid, UI32_T max_port_number, UI8_T port_id_list_ar[], UI32_T unit)
{
    CLI_API_EthStatus_T ret;
    UI32_T ifindex=0;
    char problem_port_ar[CLI_DEF_MAX_BUFSIZE]={0};
    UI8_T port;
    UI8_T switch_role;
    BOOL_T first_entry=TRUE;
    int k;

	for (k=1 ; k<=max_port_number ; k++)
    {
        port=k;
        if (port_id_list_ar[(UI32_T)((k-1)/8)] & (1 << (7-((k-1)%8))) )
        {
            if((ret=verify_ethernet(unit, port, &ifindex)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(ret, unit, port);
                return CLI_NO_ERROR;
            }
            else
            {
            	switch_role=cli_api_Get_Rspan_Remote_Vlan_Switch_Role(switch_role_argument_p);

                if(!RSPAN_PMGR_SetSessionRemoteVlan(session_id, switch_role, remote_vid, ifindex))
                {
                    if(first_entry)
                    {
                        sprintf(problem_port_ar, "Eth %lu/%u", (unsigned long)unit, port);
                        first_entry=FALSE;
                    }
                    else
                    {
                        sprintf(problem_port_ar, "%s,%u", problem_port_ar, port);
                    }
                }
            }
        }
    }

    /* Don't need to jump out when finding the first error. 2008.1.9 by Tien.
     */
    if(strlen(problem_port_ar)>0)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_1("Failed to set the RSPAN remote vlan and uplink port at %s.\r\n", problem_port_ar);
#endif
        return CLI_NO_ERROR;
    }

    return CLI_NO_ERROR;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - cli_api_Unset_Rspan_Remote_Vlan
 *-----------------------------------------------------------
 * PURPOSE : This function configures (unsets) the remote VLAN for the Rspan session.
 * INPUT   : session_id - The Rspan session ID, currently 1 or 2.
 *         : switch_role_argument_p - The pointer to switch role argument obtained from the command line.
 *         : remote_vid - The remote VLAN ID of the Rspan session.
 *         : max_port_number - The maximum number of ports allowed in the Rspan session.
 *         : port_id_list_ar[] - The port ID list for the Rspan session.
 *         : unit - The unit ID of the switch.
 * OUTPUT  : Error messages upon failure.  None otherwise.
 * RETURN  : TRUE - success in operations.
 *           FALSE - failure in operations.
 * NOTE    : This function is called by CLI_API_Rspan().
 *           7-24-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
static char cli_api_Unset_Rspan_Remote_Vlan(char session_id, char *switch_role_argument_p, UI32_T remote_vid, UI32_T max_port_number, UI8_T port_id_list_ar[], UI32_T unit)
{
    CLI_API_EthStatus_T ret;
    UI32_T ifindex=0;
	char problem_port_ar[CLI_DEF_MAX_BUFSIZE]={0};
    UI8_T port;
    UI8_T switch_role;
	BOOL_T first_entry=TRUE;
    int k;

	for (k=1 ; k<=max_port_number ; k++)
    {
        port=k;
        if (port_id_list_ar[(UI32_T)((k-1)/8)] & (1 << (7-((k-1)%8))) )
        {
            if((ret=verify_ethernet(unit, port, &ifindex)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(ret, unit, port);
                return CLI_NO_ERROR;
            }
            else
            {
            	switch_role=cli_api_Get_Rspan_Remote_Vlan_Switch_Role(switch_role_argument_p);
                if(!RSPAN_PMGR_DeleteSessionRemoteVlan(session_id, switch_role, remote_vid, ifindex))
                {
                    if(first_entry)
                    {
                        sprintf(problem_port_ar, "Eth %lu/%u", (unsigned long)unit, port);
                        first_entry=FALSE;
                    }
                    else
                    {
                        sprintf(problem_port_ar, "%s,%u", problem_port_ar, port);
                    }
                }
            }
        }
    }
    if(strlen(problem_port_ar)>0)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_1("Failed to delete the RSPAN remote vlan and uplink port at %s.\r\n", problem_port_ar);
#endif
        return CLI_NO_ERROR;
    }

	return CLI_NO_ERROR;
}
#endif

/*-----------------------------------------------------------
 * ROUTINE NAME - CLI_API_Rspan
 *-----------------------------------------------------------
 * PURPOSE : This is the public handle to execute the RSPAN configuration.
 * INPUT   : cmd_idx - global command index.
 *         : arg - CLI argument pointer.
 *         : ctrl_P - CLI environment pointer
 * OUTPUT  : Rspan session configuration.
 * RETURN  : TRUE - success in operations.
 *           FALSE - failure in operations.
 * NOTE    : 3-31-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
UI32_T CLI_API_Rspan(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RSPAN == TRUE)
    UI32_T unit;
    UI32_T remote_vid;
    UI8_T session_id;
    unsigned long ul_temp;

    session_id=atoi(arg[0]);

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_RSPAN_SESSION:
             switch(*arg[1])
             {
                 case 's':
                 case 'S':
                 	 sscanf(arg[4], "%lu/", &ul_temp);
                      unit = (UI32_T)ul_temp;
                 	 cli_api_Build_Port_ID_List(unit, ctrl_P->CMenu.port_id_list, arg[4]);
                     cli_api_Set_Rspan_Source_Interface(session_id, ctrl_P->sys_info.max_port_number, ctrl_P->CMenu.port_id_list, unit, arg[5]);
                     break;

                 case 'r':
                 case 'R':
                      sscanf(arg[3], "%lu", &ul_temp);
                      remote_vid = (UI32_T)ul_temp;
                 	 sscanf(arg[7], "%lu/", &ul_temp);
                      unit=(UI32_T)ul_temp;
                 	 cli_api_Build_Port_ID_List(unit, ctrl_P->CMenu.port_id_list, arg[7]);
                 	 cli_api_Set_Rspan_Remote_Vlan(session_id, arg[4], remote_vid, ctrl_P->sys_info.max_port_number, ctrl_P->CMenu.port_id_list, unit);
                     break;

                 case 'd':
                 case 'D':
                 	 sscanf(arg[4], "%lu/", &ul_temp);
                      unit = (UI32_T)ul_temp;
                 	 cli_api_Build_Port_ID_List(unit, ctrl_P->CMenu.port_id_list, arg[4]);
                     cli_api_Set_Rspan_Destination_Interface(session_id, ctrl_P->sys_info.max_port_number, ctrl_P->CMenu.port_id_list, unit, arg[5]);
                     break;
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_RSPAN_SESSION:
            /* Add a new command for removing a session id. 2008.1.10 by Tien.
             */
            if (arg[1] == NULL)
            {
                RSPAN_PMGR_RemoveCliWebSessionId ( session_id );
            }
            else
            {
                 switch(*arg[1])
                 {
                     case 's':
                     case 'S':
                     	 sscanf(arg[4], "%lu/", &ul_temp);
                             unit = (UI32_T)ul_temp;
                     	 cli_api_Build_Port_ID_List(unit, ctrl_P->CMenu.port_id_list, arg[4]);
                         cli_api_Unset_Rspan_Source_Interface(session_id, ctrl_P->sys_info.max_port_number, ctrl_P->CMenu.port_id_list, unit, arg[5]);
                         break;

                     case 'r':
                     case 'R':
                             sscanf(arg[3], "%lu", &ul_temp);
                             remote_vid = (UI32_T)ul_temp;
                     	 sscanf(arg[7], "%lu/", &ul_temp);
                             unit = (UI32_T)ul_temp;
                     	 cli_api_Build_Port_ID_List(unit, ctrl_P->CMenu.port_id_list, arg[7]);
                     	 cli_api_Unset_Rspan_Remote_Vlan(session_id, arg[4], remote_vid, ctrl_P->sys_info.max_port_number, ctrl_P->CMenu.port_id_list, unit);
                         break;

                     case 'd':
                     case 'D':
                     	 sscanf(arg[4], "%lu/", &ul_temp);
                             unit = (UI32_T)ul_temp;
                     	 cli_api_Build_Port_ID_List(unit, ctrl_P->CMenu.port_id_list, arg[4]);
                         cli_api_Unset_Rspan_Destination_Interface(session_id, ctrl_P->sys_info.max_port_number, ctrl_P->CMenu.port_id_list, unit);
                         break;
                }
                /* return CLI_NO_ERROR;
                 */
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif
    return CLI_NO_ERROR;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - CLI_API_Show_Rspan
 *-----------------------------------------------------------
 * PURPOSE : This is the public handle to show the Rspan session configuration.
 * INPUT   : cmd_idx - global command index.
 *         : arg - CLI argument pointer.
 *         : ctrl_P - CLI environment pointer.
 * OUTPUT  : Rspan session configuration.
 * RETURN  : TRUE - success in operations.
 *           FALSE - failure in operations.
 * NOTE    : 3-31-2008, Kwok-yam TAM.
 *----------------------------------------------------------
 */
UI32_T CLI_API_Show_Rspan(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RSPAN == TRUE)
    RSPAN_OM_SessionEntry_T rspan_session_entry;
    UI32_T line_num = 0;
    UI32_T max_port_number;
    UI32_T unit;
    UI8_T session_id;
    BOOL_T empty_session;

    memset(&rspan_session_entry , 0 , sizeof(RSPAN_OM_SessionEntry_T));
    max_port_number=ctrl_P->sys_info.max_port_number;
    STKTPLG_POM_GetMyUnitID(&unit);

    if (arg[0] != NULL)
    {
    	session_id = atoi(arg[0]);
        if (!RSPAN_PMGR_GetRspanSessionEntry(session_id, &rspan_session_entry))
    	{
            CLI_LIB_PrintStr_1("No data for RSPAN session %d.\r\n", session_id);
    	    return CLI_NO_ERROR;
    	}

        if ((line_num = cli_api_Show_One_Rspan_Session(session_id, rspan_session_entry, max_port_number, unit, line_num)) == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }
    }
    else
    {
        session_id = 0;
        empty_session = TRUE;
        while (RSPAN_PMGR_GetNextRspanSessionEntry(&session_id, &rspan_session_entry))
        {
    	    empty_session=FALSE;
            if ((line_num = cli_api_Show_One_Rspan_Session(session_id, rspan_session_entry, max_port_number, unit, line_num)) == JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
        }
        if (empty_session)
        {
            CLI_LIB_PrintStr("No data for RSPAN sessions.\r\n");
        }
    }
#endif
    return CLI_NO_ERROR;
}
