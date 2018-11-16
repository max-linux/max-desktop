#!/usr/bin/env python
# -*- coding: UTF-8 -*-


import os
import sys
import glob
import shutil
import pprint
import itertools

ORIG = sys.argv[1]
DST = sys.argv[2]
MAX = 16

renames = {}
dicc = {
    'configuracin_de_una_tarjeta_de_red_inalmbrica': 'cfgwifi',
    'configuracin_de_la_tarjeta_de_red_cableada': 'cfgwire',
    'explorador_de_archivos': 'fbrowser',
    'instalacin_de_max_en_el_disco_duro': 'hddins',
    'compartir_recursos_en_una_red': 'netshare',
    'pizarras_digitales': 'pizarra',
    '__captura_de_pantallas': '',
    'impresora_en_otro_equipo_con_max': 'priother',
    'documentos_y_experiencias': 'doxyexp',
    'uso_bsico_y_opciones': 'basico',
    'gnu_octave_simulacin_de_procesos': 'octave',
    'el_configurador_de_perfiles': 'hoalum',
    'scribus_publicaciones': 'scribus',
    'actualizacin_de_los_listados_de_paquetes_desde_los_repositorios': 'aptupd',
    'editor_de_imgenes_gimp': 'gimp',
    'configuracin_de_max': 'cfgmax',
    'screenshot_gschem_osx': 'shotosx',
    'seleccin_de_elementos': 'selem',

    '_max_columna_izquierda': '_maxizda',
    '_max_titulo_nodo': '_maxnodo',
    '_max_flecha_nav': '_maxfnav',
    '_max_fondo_pagina': '_maxbg',
    '_max_ocultar_mostrar': '_maxom',
    '_max_max_min': '_maxminm',
    '_max_cabecera': '_maxhead',

    'icon_reflection': 'iconrfl',
    'icon_question': 'iconque',
    'icon_activity': 'iconact',
    'icon_casestudy': 'iconcase',

    'aplicaciones_por_contenidos_y_tareas': 'aptask',
    'alumnos_comparten_informacin': 'pupshare',
    'synapse_lanzador_de_aplicaciones_y_documentos': 'synapse',

    'exe_media_loading': 'exeml',
    'exe_media_background': 'exemb',
    'exe_media_flashPlayer': 'exemfl',
    'exe_media_controls': 'exemc',
    'exe_media_bigplay': 'exembp',
    'exe_media_silverlightPlayer': 'exemslv',

    'exe_lightbox_sprite_prev.': 'exesptp.',
    'exe_lightbox_sprite_next.': 'exesptn.',
    'exe_lightbox_sprite_y.': 'exespty.',
    'exe_lightbox_sprite_x.': 'exesptx.',
    'exe_lightbox_sprite.': 'exespte.',
    'exe_lightbox_default_thumb': 'exeldt',
    'exe_lightbox_loader': 'exell',

    'comprimir_y_descomprimir': 'compress',
    'personalizacin_para_infantil_y_primaria': 'infpri',
    'correo_electrnico': 'email',
    'seleccionar_caractersticas_de_la_instalacin': 'instsel',
    'finalizando_synaptic': 'synapend',
    'icon_preknowledge': 'iconpkn',
    'utilizacin_bsica_del_sistema': 'usebasic',
    'buscar_un_paquete': 'schpkg',
    'alumnos_entregan_examen': 'puptest',
    'introduccin_qu_es_max': 'inmax',
    'el_arranque_y_la_bios': 'bios',
    'instalacin_de_software_de_max': 'maxinst',
    'arranque_de_synaptic': 'synapst',
    'descomprimir_archivos': 'uncomp',
    'copiar_o_mover_archivos': 'cpormv',
    'impresora_en_otro_equipo_con_windows': 'printwin',
    'arranque_del_equipo': 'boot',
    'organizador_de_imgenes_shotwell': 'shotwell',
    'personalizacin_para_infantil_y_primaria': 'peinpri',
    'equipos_con_bios_uefi_windows_8_y_10': 'buefi810',
    'diferentes_vistas_de_documentos': 'docview',
    'impresora_en_red': 'netpr',
    'Preferencias_003': 'pref003',
    'trabajar_desde_un_dvd_en_modo_live.': 'dvdlive.',
    'thunderbird__crear_una_cuenta': 'thunewacc',
    'seleccionar_el_tipo_de_instalacin': 'intype',
    'trabajar_desde_un_dispositivo_usb_en_modo_live': 'usblive',
    'nombre_del_equipo_y_grupo_de_trabajo': 'hostnam',
    'instalacin_de_software.': 'intsoft.',
    'seleccin_de_usuarios': 'usersel',
    'el_instalador_de_max': 'instmax',
    'configuracin_de_red': 'netconf',
    'terminales_ligeros_tcos': 'tcos',
    'seleccin_de_un_paquete_para_instalar': 'selpkg',
    'icon_objectives': 'iconobj',
    'alojamiento_de_ficheros_en_la_red': 'netfile',
    'speedcrunch_calculator': 'sppedcrh',
    'barra_superior': 'topbar',
    'crear_un_usb_autoarrancable': 'usbboot',
    'instalacin_infantil_y_primaria': 'insinpri',
    'gestor_de_paquetes_synaptic': 'gpsynap',
    'visor_de_imgenes_eye_of_mate': 'veom',
    'guardar_archivo_como_microsoft_office': 'savems',
    'step_simulador_fsico': 'step',
    'comprimir_archivos': 'flcomp',
    'particionando_discos_con_gparted': 'gparted',
    'trabajar_desde_un_dvd_en_modo_live_con_la_accesibilidad_activada.': 'dvdliacc.',
    'modos_de_funcionamiento_de_max': 'modemax',
    'el_profesor_quiere_enviar_ficheros': 'tchsdfile',
    'el_gestor_de_entrada_lightdm': 'lightdm',
    'cmo_accede_un_alumno_a_carpeta_compartida': 'pupshar',
    'firefox_en_escritorio': 'ffdesk',
    'trabajar_con_documentos_de_office': 'mswork',
    'compartir_una_carpeta_con_alumnos': 'sharepup',
    'configurar_la_red': 'netcfg',
    'musescore_edicin_de_partituras': 'musecor',
    'ejecutar_el_instalador': 'runinst',
    'aplicar_los_cambios_marcados': 'chapp',
    'guardar_archivo_como_pdf': 'savpdf',
    'entorno_de_escritorio_y_personalizacin': 'dskenvmod',
    'cuatro_en_raya': '4enry',
    'fuentes_tipogrficas': 'fttf',
    'trabajar_desde_el_disco_duro_con_la_distribucin_instalada': 'wkhddin',
    'finalizar_la_instalacin': 'instfin',
    'geda_schematic_editor': 'geda',
    'reproductores_multimedia': 'music',
    'directorio_de_ficheros_de_educamadrid': 'edmad',
    'conceptos_generales_sobre_la_gestin_de_paquetes': 'mngtpkg',
    'gtick_metrnomo': 'gtick',
    'aplicaciones_bsicas': 'basic',
    'comparticin_de_recursos': 'shrrec',
    'otros_programas_aadidos': 'otheadd',
    'gestin_de_repositorios': 'repos',
    'capturar_pantalla': 'screensh',
    'la_pantalla_principal': 'mainscr',
    'crear_una_nueva_carpeta': 'mkdir',
    'herramienta_de_bsqueda_de_mate': 'matesch',
    'editor_de_grficos_vectoriales_inkscape': 'inkscp',
    'archivo_ppd': 'ppdfile',
    'insta_sin_xfce': 'inoxfce',
    'openstreetmap': 'opstrmap',
    'exelearning': 'exelrn',
    'frozenbubble': 'fzbub',
    'libreoffice': 'lboff',
    'max_en_un_mac': 'maxmac',
    'Mate-logo.svg.png': 'matelogo.png',
    'quadrapassel': 'quadrap',
    'impresora_usb': 'printusb',
    'para_saber_ms': 'knomor',
    'origen_de_max': 'orgmax',

    'personalizacin.': 'perso.',
    'Thumbnail': 'tb',

    'men_principal.': 'mppal',
    'pantalla_de_bienvenida': 'pwelc',
    'trabajar_en_modo_live_desde_un_dvd_o_usb_con_la_accesibilidad_activada': 'tmlive',
    'seleccin_de_color_de_mate': 'scolmate',
    'max_en_el_aula': 'maxaula',
    'max_USB_inicial': 'maxUSB',

    # 'otros_dispositivos.': 'othedev.',
    # 'usbs_y_otros_dispositivos.': 'usbdevs.',
    # 'usbs_y_': 'usb',
}

dicc2 = {
    'usbs_y_otros_dispositivos.html': 'usbsdev.html',
    'otros_dispositivos.html': 'othdevs.html',
}


def getName(name):
    for w in dicc:
        if w in name:
            name = name.replace(w, dicc[w])
    #
    for w in dicc2:
        if w == name:
            name = dicc2[w]
    return name


def replace(fname, replaces):
    f = open(fname, 'r')
    raw = f.read()
    f.close()
    raw2 = raw
    for r in replaces:
        raw2 = raw2.replace(r, replaces[r])

    if raw == raw2:
        # print " * No changes in %s" % fname
        return 'no changes'

    f = open(fname, 'w')
    f.write(raw2)
    f.close()
    #
    return (len(raw2) / len(raw)) * 100.0


keys = sorted(sorted(dicc), key=dicc.get)
by_val = [(v, list(ks)) for v, ks in itertools.groupby(keys, dicc.get)]
first_dict = dict((ks[0], v) for v, ks in by_val)
duplicate_dict = dict((k, v) for v, ks in by_val for k in ks[1:])

if len(duplicate_dict) > 0:
    print duplicate_dict
    print " * ERROR: found duplicate values in dicc"
    sys.exit(1)


numbad = 0
for fp in glob.glob(ORIG + '/*'):
    f = os.path.basename(fp)
    # print f

    if len(f) < MAX:
        if not os.path.exists(DST + '/' + f):
            # print " * Copy %s" % f
            shutil.copy2(fp, DST + '/' + f)

    else:
        newf = getName(f)
        if len(newf) > MAX:
            print f, newf
            numbad += 1

        renames[f] = newf
        if not os.path.exists(DST + '/' + f):
            shutil.copy2(fp, DST + '/' + newf)


if numbad > 0:
    print "numbad > 0, exit"
    sys.exit(1)

# pprint.pprint(renames)


# search and replace inside text files
types = ('*.html', '*.css', '*.js')
files = []
for t in types:
    files.extend(glob.glob(DST + '/' + t))

for fp in files:
    replace(fp, renames)
    # print fp, replace(fp, renames), '%'

for f in files:
    os.system("rgrep '%s' %s " % (f, DST))
