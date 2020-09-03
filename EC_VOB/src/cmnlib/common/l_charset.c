/*
 * Copyright(C)      Accton Corporation,  2008
 *
 * Module Name: l_charset.c
 *
 * Purpose: Accton platform string verification facilities
 *
 * Notes:
 *
 * History:
 *   Date           Modifier        Reason
 * --------------- ------------     -----------------------------------
 * 2008.04.29      Steven Jiang     Linux platform ask for general string verfication APIs
 */


/* Valid Char-Set
 *      - Generic String: all printable characters
 *      - User Name: [a-zA-Z0-9@_-.] (it can support email address like names, sucha
 *                   as 'david@192.168.101.1'
 *      - Password: All printable characters include blank character.
 *                  [a-zA-Z0-9'~!@#$%^&*()[]{};':".<>/?\| ]
 *      - File Name: [a-zA-Z0-9-_.]
 *      - Path Name: [a-zA-Z0-9-_.:/\]
 */

#include <ctype.h>
#include <string.h>
#include "sys_type.h"

#define CHAR_BETWEEN(c, left, right)    (((left) <= (c)) && ((c) <= (right)))

/*
 * FUNCTION:    L_CHARSET_IsPrintableChar
 *
 * PURPOSE:     Verify if given character belongs to printable character
 *
 * INPUT:       c       - character
 *
 * RETURN:      0       - not printable char
 *              non-0   - printable char
 *
 * NOTES:       N/A
 */
int
L_CHARSET_IsPrintableChar(const char c)
{
    return isprint((int) c);
}

/*
 * FUNCTION:    L_CHARSET_TrimTrailingNonPrintableChar
 *
 * PURPOSE:     Trim the trailing non-printable char in the given string.
 *
 * INPUT:       str_p   - pointer to string to be trimmed.
 *
 * OUTPUT:      str_p   - the trimmed string.
 *
 * RETURN:      0       - Error occurs when trimming the string.
 *              1       - Successfully
 *
 * NOTES:       If all chars in the given string are non-printable, the given
 *              string will become empty string.
 */
int
L_CHARSET_TrimTrailingNonPrintableChar(char* str_p)
{
    int pos;

    if(str_p==NULL)
        return 0;

    pos=strlen(str_p);

    if(pos==0)
        return 1;

    /* trim the non-printable trailing char */
    for(pos=pos-1; pos>=0; pos--)
    {
        if(L_CHARSET_IsPrintableChar(str_p[pos])==0)
        {
            /* trim non-printable trailing char
             */
            str_p[pos]='\0';
        }
        else
        {
            break;
        }
    }
    return 1;
}

/*
 * FUNCTION:    L_CHARSET_IsValidGenericChar
 *
 * PURPOSE:     Verify if given character belongs to generic printable character
 *
 * INPUT:       c       - character
 *
 * RETURN:      0       - invalid
 *              non-0   - valid
 *
 * NOTES:       N/A
 */
int
L_CHARSET_IsValidGenericChar(const char c)
{
    return isprint((int) c) && c != '?';
}

/*
 * FUNCTION:    L_CHARSET_IsValidGenericString
 *
 * PURPOSE:     Verify if given string is a valid generic printable string
 *
 * INPUT:       s       - string to be verified
 *
 * RETURN:      0       - invalid
 *              non-0   - valid
 *
 * NOTES:       N/A
 */
int
L_CHARSET_IsValidGenericString(const char *s)
{
    /* NULL string or string without any character */
    if (!s || !*s) {
        return 0;
    }

    while (*s && L_CHARSET_IsValidGenericChar(*s)) {
        s++;
    }

    return !*s;
}

/*
 * FUNCTION:    L_CHARSET_IsValidUserNameChar
 *
 * PURPOSE:     Verify if given character belongs to valid characters can be used in user name
 *
 * INPUT:       c       - character
 *
 * RETURN:      0       - invalid
 *              non-0   - valid
 *
 * NOTES:       N/A
 */
int
L_CHARSET_IsValidUserNameChar(const char c)
{

    return (CHAR_BETWEEN(c, 'a', 'z')
            || CHAR_BETWEEN(c, 'A', 'Z')
            || CHAR_BETWEEN(c, '0', '9')
            || (c == '@')
            || (c == '_')
            || (c == '-')
            || (c == '.')
           );
}

/*
 * FUNCTION:    L_CHARSET_IsValidUserNameString
 *
 * PURPOSE:     Verify if given string is a valid user name (string)
 *
 * INPUT:       s       - string to be verified
 *
 * RETURN:      0       - invalid
 *              non-0   - valid
 *
 * NOTES:       N/A
 */
int
L_CHARSET_IsValidUserNameString(const char *s)
{
    /* NULL string or string without any character */
    if (!s || !*s) {
        return 0;
    }

    while (*s && L_CHARSET_IsValidUserNameChar(*s)) {
        s++;
    }

    return !*s;
}

/*
 * FUNCTION:    L_CHARSET_IsValidPasswordChar
 *
 * PURPOSE:     Verify if given character belongs to valid characters can be used in password
 *
 * INPUT:       c       - character
 *
 * RETURN:      0       - invalid
 *              non-0   - valid
 *
 * NOTES:       N/A
 */
int
L_CHARSET_IsValidPasswordChar(const char c)
{
#if 1
    return isprint((int) c);
#else
    return (CHAR_BETWEEN(c, 'a', 'z')
            || CHAR_BETWEEN(c, 'A', 'Z')
            || CHAR_BETWEEN(c, '0', '9')
            || (c == '`')
            || (c == '~')
            || (c == '!')
            || (c == '@')
            || (c == '#')
            || (c == '$')
            || (c == '%')
            || (c == '^')
            || (c == '&')
            || (c == '*')
            || (c == '(')
            || (c == ')')
            || (c == '-')
            || (c == '_')
            || (c == '+')
            || (c == '=')
            || (c == '[')
            || (c == '{')
            || (c == ']')
            || (c == '}')
            || (c == '\\')
            || (c == '|')
            || (c == ';')
            || (c == ':')
            || (c == '\'')
            || (c == '"')
            || (c == ',')
            || (c == '<')
            || (c == '.')
            || (c == '>')
            || (c == '/')
            || (c == '?')
            || (c == ' ')
           );
#endif
}

/*
 * FUNCTION:    L_CHARSET_IsValidPasswordString
 *
 * PURPOSE:     Verify if given string is a valid password (string)
 *
 * INPUT:       s       - string to be verified
 *
 * RETURN:      0       - invalid
 *              non-0   - valid
 *
 * NOTES:       N/A
 */
int
L_CHARSET_IsValidPasswordString(const char *s)
{
    /* NULL string or string without any character */
    if (!s || !*s) {
        return 0;
    }

    while (*s && L_CHARSET_IsValidPasswordChar(*s)) {
        s++;
    }

    return !*s;
}

/*
 * FUNCTION:    L_CHARSET_IsValidFileNameChar
 *
 * PURPOSE:     Verify if given character belongs to valid characters can be used in file name
 *
 * INPUT:       c       - character
 *
 * RETURN:      0       - invalid
 *              non-0   - valid
 *
 * NOTES:       N/A
 */
int
L_CHARSET_IsValidFileNameChar(const char c)
{
    return (CHAR_BETWEEN(c, 'a', 'z')
            || CHAR_BETWEEN(c, 'A', 'Z')
            || CHAR_BETWEEN(c, '0', '9')
            || (c == '-')
            || (c == '_')
            || (c == '.')
#if 0
            || (c == '`')
            || (c == '~')
            || (c == '!')
            || (c == '@')
            || (c == '#')
            || (c == '$')
            || (c == '%')
            || (c == '^')
            || (c == '&')
            || (c == '*')
            || (c == '(')
            || (c == ')')
            || (c == '+')
            || (c == '=')
            || (c == '[')
            || (c == '{')
            || (c == ']')
            || (c == '}')
            || (c == '\\')
            || (c == '|')
            || (c == ';')
            || (c == ':')
            || (c == '\'')
            || (c == '"')
            || (c == ',')
            || (c == '<')
            || (c == '>')
            || (c == '/')
            || (c == '?')
            || (c == ' ')
#endif
           );
}

/*
 * FUNCTION:    L_CHARSET_IsValidFileNameString
 *
 * PURPOSE:     Verify if given string is a valid file name (string)
 *
 * INPUT:       s       - string to be verified
 *
 * RETURN:      0       - invalid
 *              non-0   - valid
 *
 * NOTES:       N/A
 */
int
L_CHARSET_IsValidFileNameString(const char *s)
{
    /* NULL string or string without any character */
    if (!s || !*s) {
        return 0;
    }

    while (*s && L_CHARSET_IsValidFileNameChar(*s)) {
        s++;
    }

    return !*s;
}

/*
 * FUNCTION:    L_CHARSET_IsValidPathNameChar
 *
 * PURPOSE:     Verify if given character belongs to valid characters can be used in path name
 *
 * INPUT:       c       - character
 *
 * RETURN:      0       - invalid
 *              non-0   - valid
 *
 * NOTES:       N/A
 */
int
L_CHARSET_IsValidPathNameChar(const char c)
{
    return (L_CHARSET_IsValidFileNameChar(c)
            || (c == ':')
            || (c == '/')
            || (c == '\\')
           );
}

/*
 * FUNCTION:    L_CHARSET_IsValidPathNameString
 *
 * PURPOSE:     Verify if given string is a valid path name (string)
 *
 * INPUT:       s       - string to be verified
 *
 * RETURN:      0       - invalid
 *              non-0   - valid
 *
 * NOTES:       N/A
 */
int
L_CHARSET_IsValidPathNameString(const char *s)
{
    /* NULL string or string without any character */
    if (!s || !*s) {
        return 0;
    }

    while (*s && L_CHARSET_IsValidPathNameChar(*s)) {
        s++;
    }

    return !*s;
}


