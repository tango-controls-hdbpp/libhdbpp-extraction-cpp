#include "errorstreewidget.h"
#include <QDateTime>
#include <stdio.h>

ErrorsTreeWidget::ErrorsTreeWidget(QWidget *parent) :
    QTreeWidget(parent)
{
    this->setSortingEnabled(true);
}

QTreeWidgetItem *ErrorsTreeWidget::findItem(const QString& src, double timestamp)
{
    QList<QTreeWidgetItem *> items = findItems("*", Qt::MatchWildcard);
    foreach(QTreeWidgetItem *it, items)
    {
        if(it->toolTip(0) == src && it->data(1, Qt::UserRole).toDouble() == timestamp)
        {
            return it;
        }
    }
    printf("\e[1;31m returnin NULL\e[0m\n");
    return NULL;
}

QTreeWidgetItem* ErrorsTreeWidget::addItem(const QString& src, double timestamp, int code, const QString& message)
{
    QTreeWidgetItem *it = findItem(src, timestamp);
    if(!it)
    {
        it = new QTreeWidgetItem(this, QStringList() <<
                                                  src.section('/', -2, -1) <<
                                                  QDateTime::fromMSecsSinceEpoch(timestamp * 1000).toString()
                                                  << QString::number(code));
        it->setToolTip(0, src);
        it->setToolTip(1, QDateTime::fromMSecsSinceEpoch(timestamp * 1000).toString());
        it->setToolTip(2, message);
        it->setData(0, Qt::UserRole, message);
        it->setData(1, Qt::UserRole, timestamp);

        sortByColumn(1, Qt::AscendingOrder);
    }
    return it;
}

void ErrorsTreeWidget::select(QTreeWidgetItem *ite)
{
    QList<QTreeWidgetItem *> items = selectedItems();
    foreach(QTreeWidgetItem *it, items)
        if(ite != it)
            it->setSelected(false);
    ite->setSelected(true);
}

