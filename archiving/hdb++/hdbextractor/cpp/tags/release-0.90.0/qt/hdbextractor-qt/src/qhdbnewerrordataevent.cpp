#include "qhdbnewerrordataevent.h"

QHdbNewErrorDataEvent::QHdbNewErrorDataEvent(const std::vector<XVariant> &newdata, const QString& srcname,
                 int _step, int _totalSteps) : QHdbNewDataEvent(newdata, srcname, _step, _totalSteps,
                                                                (QEvent::Type) (QEvent::User + 1211))
{

}

QHdbNewErrorDataEvent::QHdbNewErrorDataEvent(const std::vector<XVariant> &newdata, const QString& srcname,
                 int _srcStep, int totSrcs, double _elapsed)
 : QHdbNewDataEvent(newdata, srcname, _srcStep, totSrcs, _elapsed,
                    (QEvent::Type) (QEvent::User + 1211))
{

}

