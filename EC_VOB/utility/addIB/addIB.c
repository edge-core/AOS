/*-----------------------------------------------------------------------------
 * Module Name: addIB.c
 *-----------------------------------------------------------------------------
 * PURPOSE: add infomation block to a binary file
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    9/27/2013 - Vic Chang  , Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2013
 *-----------------------------------------------------------------------------
 */

#include    <stdio.h>
#include    <string.h>
#include    <stdlib.h>
#include    <time.h>
#include    "sys_imghdr.h"
#include    "sys_type.h"

#define FILE_NAME_LENGTH    (256)
#define COPY_BUFFER_SIZE    (1024)

#define SUPPORT_BID_BMP_UNIVERSAL 0xFFFFFFFF

typedef struct input_S
{
    char    in_filename[FILE_NAME_LENGTH];
    char    out_filename[FILE_NAME_LENGTH];
    unsigned long  MagicWord;        /* IMGHDR_MAGIC_PATTERN */
    unsigned long  ImageChecksum;
    unsigned long  SWVersion;        /* B1.B2.B3:formal release number,B4:RD build nmuber */
    unsigned long  ImageLength;      /* the length does not include that of image header */
    unsigned int   ImageType     :11;
    unsigned int   SupportBidBitmap:16;
    unsigned int   isUniversal   :1;
    unsigned int   FileType      :8; /* file type of runtime/diagnostic                  */
    unsigned int   FamilySerialId:5; /* A serial number for each family project          */
    unsigned int   Year          :7; /* year base on 2000 year                           */
    unsigned int   Month         :8; /* month                                            */
    unsigned int   Day           :8; /* day                                              */
    unsigned long  HeaderChecksum;    
    int n_overwrite;
} input_T;

static char    copy_buffer[COPY_BUFFER_SIZE];

BOOL_T IB_arg_handle_help(int argn, char *arg[], input_T *input)
{
    int i;

    for(i=1; i<argn; i++)
    {
        if(memcmp(arg[i], "-h", 2) != 0)
            continue;
        else
        {
            printf("\nUsage : Add infomation block to a binary file");
            printf("\nSyntax:");
            printf("\naddIB\t[-h] [-i{input file}] [-o{output file}] [-v{SWVersion}]");
            printf("\n\t[{-f | -d{date}}]  [-p{project ID}] [-b{SupportBidBitmap}] [-n]");
            printf("\n\t[-t{FILETYPE}]");
            printf("\n");
            printf("\n\t-h = This help message");
            printf("\n");
            printf("\n\t{input file} = Input file Name");
            printf("\n\t{output file} = Output file Name");
            printf("\n\t{SWVersion} = SWVersion number in doted format (i.e. x.x.x.x)");
            printf("\n\t               (in hexidecimal format)");
            printf("\n");
            printf("\n\t-f = Use system date as the time stamp");
            printf("\n\tor -d{date} = Use {date} as the time stamp");
            printf("\n\t{date} = yyyy/mm/dd");
            printf("\n\t{Project ID} = Project ID in hexidecimal format (i.e. 0x001a)");
            printf("\n\t{SupportBidBitmap} = Use hex number expression, 0xFFFFFFFF to indicate Universal");
            printf("\n\t{FileType} = Use hex number expression");
            printf("\n\tDo not overwrite input file");
            printf("\n");
            break;
        }
    }

    if(i>=argn)
        return TRUE;
    else
        return FALSE;
}

BOOL_T IB_arg_handle_input(int argn, char *arg[], input_T *input)
{
    int i;

    for(i=1; i<argn; i++)
    {
        if(memcmp(arg[i], "-i", 2) != 0)
            continue;
        else
        {
            strcpy(input->in_filename, (arg[i]+2));
            break;
        }
    }

    if(i >=argn)
    {
        printf("\nmust use -i to input file");
        return FALSE;
    }

    return TRUE;
}

BOOL_T IB_arg_handle_overwrite(int argn, char *arg[], input_T *input)
{
    int i;

    input->n_overwrite = 0;

    for (i = 1; i < argn; i++)
    {
        if (memcmp(arg[i], "-n", 2) != 0)
            continue;
        else
        {
            input->n_overwrite = 1;
            return TRUE;
        }
    }

    return FALSE;
}

BOOL_T IB_arg_handle_output(int argn, char *arg[], input_T *input)
{
    int i;

    for(i=1; i<argn; i++)
    {
        if(memcmp(arg[i], "-o", 2) != 0)
            continue;
        else
        {
            strcpy(input->out_filename, (arg[i]+2));
            break;
        }
    }

    if(i >=argn)
    {
        printf("\nmust use -o to out file");
        return FALSE;
    }

    return TRUE;
}

BOOL_T IB_arg_handle_SWVersion(int argn, char *arg[], input_T *input)
{
    int i;
    int rv;
    unsigned int ver[4];

    for(i=1; i<argn; i++)
    {
	    if(memcmp(arg[i], "-v", 2) != 0)
		    continue;
	    else
	    {
		    sscanf((arg[i]+2),"%x", &input->SWVersion);
		    break;
	    }
    }

#if 0
    input->SWVersion[0] = (UI8_T) ver[0];
    input->SWVersion[1] = (UI8_T) ver[1];
    input->SWVersion[2] = (UI8_T) ver[2];
    input->SWVersion[3] = (UI8_T) ver[3];
#endif
    return TRUE;
}

BOOL_T IB_arg_handle_date(int argn, char *arg[], input_T *input)
{
    int i;
    unsigned int year, month, day;

    for(i=1; i<argn; i++)
    {
        if(memcmp(arg[i], "-d", 2) != 0)
            continue;
        else
        {
            sscanf((arg[i]+2),"%d/%d/%d", &year, &month, &day);
            if(year < 2000)
            {
                printf("year must > 2000");
	            return FALSE;
            }

            year = year - 2000;
            break;
        }
    }

    if(i >= argn)
    {
        for(i=1; i<argn; i++)
        {
            if(memcmp(arg[i], "-f", 2) != 0)
                continue;
            else
            {
                time_t    lt;
                struct tm *tm_ptr;
        
                lt = time(NULL);
                tm_ptr = localtime(&lt);
                year = (tm_ptr->tm_year - 100);
                month = (tm_ptr->tm_mon+1);
                day = (tm_ptr->tm_mday);
                break;
            }
        }
        
    }

    if(i >= argn)
    {
        printf("\nmust use -f / -d to input date and time");
        return FALSE;
    }

    input->Year = (UI16_T) year;
    input->Month = (UI8_T) month;
    input->Day = (UI8_T) day;            

    return TRUE;
}

BOOL_T IB_arg_handle_pid(int argn, char *arg[], input_T *input)
{
    int i;
    unsigned short pid;
    int value=0;

    for(i=1; i<argn; i++)
    {
        if(memcmp(arg[i], "-p", 2) != 0)
            continue;
        else
        {
            sscanf((arg[i]+2),"%lx", &value);
            input->ImageType = value;
            break;
        }
    }

    if(i >= argn)
    {
        printf("\nmust use -p to specify project id");
        return FALSE;
    }

    return TRUE;

}

BOOL_T IB_arg_handle_bitmap(int argn, char *arg[], input_T *input)
{
    int i;
    unsigned short pid;
    int value=0;

    for(i=1; i<argn; i++)
    {
        if(memcmp(arg[i], "-b", 2) != 0)
            continue;
        else
        {
            sscanf((arg[i]+2),"%lx", &value);
            if (value == SUPPORT_BID_BMP_UNIVERSAL)
            {
                input->isUniversal = 1;
            }
	    else
            {
                input->isUniversal = 0;
            	input->SupportBidBitmap = (UI16_T) value;
            }
            break;
        }
    }

    if(i >= argn)
    {
        printf("\nmust use -b to bitmap (board ID)");
        return FALSE;
    }

    return TRUE;
}

BOOL_T IB_arg_handle_filetype(int argn, char *arg[], input_T *input)
{
    int i;
    unsigned short pid;
    int value=0;
    
    for(i=1; i<argn; i++)
    {
        if(memcmp(arg[i], "-t", 2) != 0)
            continue;
        else
        {
            sscanf((arg[i]+2),"%lx", &value);
            input->FileType = value;
            break;
        }
    }

    if(i >= argn)
    {
        printf("\nmust use -t to FILE TYPE");
        return FALSE;
    }

    return TRUE;
}

BOOL_T IB_arg_handle(int argn, char *arg[], input_T *input)
{

  if(IB_arg_handle_help(argn, arg, input)==FALSE)
  { 
    return FALSE;
  }

  if(IB_arg_handle_input(argn, arg, input)==FALSE)
  {
    return FALSE;
  }

  if(IB_arg_handle_output(argn, arg, input)==FALSE)
  {
    return FALSE;
  }

  if(IB_arg_handle_SWVersion(argn, arg, input)==FALSE)
  {
    return FALSE;
  }

  if(IB_arg_handle_date(argn, arg, input)==FALSE)
  {
    return FALSE;
  }

  if(IB_arg_handle_pid(argn, arg, input)==FALSE)
  {
    return FALSE;
  }

  if(IB_arg_handle_bitmap(argn, arg, input)==FALSE)
  {
    return FALSE;
  }

  if(IB_arg_handle_filetype(argn, arg, input)==FALSE)
  {
    return FALSE;
  }

  return TRUE;
}

UI16_T IB_Endian_Swap16(UI16_T value)
{
    return (UI16_T)((value&0xff00)/0x0100 + (value&0x00ff)*0x0100);
}

UI32_T IB_Endian_Swap32(UI32_T value)
{
    return (UI32_T)((value&0x000000ff)*0x01000000 + (value&0x0000ff00)*0x0100 + (value&0xff0000)/0x0100 + (value&0xff000000)/0x01000000);
}

UI32_T IB_XOR_Checksum(UI8_T *copy_buffer, UI32_T cur_length, UI32_T checksum)
{
    UI32_T i;
    
    for(i=0; i<cur_length; i+=sizeof(UI32_T))
    {
        checksum ^= (*(UI32_T*)(copy_buffer+i));
    }
    
    return checksum;
}

int IB_checksum(FILE *in, IH_T *lib)
{
    UI32_T length, checksum, cur_length;
        
    length = 0;
    cur_length=0;
    checksum = 0;

    memset(copy_buffer, 0x0, COPY_BUFFER_SIZE);
    while((cur_length = fread(copy_buffer, sizeof(UI8_T), COPY_BUFFER_SIZE, in)) != 0)
    {
        if((cur_length % sizeof(UI32_T)) != 0)
        {
            cur_length = (cur_length+sizeof(UI32_T))/sizeof(UI32_T)*sizeof(UI32_T);
        }

        checksum = IB_XOR_Checksum(copy_buffer, cur_length, checksum);
        length+=cur_length;
        memset(copy_buffer, 0x0, COPY_BUFFER_SIZE);
        printf(".");
    }
    printf("ok\n\r");
    lib->ImageLength = length + sizeof(IH_T);
    lib->ImageChecksum = checksum;
    
    return TRUE;
}

int IB_file_copy(FILE *in, FILE *out)
{
    UI32_T length, cur_length;
            
    length = 0;
    cur_length=0;

    memset(copy_buffer, 0x0, COPY_BUFFER_SIZE);

    while((cur_length = fread(copy_buffer, sizeof(UI8_T), COPY_BUFFER_SIZE, in)) != 0)
    {
        if((cur_length % sizeof(UI32_T)) != 0)
        {
            cur_length = (cur_length+sizeof(UI32_T))/sizeof(UI32_T)*sizeof(UI32_T);
        }
        
        fwrite(copy_buffer, sizeof(UI8_T), cur_length, out);

        length+=cur_length;
        memset(copy_buffer, 0x0, COPY_BUFFER_SIZE);
    }
    
    return TRUE;
}

int IB_ib_checksum(IH_T *lib)
{

    UI32_T checksum;
    
    checksum = 0;
    checksum = IB_XOR_Checksum((UI8_T *) lib, sizeof(IH_T), checksum);
    printf("\n\rCPLD Header Checksum = 0x%08lX\n\r", checksum);
    lib->HeaderChecksum = IB_Endian_Swap32(checksum);   
    return TRUE;
}

int IB_append_info_block(FILE *out, IH_T *lib)
{
    
    printf("\n");
    printf("\n\rInfomation Block ");
    printf("\n\r------------------------");
    printf("\n\rProject ID     = %lx", lib->ImageType);
    printf("\n\rSWVersion        = %lx", lib->SWVersion);
    printf("\n\rSupportBidBitmap  =  %lx", lib->SupportBidBitmap);
    printf("\n\risUniversal = %01d", lib->isUniversal);
    printf("\n\rFileType = %d", lib->FileType);
    printf("\n\rImage Length   = %ld bytes (0x%08lx)", lib->ImageLength, lib->ImageLength);
    printf("\n\rImage Checksum = 0x%08lX\n", lib->ImageChecksum);
    printf("\n\rDate           = %d/%02d/%02d", lib->Year + 2000, lib->Month, lib->Day);

    /* swap entries required */
    lib->MagicWord = IB_Endian_Swap32(IMGHDR_IMAGE_CPLDTYPE);
    lib->ImageType = IB_Endian_Swap32(lib->ImageType);
    lib->SupportBidBitmap = IB_Endian_Swap32(lib->SupportBidBitmap);
    lib->isUniversal = IB_Endian_Swap32(lib->isUniversal);
    lib->SWVersion = IB_Endian_Swap32(lib->SWVersion);
    lib->ImageLength = IB_Endian_Swap32(lib->ImageLength);
    lib->ImageChecksum = IB_Endian_Swap32(lib->ImageChecksum);

    /* calculate and show header checksum */
    IB_ib_checksum(lib);

    if(fwrite(lib, sizeof(IH_T), 1, out))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
    
}

int main(int argn, char *arg[])
{
    input_T    input;
    FILE    *in, *out;
    IH_T lib;
    
    memset(&input,0, sizeof(input));

    printf("Add infomation block to a binary file");

    if(IB_arg_handle(argn, arg, &input)==FALSE)
    {
        printf("\nYou can specify -h to check command usage\n");
        return FALSE;
    }
    
    if ((in=fopen(input.in_filename, "r+b")) == NULL)
    {
        printf("\n\r(!) Unable to open input file - %s.", input.in_filename);
        return FALSE;
    }
    
    if ((out=fopen(input.out_filename, "w+b")) == NULL)
    {
        printf("\n\r(!) Unable to open output file - %s.", input.out_filename);
        fclose(in);
        return FALSE;
    }
    
    printf("\n\rSource File : %s", input.in_filename);
    printf("\n\rTarget File : %s", input.out_filename);

    printf("\n\rAppending CPLD Header. %d", input.n_overwrite);
    printf("\n\rVME CheckSum");	
		
    if(!IB_checksum(in, &lib))
    {
        printf("\n\r(!) File open error.");
        goto error;
    }

    lib.MagicWord = IMGHDR_MAGIC_PATTERN;
    
    lib.ImageType = input.ImageType;
    lib.SWVersion = input.SWVersion;
    lib.SupportBidBitmap = input.SupportBidBitmap;
    lib.isUniversal = input.isUniversal;
    lib.FileType = input.FileType;
    lib.Year = input.Year;
    lib.Month = input.Month;
    lib.Day = input.Day;

    if(!IB_append_info_block(out, &lib))
    {
        printf("\n\r(!) Failed to append information block.");
        goto error;
    }

#if 0
    if (input.n_overwrite == (UI16_T)1)
        fseek(in, 0, SEEK_SET);
    else
        fseek(in, sizeof(IH_T), SEEK_SET);
#endif
    fseek(in, 0, SEEK_SET);
    fseek(out, sizeof(IH_T), SEEK_SET);

    IB_file_copy(in,out);
    fclose(in);
    fclose(out);

    return 0;

error:    
    fclose(in);
    fclose(out);
    return 1;
            
}
