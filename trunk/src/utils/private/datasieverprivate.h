#ifndef DATAFILLERPRIVATE_H
#define DATAFILLERPRIVATE_H

#include <map>
#include <xvariant.h>
#include <string>
#include <list>

class DataSieverProgressListener;

class DataSieverPrivate
{
public:
    DataSieverPrivate();

    std::list<DataSieverProgressListener *>dataSieverProgressListeners;

    std::map<std::string, std::list<XVariant> > dataMap;

    double elapsedMicros;
};

#endif // DATAFILLERPRIVATE_H
