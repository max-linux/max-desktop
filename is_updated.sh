#!/bin/bash


CHANGELOGS=$(find ./ -maxdepth 3 -mindepth 3 -name "changelog")

for change in $CHANGELOGS; do
  source=$(dpkg-parsechangelog -l$change | awk '/^Source/ {print $2}')
  version=$(dpkg-parsechangelog -l$change | awk '/^Version/ {print $2}')
  letter=$(echo $source| cut -c1)
  #pkgdir="../../../max40/pool/main/$letter/$source/"
  #pkgdir="../../../max50/pool/main/$letter/$source/"
  pkgdir="../../../max60/pool/main/$letter/$source/"
  #echo "source=$source version=$version"

  if [ -d $pkgdir ]; then
    dscpkg=$(find $pkgdir -name "$source*dsc")
    dscversion=$(basename $dscpkg | awk -F "_" '{print $2}' | sed 's/.dsc//g')
    if [ "$version" != "$dscversion" ]; then
      echo " ** WARNING ** obsolete package $dscpkg in mirror $source NEW=$version OLD=$dscversion"
    fi
  else
    echo "**WARNING $source is not in mirror"
  fi
done
