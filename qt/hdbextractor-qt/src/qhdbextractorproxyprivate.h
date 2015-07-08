#ifndef QHDBEXTRACTORPRIVATE_H
#define QHDBEXTRACTORPRIVATE_H

class QHdbextractorThread;

#include <qhdbdataboundaries.h>

class QHdbextractorProxyPrivate
{
public:

    QHdbextractorProxyPrivate() {}

    QHdbextractorThread *thread;

    QHdbDataBoundaries dataBoundaries;

};

#endif // QHDBEXTRACTORPRIVATE_H
