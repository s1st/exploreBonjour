HEADERS       = client.h \
                bonjourservicebrowser.h \
		bonjourserviceresolver.h \
		bonjourrecord.h
SOURCES       = client.cpp \
                main.cpp \
                bonjourservicebrowser.cpp \
                bonjourserviceresolver.cpp
QT           += network widgets

#!mac:x11:LIBS+=-ldns_sd
#win32:LIBS+=-ldnssd
#CONFIG += link_pkgconfig
#PKGCONFIG += libdns_sd

#LIBPATH=/usr/include/avahi-compat-libdns_sd/

LIBS += -ldns_sd
INCLUDEPATH += /usr/lib/x86_64-linux-gnu/

