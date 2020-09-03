/*
 * Copyright (c) 2015-16  David Lamparter, for NetDEF, Inc.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _FRR_MODULE_H
#define _FRR_MODULE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct frrmod_runtime;

struct frrmod_info {
	/* single-line few-word title */
	const char *name;
	/* human-readable version number, should not contain spaces */
	const char *version;
	/* one-paragraph description */
	const char *description;

	int (*init)(void);
};

/* primary entry point structure to be present in loadable module under
 * "_frrmod_this_module" dlsym() name
 *
 * note space for future extensions is reserved below, so other modules
 * (e.g. memory management, hooks) can add fields
 *
 * const members/info are in frrmod_info.
 */
struct frrmod_runtime {
	struct frrmod_runtime *next;

	const struct frrmod_info *info;
	void *dl_handle;
	bool finished_loading;

	char *load_name;
	char *load_args;
};

/* space-reserving foo */
struct _frrmod_runtime_size {
	struct frrmod_runtime r;
	/* this will barf if frrmod_runtime exceeds 1024 bytes ... */
	uint8_t space[1024 - sizeof(struct frrmod_runtime)];
};
union _frrmod_runtime_u {
	struct frrmod_runtime r;
	struct _frrmod_runtime_size s;
};

extern union _frrmod_runtime_u _frrmod_this_module;
#define THIS_MODULE (&_frrmod_this_module.r)

#define FRR_COREMOD_SETUP(...)                                                 \
	static const struct frrmod_info _frrmod_info = {__VA_ARGS__};          \
	DSO_LOCAL union _frrmod_runtime_u _frrmod_this_module = {{             \
		NULL,                                                          \
		&_frrmod_info,                                                 \
	}};
#define FRR_MODULE_SETUP(...)                                                  \
	FRR_COREMOD_SETUP(__VA_ARGS__)                                         \
	DSO_SELF struct frrmod_runtime *frr_module = &_frrmod_this_module.r;

extern struct frrmod_runtime *frrmod_list;

extern void frrmod_init(struct frrmod_runtime *modinfo);
extern struct frrmod_runtime *frrmod_load(const char *spec, const char *dir,
					  char *err, size_t err_len);
#if 0
/* not implemented yet */
extern void frrmod_unload(struct frrmod_runtime *module);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _FRR_MODULE_H */