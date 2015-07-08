#include <QApplication>
#include "qhdbextractor.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QHdbExtractor w;
    a.setApplicationName("QHdbExtractor");
    a.setOrganizationName("Elettra");
    w.show();

    return a.exec();
}
