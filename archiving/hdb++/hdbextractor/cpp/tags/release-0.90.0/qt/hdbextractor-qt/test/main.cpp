#include <stdio.h>
#include <stdlib.h>
#include "../src/qhdbextractorproxy.h"
#include <QApplication>

using namespace std;

int main(int argc, char **argv)
{
    QApplication a(argc, argv);
    if(argc < 4)
    {
        printf("\e[1;31mUsage\e[0m \"%s domain/family/member/attribute 2014-07-20 10:00:00 2014-07-20 12:00:00\n",
               argv[0]);
        exit(EXIT_FAILURE);
    }
    else
    {
        const char *dbuser = "hdbbrowser";
        const char *dbpass = "hdbbrowser";
        const char *dbhost = "fcsproxy";
        const char *dbnam = "hdb";

        QHdbextractorProxy *qhdbxp = new QHdbextractorProxy();
        qhdbxp->connect(Hdbextractor::HDBMYSQL, dbhost, dbnam, dbuser, dbpass);
        qhdbxp->getHdbExtractor()->setUpdateProgressStep(2);
        qhdbxp->getData(argv[1], argv[2], argv[3]);
    }
    return a.exec();
}


