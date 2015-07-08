#ifndef QHDBXERROREVENT_H
#define QHDBXERROREVENT_H

#include <QEvent>
#include <QString>

class QHdbXErrorEvent : public QEvent
{
public:
    QHdbXErrorEvent(const QString& message);

    QString message;
};

#endif // QHDBXERROREVENT_H
