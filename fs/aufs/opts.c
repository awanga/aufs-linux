// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2005-2022 Junjiro R. Okajima
 */

/*
 * mount options/flags
 */

#include <linux/namei.h>
#include <linux/types.h> /* a distribution requires */
#include <linux/parser.h>
#include "aufs.h"

/* ---------------------------------------------------------------------- */

static const char *au_optstr(int *val, match_table_t tbl)
{
	struct match_token *p;
	int v;

	v = *val;
	if (!v)
		goto out;
	p = tbl;
	while (p->pattern) {
		if (p->token
		    && (v & p->token) == p->token) {
			*val &= ~p->token;
			return p->pattern;
		}
		p++;
	}

out:
	return NULL;
}

/* ---------------------------------------------------------------------- */

static match_table_t brperm = {
	{AuBrPerm_RO, AUFS_BRPERM_RO},
	/* add more later */
	{0, NULL}
};

static match_table_t brattr = {
	/* add more later */
	{0, NULL}
};

static int br_attr_val(char *str, match_table_t table, substring_t args[])
{
	int attr, v;
	char *p;

	attr = 0;
	do {
		p = strchr(str, '+');
		if (p)
			*p = 0;
		v = match_token(str, table, args);
		if (v)
			attr |= v;
		else {
			if (p)
				*p = '+';
			pr_warn("ignored branch attribute %s\n", str);
			break;
		}
		if (p)
			str = p + 1;
	} while (p);

	return attr;
}

static int au_do_optstr_br_attr(au_br_perm_str_t *str, int perm)
{
	int sz;
	const char *p;
	char *q;

	q = str->a;
	*q = 0;
	p = au_optstr(&perm, brattr);
	if (p) {
		sz = strlen(p);
		memcpy(q, p, sz + 1);
		q += sz;
	} else
		goto out;

	do {
		p = au_optstr(&perm, brattr);
		if (p) {
			*q++ = '+';
			sz = strlen(p);
			memcpy(q, p, sz + 1);
			q += sz;
		}
	} while (p);

out:
	return q - str->a;
}

int au_br_perm_val(char *perm)
{
	int val;
	char *p;
	substring_t args[MAX_OPT_ARGS];

	p = strchr(perm, '+');
	if (p)
		*p = 0;
	val = match_token(perm, brperm, args);
	if (!val) {
		if (p)
			*p = '+';
		pr_warn("ignored branch permission %s\n", perm);
		val = AuBrPerm_RO;
		goto out;
	}
	if (!p)
		goto out;

	val |= br_attr_val(p + 1, brattr, args);

out:
	return val;
}

void au_optstr_br_perm(au_br_perm_str_t *str, int perm)
{
	au_br_perm_str_t attr;
	const char *p;
	char *q;
	int sz;

	q = str->a;
	p = au_optstr(&perm, brperm);
	AuDebugOn(!p || !*p);
	sz = strlen(p);
	memcpy(q, p, sz + 1);
	q += sz;

	sz = au_do_optstr_br_attr(&attr, perm);
	if (sz) {
		*q++ = '+';
		memcpy(q, attr.a, sz + 1);
	}

	AuDebugOn(strlen(str->a) >= sizeof(str->a));
}

/* ---------------------------------------------------------------------- */

int au_opt_add(struct au_opt *opt, char *opt_str, unsigned long sb_flags,
	       aufs_bindex_t bindex)
{
	int err;
	struct au_opt_add *add = &opt->add;
	char *p;

	add->bindex = bindex;
	add->perm = AuBrPerm_RO;
	add->pathname = opt_str;
	p = strchr(opt_str, '=');
	if (p) {
		*p++ = 0;
		if (*p)
			add->perm = au_br_perm_val(p);
	}

	err = vfsub_kern_path(add->pathname, AuOpt_LkupDirFlags, &add->path);
	if (!err) {
		if (!p)
			add->perm = AuBrPerm_RO;
			/* re-commit later */
		opt->type = Opt_add;
		goto out;
	}
	pr_err("lookup failed %s (%d)\n", add->pathname, err);
	err = -EINVAL;

out:
	return err;
}

/*
 * returns tri-state.
 * plus: processed without an error
 * zero: unprocessed
 * minus: error
 */
static int au_opt_br(struct super_block *sb, struct au_opt *opt,
		     struct au_opts *opts)
{
	int err;

	err = 0;
	switch (opt->type) {
	case Opt_add:
		err = au_br_add(sb, &opt->add);
		if (!err) {
			err = 1;
			/* au_fset_opts(opts->flags, REFRESH); re-commit later */
		}
		break;
	}

	return err;
}

int au_opts_mount(struct super_block *sb, struct au_opts *opts)
{
	int err;
	unsigned int tmp;
	aufs_bindex_t bbot;
	struct au_opt *opt;
	struct au_sbinfo *sbinfo;

	SiMustWriteLock(sb);

	err = 0;
	opt = opts->opt;
	while (err >= 0 && opt->type != Opt_tail)
		/* re-commit later */
		/* err = au_opt_simple(sb, opt++, opts); */
		err = 0;
	if (err > 0)
		err = 0;
	else if (unlikely(err < 0))
		goto out;

	sbinfo = au_sbi(sb);
	tmp = sbinfo->si_mntflags;
	opt = opts->opt;
	while (err >= 0 && opt->type != Opt_tail)
		err = au_opt_br(sb, opt++, opts);
	if (err > 0)
		err = 0;
	else if (unlikely(err < 0))
		goto out;

	bbot = au_sbbot(sb);
	if (unlikely(bbot < 0)) {
		err = -EINVAL;
		pr_err("no branches\n");
		goto out;
	}

out:
	return err;
}
