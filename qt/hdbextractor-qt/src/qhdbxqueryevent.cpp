#include "qhdbxqueryevent.h"
#include <string>

QHdbXQueryEvent::QHdbXQueryEvent(const QStringList &srcs, const QString &startD, const QString &stopD)
    : QHdbXEvent(QHdbXEvent::DATA_QUERY)
{
    sources = srcs;
    startDate = startD;
    stopDate = stopD;
}
