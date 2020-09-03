/*-----------------------------------------------------------------------------
 * Module Name: passwdgenerate.c
 *-----------------------------------------------------------------------------
 * PURPOSE: use project ID and project name as input and use "simbaaos" as base string to come out a password.

 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    10/04/2013 - Vic Chang  , Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2013
 *-----------------------------------------------------------------------------
 */

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_STR_BUF_SIZE 255

#define BASE_STRING "simbaaos"
#define BASE_STRING_SIZE sizeof(BASE_STRING)
#define BASE_STRING_LEN  (BASE_STRING_SIZE-1)
#define CHARSET_STRING "abcdefghijklmnopqrstuvwxyz1234567890"
#define CHARSET_SIZE sizeof(CHARSET_STRING)
#define CHARSET_LEN (CHARSET_SIZE-1)

/* Purpose: encrypt string
 * Input/Output:    data --- data which needs to encrypt
 * Return:  None
 * Notes:
 *  
 */
void generate(char* result_p,char *project_id_p, char *project_name_p)
{
    char charset[] = "abcdefghijklmnopqrstuvwxyz1234567890";
    char base_string[BASE_STRING_SIZE]=BASE_STRING;
    int i, index[BASE_STRING_LEN]={0}, charsetLen, inputStrLen;

    charsetLen = strlen(charset);
    inputStrLen = strlen(project_id_p);

    for(i=0; i<inputStrLen; i++)
    {
       index[i%(BASE_STRING_LEN/2)] += base_string[i%(BASE_STRING_LEN/2)] + project_id_p[i];
    }

    for(i=0; i<(BASE_STRING_LEN/2); i++)
        result_p[i] = charset[index[i]%CHARSET_LEN];

    inputStrLen = strlen(project_name_p);
    
    for(i=0; i<inputStrLen; i++)
    {
       index[(BASE_STRING_LEN/2) + (i%(BASE_STRING_LEN/2))] +=
           base_string[(BASE_STRING_LEN/2) + (i%(BASE_STRING_LEN/2))] + project_name_p[i];
    }

    for(i=BASE_STRING_LEN/2; i<BASE_STRING_LEN; i++)
        result_p[i] = charset[index[i]%CHARSET_LEN];
}

static void usage (void)
{
    puts (
        "passwordgenerate\n"
        "\n"
        "Syntax:    passwdgenerate [options] project_id project_name\n"
        "\n"
        "Options:\n"
        "  -h     Help output\n"
        "\n"
        "'project_id'   is your project id\n"
        "'project_name' is your project name\n"
    );
}

int main (int argc, char *argv[])
{
    int c;
    char project_id[DEFAULT_STR_BUF_SIZE];
    char project_name[DEFAULT_STR_BUF_SIZE];
    char password[DEFAULT_STR_BUF_SIZE]="";

    if (argc < 2)
    {
        usage();
        return 0;
    }

    while ((c = getopt(argc, argv, "h")) > 0)
    {
        switch (c)
        {
            case 'h':
                usage ();
                break;
            default:
                usage ();
                break;
        }
    }

    strcpy (project_id, argv[optind]);
    strcpy (project_name, argv[optind+1]);
   
    generate(password, project_id, project_name);
    printf("Password Generate: '%s'\n", password);

    return 0;
}
