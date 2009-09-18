CC = gcc
CXX = g++
CFLAGS = -g -Wall -O2
LIBS = -lm
LIB  = -lxmlrpc++ -lssl
CXXFLAGS = -O2 -Wall

all: ipsc bxmlclient-ng 

bxmlclient-ng: bxmlclient.cpp
	$(CXX) $(CXXFLAGS) $(LIB) $< -o $@

ipsc: main.o ipsc.o ifc.o prips.o
	$(CC) $(CFLAGS) $(LIBS) main.o ipsc.o ifc.o \
		prips.o -o backharddi-ng-net-ipsc

ipsc.o: ipsc.c ipsc.h
	$(CC) $(CFLAGS) -c ipsc.c

main.o: main.c ipsc.c
	$(CC) $(CFLAGS) -c main.c

ifc.o: ifc.h ifc.c
	$(CC) $(CFLAGS) -c ifc.c

clean: 
	$(RM) *.o *~ core backharddi-net-ipsc bxmlclient-ng
