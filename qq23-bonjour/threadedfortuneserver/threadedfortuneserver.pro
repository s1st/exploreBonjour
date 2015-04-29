HEADERS       = dialog.h \
                fortuneserver.h \
                fortunethread.h \
                bonjourserviceregister.h
SOURCES       = dialog.cpp \
                fortuneserver.cpp \
                fortunethread.cpp \
                main.cpp \
                bonjourserviceregister.cpp
QT           += network

!mac:LIBS+=-ldns_sd

# install
target.path = $$[QT_INSTALL_EXAMPLES]/network/threadedfortuneserver
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS threadedfortuneserver.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/network/threadedfortuneserver
INSTALLS += target sources
