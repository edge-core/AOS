


/***************************************************************************
 *     Copyright Motorola, Inc. 1989-2001 ALL RIGHTS RESERVED
 *
 *  $ID:$
 *
 * You are hereby granted a copyright license to use, modify, and
 * distribute the SOFTWARE, also know as DINK32 (Dynamic Interactive Nano 
 * Kernel for 32-bit processors) solely in conjunction with the development 
 * and marketing of your products which use and incorporate microprocessors 
 * which implement the PowerPC(TM) architecture manufactured by 
 * Motorola and provided you comply with all of the following restrictions 
 * i) this entire notice is retained without alteration in any
 * modified and/or redistributed versions, and 
 * ii) that such modified versions are clearly identified as such. 
 * No licenses are granted by implication, estoppel or
 * otherwise under any patents or trademarks of Motorola, Inc.
 * 
 * The SOFTWARE is provided on an "AS IS" basis and without warranty. To
 * the maximum extent permitted by applicable law, MOTOROLA DISCLAIMS ALL
 * WARRANTIES WHETHER EXPRESS OR IMPLIED, INCLUDING IMPLIED WARRANTIES OF
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE AND ANY WARRANTY 
 * AGAINST INFRINGEMENT WITH REGARD TO THE SOFTWARE 
 * (INCLUDING ANY MODIFIED VERSIONS THEREOF) AND ANY ACCOMPANYING 
 * WRITTEN MATERIALS.
 * 
 * To the maximum extent permitted by applicable law, IN NO EVENT SHALL
 * MOTOROLA BE LIABLE FOR ANY DAMAGES WHATSOEVER (INCLUDING WITHOUT 
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS 
 * INTERRUPTION, LOSS OF BUSINESS INFORMATION,
 * OR OTHER PECUNIARY LOSS) ARISING OF THE USE OR INABILITY TO USE THE
 * SOFTWARE.
 * Motorola assumes no responsibility for the maintenance and support of
 * the SOFTWARE.
 ************************************************************************/
/* These are the defined return values for the I2C_do_transaction function.
 * Any non-zero value indicates failure.  Failure modes can be added for 
 * more detailed error reporting.
 */

 #include "sys_type.h"

#ifndef I2C_EXPORT_H
#define I2C_EXPORT_H

typedef enum _i2c_status
{
    I2C_SUCCESS = 0,
    I2C_ERROR,
} I2C_Status;

/* These are the defined tasks for I2C_do_transaction.
 * Modes for SLAVE_RCV and SLAVE_XMIT will be added.
 */
typedef enum _i2c_transaction_mode
{
	I2C_MASTER_RCV =  0,
	I2C_MASTER_XMIT = 1,
} I2C_TRANSACTION_MODE;

typedef enum _i2c_interrupt_mode
{
	I2C_INT_DISABLE =  0,
	I2C_INT_ENABLE = 1,
} I2C_INTERRUPT_MODE;

typedef enum _i2c_stop
{
	I2C_NO_STOP =  0,
	I2C_STOP = 1,
} I2C_STOP_MODE;

typedef enum _i2c_restart
{
	I2C_NO_RESTART =  0,
	I2C_RESTART = 1,
} I2C_RESTART_MODE;

typedef enum _i2c_bus_status
{
	I2C_IDLE =  0,
	I2C_BUSY = 1,
	I2C_LOST = 2,
	I2C_DISABLE = 3,
} I2C_BUS_STATUS;

typedef enum _i2c_channel_
{
 M41T11     = 0,
 W83782D,
 Module10G_1,
 Module10G_2,
 SFP_1,
 SFP_2,
 SFP_3,
 SFP_4,
 I2C_CHANNEL_TOTAL
 } I2C_CHANNEL;

/* I2C 7bit/10bit access mode
 */
enum
{
    I2C_7BIT_ACCESS_MODE =0,
    I2C_10BIT_ACCESS_MODE
};

#endif
