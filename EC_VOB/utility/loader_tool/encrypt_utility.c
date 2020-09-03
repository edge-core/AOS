/*-----------------------------------------------------------------------------
 * Module Name: encrypt.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Generating a encrypted password which will be defined as a constant in the header file.
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

#define INPUT_LEN 255

static void usage (void)
{
    puts (
        "encrypt\n"
        "\n"
        "Syntax:    encrypt [options] password_text\n"
        "\n"
        "Options:\n"
        "  -h     Help output\n"
        "\n"
        "'password_text'   is your password\n"
    );
}

int main (int argc, char *argv[])
{
    int c;
    bool use_rgb = false;
    char inputfile[INPUT_LEN];

    if (argc < 2)
    {
        usage();
        return 0;
    }

    while ((c = getopt(argc, argv, "hr")) > 0) {
        switch (c) {
        case 'h':
            usage ();
            exit(0);
        default:
            usage ();
            exit(1);
        }
    }

    strcpy (inputfile, argv[optind]);   
    Encrypt(inputfile);
    printf("Encrypted Password: %s\n", inputfile);

    return 0;
}
