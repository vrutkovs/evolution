/*
 * Evolution calendar - Send calendar component dialog
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
 *		JP Rosevear <jpr@ximian.com>
 *
 * Copyright (C) 1999-2008 Novell, Inc. (www.novell.com)
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n-lib.h>
#include "libevolution-utils/e-alert-dialog.h"
#include "send-comp.h"

static gboolean
component_has_new_attendees (ECalComponent *comp)
{
	g_return_val_if_fail (comp != NULL, FALSE);

	if (!e_cal_component_has_attendees (comp))
		return FALSE;

	return g_object_get_data (G_OBJECT (comp), "new-attendees") != NULL;
}

static gboolean
component_has_recipients (ECalComponent *comp)
{
	GSList *attendees = NULL;
	ECalComponentAttendee *attendee;
	ECalComponentOrganizer organizer;
	gboolean res = FALSE;

	g_return_val_if_fail (comp != NULL, FALSE);

	e_cal_component_get_organizer (comp, &organizer);
	e_cal_component_get_attendee_list (comp, &attendees);

	if (!attendees) {
		if (organizer.value && e_cal_component_get_vtype (comp) == E_CAL_COMPONENT_JOURNAL) {
			/* memos store recipients in an extra property */
			icalcomponent *icalcomp;
			icalproperty *icalprop;

			icalcomp = e_cal_component_get_icalcomponent (comp);

			for (icalprop = icalcomponent_get_first_property (icalcomp, ICAL_X_PROPERTY);
			     icalprop != NULL;
			     icalprop = icalcomponent_get_next_property (icalcomp, ICAL_X_PROPERTY)) {
				const gchar *x_name;

				x_name = icalproperty_get_x_name (icalprop);

				if (g_str_equal (x_name, "X-EVOLUTION-RECIPIENTS")) {
					const gchar *str_recipients = icalproperty_get_x (icalprop);

					res = str_recipients && g_ascii_strcasecmp (organizer.value, str_recipients) != 0;
					break;
				}
			}
		}

		return res;
	}

	if (g_slist_length (attendees) > 1 || !e_cal_component_has_organizer (comp)) {
		e_cal_component_free_attendee_list (attendees);
		return TRUE;
	}

	attendee = attendees->data;

	res = organizer.value && attendee && attendee->value && g_ascii_strcasecmp (organizer.value, attendee->value) != 0;

	e_cal_component_free_attendee_list (attendees);

	return res;
}

static gboolean
have_nonprocedural_alarm (ECalComponent *comp)
{
	GList *uids, *l;

	g_return_val_if_fail (comp != NULL, FALSE);

	uids = e_cal_component_get_alarm_uids (comp);

	for (l = uids; l; l = l->next) {
		ECalComponentAlarm *alarm;
		ECalComponentAlarmAction action = E_CAL_COMPONENT_ALARM_UNKNOWN;

		alarm = e_cal_component_get_alarm (comp, (const gchar *) l->data);
		if (alarm) {
			e_cal_component_alarm_get_action (alarm, &action);
			e_cal_component_alarm_free (alarm);

			if (action != E_CAL_COMPONENT_ALARM_NONE &&
			    action != E_CAL_COMPONENT_ALARM_PROCEDURE &&
			    action != E_CAL_COMPONENT_ALARM_UNKNOWN) {
				cal_obj_uid_list_free (uids);
				return TRUE;
			}
		}
	}

	cal_obj_uid_list_free (uids);

	return FALSE;
}

static GtkWidget *
add_checkbox (GtkBox *where,
              const gchar *caption)
{
	GtkWidget *checkbox, *align;

	g_return_val_if_fail (where != NULL, NULL);
	g_return_val_if_fail (caption != NULL, NULL);

	checkbox = gtk_check_button_new_with_mnemonic (caption);
	align = gtk_alignment_new (0.0, 0.5, 0.0, 0.0);
	gtk_alignment_set_padding (GTK_ALIGNMENT (align), 0, 0, 12, 12);
	gtk_container_add (GTK_CONTAINER (align), checkbox);
	gtk_widget_show (checkbox);
	gtk_box_pack_start (where, align, TRUE, TRUE, 2);
	gtk_widget_show (align);

	return checkbox;
}

/**
 * send_component_dialog:
 *
 * Pops up a dialog box asking the user whether he wants to send a
 * iTip/iMip message
 *
 * Return value: TRUE if the user clicked Yes, FALSE otherwise.
 **/
gboolean
send_component_dialog (GtkWindow *parent,
                       ECalClient *client,
                       ECalComponent *comp,
                       gboolean new,
                       gboolean *strip_alarms,
                       gboolean *only_new_attendees)
{
	ECalComponentVType vtype;
	const gchar *id;
	GtkWidget *dialog, *sa_checkbox = NULL, *ona_checkbox = NULL;
	GtkWidget *content_area;
	gboolean res;

	if (strip_alarms)
		*strip_alarms = TRUE;

	if (e_cal_client_check_save_schedules (client) || !component_has_recipients (comp))
		return FALSE;

	vtype = e_cal_component_get_vtype (comp);

	switch (vtype) {
	case E_CAL_COMPONENT_EVENT:
		if (new)
			id = "calendar:prompt-meeting-invite";
		else
			id = "calendar:prompt-send-updated-meeting-info";
		break;

	case E_CAL_COMPONENT_TODO:
		if (new)
			id = "calendar:prompt-send-task";
		else
			id = "calendar:prompt-send-updated-task-info";
		break;
	case E_CAL_COMPONENT_JOURNAL:
		return TRUE;
	default:
		g_message (
			"send_component_dialog(): "
			"Cannot handle object of type %d", vtype);
		return FALSE;
	}

	if (only_new_attendees && !component_has_new_attendees (comp)) {
		/* do not show the check if there is no new attendee and
		 * set as all attendees are required to be notified */
		*only_new_attendees = FALSE;

		/* pretend it as being passed NULL to simplify code below */
		only_new_attendees = NULL;
	}

	if (strip_alarms && !have_nonprocedural_alarm (comp)) {
		/* pretend it as being passed NULL to simplify code below */
		strip_alarms = NULL;
	}

	dialog = e_alert_dialog_new_for_args (parent, id, NULL);
	content_area = e_alert_dialog_get_content_area (E_ALERT_DIALOG (dialog));

	if (strip_alarms)
		sa_checkbox = add_checkbox (GTK_BOX (content_area), _("Send my reminders with this event"));
	if (only_new_attendees)
		ona_checkbox = add_checkbox (GTK_BOX (content_area), _("Notify new attendees _only"));

	res = gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_YES;

	if (res && strip_alarms)
		*strip_alarms = !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (sa_checkbox));
	if (only_new_attendees)
		*only_new_attendees = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ona_checkbox));

	gtk_widget_destroy (GTK_WIDGET (dialog));

	return res;
}

gboolean
send_component_prompt_subject (GtkWindow *parent,
                               ECalClient *client,
                               ECalComponent *comp)
{
	ECalComponentVType vtype;
	const gchar *id;

	vtype = e_cal_component_get_vtype (comp);

	switch (vtype) {
	case E_CAL_COMPONENT_EVENT:
			id = "calendar:prompt-save-no-subject-calendar";
		break;

	case E_CAL_COMPONENT_TODO:
			id = "calendar:prompt-save-no-subject-task";
		break;
	case E_CAL_COMPONENT_JOURNAL:
			id = "calendar:prompt-send-no-subject-memo";
		break;

	default:
		g_message (
			"send_component_prompt_subject(): "
			"Cannot handle object of type %d", vtype);
		return FALSE;
	}

	if (e_alert_run_dialog_for_args (parent, id, NULL) == GTK_RESPONSE_YES)
		return TRUE;
	else
		return FALSE;
}
