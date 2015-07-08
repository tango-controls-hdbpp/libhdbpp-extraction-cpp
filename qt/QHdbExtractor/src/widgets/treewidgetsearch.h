#ifndef TREEWIDGETSEARCH_H
#define TREEWIDGETSEARCH_H

class QTreeWidget;
class QString;

class TreeWidgetSearch
{
public:
    TreeWidgetSearch();

    void filter(const QTreeWidget* tree, const QString& search);
};

#endif // TREEWIDGETSEARCH_H
