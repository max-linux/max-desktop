#!/bin/sh




#metacitydir = /usr/share/themes/MaxTheme/metacity-1
#metacity_DATA = \
#     metacity-1/metacity-theme-2.xml \
#     metacity-1/metacity-theme-3.xml

DIRS=$(find ./ -mindepth 1 -type d)

for d in $DIRS; do

  dname=$(echo $d | sed -e 's/-2.0/2/g' -e 's/-3.0/3/g' -e 's@./@@g' -e 's/-1/1/g')
  dest=$(echo $d | sed -e 's@^./@@g')
  #echo "d=$d  dname=$dname"

  echo "${dname}dir = /usr/share/themes/MaxTheme/${dest}"
  echo "${dname}_DATA = \\"
  find $dest -type f | awk '{print "    "$1" \\"}'
  echo "

"

done


cat << EOF

themedir = /usr/share/themes/MaxTheme
theme_DATA = index.theme

EXTRA_DIST = \$(theme_DATA)      \\
             \$(metacity_DATA)   \\
             \$(gtk2_DATA)       \\
             \$(gtk2apps_DATA)   \\
             \$(gtk3_DATA)       \\
             \$(gtk3apps_DATA)   \\
             \$(gtk3assets_DATA) \\
             \$(unity_DATA)
EOF
