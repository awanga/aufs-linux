/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Copyright (C) 2005-2022 Junjiro R. Okajima
 */

#ifndef __AUFS_TYPE_H__
#define __AUFS_TYPE_H__

#define AUFS_NAME	"aufs"

#ifdef __KERNEL__
/*
 * define it before including all other headers.
 * sched.h may use pr_* macros before defining "current", so define the
 * no-current version first, and re-define later.
 */
#define pr_fmt(fmt)	AUFS_NAME " %s:%d: " fmt, __func__, __LINE__
#include <linux/sched.h>
#undef pr_fmt
#define pr_fmt(fmt) \
		AUFS_NAME " %s:%d:%.*s[%d]: " fmt, __func__, __LINE__, \
		(int)sizeof(current->comm), current->comm, current->pid
#include <linux/limits.h>
#else
#include <stdint.h>
#include <sys/types.h>
#include <limits.h>
#endif /* __KERNEL__ */

#define AUFS_VERSION	"6.0"

/* todo? move this to linux-2.6.19/include/magic.h */
#define AUFS_SUPER_MAGIC	('a' << 24 | 'u' << 16 | 'f' << 8 | 's')

/* ---------------------------------------------------------------------- */

#ifdef __KERNEL__
#ifdef CONFIG_AUFS_BRANCH_MAX_127
typedef int8_t aufs_bindex_t;
#define AUFS_BRANCH_MAX 127
#else
typedef int16_t aufs_bindex_t;
#ifdef CONFIG_AUFS_BRANCH_MAX_511
#define AUFS_BRANCH_MAX 511
#elif defined(CONFIG_AUFS_BRANCH_MAX_1023)
#define AUFS_BRANCH_MAX 1023
#elif defined(CONFIG_AUFS_BRANCH_MAX_32767)
#define AUFS_BRANCH_MAX 32767
#endif
#endif

#ifndef AUFS_BRANCH_MAX
#error unknown CONFIG_AUFS_BRANCH_MAX value
#endif
#endif /* __KERNEL__ */

#endif /* __AUFS_TYPE_H__ */
