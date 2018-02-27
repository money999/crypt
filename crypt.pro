#-------------------------------------------------
#
# Project created by QtCreator 2014-08-21T23:59:22
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = crypt
TEMPLATE = app


SOURCES += main.cpp\
        cryptfile.cpp

HEADERS  += cryptfile.h

RESOURCES += \
    image/image.qrc

OTHER_FILES += \
    kls.rc

RC_FILE = \
  kls.rc


LIBS+= $${PWD}/libcrypt_fzj.a


