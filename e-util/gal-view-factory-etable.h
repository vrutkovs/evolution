/*
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
 *
 * Authors:
 *		Chris Lahey <clahey@ximian.com>
 *
 * Copyright (C) 1999-2008 Novell, Inc. (www.novell.com)
 *
 */

#if !defined (__E_UTIL_H_INSIDE__) && !defined (LIBEUTIL_COMPILATION)
#error "Only <e-util/e-util.h> should be included directly."
#endif

#ifndef GAL_VIEW_FACTORY_ETABLE_H
#define GAL_VIEW_FACTORY_ETABLE_H

#include <gtk/gtk.h>
#include <e-util/gal-view-factory.h>
#include <e-util/e-table-specification.h>

/* Standard GObject macros */
#define GAL_TYPE_VIEW_FACTORY_ETABLE \
	(gal_view_factory_etable_get_type ())
#define GAL_VIEW_FACTORY_ETABLE(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), GAL_TYPE_VIEW_FACTORY_ETABLE, GalViewFactoryEtable))
#define GAL_VIEW_FACTORY_ETABLE_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_CAST \
	((cls), GAL_TYPE_VIEW_FACTORY_ETABLE, GalViewFactoryEtableClass))
#define GAL_IS_VIEW_FACTORY_ETABLE(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), GAL_TYPE_VIEW_FACTORY_ETABLE))
#define GAL_IS_VIEW_FACTORY_ETABLE_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_TYPE \
	((cls), GAL_TYPE_VIEW_FACTORY_ETABLE))
#define GAL_VIEW_FACTORY_ETABLE_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS \
	((obj), GAL_TYPE_VIEW_FACTORY_ETABLE, GalViewFactoryEtableClass))

G_BEGIN_DECLS

typedef struct _GalViewFactoryEtable GalViewFactoryEtable;
typedef struct _GalViewFactoryEtableClass GalViewFactoryEtableClass;
typedef struct _GalViewFactoryEtablePrivate GalViewFactoryEtablePrivate;

struct _GalViewFactoryEtable {
	GalViewFactory parent;
	GalViewFactoryEtablePrivate *priv;
};

struct _GalViewFactoryEtableClass {
	GalViewFactoryClass parent_class;
};

GType		gal_view_factory_etable_get_type (void);
GalViewFactory *gal_view_factory_etable_new	(ETableSpecification *specification);
ETableSpecification *
		gal_view_factory_etable_get_specification
						(GalViewFactoryEtable *factory);

G_END_DECLS

#endif /* GAL_VIEW_FACTORY_ETABLE_H */