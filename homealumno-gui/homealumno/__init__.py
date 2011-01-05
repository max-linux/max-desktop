
import os

debug=False
PACKAGE="homealumno"
selected_profile=None
APPLY=False


PROFILES_PATH="/var/lib/homealumno/profiles/"
PROFILES_CONF_FILE="/var/lib/homealumno/profiles.ini"
PRERUN_PATH="/var/lib/homealumno/pre-run/"
POSTRUN_PATH="/var/lib/homealumno/post-run/"

GLADE_DIR="/usr/share/homealumno-gui/"
IMG_DIR="/usr/share/homealumno-gui/images/"

if os.path.isfile('setup.py') and os.path.isfile('homealumno-gui-main.ui'):
    PROFILES_PATH="./profiles/"
    PROFILES_CONF_FILE="./profiles.ini"
    GLADE_DIR="./"
    IMG_DIR="./images/"
