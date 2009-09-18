SOURCES += main.cpp \
 backharddinet_monitor.cpp \
 cclient.cpp \
 cgroup.cpp \
 clientdatabase.cpp \
 clientmanager.cpp \
 clientslist.cpp \
 clientstree.cpp \
 Common.cpp \
 Database.cpp \
 generaltab.cpp \
 groupconfigform.cpp \
 groupdatabase.cpp \
 groupmanager.cpp \
 groupslist.cpp \
 Loader.cpp \
 MonitorServer.cpp \
 opentaskform.cpp \
 preferencesdialog.cpp \
 qrc_Resources.cpp \
 Task.cpp \
 TaskDatabase.cpp \
 TaskManager.cpp \
 taskstree.cpp \
 variantdelegate.cpp \
 shellwidget.cpp \
 tabwidget.cpp \
 osdwidget.cpp
TEMPLATE = app
CONFIG += warn_on \
	  thread \
          qt \
 	  precompile_header \
 	  qtestlib \
 	  uitools \
 debug
TARGET = ../bin/backharddi-ng-net-monitor


QT += script \
sql \
svg \
xml \
network \
qt3support
HEADERS += backharddinet_monitor.h \
cclient.h \
cgroup.h \
clientdatabase.h \
clientmanager.h \
clientslist.h \
clientstree.h \
Common.h \
configureGroup.h \
Database.h \
generaltab.h \
groupconfigform.h \
groupdatabase.h \
groupmanager.h \
groupslist.h \
Loader.h \
MainForm.h \
MonitorServer.h \
opentaskform.h \
preferencesdialog.h \
preferencesSQL.h \
TaskDatabase.h \
Task.h \
TaskManager.h \
taskstree.h \
variantdelegate.h \
shellwidget.h \
tabwidget.h \
osdwidget.h
FORMS += configureGroup.ui \
MainForm.ui \
preferencesSQL.ui \
ShellWidgetBase.ui
RESOURCES += application.qrc



PRECOMPILED_HEADER = stable.h
TRANSLATIONS += monitor_es.ts

INCLUDEPATH += ../qtermwidget/lib \
../qxmlrpc
LIBS += ../qtermwidget/libqtermwidget.a \
-L../qxmlrpc \
-lXmu \
-lqxmlrpc-ng
TARGETDEPS += ../qtermwidget/libqtermwidget.a



CONFIG -= release

QMAKE_CXXFLAGS_DEBUG += -D__MONITOR_DEBUG \
  -O2

QMAKE_CXXFLAGS_RELEASE += -MD \
  -O2

