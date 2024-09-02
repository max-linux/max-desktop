// evitar la denegación de ejecución de plugins no actualizados
// github #492 https://github.com/max-linux/max-desktop/issues/492
pref("extensions.blocklist.enabled",false);
// evitar que deniegue conexiones seguras via proxy a partir de la versión 27 de firefox
//pref("security.tls.version.max",1);

// allow old certificates github #889 https://github.com/max-linux/max-desktop/issues/889
//pref("security.tls.version.min",0);

// github #902 Scratch online
pref("security.tls.version.max",3);
pref("security.tls.version.min",1);


// disable PDF internal plugin
// https://github.com/max-linux/max-desktop/issues/552
pref("pdfjs.disabled",false);
//pref("plugin.disable_full_page_plugin_for_types", "application/pdf");


// Mozilla Firefox >=59 languages change
// http://herramientas.educa.madrid.org/foros/viewtopic.php?f=2&t=3733&p=12757
//pref("intl.locale.matchOS", false);
pref("intl.locale.requested", '');
