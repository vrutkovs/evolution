/*
 * e-mail-formatter-extension.c
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

#include "e-mail-formatter-extension.h"

G_DEFINE_INTERFACE (
	EMailFormatterExtension,
	e_mail_formatter_extension,
	E_TYPE_MAIL_EXTENSION)

/**
 * EMailFormatterExtension:
 *
 * The #EMailFormatterExtension is an abstract interface for all extensions for
 * #EmailFormatter.
 */

static void
e_mail_formatter_extension_default_init (EMailFormatterExtensionInterface *iface)
{

}

/**
 * e_mail_formatter_extension_format
 * @extension: an #EMailFormatterExtension
 * @formatter: an #EMailFormatter
 * @context: an #EMailFormatterContext
 * @part: a #EMailPart to be formatter
 * @stream: a #CamelStream to which the output should be written
 * @cancellable: (allow-none) a #GCancellable
 *
 * A virtual function reimplemented in all mail formatter extensions. The function
 * formats @part, generated HTML (or other format that can be displayed to user)
 * and writes it to the @stream.
 *
 * When the function is unable to format the @part (either because it's broken
 * or because it is a different mimetype then the extension is specialized for), the
 * function will return @FALSE indicating the #EMailFormatter, that it should pick
 * another extension.
 *
 * Implementation of this function must be thread-safe.
 *
 * Return value: Returns @TRUE when the @part was successfully formatted and
 * data were written to the @stream, @FALSE otherwise.
 */
gboolean
e_mail_formatter_extension_format (EMailFormatterExtension *extension,
                                   EMailFormatter *formatter,
                                   EMailFormatterContext *context,
                                   EMailPart *part,
                                   CamelStream *stream,
                                   GCancellable *cancellable)
{
	EMailFormatterExtensionInterface *interface;

	g_return_val_if_fail (E_IS_MAIL_FORMATTER_EXTENSION (extension), FALSE);
	g_return_val_if_fail (E_IS_MAIL_FORMATTER (formatter), FALSE);
	g_return_val_if_fail (context != NULL, FALSE);
	g_return_val_if_fail (part != NULL, FALSE);
	g_return_val_if_fail (CAMEL_IS_STREAM (stream), FALSE);

	interface = E_MAIL_FORMATTER_EXTENSION_GET_INTERFACE (extension);
	g_return_val_if_fail (interface->format != NULL, FALSE);

	return interface->format (extension, formatter, context, part, stream, cancellable);
}

/**
 * e_mail_formatter_extension_has_widget:
 * @extension: an #EMailFormatterExtension
 *
 * Returns whether the extension can provide a GtkWidget.
 *
 * Return value: Returns %TRUE when @extension reimplements get_widget(), %FALSE
 * otherwise.
 */
gboolean
e_mail_formatter_extension_has_widget (EMailFormatterExtension *extension)
{
	EMailFormatterExtensionInterface *interface;

	g_return_val_if_fail (E_IS_MAIL_FORMATTER_EXTENSION (extension), FALSE);

	interface = E_MAIL_FORMATTER_EXTENSION_GET_INTERFACE (extension);

	return (interface->get_widget != NULL);
}

/**
 * e_mail_formatter_extension_get_widget:
 * @extension: an #EMailFormatterExtension
 * @part: an #EMailPart
 * @params: a #GHashTable
 *
 * A virtual function reimplemented in some mail formatter extensions. The function
 * should construct a #GtkWidget for given @part. The @params hash table can contain
 * additional parameters listed in the &lt;object&gt; HTML element that has requested
 * the widget.
 *
 * When @bind_dom_func is not %NULL, the callee will set a callback function
 * which should be called when the webpage is completely rendered to setup
 * bindings between DOM events and the widget.
 *
 * Return value: Returns a #GtkWidget or %NULL, when error occurs or given @extension
 * does not reimplement this method.
 */
GtkWidget *
e_mail_formatter_extension_get_widget (EMailFormatterExtension *extension,
                                       EMailPartList *context,
                                       EMailPart *part,
                                       GHashTable *params)
{
	EMailFormatterExtensionInterface *interface;
	GtkWidget *widget;

	g_return_val_if_fail (E_IS_MAIL_FORMATTER_EXTENSION (extension), NULL);
	g_return_val_if_fail (part != NULL, NULL);
	g_return_val_if_fail (params != NULL, NULL);

	interface = E_MAIL_FORMATTER_EXTENSION_GET_INTERFACE (extension);

	widget = NULL;
	if (interface->get_widget) {
		widget = interface->get_widget (
				extension, context, part, params);
	}

	return widget;
}

/**
 * e_mail_formatter_extension_get_display_name:
 * @extension: an #EMailFormatterExtension
 *
 * A virtual function reimplemented in all formatter extensions. It returns a
 * short name of the extension that can be displayed in user interface.
 *
 * Return value: A (localized) string with name of the extension
 */
const gchar *
e_mail_formatter_extension_get_display_name (EMailFormatterExtension *extension)
{
	EMailFormatterExtensionInterface *interface;

	g_return_val_if_fail (E_IS_MAIL_FORMATTER_EXTENSION (extension), NULL);

	interface = E_MAIL_FORMATTER_EXTENSION_GET_INTERFACE (extension);
	g_return_val_if_fail (interface->get_display_name != NULL, NULL);

	return interface->get_display_name (extension);
}

/**
 * e_mail_formatter_extension_get_description:
 * @extension: an #EMailFormatterExtension
 *
 * A virtual function reimplemented in all formatter extensions. It returns a
 * longer description of capabilities of the extension.
 *
 * Return value: A (localized) string with description of the extension.
 */
const gchar *
e_mail_formatter_extension_get_description (EMailFormatterExtension *extension)
{
	EMailFormatterExtensionInterface *interface;

	g_return_val_if_fail (E_IS_MAIL_FORMATTER_EXTENSION (extension), NULL);

	interface = E_MAIL_FORMATTER_EXTENSION_GET_INTERFACE (extension);
	g_return_val_if_fail (interface->get_description != NULL, NULL);

	return interface->get_description (extension);
}