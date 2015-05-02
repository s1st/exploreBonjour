HEADERS       = server.h \
		bonjourserviceregister.h \
		bonjourrecord.h
SOURCES       = server.cpp \
                main.cpp \
                bonjourserviceregister.cpp
QT           += network widgets

!mac:x11:LIBS+=-ldns_sd

win32 {
    LIBS+=-ldnssd

}

#LIBPATH=/storage/mDNS/mDNSResponder-107.6/mDNSShared/
#INCLUDEPATH += /storage/mDNS/mDNSResponder-107.6/mDNSShared/

LIBS += -ldns_sd
INCLUDEPATH += /usr/lib/x86_64-linux-gnu/
