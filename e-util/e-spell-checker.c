/*
 * e-spell-checker.c
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "e-spell-checker.h"
#include "e-spell-dictionary.h"

#include <webkit/webkitspellchecker.h>
#include <pango/pango.h>
#include <gtk/gtk.h>

static void e_spell_checker_init_webkit_checker (WebKitSpellCheckerInterface *iface);

G_DEFINE_TYPE_EXTENDED (
	ESpellChecker,
	e_spell_checker,
	G_TYPE_OBJECT,
	0,
	G_IMPLEMENT_INTERFACE (
		WEBKIT_TYPE_SPELL_CHECKER,
		e_spell_checker_init_webkit_checker))

struct _ESpellCheckerPrivate {
	GList *active;

	EnchantBroker *broker;
};

enum {
	PROP_ACTIVE_DICTIONARIES
};

/**
 * ESpellChecker:
 *
 * #ESpellChecker represents a spellchecker in Evolution. It can be used as a
 * provider for dictionaries. It also implements #WebKitSpellCheckerInterface,
 * so it can be set as a default spell-checker to WebKit editors
 */

static void
wksc_check_spelling (WebKitSpellChecker *webkit_checker,
		     const char         *word,
		     int                *misspelling_location,
		     int                *misspelling_length)
{
	ESpellChecker *checker = E_SPELL_CHECKER (webkit_checker);
	PangoLanguage *language;
	PangoLogAttr *attrs;
	GList *dicts;
	gint length, ii;

	dicts = checker->priv->active;
	if (!dicts)
		return;

	length = g_utf8_strlen (word, -1);

	language = pango_language_get_default ();
	attrs = g_new (PangoLogAttr, length + 1);

	pango_get_log_attrs (word, -1, -1, language, attrs, length + 1);

	for (ii = 0; ii < length + 1; ii++) {
		/* We go through each character until we find an is_word_start,
		 * then we get into an inner loop to find the is_word_end
		 * corresponding */
		if (attrs[ii].is_word_start) {
			int start = ii;
			int end = ii;
			int word_length;
			gchar *cstart;
			gint bytes;
			gchar *new_word;
			GList *iter;

			while (attrs[end].is_word_end < 1) {
				end++;
			}

			word_length = end - start;
			/* Set the iterator to be at the current word end, so we don't
			 * check characters twice. */
			ii = end;

			cstart = g_utf8_offset_to_pointer (word, start);
			bytes = g_utf8_offset_to_pointer (word, end) - cstart;
			new_word = g_new0 (gchar, bytes + 1);

			g_utf8_strncpy (new_word, cstart, word_length);

			for (iter = dicts; iter; iter = iter->next) {
				ESpellDictionary *dict = iter->data;

				if (e_spell_dictionary_check (dict, new_word, word_length)) {
					if (misspelling_location)
						*misspelling_location = start;
					if (misspelling_length)
						*misspelling_length = word_length;
				} else {
					/* Stop checking, this word is ok in at
					 * least one dict. */
					if (misspelling_location)
						*misspelling_location = -1;
					if (misspelling_length)
						*misspelling_length = 0;
					break;
				}
			}

			g_free (new_word);
		}
	}

	g_free (attrs);
}

static gchar **
wksc_get_guesses (WebKitSpellChecker *webkit_checker,
		  const gchar *word,
		  const gchar *context)
{
	ESpellChecker *checker = E_SPELL_CHECKER (webkit_checker);
	GList *dicts;
	char** guesses;
	gint ii;

	guesses = g_malloc0_n (sizeof (gchar *), 11);
	ii = 0;
	for (dicts = checker->priv->active; dicts && ii < 10; dicts = dicts->next) {
		ESpellDictionary *dict;
		GList *suggestions, *iter;
		gint suggestions_count;

		dict = dicts->data;
		suggestions = e_spell_dictionary_get_suggestions (dict, word, -1);

		suggestions_count = g_list_length (suggestions);
		if (suggestions_count > 0) {
			if (suggestions_count > 10) {
				suggestions_count = 10;
			}

			for (iter = suggestions; iter && ii < 10; iter = iter->next, ii++) {
				guesses[ii] = g_strdup (iter->data);
			}

			guesses[ii] = 0;

			e_spell_dictionary_free_suggestions (suggestions);
		}
	}

	return guesses;
}

static gchar *
wksc_get_autocorrect_suggestions (WebKitSpellChecker *webkit_checker,
				  const gchar *word)
{
	/* Not supported/needed */
	return NULL;
}

static void
spell_checker_learn_word (WebKitSpellChecker *webkit_checker,
			  const gchar *word)
{
	/* Carefully, this will add the word to all active dictionaries! */

	ESpellChecker *checker;
	GList *iter;

	checker = E_SPELL_CHECKER (webkit_checker);
	for (iter = checker->priv->active; iter; iter = iter->next) {
		ESpellDictionary *dict = iter->data;

		e_spell_dictionary_learn_word (dict, word, -1);
	}
}

static void
spell_checker_ignore_word (WebKitSpellChecker *webkit_checker,
			   const gchar *word)
{
	/* Carefully, this will add the word to all active dictionaries */

	ESpellChecker *checker;
	GList *iter;

	checker = E_SPELL_CHECKER (webkit_checker);
	for (iter = checker->priv->active; iter; iter = iter->next) {
		ESpellDictionary *dict = iter->data;

		e_spell_dictionary_ignore_word (dict, word, -1);
	}
}

static void
wksc_update_languages (WebKitSpellChecker *webkit_checker,
		       const char *languages)
{
	ESpellChecker *checker;
	GList *dictionaries = NULL;
	gchar **langs;
	gint ii;

	checker = E_SPELL_CHECKER (webkit_checker);
	if (languages) {
		langs = g_strsplit (languages, ",", -1);
		for (ii = 0; langs[ii] != NULL; ii++) {
			ESpellDictionary *dict;

			dict = e_spell_checker_lookup_dictionary (checker, langs[ii]);
			dictionaries = g_list_append (dictionaries, dict);
		}
		g_strfreev (langs);
	} else {
		const gchar *language;
		ESpellDictionary *dict;

		language = pango_language_to_string (gtk_get_default_language ());
		dict = e_spell_checker_lookup_dictionary (checker, language);
		if (dict) {
			dictionaries = g_list_append (dictionaries, dict);
		} else {
			dictionaries = e_spell_checker_list_available_dicts (checker);
		}
	}

	e_spell_checker_set_active_dictionaries (checker, dictionaries);
	g_list_free_full (dictionaries, g_object_unref);
}


static void
spell_checker_set_property (GObject *object,
			    guint property_id,
			    const GValue *value,
			    GParamSpec *pspec)
{
	switch (property_id) {
		case PROP_ACTIVE_DICTIONARIES:
			e_spell_checker_set_active_dictionaries (
				E_SPELL_CHECKER (object),
				g_value_get_pointer (value));
			return;
	}

	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
spell_checker_get_property (GObject *object,
			    guint property_id,
			    GValue *value,
			    GParamSpec *pspec)
{
	switch (property_id) {
		case PROP_ACTIVE_DICTIONARIES:
			g_value_set_pointer (
				value,
				e_spell_checker_get_active_dictionaries (
					E_SPELL_CHECKER (object)));
			return;
	}

	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
spell_checker_dispose (GObject *object)
{
	ESpellCheckerPrivate *priv = E_SPELL_CHECKER (object)->priv;

	g_list_free_full (priv->active, g_object_unref);
	priv->active = NULL;

	enchant_broker_free (priv->broker);
	priv->broker = NULL;

	/* Chain up to parent implementation */
	G_OBJECT_CLASS (e_spell_checker_parent_class)->dispose (object);
}

static void
e_spell_checker_class_init (ESpellCheckerClass *klass)
{
	GObjectClass *object_class;

	e_spell_checker_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (ESpellCheckerPrivate));

	object_class = G_OBJECT_CLASS (klass);
	object_class->set_property = spell_checker_set_property;
	object_class->get_property = spell_checker_get_property;
	object_class->dispose = spell_checker_dispose;

	g_object_class_install_property (
		object_class,
		PROP_ACTIVE_DICTIONARIES,
		g_param_spec_pointer (
			"active-dictionaries",
			NULL,
			"List of active dictionaries to use for spell-checking",
			G_PARAM_READWRITE));
}

static void
e_spell_checker_init_webkit_checker (WebKitSpellCheckerInterface *iface)
{
	iface->check_spelling_of_string = wksc_check_spelling;
	iface->get_autocorrect_suggestions_for_misspelled_word = wksc_get_autocorrect_suggestions;
	iface->get_guesses_for_word = wksc_get_guesses;
	iface->ignore_word = spell_checker_ignore_word;
	iface->learn_word = spell_checker_learn_word;
	iface->update_spell_checking_languages = wksc_update_languages;
}


static void
e_spell_checker_init (ESpellChecker *checker)
{
	checker->priv = G_TYPE_INSTANCE_GET_PRIVATE (
		checker, E_TYPE_SPELL_CHECKER, ESpellCheckerPrivate);
}

typedef struct  {
	ESpellChecker *checker;
	GList *dicts;
} ListAvailDictsData;

static void
list_enchant_dicts (const char * const lang_tag,
		    const char * const provider_name,
		    const char * const provider_desc,
		    const char * const provider_file,
		    void * user_data)
{
	ListAvailDictsData *data = user_data;
	EnchantDict *dict;

	dict = enchant_broker_request_dict (data->checker->priv->broker, lang_tag);
	if (dict) {
		ESpellDictionary *e_dict;

		e_dict = e_spell_dictionary_new (data->checker, dict);

		data->dicts = g_list_prepend (data->dicts, e_dict);
	}
}


/**
 * e_spell_checker_list_available_dicts:
 * @checker: An #ESpellChecker
 *
 * Returns list of all dictionaries available to the actual
 * spell-checking backend.
 *
 * Return value: a #GList of #ESpellDictionary. Free the list using g_list_free()
 * 		when not needed anymore.
 */
GList *
e_spell_checker_list_available_dicts (ESpellChecker *checker)
{
	ESpellChecker *e_checker;
	ListAvailDictsData data;

	e_checker = E_SPELL_CHECKER (checker);

	data.checker = e_checker;
	enchant_broker_list_dicts (
		e_checker->priv->broker, list_enchant_dicts, &data);

	return g_list_reverse (data.dicts);
}

/**
 * e_spell_checker_lookup_dictionary:
 * @checker: an #ESpellChecker
 * @language_code: (allow-none) language code for which to lookup the dictionary
 *
 * Tries to find an #ESpellDictionary for given @language_code. When @language_code
 * is #NULL, this function will return a default #ESpellDictionary.
 *
 * Return value: an #ESpellDictionary for @language_code
 */
ESpellDictionary *
e_spell_checker_lookup_dictionary (ESpellChecker *checker,
				   const gchar *language_code)
{
	ESpellChecker *e_checker;
	ESpellDictionary *e_dict;

	e_checker = E_SPELL_CHECKER (checker);

	e_dict = NULL;

	if (!language_code) {
		GList *dicts = e_spell_checker_list_available_dicts (checker);

		if (dicts) {
			e_dict = g_object_ref (dicts->data);
			g_list_free_full (dicts, g_object_unref);
		}
	} else {
		EnchantDict *dict;
		dict = enchant_broker_request_dict (
			e_checker->priv->broker, language_code);
		if (dict) {
			e_dict = e_spell_dictionary_new (checker, dict);
		}
	}

	return e_dict;
}

/**
 * e_spell_checker_get_active_dictionaries:
 * @checker: an #ESpellChecker
 *
 * Returns a list of #ESpellDictionary that are to be used for spell-checking.
 *
 * Return value: a #GList of #ESpellDictionary. Free the list using g_list_fre()
 * 		 when no longer needed.
 */
GList *
e_spell_checker_get_active_dictionaries (ESpellChecker *checker)
{
	g_return_val_if_fail (E_IS_SPELL_CHECKER (checker), NULL);

	return g_list_copy (checker->priv->active);
}

/**
 * e_spell_checker_set_active_dictionaries:
 * @checker: an #ESpellChecker
 * @active_dict: a #GList of #ESpellDictionary to use for spell-checking
 *
 * Set dictionaries to be actively used for spell-checking.
 */
void
e_spell_checker_set_active_dictionaries (ESpellChecker *checker,
					 GList *active_dicts)
{
	g_return_if_fail (E_IS_SPELL_CHECKER (checker));

	g_list_free_full (checker->priv->active, (GDestroyNotify) g_object_unref);

	checker->priv->active = g_list_copy (active_dicts);
	g_list_foreach (checker->priv->active, (GFunc) g_object_ref, NULL);
}

void
e_spell_checker_free_dict (ESpellChecker *checker,
			   EnchantDict *dict)
{
	g_return_if_fail (E_IS_SPELL_CHECKER (checker));
	g_return_if_fail (dict != NULL);

	enchant_broker_free_dict (checker->priv->broker, dict);
}

/**
 * e_spell_checker_ignore_word:
 * @checker: an #ESpellChecker
 * @word: word to ignore for the rest of session
 *
 * Calls #e_spell_dictionary_ignore_word() on all active dictionaries in
 * the @checker.
 */
void
e_spell_checker_ignore_word (ESpellChecker *checker,
			     const gchar *word)
{
	WebKitSpellCheckerInterface *webkit_checker_iface;

	g_return_if_fail (E_IS_SPELL_CHECKER (checker));

	webkit_checker_iface = WEBKIT_SPELL_CHECKER_GET_IFACE (checker);
	webkit_checker_iface->ignore_word (WEBKIT_SPELL_CHECKER (checker), word);
}

/**
 * e_spell_checker_learn_word:
 * @checker: an #ESpellChecker
 * @word: word to learn
 *
 * Calls #e_spell_dictionary_learn_word() on all active dictionaries in
 * the @checker.
 */
void
e_spell_checker_learn_word (ESpellChecker *checker,
			    const gchar *word)
{
	WebKitSpellCheckerInterface *webkit_checker_iface;

	g_return_if_fail (E_IS_SPELL_CHECKER (checker));

	webkit_checker_iface = WEBKIT_SPELL_CHECKER_GET_IFACE (checker);
	webkit_checker_iface->learn_word (WEBKIT_SPELL_CHECKER (checker), word);
}
