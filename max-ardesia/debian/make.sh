
#
# Detects and parses the architecture
#

LAZARUS_VERSION=0.9.28.2

ARCH=$(dpkg-architecture -qDEB_BUILD_ARCH)

case "$ARCH" in

 "i686") ARCH="i386";;

 "i586") ARCH="i386";;

 "i486") ARCH="i386";;

 "amd64") ARCH="x86_64";;

esac

echo "Target architecture: $ARCH"

#
# Detects and parses the OS
#

OS="linux"

echo "Target operating system: $OS"


fpc -S2cgi -O1 -gl -WG -vewnhi -l -Fu/usr/lib/lazarus/$LAZARUS_VERSION/lcl/units/$ARCH-$OS/ -Fu/usr/lib/lazarus/$LAZARUS_VERSION/lcl/units/$ARCH-$OS/ -Fu/usr/lib/lazarus/$LAZARUS_VERSION/lcl/units/$ARCH-$OS/gtk2/ -Fu/usr/lib/lazarus/$LAZARUS_VERSION/packager/units/$ARCH-$OS/ -Fu. -o./magnifier -dLCL -dLCLgtk2 magnifier.dpr
