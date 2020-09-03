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
 *  Description: contain the implementation of system depended functions.
 *
 *************************************************************************/

#include "mscc_arch_functions.h"
#include "sysfun.h"

/*=========================================================================
/ Define here the Arcitecture 
/========================================================================*/         

/*#define _LINUX_PC_ARC_*/
#define _new_ARC_

#ifdef _LINUX_PC_ARC_
	
	#include <pthread.h>
    #include <unistd.h>

	static pthread_mutex_t sharedVariableMutex = PTHREAD_MUTEX_INITIALIZER;
	
/*---------------------------------------------------------------------        												                                                                                                                                           
 *    description:     Sleep function 	 				 	 			       
 *		            							     				   
 *    input :   sleepTime_mS  - sleep value in mili Seconds
 * 								minimum required range is: 20 mili seconds to 50 mili seconds  
 * 								with resulution of 10 mili second                                                  	   
 * 
 *    output:   none                   	             
 *                     
 *    return:   e_POE_STATUS_OK                         - operation succeed 
 * 			    e_POE_STATUS_ERR_SLEEP_FUNCTION_ERROR   - operation failed due to usleep function operation error                                    
 *---------------------------------------------------------------------*/
    S32 OS_Sleep_mS(U16 sleepTime_mS)
    {
    	S32 status_number = 0;
    	status_number = usleep(sleepTime_mS*1000);         
        if(status_number != e_POE_STATUS_OK)        
        	return e_POE_STATUS_ERR_SLEEP_FUNCTION_ERROR;                          	        
        
        return e_POE_STATUS_OK;
    }
   
/*---------------------------------------------------------------------        												                                                                                                                                           
 *    description:     initialize the mutex 	 				 	 			       
 *		            							     				   
 *    input :   none                                                    	   
 * 
 *    output:   none                   	             
 *                     
 *    return:   e_POE_STATUS_OK                     - operation succeed 
 * 			    e_POE_STATUS_ERR_MUTEX_INIT_ERROR   - operation failed due to mutex initialize operation error 
 *---------------------------------------------------------------------*/  	        
    S32 OS_mutex_init() 
    {    	
    	S32 status_number = 0;
    	
    	/* initializes the mutex */
    	status_number = pthread_mutex_init(&sharedVariableMutex, NULL);
    	if(status_number != e_POE_STATUS_OK)
    		return e_POE_STATUS_ERR_MUTEX_INIT_ERROR;                          	        
    	        
    	return e_POE_STATUS_OK;
    }
            
    
/*---------------------------------------------------------------------        												                                                                                                                                           
 *    description:  locking a mutex					      	 				 	 			       
 *		            							     				   
 *    input :   none                                                    	   
 * 
 *    output:   none                   	             
 *                     
 *    return:   e_POE_STATUS_OK                       - operation succeed 
 * 			    e_POE_STATUS_ERR_MUTEX_LOCK_ERROR     - operation failed due to mutex lock operation error
 * 				e_POE_STATUS_ERR_SLEEP_FUNCTION_ERROR - operation failed due to usleep function operation error                                     
 *---------------------------------------------------------------------*/
    S32 OS_mutex_lock()    
    {   			
    	S32 status_number = 0;
    	/* lock the mutex. */
    	status_number = pthread_mutex_lock(&sharedVariableMutex);
    	if(status_number != e_POE_STATUS_OK)
	    		return e_POE_STATUS_ERR_MUTEX_LOCK_ERROR;                       	            	    	        
    	    	
    	return e_POE_STATUS_OK;   	    	    	    	
    }    
    

/*---------------------------------------------------------------------        												                                                                                                                                           
 *    description:     Unlocking or releasing a mutex 	 				 	 			       
 *		            							     				   
 *    input :   none                                                    	   
 * 
 *    output:   none                   	             
 *                     
 *    return:   e_POE_STATUS_OK                       - operation succeed 
 * 			    e_POE_STATUS_ERR_MUTEX_UNLOCK_ERROR   - operation failed due to mutex unlock operation error                                    
 *---------------------------------------------------------------------*/
    S32 OS_mutex_unlock()    
    {    	 
    	S32 status_number = 0;
    	/* Release the mutex. */
    	status_number = pthread_mutex_unlock(&sharedVariableMutex);
    	if(status_number != e_POE_STATUS_OK)
    		return e_POE_STATUS_ERR_MUTEX_UNLOCK_ERROR;                       	            	    	        
    	
    	return e_POE_STATUS_OK;
    }                 
    
#elif defined(_new_ARC_)	
       
  /*---------------------------------------------------------------------        												                                                                                                                                           
   *    description:     Sleep function 	 				 	 			       
   *		            							     				   
   *    input :   sleepTime_mS  - sleep value in mili Seconds
   * 								minimum required range is: 20 mili seconds to 50 mili seconds  
   * 								with resulution of 10 mili second                                                  	   
   * 
   *    output:   none                   	             
   *                     
   *    return:   e_POE_STATUS_OK                         - operation succeed 
   * 			  e_POE_STATUS_ERR_SLEEP_FUNCTION_ERROR   - operation failed due to usleep function operation error                                    
   *---------------------------------------------------------------------*/
    S32 OS_Sleep_mS(_IN U16 sleepTime_mS)
       {
           /* TODO - implement here the function depending your architecture */
       	   SYSFUN_Sleep(sleepTime_mS/10);
       	   
           return e_POE_STATUS_OK;
       }
      
   /*---------------------------------------------------------------------        												                                                                                                                                           
    *    description:     initialize the mutex 	 				 	 			       
    *		            							     				   
    *    input :   none                                                    	   
    * 
    *    output:   none                   	             
    *                     
    *    return:   e_POE_STATUS_OK                     - operation succeed 
    * 			    e_POE_STATUS_ERR_MUTEX_INIT_ERROR   - operation failed due to mutex initialize operation error 
    *---------------------------------------------------------------------*/  	        
       S32 OS_mutex_init() 
       {   	
    	   /* TODO - implement here the function depending your architecture */
    	   
    	   return e_POE_STATUS_OK;
       }
               
       
   /*---------------------------------------------------------------------        												                                                                                                                                           
    *    description:  locking a mutex 					               	 				 	 			       
    *		            							     				   
    *    input :   none                                                    	   
    * 
    *    output:   none                   	             
    *                     
    *    return:   e_POE_STATUS_OK                       - operation succeed 
    * 			    e_POE_STATUS_ERR_MUTEX_LOCK_ERROR     - operation failed due to mutex lock operation error
    * 				e_POE_STATUS_ERR_SLEEP_FUNCTION_ERROR - operation failed due to usleep function operation error                                     
    *---------------------------------------------------------------------*/
       S32 OS_mutex_lock()    
       {     		       	       
    	   /* TODO - implement here the function depending your architecture */
    	   
    	   return e_POE_STATUS_OK;    		
       }    
       

   /*---------------------------------------------------------------------        												                                                                                                                                           
    *    description:     Unlocking or releasing a mutex 	 				 	 			       
    *		            							     				   
    *    input :   none                                                    	   
    * 
    *    output:   none                   	             
    *                     
    *    return:   e_POE_STATUS_OK                       - operation succeed 
    * 			    e_POE_STATUS_ERR_MUTEX_UNLOCK_ERROR   - operation failed due to mutex unlock operation error                                    
    *---------------------------------------------------------------------*/
       S32 OS_mutex_unlock()    
       {    	
    	   /* TODO - implement here the function depending your architecture */
    	    
    	   return e_POE_STATUS_OK;
       }         

#else
    #error UNSUPPORTED PLATFORM
#endif

/*=========================================================================
/ End of Arcitecture Definition 
/========================================================================*/
