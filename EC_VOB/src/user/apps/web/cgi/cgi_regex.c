#include "cgi.h"

pcre2_code * re_compile(const char *pattern)
{
    pcre2_code *re;
    PCRE2_SPTR _pattern;

    int errornumber;
    PCRE2_SIZE erroroffset;

    _pattern = (PCRE2_SPTR)pattern;

    re = pcre2_compile(_pattern,              /* the pattern */
        PCRE2_ZERO_TERMINATED,/* indicates pattern is zero-terminated */
        0,                    /* default options */
        &errornumber,         /* for error number */
        &erroroffset,         /* for error offset */
        NULL);                /* use default compile context */

    return re;
}

BOOL_T re_test(const char *pattern, const char *str)
{
    pcre2_code *re;
    PCRE2_SPTR _pattern;     /* PCRE2_SPTR is a pointer to unsigned code units of */
    PCRE2_SPTR subject;     /* the appropriate width (8, 16, or 32 bits). */
    size_t subject_length;

    int errornumber;
    PCRE2_SIZE erroroffset;

    pcre2_match_data *match_data;

    char *u8pattern;

    int rc;

    u8pattern = path2Regexp(pattern, FALSE);
    _pattern = (PCRE2_SPTR)u8pattern;
    subject = (PCRE2_SPTR)str;
    subject_length = strlen((char *)subject);

    re = pcre2_compile(_pattern,              /* the pattern */
        PCRE2_ZERO_TERMINATED,/* indicates pattern is zero-terminated */
        0,                    /* default options */
        &errornumber,         /* for error number */
        &erroroffset,         /* for error offset */
        NULL);                /* use default compile context */
    if (re == NULL) {
        L_MM_Free(u8pattern);
        return FALSE;
    }

    match_data = pcre2_match_data_create_from_pattern(re, NULL);
    rc = pcre2_match(re,                   /* the compiled pattern */
        subject,              /* the subject string */
        subject_length,       /* the length of the subject */
        0,                    /* start at offset 0 in the subject */
        0,                    /* default options */
        match_data,           /* block for storing the result */
        NULL);                /* use default match context */
    //            if (0 < rc) {
    //                found = TRUE;
    //            }

    L_MM_Free(u8pattern);
    pcre2_match_data_free(match_data);   /* Release memory used for the match */
    pcre2_code_free(re);                 /* data and the compiled pattern. */
    //            return found;
    return (0 <= rc) ? TRUE : FALSE;
}