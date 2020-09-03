/* INCLUDE FILE DECLARATIONS
 */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

static ssize_t __recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen);

static ssize_t __sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen);

#define recvfrom __recvfrom
#define sendto __sendto
/* test target source code */
#include "tftp.c"


/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define ASSERT assert

#define TEST_BLOCK_NBR      70000   /*It must be big enough for blkid overflow test*/

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    unsigned short packet_type;
    unsigned short block_id;
} Test_Node_T;

typedef struct
{
    UI32_T       last_test_case_id;
    UI32_T       last_block_size;
	Test_Node_T  node[TEST_BLOCK_NBR];
} Test_Info_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void TFTP_UT_INIT(void);
static void TFTP_get_UT(UI32_T last_block_size);
static void TFTP_put_UT(UI32_T last_block_size);
static void TFTP_put_no_option_UT(UI32_T last_block_size);

/* STATIC VARIABLE DECLARATIONS
 */
static I32_T test_case_id=-1;

static Test_Info_T test_case;  
   

/* LOCAL SUBPROGRAM BODIES
 */
static ssize_t __recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen)
{
    register struct tftphdr *ap;
    size_t size;

    ap = (struct tftphdr *)buf;

    if (test_case_id < 0)
    {
        printf("It should sendto first!!\r\n");
        ASSERT(0);
    }

    if (test_case_id >= TEST_BLOCK_NBR || test_case_id > test_case.last_test_case_id)
    {
        printf("Internal error\r\n");
        ASSERT(0);
    }

    switch (test_case.node[test_case_id].packet_type)
    {
        case OACK:
        {
            register char *cp;
            UI8_T blksize[TFTP_OP_BLKSIZE_VALUE_LEN];

            ap->th_opcode = htons((unsigned short)OACK);

            cp = ap->th_stuff;
            strcpy(cp, TFTP_OP_BLKSIZE_STR);
            cp += strlen(TFTP_OP_BLKSIZE_STR);
            *cp++ = '\0';
            sprintf((char *)blksize, "%d", SYS_DFLT_TFTP_OP_BLKSIZE);
            strcpy(cp, (char *)blksize);
            cp += strlen((char *)blksize);
            *cp++ = '\0';

            size = cp - (char *)ap;
        }
            break;

        case ACK:
            ap->th_opcode = htons((unsigned short)ACK);
            ap->th_block = htons(test_case.node[test_case_id].block_id);
            size = 4;
            break;

        case DATA:
            if (test_case_id == test_case.last_test_case_id)
            {
                memset(ap->th_data, 0x33, test_case.last_block_size);
                size = test_case.last_block_size + 4;
            }
            else
            {
                memset(ap->th_data, 0x33, SYS_DFLT_TFTP_OP_BLKSIZE);
                size = SYS_DFLT_TFTP_OP_BLKSIZE + 4;
            }

            ap->th_opcode = htons((unsigned short)DATA);
            ap->th_block = htons(test_case.node[test_case_id].block_id);
            break;

        default:
            printf("Please add new packet type : %d\r\n", test_case.node[test_case_id].packet_type);
            ASSERT(0);
            break;
    }

    return size;
}

static ssize_t __sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen)
{
    test_case_id ++;

    return len;
}

static void TFTP_UT_INIT()
{
    test_case_id=-1;
    memset(&test_case, 0, sizeof(Test_Info_T));
}

static void TFTP_put_UT(UI32_T last_block_size)
{
    int i;
    I32_T   rc;
    L_INET_AddrIp_T inaddr;
    char *buf;
	UI32_T total_image_size;

    /*init test case*/
    TFTP_UT_INIT();
	test_case.last_test_case_id = TEST_BLOCK_NBR - 1;
	test_case.last_block_size = last_block_size;
	
    test_case.node[0].packet_type = OACK;
    test_case.node[0].block_id = 0;
    for(i=1; i<TEST_BLOCK_NBR; i++)
    {
        test_case.node[i].packet_type = ACK;
        test_case.node[i].block_id = i;
    }

    total_image_size = SYS_DFLT_TFTP_OP_BLKSIZE * (test_case.last_test_case_id - 1) + test_case.last_block_size;
    buf = malloc(total_image_size);

    for(i=0; i<total_image_size-1; i++)
    {
        buf[i] = 0x33;
    }
    buf[i] = '\0';

    rc = TFTP_put(&inaddr, "test", "octet", buf, total_image_size, 3, 3);
    if (rc <= 0)
    {
        printf("TFTP_put_UT test error: %d\r\n", rc);
        free(buf);
        ASSERT(0);
    }

    if (test_case_id != test_case.last_test_case_id)
    {
        printf("TFTP_get_UT test error! last_test_case_id: %d\r\n", test_case_id);
        free(buf);
        ASSERT(0);
    }

    if (rc != total_image_size)
    {
	    printf("TFTP_put_UT test error!! rc:%d\r\n", rc);
        free(buf);
        ASSERT(0);
    }
    else
    {
        printf("TFTP_put_UT test success!!\r\n");
    }

    free(buf);
    return;
}

static void TFTP_put_no_option_UT(UI32_T last_block_size)
{
    int i;
    I32_T   rc;
    L_INET_AddrIp_T inaddr;
    char *buf;
	UI32_T total_image_size;

    /*init test case*/
    TFTP_UT_INIT();
    test_case.last_test_case_id = TEST_BLOCK_NBR - 1;
    test_case.last_block_size = last_block_size;

    test_case.node[0].packet_type = ACK;
    test_case.node[0].block_id = 0;
    for(i=1; i<TEST_BLOCK_NBR; i++)
    {
        test_case.node[i].packet_type = ACK;
        test_case.node[i].block_id = i;
    }

    /*The blksize must be SYS_DFLT_TFTP_BLKSIZE (512)*/
    total_image_size = SYS_DFLT_TFTP_BLKSIZE * (test_case.last_test_case_id - 1) + test_case.last_block_size;
    buf = malloc(total_image_size);

    for(i=0; i<total_image_size-1; i++)
    {
        buf[i] = 0x33;
    }
    buf[i] = '\0';

    rc = TFTP_put(&inaddr, "test", "octet", buf, total_image_size, 3, 3);
    if (rc <= 0)
    {
        printf("%s test error: %d\r\n", __FUNCTION__, rc);
        free(buf);
        ASSERT(0);
    }

    if (test_case_id != test_case.last_test_case_id)
    {
        printf("%s test error! last_test_case_id: %d\r\n", __FUNCTION__, test_case_id);
        free(buf);
        ASSERT(0);
    }

    if (rc != total_image_size)
    {
        printf("%s test error!! rc:%d\r\n", __FUNCTION__, rc);
        free(buf);
        ASSERT(0);
    }
    else
    {
        printf("%s test success!!\r\n", __FUNCTION__);
    }

    free(buf);
    return;
}

static void TFTP_get_UT(UI32_T last_block_size)
{
    int i;
    I32_T   rc;
    L_INET_AddrIp_T inaddr;
    char *buf;
    UI32_T total_image_size;

    /*init test case*/
    TFTP_UT_INIT();
    test_case.last_test_case_id = TEST_BLOCK_NBR - 1;
    test_case.last_block_size = last_block_size;
	
    test_case.node[0].packet_type = OACK;
    test_case.node[0].block_id = 0;

    for(i=1; i<TEST_BLOCK_NBR; i++)
    {
        test_case.node[i].packet_type = DATA;
        test_case.node[i].block_id = i;
    }
	
    total_image_size = SYS_DFLT_TFTP_OP_BLKSIZE * (test_case.last_test_case_id - 1) + test_case.last_block_size;
    buf = malloc(SYS_DFLT_TFTP_OP_BLKSIZE * TEST_BLOCK_NBR);

    rc = TFTP_get(&inaddr,"test","octet", FALSE, buf, SYS_DFLT_TFTP_OP_BLKSIZE*TEST_BLOCK_NBR, 3, 3);
    if (rc <= 0)
    {
        printf("TFTP_get_UT test error: %d\r\n", rc);
        free(buf);
        ASSERT(0);
    }

    if (test_case_id != (test_case.last_test_case_id+1))
    {
        printf("TFTP_get_UT test error! last_test_case_id: %d\r\n", test_case_id);
        free(buf);
        ASSERT(0);
    }

    if (rc != total_image_size)
    {
        printf("TFTP_get_UT test error!! rc:%d\r\n", rc);
        free(buf);
        ASSERT(0);
    }
    else
    {
        printf("TFTP_get_UT test success!!\r\n");
    }

    free(buf);
    return;
}

int main(int argc, char* argv[])
{
    TFTP_get_UT(1033);
    TFTP_get_UT(0);
    TFTP_put_UT(409);
    TFTP_put_UT(0);
    TFTP_put_no_option_UT(511);
    TFTP_put_no_option_UT(0);
    return 0;
}

