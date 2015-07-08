#include "treewidgetsearch.h"
#include <QTreeWidget>
#include <QTreeWidgetItem>

TreeWidgetSearch::TreeWidgetSearch()
{

}

void TreeWidgetSearch::filter(const QTreeWidget* tree, const QString& search)
{
    QString substring = search;
    QList<QTreeWidgetItem *> items = tree->findItems("*", Qt::MatchWildcard|Qt::MatchRecursive);
    /* hide all */
    foreach(QTreeWidgetItem *it, items)
        it->setHidden(true);
    /* then show what matches */
    substring = "*" + substring + "*"; /* add wildcards */
    items = tree->findItems(substring, Qt::MatchWildcard|Qt::MatchRecursive);
    foreach(QTreeWidgetItem *it, items)
    {
        QTreeWidgetItem *parent = it;
        while(parent)
        {
            parent->setHidden(false);
            parent->setExpanded(true);
            parent = parent->parent();
        }
    }
}
