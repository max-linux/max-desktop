#!/bin/bash
# Build config for build.sh
APP_NAME=ubufox
CHROME_PROVIDERS="content locale skin"
CLEAN_UP=1
ROOT_FILES=
ROOT_DIRS="defaults searchplugins components plugins modules"
BEFORE_BUILD=
BEFORE_PACK="sed -ri -e s/@DIST_RELEASE@/`lsb_release -r -s`/g -e s/@DIST_CODENAME@/`lsb_release -s -c`/g defaults/preferences/dist.js"
AFTER_BUILD=
