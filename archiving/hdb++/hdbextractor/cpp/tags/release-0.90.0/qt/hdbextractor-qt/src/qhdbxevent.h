#ifndef QHDBXEVENT_H
#define QHDBXEVENT_H

class QHdbXEvent
{

public:

    enum EventType { CONNECT, DISCONNECT, DATA_QUERY, ERROR_QUERY, SOURCESLIST };

    QHdbXEvent(EventType et);


    EventType getType() const;


private:

    EventType mType;
};

#endif // QHDBXEVENT_H
