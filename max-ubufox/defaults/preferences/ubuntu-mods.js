// look-and-feel modifications in firefox-branding.js
pref("startup.homepage_override_url","chrome://ubufox/locale/ubufox.properties");
pref("startup.homepage_welcome_url","chrome://ubufox/locale/ubufox.properties");

// look-and-feel modifications in firefox.js
pref("browser.startup.homepage","chrome://ubufox/locale/ubufox.properties");

pref("browser.link.open_newwindow", 3);
pref("browser.link.open_external", 3);
pref("middlemouse.contentLoadURL", false); // setting to false disables pasting urls on to the page

// kerberos-for-https
pref("network.negotiate-auth.trusted-uris", "https://");
