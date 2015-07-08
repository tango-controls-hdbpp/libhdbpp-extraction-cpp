#include "qhdbxevent.h"

QHdbXEvent::QHdbXEvent(EventType et)
{
    mType = et;
}

QHdbXEvent::EventType QHdbXEvent::getType() const
{
    return mType;
}
