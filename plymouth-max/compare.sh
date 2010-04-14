#!/bin/sh

ORIG_PATH=/home/mario/MaX/build/plymouth/plymouth-0.8.1/



echo meld themes/max-logo/max-logo.script $ORIG_PATH/themes/ubuntu-logo/ubuntu-logo.script >&2
diff -ur themes/max-logo/max-logo.script $ORIG_PATH/themes/ubuntu-logo/ubuntu-logo.script 

echo meld plugins/text/plugin.c $ORIG_PATH/src/plugins/splash/ubuntu-text/plugin.c >&2
diff -ur plugins/text/plugin.c $ORIG_PATH/src/plugins/splash/ubuntu-text/plugin.c

