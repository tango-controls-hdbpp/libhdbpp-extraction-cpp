#ifndef QHDBSOURCESLISTREADYEVENT_H
#define QHDBSOURCESLISTREADYEVENT_H

#include <QEvent>
#include <QStringList>

class QHdbSourcesListReadyEvent : public QEvent
{
public:
    explicit QHdbSourcesListReadyEvent(const QStringList& srcs);

    QStringList sourcesList;
};

#endif // QHDBSOURCESLISTREADYEVENT_H
