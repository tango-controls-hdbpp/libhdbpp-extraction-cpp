#ifndef MYSQLHDBSCHEMAPRIVATE_H
#define MYSQLHDBSCHEMAPRIVATE_H

#include <pthread.h>

#define MAXERRORLEN 512

class ResultListener;
class XVariantList;
class QueryConfiguration;

class DbSchemaPrivate
{
public:
    DbSchemaPrivate() {}

    ResultListener *resultListenerI;

    int notifyEveryNumRows;

    pthread_mutex_t mutex;

    XVariantList *variantList;

    QueryConfiguration *queryConfiguration;

    char errorMessage[MAXERRORLEN];

    size_t sourceStep, totalSources;
};

#endif // MYSQLHDBSCHEMAPRIVATE_H
