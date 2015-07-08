#ifndef QHDBNEWERRORDATAEVENT_H
#define QHDBNEWERRORDATAEVENT_H

#include <qhdbnewdataevent.h>

class QHdbNewErrorDataEvent : public QHdbNewDataEvent
{
public:
    QHdbNewErrorDataEvent(const std::vector<XVariant> &newdata, const QString& srcname,
                     int _step, int _totalSteps);

    QHdbNewErrorDataEvent(const std::vector<XVariant> &newdata, const QString& srcname,
                     int _srcStep, int totSrcs, double _elapsed);
};

#endif // QHDBNEWERRORDATAEVENT_H
