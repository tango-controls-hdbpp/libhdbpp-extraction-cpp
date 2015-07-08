#include "qhdbnewdataevent.h"

QHdbNewDataEvent::QHdbNewDataEvent(const std::vector<XVariant> &newdata, const QString& srcname,
                                   int _step, int _totalSteps, QEvent::Type eType)
    : QEvent(eType)
{
    data = newdata;
    step = _step;
    totalSteps = _totalSteps;
    source = srcname;
    updateType = Progress;
}

QHdbNewDataEvent::QHdbNewDataEvent(const std::vector<XVariant> &newdata, const QString& srcname,
                 int _srcStep, int totSrcs, double _elapsed, QEvent::Type eType)
    : QEvent(eType)
{
    data = newdata;
    source = srcname;
    sourceStep = _srcStep;
    totalSources = totSrcs;
    elapsed = _elapsed;
    updateType = Finish;
}

