/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 * Author :
 *  Dan Winship <danw@ximian.com>
 *
 *  Copyright 2000, Ximian, Inc. (www.ximian.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <libgnomevfs/gnome-vfs-mime.h>
#include <libgnomevfs/gnome-vfs-mime-sniff-buffer.h>
#include "mail.h"

static const char *identify_by_magic (CamelDataWrapper *data, MailDisplay *md);

/**
 * mail_identify_mime_part:
 * @part: a CamelMimePart
 * @md: the MailDisplay @part is being shown in
 *
 * Try to identify the MIME type of the data in @part (which presumably
 * doesn't have a useful Content-Type).
 *
 * Return value: the MIME type, which the caller must free, or %NULL
 * if it could not be identified.
 **/
char *
mail_identify_mime_part (CamelMimePart *part, MailDisplay *md)
{
	const char *filename, *name_type = NULL, *magic_type = NULL;
	CamelDataWrapper *data;

	filename = camel_mime_part_get_filename (part);
	if (filename) {
		/* GNOME-VFS will misidentify TNEF attachments as MPEG */
		if (!strcmp (filename, "winmail.dat"))
			return g_strdup ("application/vnd.ms-tnef");

		name_type = gnome_vfs_mime_type_from_name_or_default (filename, NULL);
	}

	data = camel_medium_get_content_object (CAMEL_MEDIUM (part));
	if (!camel_data_wrapper_is_offline (data))
		magic_type = identify_by_magic (data, md);

	/* If GNOME-VFS doesn't recognize the data by magic, but it
	 * contains English words, it will call it text/plain. If the
	 * filename-based check came up with something different, use
	 * that instead.
	 */
	if (magic_type && name_type && !strcmp (magic_type, "text/plain"))
		return g_strdup (name_type);

	/* If the MIME part data was online, and the magic check
	 * returned something, use that, since it's more reliable.
	 */
	if (magic_type)
		return g_strdup (magic_type);

	/* Otherwise try guessing based on the filename */
	if (name_type)
		return g_strdup (name_type);

	/* Another possibility to try is the x-mac-type / x-mac-creator
	 * parameter to Content-Type used by some Mac email clients. That
	 * would require a Mac type to mime type conversion table.
	 */

	/* If the data part is offline, then we didn't try magic
	 * before, so force it to be loaded so we can try again later.
	 * FIXME: In a perfect world, we would not load the content
	 * just to identify the MIME type.
	 */
	if (camel_data_wrapper_is_offline (data))
		mail_content_loaded (data, md);

	return NULL;
}

static const char *
identify_by_magic (CamelDataWrapper *data, MailDisplay *md)
{
	GnomeVFSMimeSniffBuffer *sniffer;
	CamelStream *memstream;
	const char *type;
	GByteArray *ba;

	ba = g_byte_array_new ();
	memstream = camel_stream_mem_new_with_byte_array (ba);
	camel_data_wrapper_write_to_stream (data, memstream);
	if (ba->len) {
		sniffer = gnome_vfs_mime_sniff_buffer_new_from_memory (ba->data, ba->len);
		type = gnome_vfs_get_mime_type_for_buffer (sniffer);
		gnome_vfs_mime_sniff_buffer_free (sniffer);
	} else
		type = NULL;
	camel_object_unref (CAMEL_OBJECT (memstream));

	return type;
}
