#ifndef HISTORICALVIEWWIDGETPRIVATE_H
#define HISTORICALVIEWWIDGETPRIVATE_H


#include "qhistoricalviewwidget.h"
#include <QHash>

class QHistoricalViewWidgetPrivate
{
public:

    QHistoricalViewWidgetPrivate();

    void addTypeIndex(QHistoricalViewWidget::DataType dd, int);

    int index(QHistoricalViewWidget::DataType dt);

private:

    QHash< QHistoricalViewWidget::DataType, int> m_typeIndexHash;

};

#endif // HISTORICALPLOTPRIVATE_H
