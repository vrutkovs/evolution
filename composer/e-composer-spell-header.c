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
 * Copyright (C) 1999-2008 Novell, Inc. (www.novell.com)
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <misc/e-spell-entry.h>

#include "e-composer-spell-header.h"

G_DEFINE_TYPE (
	EComposerSpellHeader,
	e_composer_spell_header,
	E_TYPE_COMPOSER_TEXT_HEADER)

static void
e_composer_spell_header_class_init (EComposerSpellHeaderClass *class)
{
	EComposerTextHeaderClass *composer_text_header_class;

	composer_text_header_class = E_COMPOSER_TEXT_HEADER_CLASS (class);
	composer_text_header_class->entry_type = E_TYPE_SPELL_ENTRY;
}

static void
e_composer_spell_header_init (EComposerSpellHeader *header)
{
}

EComposerHeader *
e_composer_spell_header_new_label (const gchar *label)
{
	return g_object_new (
		E_TYPE_COMPOSER_SPELL_HEADER,
		"label", label, "button", FALSE,
		NULL);
}

EComposerHeader *
e_composer_spell_header_new_button (const gchar *label)
{
	return g_object_new (
		E_TYPE_COMPOSER_SPELL_HEADER,
		"label", label, "button", TRUE,
		NULL);
}

void
e_composer_spell_header_set_languages (EComposerSpellHeader *spell_header,
                                       GList *languages)
{
	ESpellEntry *spell_entry;

	g_return_if_fail (spell_header != NULL);

	spell_entry = E_SPELL_ENTRY (E_COMPOSER_HEADER (spell_header)->input_widget);
	g_return_if_fail (spell_entry != NULL);

	e_spell_entry_set_languages (spell_entry, languages);
}