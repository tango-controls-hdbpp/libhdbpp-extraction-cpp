#ifndef QHDBXERRORQUERYEVENT_H
#define QHDBXERRORQUERYEVENT_H

#include <qhdbxevent.h>
#include <QString>

class QHdbXErrorQueryEvent : public QHdbXEvent
{
public:
    QHdbXErrorQueryEvent(const QString& src, double startT, double stopT);

    QString source;

    double startTime, stopTime;
};

#endif // QHDBXERRORQUERYEVENT_H
