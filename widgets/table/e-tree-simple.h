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

#ifndef _E_TREE_SIMPLE_H_
#define _E_TREE_SIMPLE_H_

#include <table/e-tree-model.h>
#include <table/e-table-simple.h>

G_BEGIN_DECLS

#define E_TREE_SIMPLE_TYPE        (e_tree_simple_get_type ())
#define E_TREE_SIMPLE(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), E_TREE_SIMPLE_TYPE, ETreeSimple))
#define E_TREE_SIMPLE_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), E_TREE_SIMPLE_TYPE, ETreeSimpleClass))
#define E_IS_TREE_SIMPLE(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), E_TREE_SIMPLE_TYPE))
#define E_IS_TREE_SIMPLE_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), E_TREE_SIMPLE_TYPE))
#define E_TREE_SIMPLE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), E_TREE_SIMPLE_TYPE, ETreeSimpleClass))

typedef GdkPixbuf* (*ETreeSimpleIconAtFn)     (ETreeModel *etree, ETreePath *path, gpointer model_data);
typedef gpointer       (*ETreeSimpleValueAtFn)    (ETreeModel *etree, ETreePath *path, gint col, gpointer model_data);
typedef void       (*ETreeSimpleSetValueAtFn) (ETreeModel *etree, ETreePath *path, gint col, gconstpointer val, gpointer model_data);
typedef gboolean   (*ETreeSimpleIsEditableFn) (ETreeModel *etree, ETreePath *path, gint col, gpointer model_data);

typedef struct {
	ETreeModel parent;

	/* Table methods */
	ETableSimpleColumnCountFn     col_count;
	ETableSimpleDuplicateValueFn  duplicate_value;
	ETableSimpleFreeValueFn       free_value;
	ETableSimpleInitializeValueFn initialize_value;
	ETableSimpleValueIsEmptyFn    value_is_empty;
	ETableSimpleValueToStringFn   value_to_string;

	/* Tree methods */
	ETreeSimpleIconAtFn icon_at;
	ETreeSimpleValueAtFn value_at;
	ETreeSimpleSetValueAtFn set_value_at;
	ETreeSimpleIsEditableFn is_editable;

	gpointer model_data;
} ETreeSimple;

typedef struct {
	ETreeModelClass parent_class;
} ETreeSimpleClass;

GType e_tree_simple_get_type (void);

ETreeModel *e_tree_simple_new  (ETableSimpleColumnCountFn     col_count,
				ETableSimpleDuplicateValueFn  duplicate_value,
				ETableSimpleFreeValueFn       free_value,
				ETableSimpleInitializeValueFn initialize_value,
				ETableSimpleValueIsEmptyFn    value_is_empty,
				ETableSimpleValueToStringFn   value_to_string,
				ETreeSimpleIconAtFn           icon_at,
				ETreeSimpleValueAtFn          value_at,
				ETreeSimpleSetValueAtFn       set_value_at,
				ETreeSimpleIsEditableFn       is_editable,
				gpointer                      model_data);

G_END_DECLS

#endif /* _E_TREE_SIMPLE_H_ */