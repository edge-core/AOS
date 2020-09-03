//
//  http_time_win.h
//  http
//
//  Created by JunYing Yeh on 2014/7/28.
//
//

#ifndef _HTTP_HEADER_TIME_WIN_H_
#define _HTTP_HEADER_TIME_WIN_H_

#if __cplusplus
extern "C" {
#endif

# ifndef timersub
#    define timersub(a, b, result) \
            (result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
            (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
            if ((result)->tv_usec < 0) { \
                    --(result)->tv_sec; \
                    (result)->tv_usec += 1000000; \
            } \

# endif

# ifndef timeradd
#    define timeradd(a, b, result) \
            (result)->tv_sec = (a)->tv_sec + (b)->tv_sec; \
            (result)->tv_usec = (a)->tv_usec + (b)->tv_usec; \
            if ((result)->tv_usec >= 1000000) \
            { \
                    ++(result)->tv_sec; \
                    (result)->tv_usec -= 1000000; \
            } \

# endif

int gettimeofday(struct timeval *tp, void *tzp);
struct tm *localtime_r(const time_t *, struct tm *);

#if __cplusplus
}
#endif


#endif /* _HTTP_HEADER_TIME_WIN_H_ */
