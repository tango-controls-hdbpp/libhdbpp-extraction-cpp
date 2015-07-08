#include "qhdbxerrorevent.h"

QHdbXErrorEvent::QHdbXErrorEvent(const QString &msg)
    : QEvent((QEvent::Type) (QEvent::User + 1002))
{
    message = msg;
}
