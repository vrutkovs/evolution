/*
 * e-mail-engine-enums.h
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the program; if not, see <http://www.gnu.org/licenses/>
 *
 */

#ifndef E_MAIL_ENGINE_ENUMS_H
#define E_MAIL_ENGINE_ENUMS_H

#include <glib.h>

G_BEGIN_DECLS

/* XXX E_MAIL_FOLDER_TEMPLATES is a prime example of why templates
 *     should be a core feature: the mailer now has to know about
 *     this specific plugin, which defeats the purpose of plugins. */
typedef enum {
	E_MAIL_LOCAL_FOLDER_INBOX,
	E_MAIL_LOCAL_FOLDER_DRAFTS,
	E_MAIL_LOCAL_FOLDER_OUTBOX,
	E_MAIL_LOCAL_FOLDER_SENT,
	E_MAIL_LOCAL_FOLDER_TEMPLATES,
	E_MAIL_LOCAL_FOLDER_LOCAL_INBOX,
	E_MAIL_NUM_LOCAL_FOLDERS
} EMailLocalFolder;

G_END_DECLS

#endif /* E_MAIL_ENGINE_ENUMS_H */
