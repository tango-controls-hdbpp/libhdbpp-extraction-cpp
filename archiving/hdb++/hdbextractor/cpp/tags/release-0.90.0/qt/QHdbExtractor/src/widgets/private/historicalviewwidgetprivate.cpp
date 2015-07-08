#include "historicalviewwidgetprivate.h"
#include "historicalviewwidget.h"

HistoricalViewWidgetPrivate::HistoricalViewWidgetPrivate(HistoricalViewWidget *hwidget)
    : q_ptr(hwidget)
{
    m_thdb = NULL;
}

void HistoricalViewWidgetPrivate::addTypeIndex(HistoricalViewWidget::DataType t, int i)
{
    m_typeIndexHash.insert(t, i);
}

int HistoricalViewWidgetPrivate::index(HistoricalViewWidget::DataType t)
{
    if(m_typeIndexHash.contains(t))
        return m_typeIndexHash.value(t);
    return -1;
}
