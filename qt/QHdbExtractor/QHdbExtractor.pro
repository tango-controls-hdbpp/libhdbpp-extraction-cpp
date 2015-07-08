#-------------------------------------------------
#
# Project created by QtCreator 2011-06-01T15:01:49
#
#-------------------------------------------------

MATHGLROOT = /usr/local/mathgl

unix:INCLUDEPATH += \
    /runtime/include/qgraphicsplot \
    /runtime/include/hdbextractor/qt \
    /runtime/include/hdbextractor/ \
    src/widgets \
    /runtime/include/qdddplot \
    /usr/local/mathgl/include

QT       += core gui widgets opengl

QMAKE_CXXFLAGS += -std=c++98

CONFIG += debug

contains(QT_VERSION, ^5\\..*\\..*) {
    VER_SUFFIX = -qt5
} else {
    VER_SUFFIX =
}

UI_DIR = ui
OBJECTS_DIR = obj

TARGET = bin/qhdbextractor
TEMPLATE = app

QMAKE_CLEAN += core*

SOURCES += src/main.cpp\
    src/widgets/qhdbconfigwidget.cpp \
    src/widgets/qhistoricalviewwidget.cpp \
    src/widgets/qhistoricalviewwidgetprivate.cpp \
    src/qhdbextractor.cpp \
    src/widgets/sourcestreewidget.cpp \
    src/widgets/sourcestreewidget_p.cpp \
    src/widgets/treewidgetsearch.cpp \
    src/widgets/errorstreewidget.cpp \
    src/widgets/waitwidget.cpp

HEADERS  += \
    src/widgets/qhdbconfigwidget.h \
    src/widgets/qhistoricalviewwidgetprivate.h \
    src/widgets/qhistoricalviewwidget.h \
    src/qhdbextractor.h \
    src/widgets/sourcestreewidget.h \
    src/widgets/sourcestreewidget_p.h \
    src/widgets/treewidgetsearch.h \
    src/widgets/errorstreewidget.h \
    src/widgets/waitwidget.h

FORMS    += src/mainwindow.ui \
    src/widgets/configWidget.ui

unix:LIBS += -L../.. -lhdbextractor++ -lhdbextractor-qt$${VER_SUFFIX} \
     -L../hdbextractor-qt -L/runtime/lib -lQGraphicsPlot$${VER_SUFFIX} \
      -lqdddplot$${VER_SUFFIX} -L$${MATHGLROOT}/lib -lmgl -lmgl-qt5
