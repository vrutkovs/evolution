/*
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

#ifndef EAB_CONTACT_FORMATTER_H
#define EAB_CONTACT_FORMATTER_H

#include <libebook/e-contact.h>
#include <addressbook/gui/widgets/eab-contact-display.h>

#include <camel/camel.h>

/* Standard GObject macros */
#define EAB_TYPE_CONTACT_FORMATTER \
(eab_contact_formatter_get_type ())
#define EAB_CONTACT_FORMATTER(obj) \
(G_TYPE_CHECK_INSTANCE_CAST \
((obj), EAB_TYPE_CONTACT_FORMATTER, EABContactFormatter))
#define EAB_CONTACT_FORMATTER_CLASS(cls) \
(G_TYPE_CHECK_CLASS_CAST \
((cls), EAB_TYPE_CONTACT_FORMATTER, EABContactFormatterClass))
#define EAB_IS_CONTACT_FORMATTER(obj) \
(G_TYPE_CHECK_INSTANCE_TYPE \
((obj), EAB_TYPE_CONTACT_FORMATTER))
#define EAB_IS_CONTACT_FORMATTER_CLASS(cls) \
(G_TYPE_CHECK_CLASS_TYPE \
((cls), EAB_TYPE_CONTACT_FORMATTER))
#define EAB_CONTACT_FORMATTER_GET_CLASS(obj) \
(G_TYPE_ISNTANCE_GET_CLASS \
((obj), EAB_TYPE_CONTACT_FORMATTER, EABContactFormatterClass))

G_BEGIN_DECLS

typedef struct _EABContactFormatter EABContactFormatter;
typedef struct _EABContactFormatterClass EABContactFormatterClass;
typedef struct _EABContactFormatterPrivate EABContactFormatterPrivate;

struct _EABContactFormatter {
        GObject parent;
        EABContactFormatterPrivate *priv;
};

struct _EABContactFormatterClass {
        GObjectClass parent_class;
};

GType           eab_contact_formatter_get_type  ();

void            eab_contact_formatter_set_render_maps
                                                (EABContactFormatter *formatter,
                                                 gboolean render_maps);
gboolean        eab_contact_formatter_get_render_maps
                                                (EABContactFormatter *formatter);

void            eab_contact_formatter_set_display_mode
                                                (EABContactFormatter *formatter,
                                                 EABContactDisplayMode mode);
EABContactDisplayMode
                eab_contact_formatter_get_display_mode
                                                (EABContactFormatter *formatter);

void            eab_contact_formatter_format_contact_sync
                                                (EABContactFormatter *formatter,
                                                 EContact *contact,
						 CamelStream *stream,
                                                 GCancellable *cancellable);

void            eab_contact_formatter_format_contact_async
                                                (EABContactFormatter *formatter,
                                                 EContact *contact,
                                                 GCancellable *cancellable,
                                                 GAsyncReadyCallback callback,
                                                 gpointer user_data);

G_END_DECLS

#endif