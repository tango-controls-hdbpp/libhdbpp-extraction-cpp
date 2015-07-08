#include "qhdbxerrorqueryevent.h"

QHdbXErrorQueryEvent::QHdbXErrorQueryEvent(const QString &src, double startT, double stopT)
    : QHdbXEvent(QHdbXEvent::ERROR_QUERY)
{
    startTime = startT;
    stopTime = stopT;
    source = src;
}
