#ifndef QHDBXCONNECTIONEVENT_H
#define QHDBXCONNECTIONEVENT_H

#include <qhdbxevent.h>
#include <hdbextractor.h>
#include <QString>

class QHdbXConnectionEvent : public QHdbXEvent
{
public:
    explicit QHdbXConnectionEvent(Hdbextractor::DbType dbType,
                                  const QString & host,
                                  const QString & db,
                                  const QString & user,
                                  const QString & passwd,
                                  unsigned short port = 3306);


    Hdbextractor::DbType dbType;

    QString host, user, pass, dbnam;

    unsigned short port;

private:


};

#endif // QHDBXCONNECTIONEVENT_H
