TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=gnu++98

SOURCES += main.cpp \
    myhdbextractorimpl.cpp

INCLUDEPATH += ../src ../src/db


LIBS += -L../src/.libs -lhdbextractor++

HEADERS += \
    myhdbextractorimpl.h

OTHER_FILES += \
    hdbxtest_example_usages.txt
