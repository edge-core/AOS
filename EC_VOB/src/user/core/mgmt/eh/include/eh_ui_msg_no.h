/* MODULE NAME:  eh_ui_msg_no.h
 * PURPOSE:
 *  This header file contains the constant definitions for UI Message ID.
 *
 * NOTES:
 *  1. This header is included by eh_type.h ONLY. Do not include other header
 *     files in this file.
 *  2. ONLY uses the macro function defined in eh_type.h in this file.
 *  3. ONLY defines the constant for UI Message ID in this file.
 *  4. Please follow the template for definitions of UI Message ID shown below.
 *  5. Always add the new UI Message Number from the end of the current max
 *     UI Message Number.
 *  6. The entity name used in eh_msg.xml and eh_ui_msg_no.dtd must be the same
 *     with the constant name defined in this file. For example, given that
 *     EH_UI_MSG_NO_SWCTRL_OUT_OF_RANGE is defined, in eh_msg.xml shall define
 *     as following to refer to this UI Message ID.
 *     <UI_Msg_No>&EH_UI_MSG_NO_SWCTRL_OUT_OF_RANGE;</UI_Msg_No>
 *     And in eh_ui_msg_no.dtd shall define as following.
 *     <!ENTITY EH_UI_MSG_NO_SWCTRL_OUT_OF_RANGE "24000">
 *
 *  7. Naming rule of UI Message ID:
 *     Example: EH_UI_MSG_NO_SWCTRL_OUT_OF_RANGE
 *         Prefix part - "EH_UI_MSG_NO_SWCTRL_"
 *                       Prefix name of each ID. Note that ¡§SWCTRL¡¨ is a CSC
 *                       Name. UI MSG ID defined for other CSC shall use its CSC
 *                       Name in this Prefix part.
 *         Non-prefix part - "OUT_OF_RANGE"
 *                           Descriptive name for the error message ID. Defined
 *                           by each CSC designer.
 *
 * HISTORY
 *      Date        -- Modifier,        Reason
 *      2013.02.08  -- Charlie Chen     Create.

 * Copyright(C)      EdgeCore Newtorks, 2013
 */
#ifndef _EH_UI_MSG_NO_H
#define _EH_UI_MSG_NO_H

#if 0 /* template for definitions of UI Message ID */
/* SWCTRL UI Message Number ------------------------------------------- START */
    /* Spaces are appended in the beginning of the following lines on purpose to
     * avoid being converted by the script "gen_ui_msg_no_dtd.sh" to generated
     * output file "eh_ui_msg_no.dtd"
     *
     * NOTE: FORMAL CONSTANT DEFINITIONS must not have spaces in the beginning
     *       of each line.
     */
    #define EH_UI_MSG_NO_SWCTRL_BASE EH_TYPE_GET_CSC_UI_MSG_ID_MIN(SYS_MODULE_SWCTRL)
    #define EH_UI_MSG_NO_SWCTRL_OUT_OF_RANGE EH_UI_MSG_NO_SWCTRL_BASE
    #define EH_UI_MSG_NO_SWCTRL_INVALID_IP   EH_UI_MSG_NO_SWCTRL_BASE+1
/* SWCTRL UI Message Number ------------------------------------------- END   */
#endif /* end of #if 0 */
/*-------------ADD FORMAL UI MESSAGE FOR EACH CSC AFTER THIS LINE-------------*/

#if 1 /* These definitions are for doing EH unit test only and shall be removed when SYS_CPNT_EH is adopted formally by the platform */
#define EH_UI_MSG_NO_SWCTRL_BASE EH_TYPE_GET_CSC_UI_MSG_ID_MIN(SYS_MODULE_SWCTRL)
#define EH_UI_MSG_NO_SWCTRL_OUT_OF_RANGE EH_UI_MSG_NO_SWCTRL_BASE
#define EH_UI_MSG_NO_SWCTRL_INVALID_IP   EH_UI_MSG_NO_SWCTRL_BASE+1
#define EH_UI_MSG_NO_SWCTRL_TEST_ARG1    EH_UI_MSG_NO_SWCTRL_BASE+2
#define EH_UI_MSG_NO_SWCTRL_TEST_ARG2    EH_UI_MSG_NO_SWCTRL_BASE+3
#define EH_UI_MSG_NO_SWCTRL_TEST_ARG3    EH_UI_MSG_NO_SWCTRL_BASE+4
#endif
#endif /* end of #ifndef _EH_UI_MSG_NO_H */
