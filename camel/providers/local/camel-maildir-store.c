/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2000 Ximian, Inc.
 *
 * Authors: Michael Zucchi <notzed@ximian.com>
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <dirent.h>

#include "camel-maildir-store.h"
#include "camel-maildir-folder.h"
#include "camel-exception.h"
#include "camel-url.h"

static CamelLocalStoreClass *parent_class = NULL;

/* Returns the class for a CamelMaildirStore */
#define CMAILDIRS_CLASS(so) CAMEL_MAILDIR_STORE_CLASS (CAMEL_OBJECT_GET_CLASS(so))
#define CF_CLASS(so) CAMEL_FOLDER_CLASS (CAMEL_OBJECT_GET_CLASS(so))
#define CMAILDIRF_CLASS(so) CAMEL_MAILDIR_FOLDER_CLASS (CAMEL_OBJECT_GET_CLASS(so))

static CamelFolder *get_folder(CamelStore * store, const char *folder_name, guint32 flags, CamelException * ex);
static CamelFolder *get_inbox (CamelStore *store, CamelException *ex);
static void delete_folder(CamelStore * store, const char *folder_name, CamelException * ex);

static CamelFolderInfo * get_folder_info (CamelStore *store, const char *top, guint32 flags, CamelException *ex);

static void camel_maildir_store_class_init(CamelObjectClass * camel_maildir_store_class)
{
	CamelStoreClass *camel_store_class = CAMEL_STORE_CLASS(camel_maildir_store_class);
	/*CamelServiceClass *camel_service_class = CAMEL_SERVICE_CLASS(camel_maildir_store_class);*/

	parent_class = (CamelLocalStoreClass *)camel_type_get_global_classfuncs(camel_local_store_get_type());

	/* virtual method overload, use defaults for most */
	camel_store_class->get_folder = get_folder;
	camel_store_class->get_inbox = get_inbox;
	camel_store_class->delete_folder = delete_folder;

	camel_store_class->get_folder_info = get_folder_info;
	camel_store_class->free_folder_info = camel_store_free_folder_info_full;
}

CamelType camel_maildir_store_get_type(void)
{
	static CamelType camel_maildir_store_type = CAMEL_INVALID_TYPE;

	if (camel_maildir_store_type == CAMEL_INVALID_TYPE) {
		camel_maildir_store_type = camel_type_register(CAMEL_LOCAL_STORE_TYPE, "CamelMaildirStore",
							  sizeof(CamelMaildirStore),
							  sizeof(CamelMaildirStoreClass),
							  (CamelObjectClassInitFunc) camel_maildir_store_class_init,
							  NULL,
							  NULL,
							  NULL);
	}

	return camel_maildir_store_type;
}

static CamelFolder *get_folder(CamelStore * store, const char *folder_name, guint32 flags, CamelException * ex)
{
	char *name, *tmp, *cur, *new;
	struct stat st;
	CamelFolder *folder = NULL;

	(void) ((CamelStoreClass *)parent_class)->get_folder(store, folder_name, flags, ex);
	if (camel_exception_is_set(ex))
		return NULL;

	name = g_strdup_printf("%s%s", CAMEL_SERVICE(store)->url->path, folder_name);
	tmp = g_strdup_printf("%s/tmp", name);
	cur = g_strdup_printf("%s/cur", name);
	new = g_strdup_printf("%s/new", name);

	if (stat(name, &st) == -1) {
		if (errno != ENOENT) {
			camel_exception_setv(ex, CAMEL_EXCEPTION_SYSTEM,
					     _("Could not open folder `%s':\n%s"),
					     folder_name, strerror(errno));
		} else if ((flags & CAMEL_STORE_FOLDER_CREATE) == 0) {
			camel_exception_setv(ex, CAMEL_EXCEPTION_STORE_NO_FOLDER,
					     _("Folder `%s' does not exist."), folder_name);
		} else {
			if (mkdir(name, 0700) != 0
			    || mkdir(tmp, 0700) != 0
			    || mkdir(cur, 0700) != 0
			    || mkdir(new, 0700) != 0) {
				camel_exception_setv(ex, CAMEL_EXCEPTION_SYSTEM,
						     _("Could not create folder `%s':\n%s"),
						     folder_name, strerror(errno));
				rmdir(tmp);
				rmdir(cur);
				rmdir(new);
				rmdir(name);
			} else {
				folder = camel_maildir_folder_new(store, folder_name, flags, ex);
			}
		}
	} else if (!S_ISDIR(st.st_mode)
		   || stat(tmp, &st) != 0 || !S_ISDIR(st.st_mode)
		   || stat(cur, &st) != 0 || !S_ISDIR(st.st_mode)
		   || stat(new, &st) != 0 || !S_ISDIR(st.st_mode)) {
		camel_exception_setv(ex, CAMEL_EXCEPTION_STORE_NO_FOLDER,
				     _("`%s' is not a maildir directory."), name);
	} else {
		folder = camel_maildir_folder_new(store, folder_name, flags, ex);
	}

	g_free(name);
	g_free(tmp);
	g_free(cur);
	g_free(new);

	return folder;
}

/* fixes bug #1138 */
static CamelFolder *
get_inbox (CamelStore *store, CamelException *ex)
{
	return get_folder (store, "", 0, ex);
}

static void delete_folder(CamelStore * store, const char *folder_name, CamelException * ex)
{
	char *name, *tmp, *cur, *new;
	struct stat st;

	name = g_strdup_printf("%s%s", CAMEL_SERVICE(store)->url->path, folder_name);

	tmp = g_strdup_printf("%s/tmp", name);
	cur = g_strdup_printf("%s/cur", name);
	new = g_strdup_printf("%s/new", name);

	if (stat(name, &st) == -1 || !S_ISDIR(st.st_mode)
	    || stat(tmp, &st) == -1 || !S_ISDIR(st.st_mode)
	    || stat(cur, &st) == -1 || !S_ISDIR(st.st_mode)
	    || stat(new, &st) == -1 || !S_ISDIR(st.st_mode)) {
		camel_exception_setv(ex, CAMEL_EXCEPTION_SYSTEM,
				     _("Could not delete folder `%s': %s"),
				     folder_name, errno?strerror(errno):_("not a maildir directory"));
	} else {
		int err = 0;

		/* remove subdirs first - will fail if not empty */
		if (rmdir(cur) == -1 || rmdir(new) == -1) {
			err = errno;
		} else {
			DIR *dir;
			struct dirent *d;

			/* for tmp (only), its contents is irrelevant */
			dir = opendir(tmp);
			if (dir) {
				while ( (d=readdir(dir)) ) {
					char *name = d->d_name, *file;

					if (!strcmp(name, ".") || !strcmp(name, ".."))
						continue;
					file = g_strdup_printf("%s/%s", tmp, name);
					unlink(file);
					g_free(file);
				}
				closedir(dir);
			}
			if (rmdir(tmp) == -1 || rmdir(name) == -1)
				err = errno;
		}

		if (err != 0) {
			/* easier just to mkdir all (and let them fail), than remember what we got to */
			mkdir(name, 0700);
			mkdir(cur, 0700);
			mkdir(new, 0700);
			mkdir(tmp, 0700);
			camel_exception_setv(ex, CAMEL_EXCEPTION_SYSTEM,
					     _("Could not delete folder `%s': %s"),
					     folder_name, strerror(err));
		} else {
			/* and remove metadata */
			((CamelStoreClass *)parent_class)->delete_folder(store, folder_name, ex);
		}
	}

	g_free(name);
	g_free(tmp);
	g_free(cur);
	g_free(new);
}

static CamelFolderInfo *camel_folder_info_new(const char *url, const char *full, const char *name, int unread)
{
	CamelFolderInfo *fi;

	fi = g_malloc0(sizeof(*fi));
	fi->url = g_strdup(url);
	fi->full_name = g_strdup(full);
	fi->name = g_strdup(name);
	fi->unread_message_count = unread;

	return fi;
}

static int scan_dir(char *root, const char *path, guint32 flags, CamelFolderInfo *parent, CamelFolderInfo **fip, CamelException *ex)
{
	DIR *dir;
	struct dirent *d;
	char *name, *uri, *tmp, *cur, *new;
	const char *base;
	CamelFolderInfo *fi = NULL;
	struct stat st;

	/* look for folders matching the right structure, recursively */
	name = g_strdup_printf("%s/%s", root, path);

	printf("checking dir '%s' part '%s' for maildir content\n", root, path);

	tmp = g_strdup_printf("%s/tmp", name);
	cur = g_strdup_printf("%s/cur", name);
	new = g_strdup_printf("%s/new", name);

	if (stat(tmp, &st) == 0 && S_ISDIR(st.st_mode)
	    && stat(cur, &st) == 0 && S_ISDIR(st.st_mode)
	    && stat(new, &st) == 0 && S_ISDIR(st.st_mode)) {
		uri = g_strdup_printf("maildir://%s#%s", root, path);
		base = strrchr(path, '/');
		if (base)
			base++;
		else
			base = path;
		fi = camel_folder_info_new(uri, path, base, -1);

		printf("found! uri = %s\n", fi->url);
		printf("  full_name = %s\n  name = '%s'\n", fi->full_name, fi->name);

		fi->parent = parent;
		fi->sibling = *fip;
		*fip = fi;
		g_free(uri);
	}

	g_free(tmp);
	g_free(cur);
	g_free(new);

	/* we only look further if we found one at this level */
	if (fi && ((flags & CAMEL_STORE_FOLDER_INFO_RECURSIVE) || parent == NULL)) {
		dir = opendir(name);
		if (dir == NULL) {
			camel_exception_setv(ex, CAMEL_EXCEPTION_SYSTEM,
					     _("Could not scan folder `%s': %s"),
					     root, strerror(errno));
			g_free(name);
			return -1;
		}

		while ( (d = readdir(dir)) ) {
			if (strcmp(d->d_name, "tmp") == 0
			    || strcmp(d->d_name, "cur") == 0
			    || strcmp(d->d_name, "new") == 0
			    || strcmp(d->d_name, ".") == 0
			    || strcmp(d->d_name, "..") == 0)
				continue;

			tmp = g_strdup_printf("%s/%s", name, d->d_name);
			if (stat(tmp, &st) == 0 && S_ISDIR(st.st_mode)) {
				new = g_strdup_printf("%s/%s", path, d->d_name);
				if (scan_dir(root, new, flags, fi, &fi->child, ex) == -1) {
					g_free(tmp);
					g_free(new);
					closedir(dir);
					return -1;
				}
				g_free(new);
			}
			g_free(tmp);
		}
		closedir(dir);
	}

	g_free(name);

	return 0;
}

static CamelFolderInfo *
get_folder_info (CamelStore *store, const char *top, guint32 flags, CamelException *ex)
{
	CamelFolderInfo *fi = NULL;
	CamelService *service = (CamelService *)store;

	if (scan_dir(service->url->path, top?top:".", flags, NULL, &fi, ex) == -1 && fi != NULL) {
		camel_store_free_folder_info_full(store, fi);
		fi = NULL;
	}

	return fi;
}
