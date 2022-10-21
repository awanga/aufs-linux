/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2005-2022 Junjiro R. Okajima
 */

/*
 * mount options/flags
 */

#ifndef __AUFS_OPTS_H__
#define __AUFS_OPTS_H__

#ifdef __KERNEL__

#include <linux/fs_parser.h>
#include <linux/namei.h>
#include <linux/path.h>

enum {
	Opt_br,
	Opt_add,
	Opt_xino, Opt_noxino,
	Opt_trunc_xino, Opt_trunc_xino_v,
	Opt_trunc_xino_path, Opt_itrunc_xino,
	Opt_trunc_xib,
	Opt_plink, Opt_list_plink,
	Opt_tail, Opt_ignore, Opt_ignore_silent, Opt_err
};

/* ---------------------------------------------------------------------- */

/* mount flags */
#define AuOpt_XINO		1		/* external inode number bitmap
						   and translation table */
#define AuOpt_TRUNC_XINO	(1 << 1)	/* truncate xino files */
#define AuOpt_PLINK		(1 << 6)	/* pseudo-link */

#define AuOpt_Def	(AuOpt_XINO \
			 | AuOpt_PLINK)

#define AuOpt_LkupDirFlags	(LOOKUP_FOLLOW | LOOKUP_DIRECTORY)

#define au_opt_test(flags, name)	(flags & AuOpt_##name)
#define au_opt_set(flags, name) do { \
	((flags) |= AuOpt_##name); \
} while (0)
#define au_opt_clr(flags, name) do { \
	((flags) &= ~AuOpt_##name); \
} while (0)

static inline unsigned int au_opts_plink(unsigned int mntflags)
{
#ifdef CONFIG_PROC_FS
	return mntflags;
#else
	return mntflags & ~AuOpt_PLINK;
#endif
}

/* ---------------------------------------------------------------------- */

struct file;

struct au_opt_add {
	aufs_bindex_t	bindex;
	char		*pathname;
	int		perm;
	struct path	path;
};

struct au_opt_xino {
	char		*path;
	struct file	*file;
};

struct au_opt_xino_itrunc {
	aufs_bindex_t	bindex;
};

struct au_opt {
	int type;
	union {
		struct au_opt_xino	xino;
		struct au_opt_xino_itrunc xino_itrunc;
		struct au_opt_add	add;
		bool			tf; /* generic flag, true or false */
		/* add more later */
	};
};

/* opts flags */
#define AuOpts_TRUNC_XIB	(1 << 2)
#define au_ftest_opts(flags, name)	((flags) & AuOpts_##name)
#define au_fset_opts(flags, name) \
	do { (flags) |= AuOpts_##name; } while (0)
#define au_fclr_opts(flags, name) \
	do { (flags) &= ~AuOpts_##name; } while (0)

struct au_opts {
	struct au_opt	*opt;
	int		max_opt;

	unsigned int	flags;
	unsigned long	sb_flags;
};

/* ---------------------------------------------------------------------- */

/* opts.c */
int au_br_perm_val(char *perm);
void au_optstr_br_perm(au_br_perm_str_t *str, int perm);

int au_opt_add(struct au_opt *opt, char *opt_str, unsigned long sb_flags,
	       aufs_bindex_t bindex);
struct super_block;
int au_opts_mount(struct super_block *sb, struct au_opts *opts);

/* fsctx.c */
int aufs_fsctx_init(struct fs_context *fc);
extern const struct fs_parameter_spec aufs_fsctx_paramspec[];

#endif /* __KERNEL__ */
#endif /* __AUFS_OPTS_H__ */
