
// ubuntu-printing overwrites
pref("print.print_command", "lpr ${MOZ_PRINTER_NAME:+-P\"$MOZ_PRINTER_NAME\"}");
pref("print.postscript.print_command", "lpr ${MOZ_PRINTER_NAME:+-P\"$MOZ_PRINTER_NAME\"}");


// look-and-feel modifications in firefox-branding.js
pref("startup.homepage_override_url","chrome://ubufox/locale/ubufox.properties");
pref("startup.homepage_welcome_url","chrome://ubufox/locale/ubufox.properties");
pref("app.update.url.details","chrome://ubufox/locale/ubufox.properties");

// look-and-feel modifications in firefox.js
pref("browser.startup.homepage","chrome://ubufox/locale/ubufox.properties");
pref("app.releaseNotesURL","chrome://ubufox/locale/ubufox.properties");
pref("browser.throbber.url","chrome://ubufox/locale/ubufox.properties");

pref("browser.link.open_newwindow", 3);
pref("browser.link.open_external", 3);
pref("middlemouse.contentLoadURL", false); // setting to false disables pasting urls on to the page

// disable-default-browser-check on startup by defaullt
pref("browser.shell.checkDefaultBrowser", false);

// locale-by-match-os
pref("intl.locale.matchOS", true);


// intl-property fixes (minimal set resurrected from ubufox 0.5 in 0.6~b2)
pref("general.useragent.locale", "chrome://global/locale/intl.properties");

// kerberos-for-https
pref("network.negotiate-auth.trusted-uris", "https://");

