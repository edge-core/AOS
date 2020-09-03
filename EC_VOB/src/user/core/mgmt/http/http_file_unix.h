//
//  http_file_unix.h
//  http
//
//  Created by JunYing Yeh on 2015/8/6.
//
//

#ifndef _HTTP_HEADER_FILE_UNIX_H_
#define _HTTP_HEADER_FILE_UNIX_H_

#if __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define FS_ISREG(sb)    S_ISREG((sb)->st_mode)
#define FS_MTIME(sb)    (sb)->st_mtime

#if __cplusplus
}
#endif

#endif /* _HTTP_HEADER_FILE_UNIX_H_ */
