 #include "qhistoricalviewwidgetprivate.h"
#include "qhistoricalviewwidget.h"

QHistoricalViewWidgetPrivate::QHistoricalViewWidgetPrivate()
{
}

void QHistoricalViewWidgetPrivate::addTypeIndex(QHistoricalViewWidget::DataType dd, int i)
{
    m_typeIndexHash.insert(dd, i);
}

int QHistoricalViewWidgetPrivate::index(QHistoricalViewWidget::DataType dd)
{
    if(m_typeIndexHash.contains(dd))
        return m_typeIndexHash.value(dd);
    return -1;
}
