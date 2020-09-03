/************************************************************
 *
 *        Copyright 2015 Accton Corporation, 2015
 *
 * Licensed under the Eclipse Public License, Version 1.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *        http://www.eclipse.org/legal/epl-v10.html
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 *
 ***********************************************************/
/* MODULE NAME:  onlp_sfputil.c
 * PURPOSE:
 *   This module implements a simple onlp utility program to
 *   get the SFP related information or set the SFP related functions
 *   through the ONLP library.
 *
 * NOTES:
 *
 * HISTORY
 *    10/28/2015 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation , 2015
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "onlp/onlp_config.h"
#include "onlp/sfp.h"
#include "AIM/aim_memory.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define DEBUG 1

/* The comment of the function onlp_sfp_eeprom_read says that it returns the
 * size of eeprom data if the operation is successful. But it actually returns
 * 0 when the operation is successful. The output eeprom data size is always
 * 256.
 */
#define ONLP_SFP_EEPROM_READ_SIZE 256

/* MACRO FUNCTION DECLARATIONS
 */

#ifdef DEBUG
#define DEBUG_MSG(fmtstr, ...) do { \
    if(debug_flag == 1) \
        {printf("%s(%d)"fmtstr, __FUNCTION__, __LINE__, ##__VA_ARGS__);} \
} while (0)

#define DEBUG_OPT_CHAR "d"
#else
#define DEBUG_MSG(...)
#define DEBUG_OPT_CHAR
#endif

extern void onlp_api_lock_init(void);

/* DATA TYPE DECLARATIONS
 */
typedef enum {
    OUTPUT_FORMAT_BINARY,
    OUTPUT_FORMAT_HEXDECIMAL,        
} OutputFormat_T;

typedef enum{
    EEPROM_AREA_INVALID=0,
    EEPROM_AREA_BASIC  =1,
    EEPROM_AREA_DDM    =2,
} EepromAreaBmp_T;

typedef enum{
    OPERATION_TYPE_EEPROM_READ    = 1,
    OPERATION_TYPE_PRESENT_STATUS = 2,
    OPERATION_TYPE_TXDISABLE_SET  = 4,
    OPERATION_TYPE_EEPROM_WRITE   = 8,
} OperationTypeBmp_T;

typedef struct BinaryOutputHeader_S
{
    int rc;
    int output_data_sz;
} BinaryOutputHeader_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void onlplib_sfp_init(void);
static void output_data(int data_offset, const uint8_t *data_p, int data_size);
static int handle_eeprom_read_op(void);
static int handle_eeprom_write_op(void);
static int handle_present_status_op(void);
static int handle_tx_disable_op(void);

/* EXPORTED VARIABLE DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static int debug_flag=0;
static OutputFormat_T output_format=OUTPUT_FORMAT_BINARY;
static EepromAreaBmp_T eeprom_area=EEPROM_AREA_INVALID;
static OperationTypeBmp_T op_type=0;
static int onlp_port = -1;
static int tx_disable=-1;
static int offset=-1;
static int value=-1;

/* EXPORTED SUBPROGRAM BODIES
 */
int main(int argc, char* argv[])
{
    int opt_h = 0;
    int opt, opt_num=0;
    int retval=0, rc;
    const char* optstr="p:emwo:v:f:st:h"DEBUG_OPT_CHAR;

    while( (opt = getopt(argc, argv, optstr)) != -1)
    {
        opt_num++;
        switch(opt)
        {
            case 'h': opt_h=1; break;
            case 'p':
            {
                onlp_port=atoi(optarg);
                break;
            }
            case 'e':
            {
                eeprom_area|=EEPROM_AREA_BASIC;
                op_type|=OPERATION_TYPE_EEPROM_READ;
                break;
            }
            case 'm':
            {
                eeprom_area|=EEPROM_AREA_DDM;
                op_type|=OPERATION_TYPE_EEPROM_READ;
                break;
            }
            case 'w':
            {
                op_type|=OPERATION_TYPE_EEPROM_WRITE;
                break;
            }
            case 'o':
            {
                offset = atoi(optarg);
                break;
            }
            case 'v':
            {
                value = atoi(optarg);
                break;
            }
            case 'f':
            {
                if (*optarg=='a')
                {
                    output_format=OUTPUT_FORMAT_HEXDECIMAL;
                }
                break;
            }
            case 's':op_type|=OPERATION_TYPE_PRESENT_STATUS; break;
            case 't':
            {
                op_type|=OPERATION_TYPE_TXDISABLE_SET;

                tx_disable = strtoul(optarg, NULL, 0);
                break;
            }
#ifdef DEBUG
            case 'd': debug_flag=1; break;
#endif
            default: opt_h=1; retval = 1; break;
        }
    }

    if((opt_num==0) || (opt_h))
    {
        printf("Usage: %s [OPTIONS]\n", argv[0]);
        printf(" Required options:\n");
        printf("   -h Show help message\n");
        printf("   -p onlp port id\n");
        printf("   SFP EEPROM read operation options:\n");
        printf("   -e Dump SFP transceiver EEPROM data\n");
        printf("   -m Dump SFP transceiver DDM EEPROM data\n");
        printf("   SFP EEPROM write operation options:\n");
        printf("   -w Write value to SFP transceiver addr 0x50\n");
        printf("      -o register offset\n");
        printf("      -v value to write to the specified register offset\n");
        printf("   SFP present status option:\n");
        printf("   -s Show present status of the specified port id\n");
        printf("      Output 1 for present and 0 for not-present.\n");
        printf("      Output negative number when error.\n");
        printf("   SFP TX Disable control option:\n");
        printf("   -t Set the tx disable control of the specified port id\n");
        printf("      The argument represents the bitmap to set on each tx\n");
        printf("      channel. The format of the argument can be decimal or\n");
        printf("      hex which starts with 0x. Bit 0 represents the first tx\n");
        printf("      channel, bit 1 represents the second tx channel, and so on.\n");
        printf("      bit value '0':Turn off TX Disable\n");
        printf("      bit value '1':Turn on TX Disable\n");
        printf("\n");
        printf(" Optional options:\n");
#ifdef DEBUG
        printf("   -d Show debug message\n");
#endif
        printf("\n");
        printf("   -f Specify the output format\n");
        printf("      sub option 'b': Dump data in binary format(Default)\n");
        printf("      sub option 'a': Dump data in hexdecimal format\n");
        printf("The layout of the binary format:\n");
        printf("--------------------------------------\n");
        printf("Byte | 0-3 | 4-7 |  8  |  9  |  ...  |\n");
        printf("-----+-----+-----+-----+-----+-------+\n");
        printf("     |  RC |  SZ |  EEPROM data      |\n");
        printf("--------------------------------------\n");
        printf("RC: The value is 0 when the SFP access operation is successful\n");
        printf("SZ: The length of EEPROM data to be output\n");
        return retval;
    }

    /* validate the required options for SFP eeprom read operation
     */
    if (op_type & OPERATION_TYPE_EEPROM_READ)
    {
        if (onlp_port<0)
        {
            printf("No port is specified(Use -p option)\n");
            return -1;
        }

        if (eeprom_area==EEPROM_AREA_INVALID)
        {
            printf("No eeprom type is specified(Use -e or -m option)\n");
            return -1;
        }
    }

    if (op_type & OPERATION_TYPE_EEPROM_WRITE)
    {
        if (onlp_port<0)
        {
            printf("No port is specified(Use -p option)\n");
            return -1;
        }
        if (offset<0)
        {
            printf("No offset is specified(Use -o option)\n");
            return -1;
        }
        if (value<0)
        {
            printf("No value is specified(Use -v option)\n");
            return -1;
        }
    }

    if (op_type & OPERATION_TYPE_PRESENT_STATUS)
    {
        if (onlp_port<0)
        {
            printf("No port is specified(Use -p option)\n");
            return -1;
        }
    }

    if (op_type & OPERATION_TYPE_TXDISABLE_SET)
    {
        if (onlp_port<0)
        {
            printf("No port is specified(Use -p option)\n");
            return -1;
        }
    }

    onlplib_sfp_init();

    if (op_type & OPERATION_TYPE_EEPROM_READ)
    {
        if (handle_eeprom_read_op() != 0)
        {
            retval=1;
        }
    }

    if (op_type & OPERATION_TYPE_EEPROM_WRITE)
    {
        if (handle_eeprom_write_op() != 0)
        {
            retval=1;
        }
    }

    if (op_type & OPERATION_TYPE_PRESENT_STATUS)
    {
        if (handle_present_status_op() != 0)
        {
            retval=1;
        }
    }

    if (op_type & OPERATION_TYPE_TXDISABLE_SET)
    {
        if (handle_tx_disable_op() != 0)
        {
            retval=1;
        }
    }

    return retval;
}

/* LOCAL SUBPROGRAM BODIES
 */
static void onlplib_sfp_init(void)
{
#if ONLP_CONFIG_INCLUDE_API_LOCK == 1
    onlp_api_lock_init();
#endif
    onlp_sfp_init();
}

static void output_data(int data_offset, const uint8_t *data_p, int data_size)
{
    int offset=data_offset;
    int end_offset=offset+data_size;

    if(output_format==OUTPUT_FORMAT_HEXDECIMAL)
    {
        offset=0;
        while (offset<end_offset)
        {
            if ((end_offset-offset) >= 16)
            {
                printf("[%02X] %02hX %02hX %02hX %02hX %02hX %02hX %02hX %02hX %02hX %02hX %02hX %02hX %02hX %02hX %02hX %02hX\n",
                    offset, data_p[offset+0], data_p[offset+1], data_p[offset+2], data_p[offset+3],
                    data_p[offset+4], data_p[offset+5], data_p[offset+6], data_p[offset+7],
                    data_p[offset+8], data_p[offset+9], data_p[offset+10], data_p[offset+11],
                    data_p[offset+12], data_p[offset+13], data_p[offset+14], data_p[offset+15]);
                    offset+=16;
            }
            else
            {
                printf("[%02X]", offset);
                for(; offset<end_offset; offset++)
                {
                    printf(" %02hX", data_p[offset]);
                }
                printf("\n");
            }
        }
    }
    else
    {
        fwrite(data_p, data_size, 1, stdout);
    }
}

/* returns 0 on success, non-zero on failed
 */
static int handle_eeprom_read_op(void)
{
    int rc=-1, retval=0;
    uint8_t *eeprom_data_p=NULL;
    BinaryOutputHeader_T output_header;

    DEBUG_MSG("EEPROM read op for ONLP port id:%d\n", onlp_port);

    if (debug_flag)
    {
        switch (output_format)
        {
            case OUTPUT_FORMAT_BINARY:
                DEBUG_MSG("Dump data in binary.\n");
                break;
            case OUTPUT_FORMAT_HEXDECIMAL:
                DEBUG_MSG("Dump data in hexdecimal.\n");
                break;
            default:
                DEBUG_MSG("Error. Unknow output format(%d)\n", (int) output_format);
                break;
        }
    }

    if (eeprom_area&EEPROM_AREA_BASIC)
    {
        rc = onlp_sfp_eeprom_read(onlp_port, &eeprom_data_p);
        DEBUG_MSG("SFP EEPROM Area = Basic\n");
        DEBUG_MSG("ONLP SFP eeprom read operation returns %d\n", rc);
    }

    if (eeprom_area&EEPROM_AREA_DDM)
    {
        rc = onlp_sfp_dom_read(onlp_port, &eeprom_data_p);
        DEBUG_MSG("SFP EEPROM Area = DDM\n");
        DEBUG_MSG("ONLP SFP eeprom read operation returns %d\n", rc);
    }

    output_header.rc=rc;
    output_header.output_data_sz=ONLP_SFP_EEPROM_READ_SIZE;
    if (rc>=0)
    {
        if (output_format==OUTPUT_FORMAT_BINARY)
        {
            output_data(0, (const uint8_t *)&output_header, (int)sizeof(output_header));
            output_data(sizeof(output_header), eeprom_data_p, ONLP_SFP_EEPROM_READ_SIZE);
        }
        else
        {
            output_data(0, eeprom_data_p, ONLP_SFP_EEPROM_READ_SIZE);
        }
    }
    else
    {
        if (output_format==OUTPUT_FORMAT_HEXDECIMAL)
        {
            printf("Read error.(rc=%d)\n", rc);
        }
        else
        {
            output_header.rc=rc;
            output_header.output_data_sz=0;
            output_data(0, (const uint8_t *)&output_header, sizeof(output_header));
        }
        retval=-1;
    }

    if (eeprom_data_p!=NULL)
    {
        aim_free(eeprom_data_p);
    }

    return retval;
}

/* returns 0 on success, non-zero on failed
 */
static int handle_eeprom_write_op(void)
{
    int rc, retval=0;

    DEBUG_MSG("EEPROM write op for ONLP port id:%d\n", onlp_port);

    rc=onlp_sfp_ioctl(onlp_port, ONLP_SFP_IOCTL_CMD_WRITE, offset, value);
	if (rc < 0)
       DEBUG_MSG("error: %d\n", rc);

    if(rc<0)
        retval=-1;

    return retval;
    
}

/* returns 0 on success, non-zero on failed
 */
static int handle_present_status_op(void)
{
    int rc, retval=0;

    DEBUG_MSG("Present status detect for ONLP port id:%d\n", onlp_port);

    rc=onlp_sfp_is_present(onlp_port);
    printf("%d\n", rc);

    if(rc<0)
        retval=-1;

    return retval;
    
}

/* returns 0 on success, non-zero on failed
 */
static int handle_tx_disable_op(void)
{
    int rc, retval=0;

    DEBUG_MSG("Set TX_DISABLE of ONLP port id %d as %#x\n", onlp_port, tx_disable);
    rc=onlp_sfp_control_set(onlp_port, ONLP_SFP_CONTROL_TX_DISABLE, tx_disable);
    /* do not show the error message if rc is ONLP_STATUS_E_UNSUPPORTED
     */
    if (rc<0 && rc!=ONLP_STATUS_E_UNSUPPORTED)
    {
        printf("Set SFP TX_DISABLE on ONLP port %d failed.(rc=%d)\n", onlp_port, rc);
        retval=-1;
    }
    return retval;
}

