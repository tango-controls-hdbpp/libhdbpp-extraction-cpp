#ifndef QHDBNEWDATAEVENT_H
#define QHDBNEWDATAEVENT_H

#include <QEvent>
#include <vector>
#include <xvariant.h>
#include <QString>

class QHdbNewDataEvent : public QEvent
{
public:
    enum UpdateType { Progress, Finish };

    QHdbNewDataEvent(const std::vector<XVariant> &newdata, const QString& srcname,
                     int _step, int _totalSteps, QEvent::Type eType = (QEvent::Type) (QEvent::User + 1001));

    QHdbNewDataEvent(const std::vector<XVariant> &newdata, const QString& srcname,
                     int _srcStep, int totSrcs, double _elapsed,
                     QEvent::Type eType = (QEvent::Type) (QEvent::User + 1001));

    std::vector<XVariant> data;

    int step, totalSteps, sourceStep, totalSources;

    QString source;

    double elapsed;

    UpdateType updateType;
};

#endif // NEWDATAEVENT_H
