/* MODULE NAME:  l_pbmp.h
 * PURPOSE:
 *  This header file provides utility macros to manipulate port bitmap.
 *
 * NOTES:
 *  A port bitmap is consisted of an array which is declared with a unsigned
 *  primitive data type (e.g. unsigned char).
 *
 *  The convention of arrangement of port to the corresponding bit in port bitmap
 *  is described below:
 *
 *  All macro functions in this header file assumes that port number is started
 *  from 1.
 *
 *  Within an element of a port bitmap array , the smallest port number will be
 *  mapped to the most significant bit within that single unsigned primitive
 *  type variable. 
 *
 *  For example, the code snippet shown below will turn on port 1 bit in the given port
 *  bitmap.
 *  {
 *      unsigned char pbmp[1]; // pbmp variable
 *      pbmp[0] |= 0x80;       // set port 1 bit on pbmp
 *  }
 *
 *  Each element in a port bitmap array will keep the bits corresponding to a set of ports.
 *  Smaller port numbers are kept in the port bitmap array with smaller index.
 *  For example, in "unsigned char pbmp[2]", pbmp[0] will keep the bits for port 1 to port 8,
 *  and pbmp[1] will keep the bits for port 9 to port 16.
 *
 *  
 *
 * HISTORY
 *    12/15/2011 - Charlie Chen, Created
 *
 * Copyright(C)      EdgeCore Networks, 2011
 */
#ifndef L_PBMP_H
#define L_PBMP_H

#include "sys_type.h"

/*------------------------------------------------------------------------------
 * MACRO NAME - L_PBMP_PORT_TO_INDEX_OF_PBMP_ARRAY
 *------------------------------------------------------------------------------
 * PURPOSE:  For converting port number to the corresponding pbmp array index.
 * INPUT:    port     -  port number
 *           elm_type -  Primitive data type of the pbmp array
 * OUTPUT:   None
 * RETURN:   pbmp array index for the given port number.
 * NOTE:     1. elm_type must be C primitive unsigned data type (such as unsigned int)
 *------------------------------------------------------------------------------
 */
#define L_PBMP_PORT_TO_INDEX_OF_PBMP_ARRAY(port, elm_type) \
    (((port)-1)/(sizeof(elm_type)*8))

/*------------------------------------------------------------------------------
 * MACRO NAME - L_PBMP_PORT_TO_BIT_WITHIN_ELM
 *------------------------------------------------------------------------------
 * PURPOSE:  For converting port number to the corresponding bit within an
 *           element of a port bitmap array.
 * INPUT:    port     -  port number
 *           elm_type -  Primitive data type of the pbmp array
 * OUTPUT:   None
 * RETURN:   The corresponding bit within an element of a port bitmap array of the
 *           given port number.
 * NOTE:     1. elm_type must be C primitive unsigned data type (such as unsigned int)
 *------------------------------------------------------------------------------
 */
#define L_PBMP_PORT_TO_BIT_WITHIN_ELM(port, elm_type) \
    ((elm_type)1<<(((sizeof(elm_type)*8)-1)-((port-1)%(sizeof(elm_type)*8))))

/*------------------------------------------------------------------------------
 * MACRO NAME - L_PBMP_FOR_EACH_PORT_IN_PBMP_ARRAY
 *------------------------------------------------------------------------------
 * PURPOSE:  For traversing each port that the corresponding bit in the given
 *           pbmp is set.
 * INPUT:    pbmp     -  port bit map array
 *           max_port -  The maximum port number in the given pbmp
 * OUTPUT:   port     -  the port number that the corresponding bit in the given
 *                       pbmp is set.
 * RETURN:   None
 * NOTE:     1. pbmp must be a C primitive unsigned data type array (such as unsigned int[ ])
 *------------------------------------------------------------------------------
 */
 #define L_PBMP_FOR_EACH_PORT_IN_PBMP_ARRAY(pbmp, max_port, port) \
    for ((port) = 1; (port) <= (max_port); (port)++) \
        if ((pbmp)[L_PBMP_PORT_TO_INDEX_OF_PBMP_ARRAY((port), typeof((pbmp)[0]))] & L_PBMP_PORT_TO_BIT_WITHIN_ELM((port), typeof((pbmp)[0])))

/*------------------------------------------------------------------------------
 * MACRO NAME - L_PBMP_SET_PORT_IN_PBMP_ARRAY
 *------------------------------------------------------------------------------
 * PURPOSE:  For setting the bit that corresponds to the given port number within
 *           the given pbmp.
 * INPUT:    pbmp     -  port bit map array
 *           port     -  The port number that corresponds to the bit to be set
 *                       within the given pbmp
 * OUTPUT:   None
 * RETURN:   None
 * NOTE:     1. pbmp must be a C primitive unsigned data type array (such as unsigned int[ ])
 *------------------------------------------------------------------------------
 */
#define L_PBMP_SET_PORT_IN_PBMP_ARRAY(pbmp, port) \
    (pbmp)[L_PBMP_PORT_TO_INDEX_OF_PBMP_ARRAY((port),typeof((pbmp)[0]))] |= L_PBMP_PORT_TO_BIT_WITHIN_ELM((port),typeof((pbmp)[0]))

/*------------------------------------------------------------------------------
 * MACRO NAME - L_PBMP_UNSET_PORT_IN_PBMP_ARRAY
 *------------------------------------------------------------------------------
 * PURPOSE:  For clearing the bit that corresponds to the given port number
 *           within the given pbmp.
 * INPUT:    pbmp     -  port bit map array
 *           port     -  The port number that corresponds to the bit to be cleared
 *                       within the given pbmp
 * OUTPUT:   None
 * RETURN:   None
 * NOTE:     1. pbmp must be a C primitive unsigned data type array (such as unsigned int[ ])
 *------------------------------------------------------------------------------
 */
#define L_PBMP_UNSET_PORT_IN_PBMP_ARRAY(pbmp, port) \
    (pbmp)[L_PBMP_PORT_TO_INDEX_OF_PBMP_ARRAY((port),typeof((pbmp)[0]))] &= ~(L_PBMP_PORT_TO_BIT_WITHIN_ELM((port),typeof((pbmp[0]))))

/*------------------------------------------------------------------------------
 * MACRO NAME - L_PBMP_GET_PORT_IN_PBMP_ARRAY
 *------------------------------------------------------------------------------
 * PURPOSE:  Get the bit that corresponds to the given port number within
 *           the given pbmp.
 * INPUT:    pbmp     -  port bit map array
 *           port     -  The port number that corresponds to the bit to be read
 *                       within the given pbmp
 * OUTPUT:   None
 * RETURN:   Non zero - if the bit that corresponds to the given port within the
 *                      given pbmp is set.
 *           Zero     - if the bit that corresponds to the given port within the
 *                      given pbmp is cleared.
 * NOTE:     1. pbmp must be a C primitive unsigned data type array (such as unsigned int[ ])
 *------------------------------------------------------------------------------
 */
#define L_PBMP_GET_PORT_IN_PBMP_ARRAY(pbmp, port) \
    (((pbmp)[L_PBMP_PORT_TO_INDEX_OF_PBMP_ARRAY((port),typeof((pbmp)[0]))]) & (L_PBMP_PORT_TO_BIT_WITHIN_ELM((port),typeof((pbmp)[0]))))

/*------------------------------------------------------------------------------
 * MACRO NAME - L_PBMP_IS_ALL_ZEROS_PBMP_ARRAY
 *------------------------------------------------------------------------------
 * PURPOSE:  Whether or not the given port bitmap array contains all zeros.
 * INPUT:    pbmp     -  port bitmap array
 *           max_port -  The maximum port number in the given pbmp
 * OUTPUT:   None
 * RETURN:   Return TRUE if the given port bitmap array contains all zeros,
 *           return FALSE otherwised.
 * NOTE:     1. pbmp must be a C primitive unsigned data type array (such as unsigned int[ ])
 *------------------------------------------------------------------------------
 */
#define L_PBMP_IS_ALL_ZEROS_PBMP_ARRAY(pbmp, max_port) \
({ \
    UI32_T idx; \
    BOOL_T ret=TRUE; \
    for (idx = 0; idx < ((max_port)+(sizeof((pbmp)[0])*8-1))/(sizeof((pbmp)[0])*8); idx++) \
        if ((pbmp)[idx]) \
        { \
            ret=FALSE; \
            break; \
        } \
    ret; \
})

/*------------------------------------------------------------------------------
 * MACRO NAME - L_PBMP_GET_NEXT_PORT_FROM_GIVEN_PORT_IN_PBMP_ARRAY
 *------------------------------------------------------------------------------
 * PURPOSE:  Get next port which has corresponding bit set in the given port
 *           bitmap array from the given port number.
 * INPUT:    pbmp       -  port bitmap array
 *           max_port   -  The maximum port number in the given pbmp
 *           start_port -  port number to start searching for next port that has
 *                         corresponding bit set in the given port bitmap array.
 * OUTPUT:   
 * RETURN:   Return 0 if there is no port can be found.
 *           Return the first port number that has corresponding bit set in the
 *           given port bitmap array from the given port number.
 * NOTE:     1. pbmp must be a C primitive unsigned data type array (such as unsigned int[ ])
 *------------------------------------------------------------------------------
 */
#define L_PBMP_GET_NEXT_PORT_FROM_GIVEN_PORT_IN_PBMP_ARRAY(pbmp, max_port, start_port) \
({ \
    UI32_T pbmp_idx,elm_idx; \
    pbmp_idx = L_PBMP_PORT_TO_INDEX_OF_PBMP_ARRAY((start_port),typeof((pbmp)[0])); \
    typeof((pbmp)[0]) mask; \
    if( pbmp[pbmp_idx] \
      & ((L_PBMP_PORT_TO_BIT_WITHIN_ELM(start_port, typeof((pbmp)[0])) - 1) | \
         (L_PBMP_PORT_TO_BIT_WITHIN_ELM(start_port, typeof((pbmp)[0])))) \
      ) \
    { \
        mask = L_PBMP_PORT_TO_BIT_WITHIN_ELM(start_port, typeof((pbmp)[0])); \
        elm_idx = (start_port-1)%(sizeof((pbmp)[0])*8); \
    } \
    else \
    { \
        for (pbmp_idx += 1; \
             pbmp_idx <= L_PBMP_PORT_TO_INDEX_OF_PBMP_ARRAY((max_port),typeof((pbmp)[0])); pbmp_idx++) \
            if ((pbmp)[pbmp_idx]) \
            { \
                mask = (typeof((pbmp)[0]))1 << (sizeof((pbmp)[0])*8 - 1); \
                elm_idx=0; \
                break; \
            } \
        if(pbmp_idx>L_PBMP_PORT_TO_INDEX_OF_PBMP_ARRAY((max_port),typeof((pbmp)[0]))) \
            0; \
    } \
    \
    while(mask) \
    { \
        if((pbmp)[pbmp_idx] & mask) \
            break; \
        mask >>= 1; \
        elm_idx++; \
    } \
    sizeof((pbmp)[0])*8*pbmp_idx + elm_idx + 1; \
})

#endif /* #ifndef L_PBMP_H */


