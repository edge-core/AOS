/*-----------------------------------------------------------------------------
 * Module Name: nand_ecc_tools.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Check nand ecc error for Master file.
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    12/17/2013 - Vic Chang  , Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2013
 *-----------------------------------------------------------------------------
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>

typedef unsigned char          uchar;


#define BLOCK_SIZE 256

/*
 * Pre-calculated 256-way 1 byte column parity
 */
static const u_char nand_ecc_precalc_table[] = {
    0x00, 0x55, 0x56, 0x03, 0x59, 0x0c, 0x0f, 0x5a, 0x5a, 0x0f, 0x0c, 0x59, 0x03, 0x56, 0x55, 0x00,
    0x65, 0x30, 0x33, 0x66, 0x3c, 0x69, 0x6a, 0x3f, 0x3f, 0x6a, 0x69, 0x3c, 0x66, 0x33, 0x30, 0x65,
    0x66, 0x33, 0x30, 0x65, 0x3f, 0x6a, 0x69, 0x3c, 0x3c, 0x69, 0x6a, 0x3f, 0x65, 0x30, 0x33, 0x66,
    0x03, 0x56, 0x55, 0x00, 0x5a, 0x0f, 0x0c, 0x59, 0x59, 0x0c, 0x0f, 0x5a, 0x00, 0x55, 0x56, 0x03,
    0x69, 0x3c, 0x3f, 0x6a, 0x30, 0x65, 0x66, 0x33, 0x33, 0x66, 0x65, 0x30, 0x6a, 0x3f, 0x3c, 0x69,
    0x0c, 0x59, 0x5a, 0x0f, 0x55, 0x00, 0x03, 0x56, 0x56, 0x03, 0x00, 0x55, 0x0f, 0x5a, 0x59, 0x0c,
    0x0f, 0x5a, 0x59, 0x0c, 0x56, 0x03, 0x00, 0x55, 0x55, 0x00, 0x03, 0x56, 0x0c, 0x59, 0x5a, 0x0f,
    0x6a, 0x3f, 0x3c, 0x69, 0x33, 0x66, 0x65, 0x30, 0x30, 0x65, 0x66, 0x33, 0x69, 0x3c, 0x3f, 0x6a,
    0x6a, 0x3f, 0x3c, 0x69, 0x33, 0x66, 0x65, 0x30, 0x30, 0x65, 0x66, 0x33, 0x69, 0x3c, 0x3f, 0x6a,
    0x0f, 0x5a, 0x59, 0x0c, 0x56, 0x03, 0x00, 0x55, 0x55, 0x00, 0x03, 0x56, 0x0c, 0x59, 0x5a, 0x0f,
    0x0c, 0x59, 0x5a, 0x0f, 0x55, 0x00, 0x03, 0x56, 0x56, 0x03, 0x00, 0x55, 0x0f, 0x5a, 0x59, 0x0c,
    0x69, 0x3c, 0x3f, 0x6a, 0x30, 0x65, 0x66, 0x33, 0x33, 0x66, 0x65, 0x30, 0x6a, 0x3f, 0x3c, 0x69,
    0x03, 0x56, 0x55, 0x00, 0x5a, 0x0f, 0x0c, 0x59, 0x59, 0x0c, 0x0f, 0x5a, 0x00, 0x55, 0x56, 0x03,
    0x66, 0x33, 0x30, 0x65, 0x3f, 0x6a, 0x69, 0x3c, 0x3c, 0x69, 0x6a, 0x3f, 0x65, 0x30, 0x33, 0x66,
    0x65, 0x30, 0x33, 0x66, 0x3c, 0x69, 0x6a, 0x3f, 0x3f, 0x6a, 0x69, 0x3c, 0x66, 0x33, 0x30, 0x65,
    0x00, 0x55, 0x56, 0x03, 0x59, 0x0c, 0x0f, 0x5a, 0x5a, 0x0f, 0x0c, 0x59, 0x03, 0x56, 0x55, 0x00
};

uint32_t byteoffs;

static inline int countbits(uint32_t byte)
{
        int res = 0;

        for (;byte; byte >>= 1)
                res += byte & 0x02;
        return res;
}

/**
 * nand_calculate_ecc - [NAND Interface] Calculate 3-byte ECC for 256-byte block
 * @mtd:    MTD block structure
 * @dat:    raw data
 * @ecc_code:    buffer for ECC
 */
int nand_calculate_ecc(u_char *dat, u_char *ecc_code)
{
    uint8_t idx, reg1, reg2, reg3, tmp1, tmp2;
    int i;

    /* Initialize variables */
    reg1 = reg2 = reg3 = 0;

    /* Build up column parity */
    for(i = 0; i < 256; i++) {
        /* Get CP0 - CP5 from table */
        idx = nand_ecc_precalc_table[*dat++];
        reg1 ^= (idx & 0x3f);

        /* All bit XOR = 1 ? */
        if (idx & 0x40) {
            reg3 ^= (uint8_t) i;
            reg2 ^= ~((uint8_t) i);
        }
    }

    /* Create non-inverted ECC code from line parity */
    tmp1  = (reg3 & 0x80) >> 0; /* B7 -> B7 */
    tmp1 |= (reg2 & 0x80) >> 1; /* B7 -> B6 */
    tmp1 |= (reg3 & 0x40) >> 1; /* B6 -> B5 */
    tmp1 |= (reg2 & 0x40) >> 2; /* B6 -> B4 */
    tmp1 |= (reg3 & 0x20) >> 2; /* B5 -> B3 */
    tmp1 |= (reg2 & 0x20) >> 3; /* B5 -> B2 */
    tmp1 |= (reg3 & 0x10) >> 3; /* B4 -> B1 */
    tmp1 |= (reg2 & 0x10) >> 4; /* B4 -> B0 */

    tmp2  = (reg3 & 0x08) << 4; /* B3 -> B7 */
    tmp2 |= (reg2 & 0x08) << 3; /* B3 -> B6 */
    tmp2 |= (reg3 & 0x04) << 3; /* B2 -> B5 */
    tmp2 |= (reg2 & 0x04) << 2; /* B2 -> B4 */
    tmp2 |= (reg3 & 0x02) << 2; /* B1 -> B3 */
    tmp2 |= (reg2 & 0x02) << 1; /* B1 -> B2 */
    tmp2 |= (reg3 & 0x01) << 1; /* B0 -> B1 */
    tmp2 |= (reg2 & 0x01) << 0; /* B7 -> B0 */

    /* Calculate final ECC code */
#ifdef CONFIG_MTD_NAND_ECC_SMC
    ecc_code[0] = ~tmp2;
    ecc_code[1] = ~tmp1;
#else
    ecc_code[0] = ~tmp1;
    ecc_code[1] = ~tmp2;
#endif
    ecc_code[2] = ((~reg1) << 2) | 0x03;

    return 0;
}

/**
 * nand_correct_data - [NAND Interface] Detect and correct bit error(s)
 * @mtd:    MTD block structure
 * @dat:    raw data read from the chip
 * @read_ecc:    ECC from the chip
 * @calc_ecc:    the ECC calculated from raw data
 *
 * Detect and correct a 1 bit error for 256 byte block
 */
int nand_correct_data(u_char *dat, u_char *read_ecc, u_char *calc_ecc)
{
    uint8_t s0, s1, s2;

#ifdef CONFIG_MTD_NAND_ECC_SMC
    s0 = calc_ecc[0] ^ read_ecc[0];
    s1 = calc_ecc[1] ^ read_ecc[1];
    s2 = calc_ecc[2] ^ read_ecc[2];
#else
    s1 = calc_ecc[0] ^ read_ecc[0];
    s0 = calc_ecc[1] ^ read_ecc[1];
    s2 = calc_ecc[2] ^ read_ecc[2];
#endif
    if ((s0 | s1 | s2) == 0)
        return 0;


    /* Check for a single bit error */
    if( ((s0 ^ (s0 >> 1)) & 0x55) == 0x55 &&
        ((s1 ^ (s1 >> 1)) & 0x55) == 0x55 &&
        ((s2 ^ (s2 >> 1)) & 0x54) == 0x54) {
        uint32_t bitnum;

        byteoffs = (s1 << 0) & 0x80;
        byteoffs |= (s1 << 1) & 0x40;
        byteoffs |= (s1 << 2) & 0x20;
        byteoffs |= (s1 << 3) & 0x10;

        byteoffs |= (s0 >> 4) & 0x08;
        byteoffs |= (s0 >> 3) & 0x04;
        byteoffs |= (s0 >> 2) & 0x02;
        byteoffs |= (s0 >> 1) & 0x01;

        bitnum = (s2 >> 5) & 0x04;
        bitnum |= (s2 >> 4) & 0x02;
        bitnum |= (s2 >> 3) & 0x01;

        dat[byteoffs] ^= (1 << bitnum);
        return 1;
    }


    if(countbits(s0 | ((uint32_t)s1 << 8) | ((uint32_t)s2 <<16)) == 1)
         return 0;
        
    return -1;
}

static void usage (void)
{
    puts (
        "nand_ecc_tools\n"
        "\n"
        "Syntax: nand_ecc_tools masterfile\n"
        "\n"
        "Options:\n"
        "  -h     Help output\n"
        "\n"
        "'masterfile'   is master file\n"
    );
}

int main(int argc, char **argv)
{
    FILE* f;
    int c;
    int i,j,k;
    int eccsteps, eccsize;
    int section_idx;
    int bytes;
    int err=0;
    u_char read_ecc[3]={0}, calc_ecc[3]={0};
    u_char dat[2048];
    u_char datbuf[256];
    u_char buf[256];
    u_char oob[64];
    
    if (argc < 2)
    {
        usage();
        return 0;
    }

    while ((c = getopt(argc, argv, "h")) > 0) {
        switch (c) {
        case 'h':
            usage ();
            exit(0);
        default:
            usage ();
            exit(1);
        }
    }

    //Open the master file, that the block size is 128Kb
    f = fopen(argv[1], "rb");
    
    if(f == NULL)
    {
        printf("I/O: error\n");
        return -1;
    }

    /* The first block of the master file contains the mini uboot which will use
    * RS ecc algorithm to do ECC, which is different from the other blocks of the
    * flash. This code only verify the bit flip of the data on the block other
    * than the first block.
    * Skip the first block data(128Kb) + OOB data(4Kb) = 0x21000
    */
    fseek(f, 0x21000, SEEK_SET);
    bytes = 0;
    eccsize = 0;
    section_idx = 0;
    for (i = 0; fread(&dat, sizeof(dat), 1, f) == 1; i++)
    {
    //read oob
        if (fread(&oob, sizeof(oob), 1, f) != 1)
        {
            break;
        }

        bytes=40;
        eccsize=0;
        for (j=0; j<sizeof(dat); j++)
        {
             if (j!=0 && j % 256 == 0)
             {
                //clone
                memcpy(datbuf, buf, 256);

                //cal ecc
                nand_calculate_ecc(buf, calc_ecc);

                read_ecc[0] = oob[bytes];
                read_ecc[1] = oob[bytes + 1];
                read_ecc[2] = oob[bytes + 2];
                bytes = bytes + 3;

                //check
                eccsteps = nand_correct_data(buf, read_ecc, calc_ecc);
                if (eccsteps == -1)
                {
                   printf("get ecc error in page %d, section %d\n", i, section_idx);
                   err = 1;
                }
                else if (eccsteps != 0)
                {
                  u_char bit = buf[byteoffs] ^ datbuf[byteoffs];
                  printf("[FIX] page %d, section %d, byte %d, diff bit = 0x%x\n", i, section_idx, byteoffs, bit);
                  err = 1;
                }

                eccsize = 0;
                buf[eccsize] = dat[j];
                eccsize++;
                section_idx++;
             }
             else
             {
                buf[eccsize] = dat[j];
                eccsize++;
             }
        }
    }
    
    fclose(f);

    if (err == 0)
      printf("I/O: finish, no ecc error\n");
    else if (err == 1)
        printf("I/O: finish, get ecc error\n");
       
    return 0;
}
