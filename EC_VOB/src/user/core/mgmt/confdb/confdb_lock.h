//
//  confdb_lock.h
//  confdb
//
//  Created by JunYing Yeh on 2014/11/7.
//
//

#ifndef _CONFDB_HEADER_LOCK_H_
#define _CONFDB_HEADER_LOCK_H_

enum
{
    CONFDB_LOCK_TYPE_RES          = 0,
    CONFDB_LOCK_TYPE_CONTEX,
    CONFDB_LOCK_NUM_OF_LOCK_TYPE
};

#define CONFDB_LOCK_MODE_LOCK       1
#define CONFDB_LOCK_MODE_UNLOCK     2
#define CONFDB_LOCK_MODE_READ       4
#define CONFDB_LOCK_MODE_WRITE      8

#if __cplusplus
extern "C" {
#endif

	void CONFDB_LOCK_Create();

	void CONFDB_LOCK_Lock(int mode, int type, const char *file, int line);

#if __cplusplus
}
#endif

#endif /* defined(_CONFDB_HEADER_LOCK_H_) */
