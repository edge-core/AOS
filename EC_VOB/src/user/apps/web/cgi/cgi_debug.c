#include "cgi_debug.h"

static UI32_T cgi_debug_flag;

UI32_T CGI_DEBUG_GetFlag()
{
    return cgi_debug_flag;
}

UI32_T CGI_DEBUG_ToggleFlag(CGI_DEBUG_FLAG flag)
{
    cgi_debug_flag ^= flag;

    return cgi_debug_flag;
}

UI32_T CGI_DEBUG_CleanFlag()
{
    cgi_debug_flag = 0;

    return cgi_debug_flag;
}


