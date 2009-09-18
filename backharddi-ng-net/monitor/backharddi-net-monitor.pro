SUBDIRS += qxmlrpc \
	qtermwidget \
 src
TEMPLATE = subdirs 
CONFIG += ordered \
	  warn_on \
          qt \
          thread 
