#include "qhdbsourceslistreadyevent.h"

QHdbSourcesListReadyEvent::QHdbSourcesListReadyEvent(const QStringList& srcs) :
    QEvent((QEvent::Type) (QEvent::User + 1011))
{
    sourcesList  = srcs;
}
