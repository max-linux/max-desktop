#!/usr/bin/env python
import os
import sys
import time
from ConfigParser import ConfigParser

def main():
    # Tell kconf_update to get rid of all group which specified a default size
    parser = ConfigParser()
    parser.readfp(sys.stdin)
    sections = [x for x in parser.sections() if x.isdigit()]
    for section in sections:
        description = parser.get(section, "Description")
        if description.endswith("initial default size"):
            print "# DELETEGROUP [%s]" % section

    if os.fork() == 0:
        # Wait a bit for kconf_update to write the file, then tell kwin to read
        # our new config
        time.sleep(2)
        os.system("dbus-send --dest=org.kde.kwin /KWin org.kde.KWin.reloadConfig")

if __name__ == "__main__":
    main()
