#!/usr/bin/python3

import os
from gi.repository import GLib, Gdk, Gtk, WebKit
from configparser import ConfigParser
import subprocess

import sys
import locale
from optparse import OptionParser


class SlideshowViewer(WebKit.WebView):
    '''
    A basic GTK widget (WebKit.WebView) which displays a slideshow in the
    ubiquity-slideshow format. Feel free to copy and paste this to your application
    and customize it as needed.
    '''

    def __init__(self, path, locale='C', rtl=False, controls=False):
        '''
        @param  path  Path to the slideshow, in which the slideshow.conf file is stored.
        @param  locale  Ideal locale to use for the slideshow
        @param  rtl  True if the given locale should be displayed right-to-left
        '''
        self.path = path

        config = ConfigParser()
        config.read(os.path.join(self.path, 'slideshow.conf'))

        slideshow_main = 'file://' + os.path.join(self.path, 'slides', 'index.html')

        parameters = []
        slideshow_locale = self._find_available_locale(locale)
        parameters.append('locale=%s' % slideshow_locale)
        if rtl:
            parameters.append('rtl')
        if controls:
            parameters.append('controls')

        WebKit.WebView.__init__(self)
        parameters_encoded = '&'.join(parameters)
        self.open('%s#%s' % (slideshow_main, parameters_encoded))

        settings = self.get_settings()
        settings.set_property("enable-default-context-menu", False)
        #Recent webkit feature. See <http://trac.WebKit.org/changeset/52087>.
        settings.set_property("enable-file-access-from-file-uris", True)

        config_width = int(config.get('Slideshow', 'width'))
        config_height = int(config.get('Slideshow', 'height'))
        self.set_size_request(config_width, config_height)

        self.connect('navigation-policy-decision-requested', self._on_navigate_decision)
        self.connect('navigation-requested', self._on_navigate)
        self.connect('new-window-policy-decision-requested', self._on_new_window_decision)
        self.connect('create-web-view', self._on_new_window)

    '''
    Determines the ideal locale for the slideshow, based on the given locale,
    or 'c' if an ideal one is not available.
    @param  locale  The full locale string, for example en_AU.UTF8
    @return  The available locale which best matches the input.
    '''
    def _find_available_locale(self, locale):
        base_slides_dir = os.path.join(self.path, 'slides', 'l10n')
        extra_slides_dir = os.path.join(self.path, 'slides', 'extra')

        ll_cc = locale.split('.')[0]
        ll = ll_cc.split('_')[0]

        for slides_dir in [extra_slides_dir, base_slides_dir]:
            for test_locale in [locale, ll_cc, ll]:
                locale_dir = os.path.join(slides_dir, test_locale)
                if os.path.exists(locale_dir):
                    return test_locale
        return 'C'

    def _new_browser_window(self, uri):
        subprocess.Popen(['xdg-open', uri], close_fds=True)

    def _on_navigate_decision(self, view, frame, req, action, decision):
        reason = action.get_reason()
        if reason == "link-clicked":
            decision.use()
            return False

        decision.ignore()
        return True

    def _on_navigate(self, view, frame, req):
        uri = req.get_uri()
        self._new_browser_window(uri)
        return True

    def _on_new_window_decision(self, view, frame, req, action, decision):
        uri = req.get_uri()
        decision.ignore()
        self._new_browser_window(uri)
        return True

    def _on_new_window(self, view, frame):
        return True


def progress_increment(progressbar, fraction):
    new_fraction = progressbar.get_fraction() + fraction
    if new_fraction > 1:
        progressbar.set_fraction(1.0)
        return False

    progressbar.set_fraction(new_fraction)
    return True


#Main program


default_path = os.path.join(os.path.abspath(os.path.dirname(sys.argv[0])) , 'build', 'ubuntu')

default_locale = locale.getlocale()[0]

parser = OptionParser()
parser.add_option("-l", "--locale", help="LOCALE to use for the slideshow", metavar="LOCALE", default=default_locale)
parser.add_option("-r", "--rtl", action="store_true", help="use output in right-to-left format")
parser.add_option("-c", "--controls", action="store_true", help="Enable controls in the slideshow (you may need to resize the window)")
parser.add_option("-p", "--path", help="path to the SLIDESHOW which will be presented", metavar="SLIDESHOW", default=default_path)

(options, args) = parser.parse_args()
options.path = os.path.abspath(options.path)
if not os.path.exists(options.path):
    sys.exit("\033[91m * Please build the slideshow content first by using the make command * \033[0m")


Gdk.threads_init()

# Set default SSL CA file for secure communication with web services.
# This is important, because libsoup is not secure by default.
soup_session = WebKit.get_default_session()
soup_session.set_property('ssl-strict', True)
soup_session.set_property('ssl-use-system-ca-file', True)


slideshow_window = Gtk.Window()
slideshow_window.set_title("Installer slideshow preview")
slideshow_window.connect('destroy', Gtk.main_quit)

slideshow_window.set_resizable(False)

slideshow_container = Gtk.VBox()
slideshow_window.add(slideshow_container)

slideshow = SlideshowViewer(options.path, locale=options.locale, rtl=options.rtl, controls=options.controls)

install_progressbar = Gtk.ProgressBar()
install_progressbar.set_margin_top(8)
install_progressbar.set_margin_right(8)
install_progressbar.set_margin_bottom(8)
install_progressbar.set_margin_left(8)
install_progressbar.set_fraction(0)


slideshow_container.add(slideshow)
slideshow_container.add(install_progressbar)

slideshow_container.set_child_packing(install_progressbar, True, False, 0, 0)


slideshow_window.show_all()


install_timer = GLib.timeout_add_seconds(2, progress_increment, install_progressbar, 0.01)


Gtk.main()
