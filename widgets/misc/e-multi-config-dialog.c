/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* e-multi-config-dialog.c
 *
 * Copyright (C) 1999-2008 Novell, Inc. (www.novell.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "e-multi-config-dialog.h"

#include <libgnome/gnome-help.h>

#define SWITCH_PAGE_INTERVAL 250

#define E_MULTI_CONFIG_DIALOG_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE \
	((obj), E_TYPE_MULTI_CONFIG_DIALOG, EMultiConfigDialogPrivate))

struct _EMultiConfigDialogPrivate {
	GtkWidget *icon_view;
	GtkWidget *notebook;
	guint timeout_id;
};

enum {
	COLUMN_TEXT,	/* G_TYPE_STRING */
	COLUMN_PIXBUF	/* GDK_TYPE_PIXBUF */
};

static gpointer parent_class;

static GtkWidget *
create_page_container (GtkWidget *widget)
{
	GtkWidget *vbox;

	vbox = gtk_vbox_new (FALSE, 0);

	gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);

	gtk_widget_show (widget);
	gtk_widget_show (vbox);

	return vbox;
}

static GdkPixbuf *
multi_config_dialog_load_pixbuf (const gchar *icon_name)
{
	GtkIconTheme *icon_theme;
	GtkIconInfo *icon_info;
	GdkPixbuf *pixbuf;
	const gchar *filename;
	gint size;
	GError *error = NULL;

	icon_theme = gtk_icon_theme_get_default ();

	if (!gtk_icon_size_lookup (GTK_ICON_SIZE_DIALOG, &size, 0))
		return NULL;

	icon_info = gtk_icon_theme_lookup_icon (
		icon_theme, icon_name, size, 0);

	if (icon_info == NULL)
		return NULL;

	filename = gtk_icon_info_get_filename (icon_info);

	pixbuf = gdk_pixbuf_new_from_file (filename, &error);

	gtk_icon_info_free (icon_info);

	if (error != NULL) {
		g_warning ("%s", error->message);
		g_error_free (error);
	}

	return pixbuf;
}

static gboolean
multi_config_dialog_timeout_cb (EMultiConfigDialog *dialog)
{
	GtkIconView *icon_view;
	GtkNotebook *notebook;
	GList *list;

	icon_view = GTK_ICON_VIEW (dialog->priv->icon_view);
	notebook = GTK_NOTEBOOK (dialog->priv->notebook);

	list = gtk_icon_view_get_selected_items (icon_view);

	if (list != NULL) {
		GtkTreePath *path = list->data;
		gint page;

		page = gtk_tree_path_get_indices (path)[0];
		gtk_notebook_set_current_page (notebook, page);
	}

	g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
	g_list_free (list);

	dialog->priv->timeout_id = 0;
	gtk_widget_grab_focus (GTK_WIDGET (icon_view));

	return FALSE;
}

static void
multi_config_dialog_selection_changed_cb (EMultiConfigDialog *dialog)
{
	if (dialog->priv->timeout_id == 0)
		dialog->priv->timeout_id = g_timeout_add (
			SWITCH_PAGE_INTERVAL, (GSourceFunc)
			multi_config_dialog_timeout_cb, dialog);
}

static void
multi_config_dialog_dispose (GObject *object)
{
	EMultiConfigDialogPrivate *priv;

	priv = E_MULTI_CONFIG_DIALOG_GET_PRIVATE (object);

	if (priv->icon_view != NULL) {
		g_object_unref (priv->icon_view);
		priv->icon_view = NULL;
	}

	if (priv->notebook != NULL) {
		g_object_unref (priv->notebook);
		priv->notebook = NULL;
	}

	/* Chain up to parent's dispose() method. */
	G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
multi_config_dialog_finalize (GObject *object)
{
	EMultiConfigDialogPrivate *priv;

	priv = E_MULTI_CONFIG_DIALOG_GET_PRIVATE (object);

	if (priv->timeout_id != 0)
		g_source_remove (priv->timeout_id);

	/* Chain up to parent's finalize() method. */
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
multi_config_dialog_map (GtkWidget *widget)
{
	GtkDialog *dialog;

	/* Chain up to parent's map() method. */
	GTK_WIDGET_CLASS (parent_class)->map (widget);

	/* Override those stubborn style properties. */
	dialog = GTK_DIALOG (widget);
	gtk_box_set_spacing (GTK_BOX (dialog->vbox), 12);
	gtk_container_set_border_width (GTK_CONTAINER (widget), 12);
	gtk_container_set_border_width (GTK_CONTAINER (dialog->vbox), 0);
	gtk_container_set_border_width (GTK_CONTAINER (dialog->action_area), 0);
}

static void
multi_config_dialog_response (GtkDialog *dialog,
		              gint response_id)
{
	GError *error = NULL;

	switch (response_id) {
		case GTK_RESPONSE_HELP:
			gnome_help_display (
				"evolution.xml", "config-prefs", &error);
			if (error != NULL) {
				g_warning ("%s", error->message);
				g_error_free (error);
			}
			break;

		case GTK_RESPONSE_CLOSE:
		default:
			gtk_widget_destroy (GTK_WIDGET (dialog));
			break;
	}
}

static void
multi_config_dialog_class_init (EMultiConfigDialogClass *class)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;
	GtkDialogClass *dialog_class;

	parent_class = g_type_class_peek_parent (class);
	g_type_class_add_private (class, sizeof (EMultiConfigDialogPrivate));

	object_class = G_OBJECT_CLASS (class);
	object_class->dispose = multi_config_dialog_dispose;
	object_class->finalize = multi_config_dialog_finalize;

	widget_class = GTK_WIDGET_CLASS (class);
	widget_class->map = multi_config_dialog_map;

	dialog_class = GTK_DIALOG_CLASS (class);
	dialog_class->response = multi_config_dialog_response;
}

static void
multi_config_dialog_init (EMultiConfigDialog *dialog)
{
	GtkListStore *store;
	GtkWidget *container;
	GtkWidget *hbox;
	GtkWidget *widget;

	dialog->priv = E_MULTI_CONFIG_DIALOG_GET_PRIVATE (dialog);

	gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
	gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);

	/* XXX Remove this once we kill Bonobo. */
	gtk_widget_realize (GTK_WIDGET (dialog));

	container = GTK_DIALOG (dialog)->vbox;

	hbox = gtk_hbox_new (FALSE, 12);
	gtk_container_add (GTK_CONTAINER (container), hbox);
	gtk_widget_show (hbox);

	widget = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (
		GTK_SCROLLED_WINDOW (widget),
		GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (
		GTK_SCROLLED_WINDOW (widget), GTK_SHADOW_IN);
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, TRUE, 0);
	gtk_widget_show (widget);

	container = widget;

	store = gtk_list_store_new (2, G_TYPE_STRING, GDK_TYPE_PIXBUF);
	widget = gtk_icon_view_new_with_model (GTK_TREE_MODEL (store));
	gtk_icon_view_set_columns (GTK_ICON_VIEW (widget), 1);
	gtk_icon_view_set_text_column (GTK_ICON_VIEW (widget), COLUMN_TEXT);
	gtk_icon_view_set_pixbuf_column (GTK_ICON_VIEW (widget), COLUMN_PIXBUF);
	g_signal_connect_swapped (
		widget, "selection-changed",
		G_CALLBACK (multi_config_dialog_selection_changed_cb), dialog);
	gtk_container_add (GTK_CONTAINER (container), widget);
	dialog->priv->icon_view = g_object_ref (widget);
	gtk_widget_show (widget);
	g_object_unref (store);

	widget = gtk_notebook_new ();
	gtk_notebook_set_show_tabs (GTK_NOTEBOOK (widget), FALSE);
	gtk_notebook_set_show_border (GTK_NOTEBOOK (widget), FALSE);
	gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);
	dialog->priv->notebook = g_object_ref (widget);
	gtk_widget_show (widget);

	gtk_dialog_add_buttons (
		GTK_DIALOG (dialog),
		GTK_STOCK_HELP, GTK_RESPONSE_HELP,
		GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
		NULL);

	gtk_dialog_set_default_response (
		GTK_DIALOG (dialog), GTK_RESPONSE_CLOSE);
}

GType
e_multi_config_dialog_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY (type == 0)) {
		const GTypeInfo type_info = {
			sizeof (EMultiConfigDialogClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) multi_config_dialog_class_init,
			(GClassFinalizeFunc) NULL,
			NULL,  /* class_data */
			sizeof (EMultiConfigDialog),
			0,     /* n_preallocs */
			(GInstanceInitFunc) multi_config_dialog_init,
			NULL   /* value_table */
		};

		type = g_type_register_static (
			GTK_TYPE_DIALOG, "EMultiConfigDialog", &type_info, 0);
	}

	return type;
}

GtkWidget *
e_multi_config_dialog_new (void)
{
	return g_object_new (e_multi_config_dialog_get_type (), NULL);
}

void
e_multi_config_dialog_add_page (EMultiConfigDialog *dialog,
				const gchar *caption,
				const gchar *icon_name,
				EConfigPage *page_widget)
{
	GtkIconView *icon_view;
	GtkNotebook *notebook;
	GtkTreeModel *model;
	GdkPixbuf *pixbuf;
	GtkTreeIter iter;

	g_return_if_fail (E_IS_MULTI_CONFIG_DIALOG (dialog));
	g_return_if_fail (caption != NULL);
	g_return_if_fail (icon_name != NULL);
	g_return_if_fail (E_IS_CONFIG_PAGE (page_widget));

	icon_view = GTK_ICON_VIEW (dialog->priv->icon_view);
	notebook = GTK_NOTEBOOK (dialog->priv->notebook);

	model = gtk_icon_view_get_model (icon_view);
	pixbuf = multi_config_dialog_load_pixbuf (icon_name);

	gtk_list_store_append (GTK_LIST_STORE (model), &iter);

	gtk_list_store_set (
		GTK_LIST_STORE (model), &iter,
		COLUMN_TEXT, caption, COLUMN_PIXBUF, pixbuf, -1);

	if (gtk_tree_model_iter_n_children (model, NULL) == 1) {
		GtkTreePath *path;

		path = gtk_tree_path_new_first ();
		gtk_icon_view_select_path (icon_view, path);
		gtk_tree_path_free (path);
	}

	gtk_notebook_append_page (
		notebook, create_page_container (
		GTK_WIDGET (page_widget)), NULL);
}

void
e_multi_config_dialog_show_page (EMultiConfigDialog *dialog,
                                 gint page)
{
	GtkIconView *icon_view;
	GtkNotebook *notebook;
	GtkTreePath *path;

	g_return_if_fail (E_IS_MULTI_CONFIG_DIALOG (dialog));

	icon_view = GTK_ICON_VIEW (dialog->priv->icon_view);
	notebook = GTK_NOTEBOOK (dialog->priv->notebook);

	path = gtk_tree_path_new_from_indices (page, -1);
	gtk_icon_view_select_path (icon_view, path);
	gtk_icon_view_scroll_to_path (icon_view, path, FALSE, 0.0, 0.0);
	gtk_tree_path_free (path);

	gtk_notebook_set_current_page (notebook, page);
}
