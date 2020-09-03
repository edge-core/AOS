/*************************************************************************
 *
 *            Copyright (c) 2008 by Microsemi Corp. Inc.
 *
 *  This software is copyrighted by, and is the sole property of Microsemi
 *  Corp. All rights, title, ownership, or other interests in the
 *  software  remain the property of Microsemi Corp. This software
 *  may only be used in accordance with the corresponding license
 *  agreement.  Any unauthorized use, duplication, transmission,
 *  distribution, or disclosure of this software is expressly forbidden.
 *
 *  This Copyright notice may not be removed or modified without prior
 *  written consent of Microsemi Corp.
 *
 *  Microsemi Corp. reserves the right to modify this software without
 *  notice. 
 *
 *************************************************************************
 *
 *  File Revision: 1.0 
 *
 *************************************************************************  
 *
 *  Description: the file contain the definitions of system functions. 
 *
 *************************************************************************/

#ifndef _MSCC_ARC_H_
	#define _MSCC_ARC_H_

	#ifdef __cplusplus
		extern "C" {
	#endif

	#include "mscc_poe_global_types.h"
			
	S32 OS_Sleep_mS(_IN U16 sleepTime_mS);	
	
	S32 OS_mutex_init();
	S32 OS_mutex_lock();   	    	    
	S32 OS_mutex_unlock();	            		
	    
	#ifdef __cplusplus
		}
	#endif
		
#endif /* _MSCC_ARC_H_ */
