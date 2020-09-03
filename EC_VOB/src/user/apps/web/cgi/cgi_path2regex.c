#include "cgi.h"

static size_t p2r_path_level(const char *path)
{
    size_t level;
    for (level = 0; *path; path++)
    {
        if (*path == '/')
        {
            ++level;
        }
    }

    return level;
}

char * path2Regexp(const char *path, BOOL_T strict)
{
    char *p;
    char *n;

    enum {
        start,
        slash,
        open_capture,
        close_capture
    };

    // not exact length
    p = n = (char *)L_MM_Malloc(strlen(path) + p2r_path_level(path) * sizeof("[^/]+?") + 10, 123);

    int state = start;

    *p++ = '^';

    for (; *path; ++path)
    {
        switch (*path)
        {
        case '/':
            //                        *p++ = '\\';
            *p++ = '/';

            state = slash;
            break;

        case '{':
            state = open_capture;
            break;

        case '}':
            if (state == open_capture)
            {
                // [^\/]+?
                *p++ = '[';
                *p++ = '^';
                //                            *p++ = '\\';
                *p++ = '/';
                *p++ = ']';
                *p++ = '+';
                *p++ = '?';
            }
            state = close_capture;
            break;

        default:
            if (state == open_capture)
            {
                break;
            }

            *p++ = *path;
        }
    }

    if (strict)
    {
        *p++ = '$';
    }

    *p = '\0';

    return n;
}