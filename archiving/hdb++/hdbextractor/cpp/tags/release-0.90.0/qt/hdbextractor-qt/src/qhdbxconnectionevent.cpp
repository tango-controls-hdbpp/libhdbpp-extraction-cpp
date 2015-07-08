#include "qhdbxconnectionevent.h"
#include <QString>

QHdbXConnectionEvent::QHdbXConnectionEvent(Hdbextractor::DbType dbTy,
                                           const QString &ho,
                                           const QString &db,
                                           const QString &username,
                                           const QString &passwd,
                                           unsigned short pt) :
    QHdbXEvent(QHdbXEvent::CONNECT)
{
    dbType = dbTy;
    host = ho;
    user = username;
    dbnam = db;
    pass = passwd;
    port = pt;
}
