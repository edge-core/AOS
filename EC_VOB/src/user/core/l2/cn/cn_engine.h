/* MODULE NAME - CN_ENGINE.H
 * PURPOSE : Provides the declarations for CN state machine.
 * NOTES   : None.
 * HISTORY : 2012/09/12 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2012
 */

#ifndef CN_ENGINE_H
#define CN_ENGINE_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */

/* events used to trigger state transition */
enum CN_ENGINE_EVENT_E
{
    CN_ENGINE_EVENT_RCV_CNPV_EV,
    CN_ENGINE_EVENT_NOT_RCV_CNPV_EV,
    CN_ENGINE_EVENT_RCV_READY_EV,
    CN_ENGINE_EVENT_NOT_RCV_READY_EV,
    CN_ENGINE_EVENT_ADMIN_EV,
    CN_ENGINE_EVENT_AUTO_EV,
    CN_ENGINE_EVENT_BEGIN,
};

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME - CN_ENGINE_ProgressStateMachine
 * PURPOSE : Progress one state machine given an event.
 * INPUT   : priority - a CNPV
 *           lport    - a logical port number
 *           event    - CN_ENGINE_EVENT_E
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_ENGINE_ProgressStateMachine(UI32_T priority, UI32_T lport, UI32_T event);

/* FUNCTION NAME - CN_ENGINE_ResetStateMachine
 * PURPOSE : Reset one state machine.
 * INPUT   : priority - a CNPV
 *           lport    - a logical port number
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_ENGINE_ResetStateMachine(UI32_T priority, UI32_T lport);

/* FUNCTION NAME - CN_ENGINE_CnTagRemovalBitmap
 * PURPOSE : Get the CN_TAG removal bitmap for a lport.
 * INPUT   : lport - the specified logical port
 * OUTPUT  : None
 * RETURN  : CN_TAG removal bitmap (each bit for a priority)
 * NOTES   : Only used by CN backdoor
 */
UI8_T CN_ENGINE_CnTagRemovalBitmap(UI32_T lport);

/* FUNCTION NAME - CN_ENGINE_L4RegisterCount
 * PURPOSE : Get the L4 register count for a lport.
 * INPUT   : lport - the specified logical port
 * OUTPUT  : None
 * RETURN  : L4 register count
 * NOTES   : Only used by CN backdoor
 */
UI32_T CN_ENGINE_L4RegisterCount(UI32_T lport);

#endif /* End of CN_ENGINE_H */
