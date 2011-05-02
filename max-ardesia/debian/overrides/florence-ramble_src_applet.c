/* 
   Florence - Florence is a simple virtual keyboard for Gnome.

   Copyright (C) 2008, 2009, 2010 François Agrech

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  

*/

#include <sys/types.h>
#include <getopt.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "system.h"
#include <panel-applet.h>
#include <gconf/gconf-client.h>
#include "trace.h"
#include "settings.h"
#include "tools.h"
#include "florence.h"


void trayicon_help(void)
{
#if GTK_CHECK_VERSION(2,14,0)
        GError *error=NULL;
        gtk_show_uri(NULL, "ghelp:florence", gtk_get_current_event_time(), &error);
        if (error) flo_error(_("Unable to open %s"), "ghelp:florence");
#else
        if (!gnome_help_display_uri("ghelp:florence", NULL)) {
                flo_error(_("Unable to open %s"), "ghelp:florence");
        }
#endif
}

struct florence *florence=NULL;

void applet_properties (BonoboUIComponent *uic, gpointer *florence, gchar *cname)
{
	settings();
}

void applet_about (BonoboUIComponent *uic, gpointer *florence, gchar *cname)
{
	trayicon_about();
}

void applet_help (BonoboUIComponent *uic, gpointer *florence, gchar *cname)
{
	trayicon_help();
}

static gboolean
florence_applet_factory(PanelApplet *applet,
                        const gchar *iid,
                        gpointer data)
{
	const char *modules;
	static const BonoboUIVerb verbs [] = {
		BONOBO_UI_VERB ("Properties", (BonoboUIVerbFn)applet_properties),
		BONOBO_UI_VERB ("About", (BonoboUIVerbFn)applet_about),
		BONOBO_UI_VERB ("Help", (BonoboUIVerbFn)applet_help),
		BONOBO_UI_VERB_END
	};
	char *menu = g_strdup_printf(
		"<popup name=\"button3\">\
		        <menuitem name=\"Properties\" verb=\"Properties\" _label=\"%s\" pixtype=\"stock\" pixname=\"gtk-properties\"/>\
		        <menuitem name=\"About\" verb=\"About\" _label=\"%s\" pixtype=\"stock\" pixname=\"gtk-about\"/>\
		        <menuitem name=\"Help\" verb=\"Help\" _label=\"%s\" pixtype=\"stock\" pixname=\"gtk-help\"/>\
		</popup>", _("_Preferences..."), _("_About..."), _("_Help..."));
	
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, FLORENCELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	g_return_val_if_fail (PANEL_IS_APPLET (applet), FALSE);

	panel_applet_setup_menu(PANEL_APPLET (applet),
		menu,
		verbs,
		florence);
	g_free(menu);

	modules = g_getenv("GTK_MODULES");
	if (!modules||modules[0]=='\0') putenv("GTK_MODULES=gail:atk-bridge");
        florence = flo_new(TRUE, NULL, applet);
	
	gtk_widget_show_all (GTK_WIDGET (applet));

	return TRUE;
}

int main (int argc, char *argv [])
{
	GOptionContext *context;
	GError *error;
	int retval;

	context = g_option_context_new ("");
	g_option_context_add_group (context, gtk_get_option_group (TRUE));
	g_option_context_add_group (context,
				    bonobo_activation_get_goption_group ());
	
	error = NULL;
	if (!g_option_context_parse (context, &argc, &argv, &error)) {
		if (error) {
			g_printerr (_("Cannot parse arguments: %s.\n"),
				    error->message);
			g_error_free (error);
		} else
			g_printerr (_("Cannot parse arguments.\n"));
		g_option_context_free (context);
		return 1;
	}
	
	gtk_init (&argc, &argv);
	gconf_init(argc, argv, NULL);
	g_type_init();	

	if (!bonobo_init (&argc, argv)) {
		g_printerr (_("Cannot initialize bonobo.\n"));
		return 1;
	}
	
	settings_init(FALSE, NULL);

	retval = panel_applet_factory_main ("OAFIID:GNOME_FlorenceApplet_Factory", 
					    PANEL_TYPE_APPLET, 
					    florence_applet_factory,
					    NULL);
	g_option_context_free (context);
	if (florence) flo_free(florence);
	
	return retval;
}


