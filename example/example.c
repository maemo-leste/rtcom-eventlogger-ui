/**
 * Copyright (C) 2005-06 Nokia Corporation.
 * Contact: Salvatore Iovene <ext-salvatore.iovene@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <hildon/hildon-program.h>
#include <libosso.h>
#include <libosso-abook/osso-abook-init.h>
#include <rtcom-eventlogger/eventlogger.h>

#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>

#include <libebook/libebook.h>

#include "rtcom-eventlogger-ui/rtcom-log-view.h"
#include "rtcom-eventlogger-ui/rtcom-log-model.h"
#include "rtcom-eventlogger-ui/rtcom-log-columns.h"
#include "rtcom-eventlogger-ui/rtcom-log-search-bar.h"

#define SERVICE "RTCOM_EL_SERVICE_SMS"
#define EVENT_TYPE "RTCOM_EL_EVENTTYPE_SMS_INBOUND"
#define LOCAL_UID "ext-salvatore.iovene@nokia.com"
#define LOCAL_NAME "Salvatore Iovene"
#define REMOTE_UID "1@foo.org"
#define REMOTE_NAME "1"
#define FREE_TEXT "Hi, how are you? See you later!"

#define PROVIDE_AGGREGATOR 0

static GtkWidget * log_view = NULL;
static RTComLogModel * log_model = NULL;
static GtkWidget * search_bar = NULL;
static RTComElQueryGroupBy static_group_by = RTCOM_EL_QUERY_GROUP_BY_NONE;

void
log_some_stuff(
        RTComEl * el)
{
    RTComElEvent ev;
    time_t t = 0;
    gint i = 0;

    g_return_if_fail(RTCOM_IS_EL(el));

    g_object_ref(el);

    t = time(NULL);
    RTCOM_EL_EVENT_SET_FIELD(&ev, service, SERVICE);
    RTCOM_EL_EVENT_SET_FIELD(&ev, event_type, EVENT_TYPE);
    RTCOM_EL_EVENT_SET_FIELD(&ev, start_time, t);
    RTCOM_EL_EVENT_SET_FIELD(&ev, local_uid, LOCAL_UID);
    RTCOM_EL_EVENT_SET_FIELD(&ev, local_name, LOCAL_NAME);
    RTCOM_EL_EVENT_SET_FIELD(&ev, remote_uid, REMOTE_UID);
    RTCOM_EL_EVENT_SET_FIELD(&ev, remote_name, REMOTE_NAME);
    RTCOM_EL_EVENT_SET_FIELD(&ev, free_text, FREE_TEXT);

    for(i=0; i < 1000; ++i)
        rtcom_el_add_event(el, &ev, NULL);

    g_object_unref(el);
}

static void
populate_all(
        GtkWidget * widget,
        gpointer data)
{
    RTComLogModel * model = NULL;

    g_debug("Populating all...");

    model = RTCOM_LOG_MODEL(data);
    rtcom_log_model_populate(model, NULL);
}

static void
populate_sms(
        GtkWidget * widget,
        gpointer data)
{
    RTComLogModel * model = NULL;
    const gchar * services[] = {"RTCOM_EL_SERVICE_SMS", NULL};

    g_debug("Populating sms...");

    model = RTCOM_LOG_MODEL(data);
    rtcom_log_model_populate(model, services);
}

static void
populate_calls(
        GtkWidget * widget,
        gpointer data)
{
    RTComLogModel * model = NULL;
    const gchar * services[] = {"RTCOM_EL_SERVICE_CALL", NULL};

    g_debug("Populating calls...");

    model = RTCOM_LOG_MODEL(data);
    rtcom_log_model_populate(model, services);
}

static void
refresh(
        GtkWidget * widget,
        gpointer data)
{
    RTComLogModel * model = NULL;

    g_debug("Refreshing...");

    model = RTCOM_LOG_MODEL(data);
    rtcom_log_model_refresh(model);
}

static void
group(
        GtkWidget * widget,
        gpointer data)
{
    RTComLogModel * model = RTCOM_LOG_MODEL(data);

    static_group_by = (1 + static_group_by) % (RTCOM_EL_QUERY_GROUP_BY_GROUP + 1);
    g_debug(G_STRLOC ": setting group_by to: %d", static_group_by);
    rtcom_log_model_set_group_by(model, static_group_by);
    rtcom_log_model_refresh(model);

    if(static_group_by == RTCOM_EL_QUERY_GROUP_BY_NONE)
        gtk_button_set_label(GTK_BUTTON(widget), "Group by contact");
    else if(static_group_by == RTCOM_EL_QUERY_GROUP_BY_CONTACT)
        gtk_button_set_label(GTK_BUTTON(widget), "Group by local/remote");
    else if(static_group_by == RTCOM_EL_QUERY_GROUP_BY_UIDS)
        gtk_button_set_label(GTK_BUTTON(widget), "Group by group-uid");
    else
        gtk_button_set_label(GTK_BUTTON(widget), "Ungroup");
}

int
main(int argc, char * argv[])
{
    HildonProgram * program = NULL;
    HildonWindow * window = NULL;

    osso_context_t * osso_context = NULL;

    GtkWidget * box = NULL;
    GtkWidget * buttons_box = NULL;
    GtkWidget * populate_all_button = NULL;
    GtkWidget * populate_sms_button = NULL;
    GtkWidget * populate_calls_button = NULL;
    GtkWidget * refresh_button = NULL;
    GtkWidget * group_button = NULL;

    GtkWidget * scrolled_window = NULL;

    OssoABookAggregator * aggr;

    gtk_init(&argc, &argv);

    osso_context = osso_initialize(
            "rtcomlogviewexample", "0.1", FALSE, NULL);
    /* Init abook */
    if (!osso_abook_init (&argc, &argv, osso_context)) {
        g_critical ("Error initializing libosso-abook");
        osso_deinitialize (osso_context);
        return 1;
    }

    program = HILDON_PROGRAM(hildon_program_get_instance());
    g_set_application_name("rtcom-eventlogger-ui example");
    window = HILDON_WINDOW(hildon_window_new());
    hildon_program_add_window(program, window);

    log_model = rtcom_log_model_new();

#if PROVIDE_AGGREGATOR
    aggr = OSSO_ABOOK_AGGREGATOR(osso_abook_aggregator_new(NULL, NULL));
    osso_abook_roster_start(OSSO_ABOOK_ROSTER(aggr));
#else
    aggr = NULL;
#endif

    rtcom_log_model_set_abook_aggregator(log_model, aggr);

    log_view = rtcom_log_view_new();
    search_bar = rtcom_log_search_bar_new();

    rtcom_log_model_set_group_by(log_model, static_group_by);
    rtcom_log_search_bar_set_model(
            RTCOM_LOG_SEARCH_BAR(search_bar),
            log_model);
    rtcom_log_search_bar_widget_hook(
            RTCOM_LOG_SEARCH_BAR(search_bar),
            GTK_WIDGET(window),
            GTK_TREE_VIEW(log_view));

    /* Let's put some stuff in the log.
    log_some_stuff(rtcom_log_model_get_eventlogger(log_model));
    */

    rtcom_log_view_set_model(
            RTCOM_LOG_VIEW(log_view),
            rtcom_log_search_bar_get_model_filter(
                RTCOM_LOG_SEARCH_BAR(search_bar)));

    /* The view has acquired its own reference to the
     * model, so we can drop ours. That way the model will
     * be freed automatically when the view is destroyed.
     */
    g_debug("Unreffing the model, because the view reffed it.");
    g_object_unref(log_model);

    rtcom_log_model_populate(log_model, NULL);

    box = gtk_vbox_new(FALSE, 10);

    buttons_box = gtk_hbox_new(FALSE, 0);

    populate_all_button = gtk_button_new_with_label("All");
    g_signal_connect(
            G_OBJECT(populate_all_button),
            "clicked",
            G_CALLBACK(populate_all),
            log_model);
    gtk_box_pack_start(GTK_BOX(buttons_box), populate_all_button, FALSE, FALSE, 0);
    gtk_widget_show(populate_all_button);

    populate_sms_button = gtk_button_new_with_label("SMS");
    g_signal_connect(
            G_OBJECT(populate_sms_button),
            "clicked",
            G_CALLBACK(populate_sms),
            log_model);
    gtk_box_pack_start(GTK_BOX(buttons_box), populate_sms_button, FALSE, FALSE, 0);
    gtk_widget_show(populate_sms_button);

    populate_calls_button = gtk_button_new_with_label("CALLS");
    g_signal_connect(
            G_OBJECT(populate_calls_button),
            "clicked",
            G_CALLBACK(populate_calls),
            log_model);
    gtk_box_pack_start(GTK_BOX(buttons_box), populate_calls_button, FALSE, FALSE, 0);
    gtk_widget_show(populate_calls_button);

    refresh_button = gtk_button_new_with_label("Refresh");
    g_signal_connect(
            G_OBJECT(refresh_button),
            "clicked",
            G_CALLBACK(refresh),
            log_model);
    gtk_box_pack_start(GTK_BOX(buttons_box), refresh_button, FALSE, FALSE, 0);
    gtk_widget_show(refresh_button);

    group_button = gtk_button_new_with_label("Group by contact");
    g_signal_connect(
            G_OBJECT(group_button),
            "clicked",
            G_CALLBACK(group),
            log_model);
    gtk_box_pack_start(GTK_BOX(buttons_box), group_button, FALSE, FALSE, 0);
    gtk_widget_show(group_button);

    gtk_box_pack_start(GTK_BOX(box), buttons_box, FALSE, FALSE, 0);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(
            GTK_SCROLLED_WINDOW(scrolled_window),
            GTK_POLICY_NEVER,
            GTK_POLICY_ALWAYS);
    gtk_container_add(
            GTK_CONTAINER(scrolled_window),
            log_view);

    gtk_box_pack_start(GTK_BOX(box), scrolled_window, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(box), search_bar, FALSE, FALSE, 0);
    gtk_container_add(
            GTK_CONTAINER(window),
            box);

    g_signal_connect(G_OBJECT(window), "delete_event",
            G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(GTK_WIDGET(window));
    gtk_widget_hide(search_bar);
    gtk_main();

#if PROVIDE_AGGREGATOR
    osso_abook_roster_stop(OSSO_ABOOK_ROSTER(aggr));
#endif

    osso_deinitialize (osso_context);

    g_debug("Quitting!");
    return 0;
}

/* vim: set ai et tw=75 ts=4 sw=4: */

