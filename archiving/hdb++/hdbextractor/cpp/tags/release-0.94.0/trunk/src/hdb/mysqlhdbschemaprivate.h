#ifndef MYSQLHDBSCHEMAPRIVATE_H
#define MYSQLHDBSCHEMAPRIVATE_H

#include <pthread.h>

#define MAXERRORLEN 512

class ResultListener;
class XVariantList;

class MySqlHdbSchemaPrivate
{
public:
    MySqlHdbSchemaPrivate() {}

    ResultListener *resultListenerI;

    int notifyEveryNumRows;

    pthread_mutex_t mutex;

    XVariantList *variantList;

    char errorMessage[MAXERRORLEN];

    int sourceStep, totalSources;
};

#endif // MYSQLHDBSCHEMAPRIVATE_H
