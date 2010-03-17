#!/bin/sh

if [ "$(whoami)" != "root" ]; then
  sudo bash $0 $@
  exit 0
fi


FILES=$(find /var/lib/dpsyco-skel/skel/skel/ -type f)

for f in $FILES; do
  ORIG=$(echo $f | sed -e 's|/var/lib/dpsyco-skel/skel/skel||g')
  echo "cat $f > $ORIG"
  cat "$f" > "$ORIG"
done


cat << EOF

  You can restore with:

       sudo update-dpsyco-skel

EOF
