#ifndef MYSQLHDBSCHEMAPRIVATE_H
#define MYSQLHDBSCHEMAPRIVATE_H

#include <pthread.h>

#define MAXERRORLEN 512

class ResultListener;
class XVariantList;
class HdbXSettings;

class DbSchemaPrivate
{
public:
    DbSchemaPrivate() {}

    ResultListener *resultListenerI;

    int notifyEveryPercent;

    pthread_mutex_t mutex;

    XVariantList *variantList;

    HdbXSettings *hdbxSettings;

    char errorMessage[MAXERRORLEN];

    size_t totalRowCnt;

    bool isCancelled;
};

#endif // MYSQLHDBSCHEMAPRIVATE_H
