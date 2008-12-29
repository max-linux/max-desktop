#!/usr/bin/env python

import os
import sys

ITEMS=[
["Thunderbird", "http://www.tcosproject.org/max/max-win32-apps/Thunderbird_Setup_2.0.0.18.exe", "max-win32-apps/disctree/programs/thunderbird/Thunderbird_Setup_2.0.0.18.exe"],
["Gimp", "http://www.tcosproject.org/max/max-win32-apps/gimp-2.6.3-i686-setup-1.exe", "max-win32-apps/disctree/programs/gimp+gtk/gimp-2.6.3-i686-setup-1.exe"],
["Qcad", "http://www.tcosproject.org/max/max-win32-apps/qcad_2_1_3_2_demo_win32.zip", "max-win32-apps/disctree/programs/qcad/qcad_2_1_3_2_demo_win32.zip"],
["Firefox", "http://www.tcosproject.org/max/max-win32-apps/Firefox_Setup_3.0.5.exe", "max-win32-apps/disctree/programs/firefox/Firefox_Setup_3.0.5.exe"],
["Audacity", "http://www.tcosproject.org/max/max-win32-apps/audacity-win-1.2.6.exe", "max-win32-apps/disctree/programs/audacity/audacity-win-1.2.6.exe"],
["OpenOffice.org", "http://www.tcosproject.org/max/max-win32-apps/OOo_3.0.0_Win32Intel_install_wJRE_es.exe", "max-win32-apps/disctree/programs/OpenOfice/OOo_3.0.0_Win32Intel_install_wJRE_es.exe"],
["KompoZer", "http://www.tcosproject.org/max/max-win32-apps/kompozer-0.7.10-win32.zip", "max-win32-apps/disctree/programs/kompozer/kompozer-0.7.10-win32.zip"],
]

ITEMS_FOR_DOWNLOAD=[
["Bin data", "http://max.educa.madrid.org/max-win32-apps/max-win32-bin.tar.gz", "max-win32-apps/max-win32-bin.tar.gz"],
["OO dicts", "http://max.educa.madrid.org/max-win32-apps/diccionarios.tar.gz", "max-win32-apps/disctree/programs/diccionarios OO/diccionarios.tar.gz"]
]

"""
#max-win32-apps/disctree/programs/thunderbird/Thunderbird Setup 1.5.0.5.exe
#max-win32-apps/disctree/programs/gimp+gtk/gimp-2.2.12-i586-setup.exe
max-win32-apps/disctree/programs/gimp+gtk/gtk+-2.8.18-setup-1.exe
#max-win32-apps/disctree/programs/qcad/qcad_2_1_0_0_demo_win32.exe
max-win32-apps/disctree/programs/mozilla/mozilla-1.7.12.es-ES.win32.installer.exe
max-win32-apps/disctree/programs/winHTTrack/httrack-3.40-2.exe
#max-win32-apps/disctree/programs/firefox/Firefox Setup 1.5.0.6.exe
#max-win32-apps/disctree/programs/OpenOfice/OOo_2.0.3rc7_060622_Win32Intel_install_es_wJRE.exe
#max-win32-apps/disctree/programs/audacity/audacity-win-1.2.4b.exe
#max-win32-apps/disctree/programs/nvu/nvu-1.0.es-ES.win32.installer.exe
"""

LAUNCHER_NAME="max-win32-apps/disctree/programs/__APPDIR__/installer.lch"
LAUNCHER="""[Launch]
ExecuteFile=${cwd}\..\disctree\programs\__APPDIR__\__APPEXE__
"""

def parse_installer(item):
    if item[0] == "Bin data": return
    if item[0] == "OO dicts": return
    appdir=os.path.basename(os.path.dirname(item))
    print "appdir:", appdir
    appexe=os.path.basename(item)
    print "appexe:", appexe
    launcher_name=LAUNCHER_NAME.replace('__APPDIR__', appdir)
    launcher=LAUNCHER.replace('__APPDIR__', appdir)
    launcher=launcher.replace('__APPEXE__', appexe)
    f=open(launcher_name, 'w')
    f.write(launcher)
    f.close()

def get_extension(filename):
    return os.path.basename(filename).split('.')[-1]

def uncompress(item):
    appdir=os.path.dirname(item[2])
    appext=get_extension(item[2])
    if appext == "gz":
        os.system("tar -zxf '%s' -C '%s'" %(item[2],appdir))
    #elif appext == "zip":
    #    os.system("unzip '%s' -d '%s'" %(item[2],appdir))
    # remote downloaded data
    os.unlink(item[2])


def download(item):
    if os.path.exists(item[2]):
        os.unlink(item[2])
    os.system("echo ' * Downloading %s ...'; wget -nv -c '%s' -O '%s'" %(item[0], item[1], item[2]) )


for item in ITEMS:
    parse_installer(item[2])
    download(item)

for item in ITEMS_FOR_DOWNLOAD:
    download(item)
    uncompress(item)
