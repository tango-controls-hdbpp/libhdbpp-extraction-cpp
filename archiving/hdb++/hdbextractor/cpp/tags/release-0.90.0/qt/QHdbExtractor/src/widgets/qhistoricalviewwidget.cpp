#include "qhistoricalviewwidget.h"
#include "qhistoricalviewwidgetprivate.h"
#include <hdbxmacros.h>

QHistoricalViewWidget::~QHistoricalViewWidget()
{
    printf("\e[1;31m~HistoricalViewWidget() - \e[0m destroyed\n");
}

QHistoricalViewWidget::QHistoricalViewWidget(QWidget *parent) : QStackedWidget(parent)
{
    printf("HistoricalViewWidget::HistoricalViewWidgetcount() %d\n", count());
    d_ptr = new QHistoricalViewWidgetPrivate();
}

void QHistoricalViewWidget::registerWidget(QWidget *w, DataType dt)
{
    int idx = addWidget(w);
    printf("\e[1;34m adding \"%s\" class \"\e[1;36m%s\e[0m\" to index %d\e[0m\n", qstoc(w->objectName()),
           w->metaObject()->className(), idx);
    d_ptr->addTypeIndex(dt, idx);
}

void QHistoricalViewWidget::switchWidget(DataType t)
{
    int idx = d_ptr->index(t);
    if(idx > -1)
    {
        printf("\e[1;32m widget switch requested to type %d\e[0m\n", t);
        setCurrentIndex(idx);
    }
    else
        perr("no widget registered for type %d", t);
}

QWidget *QHistoricalViewWidget::widget(DataType t)
{
    int idx = d_ptr->index(t);
    if(idx > -1)
        return QStackedWidget::widget(idx);
    return NULL;
}

