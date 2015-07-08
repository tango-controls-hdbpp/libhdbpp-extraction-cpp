#ifndef HISTORICALVIEWWIDGETPRIVATE_H
#define HISTORICALVIEWWIDGETPRIVATE_H


#include "historicalviewwidget.h"
#include <QHash>

class THdb;

class HistoricalViewWidgetPrivate
{
public:

    HistoricalViewWidgetPrivate(HistoricalViewWidget *hview);

    void setDataType(HistoricalViewWidget::DataType dataType);

    HistoricalViewWidget::DataType dataType() const { return m_dataType; }

    void addTypeIndex(HistoricalViewWidget::DataType, int);

    int index(HistoricalViewWidget::DataType t);

private:

    HistoricalViewWidget *q_ptr;

    THdb *m_thdb;

    HistoricalViewWidget::DataType m_dataType;

    QHash<HistoricalViewWidget::DataType, int> m_typeIndexHash;

    Q_DECLARE_PUBLIC(HistoricalViewWidget);

};

#endif // HISTORICALPLOTPRIVATE_H
