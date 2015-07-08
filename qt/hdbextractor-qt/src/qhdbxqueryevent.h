#ifndef QHDBXQUERYEVENT_H
#define QHDBXQUERYEVENT_H

#include <qhdbxevent.h>
#include <QStringList>

class QHdbXQueryEvent : public QHdbXEvent
{
public:
    QHdbXQueryEvent(const QStringList &sources, const QString & startDate, const QString &stopDate);

    QStringList sources;
    QString startDate, stopDate;
};

#endif // QHDBXQUERYEVENT_H
