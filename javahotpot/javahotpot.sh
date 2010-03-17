#!/bin/bash
NOCONFIGURADO=1

CADENA='<prefs-path-file></prefs-path-file>'
FICHERO="$HOME/Library/Preferences/com.halfbakedsoftware/hotpot6prefs.xml"

if [ ! -d $HOME/Library ] || [ $(grep -c Mozilla $HOME/Library/Preferences/com.halfbakedsoftware/hotpot6prefs.xml) != 0 ] ; then
	rm -rf $HOME/Library
        cp -r /usr/share/javahotpot/Library $HOME/
        #cp /usr/share/applications/javahotpot.desktop $HOME/Desktop/
        cp -r /usr/share/javahotpot/interface $HOME/
fi

grep  -q "$CADENA" "$FICHERO"
if [ $?  -ne $NOCONFIGURADO ];then
        arg=`echo $HOME | sed -e 's/\//+/g'`

	CAMBIO1="<prefs-path-file>'$arg'\/Library\/Preferences\/com.halfbakedsoftware\/hotpot6prefs.xml<\/prefs-path-file>"
        CAMBIO2="<interface-file>\/usr\/share\/javahotpot\/interface\/JHP6Spanish.xml<\/interface-file>"
        CAMBIO3="<config-file>\/usr\/share\/javahotpot\/config\/espanol6.cfg<\/config-file>"
        CAMBIO4="<source-folder>\/usr\/share\/javahotpot\/source<\/source-folder>"
        sed -e 's/<prefs-path-file><\/prefs-path-file>/'$CAMBIO1'/g' $HOME/Library/Preferences/com.halfbakedsoftware/hotpot6prefs.xml >/tmp/_temporal
	sed -e 's/<interface-file><\/interface-file>/'$CAMBIO2'/g' /tmp/_temporal > /tmp/_temporal1
        sed -e 's/<config-file><\/config-file>/'$CAMBIO3'/g' /tmp/_temporal1 > /tmp/_temporal2
        sed -e 's/<source-folder><\/source-folder>/'$CAMBIO4'/g' /tmp/_temporal2>/tmp/_temporal3
        sed -e 's/+/\//g' /tmp/_temporal3> $HOME/Library/Preferences/com.halfbakedsoftware/hotpot6prefs.xml
fi


export CLASSPATH=CLASSPATH:/usr/share/javahotpot

cd /usr/share/javahotpot
java -jar hotpot6.jar

