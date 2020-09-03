#include <time.h>
#include <string.h>
#include "radiusclient.h"
#include "radius_om.h"

/*
 * Function: rc_guess_seqnbr
 *
 * Purpose: return a random sequence number
 *
 */
UI8_T rc_guess_seqnbr(void)
{
/*isiah. avoid different request packet using the same identifier.*/
    static  UI8_T   previous_id = 0;
    UI8_T   current_id;
/*  srandom((unsigned int)(time(NULL)+getpid()));*/
//  srand((unsigned)(time(0)));
    srand((unsigned)(SYSFUN_GetSysTick()));
    current_id = (UI8_T)(rand() % UCHAR_MAX);
    if( previous_id == current_id )
    {
        current_id += 1;
    }
    previous_id = current_id;
    return (current_id);
}

/*
 * Function: rc_random_vector
 *
 * Purpose: generates a random vector of AUTH_VECTOR_LEN octets.
 *
 * Returns: the vector (call by reference)
 *
 */
void rc_random_vector (UI8_T *vector)
{
    int             randno;
    int             i;
#if 0
#if defined(HAVE_DEV_URANDOM)
    int     fd;

/* well, I added this to increase the security for user passwords.
   we use /dev/urandom here, as /dev/random might block and we don't
   need that much randomness. BTW, great idea, Ted!     -lf, 03/18/95   */

    if ((fd = open(_PATH_DEV_URANDOM, O_RDONLY)) >= 0)
    {
        UI8_T *pos;
        int readcount;

        i = AUTH_VECTOR_LEN;
        pos = vector;
        while (i > 0)
        {
            readcount = read(fd, (char *)pos, i);
            pos += readcount;
            i -= readcount;
        }

        close(fd);
        return;
    } /* else fall through */
#endif
#endif
    /*srand (time (0) + getppid() + getpid()); */ /* random enough :) */
        srand (time (0)); /* kevin */
    for (i = 0; i < AUTH_VECTOR_LEN;)
    {
        randno = rand ();
        memcpy ((char *) vector, (char *) &randno, sizeof (int));
        vector += sizeof (int);
        i += sizeof (int);
    }

    return;
}
